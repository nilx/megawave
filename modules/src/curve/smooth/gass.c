/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {gass};
   author = {"Lionel Moisan"};
   version = {"1.1"};
   function = {"Geometric Affine Scale Space of curves (Dlists)"};
   usage = {    
        
 'f':[first=0.]->first       "first scale",
 'l':[last=1.]->last         "last scale",
 'e':[eps=3.]->eps[2.,13.]   "relative sampling precision (-log10 scale)",
 's':step->step              "maximal scale step",
 'n':[n=5]->n                "or minimal number of iterations",
 'r':r->r                    "bounding box radius (default: minimal)",
 'v'->v                      "verbose mode",
 in->in                      "input curves (Dlists)",
 out<-out                    "output curves (Dlists)"

	    };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"

/*--- Global Variables ---*/
double **pts1,**pts2;
double tmp;

#define ABS(x)       ( (x)>0?(x):-(x) )
#define SGN(x)       (((x)==0.0)?0:(((x)>0.0)?(1):(-1)))

#define DET3(a,b,c) ((*(b)-*(a))*(*(c+1)-*(a+1)) - (*(b+1)-*(a+1))*(*(c)-*(a)))
#define area3(a,b,c) (tmp=DET3(a,b,c)/2.0,ABS(tmp))

#define EPSILON  1e-15   /* relative precision for a double */


/*** return +1, 0 or -1, the sign of det(b-a,c-b) modulo double precision ***/
static int dir(ax,ay,bx,by,cx,cy)
     double ax,ay,bx,by,cx,cy;
{
  double det,prec;

  det = (bx-ax)*(cy-by) - (by-ay)*(cx-bx);
  prec = EPSILON*(   ABS(bx-ax)*(ABS(cy)+ABS(by))
		   + ABS(cy-by)*(ABS(bx)+ABS(ax))
		   + ABS(by-ay)*(ABS(cx)+ABS(bx))
		   + ABS(cx-bx)*(ABS(by)+ABS(ay)) );
  if (ABS(det)<=prec) det = 0.0;
  return SGN(det);
}
  	

/*----------------- Split a curve into convex components -----------------*/

static int my_split_convex(in,out,ncc)
     Dlist  in;
     double **out;
     int    *ncc;
{
  int     il,i,d1,d2,ni,n,is_closed,ok;
  double  *p,*q,*pmax,mx,my,px1,py1,px2,py2,px3,py3,px4,py4,*first;

  if (!in) return(0);
  if (in->dim!=2) mwerror(FATAL,1,"Dlist dimension must be equal to 2\n");

  /*** initialization ***/
  p = in->values;
  pmax = p+in->size*2;
  is_closed = (*p==*(pmax-2) && *(p+1)==*(pmax-1));
  q = out[0];

  if (in->size<4) {
    while (p<pmax) *(q++) = *(p++);
    out[1] = q;
    return(1);
  }

  ni = 0; 
  *(q++) = px1 = *(p++); 
  *(q++) = py1 = *(p++);
  px2 = *(p++); py2 = *(p++);
  px3 = *(p++); py3 = *(p++);
  d2 = dir(px1,py1,px2,py2,px3,py3);

  /*** MAIN LOOP : d1=angle(P1-P2-P3) and d2=angle(P2-P3-P4) ***/
  
  ok = TRUE; first = NULL; il = 0;

  while (ok) {

    d1 = d2;
    px4 = *(p++); py4 = *(p++);
    d2 = dir(px2,py2,px3,py3,px4,py4);

    if (d1*d2>0) {
      
      /* convex part : store point and increment p */
      *(q++) = px1 = px2;
      *(q++) = py1 = py2;
      px2 = px3; py2 = py3;
      px3 = px4; py3 = py4;
      
    } else if (d1*d2<0) {
      
      /*** split curve ***/
      *(q++) = px2; *(q++) = py2;
      *(q++) = mx = .5*(px2+px3);
      *(q++) = my = .5*(py2+py3);
      out[++il] = q;
      if (p==first) ok = FALSE; 
      if (!first) first=p; 
      if (ok) {
	if (is_closed && !ni) q = out[il=0];
	*(q++) = mx; *(q++) = my;
	ni++;
	px1 = px2; py1 = py2;
	px2 = px3; py2 = py3;
	px3 = px4; py3 = py4;
      } 

    } else {
      
      /* indefined sign : remove one point */
      if (d1==0 || d2==0) {
	if (d1==0) {
	  px2 = px3; py2 = py3;
	} 
	px3 = px4; py3 = py4;
	d2 = dir(px1,py1,px2,py2,px3,py3);
      } else mwerror(FATAL,1,"Dlist contains NaN coordinates !\n");
    }
    
    /* test end of loop */
    if (ok && p==pmax) {
      if (is_closed) p = in->values+2;
      else ok = FALSE;
    }

    /* stop for convex closed curves */
    if (p==in->values+6 && ni==0) ok=FALSE;
  } 
  /*** END OF MAIN LOOP ***/

  if (!is_closed) {
    *(q++) = px2; *(q++) = py2;
    *(q++) = px3; *(q++) = py3;
    out[++il] = q;
  } else if (ni==0) {
    /* convex closed curve */
    if (q==out[0]+2) {
      *(q++) = out[0][0]; *(q++) = out[0][1];
    } else {
      out[0][0] = *(q-2); out[0][1] = *(q-1);
    }
    out[++il] = q;
  }

  if (ncc) {
    if (is_closed && !ni) ni = 1;
    if (!is_closed) ni++;
    *ncc = ni;
  }

  return(il);
}


/*------------------------------- SAMPLING  -------------------------------*/

/*** sample a curve : return next available address for out ***/
static double *sample(in,size,out,eps2)
     double  *in,*out;
     int     size;
     double  eps2;
{
  double  x,y,ox,oy,d2,dx,dy,threshold,*p,*q,*pmax;
  int     i,j,k,n,osize;

  /*--- return if the curve has less than 3 points ---*/
  if (size<3) {
    for(i=0;i<size*2;i++)
      *(out++) = *(in++);
    return(out);
  }

  p = in; q = out; 
  pmax = p + size*2;
  *(q++) = x = *(p++);
  *(q++) = y = *(p++);

  /*--- first pass : insert points ---*/
  while (p<pmax) {
    
    ox = x; oy = y;
    x  = *(p++); y = *(p++);
    dx = x-ox; dy = y-oy;
    d2 = dx*dx+dy*dy;

    if (d2>=2.*eps2) {
      /* insert n points */
      n = (int)floor(sqrt(d2/eps2));
      dx /= (double)n;
      dy /= (double)n;
      for (k=1;k<n;k++) {
	*(q++) = ox+dx*(double)k;
	*(q++) = oy+dy*(double)k;
      }
    } 
    *(q++) = x; *(q++) = y;
  }
  
  /*--- second pass : remove points ---*/
  pmax = q-2;
  p = out;
  q = p+2;
  ox = *(p++); oy = *(p++);

  while (p<pmax) {
    
    x  = *(p++); y  = *(p++);
    dx = x-ox; dy = y-oy;
    
    if (dx*dx+dy*dy>=eps2) {
      *(q++) = x; *(q++) = y;
      ox = x; oy = y;
    }
  }
  *(q++) = *(p++);
  *(q++) = *(p++);
  
  return(q);
}


/*** signed area of a polygonal sector p-q1-q2-p ***/
static double area_pol(p,q1,q2)
     double *p,*q1,*q2;
{
  double area,*q;

  area = 0.;
  for (q=q1;q!=q2;q+=2) {
    area += DET3(q,q+2,p);
  }
  return(area/2.0);
}

/*------------------------- AFFINE CONVEX EROSION -------------------------*/
static int aceros(in,size,out,area)
     double  *in,*out;
     int     size;
     double  area;
{
  int     j,j0,n,is_closed,stop,okp,okq;
  double  abs_area,tot_area,cur_area,inc_area,lambda;
  double  *p,*p0,*p1,*p2,*q0,*q1,*pmax;

  j = 0; 

  /* deal with singular cases (less than 2 points) */
  if (size<2) return(0);

  /*** test if the curve is closed ***/
  p = in;
  pmax = p+size*2;
  is_closed = (*p==*(pmax-2) && *(p+1)==*(pmax-1));
  if (is_closed) pmax -= 2;

  /* deal with singular cases (2 or 3 points, closed) */
  if (size<4 && is_closed) return(0);

  /* return input if segment or area=0 */
  if (size==2 || area==0.) {
    while (p<pmax) out[j++]=*(p++);
    return(j/2);
  }

  /*** compute total area ***/
  tot_area = area_pol(p,p+2,pmax-2);
  tot_area = ABS(tot_area);

  /*** check extinction ***/
  if (is_closed) {
    if (area>=tot_area/2.1) { /*** theoretically : 2.0 ***/
      mwdebug("Effective extinction area = %g\n",tot_area/2.0);
      return(0);
    }
  } else if (area>=tot_area) {
    out[j++] = *p;        out[j++] = *(p+1);
    out[j++] = *(pmax-2); out[j++] = *(pmax-1);
    mwdebug("Effective extinction area = %f\n",tot_area);
    return(j/2);
  }      

  if (is_closed) {
    n = (size-1)*2;
  } else {
    out[j++] = *p;
    out[j++] = *(p+1);
  }
  p0 = p; q0 = p1 = p0+2; q1 = q0+2;
  cur_area = 0.0;
  okp = okq = !TRUE;

  /*** MAIN LOOP : compute the middle points of significative chords ***/
  do {

    if (cur_area<=area) {
      inc_area = area3(q0,q1,p0);

      if (cur_area+inc_area>=area) {
	/*** compute middle point ***/
	lambda = (double)( (area - cur_area)/inc_area );
	out[j++] = .5*(*p0     + (1-lambda)**q0    +lambda**q1);
	out[j++] = .5*(*(p0+1) + (1-lambda)**(q0+1)+lambda**(q1+1));
      }
      if (cur_area+inc_area-area3(p0,p1,q1)>area) {
	cur_area -= area3(p0,p1,q0);
	p0 = p1; p1 += 2; if (is_closed && p1>=pmax) p1-=n;
	if (p0==p) okp=TRUE;
      } else {
	cur_area += inc_area;
	q0 = q1; q1 += 2; if (is_closed && q1>=pmax) q1-=n;
	if (q0==p+2) okq=TRUE;
      }
    } else {
      inc_area = area3(p0,p1,q0);
      if (cur_area-inc_area<=area) {
	/*** compute middle point ***/
	lambda = (double)( (cur_area - area)/inc_area );
	out[j++] = .5*(*q0     + (1-lambda)**p0    +lambda**p1);
	out[j++] = .5*(*(q0+1) + (1-lambda)**(p0+1)+lambda**(p1+1));
      }
      if (!is_closed && q1!=pmax && cur_area-inc_area+area3(p1,q0,q1)<area) {
	cur_area+=area3(p0,q0,q1);
	q0 = q1; q1 += 2; if (is_closed && q1>=pmax) q1-=n;
	if (q0==p+2) okq=TRUE;
      } else {
	cur_area -= inc_area;
	p0 = p1; p1 += 2; if (is_closed && p1>=pmax) p1-=n;
	if (p0==p) okp=TRUE;
      }
    }
    
    /*** more precise computation of cur_area if needed ***/
    if (p1==q0) cur_area=0.0;
    p2 = p1+2; if (is_closed && p2>=pmax) p2-=n;
    if (p2==q0) cur_area=area3(p0,p1,q0);   

    if (is_closed) stop = (okq && okp);      
    else stop = ((q0+2==pmax) && (cur_area<=area));

  } while (!stop);

  /*** add last point to output ***/
  if (is_closed) {
    out[j++] = out[0];
    out[j++] = out[1];
  } else {
    out[j++] = *(pmax-2);
    out[j++] = *(pmax-1);
  }
  return(j/2);
}



/*----------------------- DISCRETE AFFINE EROSION  -----------------------*/

static void dafferos(l,area,eps2,rad,ncc)
     Dlist    l;      /* input/output curve */
     double   *area;  /* desired absolute area step (real one is returned) */
     double   eps2;   /* absolute precision squared */
     double   rad;    /* bounding box radius */
     int      *ncc;   /* output: number of connected components after evol. */
{
  double        a,narea,r2,min_area,*first,*last;
  int           i,nc;

  min_area = *area/8.0;    /*** critical area for effective erosion ***/

  /*** compute convex components ***/
  nc = my_split_convex(l,pts1,ncc);

  /*** compute minimal area ***/
  narea = *area;
  for (i=0;i<nc;i++) {

    /*** resample curve ***/
    first = pts2[i];
    pts2[i+1] = sample(pts1[i],(pts1[i+1]-pts1[i])/2,first,eps2);
    last = pts2[i+1]-2;
    a = (double)area_pol(first,first,last);
    a = ABS(a);
    if (*first==*last && *(first+1)==*(last+1)) a = narea;
    if (a>min_area && a<narea) narea = a;
  }
  if (narea<*area)  *area = narea;

  /*** apply aceros to convex components and link result ***/
  l->size = 0;
  for (i=0;i<nc;i++) 
    l->size += aceros(pts2[i],(pts2[i+1]-pts2[i])/2,l->values+l->size*2,narea);
}




/*------------------------------ MAIN MODULE  ------------------------------*/


Dlists gass(in,out,first,last,eps,step,n,r,v)
     Dlists     in,out;
     double     *first,*last,*eps,*step,*r;
     char       *v;
     int        *n;
{
  int       i,j,ncc,MAX_PTS,MAX_CC,npts,maxsize;
  double    a,remaining_h,remaining_a,rad,eps2,omega,step_a,t;
  double    x,y,ox,oy,r2,r2max,*per,maxper;

  if (*n<0) mwerror(FATAL,1,"n must be positive\n");

  /* compute bounding box radius, perimeters and max size */
  per = (double *)malloc(in->size*sizeof(double));
  if (!per) mwerror(FATAL,1,"Not enough memory\n");
  r2max = maxper = 0.;
  maxsize = 0;
  for (j=0;j<in->size;j++) {
    if (in->list[j]->size>maxsize) maxsize = in->list[j]->size;
    per[j] = 0.;
    for (i=0;i<in->list[j]->size;i++) {
      x = in->list[j]->values[2*i];
      y = in->list[j]->values[2*i+1];
      r2 = x*x+y*y;
      if (r2>r2max) r2max=r2;
      if (i!=0) per[j] += ABS(x-ox)+ABS(y-oy);
      ox = x; oy = y;
    }
    if (per[j]>maxper) maxper = per[j]; 
  }
  if (!r) rad = sqrt(r2max); else rad = *r;

  /*** absolute precision (squared) ***/
  eps2 = pow(10.0,-2.0*(*eps))*rad*rad;   

  /* allocate memory */
  MAX_PTS = 2*(int)(maxper/sqrt(eps2))+2*maxsize+10;
  MAX_CC = maxsize+10;
  pts1 = (double **)malloc(MAX_CC*sizeof(double *));
  pts2 = (double **)malloc(MAX_CC*sizeof(double *));
  if (!pts1 || !pts2) mwerror(FATAL,1,"Not enough memory\n");
  pts1[0] = (double *)malloc(2*MAX_PTS*sizeof(double));
  pts2[0] = (double *)malloc(2*MAX_PTS*sizeof(double));
  if (!pts1[0] || !pts2[0]) mwerror(FATAL,1,"Not enough memory\n");

  out = mw_change_dlists(out,in->size,in->size);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");

  if (v) 
    printf("Common memory allocated : %d ko\n",(MAX_CC*8+MAX_PTS*4*8)/1000);

  /* normalization constant */
  omega = (0.5*pow(1.5,2.0/3.0));


  /*----- FIRST LOOP (all curves) -----*/

  for (j=0;j<in->size;j++) {
  
    if (v) printf("\n--- CURVE #%d\n",j);

    npts = 2*(int)(per[j]/sqrt(eps2))+in->list[j]->size+10;

    out->list[j] = mw_change_dlist(out->list[j],npts,0,2);
    if (!out->list[j]->values) mwerror(FATAL,1,"Not enough memory\n");
    mw_copy_dlist(in->list[j],out->list[j]);

    /* remaining scale (additive normalization) */
    remaining_h = ( pow((*last),4.0/3.0) -
		    pow((*first),4.0/3.0) ) * 0.75/omega;
    
    /*** compute scale (area) step ***/
    if (step) step_a = (*step)*(*step)*pow((0.75/omega),1.5);
    else      step_a = pow(remaining_h/(double)(*n),1.5);
    
    /*----- SECOND LOOP (iterations) -----*/
    
    for (i=1; out->list[j]->size && remaining_h>0.0;i++) {
      
      remaining_a = pow(remaining_h,1.5);
      a = (remaining_a<step_a?remaining_a:step_a);
      
      /*** one step ***/
      dafferos(out->list[j],&a,eps2,rad,&ncc);    
      
      remaining_h -= pow(a,2.0/3.0);
      
      if (v) {
	t = *last-pow(omega*(remaining_h>=0.0?remaining_h:0.0)/0.75,0.75);
	printf("Iter %3d : %6d cc, effective area = %f, current scale = %f\n",
	       i,ncc,a,t);
	if (!out->list[j]->size) printf("The curve has collapsed.\n");
      }
    }

    if (out->list[j]->size) {
      for (i=0;i<out->list[j]->size*2;i++) pts1[0][i]=out->list[j]->values[i];
      out->list[j]->size = 
 	(sample(pts1[0],out->list[j]->size,out->list[j]->values,eps2) 
	 - out->list[j]->values)/2;
    }
  }

  free(pts2[0]);
  free(pts1[0]);
  free(pts2);
  free(pts1);
  free(per);

  return(out);
}

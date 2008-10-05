/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {gcsf};
   version = {"1.3"};
   author = {"Frederic Cao, Lionel Moisan"};
   function = {"Generalized Curve Shortening Flow of a curve"};
   usage = {

 'g':[g=1.0]->gam           "power of the curvature (double > 1/3.)",
 'f':[f=0.]->first          "first scale",
 'l':[l=1.]->last           "last scale",
 'e':[eps=3.]->eps[2.,13.]  "relative sampling precision (-log10 scale)",
 'n':[n=1]->n               "minimal number of iterations",
 'r':r->r                   "bounding box radius (default: minimal)",
 'i':i->iter                "number of iterations",
 'c'->conv                  "do not convexify last iteration",
 'a':[a=4.]->area[2.,13.]   "relative eroded area (-log10 area)",
 'v'->v                     "verbose mode",
 in->in                     "input (Dlists)",
 out<-out                   "output (Dlists)"

};
*/
/*----------------------------------------------------------------------
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"


/*--- Global Variables ---*/
double **pts1,**pts2,**chord1,**chord2;
double tmp;

#define ABS(x)      ( (x)>0.0?(x):-(x) )
#define SGN(x)      (((x)==0.0)?0.0:(((x)>0.0)?(1.0):(-1.0)))
/*  #define DET3(a,b,c) ((*(b)-*(a))*(*(c+1)-*(a+1)) - (*(b+1)-*(a+1))*(*(c)-*(a))) */
#define area3(a,b,c) (tmp=DET3(a,b,c)/2.0,ABS(tmp))


#define EPSILON  1e-15  /*** relative precision for a double ***/

double DET3(a,b,c)
     double *a,*b,*c;
{
  return((*b-*a)*(*(c+1)-*(a+1)) - (*(b+1)-*(a+1))*(*c-*a));
}

/*** distance between two points ***/
double norm(u,v)
     double *u,*v;
{
  return (sqrt((*u-*v)*(*u-*v)+(*(u+1)-*(v+1))*(*(u+1)-*(v+1))));
}
/*** squared distance between two points ***/
double norm2(u,v)
     double *u,*v;
{
  return ((*u-*v)*(*u-*v)+(*(u+1)-*(v+1))*(*(u+1)-*(v+1)));
}

/*** dot product of two affine vectors ***/
double dot3(u,v,w)
     double *u,*v,*w;
{
  return((*v-*u)*(*w-*u)+(*(v+1)-*(u+1))*(*(w+1)-*(u+1)));
}




/*** signed area of a polygonal sector p-q1-q2-p ***/
double area_pol(p,q1,q2)
     double *p,*q1,*q2;
{
  double area,*q;

  area = 0.;
  for (q=q1;q!=q2;q+=2) {
    area += DET3(q,q+2,p);
  }
  return(area/2.0);
}

/*** return +1, 0 or -1, the sign of det(b-a,c-b) modulo double precision ***/
int dir(ax,ay,bx,by,cx,cy)
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

/*** return +1, 0 or -1, the sign of <a,b> modulo double precision ***/
int sgdot(ux,uy,vx,vy)
     double ux,uy,vx,vy;
{
  double dot,prec;

  dot = ux*vx+uy*vy;
  prec = EPSILON*(ABS(ux)*ABS(vx)+ABS(uy)*ABS(vy));
  if (ABS(dot)<=prec) dot = 0.0;
  return SGN(dot);
}

/*----------------- Split a curve into convex components -----------------*/

int my_split_convex(in,out,ncc)
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
      if(p==first){
	ok = FALSE;
	*(q++) = .5*(px2+px3);
	*(q++) = .5*(py2+py3);
	out[++il] = q;
      }
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
      if(p==first){
	ok = FALSE;
	*(q++) = px2; *(q++) = py2;
	*(q++) = .5*(px2+px3); *(q++) = .5*(py2+py3);
	out[++il] = q;
      }
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
double *sample(in,size,out,eps2)
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

/*** sample a curve and create the corresponding chord direction ***/
/*** return the number of points of the new curve ***/
int sample_chord(in,ch_in,size,out,ch_out,eps2)
     double  *in,*out,*ch_in,*ch_out;
     int     size;
     double  eps2;
{
  double  x,y,ox,oy,d2,dx,dy,threshold,*p,*q,*pmax,*c,*d,*cmax;
  int     i,j,k,n,osize;


  /*--- return if the curve has less than 3 points ---*/
  if (size<3) {
    for(i=0;i<size*2;i++){
      *(out++) = *(in++);*(ch_out++) = *(ch_in++);
    }
    return(size);
  }

  j = 0;
  p = in; q = out;
  c = ch_in; d = ch_out;
  pmax = p + size*2;
  *(q++) = x = *(p++);
  *(q++) = y = *(p++);
  *(d++) = *(c++);
  *(d++) = *(c++);
  j++;

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
	*(d++) = *(c-2); *(d++) = *(c-1);
	j++;
      }
    }
    *(q++) = x; *(q++) = y;
    *(d++) = *(c++); *(d++) = *(c++);
    j++;
  }
  return j;
}



/*** search furthest point on the arc  ***/
double searchfar(in,pmax,is_closed,first,last,fixed,pt,far,prevfar,firstfar)
     double    *in,*pmax,*first,*last,*fixed,*pt,**far,**prevfar;
     int       *firstfar,is_closed;
{
  double length,h,height;
  double *p,*q;
  int secu,s;

  s = 3;
  length = norm(fixed,pt);
  h = height = 0.0;
  if (*firstfar) *far = first;
  if(length==0.0){
    *firstfar = TRUE;
    p = first;
    secu = 0;
    while(p!=last && secu<s){
      h = norm(fixed,p);
      secu++;
      if(h>height){height = h; *far = p; secu = 0;}
      if(is_closed) {if (p+2==pmax) p = in; else p +=2;}
      else p +=2;
    }
  }else{
    p = q = *far;
    secu = 0;
    while(p!=last && secu<s){
      h = DET3(fixed,pt,p)/length;
      secu++;
      if (ABS(h)>ABS(height)){ height = h; *far = p; secu = 0;}
      if(is_closed) {if (p+2==pmax) p = in; else p +=2;}
      else p += 2;
    }
    secu = 0;
    p = q;
    while (p!=first && secu<s){
      h = DET3(fixed,pt,p)/length;
      secu++;
      if(ABS(h)>ABS(height)){height = h; *far = p; secu = 0;}
      if(is_closed) {if (p==in) p = pmax; else p -=2;}
      else p -= 2;
    }
    if (*firstfar) *prevfar = *far;
    *firstfar = FALSE;
  }
  return height;
}






/*** compute the coordinates of a new point of curve ***/
void gamma_point(ext1,ext2,far,height,gp,gam,a,alpha)
     double *ext1,*ext2,*far,*gp;
     double a,alpha,height,gam;
{
  double abscissa,h,d,length,sn,cn,dx,dy,u;

  length = norm(ext1,ext2);
  if (length==0.0){
    *gp = *ext1; *(gp+1) = *(ext1+1);
  } else {
    cn = (*ext2-*ext1)/length; sn = (*(ext2+1)-*(ext1+1))/length;

    /*** give the position of the point of a convex arc
	 that achieves the chord-arc distance :
	 compute the abscissa w.r.t to the middle point
	 and the distance to the chord***/
    if (ABS(height)<=a*alpha){
      abscissa = dot3(ext1,ext2,far)/length-0.5*length;
      /*** movement of the middle point due to
	   the position of the furthest point***/
      d = abscissa*(1.0-3.0*gam*pow(ABS(height/a),3.0*gam-1.0));
      dx = d*cn; dy = d*sn;
    /*** movement due to the power of the curvature ***/
    /*** remind that this displacement depends on the sign of height ***/
      u = height*(1.0-pow(ABS(height)/a,3.0*gam-1.0));
      dx += -sn*u; dy += cn*u;
      *gp = 0.5*(*ext1+*ext2)+dx;
      *(gp+1) = 0.5*(*(ext1+1)+*(ext2+1))+dy;
    } else {
      /* not supposed to happen : check it */
      u = SGN(height)*a*alpha*(1.0-pow(alpha,3.0*gam-1.0));
      dx = -sn*u; dy = cn*u;
      *gp=0.5*(*ext1+*ext2)+dx;
      *(gp+1)=0.5*(*(ext1+1)+*(ext2+1))+dy;
    }
  }
}




/*-------------------- Gamma CONVEX erosion --------------------*/
/* return the number of created points */
int gaceros(in,ch,size,out,gam,area,a,alpha,eps2,sign)
     double   *in,*ch,*out;
     double   gam,area,a,alpha,eps2,*sign;
     int      size;
{
  double     *p,*p0,*p1,*p2,*pmax,*q0,*q1,*v,*w,*pt,*far,*prevfar,*cv,*gp;
  double     abs_area,tot_area,cur_area,inc_area,height,h,d,l,lambda,eps;
  int        j,n,is_closed,stop,okp,okq,firstfar,firstmove;


  eps = sqrt(eps2);
  j = 0;
  v = (double *) malloc(2*sizeof(double));
  w = (double *) malloc(2*sizeof(double));
  gp = (double *) malloc(2*sizeof(double));

  /* exit if input does not contain two points */
  if(size<2) return (0);

  /*** test if the curve is closed ***/
  p = in;
  pmax = p+size*2;
  is_closed = (*p==*(pmax-2) && *(p+1)==*(pmax-1));
  if (is_closed) pmax -= 2;

  /* deal with singular cases (2 or 3 points, closed) */
  if(size<4 && is_closed) return(0);

  /* return input if segment or area==0 */
  if(size==2 || area==0.){
    while(p<pmax) out[j++]=*(p++);
    return(j/2);
  }

  /* compute total area */
  tot_area = area_pol(p,p+2,pmax-2);
  *sign = SGN(tot_area);
  tot_area = ABS(tot_area);

  /*** check extinction ***/
  if (is_closed) {
    if (area>=tot_area/2.0) {
      mwdebug("Effective extinction area = %f\n",tot_area/2.0);
      return(0);
    }
  } else if (area>=tot_area) {
    /* output has two points : the end points of the input*/
    /* the chords are the segment itself */
    out[j] = *p; ch[j++] = *(pmax-2)-*p;
    out[j] = *(p+1) ; ch[j++] = *(pmax-1)-*(p+1);
    out[j] = *(pmax-2); ch[j++] = *(pmax-2)-*p;
    out[j] = *(pmax-1); ch[j++] = *(pmax-1)-*(p+1);
    return(j/2);
  }
  if(is_closed){
    n = (size-1)*2;
  } else {
    out[j] = *p; ch[j++] = *(p+2)-*p;
    out[j] = *(p+1) ; ch[j++] = *(p+3)-*(p+1);
  }

  /*** compute points associated to significative chords ***/
  p0 = p; q0 = p1 = p0+2; q1 = q0+2;
  cur_area = 0.0;
  okp = okq = !TRUE;

  firstfar=TRUE;
  far=prevfar=p;
  firstmove=TRUE;
  do {
    if (cur_area<=area) {
      inc_area = area3(q0,q1,p0);

      if (cur_area+inc_area>=area) {
	/*** compute "middle" point ***/
	/* it may happen that cur_area=area and inc_area=0.
	   In this case we choose the middle point of the segment*/
	if (inc_area>0) lambda = (double)( (area - cur_area)/inc_area );
	else lambda=0.5;
	*v = ((1-lambda)**q0+lambda**q1);
	*(v+1) = ((1-lambda)**(q0+1)+lambda**(q1+1));
	/*** search the furthest point on the arc ***/
	height = searchfar(p,pmax,is_closed,p0,q1,p0,v,
			   &far,&prevfar,&firstfar);
      	prevfar = far;
	gamma_point(p0,v,far,height,gp,gam,a,alpha);
	/* add the new point if it is far enough from the previous one */
  	if (j==0 || (j>=2 && (norm2(out+(j-2),gp)>eps2/25.0))){
	  out[j] = *gp; ch[j++] = *v-*p0;
	  out[j] = *(gp+1); ch[j++]= *(v+1)-*(p0+1);
	}
	firstmove=TRUE;
      }
      if (cur_area+inc_area-area3(p0,p1,q1)>area) {
	cur_area -= area3(p0,p1,q0);
	p0 = p1; p1 += 2; if(is_closed && p1>=pmax) p1-=n;
	/*force furthest point to stay between extremal points*/
	if (far==p0) far = p1;
	if (p0==p) okp = TRUE;
      } else {
	cur_area += inc_area;
	q0 = q1;
	if(is_closed) {if(q1+2==pmax) q1 = p; else q1 += 2;}
	else {if(q1+2!=pmax) q1 += 2;}
	if (q0==p+2) okq = TRUE;
      }
    } else {
      inc_area = area3(p0,p1,q0);
      if (cur_area-inc_area<=area) {
	/*** compute middle point ***/
	/* if we are here, this means that cur_area>area.
	   Then inc_area cannot be 0*/
	lambda = (double)( (cur_area - area)/inc_area );
	*w = ((1-lambda)**p0+lambda**p1);
	*(w+1) = ((1-lambda)**(p0+1)+lambda**(p1+1));
	height=-searchfar(p,pmax,is_closed,p1,q1,q0,w,&far,&prevfar,&firstfar);
	prevfar = far;
	gamma_point(w,q0,far,height,gp,gam,a,alpha);
	/* add the new point if it is far enough from the previous one */
  	if (j==0 || (j>=2 && (norm2(out+(j-2),gp)>eps2/2500.0))){
	  out[j] = *gp; ch[j++] = *q0-*w;
	  out[j] = *(gp+1); ch[j++]= *(q0+1)-*(w+1);
	}
	firstmove=FALSE;
      }
      if (cur_area-inc_area+area3(p1,q0,q1)<area) {
	cur_area+=area3(p0,q0,q1);
	q0 = q1 ;
	if(is_closed) {if (q1+2==pmax) q1 = p; else q1 +=2;}
	else {if(q1+2!=pmax) q1 +=2;}
	if (q0==p+2) okq=TRUE;
      } else {
	cur_area -= inc_area;
	p0 = p1; p1 += 2; if(is_closed && p1>= pmax) p1-=n;
	/*force the furthest point to stay between p and q*/
	if (far==p0) far = p1;
	if (p0==p) okp = TRUE;
      }
    }

    /*** more precise computation of cur_area if needed ***/
    if (p1==q0) cur_area=0.0;
    p2 = p1+2; if(is_closed && p2>=pmax) p2-=n;
    if(p2==q0) cur_area = area3(p0,p1,q0);

    if (is_closed) stop = (okq && okp && p0==p+2);
    else stop = ((q0+2==pmax) && (cur_area-inc_area<=area));
  } while (!stop);

  /*** if the curve is closed make it loop
       else add last point ***/
  if(is_closed){
    out[j] = out[0]; ch[j++] = ch[0];
    out[j] = out[1]; ch[j++] = ch[1];
  } else {
    out[j] = *(pmax-2); ch[j++] = *(pmax-2)-*(pmax-4);
    out[j] = *(pmax-1); ch[j++] = *(pmax-1)-*(pmax-3);
  }
  free(v); free(w); free(gp);
  return(j/2);
}






/* Compute the chord arc-distance of a curve. The "fixed" point is not a
   point of the curve since it is interpolated to fit the area erosion.
   The furthest point is reused to decrease research time.*/
double height(in,pmax,is_closed,first,last,fixed,pt,far,firstfar)
     double  *in,*pmax,*first,*last,*pt,**far,*fixed;
     int *firstfar,is_closed;
{
  double *p,*f;
  double a,h,l;
  int secu,s;
  int okp,okq;


  s=3;
  l = norm(fixed,pt);
  h = a = 0.0;
  if(l==0.0){
    *firstfar = TRUE;
    p = first;
    secu=0;
    while(p!=last && secu<s){
      a = norm(fixed,p);
      secu++;
      if(a>h){h = a; *far = p; secu = 0;}
      p += 2; if(is_closed && p==pmax) p = in;
    }
  }else{
    f = p = *far;
    secu=0;
    while(p!=last && secu<s){
      a = DET3(fixed,pt,p)/l;
      secu++;
      if (ABS(a)>h){h = ABS(a);	f = p; secu = 0;}
      p += 2; if(is_closed && p==pmax) p = in;
    }
    *far=f;
    *firstfar=FALSE;
  }
  return h;
}


/*** compute tha max chord arc distance when the area is known***/
double saturation(in,size,area)
     double *in;
     int size;
     double area;
{
  double *p0,*p1,*q0,*q1,*p,*p2,*pt,*pmax,*far;
  int okp,okq,stop,is_closed;
  double h,a,cur_area,inc_area,tot_area;
  int firstfar,n;
  double lambda;


  /* exit if input does not contain three points */
  if (size<3) return 0.0;

  p=in;
  pmax=p+2*size;
  is_closed=(*p==*(pmax-2) && *(p+1)==*(pmax-1));
  if (is_closed) pmax -= 2;

 /*** compute total area ***/
  tot_area = area_pol(p,p+2,pmax-2);
  mwdebug("total area = %4f\n",tot_area);
  tot_area = ABS(tot_area);

 /*** compute points associated to significative chords ***/
  firstfar = TRUE;
  p = far = in;

  /* if the area of the convex component is too small,
     return total height of the component */
  if (tot_area<=area) {
    return height(in,pmax,is_closed,p,pmax,p,pmax,&far,&firstfar);
  }

  if (is_closed) n=(size-1)*2;
  pt=(double *) malloc(2*sizeof(double));
  p0 = in; q0 = p1 = p0+2; q1 = q0+2;
  cur_area = 0.0;
  a=h=0.0;
  okp = okq = !TRUE;
  do {
    if (cur_area<=area) {
      inc_area = area3(q0,q1,p0);
      if (cur_area+inc_area>=area) {
	lambda = (double)( (area - cur_area)/inc_area );
	*pt = ((1-lambda)**q0+lambda**q1);
	*(pt+1) = ((1-lambda)**(q0+1)+lambda**(q1+1));
	/*** search the furthest point on the arc ***/
	a=height(in,pmax,is_closed,p0,q1,p0,pt,&far,&firstfar);
	if (a>h) h=a;
      }

      if (cur_area+inc_area-area3(p0,p1,q1)>area) {
        cur_area -= area3(p0,p1,q0);
        p0 = p1; p1 += 2; if (is_closed && p1 >= pmax) p1 -= n;
        if (p0==in) okp=TRUE;
      } else {
	cur_area += inc_area;
	q0 = q1;  q1 += 2; if(is_closed && q1 >= pmax) q1 -= n;
	if (q0==in+2) okq=TRUE;
      }
    } else {
      inc_area = area3(p0,p1,q0);
      if (cur_area-inc_area<=area) {
	lambda = (double)( (cur_area - area)/inc_area );
	*pt = ((1-lambda)**p0+lambda**p1);
	*(pt+1) = ((1-lambda)**(p0+1)+lambda**(p1+1));
	a=height(in,pmax,is_closed,p0,q0,q0,pt,&far,&firstfar);
	if (a>h) h=a;
      }
      if (!is_closed && q1!=pmax && cur_area-inc_area+area3(p1,q0,q1)<area) {
        cur_area+=area3(p0,q0,q1);
        q0 = q1; q1 += 2; if (is_closed && q1>=pmax) q1 -= n;
        if (q0==in+2) okq=TRUE;
      } else {
	cur_area -= inc_area;
	/*force furthest point to stay between the two ends of the chord*/
	if (far==p0) far=p1;
	p0 = p1; p1 += 2; if (is_closed && p1>=pmax) p1-=n;
	if (p0==in) okp=TRUE;
      }
    }


    /*** more precise computation of cur_area if needed ***/
    if (p1==q0) cur_area=0.0;
    p2 = p1+2 ; if (is_closed && p2>=pmax) p2 -= n;
    if(p2==q0) cur_area=area3(p0,p1,q0);

    if (is_closed) stop = (okq && okp);
    else stop = ((q0+2==pmax) && (cur_area-inc_area<=area));
  } while (!stop);
  free(pt);
  return h;
}

/* convexify a closed curve created by gaceros */
/* i.e. remove the swallow tails */
/* for this, we compare the position of the created points with the */
/* position of the chords */



int convexify(in,ch,remove,size,sign)
     double *in,*ch,*remove;
     double sign;
     int size;
{
  double *p0,*p1,*q0,*q1,*p2,*pmax,*p;
  int j,pushp,is_closed,ok,stop,go_ahead,forward,removed=0;

  /* start by removing points where the direction of the
     curve and the tangent are opposed */
  for(j=2;j<2*size;j+=2)
    removed += remove[j] = (sgdot(ch[j],ch[j+1],in[j]-in[j-2],in[j+1]-in[j-1])<0);
  pmax = in+2*size;
  is_closed = (*in==*(pmax-2) && *(in+1)==*(pmax-1));
  if (is_closed) {
    pmax -= 2;
    removed += remove[0] = (sgdot(ch[0],ch[1],in[0]-*(pmax-2),in[1]-*(pmax-1))<0);
  }
  else remove[0]=remove[2*size-2]=0; /* cannot remove extremal points*/

  /*** return curve if size < 3 ***/
  if(size<3){
    if(is_closed) return(0);
    else return(2);
  }
  p1 = in+2;
  /* search first pairs of remaining points */
  while(remove[p1-in] && p1+2!=pmax) p1+=2;
  p0 = p1-2;
  if (p1+2==pmax) q1 = in; else q1 = p1+2;
  while(remove[q1-in] && q1+2!=pmax) q1+=2;
  q0 = q1-2;
  p =p1;
  forward = TRUE;
  stop = go_ahead = 0;
  if (is_closed) ok = FALSE; else ok = TRUE;

  while(!stop || !ok){
    if(forward){
/*        if ((sign*dir(*p0,*(p0+1),*p1,*(p1+1),*q1,*(q1+1)) */
/*  	  *sgdot(ch[p1-in],ch[p1-in+1],*p1-*p0,*(p1+1)-*(p0+1)))<0){ */
      if (sign*dir(*p0,*(p0+1),*p1,*(p1+1),*q1,*(q1+1))<0){
	if(!is_closed){
	  if(q1+2==pmax){/* cannot remove last point*/
	    remove[p1-in]= 1; removed ++;
	    while(remove[p1-in] && p1!=in) p1 -=2;
	    if(p1==in) stop = TRUE; else p0 = p1-2;
	  } else {
	    remove[q1-in] = 1; removed ++;
	    while(remove[q1-in]&& q1+2!=pmax) q1 += 2;
	    q0 = q1-2;
	  }
	} else {
	  remove[q1-in] = 1; removed ++;
	  pushp = (q1==p);
	  while(remove[q1-in] && q1!=p1){
	    if(q1+2==pmax) q1 = in;  else q1 += 2;
	  }
	  if (q1==in) q0 = pmax-2; else q0 = q1-2;
	  if (pushp) p = q1;
	}
	go_ahead = 0;
      } else  go_ahead += 1;
      forward = FALSE;
    } else {
      if(sign*dir(*q0,*(q0+1),*q1,*(q1+1),*p1,*(p1+1))<0){
/*  	if((sign*dir(*q0,*(q0+1),*q1,*(q1+1),*p1,*(p1+1)) */
/*  	    *sgdot(ch[q1-in],ch[q1-in+1],*q1-*q0,*(q1+1)-*(q0+1)))<0){ */
	if(!is_closed){
	  if(p1==in){/*cannot remove first point*/
	    if(q1+2==pmax) stop = TRUE;
	    else {
	      remove[q1-in] = 1; removed++;
	      while(remove[q1-in] && q1+2!=pmax) q1 +=2;
	      q0 = q1-2;
	      }
	  } else {
	    remove[p1-in]=1; removed++;
	    while(remove[p1-in] && p1!=in) p1 -= 2;
	    if(p1==in) { /*p1 is first point : force to go forward*/
	      if(q1+2==pmax) stop = TRUE;
	      else {
		p1 = q1; p0 = p1-2;
		q1 += 2;
		while(q1+2!=pmax && remove[q1-in]) q1 += 2;
		q0 = q1-2;
	      }
	    } else p0 = p1-2;
	  }
	} else {
	  pushp = (p1==p);
	  remove[p1-in] = 1; removed++;
	  while(remove[p1-in] && p1!=q0){
	    if (p1==in) p1 = pmax-2; else p1 -= 2;
	  }
	  if(p1==in) p0 = pmax-2; else p0 = p1-2;
	  if(pushp) p = p1; ok = !(p1==p);
	}
	go_ahead = 0;
      } else  go_ahead += 1;
      forward = TRUE;
    }
    if (go_ahead==2){
      p1 = q1;
      if(is_closed && p1==in) p0 = pmax-2;
      else p0 = p1-2;
      if(!is_closed) {
	if(q1+2==pmax) stop = TRUE;
	else {
	  q1 += 2;
	  while(remove[q1-in] && q1+2!=pmax) q1 +=2;
	  q0 = q1-2;
	}
      } else{
	if (q1+2==pmax) q1 = in; else q1 += 2;
	while(remove[q1-in] && q1!=p1){
	  if (q1+2==pmax) q1 = in; else q1 += 2;
	}
	if (q1==in) q0 = pmax-2; else q0 = q1-2;
      }
      go_ahead = 0; ok = TRUE;

      if(is_closed) stop = (p1==p);
      else if (!stop) stop = (p1+2==pmax);
    }
  }

  /*** remove tagged points ***/
  p = in;
  j = 0;
  while(p!=pmax){
    if(remove[p-in]) p += 2;
    else {in[j++] = *(p++); in[j++] = *(p++);}
  }
  if(is_closed) {in[j++] = in[0]; in[j++] = in[1];}
  if(!is_closed &&(in[j-2]!=*(pmax-2) || in[j-1]!=*(pmax-1)))
    mwerror(FATAL,1,"This should not happen\n");
  return(j/2);
}





/*------------ Gamma erosion -------------*/
/* global variables explicitly appear */

void g_eros(li,gam,scale,area,eps2,ncc,conv)
     Dlist    li;     /* input/output Dlist */
     double   eps2;   /* relative precision squared */
     double   gam;    /* power of the curvature*/
     double   *scale; /* final scale (return real one)*/
     double   *area;  /* erosion area */
     int      *ncc;   /* number of convex components after erosion*/
     int      conv;   /*do NOT convexify after erosion*/
{
  double        eps,a,b,alpha,delta,narea,min_area,d,h,sign;
  double        *first,*last;
  int           nc,nbpt,npc,*npt,i;


  eps = sqrt(eps2);
  /*** compute convex components ***/
  nc = my_split_convex(li,pts1,ncc);

  npt = (int *) malloc(nc*sizeof(int));
  /*** compute minimal area ***/
  min_area = *area/8.0;    /*** critical area for effective erosion ***/
  narea = *area;
  for (i=0;i<nc;i++) {
    /*** resample curve ***/
    first = pts2[i];
    pts2[i+1] = sample(pts1[i],(pts1[i+1]-pts1[i])/2,first,eps2);
    npt[i] = (pts2[i+1]-pts2[i])/2;
    last = pts2[i+1]-2;
    /* because of swallow tails, the erosion might be
       twice as long as the input curve */
    pts2[i+1] += pts2[i+1]-pts2[i];
    chord2[i+1] = chord2[i]+(pts2[i+1]-pts2[i]);
    a = (double)area_pol(first,first,last);
    a = ABS(a);
    if (*first==*last && *(first+1)==*(last+1)) a = narea;
    if (a>min_area && a<narea) narea = a;
  }
  if (narea<*area)  *area = narea;

  alpha=pow(1.0/(3*gam),1.0/(3*gam-1));

  /* Compute the saturation distance i.e. the maximal chord-arc
     distance on all the C-C */
  a=0.0;
  for(i=0;i<nc;i++) {
    b=saturation(pts2[i],npt[i],*area);
    if (b>a) a=b;
  }


  a=a*1.01/alpha; /* to secure the saturation */
  /*fit the parameters ensures inclusion principle without saturation*/
  if (a>0){
    delta=pow(*area,2.0*gam)*pow(a,1.-3.*gam);
    if (delta>*scale){
      delta=*scale;
      a=pow(pow(*area,2.*gam)/delta,1./(3.*gam-1.));
    } else *scale=delta;
  }

  /*** apply gaceros to convex components and link result ***/
  li->size = 0;
  for (i=0;i<nc;i++) {
    npc = gaceros(pts2[i],chord2[i],npt[i],pts1[0],
		  gam,narea,a,alpha,eps2,&sign);
    if (npc) npc = sample_chord(pts1[0],chord2[i],npc,li->values+li->size*2,chord1[0],eps2);
    if(!conv) {
      if (npc) li->size+= convexify(li->values+li->size*2,chord1[0],
				    chord2[0],npc,sign);
    } else   li->size += npc;
  }
}


/*------------------------------ MAIN MODULE  ------------------------------*/
/* global variables explicitly appear*/

Dlists gcsf(in,out,gam,first,last,eps,area,n,r,v,iter,conv)
     Dlists     in,out;
     double     *first,*last,*eps,*area,*r,*gam;
     char       *v,*conv;
     int        *n,*iter;
{
  int       i,j,ncc,MAX_PTS,MAX_CC,npts,maxsize,collapsed;
  double    a,are,remaining,newlast,rad,eps2,omega,t,step;
  double    x,y,ox,oy,r2,r2max,*per,maxper;
  double    xmin,xmax,ymin,ymax,dx,dy;

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
  if (!r) {
    j = 0;
    while(in->list[j]->size == 0) j++;
    xmin = xmax = in->list[j]->values[0];
    ymin = ymax = in->list[j]->values[1];
    while(j<in->size){
      for(i=0;i<in->list[j]->size;i++){
	x = in->list[j]->values[2*i];
	y = in->list[j]->values[2*i+1];
	if(x<xmin) xmin = x;
	if(x>xmax) xmax = x;
	if(y<ymin) ymin = y;
	if(y>ymax) ymax = y;
      }
      j++;
    }
    dx = xmax - xmin;
    dy = ymax - ymin;
    rad = sqrt(dx * dx + dy * dy);
    mwdebug("rad = %f\n",rad);
  }
  else rad = *r;

  /*** absolute precision (squared) ***/
  eps2 = pow(10.0,-2.0*(*eps))*rad*rad;
  are = pow(10.,-(*area))*rad*rad;

  /* allocate memory */
  /* max number of points of filtered curve = 2* number of points
     before convexification */
  MAX_PTS = 4*(int)(maxper/sqrt(eps2))+2*maxsize+10;
  /* max number of convex components = number of points*/
  MAX_CC = maxsize+10;
  pts1 = (double **)malloc(MAX_CC*sizeof(double *));
  pts2 = (double **)malloc(MAX_CC*sizeof(double *));
  chord1 = (double **)malloc(MAX_CC*sizeof(double *));
  chord2 = (double **)malloc(MAX_CC*sizeof(double *));
  if (!pts1 || !pts2 || !chord1 || !chord2) mwerror(FATAL,1,"Not enough memory\n");
  pts1[0] = (double *)malloc(2*MAX_PTS*sizeof(double));
  pts2[0] = (double *)malloc(2*MAX_PTS*sizeof(double));
  chord1[0] = (double *)malloc(2*MAX_PTS*sizeof(double));
  chord2[0] = (double *)malloc(2*MAX_PTS*sizeof(double));
  if (!pts1[0] || !pts2[0] || !chord1[0] || !chord2[0]) mwerror(FATAL,1,"Not enough memory\n");


  out = mw_change_dlists(out,in->size,in->size);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");

  if (v)
    printf("Common memory allocated : %d ko\n",(MAX_CC*4*4+MAX_PTS*4*8)/1000);
  collapsed = 0;

  /*----- FIRST LOOP (all curves) -----*/

  for (j=0;j<in->size;j++) {

    if (v) printf("\n--- CURVE #%d\n",j);

    npts = 4*(int)(per[j]/sqrt(eps2))+in->list[j]->size+100;

    out->list[j] = mw_change_dlist(out->list[j],npts,0,2);
    if (!out->list[j]->values) mwerror(FATAL,1,"Not enough memory\n");
    mw_copy_dlist(in->list[j],out->list[j]);


    /*** convert scale *final from meters to semigoup unit (modulo cte) ***/
    omega=0.5*pow(3.0/2.0,2.0/3.0);
    remaining = (pow(*last,*gam+1.)-pow(*first,*gam+1.))
      /(*gam+1.)/pow(omega,3.**gam);
    newlast = pow(*last,*gam+1.)/(*gam+1.0)/pow(omega,3.0*(*gam));
    step = remaining/(double)(*n);

    /*----- SECOND LOOP (iterations) -----*/

    for (i=1; out->list[j]->size && remaining>0.0 ;i++) {
      are = pow(10.,-(*area))*rad*rad;
      a = (remaining<step?remaining:step);
      /*** one step ***/
      g_eros(out->list[j],*gam,&a,&are,eps2,&ncc,conv && iter && (i==*iter));
      if (iter && i==*iter) remaining =0;
      else remaining -= a;

      if (v) {
	t = pow((*gam+1.)*pow(omega,3.**gam)*(newlast-remaining),
		1./(*gam+1.));;
	printf("Iter %4d : %4d cc, area = %.4f, current scale = %.4f\033[K\n\033[1A",
	       i,ncc,are,t);
	if (!out->list[j]->size) printf("\nThe curve has collapsed.\n");
      }
    }
    if (v) printf("\n");
    /* sample the output*/
    if (out->list[j]->size) {
      for (i=0;i<out->list[j]->size*2;i++) pts1[0][i]=out->list[j]->values[i];
      out->list[j]->size =
 	(sample(pts1[0],out->list[j]->size,out->list[j]->values,eps2)
	 - out->list[j]->values)/2;
    } else collapsed++;
  }
  free(chord1[0]);
  free(chord2[0]);
  free(chord1);
  free(chord2);
  free(pts2[0]);
  free(pts1[0]);
  free(pts2);
  free(pts1);
  free(per);
  if (collapsed) printf("%d curves over %d collapsed.\n",collapsed,in->size);

  return(out);
}

/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {falign_mdl};
  version = {"1.2"};
  author = {"Lionel Moisan, Andres Almansa"};
  function = {"Detect MDL-maximal meaningful segments"};
  usage = {  
 'd':[d=8]->d        "minimal number of gradient orientations",
 'l':[nd=3]->nd      "number of dyadic gradient precision levels",
 'n':[nl=256]->nl    "number of line directions to scan",
 'g':[g=1.0]->g      "noise level used to threshold gradient",
 'm'->no_mdl         "if set, do NOT use MDL",
 'e':[eps=0.0]->eps  "-log10(max. number of false alarms)",
 'c':crv<-crv        "store segments as curves (output, Flists)",
  u->u               "input Fimage",
  out<-falign_mdl    "result 6xN Fimage (x1 y1 x2 y2 -log10(nfa) precision_level)"
          };
*/

/*----------------------------------------------------------------------
 v1.01: backslash removed (lexical error on SunOS 5.x) (JF)
 v1.02: error in ll_angle() declaration corrected (JF)
 v1.1:  align_mdl becomes falign_mdl (LM)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define VERBOSE 0

#define M_PI 3.1415926535897931

struct one_segment
{
  char side;     /* side where the line starts: 0,1,2 or 3 */
  short pos;     /* line start position along the border */
  short itheta;  /* line angle */
  short start;   /* starting position (distance from border) */
  short length;  /* number of points (-> ending position is start+2*length) */
  short k;       /* number of aligned points */
  double nfa;    /* number of false alarms */
  int ngrad;     /* orientation precision level, i.e. # gradient directions */
  int precision; /* orientation precision level, i.e. ngrad = ngrad_min * 2^precision */
};


/**********************************************************/
/***   compute P(k,l) : array out[] of size n+1 * n+1   ***/
/**********************************************************/

double* tab(n,p,m)
int n;
double p;
double m;
{
  double *out;
  int adr1,adr2,x,y;
  double lambda,q;

  q = 1.0-p;
  out = (double *)calloc((n+1)*(n+1),sizeof(double));

  /*** compute proba (=x among y) ***/
  out[0] = 1.0;
  for (y=1,adr2=0;y<=n;y++) {
    adr1 = adr2;
    adr2 += n+1;    
    out[adr2] = q*out[adr1];
    for (x=1;x<=y;x++) 
      out[adr2+x] = p*out[adr1+x-1] + q*out[adr1+x];
  }  

  /*** sum to obtain proba (>=k among y) ***/
  for (y=1,adr1=n+1;y<=n;y++,adr1+=n+1) 
    for (x=y-1;x>=0;x--) 
      out[adr1+x] += out[adr1+x+1];

  /*** multiply by m (number of segments) to obtain expectation ***/
  for (adr1=(n+1)*(n+1);--adr1>=0;)
    out[adr1] *= m;

  return out;
}


/***************************************************************/
/***   compute the direction of the level line at each point ***/
/***************************************************************/

Fimage ll_angle(a,threshold)
     Fimage a; /* input image of gray levels */
     float threshold;
{
  Fimage g; /* output image of angles */
  int n,p,x,y,adr;
  double com1,com2,gx,gy,norm2;

  threshold *= threshold;
  n = a->nrow;
  p = a->ncol;
  g = mw_change_fimage(NULL,n,p);

  /*** downright boundary (orientation is not defined) ***/
  for (x=0;x<p;x++)
    g->gray[(n-1)*p+x] = -1000.0;
  for (y=0;y<n;y++)
    g->gray[p*y+p-1]   = -1000.0;
  
  /*** remaining part ***/
  for (x=0;x<p-1;x++) 
    for (y=0;y<n-1;y++) {
      adr = y*p+x;

      com1 = (double)(a->gray[adr+p+1])-(double)(a->gray[adr]);
      com2 = (double)(a->gray[adr+1])-(double)(a->gray[adr+p]);
      gx = 0.5*(com1+com2);
      gy = 0.5*(com1-com2);
      norm2 = gx*gx+gy*gy;

      if (norm2 <= threshold) /* orientation is not defined */
	g->gray[adr] = -1000.0;  
      else g->gray[adr] = (float)atan2(gx,-gy); 
    } 
      

  return g;
}


/***************************************************************/
/*                         MAIN MODULE                         */
/***************************************************************/


Fimage falign_mdl(u,d,nd,no_mdl,nl,eps,g,crv)
Fimage u;
int    *d,*nd,*nl,*no_mdl;
double *eps;
float  *g;
Flists crv;
{
  Fimage angle,res;
  struct one_segment *seg;
  int iseg,size_seg;
  int *seg_from_point;
  double **test,nfa,max_nfa;
  int *count,*startbloc,*endbloc;
  int mx,my,ox,oy,nx,ny,n;
  int jseg,xx,yy,pos,posmax,nblocs,inbloc,max_nblocs;
  int cur,i,j,k,side,tmp,l,lphase,length,D;
  int itheta,ntheta;
  float theta,theta0,dtheta,dx,dy,prec,error;
  double g_threshold;
  int precision, ngrad;
  double r,rmin,rmax;

  max_nfa = pow(10.0,-(*eps))/(double)(*nd);

  nx = u->ncol;
  ny = u->nrow;
  /* maximal length for a line */
  n = (int)ceil(hypot((double)nx,(double)ny))+1;

  /******************** memory allocation ********************/

  printf("Allocating memory...\n"); 	fflush(stdout);
  max_nblocs = n/2+1; /* maximal number of blocs */
  count =     (int *)malloc(max_nblocs*sizeof(int));
  startbloc = (int *)malloc(max_nblocs*sizeof(int));
  endbloc =   (int *)malloc(max_nblocs*sizeof(int));
  size_seg = 10000; /* initial allocation (may reallocate later) */
  seg = (struct one_segment *)malloc(size_seg*sizeof(struct one_segment));
  seg_from_point = (int *)malloc(nx*ny*sizeof(int));
  test  = (double **)calloc((*nd),sizeof(double*));
  if (!count || !startbloc|| !endbloc || !seg || !seg_from_point || !test) 
    mwerror(FATAL,1,"Not enough memory.");


  /* initialize segment reference image */
  printf("Initializing...\n"); 	fflush(stdout);
  for (i=nx*ny;i--;) seg_from_point[i]=-1;

  /* counter for segments (seg) */
  iseg = 0;

  /* At precision level r weight nfa by (2^rmin - 1)/(2^(r-rmin-1)) */
  rmin = log((double)(*d))/log(2.0);
  rmax = rmin+(double)(*nd);


  /******************** zeroth loop : precision level ********************/
  for (precision=0; precision<*nd; precision++) {

    ngrad = (*d) * (int) pow(2.0,(double) precision);
    r = rmin+(double)precision;

    printf("*** Precision %d/%d (%d gradient directions)\n",precision+1,*nd,ngrad);
    fflush(stdout);
    /*** Do only consider gradients such that d\theta > noise/|Du| 
	 otherwise, the orientation of Du has a precision below d\theta
    ***/
    g_threshold = *g/(M_PI/(double)ngrad);

    /*** compute angle map of u ***/
    angle = ll_angle(u,g_threshold);

    /*** compute P(k,l) ***/
    test[precision] = tab(n,1.0/(double)(ngrad),(double)(nx*ny)*(double)(nx*ny));

    /*** initialization ***/
    prec = M_PI/(float)(ngrad);
    ntheta = *nl/2;  /* i.e. # directions of NON-ORIENTED lines */
    dtheta = M_PI/(float)ntheta;

    /******************** first loop : the four sides ********************/

    for (side=0;side<4;side++) {
      printf("side %d/4 ",side+1);

      theta0 = 0.5*M_PI*(double)side;
      mx = ((side==0 || side==2)?1:0);
      my = ((side==1 || side==3)?1:0);
      ox = ((side==1)?nx-1:0);
      oy = ((side==2)?ny-1:0);

      posmax = nx*mx+ny*my;
    
    
      /*** second loop : angles ***/
      for (itheta = 0; itheta<ntheta; itheta++) {
	printf(".");
	fflush(stdout);
	theta = theta0 + (float)(itheta)*dtheta;
	dx = (float)cos((double)theta);
	dy = (float)sin((double)theta);
      
	/*** third loop : start positions ***/
	for (pos=0;pos<posmax;pos++) {

	  /*** fourth loop : phase for two-spaced pixels ***/
	  /* for (lphase=0;lphase<2;lphase++) { */
	  /* we skip this loop to avoid conflicts with MDL */
	  /* instead we divide by two:                     */
	  /* the length l and number of aligned points k   */
	  
	  /*** detect aligned points by blocs ***/
	  inbloc = nblocs = cur = l = count[0] = 0;
	  xx = ox+pos*mx + (int)(dx*(float)(l));
	  yy = oy+pos*my + (int)(dy*(float)(l));

	  for (;xx>=0 && xx<nx && yy>=0 && yy<ny;) {

	    error = angle->gray[yy*nx+xx];
	    if (error>-1000.0) { /* orientation defined at this point */
	      error -= theta;
	      while (error<=-M_PI) error += 2.0*M_PI;
	      while (error>M_PI) error -= 2.0*M_PI;
	      if (error<0.0) error = -error;
	      if (error<prec) {
		cur++;
		if (!inbloc) {
		  startbloc[nblocs]=l;
		  inbloc=1;
		}
	      } else {
		if (inbloc) {
		  endbloc[nblocs] = l-1;
		  nblocs++;
		  count[nblocs] = cur;
		}
		inbloc=0;
	      }
	    }
	    /* compute next point */
	    l++;
	    xx = ox+pos*mx + (int)(dx*(float)(l));
	    yy = oy+pos*my + (int)(dy*(float)(l));
	  }

	  /*** detect meaningful segments ***/
	  for (i=0;i<nblocs;i++) 
	    for (j=i;j<nblocs;j++) {
	      k = (count[j+1]-count[i]);
	      length = (1+endbloc[j]-startbloc[i]);
	      nfa = test[precision][(k/2)+(n+1)*(length/2)];
	      if ((length>0) && (nfa < max_nfa)) {

		/* store segment */
		seg[iseg].side = side;
		seg[iseg].pos = pos;
		seg[iseg].itheta = itheta;
		seg[iseg].start = startbloc[i];
		seg[iseg].length = length;
		seg[iseg].k = k;
		seg[iseg].nfa = nfa;
		seg[iseg].ngrad = ngrad;
		seg[iseg].precision = precision;

		/* update owner for each point of a dilated segment */
		for (l=0;l<length;l++) {
		  xx = ox+pos*mx + (int)(dx*(float)(l+seg[iseg].start));
		  yy = oy+pos*my + (int)(dy*(float)(l+seg[iseg].start));
		  error = angle->gray[yy*nx+xx];
		  if (error>-1000.0) { /* orientation defined at this point */
		    error -= theta;
		    while (error<=-M_PI) error += 2.0*M_PI;
		    while (error>M_PI) error -= 2.0*M_PI;
		    if (error<0.0) error = -error;
		    if (error<prec) {
		      /* for each point (xx,yy) in the dilated segment... */
		      for (D=-1;D<=1;D++) {
			xx = ox+pos*mx + (int)(dx*(float)(l+seg[iseg].start))
			               + D*((fabs(dy)>=fabs(dx))?1:0);
			yy = oy+pos*my + (int)(dy*(float)(l+seg[iseg].start))
			               + D*((fabs(dy)< fabs(dx))?1:0);
			if (xx>=0 && xx<nx && yy>=0 && yy<ny) {
			  jseg = seg_from_point[yy*nx+xx];
			  if (jseg==-1) {
			    /* set owner */
			    seg_from_point[yy*nx+xx] = iseg;
			  } else {
			    if ((nfa<seg[jseg].nfa) ||
			        ((nfa==seg[jseg].nfa) && (length>seg[jseg].length))) {
			      /* change owner */
			      seg_from_point[yy*nx+xx] = iseg;
			    } else {
			      /* updatee current seg */
			    }
			  }
			}
		      } /* for D */
		    } /* if error<prec */
		  } /* if error<-100 */
		}
		iseg++;
		/* reallocate if necessary */
		if (iseg==size_seg) {
		  size_seg = (size_seg*3)/2;
		  seg = (struct one_segment *)
		    realloc(seg,size_seg*sizeof(struct one_segment));
		  if (!seg) 
		    mwerror(FATAL,1,"Not enough memory.");
		}
	      }
	    }
	  /*}*/
	  /*** end of phase loop ***/

	}
      }
      /*** end of second loop ***/
    
      printf("\n");
    }
    /******************** end of first loop ********************/
  }
  /******************** end of zeroth loop ********************/
    

  /*** free memory (1/2) ***/
  free(endbloc);
  free(startbloc);
  free(count);

  /* build segments list */
  res = mw_change_fimage(NULL,iseg,6);
  if (!res) mwerror(FATAL,1,"Not enough memory !");
  /* ...and curves if requested */
  if (crv) crv = mw_change_flists(crv,iseg,0);
  j = 0; 
  for (i=0;i<iseg;i++)
    if (seg[i].k>0) {
      /* recover segment parameters */
      side = seg[i].side;
      theta0 = 0.5*M_PI*(double)side;
      mx = ((side==0 || side==2)?1:0);
      my = ((side==1 || side==3)?1:0);
      ox = ((side==1)?nx-1:0);
      oy = ((side==2)?ny-1:0);
      theta = 0.5*M_PI*(double)side + (float)(seg[i].itheta)*dtheta;
      dx = (float)cos((double)theta);
      dy = (float)sin((double)theta);
    
      /* count number k of points owned by this segment */
      k=0;
      for (l=seg[i].start; l<=seg[i].start+seg[i].length; l++) {
	xx = ox+seg[i].pos*mx + (int)(dx*(float)(l));
	yy = oy+seg[i].pos*my + (int)(dy*(float)(l));
	if (seg_from_point[yy*nx+xx]==i)
	  k++;
      }

      /* re-compute nfa
	 considering only aligned points owned by this segment (MDL) */
      nfa = test[seg[i].precision][(k/2)+(n+1)*(seg[i].length/2)];

      /* if still meaningful, add this segment to the output list */
      if (no_mdl || (nfa<max_nfa)) {
	xx = ox+seg[i].pos*mx + (int)(dx*(float)(seg[i].start));
	yy = oy+seg[i].pos*my + (int)(dy*(float)(seg[i].start));

	mw_plot_fimage(res,0,j,(float)xx);
	mw_plot_fimage(res,1,j,(float)yy);
	
	xx = ox+seg[i].pos*mx + (int)(dx*(float)(seg[i].start+seg[i].length));
	yy = oy+seg[i].pos*my + (int)(dy*(float)(seg[i].start+seg[i].length));
	
	mw_plot_fimage(res,2,j,(float)xx);
	mw_plot_fimage(res,3,j,(float)yy);
	
	mw_plot_fimage(res,4,j,-(float)log10(nfa) );
	mw_plot_fimage(res,5,j,(float)seg[i].ngrad );
	if (crv) {
	  crv->list[j] = mw_change_flist(NULL,2,2,2);
	  for (k=0;k<4;k++) 
	    crv->list[j]->values[k] = res->gray[j*6+k]+.5;
	}
	j++; 
      }
      /*}*/
  }
  res->nrow = j;
  if (crv) crv->size = j;

  /*** free memory (2/2) ***/
  free(seg_from_point);
  free(seg);
  for (precision=0; precision<(*nd); precision++) free(test[precision]);
  free(test);

  return res;
}


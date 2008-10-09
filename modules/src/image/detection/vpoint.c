/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {vpoint};
  version = {"1.1"};
  author = {"Andres Almansa"};
  function = {"Detect (MDL) maximal meaningful vanishing regions"};
  usage = {  
 'a'->all           "to get all meaningful v.points (before MDL)",
 'm'->masked        "to get also masked v.points",
 'v'->verbose       "display verbose messages during computations",
 'e':[eps=0.]->eps  "-log10(max. number of false alarms)",
 's':csegs<-segs    "store segments indexes associated to each vanishing region (output Flists)",
  imagein->imagein  "input image (only used to extract image domain)",
  allsegs->allsegs  "input segments (5xn)-Fimage (x1 y1 x2 y2 -log10(nfa))",
  output<-output    "detected vanishing regions: 9-Flist (x1 y1 x2 y2 x3 y3 x4 y4 -log10(nfa))",
  NVP<-vpoint       "number of detected vanishing regions",
  NVP_masked<-maskedVPs  "number of detected masked vanishing regions"
          };
*/
/*----------------------------------------------------------------------
 v1.01: backslash removed (lexical error on SunOS 5.x)  (JF)
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"

/*
   IdentifyOppositeTiles -- Global setting
                            defines how infinite opposite tiles are treated.

   IdentifyOppositeTiles = 1
     With this setting opposite infinite tiles are considered as a single tile.
     This is more symmetric, but we have to use an upper bound for the
     probability of these tiles, not the real probability.
   IdentifyOppositeTiles = 0
     With this setting opposite infinite tiles are considered as different.
     Here we use the real probability for each one, and opposite infinite
     tiles are considered as neighbours when computing local maxima of
     meaningfulness.
     The disadvantage of this setting is that exactly parallel lines
     fall exactly between both opposite infinite tiles, and noise will
     decide which one wins.
*/ 
static int IdentifyOppositeTiles=0;

/*
   SegmentsAsFimage -- Global setting
   if defined, input segments are read from an Nx5 or Nx6 Fimage,
   as in the old version of align.
   otherwise, input segments are read from a 5-Flist or a 6-Flist
   with N records, as produced by the new version of align
 */
#define SegmentsAsFimage

/* ****************************************************************
   Ref-counted Set implementation using MegaWave Flist
   **************************************************************** */
typedef  Flist SegList;
SegList  newSegList(n)
     int n;
{
  SegList l = mw_change_flist(NULL,n,0,1);
  l->data = malloc(sizeof(int)); /* used as ref_count */
  l->data_size = 1;
  (*(int*)l->data) = 1;
  return l;
}
int  deleteSegList(L)
     SegList L;
{
  if (--(*(int*)L->data) == 0) {
    free(L->data);
    L->data = NULL;
    L->data_size = 0;
    mw_delete_flist(L);
  }
  return 0;
}
SegList  newSegListRef(L)
     SegList L;
{
  (*(int*)L->data)++;
  return L;
}
#define  SegListInsert(L,i)   mwerror(FATAL,1,"SegListInsert not implemented in non-HasSTL mode.\n Use FlistAdd instead.\n")
#define  SegListSize(L)       (L->size)
typedef  float* SegListIterator;
#define  SegListBegin(L)      (L->values)
#define  SegListEnd(L,it)     (it>=(L->values+(L->size)))
#define  SegListNext(it)      (it++)
#define  SegListValue(it)     ((int)*it)
Flist    SegListCopy(L)
     SegList L;
{
  SegListIterator p;
  Flist l;

  l = mw_change_flist(NULL,SegListSize(L),0,1);
  
  for(p=SegListBegin(L); !SegListEnd(L,p); SegListNext(p))
    l->values[l->size++] = (float) SegListValue(p);

  l->data = malloc(sizeof(int)); /* used as ref_count */
  l->data_size = 1;
  (*(int*)l->data) = 1;

  return l;
}

/* ****************************************************************
   End of: Ref-counted Set implementation using MegaWave Flist
   **************************************************************** */



#define EPS 1.0e-16 /* Machine double precision */
#define Inf (-log(0.0))

#define max(x,y) (x>y)?x:y
#define _(I,x,y) ((I)->gray[(y)*((I)->ncol)+(x)]) /* 0 <= x < I->ncol
						     0 <= y < I->nrow */



/*---------------------------------------------------------------------------
   Compute a partition of the plane in regions of equal probability p,
   that a random line visible in the image meets the region.
  ---------------------------------------------------------------------------
   For the iterior of the image domain we choose square regions of side
      N*2*sin(theta)
   where
      theta is the minimal angular precision of the line segments, and
      N is the size of the image.
   Thus the probability of an image line meeting such regions is:
      pint = 8*N*sin(theta)
   For the exterior of the image we choose portions of sectors of a circle
   centered at the image center. These sectors have an angle 2*theta, and
   the portions of sectors are limited between two cords at distance
   d1 and d2 from the image center. Instead of di it will be more convenient
   to work with
      betai = acos(cos(theta)*R/di),
   where R=sqrt(2)*N is the radius of the circle circumscribed to the image.
   For such an exterior region the probability of an image line meeting the
   region is
      pext(beta1,beta2,theta)
   as computed by the corresponding function below. We also provide a
   function to compute the derivative of pext with respect to beta2:
      pext_prime(beta1,theta)
   When d1 remains fixed and d2 tends to infinity (beta2 tends to pi/2)
   the probability of an image line meeting the region becomes
      pinf(beta1,theta)
   as computed by the corresponding function below.
   fzero_convex finds a zero within the given interval of the function
      f(x) = pext(beta1,x,theta)-pint
   which is convex, with known derivative, so we can use a modified Newton
   method, which ensures a given precision both on x and f(x).
   Finally qtile uses fzero_convex to find the good values of beta which are
   needed to obtain a tiling of the plane into regions with equal
   probability. It proceeds as follows: first it fixes
      d_1 = R*cos(theta) or equivalently beta_1=0
   then for all i>1 it uses fzero_convex to find beta_i or d_i such that
      pext(beta_i-1,beta_i,theta) = pint
   The iteration stops when
      pinf(beta_i,theta) <= pint
   so the last (infinite) region has a probability between 0 and pint.
  ---------------------------------------------------------------------------*/

double pext(beta1,beta2,theta)
     double beta1,beta2,theta;
{
  double f1,f2;
  if (cos(beta1)<EPS*1.0e5)
    f1 = M_PI_2;
  else
    f1 = beta1+1.0/cos(beta1)-tan(beta1);

  if (cos(beta2)<EPS*1.0e5)
    f2 = M_PI_2;
  else
    f2 = beta2+1.0/cos(beta2)-tan(beta2);

  return ( 2.0*theta + f2-f1 ) / (double) M_PI;
}

double pext_prime(beta2,theta)
     double beta2,theta;
{
  double fp2;
  if (cos(beta2)<EPS*1.0e5)
    fp2 = -0.5;
  else
    fp2 = (sin(beta2)-1.0)/cos(beta2)/cos(beta2);

  return ( 1.0 + fp2 ) / (double) M_PI;
}

double pinf(beta1,theta)
     double beta1,theta;
{
  return pext(beta1,M_PI_2,theta);
}

double fzero_convex(a0,b0,tolx,tolf,beta1,dtheta,p)
     double a0,b0,tolx,tolf,beta1,dtheta,p;
     /* Finds the zero x0 of a convex function myfun(x) on an interval [a0,b0].
        We assume that:
	- its derivative myfunprime(x) is also known and
        - since myfun is convex,
          myfunprime is nonnegative and increasing on [a,b]
	The root x0 is given with precision tolx on the x axis
        and tolf on the y axis, i.e.  abs(myfun(x0))<tolf.

     */
{
  double a,b,fa,fb,fpa,fpb;
  a = a0; b=b0;
  fa = pext(beta1,a,dtheta)-p; /* == myfun(a); */
  fb = pext(beta1,b,dtheta)-p; /* == myfun(b); */

  mwdebug("Iterating fzero_convex...\n");
  while (fabs(fb-fa)>tolf && fabs(b-a)>tolx) {
    fpb = pext_prime(b,dtheta); /* == myfunprime(b); */
    a = a-fa/fpb; fa =  pext(beta1,a,dtheta)-p; /* == myfun(a); */
    b = b-fb/fpb; fb =  pext(beta1,b,dtheta)-p; /* == myfun(b); */
    mwdebug(".");
 }

  return (a+b)/2.0;

}

double* qtile(p,dtheta,nq,p_inf)
     double p;
     double dtheta;
     int* nq;
     double* p_inf;
{
  double tol,b;
  double *beta;
  int i;
  tol = 1.0e-8;
  
  *nq  = 1;
  beta = malloc(sizeof(double));
  beta[0] = b = 0.0;

  while ((*p_inf=pinf(b,dtheta))>p) {
    b = fzero_convex(b,M_PI_2,tol,tol,b,dtheta,p);
    beta = realloc(beta,((*nq)+1)*sizeof(double));
    beta[(*nq)++] = b;
  }

  for (i=1; i<*nq; i++) {
    mwdebug("beta=%8.3f, q=%8.3f (p-pint)/pint=%f\n",beta[i],1.0/cos(beta[i]),
	    (pext(beta[i-1],beta[i],dtheta)-p)/p);
  }
    mwdebug("beta=%8.3f, q=%8.3f (p-pint)/pint=%f\n",M_PI_2,HUGE_VAL,
	    (pinf(beta[*nq-1],dtheta)-p)/p);
    mwdebug("\n");

  /* convert beta to normalized distances q */
  for (i=0; i<*nq; i++) {
    beta[i] = 1.0/cos(beta[i]);
  }

  return beta;
}

void polar2cart(theta,q,R,Xcenter,Ycenter,x,y)
     double theta,q;           /* normalized polar coordinates */
     double R,Xcenter,Ycenter; /* parameters used for normalization */
     float  *x,*y;             /* non-normalized cartesian coordinates */
{
  *x = R*q*cos(theta)+Xcenter;
  *y = R*q*sin(theta)+Ycenter;
}


/*------------------------------------------------------------*/
/*       compute P(k,l) : array out[] of size n+1 * n+1       */
/*   P[k][l] =     sum(i=k..l) binom(l,i) p^i (1-p)^(l-i)     */
/*   P[k][l] = P[ (n+1)*l+k ]                                 */
/*------------------------------------------------------------*/


double *tab(n,p)
int n;
double p;
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

  /*** multiply by m (number of segments) to obtain expectation***
  for (adr1=(n+1)*(n+1);--adr1>=0;)
    out[adr1] *= m;
  */

  return out;
}

double *tab2(n,p)
int n;
double p;
/* alternative implementation: it computes directly the binomial tail,
   without computing the binomial density first */
{
  double *out;
  int adr1,adr2,x,y;
  double lambda,q;

  q = 1.0-p;
  out = (double *)calloc((n+1)*(n+1),sizeof(double));

  /*** compute proba (>=x among y) ***/
  out[0] = 1.0;                                      /* P(0,0) = 1 */
  for (y=1,adr2=0;y<=n;y++) {
    adr1 = adr2; /* column y-1 */
    adr2 += n+1; /* column y   */ 
    out[adr2]   = 1.0;                               /* P(0,y) = 1.0 */
    out[adr2+y] = p*out[adr1+(y-1)];                 /* P(y,y) =
							   p * P(y-1,y-1)
						      */
    for (x=1;x<y;x++) 
      out[adr2+x] = p*out[adr1+x-1] + q*out[adr1+x]; /* P(x,y) =
                                                           p * P(x-1,y-1)
							 + q * P(x,  y-1)
						      */
  }  

  return out;
}

double* binomial_tail(n,p)
int n;
double p;
{
  double *in,*out;
  int x;

  in = tab2(n,p);
  out = calloc(n+1,sizeof(double));

  for (x=0;x<=n;x++)
    out[x] = in[(n+1)*n+x];

  free(in);

  return out;
}

/* -------------------------------------------------------------------------
   Structure to represent a tiling of the image plane.
   Actually a tiling will be
      Tiling T[2]
   where T[INTERIOR_TILING] is a tiling of the image domain
   and   T[EXTERIOR_TILING] is a tiling of its complement
   ------------------------------------------------------------------------- */
#define INTERIOR_TILING 0
#define EXTERIOR_TILING 1
typedef struct tiling {
  int nx,ny;       /* number of tiles = nx*ny */
  double *x;       /* nx+1 array with x-coords of the borders of the tiling */
  double *y;       /* ny+1 array with y-coords the borders of the tiling */
                   /* For exterior tilings (x,y)=(theta,q)
		      in normalized polar coordinates */
  double *meaning; /* nx*ny array with the meaningfulness (-log10(nfa))
                      for each region */
  int *vp;         /* nx*ny array equals 1 if the region is meaningful
                      with locally maximal meaningfulness */
  int *isvalid;    /* nx*ny array equals 1 if the corresponding region
                      is valid. (Used to eliminate interior tiles which are
                      outside of the circular "image domain", 
		      as well as infinite exterior tiles
		      which are split into two.
		      It is also used to remove tiles
		      that have already been found to be MDL-maximal-meaningful). */
  SegList *segs;   /* nx*ny array of Flists. The Flist corresponding to
		      region (i,j) contains pointers to the input segs Flist
		      for all segments whose supporting line meets region (i,j)
		    */
  double *mdlmeaning;
  int *mdlvp;
  SegList *mdlsegs; /* same as meaning, vp and segs, but after MDL */
} Tiling;

/* Convenience macros to access a "pixel" of the tiling */
#define TMeaning(T,ie,x,y)    (T[ie].meaning[T[ie].nx*(y)+(x)])
#define TMDLMeaning(T,ie,x,y) (T[ie].mdlmeaning[T[ie].nx*(y)+(x)])
#define TVP(T,ie,x,y)         (T[ie].vp[T[ie].nx*(y)+(x)])
#define TMDLVP(T,ie,x,y)      (T[ie].mdlvp[T[ie].nx*(y)+(x)])
#define TIsValid(T,ie,x,y)    (T[ie].isvalid[T[ie].nx*(y)+(x)])
#define TIsValid2(T,ie,x,y)   ( (0<=(x)) && ((x)<T[ie].nx) && \
			        (0<=(y)) && ((y)<T[ie].ny) && \
			        T[ie].isvalid[T[ie].nx*(y)+(x)] )
#define TSegs(T,ie,x,y)       (T[ie].segs[T[ie].nx*(y)+(x)])
#define TMDLSegs(T,ie,x,y)    (T[ie].mdlsegs[T[ie].nx*(y)+(x)])


/* Identification of pairs of opposite tiles in the second
   semi-circle of the last ring */
#define TOpposite(T,x,y) (((x) >= T[EXTERIOR_TILING].nx/2) ? ((x)-T[EXTERIOR_TILING].nx/2) : (x)+T[EXTERIOR_TILING].nx/2)

/* Alternative to TMeaning that deals with out of bounds indices.
   Useful to compare a tile to its neighbours
 */
double TMeaning2(T,ie,ix,iy)
     Tiling* T;
     int ie,ix,iy;
{
  int nx, ny, x, y;
  nx = T[ie].nx;
  ny = T[ie].ny;

  if (ie==INTERIOR_TILING)
    if ((ix<0) || (iy<0) || (ix>=nx) || (iy>=ny))
      return -Inf;
    else
      return TMeaning(T,ie,ix,iy);
  else {    /* extend x (theta) circularly */
    x = ix;
    while (x<0) x += nx;
    while (x>=nx) x-= nx;
    y = iy;
    if ((y<0) || (y>=2*ny-IdentifyOppositeTiles))
      return -Inf;
    else if (y>=ny)
      return TMeaning(T,ie,TOpposite(T,x,2*ny-1-y-IdentifyOppositeTiles),
		           2*ny-1-y-IdentifyOppositeTiles);
    else
      return TMeaning(T,ie,x,y);
  }

}



/* Create and initialize a new Tiling of the plane into vanishing regions
   for a given angular precision level (ntheta orientations) */
Tiling* newTiling(ntheta,p,p_inf,M)
   int ntheta;   /* Number of orientations in the exterior tiling */
   double *p;    /* (output) probability of a line meeting any of the tiles */
   double *p_inf;/* (output) probability of a line meeting infinite tiles
		             (p_inf <= p) */
   int *M;       /* (input) total number of segments, and */
                 /* (output) total number of valid tiles */
{
  double dtheta, pint, *theta, *q, dxy, *xx, *yy;
  int t,nq,nx,ny;
  int dx[4],dy[4],isvalid,max_size;
  int ie,ix,iy,i;
  Tiling* T;

  /* Used to iterate on 4-neighbours */
  /* dx={0,1,0,1}; */ dx[0]=0;dx[1]=1;dx[2]=0;dx[3]=1;
  /* dy={0,0,1,1}; */ dy[0]=0;dy[1]=0;dy[2]=1;dy[3]=1;

  /* Create the tiling for this precision level */
  T =  (Tiling*) calloc(2,sizeof(Tiling));

  /* Angular precision for all regions at this precision level */
  dtheta  = M_PI/(double)ntheta;
  /* Probability of internal tiles */
  pint = 4.0*sin(dtheta)/M_PI;

  /* Compute boundaries of external tiles in
     normalized polar coordinates (theta,q) */
  theta = malloc((ntheta+1)*sizeof(double));
  for(t=0;t<=ntheta;t++) theta[t] = 2.0*dtheta*(double)t;
  q = qtile(pint,dtheta,&nq,p_inf);


  /* Compute boundaries of internal tiles in
     normalized pixel coordinates x,y. */
  dxy = 2*sin(dtheta);
  nx = ny = (int) ceil(2.0/dxy);
  xx = malloc((nx+1)*sizeof(double));
  for (t=0;t<=nx;t++)
    xx[t] = -1.0+dxy*t;
  yy = malloc((ny+1)*sizeof(double));
  for (t=0;t<=ny;t++)
    yy[t] = -1.0+dxy*t;


  /* Create data structure for exterior tiles */
  T[EXTERIOR_TILING].nx = ntheta;
  T[EXTERIOR_TILING].ny = nq;
  T[EXTERIOR_TILING].x = theta;
  T[EXTERIOR_TILING].y = q;
  T[EXTERIOR_TILING].meaning = (double*) calloc(ntheta*nq,sizeof(double));
  T[EXTERIOR_TILING].vp      = (int*)    calloc(ntheta*nq,sizeof(int));
  T[EXTERIOR_TILING].isvalid = (int*)    calloc(ntheta*nq,sizeof(int));
  T[EXTERIOR_TILING].segs    = (SegList*)  calloc(ntheta*nq,sizeof(SegList));
  T[EXTERIOR_TILING].mdlmeaning = (double*) calloc(ntheta*nq,sizeof(double));
  T[EXTERIOR_TILING].mdlvp      = (int*)    calloc(ntheta*nq,sizeof(int));
  T[EXTERIOR_TILING].mdlsegs    = (SegList*)  calloc(ntheta*nq,sizeof(SegList));

  /*  Create data structure for interior tiles */
  T[INTERIOR_TILING].nx = nx;
  T[INTERIOR_TILING].ny = ny;
  T[INTERIOR_TILING].x = xx;
  T[INTERIOR_TILING].y = yy;
  T[INTERIOR_TILING].meaning = (double*) calloc(nx*ny,sizeof(double));
  T[INTERIOR_TILING].vp      = (int*)    calloc(nx*ny,sizeof(int));
  T[INTERIOR_TILING].isvalid = (int*)    calloc(nx*ny,sizeof(int));
  T[INTERIOR_TILING].segs    = (SegList*)  calloc(nx*ny,sizeof(SegList));
  T[INTERIOR_TILING].mdlmeaning = (double*) calloc(nx*ny,sizeof(double));
  T[INTERIOR_TILING].mdlvp      = (int*)    calloc(nx*ny,sizeof(int));
  T[INTERIOR_TILING].mdlsegs    = (SegList*)  calloc(nx*ny,sizeof(SegList));


  /* Initial size for segs Flists = expected number of segments at each tile */
  max_size = (int) floor(max(((double) *M) 
			     / sqrt((double) (nx * ny + ntheta * nq)), 1.0)
			 + .5);

  /* Initialize all tiles */
  *M=0;
  for (ie=0;ie<=1;ie++)
    for (iy=0;iy<T[ie].ny;iy++)
      for (ix=0;ix<T[ie].nx;ix++) {
	TMeaning(T,ie,ix,iy) = TMDLMeaning(T,ie,ix,iy) = -Inf;
	TVP(T,ie,ix,iy)      = TMDLVP(T,ie,ix,iy)      = 0;
	if (ie==INTERIOR_TILING) {
	  /* test if one of the corners is inside unit circle
	     (i.e. inside the image domain) */
	  isvalid=0;
	  for(i=0;i<4;i++)
	  { 
	    double x, y;
	    x = T[ie].x[ix + dx[i]];
	    y = T[ie].y[iy + dy[i]];
	    if (sqrt(x * x + y * y) <= 1.0) {
	      isvalid = 1;
	      break;
	    }
	  }
	  TIsValid(T,ie,ix,iy) = isvalid;
	} else { /* ie == EXTERIOR_TILING */
	  if ((IdentifyOppositeTiles==1) && (iy==(T[ie].ny-1)))
	    /* Tile is invalid if it belongs to the second semi-circle
	       of the last (infinite) ring */
	    isvalid = (T[ie].x[ix]<M_PI)?1:0 ;
	  else
	    isvalid = 1;
	  TIsValid(T,ie,ix,iy) = isvalid;
	}
	if (isvalid) {
	  (*M)++;
	  TSegs(T,ie,ix,iy)    = newSegList(max_size);
	  TMDLSegs(T,ie,ix,iy) = newSegList(max_size);
	} else if ((ie == EXTERIOR_TILING) && (IdentifyOppositeTiles == 1)) {
	  /* In the last (infinite) ring of tiles we identify tiles
	     in the second semi-circle with their opposite tile
	     in the first semi-circle */
	  TSegs(T,ie,ix,iy)    = newSegListRef(TSegs(T,ie,TOpposite(T,ix,iy),iy));
	  TMDLSegs(T,ie,ix,iy) = newSegListRef(TMDLSegs(T,ie,TOpposite(T,ix,iy),iy));
	} else
	  TSegs(T,ie,ix,iy) = TMDLSegs(T,ie,ix,iy) = NULL;
      }

  *p = pint;
  return T;
}

void deleteTiling(T)
     Tiling* T;
{
  int ie, ix, iy;
  SegList S;

  for (ie=0;ie<=1;ie++) {
    /**/
    for (iy=0;iy<T[ie].ny;iy++)
      for (ix=0;ix<T[ie].nx;ix++) {
	S = (TSegs(T,ie,ix,iy));    if (S!=NULL) {deleteSegList(S); TSegs(T,ie,ix,iy)=NULL;};
	S = (TMDLSegs(T,ie,ix,iy)); if (S!=NULL) {deleteSegList(S); TMDLSegs(T,ie,ix,iy)=NULL;};
      }
    /**/
    free(T[ie].x);
    free(T[ie].y);
    free(T[ie].isvalid);
    free(T[ie].meaning);
    free(T[ie].vp);
    free(T[ie].segs);
    free(T[ie].mdlmeaning);
    free(T[ie].mdlvp);
    free(T[ie].mdlsegs);
  }
  free(T);

}

/* -------------------------------------------------------------------------
   Data structure to trace back the tiles met by a given segment.
   Segments is an Flists.
   Each element s of Segments is a 4-Flist which contains:
   s->data = { isvalid, x0, y0, x1, y1, nfa [, precision] }
             (float array with 6-7 elements identifying the segment)
   s->values = a list of tiles met by the segment s.
               Each 4-tuple (i,ie,ix,iy) in the list represents
	       one of these tiles and can be used e.g. as
	       TMeaning(Tilings[i],ie,iy,ix) to find its meaningfulness.
   ------------------------------------------------------------------------- */
#ifdef SegmentsAsFimage
Flists newSegments(allsegs)
     Fimage allsegs;
{
  int N, dim, i, j;
  Flists Segments;
  Flist s;

  N = allsegs->nrow;
  dim = allsegs->ncol;
  if ((dim!=6) && (dim!=5))
    mwerror(ERROR,1,"Segments Fimage should be 5xN or 6xN");

  /* convert list of segments to Flists */
  Segments = mw_change_flists(NULL,N,N);
  for(i=0;i<N;i++) {
    s = Segments->list[i] = mw_change_flist(NULL,0,0,4);
    s->data = calloc(dim+1,sizeof(float));
    s->data_size = (dim+1)*sizeof(float);
    ((float*) (s->data))[0] = 1.0; /* isvalid field */
    for(j=0;j<dim;j++)
      ((float*) (s->data))[j+1]=allsegs->gray[i*dim+j];
  }

  return Segments;
}

void deleteSegments(Segments)
     Flists Segments;
{
  int i;
  Flist s;

  for(i=0;i<Segments->size;i++) {
    s = Segments->list[i];
    free(s->data);
    s->data = NULL;
    s->data_size = 0;
  }

  mw_delete_flists(Segments);
  
}
#else
#endif

/* Convenience macros to access the attributes of a segment */
#define SegIsValid 0
#define SegX0 1
#define SegY0 2
#define SegX1 3
#define SegY1 4
#define SegMeaning 5
#define SegPrecision 6
#define STiles(S,j)     ((S)->list[j]) /* 4-Flist of tiles
					  met by j-th segment */
#define SSeg(S,j)       ((float*) ((S)->list[j]->data))
#define SIsValid(S,j)   (SSeg(S,j)[SegIsValid])
#define SX0(S,j)        (SSeg(S,j)[SegX0])
#define SY0(S,j)        (SSeg(S,j)[SegY0])
#define SX1(S,j)        (SSeg(S,j)[SegX1])
#define SY1(S,j)        (SSeg(S,j)[SegY1])
#define SMeaning(S,j)   (SSeg(S,j)[SegMeaning])
#define SPrecision(S,j) (((S)->list[j]->data_size >= 6*sizeof(float))? \
                          SSeg(S,j)[SegPrecision] : \
                          NAN)


/* -------------------------------------------------------------------------
   Auxiliary functions for TAddSegment
   ------------------------------------------------------------------------- */

/* Cross-product in R3 or join or meet in P2 */
void cross_prod(a,b,c)
     double *a,*b,*c;
{
  c[2] =  a[0]*b[1]-a[1]*b[0];
  c[0] =  a[1]*b[2]-a[2]*b[1];
  c[1] =  a[2]*b[0]-a[0]*b[2];
}


/* Normalized polar coordinates of the line supporting a segment */
void polar_coords(seg,R,Xcenter,Ycenter,rho,phi)
     float* seg;
     double R,Xcenter,Ycenter;
     double *rho,*phi;
{
  double x0,y0, x1,y1, dx,dy, px,py;
  double X0[3],X1[3],OX[3],P[3],C[3],L1[3],L2[3];

  x0 = seg[SegX0]+0.5; y0 = seg[SegY0]+0.5; /* first end-point */
  x1 = seg[SegX1]+0.5; y1 = seg[SegY1]+0.5; /* second end-point */
  dx = x1-x0;          dy = y1-y0;

  /*
     Projective coordinates of:
     X0 = first endpoint of segment
     X1 = second endpoint of segment
     OX = direction orthogonal to segment
     C  = origin of coordinate system
  */
  X0[0] = x0;          X0[1] = y0;          X0[2] = 1.0;
  X1[0] = x1;          X1[1] = y1;          X1[2] = 1.0;
  OX[0] = -1.0*dy;     OX[1] = dx;          OX[2] = 0.0;
  C[0]  = Xcenter;     C[1]  = Ycenter;     C[2]  = 1.0;

  /* Find the foot P of the perpendicular L2
     from C to supporting line of segment
  */
  cross_prod(X0,X1,L1); /* line L1 through X0,X1 */
  cross_prod(C, OX,L2); /* line L2 from C orthogonal to L1 */
  cross_prod(L1,L2,P ); /* intersection P of L1 and L2 */

  /* Now compute polar coordinates of line:
     rho = (non-signed) distance from C to line, divided by R
     phi = oriented angle in [0,2pi] from e1 to P-C
  */
  px = P[0]/P[2]-Xcenter;  py = P[1]/P[2]-Ycenter;
  *rho = sqrt(px * px + py * py) / R;
  *phi = atan2(py,px);

  if ((*phi)<0.0)
    *phi = *phi + 2.0*M_PI;

}

/* Normalized projective coordinates of the line supporting a segment */
void proj_coords(seg,R,Xcenter,Ycenter,L)
     float* seg;
     double R,Xcenter,Ycenter;
     double *L; /* should be called with an allocated L[3] */
{
  double x0,y0, x1,y1;
  double X0[3],X1[3];

  x0 = seg[SegX0]+0.5; y0 = seg[SegY0]+0.5; /* first end-point */
  x1 = seg[SegX1]+0.5; y1 = seg[SegY1]+0.5; /* second end-point */

  /*
     Projective coordinates (in normalized coord system):
     X0 = first endpoint of segment
     X1 = second endpoint of segment
  */
  X0[0] = (x0-Xcenter)/R;  X0[1] = (y0-Ycenter)/R;  X0[2] = 1.0;
  X1[0] = (x1-Xcenter)/R;  X1[1] = (y1-Ycenter)/R;  X1[2] = 1.0;

  /* line L through X0,X1 */
  cross_prod(X0,X1,L); 

}

/*
  Append a record to the end of the Flist.
  (This is used to add a segment j to an accumulator TSegs(T,ie,ix,iy))
 */
void FlistAdd(l,tuple)
     Flist l;
     float *tuple;
{
  int i,sz,dim;
  sz = l->size;
  dim = l->dim;

  if ((l->size == l->max_size) && (!mw_enlarge_flist(l)))
    mwerror(FATAL,1,"Not enough memory to enlarge flist");
  for(i=0;i<dim;i++)
    l->values[sz*dim+i] = tuple[i];

  (l->size)++;
}

Flist FlistCat(l1,l2)
     Flist l1,l2;
{
  int i, j; 

  if (l1->dim != l2->dim)
    mwerror(INTERNAL,1,"[FlistCat] Flists should have the same dimension!");

  for (i=0; i<l2->size; i++)
    if ((l1->size == l1->max_size) && (!mw_enlarge_flist(l1)))
      mwerror(ERROR,1,"Not enough memory to enlarge Flist!");
    else {
      for (j=0; j<l1->dim; j++)
	l1->values[l1->size*l1->dim + j] = l2->values[i*l2->dim + j];
      l1->size++;
    }

  return l1;
}

Flist FlistFlush(l1)
     Flist l1;
{
  l1->size=0;
  return l1;
}

/*
  Append tile (i,ie,ix,iy) to the end of L, only if:
  a) this tile is valid and
  b) it is not yet in L, and
  c) no tile equivalent to (i,ie,ix,iy) has been added to L.
  These three tests are done via l=TSegs(T,ie,ix,iy), with T = Tilings[i],
  as follows (if HasSTL is undefined)
  a)   l is non-null
  b,c) there is no l2 = TSegs(Tilings[i2],ie2,ix2,iy2) such that l2==l and
       the tuple (i2,ie2,ix2,iy2) is in L.
  (This is used to check that a segment is not counted twice.
   The search is not too expensive because we build one of these lists
   per segment, precision level. The size of ls is at most 2*nx+4*ny).
   Observe that this implementation ignores the last argument j.
*/
void STilesAddUnique(L,Tilings,i,ie,ix,iy,j)
     Flist L;   /* 4-Flist containing the tiles met by the j-th segment */
     Tiling **Tilings;
     int i,ie,ix,iy; /* 4-tuple representing the tile to be added */
     int j;          /* current segment */
{
  int k,ismember;
  Flist l2;
  SegList l;
  int i2,ie2,ix2,iy2;
  Tiling *T;

  T = Tilings[i];

  /* We test equality via l = TSegs(T,ie,ix,iy)
     since in case IdentifyOppositeTiles==1
     two different 4-tuples (i,ie,ix,iy) might refer to the same tile
  */
  l = TSegs(T,ie,ix,iy);
  if (l!=NULL) {
#ifdef HasSTL
    ismember = SegListInsert(l,j);
#else
    for (ismember=0, k=0; (!ismember) && (k<L->size); k++) {
      i2  = (int) (L->values[4*k+0]);
      ie2 = (int) (L->values[4*k+1]);
      ix2 = (int) (L->values[4*k+2]);
      iy2 = (int) (L->values[4*k+3]);
      l2 = TSegs(Tilings[i2],ie2,ix2,iy2);
      if (l2==l) {
	ismember=1;
	break;
      }
    }
#endif
    if (!ismember)
      if ((L->size == L->max_size) && (!mw_enlarge_flist(L)))
	mwerror(FATAL,1,"Not enough memory to enlarge Flist");
      else {
	L->values[4*L->size + 0] = (float) i;
	L->values[4*L->size + 1] = (float) ie;
	L->values[4*L->size + 2] = (float) ix;
	L->values[4*L->size + 3] = (float) iy;
	L->size++;
      }
  }
}

/* -------------------------------------------------------------------------
   TAddSegment - Updates Tiling with all intersections of a segment
                 with all tiles
   If the j-th segment in segs doesn't match the current precision level
   do nothing and return 0.
   Otherwise return 1 after doing the following... 
   For all tiles t in T such that the jth segment meets t, update T,
   i.e.:
   - add j to the list of segments meeting t (avoiding duplicates!)
   
   We should also (but this can be done at the end):
   - update meaning to -log10(nfa(n,N))
     (where n is the updated length of the segs list meeting t and
      N is the total number of segments)
   - update vp to meaning>eps
  -------------------------------------------------------------------------
*/
int TAddSegment(Tilings,i,S,j,R,Xcenter,Ycenter)
     Tiling **Tilings;
     int i; /* index to precision level */
     Flists S;
     int j; /* index to current segment */
     double R,Xcenter,Ycenter;
{
  double rho,phi,cos_phi_theta;
  int    nd,ntheta,nx,ny,ie;
  double *d,*theta,dtheta,theta0,*x,*y,dx,dy,xmin,ymin,xmax,ymax;
  int    id,itheta,id1,itheta1,itheta2,ix,iy;
  double            d1, theta1, theta2,xx,yy;
  int k;
  double l[3];
  Flist  L,csegs;
  float tuple;
  int max_tiles;
  Tiling *T;
  double length;

  T = Tilings[i];

  /* compute angular precision of this segment ... */
  dx = SX0(S, j) - SX1(S, j);
  dy = SY0(S, j) - SY1(S, j);
  length = sqrt(dx * dx + dy * dy);
  ie     = EXTERIOR_TILING;
  dtheta = T[ie].x[1]-T[ie].x[0];
  /* ... if it is too coarse do not add segment */
  if (atan2(1.0,length)>dtheta)
    return 0;

  ntheta = T[EXTERIOR_TILING].nx;
  nd     = T[EXTERIOR_TILING].ny;
  nx     = T[INTERIOR_TILING].nx;
  ny     = T[INTERIOR_TILING].ny;
  max_tiles = 4*nd+2*ntheta + 2*max(nx,ny);

  /*
    we do not start with L = STiles(S,j),
    because it only contains tiles from previous precision levels <i,
    whereas in this call we add tiles at precision level i.
    Thus starting with L = STiles(S,j), would unnecessarily slow down
    STilesAddUnique.
    Instead, we append the list L of tiles at precision i to STiles(S,j)
    at the end of this routine.
  */
  L = mw_change_flist(NULL,max_tiles,0,4);
  
  /*** A) Compute all intersections of j-th segment with exterior tiles ***/

  /* Normalized polar coordinates of the supporting line of j-th segment*/

     polar_coords(SSeg(S,j)                 ,R,Xcenter,Ycenter,&rho,&phi);

  ie     = EXTERIOR_TILING;
  nd     = T[ie].ny;
  d      = T[ie].y;
  ntheta = T[ie].nx;
  theta  = T[ie].x;
  dtheta = T[ie].x[1]-T[ie].x[0];
  theta0 = T[ie].x[0];

  /* Find intersection points of line (rho,phi) with each circle */
  for (id=0;id<nd;id++) {
    theta1 = phi-acos(rho/d[id]);
    theta1 = theta1+((theta1<      0.0)? 2*M_PI : 0.0);
    theta2 = acos(rho/d[id])+phi;
    theta2 = theta2-((theta2>=2.0*M_PI)? 2*M_PI : 0.0);
    itheta1 = (int) floor((theta1-theta0)/dtheta);
    itheta2 = (int) floor((theta2-theta0)/dtheta);
    /*  Add tiles touching both intersection points
	(theta1,d[id]) and (theta2,d[id])
	to the list L, avoiding duplicates */
    STilesAddUnique(L,Tilings,i,ie,itheta1,id  ,j);
    STilesAddUnique(L,Tilings,i,ie,itheta2,id  ,j);
    if (id>0) {
      STilesAddUnique(L,Tilings,i,ie,itheta1,id-1,j);
      STilesAddUnique(L,Tilings,i,ie,itheta2,id-1,j);
    }
  };
  /* Find intersection point of line (rho,phi) with each radius */
  for (itheta=0;itheta<ntheta;itheta++) {
    cos_phi_theta = cos(phi-theta[itheta]); /* when cos==0
					       we force infinite tile */
    d1 = (fabs(cos_phi_theta)<EPS)?Inf:rho/cos(phi-theta[itheta]);
    if (d1>=d[0]) {
      id1=0; while((id1<nd) && (d[id1]<d1)) id1++;
      if (id1>0) id1--;
      /*  Add tiles touching the intersection point (theta[itheta],d1)
	  to the list L, avoiding duplicates */
      STilesAddUnique(L,  Tilings,i,ie,itheta,  id1,j);
      if (itheta>0)
	STilesAddUnique(L,Tilings,i,ie,itheta-1,id1,j);
      else /* itheta==0, its circular neighbour is ntheta-1 */
	STilesAddUnique(L,Tilings,i,ie,ntheta-1,id1,j);
    }
  }

  /*** B) Compute all intersections of j-th segment with interior tiles ***/

     proj_coords(SSeg(S,j)                 ,R,Xcenter,Ycenter,l);

  ie = INTERIOR_TILING;
  nx   = T[ie].nx;
  x    = T[ie].x;
  dx   = T[ie].x[1]-T[ie].x[0];
  xmin = T[ie].x[0];
  xmax = T[ie].x[nx];
  ny   = T[ie].ny;
  y    = T[ie].y;
  dy   = T[ie].y[1]-T[ie].y[0];
  ymin = T[ie].y[0];
  ymax = T[ie].y[ny];

  if ((fabs(phi)         <=M_PI_4) ||
      (fabs(phi-M_PI)    <=M_PI_4) ||
      (fabs(phi-2.0*M_PI)<=M_PI_4)) {
    /* Rather "vertical line" :
       find intersection with horizontal lines of the tiling */
    for (iy=0; iy<ny; iy++) {
      yy = y[iy];
      xx = -(l[1]*yy+l[2])/l[0];
      ix = (int) floor((xx-xmin)/dx);
      if (TIsValid2(T,ie,ix,iy  ))
	STilesAddUnique(L, Tilings,i,ie,ix,iy  ,j);
      if (TIsValid2(T,ie,ix,iy-1))
	STilesAddUnique(L, Tilings,i,ie,ix,iy-1,j);
    }
  } else {
    /* Rather "horizontal line" :
       find intersection with vertical lines of the tiling */
    for (ix=0; ix<ny; ix++) {
      xx = x[ix];
      yy = -(l[0]*xx+l[2])/l[1];
      iy = (int) floor((yy-ymin)/dy);
      if (TIsValid2(T,ie,ix  ,iy))
	STilesAddUnique(L, Tilings,i,ie,ix  ,iy,j);
      if (TIsValid2(T,ie,ix-1,iy))
	STilesAddUnique(L, Tilings,i,ie,ix-1,iy,j);
    }
  }
#ifndef HasSTL
  /* Add segment j to all tiles in L */
  tuple = (float) j;
  for (k=0;k<L->size;k++) {
    i  = (int) (L->values[4*k+0]);
    ie = (int) (L->values[4*k+1]);
    ix = (int) (L->values[4*k+2]);
    iy = (int) (L->values[4*k+3]);
    csegs = TSegs(Tilings[i],ie,ix,iy);
    FlistAdd(csegs,&tuple);
  }
#endif

  STiles(S,j) = FlistCat(STiles(S,j),L);

  mw_delete_flist(L);

  return 1;

}

/*
  Given a low-precision tiling TT, and a high-precision tiling T.
  Find all tiles tt(ie2,ix2,iy2) in TT meeting tile t(ie,ix,iy) in T.
  Return the number of intersecting tiles.

  With the current implementation we still miss:
    - intersections between interior and exterior tiles at the same or different levels
   
 */
int FindIntersection(T,ie,ix,iy,TT,ie2,ix2,iy2)
     Tiling *T,*TT;
     int  ie,  ix,   iy;
     int *ie2,*ix2,*iy2;
{
  int k,n,m,new_tile;
  double *x,*y,*y2,*x2;
  double dx2, dy2, xmin2, ymin2;
  int ny2;
  int IE2, IX2, IY2;

  /* Used to iterate on 4-neighbours */
  int DX[4],DY[4];
  DX[0]=0;DX[1]=1;DX[2]=0;DX[3]=1;
  DY[0]=0;DY[1]=0;DY[2]=1;DY[3]=1;

  /* Compute values defining finer grid */
  *ie2 = ie;
  x    = T[ie].x;                   x2    = TT[*ie2].x;
/*dx   = T[ie].x[1]-T[ie].x[0];*/   dx2   = TT[*ie2].x[1]-T[*ie2].x[0];
/*xmin = T[ie].x[0];           */   xmin2 = TT[*ie2].x[0];

/*ny   = T[ie].ny*/                 ny2   = TT[*ie2].ny;
  y    = T[ie].y;                   y2    = TT[*ie2].y;
/*dy   = T[ie].y[1]-T[ie].y[0];*/   dy2   = TT[*ie2].y[1]-TT[*ie2].y[0];
/*ymin = T[ie].y[0];*/              ymin2 = TT[*ie2].y[0];


  for (k=0,n=0; k<4; k++) {
    IE2 = ie2[n] = ie;
    IX2 = ix2[n] = (int) floor((x[ix+DX[k]]-xmin2)/dx2);
    if (ie==INTERIOR_TILING)
      IY2 = iy2[n] = (int) floor((y[iy+DY[k]]-ymin2)/dy2);
    else {
      /* y = "distance to center" is not regularly spaced */
      /* if (y[iy]>=y2[0]) not necessary as in TAddSegment */
      IY2 = iy2[n] = (k==0)?0:iy2[0];
      while( (iy2[n] < ny2) && (y2[iy2[n]]<y[iy+DY[k]]) ) (iy2[n])++;
      if (iy2[n]>0) (iy2[n])--;
      IY2 = iy2[n];
    }
    new_tile = (1==1);
    for (m=0; m<n; m++)
      new_tile = new_tile && ( (ie2[n]!=ie2[m]) || (iy2[n]!=iy2[m]) || (ix2[n]!=ix2[m]) );
    if (new_tile)
      n++;

    if ((fabs(x2[IX2]-x[ix+DX[k]])<dx2/1000.0) && (IX2>0)) {
      ie2[n] = IE2;
      ix2[n] = IX2-1;
      iy2[n] = IY2;
      new_tile = (1==1);
      for (m=0; m<n; m++)
	new_tile = new_tile && ( (ie2[n]!=ie2[m]) || (iy2[n]!=iy2[m]) || (ix2[n]!=ix2[m]) );
      if (new_tile)
	n++;
    }
      
    if ((fabs(y2[IY2]-y[iy+DY[k]])<y2[IY2]/1000.0) && (IY2>0)) {
      ie2[n] = IE2;
      ix2[n] = IX2;
      iy2[n] = IY2-1;
      new_tile = (1==1);
      for (m=0; m<n; m++)
	new_tile = new_tile && ( (ie2[n]!=ie2[m]) || (iy2[n]!=iy2[m]) || (ix2[n]!=ix2[m]) );
      if (new_tile)
	n++;
    }
      
    
  }

  return n;
}


/*------------------------------------------------------------*/
/*                         MAIN MODULE                        */
/*------------------------------------------------------------*/


int vpoint(imagein,allsegs,output,segs,eps,all,masked,verbose,maskedVPs)
Fimage imagein;
Fimage allsegs;
Flist output;
Flists segs;
double *eps;
char   *all;
char   *masked;
char   *verbose;
int    *maskedVPs;
{
  int N,*M,min_pl,max_pl,n_pl,i,j,k,ntheta,itn;
  int    ie, ix, iy;
  int i2,ie2,ix2,iy2;
  int max_i,max_ie,max_ix,max_iy;
  float max_meaning;
  int    IE[9],IX[9],IY[9]; /* used to hold the output of FindIntersection */
  int    n;
  int NVP[2],nvp,nvp1,nvp2,nvp3;
  double R,X0,Y0;
  double **B,**Binf;
  double *p,meaning,*p_inf,threshold;
  Tiling **Tilings, *T, *TT;
  Flists Segments;
  Flist tiles;
  SegListIterator it;
  SegList CSegs;
  float  *csegs;
  int    nsegs,besttile;
  float x1,x2,x3,x4;
  float y1,y2,y3,y4;
  float d1,d2,d3,d4;
  float t1,t2,t3,t4;
  int searching_masked;
  float tuple[4];
  char *msg1, *msg2;
  Flist VPlist[2];


  /* Build data structure to hold pointers from segments to tiles
     (useful for MDL) */
     Segments = newSegments(allsegs);

  /* Number of detected lines */
     N = Segments->size;

  /* Radius and center of circumscribed circle containing image domain */
     R = sqrt((double) imagein->nrow * (double)imagein->nrow
	      + (double) imagein->ncol * (double)imagein->ncol) / 2.0; 
     X0 = (double)imagein->ncol/2.0;
     Y0 = (double)imagein->nrow/2.0;
  /* Establish series of n_pl dyadic angular precision levels
     for the vanishing region tilings */
     min_pl = 4;
     max_pl = 9;
     n_pl = max_pl-min_pl+1;
     M = calloc(n_pl,sizeof(int));
     B = calloc(n_pl,sizeof(double*));
     Binf = calloc(n_pl,sizeof(double*));
     p = calloc(n_pl,sizeof(double));
     p_inf = calloc(n_pl,sizeof(double));
 
  /* Detection threshold for NFA is epsilon
     divided by the number of precision levels.
     Detection threshold for meaningfulness (-log10(NFA)) is then
     -log10(epsilon/n_pl) == -log10(epsilon) + log10(n_pl)
  */
     threshold = *eps + log10(n_pl);

     Tilings = malloc(n_pl*sizeof(Tiling**));


  /*
     Create the Tiling data structure for all precision levels
   */

     /*** For each angular precision level ... ***/
     for(i=0; i<n_pl; i++) {
       /* Number of orientations */
       ntheta = (int) floor(pow(2.0, (double) (min_pl + i)) + .5);
       if (verbose) fprintf(stderr,
	       "\nBuilding data structure for angular precision = pi/%d\n",
	       ntheta);	       

       /* Create the tiling for this precision level */
       M[i]=N;
       T = Tilings[i] = newTiling(ntheta,&(p[i]),&(p_inf[i]),&(M[i]));
       if (verbose) fprintf(stderr,
	       "  Total number of tiles = %d internal + %d external = %d\n",
	       T[0].nx*T[0].ny,T[1].nx*T[1].ny,
	       T[0].nx*T[0].ny+T[1].nx*T[1].ny);
     }


  /*
     Search for meaningful vanishing regions ...
   */

     VPlist[0] = mw_change_flist(NULL,2,0,4);
     VPlist[1] = mw_change_flist(NULL,2,0,4);
    
     searching_masked = 0;
     NVP[0] = NVP[1] = 0;
     itn = 0;
     
     while (1) { /* main loop */

     if (verbose) fprintf(stderr,
			  "\n\n*** Searching for vanishing regions (iteration # %d)\n\n",
			  itn);
     nvp1 = 0;
     /*** For each angular precision level ... ***/
     for(i=0; i<n_pl; i++) { /* for i */

       T = Tilings[i];

       /*** For each segment, update all tiles it meets ***/
       for (N=0,j=0;j<Segments->size;j++) {
	 if (!searching_masked) N++; /* to search non-masked VPs count all segments */
	 if (SIsValid(Segments,j)>0.0) {
	     TAddSegment(Tilings,i,Segments,j,R,X0,Y0);
	     if (searching_masked) N++; /* to search masked VPs count only remaining segments */
	   }
       }

       /*** Compute NFA, meaningfulness for each tile ***/
       B[i]    = binomial_tail(N,p[i]);
       for (ie=0;ie<=1;ie++)
	 for (ix=0;ix<T[ie].nx;ix++)
	   for (iy=0;iy<T[ie].ny;iy++)
	     if (TIsValid(T,ie,ix,iy)) {
	       TMeaning(T,ie,ix,iy) = -log10(M[i]*B[i][SegListSize(TSegs(T,ie,ix,iy))]);
	     } else {
	       TMeaning(T,ie,ix,iy) = TMDLMeaning(T,ie,ix,iy); /* == -Inf by default */
	     }
       /* ATTN: for infinite tiles use p_inf < p !! */
       if (IdentifyOppositeTiles == 1)
	 /* use 2*p_inf as an upper bound of the probability
	    of a line meeting any of both opposite tiles */
	 Binf[i] = binomial_tail(N,2.0*p_inf[i]);
       else
	 Binf[i] = binomial_tail(N,p_inf[i]);
       ie = EXTERIOR_TILING;
       iy = T[ie].ny-1;
	 for (ix=0;ix<T[ie].nx;ix++)
	     if (TIsValid(T,ie,ix,iy)) {
	       TMeaning(T,ie,ix,iy) = -log10(M[i]*Binf[i][SegListSize(TSegs(T,ie,ix,iy))]);
	     } else {
	       TMeaning(T,ie,ix,iy) =  TMDLMeaning(T,ie,ix,iy); /* == -Inf by default */
	     }

       /*** Vanishing points are local maxima of meaningfulness ***/
       nvp = 0;
       for (ie=0;ie<=1;ie++)
	 for (iy=0;iy<T[ie].ny;iy++)
	   for (ix=0;ix<T[ie].nx;ix++) {
	     meaning = TMeaning(T,ie,ix,iy);

	     nvp += 
	     TVP(T,ie,ix,iy) = ( TIsValid(T,ie,ix,iy) &&
				 (meaning>=threshold) &&
				 (meaning> TMeaning2(T,ie,ix+1,iy+1)) &&
				 (meaning> TMeaning2(T,ie,ix+1,iy  )) &&
				 (meaning> TMeaning2(T,ie,ix+1,iy-1)) &&
				 (meaning>=TMeaning2(T,ie,ix-1,iy+1)) &&
				 (meaning>=TMeaning2(T,ie,ix-1,iy  )) &&
				 (meaning>=TMeaning2(T,ie,ix-1,iy-1)) &&
				 (meaning>=TMeaning2(T,ie,ix  ,iy+1)) &&
				 (meaning> TMeaning2(T,ie,ix  ,iy-1)) );

	     if (IdentifyOppositeTiles == 0) {
             /* Treat the special case of pairs of infinite tiles
		in the following way:
		If they are both meaningful with the same nfa,
		then only keep the one with lowest ix (theta)
		coordinate, as meaningful.
	     */
	     if ((ie==EXTERIOR_TILING) &&
		 (iy==(T[ie].ny-1)) &&
		 TVP(T,ie,ix,iy) &&
		 (meaning==TMeaning2(T,ie,ix  ,iy+1)) &&
		 (ix>TOpposite(T,ix,iy)))
	       TVP(T,ie,ix,iy) = 0;
	     }
	   }
       if (verbose) fprintf(stderr,
       "  Found %d maximal meaningful regions at this precision level\n",
	       nvp);

       nvp1+=nvp;
     }

     /*** Vanishing points must be also
	  maximal meaningful over precision levels ***/
     if (verbose) fprintf(stderr,
	     "\nTesting inclusion relationships between precision levels...\n"
	     );	       

     /* for each meaningful tile t(i,ie,ix,iy) */
     for(i=1; i<n_pl; i++) {
       T = Tilings[i];
       for (ie=0;ie<=1;ie++)
	 for (iy=0;iy<T[ie].ny;iy++)
	   for (ix=0;ix<T[ie].nx;ix++)
	     if ((TMeaning2(T,ie,ix,iy)>=threshold) || TMDLVP(T,ie,ix,iy)) {
	       /* find all coarser intersecting tiles t_k(j,IE[k],IX[k],IY[k]) */
	       for(j=0; j<i; j++) {
		 TT = Tilings[j];
		 n = FindIntersection(T,ie,ix,iy,TT,IE,IX,IY);
		 for (k=0; k<n; k++)
		   if ((TMeaning2(TT,IE[k],IX[k],IY[k])>=threshold) || TMDLVP(TT,IE[k],IX[k],IY[k])) {
		     if (TMeaning2(TT,IE[k],IX[k],IY[k])>TMeaning2(T,ie,ix,iy))
		       TVP(T,ie,ix,iy) = 0; /* t(i,ie,ix,iy) is not maximal */
		     else
		       TVP(TT,IE[k],IX[k],IY[k]) = 0; /* t_k(j,IE[k],IX[k],IY[k]) is not maximal */
		   }
	       }
	     }
     }

     if (all) {
       /* All maximal vanishing regions (no MDL) */
       nvp2 = 0;
       for(i=1; i<n_pl; i++) {
	 T = Tilings[i];
	 for (ie=0;ie<=1;ie++)
	   for (iy=0;iy<T[ie].ny;iy++)
	     for (ix=0;ix<T[ie].nx;ix++)
	       if (TVP(T,ie,ix,iy)) {
		 /* Remove t from the list of valid tiles */
		 TVP(T,ie,ix,iy) = TIsValid(T,ie,ix,iy) = 0;
		 /* Add t to the list of MDL meaningful tiles */
		 TMDLVP(T,ie,ix,iy) = 1;
		 TMDLMeaning(T,ie,ix,iy) = TMeaning(T,ie,ix,iy);
		 nvp2++;
		 tuple[0] = (float) i;
		 tuple[1] = (float) ie;
		 tuple[2] = (float) ix;
		 tuple[3] = (float) iy;
		 FlistAdd(VPlist[searching_masked],tuple);
		 TMDLSegs(T,ie,ix,iy) = newSegListRef(TSegs(T,ie,ix,iy));
	       }
       }
       if (verbose)
	 fprintf(stderr,
		 "  %d (out of %d) regions remaining after inclusion test\n",
		 nvp2,nvp1);
       nvp3 = nvp2;

     } else {
     /*** Minimum Description Length ***/
     if (verbose) fprintf(stderr,
	     "\nMinimum Description Length...\n"
	     );
     nvp3 = 0;
     /* search most meaningful tile */
     max_meaning = threshold-1.0;
     for(i=0; i<n_pl; i++) {
       T = Tilings[i];
       for (ie=0;ie<=1;ie++)
	 for (iy=0;iy<T[ie].ny;iy++)
	   for (ix=0;ix<T[ie].nx;ix++)
	     if (TVP(T,ie,ix,iy)) {
	       CSegs = TSegs(T,ie,ix,iy);
	       nsegs = SegListSize(CSegs);
	       meaning = TMeaning(T,ie,ix,iy);
	       if (meaning>max_meaning) {
		 max_i = i;
		 max_ie= ie;
		 max_ix= ix;
		 max_iy= iy;
		 max_meaning=meaning;
	       }
	     }
     }
     /* If meaningful add it to the list of MDL-meaningful VPs */
     if (max_meaning >= threshold) {
       T = Tilings[max_i];
       /* Remove t from the list of valid tiles */
       TVP(T,max_ie,max_ix,max_iy) = TIsValid(T,max_ie,max_ix,max_iy) = 0;
       /* Add t in the list of MDL meaningful tiles */
       TMDLVP(T,max_ie,max_ix,max_iy) = 1;
       TMDLMeaning(T,max_ie,max_ix,max_iy) = max_meaning;
       tuple[0] = (float) max_i;
       tuple[1] = (float) max_ie;
       tuple[2] = (float) max_ix;
       tuple[3] = (float) max_iy;
       FlistAdd(VPlist[searching_masked],tuple);
       /* For each segment j meeting this tile t ... */
       CSegs = TSegs(T,max_ie,max_ix,max_iy);
       for (it=SegListBegin(CSegs); !SegListEnd(CSegs,it); SegListNext(it)) {
	 /* Remove segment j from the list of Segments
	    to be considered in the next iteration */
	 j = SegListValue(it);
	 SIsValid(Segments,j) = 0;
	 /* Add j to MDLSegs of t */
#ifdef HasSTL
	 SegListInsert(TMDLSegs(T,max_ie,max_ix,max_iy),j);
#else
	 tuple[0] = (float) j;
	 FlistAdd(TMDLSegs(T,max_ie,max_ix,max_iy),tuple);
#endif
	   
       }
       nvp3++;
     } /* end if max_meaning >= threshold */

     if (verbose) fprintf(stderr,
	     "  %d (out of %d) regions remaining after MDL.\n",
	     nvp3,nvp1);

     } /* end: if (all) ... else (MDL) ... */

     if (nvp3==0) /* no more (MDL-maximal) meaningful tiles */
       if (masked && !searching_masked)
	 searching_masked = 1; /* look for masked MDL-maximal meaningful tiles */
       else
	 break; /* stop searching */

     /* delete all tile-segment intersections for next iteration */
     for (j=0;j<Segments->size;j++)
       if (SIsValid(Segments,j)>0.0)
	 FlistFlush(STiles(Segments,j));
     for(i=0; i<n_pl; i++) {
       T = Tilings[i];
       for (ie=0;ie<=1;ie++)
	 for (iy=0;iy<T[ie].ny;iy++)
	   for (ix=0;ix<T[ie].nx;ix++)
	     if (TIsValid(T,ie,ix,iy))
	       FlistFlush(TSegs(T,ie,ix,iy));
       free(B[i]);
       free(Binf[i]);
     }

     NVP[searching_masked] += nvp3;

     itn++;

     } /* main loop */


     /*** Prepare output ***/
     output = mw_change_flist(output,NVP[0]+NVP[1],0,10);
     if (segs)
       segs = mw_change_flists(segs,NVP[0]+NVP[1],0);

     for (searching_masked=0; searching_masked<=((masked)?1:0); searching_masked++) {

       if (verbose) {
	 if (all)
	   msg1 = "maximal";
	 else
	   msg1 = "MDL maximal";
	 
	 if (searching_masked)
	   msg2 = "masked ";
	 else
	   msg2 = "";

	 fprintf(stderr,
		 "\n*** Found %d %s%s meaningful vanishing regions:\n",
		 NVP[searching_masked],msg2,msg1);
       }

       for (k=0; k<VPlist[searching_masked]->size; k++) {

	 i  = (int) ((VPlist[searching_masked])->values[k*4+0]);
	 ie = (int) ((VPlist[searching_masked])->values[k*4+1]);
	 ix = (int) ((VPlist[searching_masked])->values[k*4+2]);
 	 iy = (int) ((VPlist[searching_masked])->values[k*4+3]);

	 T = Tilings[i];

	 if (ie == INTERIOR_TILING) {
	   if (verbose) fprintf(stderr,"\nVP%d:\n",output->size);
	   x1 = x4 = (T[ie].x[ix  ]*R + X0);
	   x2 = x3 = (T[ie].x[ix+1]*R + X0);
	   y1 = y2 = (T[ie].y[iy  ]*R + Y0);
	   y3 = y4 = (T[ie].y[iy+1]*R + Y0);
	   meaning = TMeaning(T,ie,ix,iy);
	   output->values[10*(output->size)+0] = x1;
	   output->values[10*(output->size)+1] = y1;
	   output->values[10*(output->size)+2] = x2;
	   output->values[10*(output->size)+3] = y2;
	   output->values[10*(output->size)+4] = x3;
	   output->values[10*(output->size)+5] = y3;
	   output->values[10*(output->size)+6] = x4;
	   output->values[10*(output->size)+7] = y4;
	   output->values[10*(output->size)+8] = meaning;
	   output->values[10*(output->size)+9] = (i+min_pl);
	   output->size++;
	   if (segs)
	     segs->list[segs->size++] = SegListCopy(TMDLSegs(T,ie,ix,iy));

	   if (verbose) fprintf(stderr,
				"   precision level = pi/%g\n",
				pow(2.0,(i+min_pl)));
	   if (verbose) fprintf(stderr,
				"   meaningfulness  = %g\n",
				meaning);
	   if (verbose) fprintf(stderr,
				"   meaningfulness after MDL = %g\n",
				TMDLMeaning(T,ie,ix,iy));
	   if (verbose) fprintf(stderr,
				"   normalized coordinates: x in [%g,%g], y in [%g,%g]\n",
				T[ie].x[ix  ],T[ie].x[ix+1],
				T[ie].y[iy  ],T[ie].y[iy+1]);
	   if (verbose) fprintf(stderr,
				"   pixel coordinates: x in [%g,%g], y in [%g,%g]\n",
				x1,x2,
				y1,y3);
	 } else { /* ie == EXTERIOR_TILING */
	   if (verbose) fprintf(stderr,"\nVP%d:\n",output->size);
	   t1 = t4 = T[ie].x[ix  ];
	   t2 = t3 = T[ie].x[ix+1];
	   d1 = d2 = T[ie].y[iy  ];
	   d3 = d4 = (iy==(T[ie].ny-1))? 2.0*T[ie].y[iy  ] : T[ie].y[iy+1];
	   polar2cart(t1,d1,R,X0,Y0,&x1,&y1);
	   polar2cart(t2,d2,R,X0,Y0,&x2,&y2);
	   polar2cart(t3,d3,R,X0,Y0,&x3,&y3);
	   polar2cart(t4,d4,R,X0,Y0,&x4,&y4);
	   meaning = TMeaning(T,ie,ix,iy);
	   output->values[10*(output->size)+0] = x1;
	   output->values[10*(output->size)+1] = y1;
	   output->values[10*(output->size)+2] = x2;
	   output->values[10*(output->size)+3] = y2;
	   output->values[10*(output->size)+4] = x3;
	   output->values[10*(output->size)+5] = y3;
	   output->values[10*(output->size)+6] = x4;
	   output->values[10*(output->size)+7] = y4;
	   output->values[10*(output->size)+8] = meaning;
	   output->values[10*(output->size)+9] = (i+min_pl);
	   output->size++;
	   if (segs)
	     segs->list[segs->size++] = SegListCopy(TMDLSegs(T,ie,ix,iy));
	   
	   if (verbose) fprintf(stderr,
				"   precision level = pi/%g\n",
				pow(2.0,(i+min_pl)));
	   if (verbose) fprintf(stderr,
				"   meaningfulness  = %g\n",
				meaning);
	   if (verbose) fprintf(stderr,
				"   meaningfulness after MDL = %g\n",
				TMDLMeaning(T,ie,ix,iy));
	   if (verbose) fprintf(stderr,
				"   normalized coordinates: theta in [%g,%g], rho in [%g,%g]\n",
				t1,t2,
				d1,d3);
	   if (verbose) fprintf(stderr,
				"   pixel coordinates: x in [%g,%g], y in [%g,%g]\n",
				x1,x2,
				y1,y3);
	 }
       }
     }


     /* Cleanup */

     deleteSegments(Segments);
     for(i=0; i<n_pl; i++) {
       deleteTiling(Tilings[i]);
       free(B[i]);
       free(Binf[i]);
     }
     free(Tilings);
     free(B);
     free(Binf);
     free(M);

     /* screen output */
     *maskedVPs = NVP[1];
     return NVP[0];


}






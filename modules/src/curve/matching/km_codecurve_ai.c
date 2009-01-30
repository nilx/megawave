/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {km_codecurve_ai};
version = {"1.1"};
author = {"Jose-Luis Lisani, Pablo Muse, Frederic Sur"};
function = {"Compute affine-invariant codes for a single curve"};
usage = {
   NNorm->NN             "number of points in the coder",
   FNorm->FN             "ratio of points to be encoded",
   NCurve->NC            "index of the curve in the list",
   curve->curve          "curve to encode (Flist)",
   curve_IP->curve_IP    "indices of inflexion points of the curve (1-Flist)",
   curve_FP->curve_FP    "indices of flat points of the curve (1-Flist)",
   curve_BP->curve_BP    "indices of bitangent points of the curve (2-Flist)",
   dictionary<-dict      "output dictionnary (Flists)"
        };
*/

/*----------------------------------------------------------------------
 v1.1: corrected minor FABS bug (L.Moisan)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include "mw.h" 

#define FABSF(x) ((float)fabs((double)(x)))

#define qnorm(a,b) ((a)*(a)+(b)*(b))

#define _(a,i,j) ((a)->values[(i)*((a)->dim)+(j)])

#define PI 3.1415926535897931
#define EPSDIST6 1E-4F
/*#define MINANGLE 1.309F */ /*75 degres*/
/*#define MINANGLE 0.7854F*/ /*45 degres*/
/*#define MINANGLE 0.52360F*/ /*30 degres*/
/*#define MINANGLE 0.34907F*//*20 degres*/
#define MINANGLE 0.17454F/*10 degres*/
/*#define MINANGLE 1.0472F*/ /*60 degres*/
/*#define MINANGLE 0.8727F*/ /*50 degres*/

static int NNorm;
static float FNorm;
static char Closed;
static int N_Points;
static int Ncurve;

static float M_aff[3][3]; 

 
struct NormDataAI { 
  int Numcurve; 
  int i_left, i_right; 
  float xC, yC; 
  int iL1, iL2; 
  float xR1, yR1, xR2, yR2, xR3, yR3; 
  float disc; 
}; 
 

/* compute index following i and last index of the curve (depending on the 
   direction 1 or 0) */

static int get_next_index(int i, int iLast, unsigned char type)
{
  int i_next;
  
  if (type == 1) {
    if ((Closed) && (iLast == 0)) iLast=N_Points-1;
  } else {
    if ((Closed) && (iLast == N_Points-1)) iLast=0;
  }
  
  if (type == 1) {
    i_next=i+1;
    if ((i_next > N_Points-1) && (!Closed)) return -1;
    if ((i_next > N_Points-1) && (Closed)) i_next=1;
  } else {
    i_next=i-1;
    if ((i_next < 0) && (!Closed)) return -1;
    if ((i_next < 0) && (Closed)) i_next=N_Points-2;
  }
  if (i_next == iLast) return -1;
  
  return i_next;
}


/* test the position of a point w.r.t. a line */ 

static int halfplane(float x1, float y1, float x2, float y2, float x, float y)
{
  float u, v;

  u=(x2-x1)*(y2-y);
  v=(x2-x)*(y2-y1);
  if (u > v) { if ((u-v)<EPSDIST6) return(0); else return(1); } 
  if ((v-u)<EPSDIST6) return(0); else return(-1);
}


/* compute angle between u0 and v0. 
   Radians, between -PI and +PI */

static float angle(float u0x, float u0y, float v0x, float v0y)
{
  float c,s;
  
  c=(u0x*v0x+u0y*v0y)/((float)sqrt(qnorm(u0x,u0y)*qnorm(v0x,v0y)));
  s=(u0x*v0y-u0y*v0x)/((float)sqrt(qnorm(u0x,u0y)*qnorm(v0x,v0y)));
  return ((float) atan2(s,c));
}


static int get_next_tangent_aux(Flist curve, int iFirst, int iLast, float x1, float y1, float x2, float y2, unsigned char *same_orientation)
{
  int i, i_next, i_prev, hL, hR;
  float xL, yL, xR, yR, x0, y0;

  i_prev=iFirst;
  i=get_next_index(i_prev, iLast, 1);
  if (i < 0) return -1;

  i_next=get_next_index(i, iLast, 1);
  if (i_next < 0) return -1;

  xL = _(curve, i_prev, 0);
  yL = _(curve, i_prev, 1);    
  xR = _(curve, i_next, 0);    
  yR = _(curve, i_next, 1);    
  x0 = _(curve, i, 0);     
  y0 = _(curve, i, 1);     


  do {
    hL=halfplane(x0, y0, x0+x2-x1, y0+y2-y1, xL, yL);
    hR=halfplane(x0, y0, x0+x2-x1, y0+y2-y1, xR, yR);
    if (hL*hR >= 0) {
      if (((xR-x0)*(x2-x1)+(yR-y0)*(y2-y1)) >= 0) *same_orientation=1;
      else {
	*same_orientation=0;
	return i;
      }
    }
    i_prev=i;
    xL=x0; yL=y0;
    i=i_next;
    x0=xR; y0=yR;
    i_next=get_next_index(i, iLast, 1);
    if (i_next < 0) return -1;
    xR = _(curve, i_next, 0);
    yR = _(curve, i_next, 1);
  } while (1);

  return -1;
}

static int get_next_tangent(Flist curve, int iFirst, int iLast, int iFirstC, unsigned char *same_orientation)
{
  float x1, y1, x2, y2;
  int i;

  x1 = _(curve, iFirst, 0);
  y1 = _(curve, iFirst, 1);
  x2 = _(curve, iLast, 0);
  y2 = _(curve, iLast, 1);

  i=get_next_tangent_aux(curve, iFirstC, iFirst, x1, y1, x2, y2, same_orientation);

  return i;

}

/* get intersection point between straight lines (A1,A2) and (A3,A4)
   if angle of intersection < Minangle then intersection not valid */
static unsigned char get_intersection_point(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float *x, float *y, float MinAngle)
{
  float det, lambda, ang;

  ang = angle(x2-x1, y2-y1, x4-x3, y4-y3);
  if ((FABSF(ang) < MinAngle) || (PI-FABSF(ang) < MinAngle)) {
    return 0;
  }

  det=(x2-x1)*(y3-y4)-(y2-y1)*(x3-x4);
  if (det == 0) {
     return 0;
  }
  lambda=(x3-x1)*(y3-y4)-(y3-y1)*(x3-x4);
  lambda/=det;
  
  *x=x1+lambda*(x2-x1);
  *y=y1+lambda*(y2-y1);

  return 1;
}

static void get_perpendicular_points(Flist curve, int i1, int i2, int i3, float *xP1, float *yP1, float *xP2, float *yP2)
{
  float x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;

  x1 = _(curve, i1, 0);
  y1 = _(curve, i1, 1);
  x2 = _(curve, i2, 0);
  y2 = _(curve, i2, 1);
  x3 = _(curve, i3, 0);
  y3 = _(curve, i3, 1);
  x4 = x3 + x2 - x1;
  y4 = y3 + y2 - y1;
  x5 = x1 + y1 - y2;
  y5 = y1 + x2 - x1;
  get_intersection_point(x1, y1, x5, y5, x3, y3, x4, y4, xP2, yP2, 0.0);
  *xP1 = x1; *yP1 = y1;
}


/* intersection between a line and the curve */
static unsigned char intersection_curve_line(Flist curve, int iFirst, int iLast, float x1, float y1, float x2, float y2, float *xC, float *yC, int *i_left, int *i_right)
{
  int i, i_next, hL, hR;
  float xL, yL, xR, yR;

  i=iFirst;
  xL = _(curve, i, 0);
  yL = _(curve, i, 1);
  hL=halfplane(x1, y1, x2, y2, xL, yL);
  do {
	i_next=get_next_index(i, iLast, 1);
	if (i_next < 0) {
	  return 0;
	}
	xR = _(curve, i_next, 0);
	yR = _(curve, i_next, 1);
	hR=halfplane(x1, y1, x2, y2, xR, yR);
	if (hR*hL <= 0) {
	  if (hL == 0) { *xC=xL; *yC=yL; *i_left=*i_right=i;}
	  if (hR == 0) { *xC=xR; *yC=yR; *i_left=*i_right=i_next;}
	  if (hL*hR != 0) {
		if (!get_intersection_point(x1, y1, x2, y2, xL, yL, xR, yR, xC, yC, 0.0)) return 0;
	    *i_left=i;
	    *i_right=i_next;
	  }
	} else {
	  hL=hR;
	  xL=xR;
	  yL=yR;
	  i=i_next;
	}
  } while (hR*hL > 0);

  return 1;
}



/* get parallel to the two basis straight lines with distance d (=1/3) */ 

static unsigned char get_cross_parallel(Flist curve, int i1, int i2, int i3, float xP1, float yP1, float xP2, float yP2, float d, float *xC, float *yC, int *i_left, int *i_right)
{
  float x1, y1, x2, y2, x3, y3, x4, y4;

  x1 = _(curve, i1, 0);
  y1 = _(curve, i1, 1);
  x2 = _(curve, i2, 0);
  y2 = _(curve, i2, 1);
  x3=xP1+d*(xP2-xP1);
  y3=yP1+d*(yP2-yP1);
  x4=x3+x2-x1;
  y4=y3+y2-y1;

  if (!intersection_curve_line(curve, i2, i3, x3, y3, x4, y4, xC, yC, i_left, i_right)) return 0;

  return 1;
}


static unsigned char find_intersection_pointsAI(Flist curve, int i1, int i2, int i3, int i4, float xC1, float yC1, float xC2, float yC2, float *xR1, float *yR1, float *xR2, float *yR2, float *xR3, float *yR3, float *xR4, float *yR4, float MinAngle)
{
  float x1, y1, x2, y2, x3, y3, x3b, y3b, x4, y4, x4b, y4b;
  
  x1 = _(curve, i1, 0);
  y1 = _(curve, i1, 1);
  x2 = _(curve, i2, 0);
  y2 = _(curve, i2, 1);
  x3 = _(curve, i3, 0);
  y3 = _(curve, i3, 1);
  x4 = _(curve, i4, 0);
  y4 = _(curve, i4, 1);
  x3b=x3+x2-x1;
  y3b=y3+y2-y1;
  x4b=x4+xC2-xC1;
  y4b=y4+yC2-yC1;

  if (!get_intersection_point(x1, y1, x2, y2, x4, y4, x4b, y4b, xR1, yR1, MinAngle)) return 0;
  if (!get_intersection_point(x1, y1, x2, y2, xC1, yC1, xC2, yC2, xR2, yR2, MinAngle)) return 0;
  if (!get_intersection_point(x3, y3, x3b, y3b, x4, y4, x4b, y4b, xR3, yR3, MinAngle)) return 0;
  if (!get_intersection_point(x3, y3, x3b, y3b, xC1, yC1, xC2, yC2, xR4, yR4, MinAngle)) return 0;

  return 1;
}


static float detM2(float x1, float y1, float x2, float y2)
{
  return (x1*y2-y1*x2);
}


static unsigned char getMatrixAffine(float x1, float y1, float x2, float y2, float x3, float y3, float x1N, float y1N, float x2N, float y2N, float x3N, float y3N, float (*A)[3])
{
  float a11, a12, a13, a21, a22, a23, a31, a32, a33;
  float a, b, c, d, Tx, Ty;
  float det;

  det=detM2(x2, y2, x3, y3)-detM2(x1, y1, x3, y3)+detM2(x1, y1, x2, y2);

  if (det == 0) return 0;

  a11=detM2(x2, x3, y2, y3)/det;
  a12=-detM2(x1, x3, y1, y3)/det;
  a13=detM2(x1, x2, y1, y2)/det;
  a21=-detM2(1.0, 1.0, y2, y3)/det;
  a22=detM2(1.0, 1.0, y1, y3)/det;
  a23=-detM2(1.0, 1.0, y1, y2)/det;
  a31=detM2(1.0, 1.0, x2, x3)/det;
  a32=-detM2(1.0, 1.0, x1, x3)/det;
  a33=detM2(1.0, 1.0, x1, x2)/det;

  Tx=a11*x1N+a12*x2N+a13*x3N;
  Ty=a11*y1N+a12*y2N+a13*y3N;
  a =a21*x1N+a22*x2N+a23*x3N;
  c =a21*y1N+a22*y2N+a23*y3N;
  b =a31*x1N+a32*x2N+a33*x3N;
  d =a31*y1N+a32*y2N+a33*y3N;

  A[0][0]=a;
  A[0][1]=b;
  A[0][2]=Tx;
  A[1][0]=c;
  A[1][1]=d;
  A[1][2]=Ty;
  A[2][0]=0;
  A[2][1]=0;
  A[2][2]=1;

  return 1;
}

/* compute the normalized coords of (x,y) */
static void normalizeAI(float *x, float *y)
{
  float xN, yN;
  
  xN=M_aff[0][0]*(*x)+M_aff[0][1]*(*y)+M_aff[0][2];
  yN=M_aff[1][0]*(*x)+M_aff[1][1]*(*y)+M_aff[1][2];
  
  *x=xN; *y=yN;
}

static void add_codeAI(Flist arc_code_AI, float x, float y, int m)
{
  normalizeAI(&x, &y);
  _(arc_code_AI,m,0)=x;
  _(arc_code_AI,m,1)=y;
}


/* get next point with distance d */

static int get_next_point_length(Flist fcrv, float *xI0, float *yI0, int iFirst, int iLast, float d, unsigned char type)
{
  int i, i_last;
  float x, y, xP, yP, s, t;
  double dx, dy;
  float splust;

  s=0.0f;
  
  i_last=get_next_index(iLast, iLast, type);
  xP=*xI0;
  yP=*yI0;
  i=iFirst;
  do {
    x=_(fcrv,i,0);
    y=_(fcrv,i,1);
    dx = x - xP;
    dy = y - yP;
    t = (float) sqrt(dx * dx + dy * dy);
    splust=s+t; 
    if (splust < d) {
      s=splust;
      xP=x;
      yP=y;
      i=get_next_index(i, i_last, type);
      if (i < 0) return -1;
    }
  } while (splust < d);
  *xI0=xP+(d-s)*(x-xP)/t;
  *yI0=yP+(d-s)*(y-yP)/t;
  
  return i;
}


/* normalize a piece of curve whose arclength is Lmax */

static int get_normalized_arcAI(Flist fcrvN, Flist fcrv, int iFirst, int iLast, float Lmax, int *iMax, unsigned char type)
{
  float d, x1, y1, x2, y2;
  int i, i_next, n_pointsN, n;

  d=0.0;
  n_pointsN=0;
  i=iFirst;
 
  x1 = _(fcrv, i, 0);
  y1 = _(fcrv, i, 1);
  normalizeAI(&x1, &y1);
  n_pointsN++;
  do {
     i_next=get_next_index(i, iLast, type);
    if (i_next < 0) return 0;
    x2 = _(fcrv, i_next, 0);
    y2 = _(fcrv, i_next, 1);
    normalizeAI(&x2, &y2);
    d+=(float) sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    i=i_next;
    x1=x2; y1=y2;
    n_pointsN++;
  } while (d < Lmax);

  *iMax=i;

  if (mw_change_flist(fcrvN, n_pointsN, n_pointsN, 2)==NULL)
    mwerror(FATAL,1,"error, not enough memory\n");
  
  n=0;
  d=0.0;
  i=iFirst;
  x1 = _(fcrv, i, 0);
  y1 = _(fcrv, i, 1);

  normalizeAI(&x1, &y1);
  _(fcrvN, n, 0) = x1;
  _(fcrvN, n, 1) = y1;
  n++;

  do {
    i_next=get_next_index(i, iLast, type);
    if (i_next < 0) return 0;
    x2 = _(fcrv, i_next, 0);
    y2 = _(fcrv, i_next, 1);
    normalizeAI(&x2, &y2);
    d+=(float) sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    i=i_next;
    x1=x2; y1=y2;
    _(fcrvN, n, 0) = x1;
    _(fcrvN, n, 1) = y1;
    n++;
  } while (d < Lmax);
  
  return 1;

}


static int codeAI_aux(Flist arc_code_AI, Flist curve, float xC, float yC, int i_left, int i_right, float xR1, float yR1, float xR2, float yR2, float xR3, float yR3, float xR4, float yR4)
{
  Flist fcrvAI_left, fcrvAI_right;
  float disc, xN, yN, xP, yP;
  int iN, iP, iN_max, iP_max, m;
  int i_max_left, i_max_right;
  struct NormDataAI *my_data;

  fcrvAI_left = mw_new_flist();
  fcrvAI_right = mw_new_flist();
  my_data = (struct NormDataAI*)malloc(sizeof(struct NormDataAI));

  if (!get_normalized_arcAI(fcrvAI_left,curve, i_left, i_right, FNorm/2.0f, &i_max_left, 0)){
      return 0;
  }
  
  if(!get_normalized_arcAI(fcrvAI_right,curve, i_right, i_max_left, FNorm/2.0f, &i_max_right, 1))
  {  
    mw_delete_flist(fcrvAI_left);
    return 0;
  }

  disc = FNorm/((float) (NNorm-1)); /*Nnorm must be an odd number*/
    
  add_codeAI(arc_code_AI, xC, yC, (NNorm-1)/2);
  xN=xC; yN=yC;
    
    
  normalizeAI(&xN, &yN);
  xP=xN; yP=yN;
  iN=0;
  iP=0;
  iN_max=((int) fcrvAI_right->size)-1;
  iP_max=((int) fcrvAI_left->size)-1;
  
  for (m=0; m < (NNorm-1)/2; m++) {
    iN=get_next_point_length(fcrvAI_right, &xN, &yN, iN, iN_max, disc, 1);
    _(arc_code_AI,(NNorm-1)/2+m+1,0)=xN;
    _(arc_code_AI,(NNorm-1)/2+m+1,1)=yN;
    iP=get_next_point_length(fcrvAI_left, &xP, &yP, iP, iP_max, disc, 1); 
    _(arc_code_AI,(NNorm-1)/2-m-1,0)=xP;
    _(arc_code_AI,(NNorm-1)/2-m-1,1)=yP;
  }
 
  mw_delete_flist(fcrvAI_left);
  mw_delete_flist(fcrvAI_right);

  my_data->i_left = i_left;
  my_data->i_right = i_right;
  my_data->Numcurve = Ncurve;
  my_data->xC = xC;
  my_data->yC = yC;
  my_data->iL1 = i_max_left;
  my_data->iL2 = i_max_right;
  my_data->xR1 = xR1; my_data->yR1 = yR1;
  my_data->xR2 = xR2; my_data->yR2 = yR2;
  my_data->xR3 = xR3; my_data->yR3 = yR3;
  my_data->disc = disc;

  arc_code_AI->data_size = sizeof(struct NormDataAI);
  arc_code_AI->data = (void*)my_data;

  return 1;
}


/* code the piece of curve in the basis of (i1,i2) */
static int codeAI(Flist arc_code_AI, Flist curve, int i1, int i2, float MinAngle)
{
  int i_left, i_right;  float xC, yC;
  unsigned char same_orientation;
  int iP;
  float xP1, yP1, xP2, yP2, xC1, yC1, xC2, yC2;
  int iT;
  float xR1, yR1, xR2, yR2, xR3, yR3, xR4, yR4;
  
  iP=i2;
  iP=get_next_tangent(curve, i1, i2, iP, &same_orientation);
  if (iP < 0){
    return 0;
  }

  get_perpendicular_points(curve, i1, i2, iP, &xP1, &yP1, &xP2, &yP2);

  if (!get_cross_parallel(curve, i1, i2, iP, xP1, yP1, xP2, yP2, 
			  1.0f/3.0f, &xC1, &yC1, &i_left, &i_right))  {
    return 0;
  }

  if (!get_cross_parallel(curve, i1, i2, iP, xP1, yP1, xP2, yP2, 
			  2.0f/3.0f, &xC2, &yC2, &i_left, &i_right)) {
    return 0;
  }
   
  iT=get_next_tangent_aux(curve, iP, i2, xC1, yC1, xC2, yC2, &same_orientation);
  if (iT < 0) {
    return 0;
  }
 
  if (!find_intersection_pointsAI(curve, i1, i2, iP, iT, xC1, yC1, xC2, yC2, 
				  &xR1, &yR1, &xR2, &yR2, &xR3, &yR3, &xR4, &yR4, MinAngle)) {
    return 0;
  }
 

  if (!getMatrixAffine(xR1, yR1, xR2, yR2, xR3, yR3, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, M_aff))
    return 0;

  get_cross_parallel(curve, i1, i2, iP, xP1, yP1, xP2, yP2, 
		     1.0f/2.0f, &xC, &yC, &i_left, &i_right);

  return codeAI_aux(arc_code_AI, curve, xC, yC, i_left, i_right, xR1, yR1, xR2, yR2, xR3, yR3, xR4, yR4);

}



/*------------------------------ MAIN MODULE ------------------------------*/

Flists km_codecurve_ai(Flist curve, Flist curve_IP, Flist curve_FP, Flist curve_BP, Flists dict, int NC, int NN, float FN)
{ 
  Flist arc_code_AI;
  int i1,i2,k;
  float MinAngle;
  
  MinAngle = MINANGLE;

  FNorm = FN;
  Ncurve = NC;
  NNorm = NN;
 

  Closed=((_(curve,0,0)==_(curve,curve->size-1,0))&&(_(curve,0,1)==_(curve,curve->size-1,1)));
  N_Points=curve->size;
      
  if ((arc_code_AI=mw_change_flist(NULL,NNorm,NNorm,2))==NULL) 
    mwerror(FATAL,1,"error, not enough memory\n");
  if ((dict=mw_change_flists(dict,curve_IP->size+curve_FP->size+curve_BP->size,0))==NULL) 
    mwerror (FATAL,1,"error, not enough memory\n");

  /* codes for inflexion points */
  for (k=0;k<curve_IP->size;k++) {
    i1=_(curve_IP,k,0); 
    i2=get_next_index(i1,i1,1);

    if (codeAI(arc_code_AI,curve,i1,i2,MinAngle)){
      dict->list[dict->size] = mw_copy_flist(arc_code_AI,NULL);
      dict->size++;
    }
  }
  
  /* codes for flat points */
  for (k=0;k<curve_FP->size;k++) {
    i1=_(curve_FP,k,0); 
    i2=get_next_index(i1,i1,1);
    if(codeAI(arc_code_AI,curve,i1,i2,MinAngle)){
      dict->list[dict->size] = mw_copy_flist(arc_code_AI,NULL);
      dict->size++;
    }
  }

  /* codes for bitangent points */
  for (k=0;k<curve_BP->size;k++) {
    i1=_(curve_BP,k,0);
    i2=_(curve_BP,k,1); 

    if(codeAI(arc_code_AI,curve,i1,i2,MinAngle)){
      dict->list[dict->size] = mw_copy_flist(arc_code_AI,NULL);
      dict->size++;
    }
  }
  mw_delete_flist(arc_code_AI);
  mw_realloc_flists(dict,dict->size);
  
  return(dict);  
}



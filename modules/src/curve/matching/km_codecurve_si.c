/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {km_codecurve_si};
version = {"1.0"};
author = {"Jose-Luis Lisani, Pablo Muse, Frederic Sur"};
function = {"Compute similitude-invariant codes for a single curve"};
usage = {
   NNorm->NN             "number of points in the code",
   FNorm->FN             "ratio length to be encoded",
   NCurve->NC            "index of the curve in the list",
   curve->curve          "curve to encode (Flist)",
   curve_IP->curve_IP    "indices of inflexion points of the curve (1-Flist)",
   curve_FP->curve_FP    "indices of flat points of the curve (1-Flist)",
   curve_BP->curve_BP    "indices of bitangent points of the curve (2-Flist)",
   dictionary<-dict      "output dictionnary (Flists)"
        };
*/

#include <stdlib.h>
#include <math.h>
#include "mw.h" 


#define qnorm(a,b) ((a)*(a)+(b)*(b))

#define _(a,i,j) ((a)->values[(i)*((a)->dim)+(j)])

#define PI 3.1415926535897931
#define EPSDIST6 1E-4F

static int NNorm;
static int NNorm2;
static float FNorm;
static char Closed;
static int N_Points;
static int Ncurve;

struct NormDataSIconcat { 
  int Numcurve_in_llconcat;
  int Numimage;
  int Numcurve; 
  int i_left, i_right; 
  float xC, yC; 
  int iL1, iL2; 
  float xR1, yR1, xR2, yR2; 
  float disc; 
}; 

/* compute index following i and last index of the curve (depending on the 
   direction 1 or 0) */
static int get_next_index(i, iLast, type)
     int i, iLast;
     unsigned char type;
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


/* compute angle between u0 and v0 */

static float angle(u0x,u0y,v0x,v0y)
     float u0x,u0y,v0x,v0y; 
{
  float c,s;

  c=(u0x*v0x+u0y*v0y)/((float)sqrt(qnorm(u0x,u0y)*qnorm(v0x,v0y)));
  s=(u0x*v0y-u0y*v0x)/((float)sqrt(qnorm(u0x,u0y)*qnorm(v0x,v0y)));
  return ((float) atan2(s,c));
}


/* compute index following i2 that makes an angle ang with (i1,i2) and 
   that is smaller than imax */

static int get_next_point_angle(fcrv, i1, i2, i_max, angle_max, type)
     Flist fcrv;
     int i1,i2,i_max;
     float angle_max;
     unsigned char type;
{
  int i, i_next;
  float x1, y1, x2, y2, vx0, vy0, vx, vy, alpha;

  if (type == 1) {
	if ((Closed) && (i_max == 0)) i_max=N_Points-1;
  } else {
    if ((Closed) && (i_max == N_Points-1)) i_max=0;
  }

  i_next=i2;
  x1=_(fcrv,i1,0);
  y1=_(fcrv,i1,1);
  x2=_(fcrv,i_next,0);
  y2=_(fcrv,i_next,1);
  vx0=x2-x1;
  vy0=y2-y1;

  do {
    i=i_next;
    x1=x2;
    y1=y2;
    i_next=get_next_index(i, i_max, type);
    if (i_next < 0) return -1;
    x2=_(fcrv,i_next,0);
    y2=_(fcrv,i_next,1);
    vx=x2-x1;
    vy=y2-y1;
    alpha=angle(vx0, vy0, vx, vy);
  } while (fabs(alpha) < angle_max);
  return i;
}



/* find intersection points between (i1,i2) and the tangent to i */

void find_intersection_point(curve,i1,i2,i,xR,yR)
     Flist curve;
     int i1,i2,i;
     float *xR, *yR;
{
  float ux, uy, vx, vy, lambda, x, y;

  x=_(curve,i1,0);
  y=_(curve,i1,1);
  ux=_(curve,i2,0)-x;
  uy=_(curve,i2,1)-y;
  vx=_(curve,i,0)-x;
  vy=_(curve,i,1)-y;
  lambda=(ux*vx+uy*vy)/qnorm(ux,uy);
  *xR=x+lambda*ux;
  *yR=y+lambda*uy;
}


/* test the position of a point w.r.t. a line */ 

static int halfplane(x1, y1, x2, y2, x, y)
     float x1,y1,x2,y2,x,y;
{
  float u, v;

  u=(x2-x1)*(y2-y);
  v=(x2-x)*(y2-y1);
  if (u > v) { if ((u-v)<EPSDIST6) return(0); else return(1); } 
  if ((v-u)<EPSDIST6) return(0); else return(-1);
}


/* compute the central point C on the curve (coords xc,yc between points 
   of indices ileft and iright */

void find_central_point(curve,xR1,yR1,xR2,yR2,iFirst,iLast,xC,yC,ileft,iright)
     Flist curve;
     float xR1,yR1,xR2,yR2;
     int iFirst, iLast;
     float *xC, *yC;
     int *ileft, *iright;
{
  int i_last, i_next, i;
  float x1,x2,y1,y2,xL,yL,xR,yR;
  float lambda;
  int hL,hR;

  x1=(xR1+xR2)/2;
  y1=(yR1+yR2)/2;
  x2=x1+(yR2-yR1);
  y2=y1-(xR2-xR1);
  i_last=get_next_index(iLast,iLast,1);
  
  i=iFirst;
  xL=_(curve,i,0);
  yL=_(curve,i,1);
  hL=halfplane(x1, y1, x2, y2, xL, yL);
  do {
    i_next=get_next_index(i, i_last, 1);
    xR=_(curve,i_next,0);
    yR=_(curve,i_next,1);
    hR=halfplane(x1, y1, x2, y2, xR, yR);
    if (hR*hL <= 0) {
      if (hL == 0) { *xC=xL; *yC=yL; *ileft=i; *iright=i;}
      if (hR == 0) { *xC=xR; *yC=yR; *ileft=i_next; *iright=i_next;}
      if (hL*hR != 0) {
	lambda=((y1-yL)*(x2-x1)-(x1-xL)*(y2-y1))/((x2-x1)*(yR-yL)-(y2-y1)*(xR-xL));
	*xC=xL+lambda*(xR-xL);
	*yC=yL+lambda*(yR-yL);
	*ileft=i;
	*iright=i_next;
      }
    } else {
      hL=hR;
      xL=xR;
      yL=yR;
      i=i_next;
    }
  } while (hR*hL > 0);
}


/* compute next point with distance d */ 

static int get_next_point_length(fcrv, xI0, yI0, iFirst, iLast, d, type)
     Flist fcrv;
     float *xI0, *yI0;
     int iFirst, iLast;
     float d;
     unsigned char type;
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


/* add the normalized coords of (x,y) in arc_code_SI at index m */

void add_codeSI(arc_code_SI,x,y,m,x0,y0,vux,vuy,L0)
     Flist arc_code_SI;
     float x,y;
     int m;
     float x0,y0,vux,vuy,L0;
{
  float xx, yy;

  xx=((x-x0)*vux+(y-y0)*vuy)/L0;
  yy=(-(x-x0)*vuy+(y-y0)*vux)/L0;
  _(arc_code_SI,m,0)=xx;
  _(arc_code_SI,m,1)=yy;
}


/* compute the NNorm normalized points */

int get_code_SI(arc_code_SI,curve,xC,yC,i_left,i_right,xR1,yR1,xR2,yR2)
     Flist arc_code_SI;
     Flist curve;
     float xC, yC;
     int i_left, i_right;
float xR1,yR1,xR2,yR2;
{
  float x,y,xN,yN,xP,yP;
  int iN, iP, m; 
  float x0,y0,L0,vux,vuy,disc;
  struct NormDataSIconcat *my_data;
  int iNtotal, iPtotal;
  double dx, dy;

  my_data=(struct NormDataSIconcat*)malloc(sizeof(struct NormDataSIconcat));
  x0=(xR1+xR2)/2;
  y0=(yR1+yR2)/2;
  dx = xR2 - xR1;
  dy = yR2 - yR1;
  L0 = (float) sqrt(dx * dx + dy * dy);
  
  vux=(xR2-xR1)/L0;
  vuy=(yR2-yR1)/L0;

  x=xC; y=yC;
  iNtotal=get_next_point_length(curve, &x, &y, i_right, i_left, L0*FNorm/2.0f, 1);
  if (iNtotal < 0) return -1;
  
  x=xC; y=yC;
  iPtotal=get_next_point_length(curve, &x, &y, i_left, iNtotal, L0*FNorm/2.0f, 0);
  if (iPtotal < 0) return -1;

  disc=(L0*FNorm)/((float) (NNorm-1)); /* Nnorm must be an odd number */
  add_codeSI(arc_code_SI, xC, yC,NNorm2,x0,y0,vux,vuy,L0);
  
  xN=xC; yN=yC;
  xP=xC; yP=yC;
  iN=i_right; 
  iP=i_left; 

  for (m=0; m < NNorm2; m++) {
    iN=get_next_point_length(curve, &xN, &yN, iN, iPtotal, disc, 1);
    add_codeSI(arc_code_SI,xN,yN,NNorm2+m+1,x0,y0,vux,vuy,L0);
    iP=get_next_point_length(curve, &xP, &yP, iP, iNtotal, disc, 0);
    add_codeSI(arc_code_SI,xP,yP,NNorm2-m-1,x0,y0,vux,vuy,L0);
  } 
  
  my_data->Numimage=1;
  my_data->Numcurve=Ncurve;
  my_data->Numcurve_in_llconcat=Ncurve;

  my_data->i_left = i_left;
  my_data->i_right = i_right;
  my_data->iL1=iP;
  my_data->iL2=iN;
  my_data->xC = xC;
  my_data->yC = yC;
  my_data->xR1 = xR1; my_data->yR1 = yR1;
  my_data->xR2 = xR2; my_data->yR2 = yR2;
  my_data->disc = disc;
  arc_code_SI->data_size=sizeof(struct NormDataSIconcat);
  arc_code_SI->data=(void*)my_data;
  return 1;
}


/* compute the normalized descriptor of piece of curve in the coords 
   defined by points of indices i1 and i2 */

int codeSI(arc_code_SI,curve,i1,i2)
     Flist arc_code_SI, curve;
     int i1, i2;
{
  int iN, iP, ileft, iright;
  float xC, yC, xR1, yR1, xR2, yR2; 

  iN=get_next_point_angle(curve,i1,i2,i1,(float)PI/2,1); 
  if (iN<0) return -1; 
  iP=get_next_point_angle(curve,i2,i1,i2,(float)PI/2,0);
  if (iP<0) return -1; 
  find_intersection_point(curve,i1,i2,iN,&xR1,&yR1);
  find_intersection_point(curve,i1,i2,iP,&xR2,&yR2);
  find_central_point(curve,xR1,yR1,xR2,yR2,iP,iN,&xC,&yC,&ileft,&iright);
  mwdebug ("xC: %f  yC: %f  i1: %d  i2: %d  ileft: %d  iright: %d\n",xC,yC,i1,i2,ileft,iright);
  return get_code_SI(arc_code_SI,curve,xC,yC,ileft,iright,xR1,yR1,xR2,yR2);
}



/*------------------------------ MAIN MODULE ------------------------------*/

Flists km_codecurve_si(curve,curve_IP,curve_FP,curve_BP,dict,NC,NN,FN)
     Flist curve;
     Flist curve_IP;
     Flist curve_FP;
     Flist curve_BP;
     Flists dict;
     int NC;
     int NN;
     float FN; 
{ 
  Flist arc_code_SI;
  int i1,i2,k;

  Ncurve=NC;
  NNorm=NN;
  NNorm2=(NNorm-1)/2; /* NNorm must be an odd number */
  FNorm=FN;
  Closed=((_(curve,0,0)==_(curve,curve->size-1,0))&&(_(curve,0,1)==_(curve,curve->size-1,1)));
  N_Points=curve->size;
  
  if ((arc_code_SI=mw_change_flist(NULL,NNorm,NNorm,2))==NULL) mwerror(FATAL,1,"error, not enough memory\n");
  if ((dict=mw_change_flists(dict,curve_IP->size+curve_FP->size+curve_BP->size,0))==NULL) mwerror (FATAL,1,"error, not enough memory\n");
  
  /* codes for inflexion points */
  for (k=0;k<curve_IP->size;k++) {
    i1=_(curve_IP,k,0); 
    i2=get_next_index(i1,i1,1);
    if (codeSI(arc_code_SI,curve,i1,i2)>0) {
      dict->list[dict->size] = mw_copy_flist(arc_code_SI,dict->list[dict->size] );
      dict->size++;
    }
  }

  /* codes for flat points */
  for (k=0;k<curve_FP->size;k++) {
    i1=_(curve_FP,k,0); 
    i2=get_next_index(i1,i1,1);
    if (codeSI(arc_code_SI,curve,i1,i2)>0) {
      dict->list[dict->size] = mw_copy_flist(arc_code_SI,dict->list[dict->size] );
      dict->size++;
    }
  }

  /* codes for bitangent points */
  for (k=0;k<curve_BP->size;k++) {
    i1=_(curve_BP,k,0);
    i2=_(curve_BP,k,1); 
    if (codeSI(arc_code_SI,curve,i1,i2)>0) {
      dict->list[dict->size] = mw_copy_flist(arc_code_SI,dict->list[dict->size] );
      dict->size++;
    }
  }

  mwdebug("number of codes in the dictionary: %d\n",dict->size);
  mw_delete_flist(arc_code_SI);
  mw_realloc_flists(dict,dict->size);
  
  return(dict);
}

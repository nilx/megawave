/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {km_inflexionpoints};
version = {"1.0"};
author = {"Jose-Luis Lisani, Pablo Muse, Frederic Sur"};
function = {"Find inflexion points of a curve"};
usage = {
   curve->curve         "input curve (2-Flist)",
   curve_IP<-curve_IP   "output inflexion points indices (1-Flist)"
        };
*/

#include "mw.h"
#include "mw-modules.h" 


#define _(a,i,j) ((a)->values[(i)*((a)->dim)+(j)])
/* if a is a Flist including elements constituted of dim components
   then _(a,i,j) is j-th component of i-th element */

#define EPS 1E-5F

static char Closed;
static int N_Points;

/* return sign of det(u,v) modulo EPS */
static int signdet(float ux, float uy, float vx, float vy)
{
  float aux;

  aux=ux*vy-uy*vx;
  if (aux>EPS) return 1;
  if (aux<(-EPS)) return -1;
  return 0;
}


/*------------------------------ MAIN MODULE ------------------------------*/

Flist km_inflexionpoints(Flist curve, Flist curve_IP)
{ 
  int i,sign1,sign2,iaux=0,signaux,iauxinit=0,signauxinit;
  float x1,y1,x2,y2,x3,y3;
  
  if (curve->dim!=2) 
    mwerror (FATAL,1,"error, curve->dim!=2\n");
  if ((curve_IP=mw_change_flist(curve_IP,curve->size,0,1))==NULL) 
    mwerror (FATAL,1,"error, not enough memory\n");
  N_Points=curve->size;
  if (N_Points<4) return curve_IP;
  Closed=((_(curve,0,0)==_(curve,N_Points-1,0))
	  &&(_(curve,0,1)==_(curve,N_Points-1,1)));

  /* signauxinit is the sign of the first non-zero vector product; 
     useful if there is an inflexion point at index 0 
    (for example if the curve is the result of module gass). 
    iauxinit is the corresponding index. */
  signauxinit=0; 

  /* signaux is the sign of the latest non-zero vector product; 
     useful if there is a piece of straight line in the curve. 
     iaux is the corresponding index */ 
  signaux=0;
  
  /* if the curve is closed */
  if (Closed) {
    x1=_(curve,N_Points-3,0); y1=_(curve,N_Points-3,1);
    x2=_(curve,N_Points-2,0); y2=_(curve,N_Points-2,1);
    x3=_(curve,0,0); y3=_(curve,0,1);
    sign1=signdet(x3-x2,y3-y2,x1-x2,y1-y2);
    x1=x2; y1=y2;
    x2=x3; y2=y3;
    x3=_(curve,1,0); y3=_(curve,1,1);
    sign2=signdet(x3-x2,y3-y2,x1-x2,y1-y2);
    if (sign1*sign2<0) _(curve_IP,curve_IP->size++,0)=0;
    if ((sign2==0)&&(sign1!=0)) {
      signaux=sign1; iaux=1; 
      signauxinit=signaux; iauxinit=iaux; 
    }
    sign1=sign2;
    x1=x2; y1=y2;
    x2=x3; y2=y3;
    x3=_(curve,2,0); y3=_(curve,2,1);
    sign2=signdet(x3-x2,y3-y2,x1-x2,y1-y2);
    if (sign1*sign2<0) _(curve_IP,curve_IP->size++,0)=1;
    if ((sign2==0)&&(sign1!=0)) {
      signaux=sign1; iaux=2;
      if (signauxinit==0) {
        signauxinit=signaux; iauxinit=iaux; 
      }
    } 
  }
  
  /* general case */
  x1=_(curve,0,0); y1=_(curve,0,1);
  x2=_(curve,1,0); y2=_(curve,1,1);
  x3=_(curve,2,0); y3=_(curve,2,1);
  sign1=signdet(x3-x2,y3-y2,x1-x2,y1-y2);
  for (i=3;i<N_Points;i++) {
    x1=x2; y1=y2;
    x2=x3; y2=y3;
    x3=_(curve,i,0); y3=_(curve,i,1);
    sign2=signdet(x3-x2,y3-y2,x1-x2,y1-y2);
    if (sign1*sign2<0) _(curve_IP,curve_IP->size++,0)=(float)(i-1);
    if ((sign2==0)&&(sign1!=0)) {
      signaux=sign1; iaux=i; 
      if (signauxinit==0) {signauxinit=signaux; iauxinit=iaux;}
    }
    if ((sign1==0)&&(sign2!=0)) {
      if ((sign2*signaux)<0) 
	{_(curve_IP,curve_IP->size++,0)=(float)((i-1+iaux)/2);}
    }
    sign1=sign2;
  }
  
  /* we verify if there is an inflexion point near index 0 
     (case of a closed curve) : check if the sign change */ 

  if ((Closed)&&((sign2*signauxinit<0)||(signaux*signauxinit<0))) { 
    iaux=((i-1+iauxinit+N_Points)/2)%N_Points; 
    if (iaux<N_Points/2) _(curve_IP,curve_IP->size++,0)=(float)N_Points-1; 
    else _(curve_IP,curve_IP->size++,0)=(float)iaux; 
  }  
  mwdebug("initial curve: %d  %d\n",N_Points,Closed);
  mwdebug("number of inflexion points: %d\n",curve_IP->size);
  mw_realloc_flist(curve_IP,curve_IP->size);

  return(curve_IP);
}



/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {ofdraw};
 author = {"Lionel Moisan"};
 version = {"1.1"};
 function = {"Draw optical flow data"};
 usage = {
   'a'->a          "do NOT draw arrows",
   'm':[m=0.]->m   "minimum norm threshold for drawing",
   'p':[p=8]->p    "grid step for drawing",
   'z':[z=2]->z    "zoom factor",
   'h':[h=20]->h   "height of a unit speed vector in pixels",
   U->U            "optical flow (input Fmovie of X coordinates)",
   V->V            "optical flow (input Fmovie of Y coordinates)",
   out<-ofdraw     "output Cmovie"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define M_PI 3.1415926535897931

extern double floor();

int dirx[8]={1,1,0,-1,-1,-1,0,1};
int diry[8]={0,1,1,1,0,-1,-1,-1};

void draw_arrow(u,x,y,angle,c)
     Cimage u;
     int x,y;
     double angle;
     unsigned char c;
{
  int n,i,j,dx,dy,a;

  n = u->ncol;
  a = ((int)floor(angle*4./M_PI+8.5))%8;
  dx = dirx[a];
  dy = diry[a];
  
  if (a%2)
    /* tilted arrow */
    for (i=0;i<=3;i++)
      for (j=0;j<=3-i;j++) 
	u->gray[(y-dy*j)*n+x-dx*i]=c;
  else if (dx==0)
    /* vertical arrow */
    for (i=0;i<=2;i++)
      for (j=-i;j<=i;j++) 
	u->gray[(y-dy*i)*n+x-j]=c;
  else 
    /* horizontal arrow */
    for (i=0;i<=2;i++)
      for (j=-i;j<=i;j++) 
	u->gray[(y-j)*n+x-dx*i]=c;
}

/*------------------------------ MAIN MODULE ------------------------------*/

Cmovie ofdraw(U,V,a,m,p,z,h)
     Fmovie U,V;
     float *m,*h;
     int *p,*z,*a;
{
  Cmovie out;
  Cimage new,prev,*next;
  Fimage X,Y;
  int ix,iy,nx,ny,fx,fy;
  float vx,vy;

  out = mw_new_cmovie();
  next = &(out->first);
  prev = NULL;

  for (X=U->first,Y=V->first;X&&Y;X=X->next,Y=Y->next) {
    nx = X->ncol; ny = X->nrow;
    new = mw_change_cimage(NULL,*z*ny,*z*nx);
    new->previous = prev;
    *next = prev = new;
    next = &(new->next);
    mw_clear_cimage(new,255);
    for (ix=*p;ix<nx;ix+=*p)
      for (iy=*p;iy<ny;iy+=*p) {
	vx = X->gray[iy*nx+ix];
	vy = Y->gray[iy*nx+ix];
	if (vx*vx+vy*vy>*m*(*m)) {
	  fx = *z*ix+(int)floor((double)(*h*vx+0.5));
	  fy = *z*iy+(int)floor((double)(*h*vy+0.5));
	  if (fx>=3 && fx<new->ncol-3 && fy>=3 && fy<new->nrow-3) {
	    mw_draw_cimage(new,*z*ix,*z*iy,fx,fy,0);
	    if (!a) draw_arrow(new,fx,fy,atan2((double)vy,(double)vx),0);
	  } 
	}
	new->gray[(*z*iy)*new->ncol+*z*ix] = 0;
      }
  }
  *next = NULL;

  return(out);
}


/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fzrt};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Zoom, Rotate then translate an image"};
   usage = {
    'o':[o=3]->o   "order: 0,1=linear,-3=cubic,3,5..11=spline, default 3",
    'p':[p=-.5]->p "Keys' parameter (when o=-3), in [-1,0], default -0.5",
    'b':[b=0.]->b  "background grey value, default: 0.0",
    in->in         "input Fimage",
    out<-out       "output Fimage",
    zoom->zoom     "zoom factor",
    angle->angle   "rotation angle (in degrees, counterclockwise)",
    x->x           "translation vector (x coordinate)",
    y->y           "translation vector (y coordinate)"
   };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

void rotate(cx,cy,ca,sa,x,y,nx,ny)
     double cx,cy,ca,sa,x,y;
     float *nx,*ny;
{
  *nx = (float)(cx + (x-cx)*ca-(y-cy)*sa);
  *ny = (float)(cy + (x-cx)*sa+(y-cy)*ca);
}

void fzrt(in,out,zoom,angle,x,y,o,p,b)
     Fimage in,out;
     float zoom,angle,x,y;
     int *o;
     float *p,*b;
{
  int nx,ny,sx,sy;
  float X1,Y1,X2,Y2,X3,Y3;
  double ca,sa,cx,cy;

  nx = in->ncol; ny = in->nrow;
  ca = cos((double)angle*M_PI/180.0);
  sa = sin((double)angle*M_PI/180.0);
  cx = 0.5*(double)nx;
  cy = 0.5*(double)ny;
  rotate(cx,cy,ca,sa,0.,0.,&X1,&Y1);
  rotate(cx,cy,ca,sa,cx*2.,0.,&X2,&Y2);
  rotate(cx,cy,ca,sa,0.,cy*2.,&X3,&Y3);
  x /= zoom; y /= zoom;
  X1 -= x; X2 -= x; X3 -= x; 
  Y1 -= y; Y2 -= y; Y3 -= y;
  sx = (int)ceil((double)zoom*(double)nx);
  sy = (int)ceil((double)zoom*(double)ny);
  fproj(in,out,&sx,&sy,b,o,p,NULL,X1,Y1,X2,Y2,X3,Y3,NULL,NULL);
}

/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fkplot};
   version = {"1.3"};
   author = {"Thierry Cohignac, Lionel Moisan"};
   function = {"Plot Fcurves on a Cimage"};
   usage = {            
     's'->s_flag      "shift Fcurves to fit exactly in the image",
     in->cs           "input Fcurves",
     out<-out         "output Cimage"
   };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

extern void fkbox();

#define BLACK 0
#define WHITE 255


Cimage fkplot(cs,out,s_flag)
Fcurves cs;
Cimage  out;
char *s_flag;
{
  Fcurve        c;
  Point_fcurve  p;
  int           nx,ny,x,y;
  float         xmax,ymax,xmin,ymin;
  
  fkbox(cs,&xmin,&ymin,&xmax,&ymax,NULL,NULL);
  
  if (!s_flag) {
    xmin=0.0;
    ymin=0.0;
  }
  nx = nint(xmax-xmin)+1;
  ny = nint(ymax-ymin)+1;
  out = mw_change_cimage(out,ny,nx);
  if (!out) mwerror(FATAL,1,"cannot allocate a %d x %d image\n",nx,ny);
  mw_clear_cimage(out,WHITE);
  
  for (c=cs->first; c; c=c->next) 
    for (p=c->first; p; p=p->next) {
      
      x = nint(p->x - xmin);
      y = nint(p->y - ymin);
      
      if (x>=0 && y>=0) out->gray[nx*y+x]=BLACK;
    }
  
  return out;  
}



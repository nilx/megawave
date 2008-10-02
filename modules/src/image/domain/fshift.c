/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fshift};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Integer periodic shift of an image"};
   usage = {
     'h'->h    "shift of half the image size",           
     'x':x->x  "or specify new x position of (0,0)",
     'y':y->y  "and new y position of (0,0)",
     'i'->i    "inverse shift",
     in->in    "input Fimage",
     out<-out  "output Fimage"
   };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): added allocation error check (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

Fimage fshift(in,out,x,y,h,i)
     Fimage in,out;
     int *x,*y,*h,*i;
{
  int xx,yy,nx,ny,dx,dy;

  if ( (h?2:0) + (x?1:0) + (y?1:0) != 2 ) 
    mwerror(FATAL,1,"Please select either -h or both -x/-y options.\n");

  nx = in->ncol;
  ny = in->nrow;
  dx = (i?-1:1)*(h?nx/2:*x); if (dx>0) dx -= (1+dx/nx)*nx;
  dy = (i?-1:1)*(h?ny/2:*y); if (dy>0) dy -= (1+dy/ny)*ny;
  
  out = mw_change_fimage(out,ny,nx);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");

  for (xx=0;xx<nx;xx++)
    for (yy=0;yy<ny;yy++) 
      out->gray[yy*nx+xx] = in->gray[((yy-dy)%ny)*nx+(xx-dx)%nx];

  return(out);
}

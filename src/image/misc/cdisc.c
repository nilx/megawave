/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {cdisc};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Draw a disc"};
  usage = {
  'x':x->x          "disc center (x coordinate)",
  'y':y->y          "disc center (y coordinate)",
  'r':r->r          "disc radius",
  out<-out          "output Cimage",
  nx->nx            "image size (x coordinate)",
  ny->ny            "image size (y coordinate)"
  };
*/

#include <stdio.h>
#include "mw.h"

Cimage cdisc(out,nx,ny,x,y,r)
     Cimage out;
     int nx,ny;
     float *x,*y;
     float *r;
{
  float cx,cy,rad2;
  int ix,iy;

  out = mw_change_cimage(out,ny,nx);
  if (!out) mwerror(FATAL,1,"Not enough memory");
  
  mw_clear_cimage(out,255);
  cx = (x?*x:0.5*(float)(nx-1));
  cy = (x?*x:0.5*(float)(ny-1));
  rad2 = (r?*r:.4*(float)(nx<ny?nx:ny));
  rad2 *= rad2;

  for (ix=0;ix<nx;ix++)
    for (iy=0;iy<ny;iy++) 
      if (((float)ix-cx)*((float)ix-cx)+((float)iy-cy)*((float)iy-cy)<=rad2)
	out->gray[iy*nx+ix] = 0.;
  
  return(out);
}

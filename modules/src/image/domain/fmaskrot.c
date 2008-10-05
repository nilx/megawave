/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fmaskrot};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"crop the central circular region of a Fimage"};
   usage = {  
    'b':[bg=0.0]->bg   "background grey value",
    's':[s=10.]->s     "linear transition support (in pixels)",
    in->in             "input Fimage",
    out<-out           "output Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

/* NB : calling this module with out=in is possible */

Fimage fmaskrot(in,out,bg,s)
     Fimage in,out;
     float *bg,*s;
{
  int nx,ny,x,y,adr;
  double cx,cy,r,d,dx,dy;

  nx = in->ncol;
  ny = in->nrow;

  out = mw_change_fimage(out,ny,nx);

  cx = 0.5*(double)(nx-1);
  cy = 0.5*(double)(ny-1);
  r = (cx<cy?cx:cy);

  for (x=nx;x--;) for (y=ny;y--;) {
    adr = y*nx+x;
    dx = x - cx;
    dy = y - cy;
    d = sqrt(dx * dx + dy * dy);
    if (d>=r) out->gray[adr] = *bg;
    else if (d<=r-*s) out->gray[adr] = in->gray[adr];
    else out->gray[adr] = (*bg*(*s-(float)(r-d)) + 
			   in->gray[adr]*(float)(r-d)) /(*s);
  }
  return(out);
}

/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
  name = {fconst};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Produce a constant Fimage"};
  usage = {
    out<-fconst   "output Fimage",
    g->g          "grey level value to set",
    x->x          "size of output image along x axis",
    y->y          "size of output image along y axis"
    };
*/
#include <stdio.h>
#include "mw.h"

Fimage fconst(g,x,y)
float g;
int x,y;
{
  Fimage out;
  
  out = mw_change_fimage(NULL,y,x);
  if (out==NULL) mwerror(FATAL,1,"Not enough memory.");
  mw_clear_fimage(out,g);

  return(out);
}




/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fpset};
 version = {"1.0"};
 author = {"Lionel Moisan"};
 function = {"Set a point value of a Fimage"};
 usage = {
    in->in        "input Fimage",
    x->x          "position (x)",
    y->y          "position (y)",
    g->g          "value to set",
    out<-fpset    "output Fimage (modified input)"
};
*/

#include "mw.h"

Fimage fpset(in,x,y,g)
     Fimage in;
     int x,y;
     float g;
{
  mw_plot_fimage(in,x,y,g);
  return(in);
}




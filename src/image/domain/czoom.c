/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
  name = {czoom};
  version = {"2.0"};
  author = {"Lionel Moisan"};
  function = {"Zoom of a char image"};
  usage = {
'x'->x_flg          "to zoom only in the x direction", 
'y'->y_flg          "to zoom only in the y direction",      
'X':[zoom=2.]->zoom "zoom factor (default 2.0)",
'o':[o=0]->o        "order: 0,1,-3,3,5,..11 (zoom) / 0..5 (unzoom) default 0",
'i'->i_flg          "apply inverse zooming",
in->in              "input Cimage",
out<-out            "output Cimage"
};
*/

#include <stdio.h>
#include "mw.h"

extern Fimage fzoom();

/* NB : calling this module with out=in is possible */

Cimage czoom(in,out,x_flg,y_flg,zoom,o,i_flg)
     Cimage in,out;
     char *x_flg,*y_flg,*i_flg;
     float *zoom;
     int *o;
{
  Fimage tmp;

  tmp = mw_cimage_to_fimage(in,NULL);
  fzoom(tmp,tmp,x_flg, y_flg,zoom,o,NULL,i_flg);
  out = mw_fimage_to_cimage(tmp,out);
  mw_delete_fimage(tmp);

  return(out);
}

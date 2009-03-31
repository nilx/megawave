/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {czoom};
  version = {"2.1"};
  author = {"Lionel Moisan"};
  function = {"Zoom of a char image"};
  usage = {
    'x'->x_flg           "to zoom only in the x direction",
    'y'->y_flg           "to zoom only in the y direction",
    'X':[zoom=2.]->zoom  "zoom factor",
    'o':[o=0]->o         "order: 0,1,-3,3,5,..11 (zoom) / 0..5 (unzoom)",
    'i'->i_flg           "apply inverse zooming",
    in->in               "input Cimage",
    out<-out             "output Cimage"
};
*/
/*----------------------------------------------------------------------
 v2.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for fzoom() */

/* NB : calling this module with out=in is possible */

Cimage czoom(Cimage in, Cimage out, char *x_flg, char *y_flg, float *zoom,
             int *o, char *i_flg)
{
    Fimage tmp;

    tmp = mw_cimage_to_fimage(in, NULL);
    fzoom(tmp, tmp, x_flg, y_flg, zoom, o, NULL, i_flg);
    out = mw_fimage_to_cimage(tmp, out);
    mw_delete_fimage(tmp);

    return (out);
}

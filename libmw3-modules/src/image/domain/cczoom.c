/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {cczoom};
  version = {"2.1"};
  author = {"Lionel Moisan"};
  function = {"Zoom of a color char image"};
  usage = {
    'x'->x_flg          "to zoom only in the x direction",
    'y'->y_flg          "to zoom only in the y direction",
    'X':[zoom=2.]->zoom "zoom factor",
    'o':[o=0]->o        "order: 0,1=linear,-3=cubic,3,5..11=spline",
    'i'->i_flg          "apply inverse zooming",
    in->in              "input Ccimage",
    out<-out            "output Ccimage"
};
*/
/*----------------------------------------------------------------------
 v2.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for czoom() */

/* NB : calling this module with out=in is possible */

Ccimage cczoom(Ccimage in, Ccimage out, char *x_flg, char *y_flg, float *zoom,
               int *o, char *i_flg)
{
    Cimage aux_in, aux_out;

    aux_in = mw_new_cimage();
    aux_in->nrow = in->nrow;
    aux_in->ncol = in->ncol;

    /* red component */
    aux_in->gray = in->red;
    aux_out = czoom(aux_in, NULL, x_flg, y_flg, zoom, o, i_flg);
    out = mw_change_ccimage(out, aux_out->nrow, aux_out->ncol);
    memcpy(out->red, aux_out->gray,
           out->nrow * out->ncol * sizeof(unsigned char));

    /* green component */
    aux_in->gray = in->green;
    aux_out = czoom(aux_in, aux_out, x_flg, y_flg, zoom, o, i_flg);
    memcpy(out->green, aux_out->gray,
           out->nrow * out->ncol * sizeof(unsigned char));

    /* blue component */
    aux_in->gray = in->blue;
    aux_out = czoom(aux_in, NULL, x_flg, y_flg, zoom, o, i_flg);
    memcpy(out->blue, aux_out->gray,
           out->nrow * out->ncol * sizeof(unsigned char));

    mw_delete_cimage(aux_out);
    free(aux_in);

    return (out);
}

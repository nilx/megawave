/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {fzoom};
  version = {"2.2"};
  author = {"Lionel Moisan"};
  function = {"Zoom of a floating point image"};
  usage = {
    'x'->x_flg          "to zoom only in the x direction", 
    'y'->y_flg          "to zoom only in the y direction",      
    'X':[zoom=2.]->zoom "zoom factor",
    'o':[o=0]->o        "order: 0,1,-3,3,5,..11 (zoom) / 0..5 (unzoom)",
    'p':p->p            "Keys' parameter (when o=-3), in [-1,0]",
    'i'->i_flg          "apply inverse zooming",
    in->in              "input Fimage",
    out<-out            "output Fimage"
};
*/
/*----------------------------------------------------------------------
 v2.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for fcrop(), funzoom() */

/* NB : calling this module with out=in is possible */

Fimage fzoom(Fimage in, Fimage out, char *x_flg, char *y_flg, float *zoom, int *o, float *p, char *i_flg)
{
  float nx,ny,sx,sy,zero;

  if (x_flg && y_flg) 
    mwerror(USAGE,0,"Options -x and -y are not compatible.");

  nx = (float)in->ncol; ny = (float)in->nrow;
  zero = 0.;

  if (i_flg) {

    /* inverse zooming */
    if (p || x_flg || y_flg) 
      mwerror(USAGE,0,"Options -p/-x/-y and -i are not compatible.");
    if (*o==-3) 
      mwerror(USAGE,0,
	 "No cubic Keys unzoom available. Use cubic spline (o=3) instead\n");
    if (*o<0 || *o>5) mwerror(FATAL,1,"unrecognized interpolation order.\n");
    out = funzoom(in,out,zoom,o,&zero,&zero);

  } else {

    /* regular zooming */
    sx = (y_flg?nx:*zoom*nx);
    sy = (x_flg?ny:*zoom*ny);
    out = fcrop(in,out,&sx,&sy,(float *)NULL,&zero,o,p,0.,0.,nx,ny);

  }

  return(out);
}



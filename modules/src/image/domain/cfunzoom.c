/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {cfunzoom};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Color Image reduction"};
   usage = {  
     'z':[z=2.0]->z     "unzoom factor",
     'x':tx->tx         "to first translate (x) the original image",
     'y':ty->ty         "to first transalte (y) the original image",
     'o':[o=0]->o       "spline space order, 0..5",
     in->in             "input Cfimage",
     out<-out           "output Cfimage"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mw.h"

extern Fimage funzoom();

Cfimage cfunzoom(in,out,z,o,tx,ty)
     Cfimage in,out;
     float *z,*tx,*ty;
     int *o;
{
  Fimage aux_in,aux_out;

  aux_in = mw_new_fimage(); 
  aux_in->nrow = in->nrow; 
  aux_in->ncol = in->ncol;

  /* red component */
  aux_in->gray = in->red;
  aux_out = funzoom(aux_in,NULL,z,o,tx,ty);
  out = mw_change_cfimage(out,aux_out->nrow,aux_out->ncol);
  memcpy(out->red,aux_out->gray,out->nrow*out->ncol*sizeof(float));

  /* green component */
  aux_in->gray = in->green;
  funzoom(aux_in,aux_out,z,o,tx,ty);
  memcpy(out->green,aux_out->gray,out->nrow*out->ncol*sizeof(float));

  /* blue component */
  aux_in->gray = in->blue;
  funzoom(aux_in,aux_out,z,o,tx,ty);
  memcpy(out->blue,aux_out->gray,out->nrow*out->ncol*sizeof(float));

  mw_delete_fimage(aux_out);
  free(aux_in); 
  return(out);
}

/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {binarize};
  version = {"1.02"};
  author = {"Lionel Moisan"};
  function = {"Binarize an image"};
  usage = {
            't':[t=128.0]->t   "threshold value (default:128.0)",
            'i'->i             "apply video inverse to output",
            input->in          "input Fimage",
            output<-out        "output Cimage (grey levels 0 and 255 only)"
          };
*/
/*-- MegaWave2 - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"
 
void binarize(in, out, t, i)
Fimage in;
Cimage out;
float  *t;
char   *i;
{
  int ptr;

  out = mw_change_cimage(out,in->nrow,in->ncol);
  if (i) 
    for (ptr=in->nrow*in->ncol;ptr--;) 
      *(out->gray+ptr) = (*(in->gray+ptr)>*t?0:255);
  else 
    for (ptr=in->nrow*in->ncol;ptr--;) 
      *(out->gray+ptr) = (*(in->gray+ptr)>*t?255:0);
}


/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {cmzoom};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Zoom of a char movie"};
  usage = {
  'x'->x_flg        "Zoom only in the X direction", 
  'y'->y_flg        "Zoom only in the Y direction",      
  'X':[factor=2.0]->factor    [0.01,100.0]  "Zoom factor (float value)",
  A->Input   "Input (could be a cmovie)",
  B<-Output  "Output (zoomed movie)"
};
*/
/*-- MegaWave2 - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

void cmzoom(Input, Output, x_flg, y_flg, factor)

char *x_flg, *y_flg;
float *factor;
Cmovie Input;
Cmovie Output;

{
  Cimage src,dst,prev;
  
  Output = mw_change_cmovie(Output);
  if (Output == NULL) mwerror(FATAL,1,"Not enough memory.\n");

  src = Input->first;
  prev = NULL;
  while (src) 
    {
      dst = mw_new_cimage();
      czoom(src,dst, x_flg, y_flg, factor);
      if (prev == NULL) 
	Output->first = dst;
      else 
	{
	  prev->next = dst;
	  dst->previous = prev;	
	}
      prev = dst;
      src = src->next;
    }
}

/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {ccmzoom};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Zoom of a color char movie"};
  usage = {
  'x'->x_flg        "Zoom only in the X direction", 
  'y'->y_flg        "Zoom only in the Y direction",      
  'X':[factor=2.0]->factor    [0.01,100.0]  "Zoom factor (float value)",
  A->Input   "Input (could be a ccmovie)",
  B<-Output  "Output (zoomed movie)"
};
*/
/*-- MegaWave2 - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

#ifdef __STDC__

extern void cczoom(Ccimage, Ccimage, char *, char *, float *);

#else

extern void cczoom();

#endif

void ccmzoom(Input, Output, x_flg, y_flg, factor)

char *x_flg, *y_flg;
float *factor;
Ccmovie Input;
Ccmovie Output;

{
  Ccimage src,dst,prev;
  
  Output = mw_change_ccmovie(Output);
  if (Output == NULL) mwerror(FATAL,1,"Not enough memory.\n");

  src = Input->first;
  prev = NULL;
  while (src) 
    {
      dst = mw_new_ccimage();
      cczoom(src,dst, x_flg, y_flg, factor);
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
  strcpy(Output->cmt,Input->cmt);
}

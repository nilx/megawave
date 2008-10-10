/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {cmzoom};
  version = {"1.2"};
  author = {"Jacques Froment, Lionel Moisan"};
  function = {"Zoom of a char movie"};
  usage = {
  'x'->x_flg                "Zoom only in the X direction", 
  'y'->y_flg                "Zoom only in the Y direction",      
  'X':[factor=2.0]->factor  "Zoom factor",
  'o':[o=0]->o              "order: 0,1=linear,-3=cubic,3,5..11=spline",
  'i'->i_flg                "apply inverse zooming",
  A->Input                  "Input (could be a cmovie)",
  B<-Output                 "Output (zoomed movie)"
};
*/
/*----------------------------------------------------------------------
 v1.1: added -o and -i options (L.Moisan)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for czoom() */

void cmzoom(Input, Output, x_flg, y_flg, factor, o, i_flg)

     char *x_flg, *y_flg, *i_flg;
     float *factor;
     int *o;
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
      dst = czoom(src, NULL, x_flg, y_flg, factor, o, i_flg);
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

/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {ccmzoom};
  version = {"1.2"};
  author = {"Jacques Froment, Lionel Moisan"};
  function = {"Zoom of a color char movie"};
  usage = {
  'x'->x_flg        "Zoom only in the X direction", 
  'y'->y_flg        "Zoom only in the Y direction",      
  'X':[factor=2.0]->factor    [0.01,100.0]  "Zoom factor",
  'o':[o=0]->o      "order: 0,1=linear,-3=cubic,3,5..11=spline",
  'i'->i_flg        "apply inverse zooming",
  A->Input          "Input (could be a ccmovie)",
  B<-Output         "Output (zoomed movie)"
};
*/
/*----------------------------------------------------------------------
 v1.1: added -o and -i options (L.Moisan)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "mw.h"
#include "mw-modules.h" /* for cczoom() */

void ccmzoom(Input, Output, x_flg, y_flg, factor, o, i_flg)
     
     char *x_flg, *y_flg, *i_flg;
     float *factor;
     int *o;
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
      dst = cczoom(src, NULL, x_flg, y_flg, factor, o, i_flg);
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

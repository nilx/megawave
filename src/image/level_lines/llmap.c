/* mwcommand
name = {llmap};
version={"1.6"};
author={"Jacques Froment, Frederic Guichard"};
function={"Map the level lines of an image"};
usage = {
's':[level_step=20]->ls [1,255] "gray level step (default:20)",
't'->tmap "code the type of border",
image -> input "image input",
level_lines <- output "level lines of input"
};
*/
/*----------------------------------------------------------------------
 v1.6: return result (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

/* Type of borders */
#define VOID 255
#define LEFT 200
#define UP   100
#define BOTH 0

Cimage llmap(ls,tmap,input, output)
     
short *ls;
char *tmap;
Cimage input;
Cimage output;

{
  register int l;
  register unsigned char *in,*out;
  int dx,dy,size,beg;
  unsigned char step,U,L,C;
  
  step = (unsigned char) *ls;
  dy= input->nrow;
  dx= input->ncol;
  size=dx*dy;
  
  output=mw_change_cimage(output,dy,dx);
  if (!output) mwerror(FATAL,1,"Not enough memory.\n");
  mw_clear_cimage(output,255);
  
  beg=dx;
  if (!tmap)
    { /* Every borders are coded as value 0 */
      for (l=beg, in=input->gray+beg, out = output->gray+beg; 
	   l<size; 
	   l++,in++,out++)
	if ( ((*(in-1)/step)!=(*in/step)) || ((*(in-dx)/step)!=(*in/step)) )
	  *out=0;
	else
	  *out=255;
    }
  else
    {
      for (l=beg, in=input->gray+beg, out = output->gray+beg; 
	   l<size; 
	   l++,in++,out++)
	{
	  C=*in/step;
	  L=*(in-1)/step;
	  U=*(in-dx)/step;
	  if ((L==C) && (U==C)) *out=VOID;
	  else
	    if ((L!=C) && (U!=C)) *out=BOTH;
	    else
	      if (L!=C) *out=LEFT;
	      else
		*out=UP;
	}
    }
  return(output);
}

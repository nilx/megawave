/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {make_cimage};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Make a very simple image (as a demo of image use for output)"};
  usage = {cimage_out<-Output "output image"};
*/

#include <stdio.h>
#include "mw.h"

void make_cimage(Output)

Cimage Output;

{
  int dx,dy,x,y;

  dx = dy = 256;
  Output = mw_change_cimage(Output,dy,dx);
  if (Output == NULL) mwerror(FATAL,1,"Not enough memory.");

  for (x=0;x<dx;x++) for(y=0;y<dy;y++) 
    Output->gray[dx*y+x] = x;
}




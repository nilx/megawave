/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {creadasc};
 version = {"1.0"};
 author = {"Jacques Froment"};
 function = {"Read an image in ascii format (char values)"};
 usage = {
    output<-u  "output cimage",
    dx->dx     "number of columns",
    dy->dy     "number of rows"
};
*/

#include <stdio.h>
#include "mw.h"
 
void creadasc(u,dx,dy)
     Cimage u;
     int dx,dy;
{
  register unsigned char *ptr;
  int i,c;
  
  u = mw_change_cimage(u,dy,dx); 
  if (!u) mwerror(FATAL,1,"Not enough memory\n");

  for (i=0, ptr=u->gray; i<dx*dy; i++, ptr++)
    {
      if (scanf("%d",&c) != 1) 
	mwerror(WARNING,1,"Size of input data less than given size (dx,dy)\n");
      *ptr = c;
    }
}

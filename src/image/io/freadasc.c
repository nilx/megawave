/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {freadasc};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Read an image in ascii format (float values)"};
  usage = {
    output<-u  "output fimage",
    dx->dx     "number of columns",
    dy->dy     "number of rows"
    };
*/
/*--- MegaWave - Copyright (C) 1992 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"
 
void freadasc(u,dx,dy)
Fimage u;
int dx,dy;
{
  register float *ptr;
  int i;
  float c;
  
  u = mw_change_fimage(u,dy,dx); 
  if (!u) mwerror(FATAL,1,"Not enough memory\n");

  for (i=0, ptr=u->gray; i<dx*dy; i++, ptr++)
    {
      if (scanf("%f",&c) != 1) 
	mwerror(WARNING,1,"Size of input data less than given size (dx,dy)\n");
      *ptr = c;
    }
}

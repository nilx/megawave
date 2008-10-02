/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {freadasc};
 version = {"2.0"};
 author = {"Jacques Froment, Said Ladjal"};
 function = {"Read an image in ascii format (float values)"};
 usage = {    
      output<-u  "output fimage",
    { dx->Dx     "number of columns",
      dy->Dy     "number of rows" }

};
*/
/*----------------------------------------------------------------------
 v2.0: optional dimensions can be read from standart input (S.Ladjal)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
 
void freadasc(u,Dx,Dy)
     Fimage u;
     int *Dx,*Dy;
{
  register float *ptr;
  int i;
  int dx,dy;
  float c;

  /* this part was added by Saïd */
  if (!Dx || !Dy) { /* read them from standart input */
    if (scanf("%f",&c) != 1) 
      mwerror(WARNING,1,"File too short\n");
    dy=(int)c;
    if (scanf("%f",&c) != 1) 
      mwerror(WARNING,1,"File too short\n");
    dx=(int)c;
  }
  else 
    if ((!Dx && Dy) || (Dx && !Dy)) 
      mwerror(FATAL,1,"you gave only dx or dy \n");
    else {
      dx=*Dx; 
      dy=*Dy;
    }
  /* end of Saïd's part */

  u = mw_change_fimage(u,dy,dx); 
  if (!u) mwerror(FATAL,1,"Not enough memory\n");

  for (i=0, ptr=u->gray; i<dx*dy; i++, ptr++)
    {
      if (scanf("%f",&c) != 1) 
	mwerror(WARNING,1,"Size of input data less than given size (dx,dy)\n");
      *ptr = c;
    }
}

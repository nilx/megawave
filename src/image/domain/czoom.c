/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
  name = {czoom};
  version = {"1.2"};
  author = {"Jacques Froment"};
  function = {"Zoom of a char image"};
  usage = {
  'x'->x_flg        "Zoom only in the X direction", 
  'y'->y_flg        "Zoom only in the Y direction",      
  'X':[factor=2.0]->factor    [0.01,100.0]  "Zoom factor (float value)",
  A->Input   "Input (could be a cimage)",
  B<-Output  "Output (zoomed image)"
};
*/
/*--- MegaWave2 - Copyright (C)1994 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>
#include <math.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

void czoom(Input, Output, x_flg, y_flg, factor)

char *x_flg, *y_flg;
float *factor;
Cimage Input;
Cimage Output;

{
  long l;
  short x,y,c,i,j;
  float s;
  short  dx,dy;      /* Size of Input */
  short  zx,zy;      /* Size of Output */
  short  cx,cy;      /* Zoom factor */
  char unzoom_flg;
  float xfact,yfact;
  register unsigned char *ptrI,*ptrO;
  int Factor;

  if (x_flg && y_flg) mwerror(USAGE, 0, "Options -x and -y are not compatible.");

  dx = Input->ncol;
  dy = Input->nrow;
  ptrI = Input->gray;

  if (*factor >= 1.0) 
    {unzoom_flg=0; Factor = *factor; s =(float) Factor - (*factor); } 
  else
    {unzoom_flg=1; Factor = 1.0/ (*factor); s = (float) Factor - 1.0 / (*factor);} 

  if (s == 0.0) 
    /* Zoom factor is an integer */
    { 
      mwdebug("Zoom factor is an integer\n");
      if (!y_flg) cx = Factor; else cx = 1;
      if (!x_flg) cy = Factor; else cy = 1;

      if (unzoom_flg == 0)  {   /* Zoom forward */

	zx = cx * dx;
	zy = cy * dy;

	Output = mw_change_cimage(Output, zy, zx); 
	if (Output == NULL) mwerror(FATAL,1,"Not enough memory.\n");
	ptrO = Output->gray;

	for (x=0;x<dx;x++) for(y=0;y<dy;y++) 
	  {
	    l = (long)zx*cy*y+cx*x;
	    c = ptrI[(long)dx*y+x];
	    for (i=0;i<cx;i++) for (j=0;j<cy;j++) ptrO[l+j*zx+i] = c;
	  }
      } else {  /* Zoom backward */

	zx = dx / cx; 
	zy = dy / cy;

	Output = mw_change_cimage(Output, zy, zx); 
	if (Output == NULL) mwerror(FATAL,1,"Not enough memory.\n");
	ptrO = Output->gray;
	
	for (x=0;x<dx-1;x+=cx) for(y=0;y<dy-1;y+=cy) {
	  l = (long)dx*y+x;
	  s = 0.0;
	  for (i=0;i<cx;i++) for (j=0;j<cy;j++) s += ptrI[l+j*dx+i];
	  ptrO[(long)zx*y/cy+x/cx] = (unsigned char) (s / (cx*cy));
	}
      }
    }
     else
       /* Zoom factor is not an integer */
 {
   mwdebug("Zoom factor is not an integer\n");
   if (!y_flg) xfact =  1.0/(*factor); else xfact = 1.0;
   if (!x_flg) yfact =  1.0/(*factor); else yfact = 1.0;
   zx = nint(dx / xfact);
   zy = nint(dy / yfact);

   Output = mw_change_cimage(Output, zy, zx); 
   if (Output == NULL) mwerror(FATAL,1,"Not enough memory.\n");
   ptrO = Output->gray;

   for (x=0;x<zx;x++) for(y=0;y<zy;y++) 
     ptrO[y*zx+x] = ptrI[ ((int)(yfact*y))*dx+((int)(xfact*x))];
 }
  mwdebug("Size of Output : zx = %d, zy = %d \n",zx,zy);
  strcpy(Output->cmt,Input->cmt);
}




/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
  name = {cczoom};
  version = {"1.1"};
  author = {"Jacques Froment"};
  function = {"Zoom of a color char image"};
  usage = {
  'x'->x_flg        "Zoom only in the X direction", 
  'y'->y_flg        "Zoom only in the Y direction",      
  'X':[factor=2.0]->factor    [0.01,100]  "Zoom factor (float value)",
  A->Input   "Input (could be a ccimage)",
  B<-Output  "Output (zoomed image)"
};
*/
/*--- MegaWave2 - Copyright (C)1994 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

void cczoom(Input, Output, x_flg, y_flg, factor)

char *x_flg, *y_flg;
float *factor;
Ccimage Input;
Ccimage Output;

{
  long l,l1,l2;
  short x,y,i,j;
  unsigned char r,g,b;
  float s;
  float rr,rg,rb;
  short  dx,dy;      /* Size of Input */
  short  zx,zy;      /* Size of Output */
  short  cx,cy;      /* Zoom factor */
  char unzoom_flg;
  float xfact,yfact;
  int Factor;

  if (x_flg && y_flg) mwerror(USAGE, 0, "Options -x and -y are not compatible.");

  dx = Input->ncol; 
  dy = Input->nrow;

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

	Output = mw_change_ccimage(Output, zy, zx); 
	if (Output == NULL) mwerror(FATAL,1,"Not enough memory.");

	for (x=0;x<dx;x++) for(y=0;y<dy;y++) 
	  {
	    l = (long)zx*cy*y+cx*x;
	    l1 = (long)dx*y+x;
	    r = Input->red[l1];
	    g = Input->green[l1];
	    b = Input->blue[l1];
	    for (i=0;i<cx;i++) for (j=0;j<cy;j++) 
	      {
		l2 = l+j*zx+i;
		Output->red[l2] = r;
		Output->green[l2] = g;
		Output->blue[l2] = b;
	      }
	  }
      } else {  /* Zoom backward */

	zx = dx / cx; 
	zy = dy / cy;

	Output = mw_change_ccimage(Output, zy, zx); 
	if (Output == NULL) mwerror(FATAL,1,"Not enough memory.");
	
	for (x=0;x<dx-1;x+=cx) for(y=0;y<dy-1;y+=cy) {
	  l = (long)dx*y+x;
	  rr = rg = rb = 0.0;
	  for (i=0;i<cx;i++) for (j=0;j<cy;j++) 
	    {
	      l1 = l+j*dx+i;
	      rr += Input->red[l1];
	      rg += Input->green[l1];
	      rb += Input->blue[l1];
	    }
	  l2 = (long)zx*y/cy+x/cx;
	  l = (long) cx*cy;
	  Output->red[l2] = (unsigned char) (rr / l);
	  Output->green[l2] = (unsigned char) (rg / l);
	  Output->blue[l2] = (unsigned char) (rb / l);
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

      Output = mw_change_ccimage(Output, zy, zx); 
      if (Output == NULL) mwerror(FATAL,1,"Not enough memory.\n");
      
      for (x=0;x<zx;x++) for(y=0;y<zy;y++) 
	{
	  l = y*zx+x;
	  l2 = ((int)(yfact*y))*dx+((int)(xfact*x));
	  Output->red[l] = Input->red[l2];
	  Output->green[l] = Input->green[l2];
	  Output->blue[l] = Input->blue[l2];
	}
    }
  mwdebug("Size of Output : zx = %d, zy = %d \n",zx,zy);
  strcpy(Output->cmt,Input->cmt);
}




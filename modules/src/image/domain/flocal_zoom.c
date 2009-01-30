/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {flocal_zoom};
  version = {"1.2"};
  author = {"Jacques Froment"};
  function = {"In-place local Zoom of a float image"};
  usage = {
    'x':[center_x=256]->X  "X coordinate for the center of the zoom array",
    'y':[center_y=256]->Y  "Y coordinate for the center of the zoom array",
    'W':[width=40]->W      "Width of the zoom array",
    'X':[factor=2]->factor [1,10] "Zoom factor",
    A->Input        "Input (could be a fimage)", 
    B<-flocal_zoom  "Output (zoomed image)"
};
*/
/*----------------------------------------------------------------------
 v1.2: fixed bug: values of x0 and y0 (*factor-1 term) (L.Moisan)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mw.h"

Fimage flocal_zoom(Fimage Input, int *X, int *Y, int *W, int *factor)
{
  float *square,c;
  register float *ptr;
  register int x,y,i,j;
  int  dx,dy;      /* Size of Input & Ouput */
  int  sx,sy;      /* Size of square */     
  int x0,x1,y0,y1;
  int D,Df,l;

  dx = Input->ncol; 
  dy = Input->nrow;
  
  D = *W/2;
  if (*X-D < 0) D=*X;
  if (*X+D >= dx) D=dx-*X-1;
  if (*Y-D < 0) D=*Y;
  if (*Y+D >= dy) D=dy-*Y+1;

  sx = 2*D+1; sy = sx;

  square = (float *) malloc(sx*sy*sizeof(float));
  if (square == NULL) mwerror(FATAL,1,"Not enough memory.\n");

  ptr = Input->gray;
  for (y=0;y<sy;y++) 
    memcpy(&square[sx*y],&ptr[*X-D+dx*(y+*Y-D)],sx*sizeof(float));

  Df = (*factor)*D;
  x0 = 0; x1 = sx-1;
  l = (*X-Df-(*factor-1))/ (*factor); 
  if (l < 0) x0 = -l;
  l = (dx+Df-*X)/ (*factor);
  if (l <= sx) x1 = l-1;

  y0 = 0; y1 = sy-1;
  l = (*Y-Df-(*factor-1))/ (*factor); 
  if (l < 0) y0 = -l;
  l = (dy+Df-*Y)/ (*factor);
  if (l <= sy) y1 = l-1;

  for (x=x0;x<=x1;x++) for(y=y0;y<=y1;y++) 
      {
	c = square[x+sx*y];
	l = *X+((*factor)*x)-Df + dx*(*Y+((*factor)*y)-Df);
	for (i=0;i<(*factor);i++) for (j=0;j<(*factor);j++) ptr[l+j*dx+i] = c;
      }

  free(square);
  return(Input);
}




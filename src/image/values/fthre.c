/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {fthre};
 author = {"Jacques Froment"};
 version = {"1.0"};
 function = {"Thresold the pixel's gray-levels of a fimage"};
 usage = {
 'n'->norm    "Normalize pixel values into [min,max]",
 'm':min->m0  "Force minimal pixel value",
 'M':max->m1  "Force maximal pixel value",
 fimage->A    "Input fimage", 
 result<-B    "Output image"
};
*/
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include  "mw.h"

void fthre(A,B,norm,m0,m1)

Fimage	A,B;
char *norm;
float *m0,*m1;

{
  register float *ptr;
  register int i;
  float min,max,a,b;
  
  if (!m0 && !m1) mwerror(USAGE,0,"At least min or max pixel value requested\n");
  if (norm && (!m0 || !m1)) mwerror(USAGE,0,"Normalization needs selection of [min,max] values\n");
  if (m0 && m1 && (*m1 <= *m0)) mwerror(USAGE,0,"Illegal values of [min,max]\n");

  if ((B = mw_change_fimage(B,A->nrow,A->ncol)) == NULL)
    mwerror(FATAL, 1, "Not enough memory !\n");  
  
  mw_copy_fimage(A,B);  /* Copy pixel values of A into B */

  if (norm)  /* Normalization */
    {
      min=1e20; max=-min;
      for (ptr=B->gray, i=0;  i < B->nrow*B->ncol; ptr++, i++) 
	{
	  if (*ptr < min) min=*ptr;
	  if (*ptr > max) max=*ptr;
	}
      if (fabs((double) max-min) <= 1e-20)
	mwerror(FATAL,1,"Cannot normalize: constant input image\n");
      a = (*m1-*m0)/(max-min);
      b = *m0 - a * min;
      for (ptr=B->gray, i=0;  i < B->nrow*B->ncol; ptr++, i++) 
	*ptr = a * *ptr + b;
    }

  /* Thresolding */
  if (m0) for (ptr=B->gray, i=0;  i < B->nrow*B->ncol; ptr++, i++)
    if (*ptr < *m0) *ptr=*m0;
  if (m1) for (ptr=B->gray, i=0;  i < B->nrow*B->ncol; ptr++, i++)
    if (*ptr > *m1) *ptr=*m1;
}

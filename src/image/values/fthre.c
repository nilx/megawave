/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {fthre};
 author = {"Jacques Froment, Lionel Moisan"};
 version = {"1.1"};
 function = {"Threshold/normalize the pixel's gray-levels of a fimage"};
 usage = {
 'n'->norm    "Normalize pixel values from actual to [min,max]",
 'N'->nimg    "Normalize pixel values from actual or [min,max] to [0,255]",
 'm':min->m0  "Force minimal pixel value",
 'M':max->m1  "Force maximal pixel value",
 fimage->A    "Input fimage", 
 result<-B    "Output image"
};
*/
/*----------------------------------------------------------------------
 v2.0: added -N option + some other changes (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include  "mw.h"

void fthre(A,B,norm,nimg,m0,m1)

Fimage	A,B;
char *norm,*nimg;
float *m0,*m1;

{
  register float *ptr;
  register int i;
  float min,max,dmin,dmax,a,b;
  
  if (!nimg && !m0 && !m1) 
    mwerror(USAGE,0,"At least min or max value is required\n");

  if (norm && (!m0 || !m1)) 
    mwerror(USAGE,0,"-n option requires [min,max] values\n");

  if (m0 && m1 && (*m1 < *m0)) 
    mwerror(USAGE,0,"Empty interval [min,max]\n");

  if ((B = mw_change_fimage(B,A->nrow,A->ncol)) == NULL)
    mwerror(FATAL, 1, "Not enough memory !\n");  
  
  /* Copy pixel values of A into B */
  mw_copy_fimage(A,B); 

  /* Threshold if necessary */
  if (!norm) {
    if (m0) 
      for (ptr=B->gray, i=0;  i < B->nrow*B->ncol; ptr++, i++)
	if (*ptr < *m0) *ptr=*m0;
    if (m1) 
      for (ptr=B->gray, i=0;  i < B->nrow*B->ncol; ptr++, i++)
	if (*ptr > *m1) *ptr=*m1;
  }

  /* Normalize if necessary */
  if (norm || nimg) {
    dmin = (nimg?0:*m0);
    dmax = (nimg?255:*m1);
    min=max=B->gray[0];
    for (ptr=B->gray, i=0;  i < B->nrow*B->ncol; ptr++, i++) {
      if (*ptr < min) min=*ptr;
      if (*ptr > max) max=*ptr;
    }
    if (max-min <= 1e-20) {
      mwerror(WARNING,0,"constant input image, normalized to mid value\n");
      a = 0.;
      b = 0.5*(dmin+dmax);
    } else {
      a = (dmax-dmin)/(max-min);
      b = dmin - a * min;
    }
    for (ptr=B->gray, i=0;  i < B->nrow*B->ncol; ptr++, i++) 
      *ptr = a * *ptr + b;
  }
}

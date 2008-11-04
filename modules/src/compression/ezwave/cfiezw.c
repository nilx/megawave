/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {cfiezw};
version = {"1.11"};
author = {"Jean-Pierre D'Ales"};
function = {"Decompress a color image compressed by EZW algorithm"};
usage = {
 'e':EdgeIR->Edge_Ri
      	"Impulse reponses of edge and preconditionning filters (orthogonal transform)",
 'b':ImpulseResponse2->Ri2
	"Impulse response of filter 2 (biorthogonal transform)",
 'w':WeightFac->WeightFac
        "Scaling factor for wavelet coefficients", 
 'c':Conv->Conv [0,2]
	"0 : convert from RGB to YUV", 

 Cimage->Compress     "Input string of codewords",
 ImpulseResponse->Ri  "Impulse response of inner filters", 
 QImage<-Output       "Output reconstructed image"

};
*/


#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fiezw(), cfchgchannels() */

/*--- Constants ---*/

#define RED 0
#define GREEN 1
#define BLUE 2

Fsignal      ORI1, ORI2;          /* Non normalized filters */



static void
INIT_RI(ri1, ri2)

Fsignal ri1, ri2;

{
  int i;

  ORI1 = mw_new_fsignal();
  if (mw_alloc_fsignal(ORI1, ri1->size) == NULL)
    mwerror(FATAL, 1, "Not enough memory for ri1!\n");

  ORI2 = mw_new_fsignal();
  if (mw_alloc_fsignal(ORI2, ri2->size) == NULL)
    mwerror(FATAL, 1, "Not enough memory for ri2!\n");

  for (i=0; i<ri1->size; i++)
    ORI1->values[i] = ri1->values[i];

  for (i=0; i<ri2->size; i++)
    ORI2->values[i] = ri2->values[i];

}



static void
REFRESH_FILTERS(ri1, ri2)

Fsignal ri1, ri2;

{
  int i;


  for (i=0; i<ri1->size; i++)
    ri1->values[i] = ORI1->values[i];

  for (i=0; i<ri2->size; i++)
    ri2->values[i] = ORI2->values[i];

}




static void
COPY_FIMAGE2CHANNEL(image, chimage, color)

Cfimage     image;		/* Output color image */
Fimage      chimage;            /* Original channel image */
int         color;              /* Index of channel */

{
  int             x, size;
  register float *ptr1, *ptr2;

  size = image->nrow * image->ncol;
  ptr2 = chimage->gray;
  if (color == RED)
    ptr1 = image->red;
  else
    if (color == GREEN)
      ptr1 = image->green;
    else
      ptr1 = image->blue;
  for (x = 0; x < size; x++, ptr1++, ptr2++)
    *ptr1 = *ptr2;

}






void
cfiezw(Edge_Ri, Ri2, WeightFac, Conv, Compress, Ri, Output)

	/*--- Computes the orthogonal wavelet transform of image `Image` ---*/

Fimage	    Edge_Ri;		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
Fsignal     Ri2;		/* Impulse response of the low pass filter */
				/* for synthesis */
float      *WeightFac;          /* Weighting factor for wavelet coeff. */
int        *Conv;		/* Conversion type */
Cimage      Compress;		/* Input compressed image */
Fsignal     Ri;			/* Impulse response of the low pass filter */
Cfimage     Output;	        /* Reconstructed image */

{
  Fimage      QChImage;         /* Reconstructed channel image */
  int         Norm;             /* Flag for normalisation 
				 * in color conversion */
  int         Inverse;          /* Flag for inversion in color conversion */

  QChImage = mw_new_fimage();
  
  if (Ri2) 
    INIT_RI(Ri, Ri2);

  /*--- Reconstruct red channel ---*/

  fiezw(Edge_Ri, Ri2, WeightFac, Compress, Ri, QChImage);
  Output = mw_change_cfimage(Output, QChImage->nrow, QChImage->ncol);
  if (Output == NULL)
    mwerror(FATAL, 1, "Not enough memory for quantized color image!\n");
  COPY_FIMAGE2CHANNEL(Output, QChImage, RED);
  
  /*--- Reconstruct green channel ---*/

  if (Ri2) 
    REFRESH_FILTERS(Ri, Ri2);

  fiezw(Edge_Ri, Ri2, WeightFac, Compress, Ri, QChImage);
  COPY_FIMAGE2CHANNEL(Output, QChImage, GREEN);
  
  /*--- Reconstruct blue channel ---*/

  if (Ri2) 
    REFRESH_FILTERS(Ri, Ri2);

  fiezw(Edge_Ri, Ri2, WeightFac, Compress, Ri, QChImage);
  COPY_FIMAGE2CHANNEL(Output, QChImage, BLUE);
  
  if (Conv)
    cfchgchannels(Conv, &Inverse, &Norm, Output, Output);

}

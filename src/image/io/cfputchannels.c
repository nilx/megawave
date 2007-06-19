/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {cfputchannels};
 version = {"1.0"};
 author = {"Jean-Pierre D'Ales"};
 function = {"Make a color float image from three float channel images"};
 usage = {
   Red_image->RImage    "Input red channel image",
   Green_image->GImage  "Input green channel image",
   Blue_image->BImage   "Input blue channel image",
   Cfimage<-Image       "Output cfimage"
};
*/

#include <stdio.h>
#include <math.h>
#include  "mw.h"


void cfputchannels(RImage, GImage, BImage, Image)

	/*--- Computes the orthogonal wavelet transform of image `Image` ---*/

Fimage      RImage, GImage, BImage;    /* Input channel images */
Cfimage     Image;		       /* Output color image */

{
  register float  *ptri, *ptrc;         /* Pointers to red, green and blue 
				    * channels in color and channel image */
  int         c, size;

  if ((RImage->ncol != GImage->ncol) || (RImage->ncol != BImage->ncol) || (RImage->nrow != GImage->nrow) || (RImage->nrow != BImage->nrow))
    mwerror(FATAL, 2, "Input channel images do not have the same dimensions!\n");

  /*--- Memory allocation for channel images ---*/

  Image = mw_change_cfimage(Image, RImage->nrow, RImage->ncol);
  if (Image == NULL)
    mwerror(FATAL, 1, "Not enough memory for color image!\n");

  size = Image->nrow * Image->ncol;

  /*--- Copy values to channels ---*/

  ptri = Image->red;
  ptrc = RImage->gray;
  for (c = 0; c < size; c++, ptri++, ptrc++)
    *ptri = *ptrc;

  ptri = Image->green;
  ptrc = GImage->gray;
  for (c = 0; c < size; c++, ptri++, ptrc++)
    *ptri = *ptrc;

  ptri = Image->blue;
  ptrc = BImage->gray;
  for (c = 0; c < size; c++, ptri++, ptrc++)
    *ptri = *ptrc;

}

/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {cfgetchannels};
 version = {"1.0"};
 author = {"Jean-Pierre D'Ales"};
 function = {"Extract the three float channel images from a color float image"};
 usage = {
   Cfimage->Image      "Input image", 
   Red_image<-RImage   "Output red channel image",
   Green_image<-GImage "Output green channel image",
   Blue_image<-BImage  "Output blue channel image"
};
*/

#include <stdio.h>
#include <math.h>
#include  "mw.h"

void cfgetchannels(Image, RImage, GImage, BImage)
     Cfimage     Image;		            /* Input color image */
     Fimage      RImage, GImage, BImage;    /* Output channel images */
     
{
  register float      *ptri, *ptrc;         /* Pointers to red, green and blue 
				    * channels in color and channel image */
  int         c, size;

  size = Image->nrow * Image->ncol;

  /*--- Memory allocation for channel images ---*/

  RImage = mw_change_fimage(RImage, Image->nrow, Image->ncol);
  if (RImage == NULL)
    mwerror(FATAL, 1, "Not enough memory for red channel image!\n");

  GImage = mw_change_fimage(GImage, Image->nrow, Image->ncol);
  if (GImage == NULL)
    mwerror(FATAL, 1, "Not enough memory for red channel image!\n");

  BImage = mw_change_fimage(BImage, Image->nrow, Image->ncol);
  if (BImage == NULL)
    mwerror(FATAL, 1, "Not enough memory for red channel image!\n");


  /*--- Copy values to channels ---*/

  ptri = Image->red;
  ptrc = RImage->gray;
  for (c = 0; c < size; c++, ptri++, ptrc++)
    *ptrc = *ptri;

  ptri = Image->green;
  ptrc = GImage->gray;
  for (c = 0; c < size; c++, ptri++, ptrc++)
    *ptrc = *ptri;

  ptri = Image->blue;
  ptrc = BImage->gray;
  for (c = 0; c < size; c++, ptri++, ptrc++)
    *ptrc = *ptri;

}

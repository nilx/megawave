/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fftrot};
   version = {"1.01"};
   author = {"Pascal Monasse"};
   function = {"Rotate then translate an image using Fourier interpolation"};
   usage = {

   'a':[a=0.0]->a  
       "rotation angle (in degrees, counterclockwise, default 0.0)",
   'x':[x=0.0]->x  
       "translation vector (x coordinate, default 0.0)",
   'y':[y=0.0]->y  
       "translation vector (y coordinate, default 0.0)",
   'o'->o_flag     
       "to take (0,0) as the rotation center (instead of the image center)",
   in->in              
       "input Fimage",
   out<-out            
       "output Fimage"
   };
   */
/*-- MegaWave 2- Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>
#include <math.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

extern double sin(),cos(),tan();

#define HORIZONTAL 0
#define VERTICAL 1

/* Perform a shear to the image. The two images can be the same */
void yr_shear(dAmountOfShear, pFloatImageInput, pFloatImageOutput, iAxis, fDelta,PleaseDoNotCenter)
double dAmountOfShear;
Fimage pFloatImageInput, pFloatImageOutput;
int iAxis;
float fDelta;
int PleaseDoNotCenter;
{
  int i, j, iSize, iHalfSize, iOtherSize, iHalfOtherSize;
  Fsignal pRealSignal, pImaginarySignal, pRealSignalTransformed, pImaginarySignalTransformed;
  double dTranslation;
  float fCos, fSin;
  float fReal;
  
  /* iSize is the size of the image (must be a power of 2) in the direction of the
     axis of shear. iOtherSize the size of the image in the orthogonal direction */
  if(iAxis == HORIZONTAL)
    {
      iHalfSize = (iSize = pFloatImageInput->ncol) >> 1;
      iHalfOtherSize = (iOtherSize = pFloatImageInput->nrow) >> 1;
    }
  else
    {
      iHalfSize = (iSize = pFloatImageInput->nrow) >> 1;
      iHalfOtherSize = (iOtherSize = pFloatImageInput->ncol) >> 1;
    }

  /* Create temporary signals to compute the fourier transform of a line (or a column)
     of the image */
  pRealSignal = mw_change_fsignal(0, iSize);
  if(pRealSignal == 0)
    mwerror(FATAL, 1, "fftrot --> not enough memory to allocate the real signal\n");
  pImaginarySignal = mw_change_fsignal(0, iSize);
  if(pImaginarySignal == 0)
    mwerror(FATAL, 1, "fftrot --> not enough memory to allocate the imaginary signal\n");
  pRealSignalTransformed = mw_new_fsignal();
  if(pRealSignalTransformed == 0)
    mwerror(FATAL, 1, "fftrot --> not enough memory to allocate the real transformed signal\n");
  pImaginarySignalTransformed = mw_new_fsignal();
  if(pImaginarySignalTransformed == 0)
    mwerror(FATAL, 1, "fftrot --> not enough memory to allocate the imaginary transformed signal\n");

  for(i = 0; i < iOtherSize; i++)
    {
      if(iAxis == HORIZONTAL)
	memcpy(pRealSignal->values, pFloatImageInput->gray + i * iSize, iSize * sizeof(float));
      else
	for(j = 0; j < iSize; j++)
	  pRealSignal->values[j] = pFloatImageOutput->gray[j * iOtherSize + i];
      memset(pImaginarySignal->values, 0, iSize * sizeof(float));

      /* Compute the FFT of the current line (or column) of the image */
      fft1d(pRealSignal, pImaginarySignal, pRealSignalTransformed, pImaginarySignalTransformed, 0);

      if (PleaseDoNotCenter) 
	dTranslation = - (i * dAmountOfShear + fDelta) * 2. * M_PI;
      else dTranslation = - ((i - iHalfOtherSize - .5) * dAmountOfShear + fDelta) * 2. * M_PI;

      for(j = 1; j < iHalfSize; j++)
	{
	  fCos = (float) cos(j * dTranslation / iSize);
	  fSin = (float) sin(j * dTranslation / iSize);
	  fReal = pRealSignalTransformed->values[j];
	  pRealSignalTransformed->values[j] = 
	    fCos * fReal - fSin * pImaginarySignalTransformed->values[j];
	  pImaginarySignalTransformed->values[j] =
	    fSin * fReal + fCos * pImaginarySignalTransformed->values[j];
	  pRealSignalTransformed->values[iSize - j] = pRealSignalTransformed->values[j];
	  pImaginarySignalTransformed->values[iSize - j] = -pImaginarySignalTransformed->values[j];
	}
      pRealSignalTransformed->values[iHalfSize] = cos(dTranslation * .5) * pRealSignalTransformed->values[iHalfSize] - sin(dTranslation * .5) * pImaginarySignalTransformed->values[iHalfSize];
      pImaginarySignalTransformed->values[iHalfSize] = 0.;

      /* Compute the inverse FFT of the current line (or column) */
      fft1d(pRealSignalTransformed, pImaginarySignalTransformed, pRealSignal, pImaginarySignal, (char*)1);
      if(iAxis == HORIZONTAL)
	memcpy(pFloatImageOutput->gray + i * iSize, pRealSignal->values, iSize * sizeof(float));
      else
	for(j = 0; j < iSize; j++)
	  pFloatImageOutput->gray[j * iOtherSize + i] = pRealSignal->values[j];
    }

  /* Delete the previously allocated temporary signals */
  mw_delete_fsignal(pImaginarySignalTransformed);
  mw_delete_fsignal(pRealSignalTransformed);
  mw_delete_fsignal(pImaginarySignal);
  mw_delete_fsignal(pRealSignal);
}

/*------------------------- MAIN MODULE ----------------------------*/

/* Perform a rotation of an image by the angle fTheta. 
   The center of rotation is the center of the image. 
   The angle is oriented in the trigonometric sense.
   It uses the 3-pass sinc interpolation algorithm of Yaroslavsky. */

void fftrot(in,out,a,x,y,o_flag)
float  *a, *x, *y;
char   *o_flag;
Fimage in, out;
{
  double angle; /* angle in radians */

  /* Express the angle of rotation in radians and take the opposite */
  angle = -(*a * M_PI / 180.);
  
  /* Create the output image */
  out = mw_change_fimage(out, in->nrow, in->ncol);
  if(out == 0)
    mwerror(FATAL,1,"fftrot --> not enough memory to allocate output image\n");

  /* The rotation is decomposed into three shears :
     two horizontal and one vertical */
  yr_shear(  tan(angle * .5), in,  out, HORIZONTAL, 0.,    o_flag);
  yr_shear(- sin(angle     ), out, out, VERTICAL,   -(*y), o_flag );
  yr_shear(  tan(angle * .5), out, out, HORIZONTAL, -(*x), o_flag );
}



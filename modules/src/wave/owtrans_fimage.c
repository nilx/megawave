/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {owtrans_fimage};
version = {"1.2"};
author = {"Jean-Pierre D'Ales"};
function = {"Generates a fimage for visualization of orthonormal wavelet decomposition"};
usage = {
 'r':[RecursNum=1]->NumRec    "Number of levels of wavelet transform",
 'c':[Contrast=1.0]->Contrast "Multiplicative facor for wavelet coefficients", 
 WavTrans->Wtrans             "Input wavelet transform", 
 VisuImage<-Output            "Visualization image"
	};
*/
/*----------------------------------------------------------------------
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include  "mw.h"



void owtrans_fimage(Wtrans, Output, NumRec, Contrast)
     
     Wtrans2d        Wtrans;	/* Wavelet transform (input) */
     Fimage          Output;	/* visualization image (output) */
     int            *NumRec;	/* Number de recursion (-j) */
     double         *Contrast;	/* Multiplicative constant for details */
     

{
  
  int             J;		/* Current level of decomposition */
  long            xd1, yd1;	/* Coordinates of the detail sub-image D1's
				 * upper left point at current level */
  long            xd2, yd2;	/* Coordinates of the detail sub-image D2's
				 * upper left point at current level */
  long            xd3, yd3;	/* Coordinates of the detail sub-image D3's
				 * upper left point at current level */
  long            lx, ly;	/* Size of the sub-image at current level */
  long            llx, ldx;
  long            dx, dy;	/* Size of the visualization image */
  long            l, c;	        /* Index for columns and lines in images */
  
  /*--- Computation of the size of vivisualization image ---*/
  
  dx = Wtrans->images[*NumRec][0]->ncol + *NumRec;
  dy = Wtrans->images[*NumRec][0]->nrow + *NumRec;
  for (J = *NumRec; J >= 1; J--)
    {
      dx += Wtrans->images[J][1]->ncol;
      dy += Wtrans->images[J][2]->nrow;
    }
  
  /*--- Memory allocation for inverse wavelet transform Output ---*/
  
  Output = mw_change_fimage(Output, dy, dx);
  if (Output == NULL)
    mwerror(FATAL, 1, "Allocation for output image refused!\n");
  
  
  lx = Wtrans->images[*NumRec][0]->ncol;
  ly = Wtrans->images[*NumRec][0]->nrow;
  
  xd1 = 0;
  yd1 = 0;
  xd2 = 0;
  yd2 = 0;
  xd3 = xd1;
  yd3 = yd2;
  
  /*--- Copy the low-level average sub-image ---*/
  
  llx = ldx = 0;
  for (l = 0; l < ly; l++)
    {
      for (c = 0; c < lx; c++)
	Output->gray[ldx + c] = Wtrans->images[*NumRec][0]->gray[llx + c];
      llx += lx;
      ldx += dx;
    }
  
  for (J = *NumRec; J >= 1; J--)
    {
      if (J < *NumRec)
	{
	  yd1 += 2 * ly - Wtrans->images[J][1]->nrow;
	  xd2 += 2 * lx - Wtrans->images[J][2]->ncol;
	}
      
      xd1 += lx + 1;
      xd3 = xd1;
      yd2 += ly + 1;
      yd3 = yd2;
      
      lx = Wtrans->images[J][0]->ncol;
      ly = Wtrans->images[J][0]->nrow;
      
      /*--- Multiply the gray-levels of details sub-images at level J ---*/
      /*--- by a constant ---*/
      
      llx = 0;
      ldx = yd1 * dx;
      for (l = 0; l < ly; l++)
	{
	  for (c = 0; c < lx; c++)
	    Output->gray[ldx + xd1 + c] = fabs(Wtrans->images[J][1]->gray[llx + c]) * *Contrast;
	  llx += lx;
	  ldx += dx;
	}
      
      llx = 0;
      ldx = yd2 * dx;
      for (l = 0; l < ly; l++)
	{
	  for (c = 0; c < lx; c++)
	    Output->gray[ldx + xd2 + c] = fabs(Wtrans->images[J][2]->gray[llx + c]) * *Contrast;
	  llx += lx;
	  ldx += dx;
	}
      
      llx = 0;
      ldx = yd3 * dx;
      for (l = 0; l < ly; l++)
	{
	  for (c = 0; c < lx; c++)
	    Output->gray[ldx + xd3 + c] = fabs(Wtrans->images[J][3]->gray[llx + c]) * *Contrast;
	  llx += lx;
	  ldx += dx;
	}
      
      /*--- Trace white lines between sub-images ---*/
      
      for (l = 0; l < yd2 + ly; l++)
	Output->gray[l * dx + xd1 - 1] = 255.0;
      for (c = 0; c < xd1 + lx; c++)
	Output->gray[(yd2 - 1) * dx + c] = 255.0;
      
      for (l = yd2; l < yd2 + ly; l++)
	for (c = 0; c < xd2; c++)
	  Output->gray[l * dx + c] = 255.0;
      for (c = xd1; c < xd1 + lx; c++)
	for (l = 0; l < yd1; l++)
	  Output->gray[l * dx + c] = 255.0;
      
    }
  
}

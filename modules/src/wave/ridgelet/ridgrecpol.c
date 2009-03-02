/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {ridgrecpol};
 version = {"1.0"};
 author = {"Guillaume Charpiat, Vincent Feuvrier"};
 function = {"Get the polar frequency grid used to perform ridgelet decomposition"};
 usage = {
     'I':input_im->in_im    "imaginary input of the 2D-FFT image",
     'R':output_re<-out_re  "real output (2D-FFT image in polar grid)",
     'C':output_im<-out_im  "imaginary output (2D-FFT image in polar grid)",
     input_re->in_re        "real input of the 2D-FFT image"
};
*/
/*----------------------------------------------------------------------
 v1.0: initial revision before publication (J.Froment)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

#define near(x) ((((x)-floor(x))>.5)?(ceil(x)):(floor(x)))


/* NB :  
 * The input Fimages must be square-shaped. 
 * If the size of the image (its width or length) is n, the size of the output
   (Fimage) will be n * 2n;
 * calling this module with in_im=NULL is possible and means 
   in_im(x,y) = 0 everywhere; 
 * calling this module with out_re=NULL or out_im=NULL 
   is possible (if you are not interested on one of these outputs).
*/


void ridgrecpol(Fimage in_re, Fimage in_im, Fimage out_re, Fimage out_im)
{
  int     indic1, indic2, j;
  long n,i,j1;
  float x, y;
  int x1, y1;

  if (out_re == NULL) indic1 = 0; else indic1 = 1;
  if (out_im == NULL) indic2 = 0; else indic2 = 1;

  if ((indic1 == 0) && (indic2 == 0)) mwerror(USAGE,1, "At least one output needed\n");
  if (in_re == NULL) mwerror(USAGE,1, "Input is empty...\n");
  
  n = in_re->nrow;

  if (!(n == in_re->ncol)) mwerror(USAGE,1, "The image is not square-shaped");

  /***** Allocate input imaginery image (black) if necessary *****/

  if (in_im == NULL) { 
    in_im = mw_change_fimage(in_im, n, n); 
    if (in_im == NULL) mwerror(FATAL,1, "Not enough memory");
    mw_clear_fimage(in_im, 0.);
  }
  
  /******  Allocate output ******/

  if (indic1 == 1) {
    out_re = mw_change_fimage(out_re, 2*n, n);
    if (out_re == NULL)  mwerror(FATAL,1, "Not enough memory");
  }

  if (indic2 == 1) {
    out_im = mw_change_fimage(out_im, 2*n, n);
    if (out_im == NULL)  mwerror(FATAL,1, "Not enough memory");
  }

  /*** Convert rectangular to polar coordinates ***/
  /** i : ieme ligne de la transformee de Radon **/
  /** j : jieme colonne de la ieme ligne **/

  for (i=0; i<(n+1); i++) {
    for (j=0; j<(n+1); j++) {
            
      x = j-1./2;
      y = i + j*(1-2.*i/n)-1./2;
      
      x1 = near(x);
      y1 = near(y);

      if (x1 < 0) x1 = 0;
      if (x1 > (n-1)) x1 = n-1;
      if (y1 < 0) y1 = 0;
      if (y1 > (n-1)) y1 = n-1;

      j1 = j;
      if (j1 > (n/2)) j1--;

      if (indic1 == 1) out_re->gray[j1+n*i] = in_re->gray[x1+n*y1];
      if (indic2 == 1) out_im->gray[j1+n*i] = in_im->gray[x1+n*y1];
      
    }
  }
  
  for (i=1; i<n; i++) {
    for (j=0; j<(n+1); j++) {
            

      x = i+j*(1-2.*i/n)-1./2;
      y = n - j - 1./2;
      
      x1 = near(x);
      y1 = near(y);

      if (x1 < 0) x1 = 0;
      if (x1 > (n-1)) x1 = n-1;
      if (y1 < 0) y1 = 0;
      if (y1 > (n-1)) y1 = n-1;

      j1 = j;
      if (j1 > (n/2)) j1--;

      if (indic1 == 1) out_re->gray[j1+n*i+n*n] = in_re->gray[x1+n*y1];
      if (indic2 == 1) out_im->gray[j1+n*i+n*n] = in_im->gray[x1+n*y1];
      
    }
  }
  
}


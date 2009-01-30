/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {ridgpolrec};
   version = {"1.0"};
   author = {"Guillaume Charpiat, Vincent Feuvrier"};
   function = {"Get the rectangular frequency grid used to perform ridgelet reconstruction"};
   usage = {
      'R':output_re<-out_re  "real output (2D-FFT image in rect. grid)",
      'I':output_im<-out_im  "imaginary output (2D-FFT image in rect. grid)",
      'C':input_im->in_im    "imaginary input (2D-FFT image in polar grid)",
      input_re->in_re        "real input (2D-FFT image in polar grid)"
   };
*/
/*----------------------------------------------------------------------
 v1.0: initial revision before publication (J.Froment)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define near(x) ((int)((((x)-floor(x))>.5)?(ceil(x)):(floor(x))))

/* NB :  
 * The input Fimages must be 2n * n;
 * The output Fimages will be square-shaped. 
 * calling this module with in_im=NULL is possible and means 
   in_im = 0 everywhere; 
 * calling this module with out_re=NULL is not possible; 
   but you can call this module with out_im=NULL 
   (if you are not interested on this output).
*/


void ridgpolrec(Fimage out_re, Fimage out_im, Fimage in_re, Fimage in_im)
{
  Fimage A;
  int indic,indic3;
  long   i, n, j;
  int j1;
  float x, y;
  long x1, y1;
  

  if (out_re == NULL) indic3 = 0; else indic3 = 1;
  if (out_im == NULL) indic = 0; else indic = 1;

  if ((indic == 0) && (indic3 == 0)) mwerror(USAGE,1, "No output specified");

  if (in_re == NULL) mwerror(USAGE,1, "Input is empty...\n");
  
  n = in_re->ncol;

  /**** Allocate A ****/

  A = mw_new_fimage();
  A = mw_change_fimage(A, n, n);

  if (A == NULL) mwerror(FATAL,1, "Not enough memory");

  mw_clear_fimage(A,0.);

  /**** Allocate output ****/

  if (indic3 == 1) {
    out_re = mw_change_fimage(out_re, n, n);
    if (out_re == NULL)  mwerror(FATAL,1, "Not enough memory");
    mw_clear_fimage(out_re, 0.);
  }

  if (indic == 1) {
    out_im = mw_change_fimage(out_im, n, n);
    if (out_im == NULL)  mwerror(FATAL,1, "Not enough memory");
    mw_clear_fimage(out_im, 0.);
 }
  
  /***** Allocate input imaginery image (black) if necessary *****/

  if ((in_im == NULL) && (indic == 1)) { 
    indic = 0;
    mw_clear_fimage(out_im, 0.);
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
      if (!(j1 == (n/2))) { 
	if (j1 > (n/2)) j1--;    

	(A->gray[x1+n*y1])++;

	if (indic3 == 1) 

	   
	    out_re->gray[x1+n*y1] = (((A->gray[x1+n*y1])-1)*(out_re->gray[x1+n*y1]) + in_re->gray[j1+n*i])/(A->gray[x1+n*y1]);


	if (indic == 1) out_im->gray[x1+n*y1] = ((A->gray[x1+n*y1]-1)*(out_im->gray[x1+n*y1]) + in_im->gray[j1+n*i])/(A->gray[x1+n*y1]);
      }
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
      if (!(j1 == (n/2))) { 
	if (j1 > (n/2)) j1--;
	(A->gray[x1+n*y1])++;
	if (indic3 == 1) out_re->gray[x1+n*y1] = ((A->gray[x1+n*y1]-1)*(out_re->gray[x1+n*y1]) + in_re->gray[j1 + n*i + n*n])/(A->gray[x1+n*y1]);
	if (indic == 1) out_im->gray[x1+n*y1] = ((A->gray[x1+n*y1]-1)*(out_im->gray[x1+n*y1]) + in_im->gray[j1 + n*i + n*n])/(A->gray[x1+n*y1]);
      }
    }
  }
    
  mw_delete_fimage(A);
  A = NULL;
}


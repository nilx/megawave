/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fft2d};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Rectangular 2D Fast Fourier Transform"};
   usage = {
   'i'->i_flag     "to compute inverse FFT",
   'I':input_im->in_im       "imaginary input (Fimage)",
   'A':output_re<-out_re     "real output (Fimage)",
   'B':output_im<-out_im     "imaginary output (Fimage)",
    input_re->in_re "real input (Fimage)"
   };
   */
/*-- MegaWave - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>
#include "mw.h"

extern void   fft1d();
extern void   fshrink2();

/* NB : 
     * calling this module with in_im=NULL is possible and means 
       in_im(x,y) = 0 everywhere 
     * calling this module with out_re=NULL or out_im=NULL 
       is possible (if you are not interested on one of these outputs)
*/

void fft2d(in_re, in_im, out_re, out_im, i_flag)
Fimage  in_re, in_im, out_re, out_im;
char    *i_flag;
{
    int      i,j,n,p,n0,p0;
    Fsignal  f1_re,f1_im,f2_re,f2_im;
    Fimage   tmp_re,tmp_im,dre_flag,dim_flag;

    if ((!out_re) && (!out_im)) mwerror(USAGE,1,
					"At least one output needed\n");

    /***** allocate output images if necessary *****/
    dre_flag=out_re;
    if (dre_flag == NULL) out_re = mw_new_fimage();
    dim_flag=out_im;
    if (dim_flag == NULL) out_im = mw_new_fimage();

    /***** reduce original images to proper 2^p x 2^n dimensions *****/
    /*****      reduced images are then in out_re and out_im     *****/  

    n0 = in_re->nrow;
    p0 = in_re->ncol;
    fshrink2(in_re,out_re);
    n = out_re->nrow;
    p = out_re->ncol;

    if (in_im) {
	fshrink2(in_im,out_im); 
	if (out_im->nrow!=n || out_im->ncol!=p) 
	  mwerror(FATAL,1,"Input images have no compatible sizes");
    } else {
	out_im = mw_change_fimage(out_im,n,p);
	mw_clear_fimage(out_im,0.0);
    }

    if (n!=n0 || p!=p0) 
      mwerror(WARNING,0,"%dx%d input image has been shrinked to %dx%d\n",
	      p0,n0,p,n);

    /***** allocations *****/
    tmp_re = mw_change_fimage(NULL,n,p);
    tmp_im = mw_change_fimage(NULL,n,p);
    if (!tmp_re || !tmp_im) mwerror(FATAL,1,"Not enough memory.");

    /***** FFT 1D sur les lignes *****/
    f1_re = mw_change_fsignal(NULL,p);
    f1_im = mw_change_fsignal(NULL,p);
    f2_re = mw_change_fsignal(NULL,p);
    f2_im = mw_change_fsignal(NULL,p);
    if (!f1_re || !f1_im || !f2_re || !f2_im) 
      mwerror(FATAL,1,"Not enough memory.");

    for (i=0;i<n;i++) {
      for (j=0;j<p;j++) {
	f1_re->values[j] = out_re->gray[p*i+j];
	f1_im->values[j] = out_im->gray[p*i+j];
      }
      fft1d(f1_re,f1_im,f2_re,f2_im,i_flag);
      for (j=0;j<p;j++) {
	tmp_re->gray[p*i+j] = f2_re->values[j];
	tmp_im->gray[p*i+j] = f2_im->values[j]; 
      }
    }

   /***** FFT 1D sur les colones *****/
    f1_re = mw_change_fsignal(f1_re,n);
    f1_im = mw_change_fsignal(f1_im,n);
    f2_re = mw_change_fsignal(f2_re,n);
    f2_im = mw_change_fsignal(f2_im,n);
    for (j=0;j<p;j++) {
      for (i=0;i<n;i++) {
	f1_re->values[i] = tmp_re->gray[p*i+j];
	f1_im->values[i] = tmp_im->gray[p*i+j];
      }
      fft1d(f1_re,f1_im,f2_re,f2_im,i_flag);
      for (i=0;i<n;i++) {
	out_re->gray[p*i+j] = f2_re->values[i];
	out_im->gray[p*i+j] = f2_im->values[i]; 
      }
    }	

    /***** free signals *****/
    mw_delete_fsignal(f2_re);
    mw_delete_fsignal(f2_im);
    mw_delete_fsignal(f1_im);
    mw_delete_fsignal(f1_re);
    mw_delete_fimage(tmp_im);
    mw_delete_fimage(tmp_re);

    /***** free output images if necessary *****/
    if (dre_flag == NULL) mw_delete_fimage(out_re);
    if (dim_flag == NULL) mw_delete_fimage(out_im);
}


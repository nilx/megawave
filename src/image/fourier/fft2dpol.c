/*--------------------------- MegaWave2 Command -----------------------------*/
/* mwcommand
   name = {fft2dpol};
   version = {"1.2"};
   author = {"Lionel Moisan"};
   function = {"Polar 2D FFT"};
   usage = {
       'i'->i_flag                  "to compute inverse FFT",
       'I':input_im->in_im          "imaginary input (Fimage)",
       'M':output_rho<-out_rho      "modulus output (Fimage)",
       'P':output_theta<-out_theta  "phase output (Fimage) in [-M_PI,M_PI]",
       input_re->in_re              "real input (Fimage)"
   };
*/
/*----------------------------------------------------------------------
 v1.2: corrected missing i_flag bug (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

/* NB : as for fft2d : 
     * calling this module with in_im=NULL is possible and means 
       in_im(x,y) = 0 everywhere 
     * calling this module with out_rho=NULL or out_theta=NULL 
       is possible (if you are not interested on one of these outputs)
*/

/* for lisibility */
#define out_re out_rho
#define out_im out_theta

void fft2dpol(in_re, in_im, out_rho, out_theta, i_flag)
     Fimage in_re, in_im, out_rho, out_theta;
     char *i_flag;
{
  int    i;
  float  rho,theta;
  Fimage drho_flag,dtheta_flag;
  
  if ((!out_re) && (!out_im)) mwerror(USAGE,1,
				      "At least one output needed\n");
  
  /***** allocate output images if necessary *****/
  drho_flag = out_rho;
  if (drho_flag == NULL) out_rho = mw_new_fimage();
  dtheta_flag = out_theta;
  if (dtheta_flag == NULL) out_theta = mw_new_fimage();    
  if (!out_re || !out_im) mwerror(FATAL,1,"Not enough memory.");
  
  /*** Compute cartesian FFT ***/
  fft2d(in_re,in_im,out_re,out_im,i_flag);
  
  /*** Convert result to polar coordinates ***/
  for (i=out_rho->nrow*out_rho->ncol; i-- ; ) {
    rho   = (float)hypot((double)out_re->gray[i],(double)out_im->gray[i]);
    theta = (float)atan2((double)out_im->gray[i],(double)out_re->gray[i]);
    out_rho->gray[i]   = rho;
    out_theta->gray[i] = theta;
  }
  
  /***** free output images if necessary *****/
  if (drho_flag == NULL) mw_delete_fimage(out_rho);
  if (dtheta_flag == NULL) mw_delete_fimage(out_theta);
}

/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {smse};
author = {"Jean-Pierre D'Ales, Jacques Froment"};
function = {"Computes the mean square error between two fsignals"};
version = {"1.02"};
usage = {
 'n'->Norm      "flag to normalize the signals",
 Signal1->Sig1  "original signal", 
 Signal2->Sig2  "reconstructed signal",
 SNR<-SNR       "signal to noise ratio / `Sig1` (SNR)",
 PSNR<-PSNR     "peak signal to noise ratio / `Sig1` (PSNR)",
 MSE<-MSE       "mean square error between Sig1 and Sig2 (MSE)",
 MRD<-MRD       "maximal relative difference (MRD)"
};
 */

/*----------------------------------------------------------------------
 v1.02:  (JF) Bug on min and max initialization corrected (JF)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>

#include  "mw.h"


static void NORM_SIG(signal)
     
     /*--- Normalize `signal` to 0.0 mean and 1.0 variance ---*/
     
     Fsignal	signal;
     
{
  long	i;		/* Index of current point in `signal` */
  double	mean, var;	/* Mean and variance of `signal` */
  
  mean = 0.0;
  for(i = 0; i < signal->size; i++)
    mean += signal->values[i];
  mean /= (double) signal->size;
  for(i = 0; i < signal->size; i++)
    signal->values[i] -= mean;
  
  var = 0.0;
  for(i = 0; i < signal->size; i++)
    var += signal->values[i] * signal->values[i];
  var = sqrt(((double) signal->size - 1.0) / var);
  for(i = 0; i < signal->size; i++)
    signal->values[i] *= var;
  
}



void smse(Sig1, Sig2, Norm, SNR, PSNR, MSE, MRD)
     
     /*--- Computes the mean square error between Sig1 and Sig2 ---*/
     
     Fsignal	Sig1, Sig2;	/* Input signals */
     int        *Norm;          /* Normalisation to 0 mean and 1.0 variance */
     double	*SNR;		/* Signal to noise ratio / `Sig1` */
     double	*PSNR;		/* Peak signal to noise ratio / `Sig1` */
     double	*MSE;		/* Mean square error between Sig1 and Sig2 */
     double	*MRD;		/* Maximal relative difference */	
     
{
  long	        i;              /* Index of current point in `Sig1`, `Sig2` */
  double	diff;		/* Difference between two values */
  double	min1, max1;	/* Minimum and maximum of `Sig1` values */
  double	min, max;	/* Minimum and maximum of `Sig1` and `Sig2` 
				 * values */
  double	mean1;		/* Mean value of `Sig1` */
  double	var1;		/* Empirical variance of `Sig1` */
  double	DMAX;		/* Absolute value of the maximum difference
				 * between values of `Sig1` and `Sig2` */

  /*--- Verification of signals' sizes ---*/
  
  if(Sig1->size != Sig2->size)
    mwerror(FATAL, 1, "Signal1 and Signal2 have not the same size!\n");
  
  
  /*--- Normalisation of signals (if selected) ---*/
  
  if(Norm)
    {
      NORM_SIG(Sig1);
      NORM_SIG(Sig2);
    }
  
  /*--- Computation of minimum and maximum values in `Sig1` ---*/
  
  min1 = 1e30; min=min1;
  max1 = -min1; max=max1;
  
  for(i = 0; i < Sig1->size; i++)
    {
      if(Sig1->values[i] < min1)
	{
	  min1 = Sig1->values[i];
	  if(Sig1->values[i] < min)
	    min = min1;
	}
      if(Sig1->values[i] > max1)
	{
	  max1 = Sig1->values[i];
	  if(Sig1->values[i] > max)
	    max = max1;
	}
      if(Sig2->values[i] < min)
	min = Sig2->values[i];
      if(Sig2->values[i] > max)
	max = Sig2->values[i];
    }
  
  /*--- Computation of variance of `Sig1` ---*/
  
  mean1 = 0.0;
  if (!Norm)
    {
      for(i = 0; i < Sig1->size; i++)
	mean1 += Sig1->values[i];
      mean1 /= (double) Sig1->size;
    }
  
  var1 = 0.0;
  for(i = 0; i < Sig1->size; i++)
    var1 += (Sig1->values[i] - mean1) * (Sig1->values[i] - mean1);
  var1 /= ((double) Sig1->size - 1.0);
  
  /*--- Computation of m.s.e. and s.n.r. ---*/
  
  DMAX = 0.0;
  *MSE = 0.0;
  for(i = 0; i < Sig1->size; i++)
    {
      diff = fabs((double) Sig1->values[i] - Sig2->values[i]);
      if(diff > DMAX)
	DMAX = diff;
      *MSE += diff * diff;
    }
  
  *MRD = 100.0 * DMAX / (max - min);
  *MSE /= (double) Sig1->size;
  *SNR = 10.0 * log10(var1/ (*MSE));
  *PSNR = 10.0 * log10 ((max1 - min1) * (max1 - min1) / (*MSE));
  
  /*--- Printing of results ---*/
  
  mwdebug("-> Maximal relative difference = %3.1lg\n",*MRD);
  mwdebug("-> Peak signal to noise ratio : PSNR = %lg db\n", *PSNR);
  mwdebug("-> Signal to noise ratio : SNR = %lg db\n", *SNR);
  mwdebug("-> Mean square error : MSE = %lg\n",*MSE);          
  
}

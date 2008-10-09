/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
  name = {iridgelet};
  version = {"1.0"};
  author = {"Claire Jonchery, Amandine Robin"};
  function ={"inverse ridgelet transform : reconstruct an image from its ridgelets coefficients"};
 usage = {
 'I':in_im->in_im   "imaginary input (imaginary ridgelets coefficients 2N*2N)",
 'C':out_im<-out_im "imaginary output (Fimage N*N)",
 np->np             "resolution np",
 in_re->in_re       "real input (real ridgelets coefficients 2N*2N)",
 out_re<-out_re     "real output (Fimage N*N)"

};
*/
/*----------------------------------------------------------------------
 v1.0: initial revision before publication (J.Froment)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "mw.h"
#include "mw-modules.h" /* for stkwave1(), ridgrecpol(),
			 * fline_extract(), fft2d() */

void iridgelet(in_re,in_im,np,out_re,out_im)
     int np;
     Fimage in_re,in_im;
     Fimage out_re,out_im;

{
  long N,i,j;
  Fimage pol_re=NULL,pol_im=NULL,out1_re=NULL,out1_im=NULL;
  Fsignal ligne=NULL,inv_ond=NULL;
  char c;

  N=(int)((in_re->nrow)/2);
  out_re=mw_change_fimage(out_re,N,N);
  if (!out_re) mwerror(FATAL,1,"not enough memory !\n");

  out_im=mw_change_fimage(out_im,N,N);
  if (!out_im) mwerror(FATAL,1,"not enough memory !\n");

  out1_re=mw_change_fimage(out1_re,N,N);
  if (!out1_re) mwerror(FATAL,1,"not enough memory !\n");

  out1_im=mw_change_fimage(out1_im,N,N);
  if (!out1_im) mwerror(FATAL,1,"not enough memory !\n");

  pol_re=mw_change_fimage(pol_re,2*N,N);
  if (!pol_re) mwerror(FATAL,1,"not enough memory !\n");

  pol_im=mw_change_fimage(pol_im,2*N,N);
  if (!pol_im) mwerror(FATAL,1,"not enough memory !\n");
  
  ligne=mw_change_fsignal(ligne,2*N);
  if (!ligne) mwerror(FATAL,1,"not enough memory !\n");

  inv_ond=mw_change_fsignal(inv_ond,N);
  if (!inv_ond) mwerror(FATAL,1,"not enough memory !\n");

  for(i=0;i<(2*N);i++)
    {
      fline_extract(NULL,in_re,ligne,(long)i);
      istkwave1(np,ligne,inv_ond);
      for(j=0;j<N;j++)
	  pol_re->gray[i*N+j]=inv_ond->values[j];
    }

  if (in_im)
    for(i=0;i<(2*N);i++)
      {
	fline_extract(NULL,in_im,ligne,(long)i);
	istkwave1(np,ligne,inv_ond);
	for(j=0;j<N;j++)
	    pol_im->gray[i*N+j]=inv_ond->values[j];
	   
      }


  ridgpolrec(out1_re,out1_im,pol_re,pol_im);

  fft2d(out1_re,out1_im,out_re,out_im,&c); 

  mw_delete_fimage(pol_re);
  mw_delete_fimage(pol_im);
  mw_delete_fimage(out1_re);
  mw_delete_fimage(out1_im);
  mw_delete_fsignal(ligne);
  mw_delete_fsignal(inv_ond);
}

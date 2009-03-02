/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fhamming};
 version = {"1.0"};
 author = {"Lionel Moisan"};
 function = {"Apply Hamming window to a Fimage"};
 usage = {
      in->in      "input Fimage",
      out<-out    "windowed Fimage"
   };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

/* NB: Calling this module with out=in is possible */

void fhamming(Fimage in, Fimage out)
{
  int     n,p,i,j;
  double  alpha,x,y,w;

  n = in->nrow;
  p = in->ncol;
  if (n<=1 || p<=1) mwerror(FATAL,2,"Image is too small.");
  out = mw_change_fimage(out,n,p);
  if (!out) mwerror(FATAL,1,"Not enough memory.");
  
  alpha = 0.54;   /* use 0.54 for Hamming window, 0.5 for Hanning window */
  
  for (i=0;i<n;i++) for (j=0;j<p;j++) {
    x = M_PI*( 2.0*(double)j/(double)p-1.0 );
    y = M_PI*( 2.0*(double)i/(double)n-1.0 );
    w = (alpha+(1.0-alpha)*cos(x)) * (alpha+(1.0-alpha)*cos(y));
    out->gray[i*p+j] = (float)w * in->gray[i*p+j];
  }
}


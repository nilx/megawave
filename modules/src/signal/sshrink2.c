/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {sshrink2};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Shrink a Fsignal and make its size a power of two"};
   usage = {
           in->in       "input Fsignal",
           out<-out     "shrinked Fsignal"
   };
*/

/*----------------------------------------------------------------------
 v1.1: preserve header info for e.g. sound processing (JF)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

/* NB : calling this module with out=in is nonsense */

Fsignal sshrink2(Fsignal in, Fsignal out)
{
  int n,nn,tmp,i,iofs;
  
  /* Compute new signal size */
  n = in->size;
  nn = 1; tmp = n>>1;
  while (tmp) {tmp>>=1; nn<<=1;}
  
  /* copy center part of input signal */
  out = mw_change_fsignal(out,nn);
  if (!out) mwerror(FATAL,1,"Not enough memory.");
  mw_copy_fsignal_header(in,out);
  
  iofs = (n-nn)>>1;
  for (i=0;i<nn;i++) out->values[i] = in->values[i+iofs];
  
  return(out);
}




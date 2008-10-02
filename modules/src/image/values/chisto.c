/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {chisto};
 version = {"1.2"};
 author = {"Jacques Froment, Lionel Moisan"};
 function = {"Compute the histogram signal of a Cimage"};
 usage = {
    'i'->i_flag  "to compute the integral of the histogram",
    A->A         "input cimage",
    S<-S         "output histogram fsignal"
};
*/
/*----------------------------------------------------------------------
 v1.1: added -i option (L.Moisan)
 v1.2: return S (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include  "mw.h"

Fsignal chisto(A,S,i_flag)
     Cimage A;
     Fsignal S;
     char *i_flag;
{
  int n;
  register float *ptrS;
  register unsigned char *ptrA;
  register int i;

  n = 256;
  S = mw_change_fsignal(S,n);
  if (S == NULL) mwerror(FATAL,1,"Not enough memory.\n");

  for (i=0, ptrS = S->values; i<n; i++, ptrS++) *ptrS=0.0;

  for (i=0, ptrA = A->gray; i < (A->ncol*A->nrow); i++, ptrA++)
    S->values[*ptrA]++;

  if (i_flag) for (i=1;i<n;i++)
    S->values[i] += S->values[i-1];

  return(S);
}

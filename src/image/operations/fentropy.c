/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {fentropy};
author = {"Jacques Froment"};
version = {"1.2"};
function = {"Compute the entropy of an image"};
usage = {
A->A "input fimage",
e<-fentropy "output entropy value"
};
*/
/*----------------------------------------------------------------------
 v1.2: uses fvalues() instead of fhisto() (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#include  "mw.h"

double entropy(input)

Fsignal input;

{
  int i;
  double sum, entr, p;

  if ((!input) || (!input->values)) return(0.0);

  sum = 0.0;
  for (i=0;i<input->size;i++)
    {
      if (input->values[i] < 0)
	mwerror(FATAL,1,"entropy : negative value in the histogram !\n");
      sum += input->values[i];
    }

  if (sum == 0.0) return(0.0);

  entr = 0.0;
  for (i=0;i<input->size;i++)
    if(p = (input->values[i] / sum))
      entr += p * log2(p);

  mwdebug("entropy : size = %d, sum = %f,  E = %f\n",
	  input->size,sum,-entr);

  return(-entr);
}

float fentropy(A)

Fimage A;

{
  Fsignal B,C;
  int n;
  float e;

  B = mw_new_fsignal();
  C = mw_new_fsignal();
  if (!B || !C) mwerror(FATAL,1,"Not enough memory\n");

  fvalues(NULL,B,A,C);
  e = (float) entropy(B);
  mw_delete_fsignal(C);
  mw_delete_fsignal(B);

  return(e);
}


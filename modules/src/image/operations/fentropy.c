/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fentropy};
 author = {"Jacques Froment"};
 version = {"1.4"};
 function = {"Compute the entropy of an image"};
 usage = {
   A->A         "input fimage",
   e<-fentropy  "output entropy value"
};
*/
/*----------------------------------------------------------------------
 v1.2: uses fvalues() instead of fhisto() (L.Moisan)
 v1.3: fix bug in fvalues() call (L.Moisan)
 v1.4: upgrade for new fvalues() call (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* fvalues() */

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
	 entr += p * ((double)log(p)/log(2.));

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
  if (!B) mwerror(FATAL,1,"Not enough memory\n");

  C = fvalues(NULL,B,NULL,A);
  e = (float) entropy(B);
  mw_delete_fsignal(B);
  mw_delete_fsignal(C);

  return(e);
}


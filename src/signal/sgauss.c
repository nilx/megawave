/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {sgauss};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Create a Gaussian Fsignal with unit mass"};
  usage = {
     'd':[std=1.0]->std[0.0,1e10]   "standart deviation (default 1.0)",
     's':size->size                 "size of signal (default: 2*ceil(4std)+1)",
     sgauss<-signal                 "output Fsignal"
  };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

void sgauss(std,size,signal)
float *std;
int *size;
Fsignal signal;
{
  int i,n;
  double sum,v;

  n = 2*(int)ceil((double)(*std*4.0)) + 1;
  if (size) {
    if (*size < n) 
      mwerror(WARNING,0,"small support (%d), %d would be better\n",*size,n);
    n = *size;
  }  

  signal = mw_change_fsignal(signal, n);
  if (!signal) mwerror(FATAL,1,"Not enough memory.");
  strcpy(signal->cmt,"Gaussian");
  signal->shift = -0.5*(float)(n-1);

  if (n==1) {
    signal->values[0]=1.0;
  } else {
    /* store Gaussian signal */
    for (i=(n+1)/2;i--;) {
      v = ((double)i+(double)signal->shift)/(double)(*std);
      signal->values[i] = signal->values[n-1-i] = (float)exp(-0.5*v*v); 
    }
    /* normalize to get unit mass */
    for (sum=0.0,i=n;i--;) sum += (double)signal->values[i];
    for (i=n;i--;) signal->values[i] /= (float)sum;
  }

}








/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {ssinus};
  version = {"1.1"};
  author = {"Jacques Froment"};
  function = {"Create a sine fsignal"};
  usage = {
  's':[size=512]->size     "number of samples in the signal",
  'a':[amplitude=1.0]->A   "amplitude",
  'd':[dilatation=1.0]->D  "dilatation",
  sine<-signal             "output fsignal"
  };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mw.h"

#define M_PI 3.1415926535897931

void ssinus(size,A,D,signal)
     
     int *size;
     float *A,*D;
     Fsignal signal;
     
{
  int i;

  signal = mw_change_fsignal(signal, *size);
  if (signal == NULL) mwerror(FATAL,1,"Not enough memory.");

  strcpy(signal->cmt,"Sine");

  for(i = 0; i< *size; i++)
    signal->values[i] = (float)
      *A * sin((2.0* *D * i / (*size) - *D)*2.0*M_PI);
}








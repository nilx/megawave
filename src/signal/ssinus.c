/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {ssinus};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Create a Sinus fsignal"};
  usage = {
  's':[size=512]->size   "number of samples in the signal (default:512)",
  'a':[amplitude=1.0]->A  "amplitude (default:1)",
  'd':[dilatation=1.0]->D "dilatation (default:1)",
  sinus<-signal           "output fsignal"
  };
*/
/*--- MegaWave - Copyright (C) 1992 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>
#include <math.h>

/* Include always the MegaWave2 include file */
#include "mw.h"

void ssinus(size,A,D,signal)

int *size;
float *A,*D;
Fsignal signal;

{
  int i;

  signal = mw_change_fsignal(signal, *size);
  if (signal == NULL) mwerror(FATAL,1,"Not enough memory.");

  strcpy(signal->cmt,"Sinus");

  for(i = 0; i< *size; i++)
    signal->values[i] = (float)
      *A * sin((2.0* *D * i / (*size) - *D)*2.0*M_PI);
}








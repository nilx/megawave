/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {sdirac};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Create a Dirac fsignal"};
  usage = {
  's':[size=512]->size "number of samples in the signal (default:512)",
  'a':[amplitude=1.0]->A "amplitude (default:1)",
  dirac<-signal "output fsignal"
  };
*/
/*--- MegaWave - Copyright (C) 1992 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>
#include <math.h>

/* Include always the MegaWave2 include file */
#include "mw.h"

void sdirac(size,A,signal)

int *size;
float *A;
Fsignal signal;

{
  int i,med;

  signal = mw_change_fsignal(signal, *size);
  if (signal == NULL) mwerror(FATAL,1,"Not enough memory.");

  strcpy(signal->cmt,"Dirac");

  med = *size >> 1;

/* Provoque un BUG 

  for(i = 0; i<size; i++) signal->values[i] = *A*((i == med) ? 1. : 0.);
*/

  for(i = 0; i< *size; i++)
    if (i == med) signal->values[i] = *A; else 
      signal->values[i] = 0.0;
  
}








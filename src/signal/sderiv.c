/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {sderiv};
  version = {"0.0"};
  author = {"Jacques Froment"};
  function = {"Compute the discrete derivative of a fsignal"};
  usage = {
  fsignal_in->A "input fsignal",
  fsignal_out<-B "output fsignal"
  };
*/
/*--- MegaWave - Copyright (C) 1992 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>
#include <math.h>

/* Include always the MegaWave2 include file */
#include "mw.h"

void sderiv(A,B)

Fsignal A,B;

{
  int i,size;

  size = A->size;
  B = mw_change_fsignal(B, size);
  if (B == NULL) mwerror(FATAL,1,"Not enough memory.");

  strcpy(A->cmt,"Derivative");

  for(i = 1; i<size; i++)
    B->values[i] = (float) (A->values[i] - A->values[i-1]);
  B->values[0] = 0.0;

}








/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {sdirac};
  version = {"1.1"};
  author = {"Jacques Froment"};
  function = {"Create a Dirac fsignal"};
  usage = {
    's':[size=512]->size    "number of samples in the signal",
    'a':[amplitude=1.0]->A  "amplitude",
    dirac<-signal           "output fsignal"
  };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

void sdirac(int *size, float *A, Fsignal signal)
{
    int i, med;

    signal = mw_change_fsignal(signal, *size);
    if (signal == NULL)
        mwerror(FATAL, 1, "Not enough memory.");

    strcpy(signal->cmt, "Dirac");

    med = *size >> 1;

/* Provoque un BUG

  for(i = 0; i<size; i++) signal->values[i] = *A*((i == med) ? 1. : 0.);
*/

    for (i = 0; i < *size; i++)
        if (i == med)
            signal->values[i] = *A;
        else
            signal->values[i] = 0.0;

}

/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {sconst};
  version = {"1.2"};
  author = {"Jacques Froment"};
  function = {"Create a constant fsignal"};
  usage = {
  's':[size=512]->size    "number of samples in the signal",
  'a':[amplitude=1.0]->A  "amplitude",
  sconst<-signal          "output fsignal"
  };
*/

/*----------------------------------------------------------------------
 v1.01: added string.h include file (strcpy warning) (LM)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

void sconst(int *size, float *A, Fsignal signal)
{

  signal = mw_change_fsignal(signal, *size);
  if (signal == NULL) mwerror(FATAL,1,"Not enough memory.");
  strcpy(signal->cmt,"Constant");

  mw_clear_fsignal(signal,*A);
}








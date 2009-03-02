/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {sintegral};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Compute IN PLACE the integral of a Fsignal"};
  usage = {
    'n'->n          "normalize total sum to 1",
    'r'->r          "reverse integral (sum from end)",
    in->in          "input Fsignal",
    out<-sintegral  "output Fsignal (modified input)"
  };
*/

#include "mw.h"
#include "mw-modules.h"

Fsignal sintegral(Fsignal in, char *n, char *r)
{
  int i;
  double v;

  if (!r) {
    v = (double)in->values[0];
    for (i=1;i<in->size;i++) {
      v += (double)in->values[i];
      in->values[i] = v;
    }
  } else {
    v = (double)in->values[in->size-1];
    for (i=in->size-2;i>=0;i--) {
      v += (double)in->values[i];
      in->values[i] = v;
    }
  }
  if (n) for (i=in->size;i--;) in->values[i]/=v;

  return(in);
}








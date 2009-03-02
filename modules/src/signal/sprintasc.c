/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {sprintasc};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Print a part of a Fsignal in ascii format"};
  usage = {
    'v'->verbose   "verbose (display index)",
    s->s           "input Fsignal",
    {
      i1->i1       "first sample",
      i2->i2       "last sample"
    }
};
*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"
 
void sprintasc(Fsignal s, int *i1, int *i2, char *verbose)
{
  int i,vi1,vi2;

  if (verbose) printf("number of samples : %d\n\n",s->size);
  vi1 = (i1?*i1:0);
  vi2 = (i2?*i2:s->size-1);
  for (i=vi1;i<=vi2;i++) {
    if (verbose) printf("%10d : ",i);
    printf("%g\n",s->values[i]);
  }
}


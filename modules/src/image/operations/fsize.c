/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fsize};
 version = {"1.0"};
 author = {"Lionel Moisan"};
 function = {"print the size (x and y dimensions) of a Fimage"};
 usage = {
    in->in   "input Fimage"
};
*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

void fsize(Fimage in)
{
  printf("%d %d\n",in->ncol,in->nrow);
}




/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {flscale};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Scale the components of a Flists"};
   usage = {
     in->in              "input Flists",
     s->s                "input coefficients (Fsignal)",
     out<-flscale        "output Flists"
   };
*/

#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"

Flists flscale(Flists in, Fsignal s)
{
    int i, j, k, d;

    for (i = in->size; i--;)
    {
        d = in->list[i]->dim;
        if (d > s->size)
            mwerror(FATAL, 1, "Fsignal size is too small.");
        for (k = d; k--;)
            for (j = in->list[i]->size; j--;)
                in->list[i]->values[d * j + k] *= s->values[k];
    }
    return (in);
}

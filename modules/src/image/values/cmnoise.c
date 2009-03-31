/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {cmnoise};
 version = {"1.1"};
 author = {"Lionel Moisan"};
 function = {"Put noise on a Cmovie"};
 usage = {
   'g':std->std
     "additive Gaussian noise with standard deviation std",
   'i':p->p[0.0,100.0]
     "impulse noise (range 0..255), applied to p percent of the pixels",
   'q':q->q
     "additive uniform noise in [-q/2,q/2]",
    in->in           "input Cmovie",
    out<-cmnoise     "output Cmovie"
};
*/
/*----------------------------------------------------------------------
 v1.1: added -q option (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "mw.h"
#include "mw-modules.h"         /* for cnoise() */

Cmovie cmnoise(Cmovie in, float *std, float *p, float *q)
{
    Cmovie out;
    Cimage u, new, prev, *next;
    char *init;

    if ((std ? 1 : 0) + (p ? 1 : 0) + (q ? 1 : 0) != 1)
        mwerror(FATAL, 1,
                "Please select exactly one of the -g, -i and -q options.");

    out = mw_new_cmovie();
    prev = NULL;
    next = &(out->first);
    init = NULL;

    for (u = in->first; u; u = u->next)
    {
        new = mw_new_cimage();
        cnoise(u, new, std, p, q, init);
        init = (char *) 1;
        new->previous = prev;
        *next = prev = new;
        next = &(new->next);
    }
    *next = NULL;

    return out;
}

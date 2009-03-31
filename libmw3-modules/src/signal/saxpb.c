/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {saxpb};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Gain/Offset correction to a Fsignal (a x plus b)"};
   usage = {
    'a':a->a   "set a directly (default: 1.0)",
    's':s->s   "set a indirectly by selecting the output standart deviation s",
    'M':M->M   "set a indirectly (and b=0) by selecting total mass M",
    'b':b->b   "set b directly (default: 0.0)",
    'm':m->m   "set b indirectly by selecting the output mean m",
    'k'->k     "set b indirectly by keeping input mean",
    in->in     "input Fsignal",
    out<-out   "output Fsignal"
   };
*/
/*----------------------------------------------------------------------
 v1.1: added -M option and preserve fields shift and scale (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

/*** NB: Calling this module with out=in is possible ***/

void saxpb(Fsignal in, Fsignal out, float *a, float *s, float *b, float *m,
           char *k, float *M)
{
    double mean, var;
    float gain, ofs1, ofs2, *pin, *pout;
    int i;

    if (M)
    {

        if ((s ? 1 : 0) + (a ? 1 : 0) + (m ? 1 : 0) + (b ? 1 : 0) +
            (k ? 1 : 0) > 0)
            mwerror(USAGE, 1, "please use -M option alone");
        ofs1 = ofs2 = mean = 0.;
        for (i = in->size; i--;)
            mean += (double) in->values[i];
        gain = *M / mean;

    }
    else
    {

        if (s && a)
            mwerror(USAGE, 1, "-a and -s options cannot be used together");
        if ((m ? 1 : 0) + (b ? 1 : 0) + (k ? 1 : 0) > 1)
            mwerror(USAGE, 1,
                    "please select no more than one of -b -m -k options");

        mean = var = 0.;
        for (i = in->size; i--;)
            mean += (double) in->values[i];
        mean /= (double) in->size;

        if (s)
        {
            var = 0.;
            for (i = in->size; i--;)
                var +=
                    ((double) in->values[i] -
                     mean) * ((double) in->values[i] - mean);
            var /= (double) in->size;
            gain = *s / (float) sqrt(var);
        }
        else
            gain = (a ? *a : 1.0);

        if (m)
        {
            ofs1 = (float) mean;
            ofs2 = *m;
        }
        else if (k)
        {
            ofs1 = (float) mean;
            ofs2 = ofs1;
        }
        else
        {
            ofs1 = 0.0;
            ofs2 = (b ? *b : 0.0);
        }
    }

    mwdebug("a = %f\n", gain);
    mwdebug("b = %f\n", ofs2 - gain * ofs1);

    out = mw_change_fsignal(out, in->size);
    if (!out)
        mwerror(FATAL, 1, "Not enough memory.");
    out->shift = in->shift;
    out->scale = in->scale;

    for (i = in->size, pin = in->values, pout = out->values; i--;
         pin++, pout++)
        *pout = gain * (*pin - ofs1) + ofs2;
}

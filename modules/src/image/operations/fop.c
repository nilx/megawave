/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fop};
 version = {"1.1"};
 author = {"Lionel Moisan"};
 function = {"Perform an elementary operation between 2 Fimages"};
 usage = {
            'A':A->A       "take Fimage A as left term",
            'a':a->a       "take constant a as left term",
            'p' -> plus    "the A + B (plus) operator",
            'm' -> minus   "the A - B (minus) operator",
            't' -> times   "the A x B (times) operator",
            'd' -> divide  "the A / B (divide) operator",
            'D' -> dist    "the |A-B| (distance) operator",
            'N' -> norm    "the hypot(A,B)=(A^2+B^2)^(1/2) (norm) operator",
            'i' -> inf     "the inf(A,B) operator",
            's' -> sup     "the sup(A,B) operator",
            'l' -> less    "the A < B  operator (result: 1=true, 0=false)",
            'g' -> greater "the A > B  operator (result: 1=true, 0=false)",
            'e' -> equal   "the A == B operator (result: 1=true, 0=false)",
            B->B           "input Fimage B (right term)",
            out<-out       "result Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1: fixed check for -l -g -e options (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

#define ABS(x) ((x)>0?(x):(-(x)))

void fop(Fimage B, Fimage out, Fimage A, float *a, char *plus, char *minus,
         char *times, char *divide, char *dist, char *norm, char *inf,
         char *sup, char *less, char *greater, char *equal)
{
    int i;
    float left, res = 0.0;

    /* check options */
    if ((plus ? 1 : 0) + (minus ? 1 : 0) + (times ? 1 : 0) + (divide ? 1 : 0)
        + (dist ? 1 : 0) + (norm ? 1 : 0) + (inf ? 1 : 0) + (sup ? 1 : 0)
        + (less ? 1 : 0) + (greater ? 1 : 0) + (equal ? 1 : 0) != 1)
        mwerror(USAGE, 1,
                "please select exactly one of the operator options");
    if ((A ? 1 : 0) + (a ? 1 : 0) != 1)
        mwerror(USAGE, 1, "please select exactly one left term (-a or -A)");

    /* prepare output */
    out = mw_change_fimage(out, B->nrow, B->ncol);
    if (!out)
        mwerror(FATAL, 1, "Not enough memory.");

    /* main loop */
    for (i = B->nrow * B->ncol; i--;)
    {
        left = (A ? A->gray[i] : *a);
        if (plus)
            res = left + B->gray[i];
        else if (minus)
            res = left - B->gray[i];
        else if (times)
            res = left * B->gray[i];
        else if (divide)
            res = left / B->gray[i];
        else if (dist)
            res = ABS(left - B->gray[i]);
        else if (norm)
            res = sqrt((double) left * (double) left
                       + (double) (B->gray[i]) * (double) (B->gray[i]));
        else if (inf)
            res = (left < B->gray[i] ? left : B->gray[i]);
        else if (sup)
            res = (left > B->gray[i] ? left : B->gray[i]);
        else if (less)
            res = (left < B->gray[i] ? 1.0 : 0.0);
        else if (greater)
            res = (left > B->gray[i] ? 1.0 : 0.0);
        else if (equal)
            res = (left == B->gray[i] ? 1.0 : 0.0);
        out->gray[i] = res;
    }
}

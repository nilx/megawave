/*------------------------- MegaWave2 module ------------------------------*/
/* mwcommand
 name = {finfo};
 author = {"Lionel Moisan"};
 version = {"1.2"};
 function = {"Compute and display several measures on a Fimage."};
 usage = {
      u->u    "input image"
 };
*/
/*----------------------------------------------------------------------
 v1.1: upgrade for new fvalues() call (L.Moisan)
 v1.2: mwcommand and version syntax fixed (JF)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"         /* fvalues(), fentropy(), fmean(), fnorm() */

static int test_chars(Fsignal s)
{
    int ok, i, c;

    for (ok = 1, i = s->size; i-- && ok;)
    {
        c = (int) s->values[i];
        ok = (s->values[i] == (float) c && c >= 0 && c <= 255);;
    }
    return ok;
}

/*------------------------------ MAIN MODULE ------------------------------*/

void finfo(Fimage u)
{
    Fsignal v;
    int b;
    float p;

    printf("-----------------------------------");
    printf("-----------------------------------\n");
    printf("name = %s\n", u->name);
    printf("cmt = %s\n", u->cmt);
    printf("ncol = %d\n", u->ncol);
    printf("nrow = %d\n", u->nrow);
    v = fvalues(NULL, NULL, NULL, u);
    printf("number of grey levels = %d\n", v->size);
    printf("entropy = %f\n", fentropy(u));
    if (test_chars(v))
        printf("This could be a char image.\n");
    printf("min = %f\n", v->values[0]);
    printf("max = %f\n", v->values[v->size - 1]);
    printf("mean = %f\n", fmean(u));
    b = 0;
    p = 2.;
    printf("normalized l2 norm = %f\n",
           fnorm(u, NULL, &p, NULL, NULL, &b, NULL, NULL));
    printf("normalized bv norm = %f\n",
           fnorm(u, NULL, NULL, NULL, (char *) 1, &b, NULL, NULL));
    printf("-----------------------------------");
    printf("-----------------------------------\n");
}

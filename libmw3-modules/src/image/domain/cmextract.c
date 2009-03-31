/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {cmextract};
author = {"Lionel Moisan"};
function = {"Extract a subpart of a Cmovie"};
version = {"1.8"};
usage = {
 'b':[b=0]->b    "background grey level",
 'r'->r          "if set, X2,Y2,T2 must be the SIZE of the extracted region",
 in->in          "input Cmovie",
 out<-cmextract  "output Cmovie",
 X1->X1          "upleft corner of the region to extract from input (x)",
 Y1->Y1          "upleft corner of the region to extract from input (y)",
 T1->T1          "index of first extracted image (1 is first index)",
 X2->X2          "downright corner of the region to extract from input (x)",
 Y2->Y2          "downright corner of the region to extract from input (y)",
 T2->T2          "index of last extracted image (1 is first index)",
  {
    bg->bg       "background Cmovie",
    [Xc=0]->Xc   "new location of X1 on the background",
    [Yc=0]->Yc   "new location of Y1 on the background",
    [Tc=1]->Tc   "new location of T1 on the background"
  }
};
*/
/*-------------------------------------------------------------------------
 v1.6: module rewritten, extended parameter values, -r,-b options (L.Moisan)
 v1.7: new possibilities added (L.Moisan)
 v1.8 (04/2007): simplified header (LM)
-------------------------------------------------------------------------*/

#include <stdlib.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for cextract() */

Cmovie cmextract(int *b, Cmovie in, Cmovie bg, int X1, int Y1, int T1, int X2,
                 int Y2, int T2, int *Xc, int *Yc, int *Tc, char *r)
{
    Cmovie out;
    Cimage u, *u1, *u2 = NULL, new, prev, *next, im1, im2;
    int i1, i2 = 0, i3, j1, j2, j3;

    /* put input movie(s) into array(s) */
    for (i1 = 0, u = in->first; u; u = u->next)
        i1++;
    u1 = (Cimage *) malloc((i1 + 1) * sizeof(Cimage));
    if (!u1)
        mwerror(FATAL, 1, "Not enough memory\n");
    for (i1 = 0, u = in->first; u; u = u->next)
        u1[i1++] = u;
    u1[i1] = mw_change_cimage(NULL, u1[0]->nrow, u1[0]->ncol);
    mw_clear_cimage(u1[i1], *b);

    if (bg)
    {
        for (i2 = 0, u = bg->first; u; u = u->next)
            i2++;
        u2 = (Cimage *) malloc((i2 + 1) * sizeof(Cimage));
        if (!u2)
            mwerror(FATAL, 1, "Not enough memory\n");
        for (i2 = 0, u = bg->first; u; u = u->next)
            u2[i2++] = u;
        u2[i2] = mw_change_cimage(NULL, u2[0]->nrow, u2[0]->ncol);
        mw_clear_cimage(u2[i2], *b);
    }

    out = mw_new_cmovie();
    next = &(out->first);
    prev = NULL;
    if (r)
        T2 += T1 - 1;
    i3 = T2 - T1 + 1;
    if (bg)
    {
        i3 += *Tc - 1;
        if (i2 > i3)
            i3 = i2;
    }
    for (j3 = 0; j3 < i3; j3++)
    {
        j1 = j3 + (T1 - 1);
        im1 =
            u1[(j1 >= 0 && j1 < i1 && j1 >= T1 - 1
                && j1 <= T2 - 1 ? j1 : i1)];
        if (bg)
        {
            j2 = j3 + (*Tc - 1);
            im2 = u2[(j2 >= 0 && j2 < i2 ? j2 : i2)];
        }
        else
            im2 = NULL;
        new = cextract(b, im1, im2, NULL, X1, Y1, X2, Y2, Xc, Yc, r);
        new->previous = prev;
        *next = prev = new;
        next = &(new->next);
    }
    *next = NULL;

    return (out);
}

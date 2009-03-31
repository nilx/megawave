/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {ccontrast};
 version = {"1.0"};
 author = {"Lionel Moisan"};
 function = {"Contrast improvement by histogram adjusting"};
 usage = {
    'g':g->g [0.0,1e20]  "gamma factor (visual correction)",
    input->in            "source image",
    output<-out          "processed image"
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

/***** gamma correction [0,1]->[0,1] *****/

static float fgamma(float g, float x)
{
    return (float) pow((double) x, 1.0 / (double) g);
}

/****************************** CONTRAST ******************************/

void ccontrast(Cimage in, Cimage out, float *g)
{
    int *histo, h;
    int adr, i, x2, size, ofs;
    unsigned char *new, y;
    float yd;

    /* Allocate memory */
    out = mw_change_cimage(out, in->nrow, in->ncol);
    size = in->nrow * in->ncol;
    histo = (int *) calloc(256, sizeof(int));
    new = (unsigned char *) calloc(256, sizeof(unsigned char));
    if (!out || !histo || !new)
        mwerror(FATAL, 1, "Not enough memory !\n");

    /* Compute the old histogram */
    for (adr = 0; adr < size; adr++)
        histo[(unsigned int) (in->gray[adr])]++;

    /* Compute the new histogram */
    ofs = 0;
    x2 = 0;
    for (i = 0; i < 256; i++)
        if (0 != (h = histo[i]))
        {
            /* grey level 0 correction */
            if (x2 == 0)
                ofs = -h;
            yd = (float) (x2 + ofs + h) * 0.5 / size;
            /* gamma correction if any */
            if (g)
                yd = fgamma(*g, yd);
            y = (int) (yd * 256.0);
            /* grey level 255 correction */
            if (x2 + h == size << 1)
                y = 255;
            x2 += 2 * h;
            new[i] = y;
            mwdebug("gray %d -> %d\n", i, y);
        }

    /* Process image */
    for (adr = 0; adr < size; adr++)
        out->gray[adr] = new[in->gray[adr]];
}

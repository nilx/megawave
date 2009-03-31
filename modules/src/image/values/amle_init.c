/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {amle_init};
 version = {"1.1"};
 author = {"Jacques Froment"};
 function = {"Compute initial data for the level line image
             interpolation scheme (AMLE)"};
 usage = {
   in->in
         "Input uniformly quantized fimage",
   delta->delta
         "Width step of the uniform quantization used for the input image",
   out<-out
         "Output fimage as initial data (input) for the AMLE model"
};
*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

/* Compute the 4 neighbour pixels of the current pixel p              */
/* When p touches the border of the image, a mirror effect is applied */

static void neighbor_4(register int x, register int y, register int xmax,
                       register int ymax, register float *p, float **left,
                       float **right, float **up, float **down)
{
    if (x > 0)
    {
        *left = p - 1;
        if (x < xmax)
        {
            *right = p + 1;
            if (y > 0)
            {
                *up = p - xmax - 1;
                if (y < ymax)
                    /* 0 < x < xmax  0 < y < ymax */
                    *down = p + xmax + 1;
                else            /* 0 < x < xmax   y = ymax */
                    *down = *up;
            }
            else                /* 0 < x < xmax   y = 0 */
            {
                *down = p + xmax + 1;
                *up = *down;
            }
        }
        else                    /* x = xmax */
        {
            *right = *left;
            if (y > 0)
            {
                *up = p - xmax - 1;
                if (y < ymax)
                    /* x = xmax  0 < y < ymax */
                    *down = p + xmax + 1;
                else            /* x = xmax   y = ymax */
                    *down = *up;
            }
            else                /* x = xmax  y = 0 */
            {
                *down = p + xmax + 1;
                *up = *down;
            }
        }
    }
    else                        /* x = 0 */
    {
        *right = p + 1;
        *left = *right;
        if (y > 0)
        {
            *up = p - xmax - 1;
            if (y < ymax)
                /* x = 0  0 < y < ymax */
                *down = p + xmax + 1;
            else                /* x = 0   y = ymax */
                *down = *up;
        }
        else                    /* x = 0   y = 0 */
        {
            *down = p + xmax + 1;
            *up = *down;
        }
    }
}

void amle_init(Fimage in, float delta, Fimage out)
{
    int NC, NL, x, y;
    register float *I, *O;
    float *left, *right, *up, *down;
    float Max;

    if (in == NULL)
        mwerror(FATAL, 1, "Null input fimage.\n");

    NL = in->nrow;
    NC = in->ncol;

    out = mw_change_fimage(out, NL, NC);
    if (out == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");

    mw_clear_fimage(out, 0.0);

    I = in->gray;
    O = out->gray;

    for (y = 0; y < NL; y++)
        for (x = 0; x < NC; x++, I++, O++)
        {
            neighbor_4(x, y, NC - 1, NL - 1, I, &left, &right, &up, &down);
            if ((*I != *left) || (*I != *right) || (*I != *up)
                || (*I != *down))
            {
                Max = *I;
                if (*left > Max)
                    Max = *left;
                if (*right > Max)
                    Max = *right;
                if (*up > Max)
                    Max = *up;
                if (*down > Max)
                    Max = *down;

                if (*I < Max)
                    *O = *I + delta;
                else
                    *O = *I;
            }
        }
}

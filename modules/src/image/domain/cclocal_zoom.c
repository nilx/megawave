/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {cclocal_zoom};
  version = {"1.2"};
  author = {"Jacques Froment"};
  function = {"In-place local Zoom of a color char image"};
  usage = {
    'x':[center_x=256]->X  "X coordinate for the center of the zoom array",
    'y':[center_y=256]->Y  "Y coordinate for the center of the zoom array",
    'W':[width=40]->W      "Width of the zoom array",
    'X':[factor=2]->factor [1,10] "Zoom factor",
    A->Input        "Input (could be a ccimage)",
    B<-cclocal_zoom  "Output (zoomed image)"
};
*/
/*----------------------------------------------------------------------
 v1.2: fixed bug: values of x0 and y0 (*factor-1 term) (L.Moisan)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mw.h"
#include "mw-modules.h"

Ccimage cclocal_zoom(Ccimage Input, int *X, int *Y, int *W, int *factor)
{
    unsigned char *squareR, *squareG, *squareB;
    unsigned char r, g, b;
    register unsigned char *Red, *Green, *Blue;
    register int x, y, i, j;
    int dx, dy;                 /* Size of Input & Ouput */
    int sx, sy;                 /* Size of square */
    int x0, x1, y0, y1;
    int D, Df, l, ll;

    dx = Input->ncol;
    dy = Input->nrow;

    D = *W / 2;

    if (*X - D < 0)
        D = *X;
    if (*X + D >= dx)
        D = dx - *X - 1;
    if (*Y - D < 0)
        D = *Y;
    if (*Y + D >= dy)
        D = dy - *Y + 1;

    sx = 2 * D + 1;
    sy = sx;

    squareR = (unsigned char *) malloc(sx * sy);
    if (squareR == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");
    squareG = (unsigned char *) malloc(sx * sy);
    if (squareG == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");
    squareB = (unsigned char *) malloc(sx * sy);
    if (squareB == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");

    Red = Input->red;
    Green = Input->green;
    Blue = Input->blue;

    for (y = 0; y < sy; y++)
    {
        memcpy(&squareR[sx * y], &Red[*X - D + dx * (y + *Y - D)], sx);
        memcpy(&squareG[sx * y], &Green[*X - D + dx * (y + *Y - D)], sx);
        memcpy(&squareB[sx * y], &Blue[*X - D + dx * (y + *Y - D)], sx);
    }

    Df = (*factor) * D;
    x0 = 0;
    x1 = sx - 1;
    l = (*X - Df - (*factor - 1)) / (*factor);
    if (l < 0)
        x0 = -l;
    l = (dx + Df - *X) / (*factor);
    if (l <= sx)
        x1 = l - 1;

    y0 = 0;
    y1 = sy - 1;
    l = (*Y - Df - (*factor - 1)) / (*factor);
    if (l < 0)
        y0 = -l;
    l = (dy + Df - *Y) / (*factor);
    if (l <= sy)
        y1 = l - 1;

    for (x = x0; x <= x1; x++)
        for (y = y0; y <= y1; y++)
        {
            i = x + sx * y;
            r = squareR[i];
            g = squareG[i];
            b = squareB[i];
            l = *X + ((*factor) * x) - Df + dx * (*Y + ((*factor) * y) - Df);
            for (i = 0; i < (*factor); i++)
                for (j = 0; j < (*factor); j++)
                {
                    ll = l + j * dx + i;
                    Red[ll] = r;
                    Green[ll] = g;
                    Blue[ll] = b;
                }
        }
    free(squareB);
    free(squareG);
    free(squareR);

    return (Input);
}

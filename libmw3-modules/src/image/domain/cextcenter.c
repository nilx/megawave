/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {cextcenter};
 version = {"1.1"};
 author = {"Jean-Pierre D'Ales"};
 function = {"Copy the center part of a cimage into another image"};
 usage = {
   'f':[Fact=16]->Fact      "Factor",
   OriginalImage->Image     "Input Cimage",
   CopyImage<-cextcenter    "Output center part of the input"
};
 */
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw3.h"
#include "mw3-modules.h"

Cimage cextcenter(int *Fact, Cimage Image)
                             /* Output size must be a multiple of Fact */
                                /* Original image */
{
    Cimage Result;              /* Resulting image */
    int dx, dy;                 /* Size of the input image */
    int rdx, rdy;               /* Size of the resulting image */
    int orx, ory;               /* Coordinates of the upper left corner
                                 * of Result in input image */
    int l, c;                   /* Index for column and lines in images */
    register unsigned char *ptrI, *ptrR;
    /* Pointers to graylevels of Image
     * and Result */

  /*--- Reading of default values ---*/

    dx = Image->ncol;
    dy = Image->nrow;
    rdx = (dx / *Fact) * *Fact;
    rdy = (dy / *Fact) * *Fact;
    orx = (dx - rdx) / 2;
    ory = (dy - rdy) / 2;

  /*--- Memory allocation for resulting image ---*/

    Result = mw_new_cimage();
    if (mw_alloc_cimage(Result, rdy, rdx) == NULL)
        mwerror(FATAL, 1, "Memory allocation refused for `Result`!\n");

  /*--- Copy the extracted sub-image on result ---*/

    ptrR = Result->gray;
    ptrI = Image->gray + (ory * dx + orx);
    for (l = 0; l < rdy; l++, ptrI += dx - rdx)
        for (c = 0; c < rdx; c++, ptrI++, ptrR++)
            *ptrR = *ptrI;

    return (Result);

}

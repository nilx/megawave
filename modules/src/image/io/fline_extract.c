/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fline_extract};
 author = {"Jean-Pierre D'Ales, Jacques Froment"};
 function = {"Extract a line or a column of a fimage
             and store it as an 1D fsignal"};
 version = {"1.02"};
 usage = {
    'c'->cflag            "Flag for column number",
    Image->Image          "Input fimage",
    ExtractedSig<-Signal  "Output fsignal",
    Number->Index         "Line (default) or column number"
};
*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

void fline_extract(char *cflag, Fimage Image, Fsignal Signal, long int Index)
        /*--- Extract line `LineIndex` or column `ColumnIndex` in `Image` ---*/
                        /* Indices of the line or the column
                         * to be extracted */
                                        /* Input image */
                                /* Output, extracted line or column */
{

    long size;                  /* Size of the extracted line  or column */
    short l, c;                 /* Index of the current point
                                 * in the extracted line/column */

    if (Index < 0)
        mwerror(FATAL, 1, "Negative Index value !\n");

    if (cflag)                  /* Index is for column */
    {
        size = Image->nrow;
        if (Index >= Image->ncol)
            mwerror(FATAL, 1,
                    "Index is greater than the number of columns in Image!\n");
    }
    else                        /* Index is for line */
    {
        size = Image->ncol;
        if (Index >= Image->nrow)
            mwerror(FATAL, 1,
                    "Index is greater than the number of rows in Image!\n");
    }

    if ((Signal = mw_change_fsignal(Signal, size)) == NULL)
        mwerror(FATAL, 1, "Not enough memory\n");

    if (cflag)
    {
        for (l = 0; l < size; l++)
            Signal->values[l] = mw_getdot_fimage(Image, Index, l);

        Signal->firstp = Image->firstrow;
        Signal->lastp = Image->lastrow;
        /* Signal->scale = Image->scale; */
        Signal->scale = 1.0;
        sprintf(Signal->cmt, "Column %ld of %s", Index, Image->name);
    }
    else
    {
        for (c = 0; c < size; c++)
            Signal->values[c] = mw_getdot_fimage(Image, c, Index);
        Signal->firstp = Image->firstcol;
        Signal->lastp = Image->lastcol;
        /* Signal->scale = Image->scale; */
        Signal->scale = 1.0;
        sprintf(Signal->cmt, "Line %ld of %s", Index, Image->name);
    }
}

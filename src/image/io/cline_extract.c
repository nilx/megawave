/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {cline_extract};
author = {"Jacques Froment"};
function = {"Extract a line or a column of a cimage and store it as an 1D fsignal"};
version = {"1.0"};

usage = {
  'c'->cflag "Flag for column number",
  Image->Image "Input image (should be a cimage)", 
  ExtractedSig<-Signal "Output fsignal",
  Number->Index "Line (default) or column number"
};

 */

/*--- Fichiers inclus UNIX C ---*/
#include <stdio.h>

/*--- Bibliotheque megawave2 ---*/
#include  "mw.h"


void cline_extract(cflag, Image, Signal, Index)

	/*--- Extract line `LineIndex` or column `ColumnIndex` in `Image` ---*/

    char        *cflag;
    long	Index;	/* Indices of the line or the column 
					 * to be extracted */
    Cimage	Image;			/* Input image */
    Fsignal	Signal;		/* Output, extracted line or column */

{

    long	size;		/* Size of the extracted line  or column */
    short	l, c;		/* Index of the current point 
				 * in the extracted line/column */

    if (Index < 0) mwerror(FATAL,1,"Negative Index value !\n");

    if (cflag)  /* Index is for column */
      {
	size = Image->nrow;
	if(Index >= Image->ncol)
	  mwerror(FATAL, 1, "Index is greater than the number of columns in Image!\n");
      }
    else      /* Index is for line */
      {
	size = Image->ncol;
	if(Index >= Image->nrow)
	  mwerror(FATAL, 1, "Index is greater than the number of rows in Image!\n");
      }

    if ((Signal = mw_change_fsignal(Signal, size)) == NULL)
      mwerror(FATAL,1,"Not enough memory\n");

    if (cflag)
      {
	for (l = 0; l < size; l++)
	    Signal->values[l] = mw_getdot_cimage(Image,Index,l);

	Signal->firstp = Image->firstrow;
	Signal->lastp = Image->lastrow;
	/* Signal->scale = Image->scale; */
	Signal->scale = 1.0;
	sprintf(Signal->cmt, "Column %d of %s", Index, Image->name);
    } else
      {
	for (c = 0; c < size; c++)
	    Signal->values[c] = mw_getdot_cimage(Image,c,Index);
	Signal->firstp = Image->firstcol;
	Signal->lastp = Image->lastcol;
	/* Signal->scale = Image->scale; */
	Signal->scale = 1.0;
	sprintf(Signal->cmt, "Line %d of %s", Index, Image->name);
      }
}

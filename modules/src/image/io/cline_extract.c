/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {cline_extract};
 author = {"Jacques Froment"};
 function = {"Extract a line or a column of a cimage and store it as an 1D fsignal"};
 version = {"1.1"};
 usage = {
   'c'->cflag            "Flag for column number",
   Image->Image          "Input image (should be a cimage)", 
   ExtractedSig<-Signal  "Output fsignal",
   Number->Index         "Line (default) or column number",
   {
     OutImage<-OutImage "Output image (with selected line/column marked)"
   }
};
*/
/*----------------------------------------------------------------------
  v1.1: added "Output image" option (Jacques Froment)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"


void cline_extract(char *cflag, Cimage Image, Fsignal Signal, long int Index, Cimage OutImage)

	/*--- Extract line `LineIndex` or column `ColumnIndex` in `Image` ---*/

                       
        	      	/* Indices of the line or the column 
					 * to be extracted */
          	      			/* Input image */
           	       		/* Output, extracted line or column */
                         

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

    if (OutImage)
      {
	if (!(OutImage=mw_change_cimage(OutImage,Image->nrow,Image->ncol)))
	  mwerror(FATAL,1,"Not enough memory\n");	  
	mw_copy_cimage(Image,OutImage);
      }
    
    if (cflag)
      {
	for (l = 0; l < size; l++)
	    Signal->values[l] = mw_getdot_cimage(Image,Index,l);

	Signal->firstp = Image->firstrow;
	Signal->lastp = Image->lastrow;
	/* Signal->scale = Image->scale; */
	Signal->scale = 1.0;
	sprintf(Signal->cmt, "Column %ld of %s", Index, Image->name);

	if (OutImage)
	  for (l = 0; l < size; l++)
	    mw_plot_cimage(OutImage,Index,l,255*(l%2));
	
    } else
      {
	for (c = 0; c < size; c++)
	    Signal->values[c] = mw_getdot_cimage(Image,c,Index);
	Signal->firstp = Image->firstcol;
	Signal->lastp = Image->lastcol;
	/* Signal->scale = Image->scale; */
	Signal->scale = 1.0;
	sprintf(Signal->cmt, "Line %ld of %s", Index, Image->name);

	if (OutImage)
	  for (c = 0; c < size; c++)
	    mw_plot_cimage(OutImage,c,Index,255*(c%2));
      }
}

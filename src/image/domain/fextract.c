/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {fextract};
author = {"Jean-Pierre D'Ales, Jacques Froment"};
function = {"Copy a part of a fimage into another image, in an optional background"};
version = {"1.3"};
usage = {
  OriginalImage->Image "Original image" , 
  CopyImage<-Result    "Output image"   ,
  Xo->X1               "X Coor. of upper left point of subimage in original image",
  Yo->Y1               "Y Coor. of upper left point of subimage in original image",
  Xf->X2  "X Coor. of lower right point of subimage in original image",              
  Yf->Y2  "Y Coor. of lower right point of subimage in original image",
  {
    Background->Back  "Background input image",
    [Xc=0]->Xc "X Coor. of the upper left point in the background",
    [Yc=0]->Yc "Y Coor. of the upper left point in the background"
  }
};
*/


/*--- Include files UNIX C ---*/
#include <stdio.h>
#include <math.h>

/*--- Library megawave2 ---*/
#include  "mw.h"


Fimage fextract(Image, Back, Result, X1, Y1, X2, Y2, Xc, Yc)

    Fimage          Image;	/* Original image */
    Fimage          Back;	/* Back image */
    Fimage          Result;	/* Resulting image */
    int             X1, Y1;	/* Coordinates of upper left point of sub-image
				 * extracted in original image */
    int 	    X2, Y2;	/* Coordinates of lower right point of sub-image
				 * extracted in original image */
    int 	    *Xc, *Yc;	/* Coordinates of the origin point for inserting
				 * in Background image */

{
    int             rdx, rdy;	/* Size of the resulting image */
    int             tdx, tdy;	/* Size of the Back image */
    int             lrdx, ltdx, lidx;
    int             DX, DY;	/* Size of the extracted sub-image */
    register int             l, c;  /* Index for column and lines in images */
    register float *ptrR,*ptrT;

    /*--- Reading of default values ---*/

    DX = X2 - X1;
    DY = Y2 - Y1;

    if ((X1<0) || (Y1<0) || (DX < 0) || (DY < 0) || 
	(Image->ncol <= X2) || (Image->nrow <= Y2))
      mwerror(FATAL,1,"Illegal coordinates specification\n");
      
    if (Back) tdx = Back->ncol; else tdx = DX+1;
    if (Back) tdy = Back->nrow; else tdy = DY+1;

    if (tdx <= *Xc + DX)
	rdx = *Xc + DX + 1;
    else
	rdx = tdx;

    if (tdy <= *Yc + DY)
	rdy = *Yc + DY + 1;
    else
	rdy = tdy;

    /*--- Memory allocation for resulting image ---*/

    if ((Result = mw_change_fimage(Result, rdy, rdx)) == NULL)
      mwerror(FATAL, 1, "Memory allocation refused for `Result`!\n");

    /*--- Copy Back image on result image ---*/

    ltdx = lrdx = 0;
    ptrR = Result->gray;

    if (Back)
      {
	ptrT = Back->gray;
	for (l = 0; l < Back->nrow; l++)
	  {
	    for (c = 0; c < Back->ncol; c++)
	      ptrR[lrdx + c] = ptrT[ltdx + c];
	    lrdx += rdx;
	    ltdx += tdx;
	  }

    /*--- Put other gray-levels to 0 if Result ---*/
    /*--- has greater size than Back ---*/

    lrdx = tdy * rdx;
    for (l = tdy; l < rdy; l++)
    {
	for (c = 0; c < rdx; c++)
	    ptrR[lrdx + c] = 0.0;
	lrdx += rdx;
    }

    lrdx = 0;
    for (l = 0; l < Back->nrow; l++)
    {
	for (c = Back->ncol; c < rdx; c++)
	    ptrR[lrdx + c] = 0.0;
	lrdx += rdx;
    }

      } /* if (Back) */

    /*--- Copy the extracted sub-image on result ---*/

    lrdx = *Yc * rdx;
    lidx = Y1 * Image->ncol;
    ptrT = Image->gray;
    for (l = 0; l <= DY; l++)
    {
	for (c = 0; c <= DX; c++)
	    ptrR[lrdx + *Xc + c] = ptrT[lidx + X1 + c];
	lrdx += rdx;
	lidx += Image->ncol;
    }
}

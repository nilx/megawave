/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {cfextcenter};
author = {"Jean-Pierre D'Ales"};
function = {"Copy the center part of a cfimage into another image"};
usage = {
'f':[Fact=16]->Fact
  "Factor (default:16)", 
OriginalImage->Image 
  "Input Cfimage", 
CopyImage<-cfextcenter
  "Output center part of the input"
};
version = {"1.00"};
 */


/*--- Include files UNIX C ---*/
#include <stdio.h>

/*--- Library megawave2 ---*/
#include  "mw.h"


Cfimage
cfextcenter(Fact, Image)

int            *Fact;   /* Output size must be a multiple of Fact */
Cfimage         Image;	/* Original image */

{
  Cfimage         Result;	    /* Resulting image */
  int             dx, dy;           /* Size of the input image */
  int             rdx, rdy;	    /* Size of the resulting image */
  int             orx, ory;	    /* Coordinates of the upper left corner 
				     * of Result in input image */
  int             l, c;		    /* Index for column and lines in images */
  register float *ptrIR, *ptrIG, *ptrIB; /* Pointers to channels of Image */
  register float *ptrRR, *ptrRG, *ptrRB; /* Pointers to channels of Result */

  /*--- Reading of default values ---*/

  dx = Image->ncol;
  dy = Image->nrow;
  rdx = (dx / *Fact) * *Fact;
  rdy = (dy / *Fact) * *Fact;
  orx = (dx - rdx) / 2;
  ory = (dy - rdy) / 2;

  /*--- Memory allocation for resulting image ---*/

  Result = mw_new_cfimage();
  if (mw_alloc_cfimage(Result, rdy, rdx) == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for `Result`!\n");

  /*--- Copy the extracted sub-image on result ---*/

  ptrRR = Result->red;
  ptrRG = Result->green;
  ptrRB = Result->blue;
  ptrIR = Image->red + (ory * dx + orx);
  ptrIG = Image->green + (ory * dx + orx);
  ptrIB = Image->blue + (ory * dx + orx);
  for (l = 0; l < rdy; l++, ptrIR += dx - rdx, ptrIG += dx - rdx, ptrIB += dx - rdx)
    for (c = 0; c < rdx; c++, ptrIR++, ptrIG++, ptrIB++, ptrRR++, ptrRG++, ptrRB++) {
      *ptrRR = *ptrIR;
      *ptrRG = *ptrIG;
      *ptrRB = *ptrIB;
    }

  return (Result);

}

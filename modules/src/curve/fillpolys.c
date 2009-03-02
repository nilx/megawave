/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {fillpolys};
  version = {"1.3"};
  author = {"Jean-Pierre D'Ales, Jacques Froment"};
  function = {"Fill a set of Polygons and generate a Cimage"};
  usage = {
  'x':[size_x=256]->dx  "size in x (nb of columns) of the output cimage",
  'y':[size_y=256]->dy  "size in y (nb of rows) of the output cimage",
  Polygons->polys       "set of Polygons (input)",
  Cimage<-bitmap        "bitmapped Cimage of the filled polygons (output)"
  };
*/
/*----------------------------------------------------------------------
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for fillpoly() */

/*--- Constants ---*/

#define NCHANNEL_POLYG 1   /* Nuber of channels in polygons */
#define BG_SYMB 255        /* Symbol for background in bitmap */
#define MAX_NPOLYG 254     /* Maximum number of polygons */


static int     fac;


static void
UPDATE_BITMAP(Cimage bitmap, Cimage bitmap_poly)

                                 /* Global bitmap image */
                                 /* bitmap image for one polygon */
                                 /* Index of polygon */

{
  register unsigned char  *ptrg;     /* Pointers to bitmap */
  register unsigned char  *ptrb;     /* Pointer to bitmap_poly */
  long             x;                /* Buffer index for current point */
  long             size;             /* Size of bitmap images */

  size = bitmap->nrow * bitmap->ncol;
  ptrg = bitmap->gray;
  ptrb = bitmap_poly->gray;
  for (x = 0; x < size; x++, ptrg++, ptrb++) 
     if (*ptrb != BG_SYMB) 
       *ptrg = 0;
  /*
       *ptrg = fac * p;
  */
}


void 
fillpolys(int *dx, int *dy, Polygons polys, Cimage bitmap)

                                     /* Size of output bitmap */
                                     /* Input polygons */
                                     /* Output bitmap */

{
  register unsigned char  *ptra;     /* Pointer to significance map */
  long             x;                /* Buffer index for current point 
				      * in bitmap */
  long             size;             /* Size of bitmap */
  Polygon          ptr_polyg;        /* Pointer to the current polygon */
  unsigned char    p;                /* Index of polygon */
  int              npolyg;           /* Number of polygons */
  Cimage           bitmap_poly;      /* Output bitmap for one polygon */

  /*--- Memory allocation and initialization for global bitmap ---*/

  bitmap = mw_change_cimage(bitmap, *dy, *dx);
  if (bitmap == NULL) 
    mwerror(FATAL,1,"Not enough memory.\n");
  size = bitmap->nrow * bitmap->ncol;
  for (x = 0, ptra = bitmap->gray; x < size; x++, ptra++) 
    *ptra = BG_SYMB;      

  /*--- Memory allocation for temporary bitmap ---*/

  bitmap_poly = mw_change_cimage(NULL, *dy, *dx);
  if (bitmap_poly == NULL) 
    mwerror(FATAL,1,"Not enough memory.\n");

  /*--- Count number of polygons ---*/

  ptr_polyg = polys->first;
  npolyg = 0;
  while (ptr_polyg) {
    npolyg++;
    ptr_polyg = ptr_polyg->next;
  }

  if (npolyg > MAX_NPOLYG)
    mwerror(WARNING, 0, "Too many polygons in input (%d)!\n", npolyg);

  if (npolyg > 0)
    fac = BG_SYMB / npolyg;

  ptr_polyg = polys->first;
  p = 0;
  while (ptr_polyg) {

    /*--- Fill one polygon ---*/

    fillpoly(dx, dy, ptr_polyg, bitmap_poly);

    /*--- Copy result on global bitmap ---*/ 

    UPDATE_BITMAP(bitmap, bitmap_poly);

    ptr_polyg = ptr_polyg->next;
    p++;
  }

    mw_delete_cimage(bitmap_poly);
}








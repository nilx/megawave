/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {emptypoly};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Empty the polygons contained in a cimage"};
  usage = {
  cimage_polys->A "cimage of polygons (input)",
  cimage_empty<-B "the same cimage with empty polygons (output)"
  };
*/
/*--- MegaWave - Copyright (C) 1992 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 include file */
#include "mw.h"

emptypoly(A,B)

Cimage A,B;

{
  register unsigned char *ptrA,*ptrB;
  register int i;
  unsigned char *P1,*P2,*P3,*P4,*P5,*P6,*P7,*P8;
  
  B = mw_change_cimage(B, A->nrow, A->ncol);
  if (B == NULL) mwerror(FATAL,1,"Not enough memory.\n");
  mw_clear_cimage(B,0);

  for (i=A->ncol+1, ptrA = A->gray + A->ncol+1, ptrB = B->gray + A->ncol+1;
       i< (A->ncol-1)*(A->nrow-1);
       i++, ptrA++, ptrB++)
    if (*ptrA)
    {
      P1 = ptrA+1;            /* i+1,j */
      P2 = P1+A->ncol;        /* i+1,j+1 */
      P3 = ptrA+A->ncol;      /* i,j+1 */
      P4 = P3-1;              /* i-1,j+1 */
      P5 = ptrA-1;            /* i-1, j */
      P6 = P5-A->ncol;        /* i-1, j-1 */
      P7 = ptrA-A->ncol;      /* i,j-1 */
      P8 = P7+1;              /* i+1.j-1 */

      if (!(*P1) || !(*P2) || !(*P3) || !(*P4) || !(*P5) ||
	  !(*P6) || !(*P7) || !(*P8))
	*ptrB = *ptrA;
    }
  
}








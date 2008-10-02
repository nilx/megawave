/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {mscarea};
 version = {"1.1"};
 author = {"Jacques Froment"};
 function = {"compute the area of a morpho set, from a cimage"};
 usage = {
   'c'->connex8  "8-connexity (default : 4)",
   'o':O<-O      "Output the m.s. in a cimage",
   's':stoparea->stoparea "stop when the given area is reached",
   U->U   "input cimage U",
   a->a   "min value a for the m.s. {(x,y)/ a <= U(x,y) <= b}",
   b->b   "max value b for the m.s. {(x,y)/ a <= U(x,y) <= b}",
   x0->x0 "x0 coordinate such that the point (x0,y0) belongs to the m.s.",
   y0->y0 "y0 coordinate such that the point (x0,y0) belongs to the m.s.",
   area<-mscarea "output area of the m.s."
};
*/

#include <stdio.h>
#include "mw.h"

void compute_area(connex8, U, O, M, nrow, ncol, a, b, x, y, l, stoparea, area)
     char *connex8;
     unsigned char *U,*O,*M;
     int nrow,ncol,a,b,x,y,l;
     int *stoparea,*area;
{
  int k;

  if ((stoparea != NULL) && (*stoparea <= *area)) return;

  (*area)++;
  M[l]=1;   /* This point is marked */
  if (O!=NULL) O[l]=U[l];

  /* Left neighbor */
  k=l-1;
  if ((x>0) && (M[k]==0) && (a<=U[k]) && (U[k]<=b))
    compute_area(connex8, U, O, M, nrow, ncol, a, b, x-1, y, k, stoparea, area);

  /* Upper neighbor */
  k=l-ncol;
  if ((y>0) && (M[k]==0) && (a<=U[k]) && (U[k]<=b))
    compute_area(connex8, U, O, M, nrow, ncol, a, b, x, y-1, k, stoparea, area);

  /* Right neighbor */
  k=l+1;
  if ((x<ncol-1) && (M[k]==0) && (a<=U[k]) && (U[k]<=b))
    compute_area(connex8, U, O, M, nrow, ncol, a, b, x+1, y, k, stoparea, area);

  /* Lower neighbor */
  k=l+ncol;
  if ((y<nrow-1) && (M[k]==0) && (a<=U[k]) && (U[k]<=b))
    compute_area(connex8, U, O, M, nrow, ncol, a, b, x, y+1, k, stoparea, area);

  if (connex8 != NULL)
    {
      /* Upper left neighbor */
      k=l-ncol-1;
      if ((x>0) && (y>0) && (M[k]==0) && (a<=U[k]) && (U[k]<=b))
	compute_area(connex8, U, O, M, nrow, ncol, a, b, x-1, y-1, k, stoparea, area);
      /* Upper right neighbor */      
      k=l-ncol+1;
      if ((x<ncol-1) && (y>0) && (M[k]==0) && (a<=U[k]) && (U[k]<=b))
	compute_area(connex8, U, O, M, nrow, ncol, a, b, x+1, y-1, k, stoparea, area);
      /* Lower left neighbor */
      k=l+ncol-1;
      if ((x>0) && (y<nrow-1) && (M[k]==0) && (a<=U[k]) && (U[k]<=b))
	compute_area(connex8, U, O, M, nrow, ncol, a, b, x-1, y+1, k, stoparea, area);
      /* Lower right neighbor */      
      k=l+ncol+1;
      if ((x<ncol-1) && (y<nrow-1) && (M[k]==0) && (a<=U[k]) && (U[k]<=b))
	compute_area(connex8, U, O, M, nrow, ncol, a, b, x+1, y+1, k, stoparea, area);
    }
}


int mscarea(connex8, U, O, stoparea, a, b, x0, y0)
     char *connex8;
     Cimage U,O;
     int a,b,x0,y0;
     int *stoparea;
{
  Cimage M=NULL; /* Cimage to mark the pixels */
  int area,l,c;
  unsigned char *Og;

  if ((x0 < 0) || (x0 >= U->ncol)) 
  mwerror(FATAL,1,"m.s. {%d <= U <= %d} at point (%d,%d) : coordinate x0=%d out of image U.\n",a,b,x0,y0,x0);
  if ((y0 < 0) || (y0 >= U->nrow)) 
  mwerror(FATAL,1,"m.s. {%d <= U <= %d} at point (%d,%d) : coordinate y0=%d out of image U.\n",a,b,x0,y0,y0);
  if (a > b)   mwerror(FATAL,1,"m.s. {%d <= U <= %d} at point (%d,%d) : bad values a and b.\n",a,b,x0,y0);

  if (O!=NULL) 
    {
      O=mw_change_cimage(O, U->nrow, U->ncol);
      if (O==NULL) mwerror(FATAL,1,"Not enough memory.\n");
      mw_clear_cimage(O,0);
      Og=O->gray;
    }
  else Og = NULL;

  l=y0*U->ncol+x0;
  c = U->gray[l];
  if ((a > c) || (c > b)) return(0);

  M=mw_change_cimage(M, U->nrow, U->ncol);
  if (M==NULL) mwerror(FATAL,1,"Not enough memory.\n");
  mw_clear_cimage(M,0);

  area=0;
  compute_area(connex8, U->gray, Og, M->gray, U->nrow, U->ncol, 
	       a, b, x0, y0, l, stoparea, &area);

  mw_delete_cimage(M);
  return(area);
}


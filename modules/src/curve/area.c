/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {area};
   author = {"Lionel Moisan"};
   version = {"1.0"};
   function = {"Compute the (algebraic) area of a curve (Dlist)"};
   usage = {    
      in->in      "input curve (Dlist)",
      out<-area   "result (double)"
   };
*/

#include "mw.h"

/* determinant of an affine basis = det(b-a,c-a) */
#define DET3(ax,ay,bx,by,cx,cy) ((bx-ax)*(cy-ay) - (by-ay)*(cx-ax))

double area(Dlist in)
{
  double  *first,*p,a;
  int     d,i;
  
  if (!in || in->size<3) return(0.);
  p = first = in->values;
  a = 0.;
  d = in->dim;
  if (d<2) mwerror(FATAL,1,"Not a curve: dim < 2\n");
  for (i=in->size-1;i--;p+=d) 
    a += DET3(*first,*(first+1),*p,*(p+1),*(p+d),*(p+d+1));
  return(.5*a);
}

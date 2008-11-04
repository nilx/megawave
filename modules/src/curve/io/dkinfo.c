/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {dkinfo};
   author = {"Lionel Moisan"};
   version = {"1.0"};
   function = {"Informations on a set of curves (Dlists)"};
   usage = {    
      in->in    "input curves (Dlists)"
   };
*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for area(), perimeter(), dsplit_convex() */

static int is_closed(c)
     Dlist c;
{
  if (c->size<2) return(0);
  else return( (c->values[0]==c->values[c->dim*(c->size-1)  ]) &&
	       (c->values[1]==c->values[c->dim*(c->size-1)+1]) );
}

void dkinfo(in)
     Dlists in;
{
  Dlists res;
  Dlist c;
  int i;
  double min_dist,max_dist,per,eps;
  int ncc;

  if (!in) return;
  if (in->size>1) {
    printf("\n-----------------------------------");
    printf("-----------------------------------\n");
    printf("Number of curves: %d\n",in->size);
    printf("-----------------------------------");
    printf("-----------------------------------\n");
  } else printf("\n");

  /* loop on curves */

  for (i=0;i<in->size;i++) {
    c = in->list[i];
    if (in->size>1) printf("\n----- Curve # %d -----\n",i+1);
    if (!c || c->size==0) printf("Empty curve\n"); 
    else
      if (c->dim<2) printf("This is not a curve (dim=%d)\n",c->dim); 
      else {
	printf("This is a %s curve with %d point%s",
	       is_closed(c)?"closed":"non-closed",c->size,(c->size<2?"":"s"));
	if (c->dim>2) printf(" (dimension %d)\n",c->dim); else printf("\n");
	if (c->size>=2) {
	  per = perimeter(c,&min_dist,&max_dist);
	  printf("Minimum step distance: %g\n",min_dist);
	  printf("Maximum step distance: %g\n",max_dist);
	  printf("Average step distance: %g\n",per/(double)(c->size-1));
	  printf("Perimeter: %g\n",per);
	  if (c->size>=3) printf("Algebraic Area: %g\n",area(c));
	  eps = 1e-15;
	  res = dsplit_convex(c,NULL,&ncc,&eps); 
	  printf("Number of convex components (double resolution): %d\n",
		 ncc);
	  eps = 1e-7;
	  res = dsplit_convex(c,res,&ncc,&eps); 
	  mw_delete_dlists(res);
	  printf("Number of convex components (float resolution) : %d\n",
		 ncc);
	}
      }
  }
  printf("\n");
}


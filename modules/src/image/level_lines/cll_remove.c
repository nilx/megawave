/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {cll_remove};
 version = {"1.0"};
 author = {"Jacques Froment"};
 function = {"Remove small level lines in a color mimage"};
 usage = {
   'l':[L=10]->L       "Minimal level lines length",
   input->cmimage      "Original cmimage",
   output<-cll_remove  "Output cmimage with missing level lines"
};
*/

#include <stdio.h>
#include "mw.h"

void check_ll(ll)

Cmorpho_line ll;

{
  Point_curve point;
  Point_type type;
  int np,nt;
 
  np=nt=0;
  for (point=ll->first_point; point; point=point->next) np++;
  for (type=ll->first_type; type; type=type->next) nt++;
  printf("check_ll : np=%d  nt=%d\n",np,nt);
}

void remove_cll(cmimage,ll)

Cmimage cmimage;
Cmorpho_line ll;

{
  Cmorpho_line ll0,ll1;

  ll0 = ll->previous;
  ll1 = ll->next;
  
  if (ll0 == NULL) cmimage->first_ml = ll1;
  else ll0->next = ll1;
  if (ll1 != NULL) ll1->previous = ll0;
  /*
   mw_delete_cmorpho_line(ll);

   The free() call into the function mw_delete_point_curve()
   may cause inconsistent memory in the cmimage structure !!! (why ?)
  */
}

Cmimage cll_remove(cmimage,L)
Cmimage cmimage;
int *L;

{
  Cmorpho_line ll,ll_next;
  unsigned long N,Nrm;
  float infty = MORPHO_INFTY, max,min;
  int ll_type;

  if((ll=cmimage->first_ml)==NULL)
    mwerror(FATAL,1,"No cmorpho_lines in cmimage.\n");

  if ((ll->minvalue.red == -infty)&&
      (ll->minvalue.green == -infty)&&
      (ll->minvalue.blue == -infty)) ll_type = 1;
  else 
    if ((ll->maxvalue.red == infty)&&
      (ll->maxvalue.green == infty)&&
      (ll->maxvalue.blue == infty)) ll_type = 0;
    else 
	mwerror(FATAL,1,"Input cmimage does not contain level lines but morpho lines !\n");


  N=Nrm=0;
  for (ll=cmimage->first_ml; ll; ll=ll_next, N++)
    {
      if ( ((ll_type == 0)&&(
			     (ll->maxvalue.red != infty) ||
			     (ll->maxvalue.green != infty) ||
			     (ll->maxvalue.blue != infty))) ||
	   ((ll_type == 1)&&(
			     (ll->minvalue.red != -infty) ||
			     (ll->minvalue.green != -infty) ||
			     (ll->minvalue.blue != -infty) )))
	mwerror(FATAL,1,"Input cmimage contains mixed types of level lines !\n");
      ll_next = ll->next;
      /*  check_ll(ll);*/
      if (mw_length_cmorpho_line(ll) < *L) {remove_cll(cmimage,ll); Nrm++; }
    }

  if (ll_type == 0)
    mwdebug("Removed %d level lines of length < %d (local maxima) over %d (%3.1f %%).\n\tRemain %d level lines of type {(x,y) / f(x,y) >= v}\n",Nrm,*L,N,(100.0*Nrm)/N,N-Nrm); 
  else
    mwdebug("Removed %d level lines of length < %d (local minima) over %d (%3.1f %%).\n\tRemain %d level lines of type {(x,y) / f(x,y) <= v}\n",Nrm,*L,N,(100.0*Nrm)/N,N-Nrm); 

  return(cmimage);
}




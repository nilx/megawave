/*------------------------- MegaWave2 Module -------------------------*/
/* mwcommand
name = {ll_remove};
version = {"0.0"};
author = {"Jacques Froment"};
function = {"Remove small level lines in a mimage"};
usage = {
  'l':[L=10]->L "Minimal level lines length",
  input->mimage  "Original mimage",
  output<-ll_remove "Output mimage with missing level lines"
  };
*/
/*-------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

void remove_ll(mimage,ll)

Mimage mimage;
Morpho_line ll;

{
  Morpho_line ll0,ll1;

  ll0 = ll->previous;
  ll1 = ll->next;
  
  if (ll0 == NULL) mimage->first_ml = ll1;
  else ll0->next = ll1;
  if (ll1 != NULL) ll1->previous = ll0;
  /*
   mw_delete_morpho_line(ll);

   The free() call into the function mw_delete_point_curve()
   may cause inconsistent memory in the mimage structure !!! (why ?)
  */
}

Mimage ll_remove(mimage,L)
Mimage mimage;
int *L;

{
  Morpho_line ll,ll_next;
  unsigned long N,Nrm;
  float infty = MORPHO_INFTY, max,min;
  int ll_type;

  if((ll=mimage->first_ml)==NULL)
    mwerror(FATAL,1,"No morpho_lines in mimage.\n");

  if (ll->minvalue == -infty) ll_type = 1;
  else 
    if (ll->maxvalue == infty) ll_type = 0;
    else 
      {
	mwdebug("minvalue=%f, maxvalue=%f,MORPHO_INFTY=%f\n",
		ll->minvalue,ll->maxvalue,MORPHO_INFTY);
	mwerror(FATAL,1,"Input mimage does not contain level lines but morpho lines !\n");
      }

  N=Nrm=0;
  for (ll=mimage->first_ml; ll; ll=ll_next, N++)
    {
      if ( ((ll_type == 0)&&(ll->maxvalue != infty)) ||
	  ((ll_type == 1)&&(ll->minvalue != -infty)) )
	mwerror(FATAL,1,"Input mimage contains mixed types of level lines !\n");
      ll_next = ll->next;
      if (mw_morpho_line_length(ll) < *L) {remove_ll(mimage,ll); Nrm++; }
    }

  if (ll_type == 0)
    mwdebug("Removed %d level lines of length < %d (local maxima) over %d (%3.1f %%).\n\tRemain %d level lines of type {(x,y) / f(x,y) >= v}\n",Nrm,*L,N,(100.0*Nrm)/N,N-Nrm); 
  else
    mwdebug("Removed %d level lines of length < %d (local minima) over %d (%3.1f %%).\n\tRemain %d level lines of type {(x,y) / f(x,y) <= v}\n",Nrm,*L,N,(100.0*Nrm)/N,N-Nrm); 

  return(mimage);
}




/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {flconcat};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Concatenates two flists"};
   usage = {          
        l1->l1    "1st input Flists",
	l2->l2    "2nd input Flists",
	out<-out  "output Flists"
   };
*/

#include <stdio.h>
#include "mw.h"

Flists flconcat(l1,l2,out)
     Flists l1,l2,out;
{
  int i,j;

  out = mw_change_flists(out,l1->size+l2->size,l1->size+l2->size);
  j = 0;
  for (i=0;i<l1->size;i++)
    out->list[j++] = l1->list[i];
  for (i=0;i<l2->size;i++)
    out->list[j++] = l2->list[i];
  l1->size = l2->size = 0;
  l1->max_size = l2->max_size = 0;

  return(out);
}

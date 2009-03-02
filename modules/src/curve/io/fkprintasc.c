/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fkprintasc};
   version = {"1.1"};
   author = {"Jacques Froment"};
   function = {"Print the content of a fcurves"};
   usage = {            
      in->fcvs    "input Fcurves"
   };
*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

void fkprintasc(Fcurves fcvs)
{
  Fcurve fcv;
  Point_fcurve  p;
  int i;
  
  for (fcv = fcvs->first, i=1; fcv; fcv=fcv->next, i++)
    {
      printf("\tFcurve #%d :\n",i);
      for (p=fcv->first; p; p=p->next) 
	printf("%g %g\n",p->x,p->y);
    }
}





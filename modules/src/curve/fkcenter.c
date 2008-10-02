/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fkcenter};
   version = {"1.2"};
   author = {"Thierry Cohignac, Lionel Moisan"};
   function = {"Compute the center of mass (barycenter) of a set of Fcurves"};
   usage = {            
            in->cs               "input (Fcurves)",
	    xg<-xg               "barycenter (x coordinate)",
	    yg<-yg               "barycenter (y coordinate)"
   };
*/

#include <stdio.h>
#include "mw.h"


void fkcenter(cs,xg,yg)
     Fcurves cs;
     float *xg,*yg;
{
  int           n;
  Fcurve        c;
  Point_fcurve  p;
  
  *xg = *yg = 0.0;
  n = 0;
  
  for (c=cs->first; c; c=c->next) 
    for (p=c->first; p; p=p->next) {
      
      *xg += p->x;
      *yg += p->y;
      
      n++;
    }
  
  *xg /= (float)n;
  *yg /= (float)n;
}





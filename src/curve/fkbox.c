/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fkbox};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Compute the bounding box of a Fcurves"};
   usage = {            
            in->cs               "input (Fcurves)",
	    xmin<-xmin           "minimum along X coordinate",
	    ymin<-ymin           "minimum along Y coordinate",
	    xmax<-xmax           "maximum along X coordinate",
	    ymax<-ymax           "maximum along Y coordinate"
   };
*/

#include <stdio.h>
#include "mw.h"


void fkbox(cs,xmin,ymin,xmax,ymax)
Fcurves cs;
float *xmin,*ymin,*xmax,*ymax;
{
  Fcurve        c;
  Point_fcurve  p;
  
  if (cs->first) if (cs->first->first) {
    *xmax = *xmin = cs->first->first->x;
    *ymax = *ymin = cs->first->first->y;
  }
  
  for (c=cs->first; c; c=c->next) 
    for (p=c->first; p; p=p->next) {
      
      if (p->x>*xmax) *xmax = p->x;
      if (p->x<*xmin) *xmin = p->x;
      if (p->y>*ymax) *ymax = p->y;
      if (p->y<*ymin) *ymin = p->y;
      
    }  
}




/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fkbox};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Compute the bounding box of a Fcurves"};
   usage = {            
            'z':z->z             "magnification factor to apply to the box",
            in->cs               "input (Fcurves)",
	    xmin<-xmin           "minimum along X coordinate",
	    ymin<-ymin           "minimum along Y coordinate",
	    xmax<-xmax           "maximum along X coordinate",
	    ymax<-ymax           "maximum along Y coordinate",
        {   box<-box             "create bounding box as a Fcurve"  }
   };
*/
/*----------------------------------------------------------------------
 v1.1: added z and box options (L.Moisan)
 ----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

Point_fcurve new_point(p,x,y)
Point_fcurve p;
float x,y;
{
  Point_fcurve q;
  
  q = mw_new_point_fcurve();
  q->x = x;
  q->y = y;
  q->previous = p;
  if (p) p->next = q;

  return q;
}

/*------------------------------ MAIN MODULE ------------------------------*/

void fkbox(cs,xmin,ymin,xmax,ymax,z,box)
Fcurves cs;
float *xmin,*ymin,*xmax,*ymax;
float *z;
Fcurve box;
{
  Fcurve        c;
  Point_fcurve  p;
  float tmp;

  if (cs->first && cs->first->first) {
    *xmax = *xmin = cs->first->first->x;
    *ymax = *ymin = cs->first->first->y;
  }
  
  /*** compute bounding box ***/
  for (c=cs->first; c; c=c->next) 
    for (p=c->first; p; p=p->next) {
      
      if (p->x>*xmax) *xmax = p->x;
      if (p->x<*xmin) *xmin = p->x;
      if (p->y>*ymax) *ymax = p->y;
      if (p->y<*ymin) *ymin = p->y;
      
    }  

  /*** magnify box if requested ***/
  if (z) {
    tmp   = 0.5*(*xmax + *xmin) - 0.5*(*z)*(*xmax - *xmin);
    *xmax = 0.5*(*xmax + *xmin) + 0.5*(*z)*(*xmax - *xmin);
    *xmin = tmp; 
    tmp   = 0.5*(*ymax + *ymin) - 0.5*(*z)*(*ymax - *ymin);
    *ymax = 0.5*(*ymax + *ymin) + 0.5*(*z)*(*ymax - *ymin);
    *ymin = tmp; 
  }
      
  /*** create Fcurve box if requested ***/
  if (box) {
    box->first = new_point(NULL,*xmin,*ymin);
    box->first->next = new_point(box->first,*xmin,*ymax);
    box->first->next->next = new_point(box->first->next,*xmax,*ymax);
    box->first->next->next->next 
      = new_point(box->first->next->next,*xmax,*ymin);
    box->first->next->next->next->next 
      = new_point(box->first->next->next->next,*xmin,*ymin);
    box->first->next->next->next->next->next = NULL;
  }
}




/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fkcrop};
   version = {"1.1"};
   author = {"Lionel Moisan"};
   function = {"Crop Fcurves with a rectangular box"};
   usage = {                        
            'b':box<-box         "store the rectangular box as a Fcurve",
            x1->X1               "first corner (x coordinate)",
            y1->Y1               "first corner (y coordinate)",
            x2->X2               "opposite corner (x coordinate)",
            y2->Y2               "opposite corner (y coordinate)",
	    in->cs               "input (Fcurves)",
            out<-fkcrop          "output (Fcurves)"
	    };
*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h"

static Point_fcurve new_point(Point_fcurve p, float x, float y)
{
  Point_fcurve q;
  
  q = mw_new_point_fcurve();
  q->x = x;
  q->y = y;
  q->previous = p;
  if (p) p->next = q;
  
  return q;
}


Fcurves fkcrop(float X1, float Y1, float X2, float Y2, Fcurves cs, Fcurve box)
{
  Fcurves out;
  Fcurve c,newc,cprev,*cnext;
  Point_fcurve p,newp,pprev=NULL,*pnext=NULL;
  float tmp;
  int count_p,count_c,newc_flag,close_flag;
  
  /*** prepare box ***/
  if (X2<X1) {tmp=X1; X1=X2; X2=tmp;}
  if (Y2<Y1) {tmp=Y1; Y1=Y2; Y2=tmp;}
  
  /*** create Fcurve box if requested ***/
  if (box) {
    box->first = new_point(NULL,X1,Y1);;
    box->first->next = new_point(box->first,X1,Y2);
    box->first->next->next = new_point(box->first->next,X2,Y2);
    box->first->next->next->next = new_point(box->first->next->next,X2,Y1);
    box->first->next->next->next->next 
      = new_point(box->first->next->next->next,X1,Y1);
    box->first->next->next->next->next->next = NULL;
  }
  
  /*** prepare structures ***/
  out = mw_new_fcurves();
  cnext = &(out->first);
  cprev = NULL;
  newc_flag = TRUE;
  close_flag = FALSE;
  count_c = count_p = 0;
  
  /*** main loop ***/
  for (c=cs->first;c;c=c->next) {
    for (p=c->first;p;p=p->next) {
      if (p->x>=X1 && p->x<=X2 && p->y>=Y1 && p->y<=Y2) {
	if (newc_flag) {
	  if (close_flag) *pnext = NULL; else close_flag = TRUE;
	  newc = mw_new_fcurve();
	  count_c++;
	  newc->previous = cprev;
	  cprev = *cnext = newc;
	  cnext = &(newc->next);
	  pnext = &(newc->first);
	  pprev = NULL;
	  newc_flag = FALSE;
	}
	newp = mw_new_point_fcurve();
	count_p++;
	newp->x = p->x;
	newp->y = p->y;
	newp->previous = pprev;
	pprev = *pnext = newp;
	pnext = &(newp->next);
      } else newc_flag = TRUE;
    }
    newc_flag = TRUE;
  }
  if (close_flag) *pnext = NULL;
  *cnext = NULL;
  
  mwdebug("%d components, %d points\n",count_c,count_p);
  
  return out;
}

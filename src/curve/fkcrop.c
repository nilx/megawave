/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fkcrop};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Crop Fcurves with a rectangular box"};
   usage = {                        
            'b':box<-box         "store the rectangular box as a Fcurve",
            x1->x1               "first corner (x coordinate)",
            y1->y1               "first corner (y coordinate)",
            x2->x2               "opposite corner (x coordinate)",
            y2->y2               "opposite corner (y coordinate)",
	    in->cs               "input (Fcurves)",
            out<-fkcrop          "output (Fcurves)"
	    };
   */
/*-- MegaWave - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

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


Fcurves fkcrop(x1,y1,x2,y2,cs,box)
float x1,y1,x2,y2;
Fcurves cs;
Fcurve box;
{
  Fcurves out;
  Fcurve c,newc,cprev,*cnext;
  Point_fcurve p,newp,pprev,*pnext;
  float tmp;
  int count_p,count_c,newc_flag,close_flag;
  
  /*** prepare box ***/
  if (x2<x1) {tmp=x1; x1=x2; x2=tmp;}
  if (y2<y1) {tmp=y1; y1=y2; y2=tmp;}
  
  /*** create Fcurve box if requested ***/
  if (box) {
    box->first = new_point(NULL,x1,y1);;
    box->first->next = new_point(box->first,x1,y2);
    box->first->next->next = new_point(box->first->next,x2,y2);
    box->first->next->next->next = new_point(box->first->next->next,x2,y1);
    box->first->next->next->next->next 
      = new_point(box->first->next->next->next,x1,y1);
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
      if (p->x>=x1 && p->x<=x2 && p->y>=y1 && p->y<=y2) {
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

/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {disk};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"build a disk as a Curve structure"};
  usage = {
     'r':[r=1.0]->r   "disk radius (default: 1.0)", 
     out<-disk        "output shape (Curve structure)"
          };
*/
/*-- MegaWave - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/
#include <stdio.h>
#include <math.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

Curve disk(r)
float *r;
{
  Curve        c;
  Point_curve  p,prev,*next;
  int          ir,dx,dy;
  
  c = mw_new_curve();
  c->previous = c->next = NULL;
  next = &(c->first);
  prev = NULL;

  ir = (int)ceil(*r);
  for (dx=-ir;dx<=ir;dx++)
    for (dy=-ir;dy<=ir;dy++) {
      if ( (float)(dx*dx+dy*dy)<=(*r)*(*r)) {
	p = mw_new_point_curve();
	p->x = dx;
	p->y = dy;
	p->previous = prev;
	*next = prev = p;
	next = &(p->next);
      }
    }
  *next = NULL;

  return c;
}


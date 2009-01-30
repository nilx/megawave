/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {disc};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"build a disc as a Curve structure"};
  usage = {
     'i':i->i         "inner radius (to produce a ring)",
     out<-disc        "output shape (Curve structure)",
     r->r             "disc radius"
          };
*/
/*----------------------------------------------------------------------
 v1.1: renamed, added -i option, removed -r option (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"


Curve disc(float r, float *i)
{
  Curve        c;
  Point_curve  p,prev,*next;
  int          ir,dx,dy;
  float        r2,d2,i2;
  
  c = mw_new_curve();
  if (!c) mwerror(FATAL,1,"Not enough memory\n");
  c->previous = c->next = NULL;
  next = &(c->first);
  prev = NULL;

  ir = (int)ceil(r);
  r2 = r*r;
  i2 = (i?*i*(*i):-1.);

  for (dx=-ir;dx<=ir;dx++)
    for (dy=-ir;dy<=ir;dy++) {
      d2 = (float)(dx*dx+dy*dy);
      if (d2<=r2 && d2>i2) {
	p = mw_new_point_curve();
	if (!p) mwerror(FATAL,1,"Not enough memory\n");
	p->x = dx;
	p->y = dy;
	p->previous = prev;
	*next = prev = p;
	next = &(p->next);
      }
    }
  *next = NULL;

  return(c);
}


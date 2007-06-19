/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {kreadfig};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Read on standard input XFIG 3.2 polygons (closed or not)"};
   usage = { 
       out<-kreadfig   "result (Curves)"
   };
*/

#include <stdio.h>
#include "mw.h"

Curves kreadfig()
{
  Curves       cs;
  Curve        c,cprev,*cnext;
  Point_curve  p,pprev,*pnext;
  int          i,x,y,n,id1,id2,ok,nc;

  cs = mw_new_curves();
  cprev = NULL;
  cnext = &(cs->first);
  nc = 0;
  
  /*** skip XFIG 3.2 header ***/
  scanf("%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s");

  do {
    id1 = 0;
    scanf("%d %d",&id1,&id2);
    ok = ((id1==2) && (id2==1 || id2==3));
    if (ok) {
      scanf("%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%d",&n);
      c = mw_new_curve();
      nc++;
      c->previous = cprev;
      *cnext = cprev = c;
      cnext = &(c->next);
      pprev = NULL;
      pnext = &(c->first);
      for (i=0;i<n;i++) {
	scanf("%d%d",&x,&y);
	p = mw_new_point_curve();
	p->x = x; p->y = y;
	p->previous = pprev;
	*pnext = pprev = p;
	pnext = &(p->next);
      }
      if (id2==3) {
	/*** close curve ***/
	p = mw_new_point_curve();
	p->x = c->first->x; p->y = c->first->y;
	p->previous = pprev;
	*pnext = pprev = p;
	pnext = &(p->next);
      }
      *pnext = NULL;
    }
  } while (ok);
  *cnext = NULL;
  printf("%d curves read.\n",nc);

  return cs;
}


/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {kplot};
  version = {"1.6"};
  author = {"Jacques Froment"};
  function = {"Bitmap the geometry of Curves onto a cimage"};
  usage = {
  'i':cimage_in->A  "optional background Cimage (input)",
  'l'->line         "draw a line between successive points of a curve",
  'd':[d=1]->d      "d=0: draw black lines, d=1: add white surround (default)",
  curves_in->curves "set of Curves (input)",
  cimage_out<-B     "bitmapped Cimage (output)"
  };
*/
/*----------------------------------------------------------------------
 v1.5: no more translation in case of negative points (L.Moisan) 
 v1.6: added -d option (L.Moisan) 
 ----------------------------------------------------------------------*/


#include <stdio.h>
#include "mw.h"

/* Compute the bounding box of a Curves */

void bound_curves(cs,xmin,ymin,xmax,ymax)
Curves cs;
int *xmin,*ymin,*xmax,*ymax;
{
  Curve        c;
  Point_curve  p;
  
  if (!cs->first) mwerror(INTERNAL,1,"[bound_curves] NULL curves\n");
  if (!cs->first->first) mwerror(INTERNAL,1,"[bound_curves] NULL curve\n");

  *xmax = *xmin = cs->first->first->x;
  *ymax = *ymin = cs->first->first->y;

  for (c=cs->first; c; c=c->next) 
    for (p=c->first; p; p=p->next) {
      
      if (p->x>*xmax) *xmax = p->x;
      if (p->x<*xmin) *xmin = p->x;
      if (p->y>*ymax) *ymax = p->y;
      if (p->y<*ymin) *ymin = p->y;
      
    }  
}


void bitmap_curve_with_lines(curve,A,B,xmin,ymin,mode)

Curve curve;
Cimage A,B;
int xmin,ymin,mode;

{ 
  register Point_curve p;
  register int x0,y0;

  if (curve == NULL)
    {
      mwdebug("[bitmap_curve_with_lines] NULL input curve !\n");
      return;
    }

  p=curve->first;
  x0 = p->x;
  y0 = p->y;

  if (p->next == NULL)  /* Only one point in the curve */
    {
      mw_plot_cimage(B, x0-xmin, y0-ymin, 0);
      if (A && mode==1)
	{
	  if (y0-ymin < B->nrow/2) 
	    mw_plot_cimage(B, x0-xmin, y0-ymin+1, 255);
	  else
	    mw_plot_cimage(B, x0-xmin, y0-ymin-1, 255);
	  mwdebug(
    "[bitmap_curve_with_lines] Only one point (%d,%d) in the input curve !\n",
		  x0,y0);
	}  
    }
  else
    for (p=p->next; p; p=p->next)
      {
	mwdebug("Line (%d,%d)-(%d,%d)\n",x0,y0,p->x,p->y);
	mw_draw_cimage(B, x0-xmin, y0-ymin, p->x-xmin, p->y-ymin, 0);
	if (A && mode==1)
	  {
	    if (abs(x0-p->x-xmin) >= abs(y0-p->y-ymin)) 
	      mw_draw_cimage(B, x0-xmin,y0+1-ymin,p->x-xmin,p->y-ymin+1,255);
	    else 
	      mw_draw_cimage(B, x0-xmin+1,y0-ymin,p->x-xmin+1,p->y-ymin,255);
	  }
	x0 = p->x;
	y0 = p->y;
      }
}

void bitmap_curve(curve,A,B,xmin,ymin)

Curve curve;
Cimage A,B;
int xmin,ymin;

{ 
  register Point_curve p;

  if (curve == NULL)
    {
      mwdebug("[bitmap_curve] NULL input curve !\n");
      return;
    }

  for (p=curve->first; p; p=p->next) 
    mw_plot_cimage(B, p->x-xmin, p->y-ymin,0);
}


void kplot(A,line,curves,B,d)

Cimage A,B;
char *line;
Curves curves;
int *d;

{ Curve curve;
  int i,nx,ny;
  int xmax,ymax,xmin,ymin;

  if (curves->first == NULL) mwerror(FATAL,1,"No curve found !\n");
  bound_curves(curves,&xmin,&ymin,&xmax,&ymax);
 
  if (A == NULL)
    {
      nx = xmax-xmin+1;
      ny = ymax-ymin+1;
      mwdebug("Bound of curves: (%d,%d)-(%d,%d). Dim= %d x %d\n",xmin,ymin,
	      xmax,ymax,nx,ny);
      B = mw_change_cimage(B,ny,nx);
      if (!B) mwerror(FATAL,1,"cannot allocate a %d x %d image\n",nx,ny);
      mw_clear_cimage(B,255);
     } 
  else
    {
      B = mw_change_cimage(B,A->nrow,A->ncol);
      if (B == NULL) mwerror(FATAL,1,"Not enough memory\n");
      mw_copy_cimage(A,B);
      /*if (xmin < 0) xmin += (A->ncol/2);
	if (ymin < 0) ymin += (A->nrow/2);*/
      xmin = ymin = 0;
    }  

  if (line)
    for (curve=curves->first, i=1; curve; curve=curve->next,i++)
      {
	mwdebug("Curve #%d\n",i);
	if (curve->first != NULL)  
	  bitmap_curve_with_lines(curve,A,B,xmin,ymin,*d);
	else mwdebug("Empty curve found !\n");
      }
  else
    for (curve=curves->first, i=1; curve; curve=curve->next,i++)
      {
	mwdebug("Curve #%d\n",i);
	if (curve->first != NULL)  bitmap_curve(curve,A,B,xmin,ymin);
	else mwdebug("Empty curve found !\n");
      }
}








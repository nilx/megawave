/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   curve.c
   
   Vers. 1.5
   (C) 1993-99 Jacques Froment
   Basic memory routines for the curve internal type

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>

#include "mw.h"

/* creates a new point_curve structure */

Point_curve mw_new_point_curve()
{
  Point_curve point;

  if(!(point = (Point_curve) (malloc(sizeof(struct point_curve)))))
    {
      mwerror(ERROR, 0, "[mw_new_point_curve] Not enough memory\n");
      return(NULL);
    }
  point->x = point->y = -1;
  point->previous = NULL;
  point->next = NULL;

  return(point);
}

/* Define the struct if it's not defined */

Point_curve mw_change_point_curve(point)
     Point_curve point;
{
  if (point == NULL) point = mw_new_point_curve();
  return(point);
}

/* desallocate all the point_curve structures from a starting point */

void mw_delete_point_curve(point)
     Point_curve point;

{
  Point_curve point_next,point_first;

  if (point == NULL)
    {
      mwerror(ERROR, 0,
	      "[mw_delete_point_curve] cannot delete : point_curve structure is NULL\n");
      return;
    }

  point_first=point;
  do
    {
      point_next = point->next;
      free(point);
      point = point_next;
    } while ((point != NULL)&&(point != point_first));
}

/* Copy all the points of a curve starting from a Point_curve into another chain */

void mw_copy_point_curve(in, out)

Point_curve in,out;

{ 
  Point_curve pc,qc0,qc1;

  if ((!in) || (!out))
    {
      mwerror(ERROR, 0,
	      "[mw_copy_point_curve] NULL input or output point_curve\n");
      return;
    }

  out->x = in->x;
  out->y = in->y;
  out->previous = NULL;

  qc0 = out;
  qc1 = NULL;
  for (pc=in->next; pc; pc=pc->next)
    {
      qc1 = mw_new_point_curve();
      if (qc1 == NULL)
	    {
	      mw_delete_point_curve(qc1);
	      mwerror(FATAL, 1,"Not enough memory to create a point_curve\n");
	      return;
	    }
      qc1->x = pc->x;
      qc1->y = pc->y;
      qc1->previous = qc0;
      qc0->next = qc1;
      qc0 = qc1;
    }
}
/* ----- */

/* Creates a new curve structure */

Curve mw_new_curve()

{
  Curve curve;

  if(!(curve = (Curve) (malloc(sizeof(struct curve)))))
    {
      mwerror(ERROR, 0, "[mw_new_curve] Not enough memory\n");
      return(NULL);
    }
  
  curve->first = NULL;
  curve->previous = NULL;
  curve->next = NULL;
  return(curve);
}

/* Define the struct if it's not defined */

Curve mw_change_curve(cv)

Curve cv;

{
  if (cv == NULL) cv = mw_new_curve();
  return(cv);
}

/* desallocate the curve structure */

void mw_delete_curve(curve)
     Curve curve;

{
  if (curve == NULL)
    {
      mwerror(ERROR, 0,
	      "[mw_delete_curve] cannot delete : curve structure is NULL\n");
      return;
    }

  mw_delete_point_curve(curve->first);
  free(curve);
  curve=NULL;
}

/* Copy a curve into another curve */

void mw_copy_curve(in, out)

Curve in,out;

{ 
  Point_curve pc,qc0,qc1;

  if ((!in) || (!out))
    {
      mwerror(ERROR, 0,
	      "[mw_copy_curve] NULL input or output curve\n");
      return;
    }

  out->first = mw_new_point_curve();
  if (out->first == NULL)
    {
      mw_delete_point_curve(out->first);
      mwerror(FATAL, 1,"Not enough memory to create a curve\n");
      return;
    }
  mw_copy_point_curve(in->first, out->first);
}

/* Return the number of point into a curve */

unsigned int mw_curve_length(curve)

Curve curve;

{ 
  unsigned int n;
  Point_curve p,pfirst;

  if ((!curve) || (!curve->first)) return(0);

  for (p=pfirst=curve->first, n=0; 
       (p != NULL)&&(p->next != pfirst); n++, p=p->next);
  return(n);
}

/* ----- */

/* creates a new curves structure */

Curves mw_new_curves()
{
  Curves curves;

  if(!(curves = (Curves) (malloc(sizeof(struct curves)))))
    {
      mwerror(ERROR, 0, "[mw_new_curves] Not enough memory\n");
      return(NULL);
    }
  curves->first = NULL;
  strcpy(curves->cmt,"?");
  strcpy(curves->name,"?");
  return(curves);
}

/* Define the struct if it's not defined */

Curves mw_change_curves(cvs)
     Curves cvs;
{
  if (cvs == NULL) cvs = mw_new_curves();
  return(cvs);
}


/* desallocate the curves structure */

void mw_delete_curves(curves)
     Curves curves;

{
  Curve cv, cv_next;

  if (curves == NULL)
    {
      mwerror(ERROR, 0,
	      "[mw_delete_curves] cannot delete : curves structure is NULL\n");
      return;
    }

  cv = curves->first;
  while (cv != NULL)
    {
      cv_next = cv->next;
      mw_delete_curve(cv);
      cv = cv_next;
    }

  free(curves);
  curves=NULL;
}


/* Return the number of curves into a curves */

unsigned int mw_curves_length(curves)

Curves curves;

{ 
  unsigned int n;
  Curve pfirst,p;

  if ((!curves) || (!curves->first)) return(0);

  for (p=pfirst=curves->first, n=0; 
       (p != NULL)&&((n==0)||(p != pfirst)); n++, p=p->next);
  return(n);
}

/* Return the total number of points into a curves */

unsigned int mw_curves_npoints(curves)

Curves curves;

{ 
  unsigned int m,n;
  Curve pfirst,p;

  if ((!curves) || (!curves->first)) return(0);

  n=0;
  for (p=pfirst=curves->first, m=0;
       (p != NULL)&&((m==0)||(p != pfirst)); m++, p=p->next)
	 n += mw_curve_length(p);
  return(n);
}






/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   fcurve.c
   
   Vers. 1.5
   (C) 1995-2002 Jacques Froment
   Basic memory routines for the fcurve internal type

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

/* creates a new point_fcurve structure */

Point_fcurve mw_new_point_fcurve()
{
  Point_fcurve point;

  if(!(point = (Point_fcurve) (malloc(sizeof(struct point_fcurve)))))
    {
      mwerror(ERROR, 0, "[mw_new_point_fcurve] Not enough memory\n");
      return(NULL);
    }
  point->x = point->y = -1.0;
  point->previous = NULL;
  point->next = NULL;

  return(point);
}

/* Define the struct if it's not defined */

Point_fcurve mw_change_point_fcurve(point)
     Point_fcurve point;
{
  if (point == NULL) point = mw_new_point_fcurve();
  return(point);
}

/* deallocate all the point_fcurve structures from a starting point */

void mw_delete_point_fcurve(point)
     Point_fcurve point;

{
  Point_fcurve point_next,point_first;

  if (point == NULL)
    {
      mwerror(ERROR, 0,
	      "[mw_delete_point_fcurve] cannot delete : point_fcurve structure is NULL\n");
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

/* Copy all the points of a fcurve starting from a Point_fcurve into another chain */

Point_fcurve mw_copy_point_fcurve(in, out)

Point_fcurve in,out;

{ 
  Point_fcurve pc,qc0,qc1;

  if (!in)
    {
      mwerror(ERROR, 0,"[mw_copy_point_fcurve] NULL input point_fcurve\n");
      return(NULL);
    }

  if (!out)
    {
      out = mw_new_point_fcurve();
      if (!out) mwerror(ERROR, 0,"[mw_copy_point_fcurve] Not enough memory to create a point_fcurve\n");
      return(NULL);      
    }

  out->x = in->x;
  out->y = in->y;
  out->previous = NULL;

  qc0 = out;
  qc1 = NULL;
  for (pc=in->next; pc; pc=pc->next)
    {
      qc1 = mw_new_point_fcurve();
      if (qc1 == NULL)
	    {
	      mw_delete_point_fcurve(qc1);
	      mwerror(ERROR, 0,"[mw_copy_point_fcurve] Not enough memory to create a point_fcurve\n");
	      return(NULL);
	    }
      qc1->x = pc->x;
      qc1->y = pc->y;
      qc1->previous = qc0;
      qc0->next = qc1;
      qc0 = qc1;
    }
  return(out);
}

/* ----- */

/* Creates a new fcurve structure */

Fcurve mw_new_fcurve()

{
  Fcurve fcurve;

  if(!(fcurve = (Fcurve) (malloc(sizeof(struct fcurve)))))
    {
      mwerror(ERROR, 0, "[mw_new_fcurve] Not enough memory\n");
      return(NULL);
    }
  
  fcurve->first = NULL;
  fcurve->previous = NULL;
  fcurve->next = NULL;
  return(fcurve);
}

/* Define the struct if it's not defined */

Fcurve mw_change_fcurve(cv)

Fcurve cv;

{
  if (cv == NULL) cv = mw_new_fcurve();
  return(cv);
}

/* deallocate the fcurve structure */

void mw_delete_fcurve(fcurve)
     Fcurve fcurve;

{
  if (fcurve == NULL)
    {
      mwerror(ERROR, 0,
	      "[mw_delete_fcurve] cannot delete : fcurve structure is NULL\n");
      return;
    }

  mw_delete_point_fcurve(fcurve->first);
  free(fcurve);
  fcurve=NULL;
}

/* Copy a fcurve into another fcurve */

Fcurve mw_copy_fcurve(in, out)

Fcurve in,out;

{ 
  Point_fcurve pc,qc0,qc1;

  if (!in)
    {
      mwerror(ERROR, 0,"[mw_copy_fcurve] NULL input fcurve\n");
      return(NULL);
    }

  if (!out)
    {
      out=mw_new_fcurve();
      if (!out) 
	{
	  mwerror(ERROR,0,"[mw_copy_fcurve] Not enough memory to create a fcurve !\n");
	  return(NULL);
	}
    }

  out->first = mw_new_point_fcurve();
  if (out->first == NULL)
    {
      mw_delete_point_fcurve(out->first);
      mwerror(ERROR, 0,"[mw_copy_fcurve] Not enough memory to copy the fcurve\n");
      return(NULL);
    }
  if (!mw_copy_point_fcurve(in->first, out->first))
    {
      mwerror(ERROR, 0,"[mw_copy_fcurve] Not enough memory to copy the fcurve\n");
      return(NULL);
    }
  return(out);
}

/* Return the number of point into a fcurve */

unsigned int mw_length_fcurve(fcurve)

Fcurve fcurve;

{ 
  unsigned int n;
  Point_fcurve p,pfirst;

  if ((!fcurve) || (!fcurve->first)) return(0);

  for (p=pfirst=fcurve->first, n=0; 
       (p != NULL)&&(p->next != pfirst); n++, p=p->next);
  return(n);
}

/* ----- */

/* creates a new fcurves structure */

Fcurves mw_new_fcurves()
{
  Fcurves fcurves;

  if(!(fcurves = (Fcurves) (malloc(sizeof(struct fcurves)))))
    {
      mwerror(ERROR, 0, "[mw_new_fcurves] Not enough memory\n");
      return(NULL);
    }
  fcurves->first = NULL;
  strcpy(fcurves->cmt,"?");
  strcpy(fcurves->name,"?");
  return(fcurves);
}

/* Define the struct if it's not defined */

Fcurves mw_change_fcurves(cvs)
     Fcurves cvs;
{
  if (cvs == NULL) cvs = mw_new_fcurves();
  return(cvs);
}


/* deallocate the fcurves structure */

void mw_delete_fcurves(fcurves)
     Fcurves fcurves;

{
  Fcurve cv, cv_next;

  if (fcurves == NULL)
    {
      mwerror(ERROR, 0,
	      "[mw_delete_fcurves] cannot delete : fcurves structure is NULL\n");
      return;
    }

  cv = fcurves->first;
  while (cv != NULL)
    {
      cv_next = cv->next;
      mw_delete_fcurve(cv);
      cv = cv_next;
    }

  free(fcurves);
  fcurves=NULL;
}

/* Return the number of fcurves into a fcurves */

unsigned int mw_length_fcurves(fcurves)

Fcurves fcurves;

{ 
  unsigned int n;
  Fcurve pfirst,p;

  if ((!fcurves) || (!fcurves->first)) return(0);

  for (p=pfirst=fcurves->first, n=0; 
       (p != NULL)&&((n==0)||(p != pfirst)); n++, p=p->next);
  return(n);
}

/* Return the total number of points into a fcurves */

unsigned int mw_npoints_fcurves(fcurves)

Fcurves fcurves;

{ 
  unsigned int m,n;
  Fcurve pfirst,p;

  if ((!fcurves) || (!fcurves->first)) return(0);

  n=0;
  for (p=pfirst=fcurves->first, m=0; 
       (p != NULL)&&((m==0)||(p != pfirst)); m++, p=p->next)
	 n += mw_length_fcurve(p);
  return(n);
}





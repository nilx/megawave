/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  dcurve.c
   
  Vers. 1.2
  Author : Jacques Froment
  Basic memory routines for the Dcurve internal type

  Main changes :
  v1.2 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdlib.h>
#include <string.h>

#include "libmw.h"
#include "utils.h"

#include "dcurve.h"

/* creates a new point_dcurve structure */

Point_dcurve mw_new_point_dcurve(void)
{
     Point_dcurve point;

     if(!(point = (Point_dcurve) (malloc(sizeof(struct point_dcurve)))))
     {
	  mwerror(ERROR, 0, "[mw_new_point_dcurve] Not enough memory\n");
	  return(NULL);
     }
     point->x = point->y = -1.0;
     point->previous = NULL;
     point->next = NULL;

     return(point);
}

/* Define the struct if it's not defined */

Point_dcurve mw_change_point_dcurve(Point_dcurve point)
{
     if (point == NULL) point = mw_new_point_dcurve();
     return(point);
}

/* desallocate all the point_dcurve structures from a starting point */

void mw_delete_point_dcurve(Point_dcurve point)
{
     Point_dcurve point_next,point_first;

     if (point == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_point_dcurve] cannot delete : point_dcurve structure is NULL\n");
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

/* Copy all the points of a dcurve starting from a Point_dcurve into another chain */

Point_dcurve mw_copy_point_dcurve(Point_dcurve in, Point_dcurve out)
{ 
     Point_dcurve pc,qc0,qc1;

     if (!in)
     {
	  mwerror(ERROR, 0,"[mw_copy_point_dcurve] NULL input point_dcurve\n");
	  return(NULL);
     }

     if (!out)
     {
	  out = mw_new_point_dcurve();
	  if (!out) mwerror(ERROR, 0,"[mw_copy_point_dcurve] Not enough memory to create a point_curve\n");
	  return(NULL);      
     }
  
     out->x = in->x;
     out->y = in->y;
     out->previous = NULL;

     qc0 = out;
     qc1 = NULL;
     for (pc=in->next; pc; pc=pc->next)
     {
	  qc1 = mw_new_point_dcurve();
	  if (qc1 == NULL)
	  {
	       mw_delete_point_dcurve(qc1);
	       mwerror(ERROR, 0,"[mw_copy_point_dcurve] Not enough memory to copy the point_dcurve\n");
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

/* Creates a new dcurve structure */

Dcurve mw_new_dcurve(void)
{
     Dcurve dcurve;

     if(!(dcurve = (Dcurve) (malloc(sizeof(struct dcurve)))))
     {
	  mwerror(ERROR, 0, "[mw_new_dcurve] Not enough memory\n");
	  return(NULL);
     }
  
     dcurve->first = NULL;
     dcurve->previous = NULL;
     dcurve->next = NULL;
     return(dcurve);
}

/* Define the struct if it's not defined */

Dcurve mw_change_dcurve(Dcurve cv)
{
     if (cv == NULL) cv = mw_new_dcurve();
     return(cv);
}

/* desallocate the dcurve structure */

void mw_delete_dcurve(Dcurve dcurve)
{
     if (dcurve == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_dcurve] cannot delete : dcurve structure is NULL\n");
	  return;
     }

     mw_delete_point_dcurve(dcurve->first);
     free(dcurve);
     dcurve=NULL;
}

/* Copy a dcurve into another dcurve */

Dcurve mw_copy_dcurve(Dcurve in, Dcurve out)
{ 
     if (!in)
     {
	  mwerror(ERROR, 0,"[mw_copy_dcurve] NULL input dcurve\n");
	  return(NULL);
     }

     if (!out)
     {
	  out=mw_new_dcurve();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_dcurve] Not enough memory to create a dcurve !\n");
	       return(NULL);
	  }
     }

     out->first = mw_new_point_dcurve();
     if (out->first == NULL)
     {
	  mw_delete_point_dcurve(out->first);
	  mwerror(ERROR, 0,"[mw_copy_dcurve] Not enough memory to copy the dcurve\n");
	  return(NULL);
     }
     if (!mw_copy_point_dcurve(in->first, out->first))
     {
	  mwerror(ERROR, 0,"[mw_copy_dcurve] Not enough memory to copy the dcurve\n");
	  return(NULL);
     }
     return(out);
}

/* Return the number of point into a dcurve */

unsigned int mw_length_dcurve(Dcurve dcurve)
{ 
     unsigned int n;
     Point_dcurve p,pfirst;

     if ((!dcurve) || (!dcurve->first)) return(0);

     for (p=pfirst=dcurve->first, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);
     return(n);
}

/* ----- */

/* creates a new dcurves structure */

Dcurves mw_new_dcurves(void)
{
     Dcurves dcurves;

     if(!(dcurves = (Dcurves) (malloc(sizeof(struct dcurves)))))
     {
	  mwerror(ERROR, 0, "[mw_new_dcurves] Not enough memory\n");
	  return(NULL);
     }
     dcurves->first = NULL;
     strcpy(dcurves->cmt,"?");
     strcpy(dcurves->name,"?");
     return(dcurves);
}

/* Define the struct if it's not defined */

Dcurves mw_change_dcurves(Dcurves cvs)
{
     if (cvs == NULL) cvs = mw_new_dcurves();
     return(cvs);
}


/* desallocate the dcurves structure */

void mw_delete_dcurves(Dcurves dcurves)
{
     Dcurve cv, cv_next;

     if (dcurves == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_dcurves] cannot delete : dcurves structure is NULL\n");
	  return;
     }

     cv = dcurves->first;
     while (cv != NULL)
     {
	  cv_next = cv->next;
	  mw_delete_dcurve(cv);
	  cv = cv_next;
     }

     free(dcurves);
     dcurves=NULL;
}

/* Return the number of dcurves into a dcurves */

unsigned int mw_length_dcurves(Dcurves dcurves)
{ 
     unsigned int n;
     Dcurve pfirst,p;

     if ((!dcurves) || (!dcurves->first)) return(0);

     for (p=pfirst=dcurves->first, n=0; 
	  (p != NULL)&&((n==0)||(p != pfirst)); n++, p=p->next);
     return(n);
}

/* Return the total number of points into a dcurves */

unsigned int mw_npoints_dcurves(Dcurves dcurves)
{ 
     unsigned int m,n;
     Dcurve pfirst,p;

     if ((!dcurves) || (!dcurves->first)) return(0);

     n=0;
     for (p=pfirst=dcurves->first, m=0; 
	  (p != NULL)&&((m==0)||(p != pfirst)); m++, p=p->next)
	  n += mw_length_dcurve(p);
     return(n);
}

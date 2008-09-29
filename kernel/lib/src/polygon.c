/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  polygon.c
   
  Vers. 1.14
  Author : Jacques Froment
  Basic memory routines for the polygon internal type

  Main changes :
  v1.14 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
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

#include "libmw-defs.h"
#include "utils.h"
#include "curve.h"

#include "polygon.h"

/* Creates a new polygon structure with 0 channel.                  */

Polygon mw_new_polygon(void)
{
     Polygon polygon;

     if(!(polygon = (Polygon) (malloc(sizeof(struct polygon)))))
     {
	  mwerror(ERROR, 0, "[mw_new_polygon] Not enough memory\n");
	  return(NULL);
     }
  
     polygon->nb_channels = 0;
     polygon->first = NULL;
     polygon->previous = NULL;
     polygon->next = NULL;
     return(polygon);
}

/* Allocates <nc> channels in the polygon.                              */
/* The meanning of channels is given by the user. Typically, if <nc>=1, */
/* channel[0] means gray level of the polygon.                          */

Polygon mw_alloc_polygon(Polygon polygon, int nc)
{
     int i;

     if (polygon == NULL)
     {
	  mwerror(ERROR, 0, 
		  "[mw_alloc_polygon] cannot alloc channels : polygon structure is NULL\n");
	  return(NULL);
     }
     if (nc <= 0)
     {
	  mwerror(ERROR, 0, "[mw_new_polygon] Illegal number of channels %d\n",nc);
	  return(NULL);      
     }

     if (!(polygon->channel = (float *) malloc(nc*sizeof(float))))
     {
	  mwerror(ERROR, 0, "[mw_new_polygon] Not enough memory\n");
	  return(NULL);
     }
     polygon->nb_channels = nc;
     for (i=0;i<nc;i++) polygon->channel[i] = -1.0;
     return(polygon);
}

/* Define the struct if it's not defined */
/* and/or change the number of channels <nc> */

Polygon mw_change_polygon(Polygon poly, int nc)
{
     if (poly == NULL) poly = mw_new_polygon();
     if (poly == NULL) return(NULL);

     if (poly->nb_channels >= nc)
	  poly->nb_channels = nc;
     else
     {
	  if (poly->nb_channels > 0) 
	  {
	       free(poly->channel);
	       poly->channel = 0;
	  } 
	  if (mw_alloc_polygon(poly,nc) == NULL)
	  {
	       mw_delete_polygon(poly);
	       return(NULL);
	  }
     }
     return(poly);
}

/* desallocate the polygon structure */

void mw_delete_polygon(Polygon polygon)
{
     if (polygon == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_polygon] cannot delete : polygon structure is NULL\n");
	  return;
     }

     mw_delete_point_curve(polygon->first);
     if (polygon->nb_channels > 0) free(polygon->channel);
     free(polygon);
     polygon=NULL;
}

/* Return the number of point into a polygon */

unsigned int mw_length_polygon(Polygon poly)
{ 
     unsigned int n;
     Point_curve p,pfirst;

     if ((!poly) || (!poly->first)) return(0);

     for (p=pfirst=poly->first, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);
     return(n);
}


/* ----- */

/* creates a new polygons structure */

Polygons mw_new_polygons(void)
{
     Polygons polygons;

     if(!(polygons = (Polygons) (malloc(sizeof(struct polygons)))))
     {
	  mwerror(ERROR, 0, "[mw_new_polygons] Not enough memory\n");
	  return(NULL);
     }
     polygons->first = NULL;
     strcpy(polygons->cmt,"?");
     strcpy(polygons->name,"?");
     return(polygons);
}

/* Define the struct if it's not defined */

Polygons mw_change_polygons(Polygons poly)
{
     if (poly == NULL) poly = mw_new_polygons();
     return(poly);
}


/* desallocate the polygons structure */

void mw_delete_polygons(Polygons polygons)
{
     Polygon poly, poly_next;

     if (polygons == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_polygons] cannot delete : polygons structure is NULL\n");
	  return;
     }

     poly = polygons->first;
     while (poly != NULL)
     {
	  poly_next = poly->next;
	  mw_delete_polygon(poly);
	  poly = poly_next;
     }

     free(polygons);
     polygons=NULL;
}

/* Return the number of polygons into a polygons */

unsigned int mw_length_polygons(Polygons polys)
{ 
     unsigned int n;
     Polygon pfirst,p;

     if ((!polys) || (!polys->first)) return(0);

     for (p=pfirst=polys->first, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);
     return(n);
}









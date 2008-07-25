/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  fpolygon.c
   
  Vers. 1.4
  Author : Jacques Froment
  Basic memory routines for the fpolygon internal type

  Main changes :
  v1.4 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <string.h>

#include "mw.h"

/* Creates a new fpolygon structure with 0 channel.                  */

Fpolygon mw_new_fpolygon(void)
{
     Fpolygon fpolygon;

     if(!(fpolygon = (Fpolygon) (malloc(sizeof(struct fpolygon)))))
     {
	  mwerror(ERROR, 0, "[mw_new_fpolygon] Not enough memory\n");
	  return(NULL);
     }
  
     fpolygon->nb_channels = 0;
     fpolygon->first = NULL;
     fpolygon->previous = NULL;
     fpolygon->next = NULL;
     return(fpolygon);
}

/* Allocates <nc> channels in the fpolygon.                              */
/* The meanning of channels is given by the user. Typically, if <nc>=1, */
/* channel[0] means gray level of the fpolygon.                          */

Fpolygon mw_alloc_fpolygon(Fpolygon fpolygon, int nc)
{
     int i;

     if (fpolygon == NULL)
     {
	  mwerror(ERROR, 0, 
		  "[mw_alloc_fpolygon] cannot alloc channels : fpolygon structure is NULL\n");
	  return(NULL);
     }
     if (nc <= 0)
     {
	  mwerror(ERROR, 0, "[mw_new_fpolygon] Illegal number of channels %d\n",nc);
	  return(NULL);      
     }

     if (!(fpolygon->channel = (float *) malloc(nc*sizeof(float))))
     {
	  mwerror(ERROR, 0, "[mw_new_fpolygon] Not enough memory\n");
	  return(NULL);
     }
     fpolygon->nb_channels = nc;
     for (i=0;i<nc;i++) fpolygon->channel[i] = -1.0;
     return(fpolygon);
}

/* Define the struct if it's not defined */
/* and/or change the number of channels <nc> */

Fpolygon mw_change_fpolygon(Fpolygon poly, int nc)
{
     if (poly == NULL) poly = mw_new_fpolygon();
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
	  if (mw_alloc_fpolygon(poly,nc) == NULL)
	  {
	       mw_delete_fpolygon(poly);
	       return(NULL);
	  }
     }
     return(poly);
}

/* desallocate the fpolygon structure */

void mw_delete_fpolygon(Fpolygon fpolygon)
{
     if (fpolygon == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_fpolygon] cannot delete : fpolygon structure is NULL\n");
	  return;
     }

     mw_delete_point_fcurve(fpolygon->first);
     if (fpolygon->nb_channels > 0) free(fpolygon->channel);
     free(fpolygon);
     fpolygon=NULL;
}

/* Return the number of point into a fpolygon */

unsigned int mw_length_fpolygon(Fpolygon fpoly)
{ 
     unsigned int n;
     Point_fcurve p,pfirst;

     if ((!fpoly) || (!fpoly->first)) return(0);

     for (p=pfirst=fpoly->first, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);
     return(n);
}


/* ----- */

/* creates a new fpolygons structure */

Fpolygons mw_new_fpolygons(void)
{
     Fpolygons fpolygons;

     if(!(fpolygons = (Fpolygons) (malloc(sizeof(struct fpolygons)))))
     {
	  mwerror(ERROR, 0, "[mw_new_fpolygons] Not enough memory\n");
	  return(NULL);
     }
     fpolygons->first = NULL;
     strcpy(fpolygons->cmt,"?");
     strcpy(fpolygons->name,"?");
     return(fpolygons);
}

/* Define the struct if it's not defined */

Fpolygons mw_change_fpolygons(Fpolygons poly)
{
     if (poly == NULL) poly = mw_new_fpolygons();
     return(poly);
}


/* desallocate the fpolygons structure */

void mw_delete_fpolygons(Fpolygons fpolygons)
{
     Fpolygon poly, poly_next;

     if (fpolygons == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_fpolygons] cannot delete : fpolygons structure is NULL\n");
	  return;
     }

     poly = fpolygons->first;
     while (poly != NULL)
     {
	  poly_next = poly->next;
	  mw_delete_fpolygon(poly);
	  poly = poly_next;
     }

     free(fpolygons);
     fpolygons=NULL;
}

/* Return the number of fpolygons into a fpolygons */

unsigned int mw_length_fpolygons(Fpolygons fpolys)
{ 
     unsigned int n;
     Fpolygon pfirst,p;

     if ((!fpolys) || (!fpolys->first)) return(0);

     for (p=pfirst=fpolys->first, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);
     return(n);
}









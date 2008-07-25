/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  wmax2d.c
   
  Vers. 1.1
  Author : Jacques Froment
  Basic memory routines for the various 2D Wavelet Maxima internal type


  Main changes :
  v1.1 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)

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

/* ----- Virtual Maxima points & chains ----- */

/* creates a new vpoint_wmax structure */

Vpoint_wmax mw_new_vpoint_wmax(void)
{
     Vpoint_wmax vpoint;
     int n;

     if(!(vpoint = (Vpoint_wmax) (malloc(sizeof(struct vpoint_wmax)))))
     {
	  mwerror(ERROR, 0, "[mw_new_vpoint_wmax] Not enough memory\n");
	  return(NULL);
     }
     vpoint->x = vpoint->y = -1;
     vpoint->previous = NULL;
     vpoint->next = NULL;

     for (n=0;n<mw_max_nlevel;n++) 
     {
	  vpoint->mag[n] = mw_not_a_magnitude;
	  vpoint->arg[n] = mw_not_an_argument;
     }
     vpoint->argp = mw_not_an_argument;

     return(vpoint);
}

/* Define the struct if it's not defined */

Vpoint_wmax mw_change_vpoint_wmax(Vpoint_wmax vpoint)
{
     if (vpoint == NULL) vpoint = mw_new_vpoint_wmax();
     return(vpoint);
}

/* desallocate the vpoint_wmax structure */

void mw_delete_vpoint_wmax(Vpoint_wmax vpoint)
{
     int n;

     if (vpoint == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_vpoint_wmax] cannot delete : vpoint_wmax structure is NULL\n");
	  return;
     }
     vpoint->previous = NULL;
     vpoint->next = NULL;
     for (n=0;n<mw_max_nlevel;n++) 
     {
	  vpoint->mag[n] = mw_not_a_magnitude;
	  vpoint->arg[n] = mw_not_an_argument;
     }
     vpoint->argp = mw_not_an_argument;

     free(vpoint);
     vpoint=NULL;
}

/* ----- */

/* creates a new vchain_wmax structure */

Vchain_wmax mw_new_vchain_wmax(void)
{
     Vchain_wmax vchain;

     if(!(vchain = (Vchain_wmax) (malloc(sizeof(struct vchain_wmax)))))
     {
	  mwerror(ERROR, 0, "[mw_new_vchain_wmax] Not enough memory\n");
	  return(NULL);
     }
     vchain->size = -1;
     vchain->first = NULL;
     vchain->previous = NULL;
     vchain->next = NULL;

     return(vchain);
}

/* Define the struct if it's not defined */

Vchain_wmax mw_change_vchain_wmax(Vchain_wmax vchain)
{
     if (vchain == NULL) vchain = mw_new_vchain_wmax();
     return(vchain);
}

/* desallocate the vchain_wmax structure */

void mw_delete_vchain_wmax(Vchain_wmax vchain)
{
     Vpoint_wmax vpoint;

     if (vchain == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_vchain_wmax] cannot delete : vchain_wmax structure is NULL\n");
	  return;
     }

     vpoint=vchain->first;
     if (vpoint != NULL)
     {
	  for (vpoint; vpoint && vpoint->next; vpoint = vpoint->next);
	  for (vpoint; vpoint && vpoint->previous ; vpoint = vpoint->previous)
	       if (vpoint->next) mw_delete_vpoint_wmax(vpoint->next);
	  mw_delete_vpoint_wmax(vchain->first);
     }
     vchain->size=-1;
     vchain->previous = NULL;
     vchain->next = NULL;
     free(vchain);
     vchain=NULL;
}

/* ----- */


/* creates a new vchains_wmax structure */

Vchains_wmax mw_new_vchains_wmax(void)
{
     Vchains_wmax vchains;

     if(!(vchains = (Vchains_wmax) (malloc(sizeof(struct vchains_wmax)))))
     {
	  mwerror(ERROR, 0, "[mw_new_vchains_wmax] Not enough memory\n");
	  return(NULL);
     }
     vchains->size = -1;
     vchains->ref_level = -1;
     vchains->nlevel = -1;
     vchains->nrow = vchains->ncol = -1;
     vchains->first = NULL;
     strcpy(vchains->cmt,"?");
     strcpy(vchains->name,"?");
  
     return(vchains);
}

/* Define the struct if it's not defined */

Vchains_wmax mw_change_vchains_wmax(Vchains_wmax vchains)
{
     if (vchains == NULL) vchains = mw_new_vchains_wmax();
     return(vchains);
}

/* desallocate the vchains_wmax structure */

void mw_delete_vchains_wmax(Vchains_wmax vchains)
{
     Vchain_wmax vchain;

     if (vchains == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_vchains_wmax] cannot delete : vchains_wmax structure is NULL\n");
	  return;
     }

     vchain=vchains->first;
     if (vchain != NULL)
     {
	  for (vchain; vchain && vchain->next; vchain = vchain->next);
	  for (vchain; vchain && vchain->previous ; vchain = vchain->previous)
	       if (vchain->next) mw_delete_vchain_wmax(vchain->next);
	  mw_delete_vchain_wmax(vchains->first);
     }

     vchains->size = -1;
     vchains->ref_level = -1;
     vchains->nlevel = -1;
     vchains->nrow = vchains->ncol = -1;
     vchains->first = NULL;

     free(vchains);
     vchains=NULL;
}

/* ----- */

/* Copy vpoint0 into vpoint1. Alloc vpoint1 if necessary. */

Vpoint_wmax mw_copy_vpoint_wmax(Vpoint_wmax vpoint1, Vpoint_wmax vpoint0)
{
     int l;

     if (vpoint1 == NULL) vpoint1 = mw_new_vpoint_wmax();
     if (vpoint1 == NULL) return(NULL);

     vpoint1->x = vpoint0->x;
     vpoint1->y = vpoint0->y;
     vpoint1->argp = vpoint0->argp;
     for (l=0;l<mw_max_nlevel;l++) 
     {
	  vpoint1->mag[l] = vpoint0->mag[l];
	  vpoint1->arg[l] = vpoint0->arg[l];
     }
     vpoint1->previous = vpoint0->previous;
     vpoint1->next = vpoint0->next;

     return(vpoint1);
}


/* Copy vchain0 into vchain1. Alloc vchain1 if necessary. */

Vchain_wmax mw_copy_vchain_wmax(Vchain_wmax vchain1, Vchain_wmax vchain0)
{
     Vpoint_wmax vpoint0,vpoint1,oldvpoint1,nextvpoint1;

     if (vchain1 == NULL) vchain1 = mw_new_vchain_wmax();   
     if (vchain1 == NULL) return(NULL);

     vchain1->size = vchain0->size;
     vchain1->previous = vchain0->previous;
     vchain1->next = vchain0->next;
     vchain1->first = NULL;

     if (!vchain0->first) return(vchain1);

     oldvpoint1 = NULL;
     for (vpoint0=vchain0->first;  vpoint0; vpoint0 = vpoint0->next)
     {
	  vpoint1 = mw_copy_vpoint_wmax(NULL,vpoint0);
	  if (vpoint1 == NULL) return(NULL);
	  if (vchain1->first == NULL) vchain1->first = vpoint1;
	  if (oldvpoint1 != NULL) oldvpoint1->next = vpoint1;
	  vpoint1->previous = oldvpoint1;
	  vpoint1->next = NULL;
	  oldvpoint1 = vpoint1;
     }


/* DEBUG ---
   printf("\n\n");
   for (vpoint0=vchain0->first;  vpoint0; vpoint0 = vpoint0->next)
   {
   printf("vpoint0=%x ",vpoint0);
   printf("\tprevious=%x ",vpoint0->previous);
   printf("\tnext=%x ",vpoint0->next);
   if (vpoint0->next) 
   printf("\tnext->previous=%x\n",(vpoint0->next)->previous);
   }
   printf("\n\n");
   for (vpoint1=vchain1->first;  vpoint1; vpoint1 = vpoint1->next)
   {
   printf("vpoint1=%x ",vpoint1);
   printf("\tprevious=%x ",vpoint1->previous);
   printf("\tnext=%x ",vpoint1->next);
   if (vpoint1->next) 
   printf("\tnext->previous=%x\n",(vpoint1->next)->previous);
   }
   printf("\n\n");
   --- */

     return(vchain1);
}

/* Return the number of levels (octaves) defined in a vchain */
/* (This information is recorded only in the vchains_wmax structure) */

int mw_give_nlevel_vchain(Vchain_wmax vchain)
{
     int nlevel;
     Vpoint_wmax vpoint;

     if (! vchain->first) return(0);
  
     nlevel = 0;
     for(vpoint = vchain->first; vpoint && (nlevel == 0);  vpoint = vpoint->next)
	  for (nlevel=0; (nlevel < mw_max_nlevel) &&
		    (vpoint->mag[nlevel] != mw_not_a_magnitude); nlevel++);
     return(nlevel);
}





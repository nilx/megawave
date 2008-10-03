/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  wtrans2d.c
   
  Vers. 1.1
  Author : Jacques Froment
  Basic memory routines for the wtrans2d internal type

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

#include <stdlib.h>
#include <string.h>

#include "libmw-defs.h"
#include "mw.h"

#include "fimage.h"

#include "wtrans2d.h"

/* creates a new wtrans2d structure */

Wtrans2d mw_new_wtrans2d(void)
{
     Wtrans2d wtrans;
     int j,d;

     if(!(wtrans = (Wtrans2d) (malloc(sizeof(struct wtrans2d)))))
     {
	  mwerror(ERROR, 0, "[mw_new_wtrans2d] Not enough memory\n");
	  return(NULL);
     }

     wtrans->type = wtrans->edges = wtrans->nlevel = wtrans->norient = 0;
     wtrans->nfilter = wtrans->nrow = wtrans->ncol = 0;

     strcpy(wtrans->cmt,"?");
     strcpy(wtrans->name,"?");

     for (j=0;j<mw_max_nlevel;j++) for (d=0;d<mw_max_norient;d++)
					wtrans->images[j][d] = NULL;

     for (j=0;j<mw_max_nfilter;j++) strcpy(wtrans->filter_name[j],"?");

     return (wtrans);
}

/* Alloc a wtrans2d for a wavelet decomposition with a given number */
/* of orientations (internal use only).                             */

void *_mw_alloc_wtrans2d_norient(Wtrans2d wtrans, int level, 
				 int nrow, int ncol, 
				 int Norient, int sampling)
{
     int dx,dy,j,d,jj,dd;

     if (wtrans == NULL)
     {
	  mwerror(ERROR, 0, 
		  "cannot alloc wtrans2d: wtrans2d structure is NULL\n");
	  return(NULL);
     }

     if ((nrow <= 0) || (ncol <= 0))
     {
	  mwerror(ERROR, 0, 
		  "cannot alloc wtrans2d: Illegal size of the original image\n");
	  return(NULL);
     }

     if (level > mw_max_nlevel)
     {
	  mwerror(ERROR, 0, 
		  "cannot alloc wtrans2d: too many levels (%d) in the decomposition\n",level);
	  return(NULL);
     }

     if (Norient > mw_max_norient)
     {
	  mwerror(ERROR, 0, 
		  "cannot alloc wtrans2d: too many orientations (%d) in the decomposition\n",Norient);
	  return(NULL);
     }

     wtrans->ncol = ncol;
     wtrans->nrow = nrow;

     if (sampling == 1) 
     {                      /* ------ Orthonormal/Biorthonormal case ----- */

/*
  if ( ((ncol % 2) != 0) || ((nrow % 2) != 0))
  {
  mwerror(ERROR, 0, 
  "cannot alloc wtrans2d: Size of original image is (%d,%d) - not an even size -\n",ncol,nrow);
  return(NULL);
  }
*/

	  dx = ncol/2;
	  dy = nrow/2;
	  for (j=1;j<=level;j++, dx/=2, dy/=2) 
	       for (d=0;d<=Norient;d++)
	       {
/*
  if ( ((dx % 2) != 0) || ((dy % 2) != 0))
  {
  mwerror(ERROR, 0, 
  "cannot alloc wtrans2d: Size of original image is (%d,%d) and max. level is %d\n",ncol,nrow,level);
  
  for (jj=1;jj<=j;jj++) for (dd=0;dd<=Norient;dd++)
  if (wtrans->images[jj][dd] != NULL)
  {
  mw_delete_fimage(wtrans->images[jj][dd]);
  wtrans->images[jj][dd] = NULL;
  }
  return(NULL);
  }
*/
		    wtrans->images[j][d] = mw_change_fimage(NULL,dy,dx);
		    if (wtrans->images[j][d] == NULL) 
		    {
			 mwerror(ERROR, 0,"cannot alloc wtrans2d: Not enough memory\n");
			 for (jj=1;jj<=j;jj++) 
			      for (dd=0;dd<=Norient;dd++)
				   if (wtrans->images[jj][dd] != NULL)
				   {
					mw_delete_fimage(wtrans->images[jj][dd]);
					wtrans->images[jj][dd] = NULL;
				   }
			 return(NULL);
		    }
	       }
     }
     else { /* ------ Dyadic/Continuous case ----- */
	  
	  dx = ncol;
	  dy = nrow;
	  
	  for (j=1;j<=level;j++) 
	       for (d=0;d<=Norient;d++)
	       {
		    wtrans->images[j][d] = mw_change_fimage(NULL,dy,dx);
		    if (wtrans->images[j][d] == NULL) 
		    {
			 mwerror(ERROR, 0,"cannot alloc wtrans2d: Not enough memory\n");
			 for (jj=1;jj<=j;jj++) 
			      for (dd=0;dd<=Norient;dd++)
				   if (wtrans->images[jj][dd] != NULL)
				   {
					mw_delete_fimage(wtrans->images[jj][dd]);
					wtrans->images[jj][dd] = NULL;
				   }
			 return(NULL);
		    }
	       }
     }
     
     wtrans->nlevel = level;
     wtrans->norient = Norient;
     return(wtrans);
}    

/* Alloc a wtrans2d for an orthonormal decomposition */ 

void *mw_alloc_ortho_wtrans2d(Wtrans2d wtrans, int level, int nrow, int ncol)
{
     if  ( _mw_alloc_wtrans2d_norient(wtrans,level,nrow,ncol,3,1) == NULL)
     {
	  mwerror(ERROR, 0,"[mw_alloc_ortho_wtrans2d] Cannot alloc orthogonal wtrans2d.\n");
	  return(NULL);
     }
     wtrans->type = mw_orthogonal;
     return(wtrans);
}

/* Alloc a wtrans2d for a biorthonormal decomposition */ 

void *mw_alloc_biortho_wtrans2d(Wtrans2d wtrans, int level, int nrow, int ncol)
{
     if  ( _mw_alloc_wtrans2d_norient(wtrans,level,nrow,ncol,3,1) == NULL)
     {
	  mwerror(ERROR, 0,"[mw_alloc_biortho_wtrans2d] Cannot alloc biorthogonal wtrans2d.\n");
	  return(NULL);
     }
     wtrans->type = mw_biorthogonal;
     return(wtrans);
}

/* Alloc a wtrans2d for a dyadic decomposition */ 

void *mw_alloc_dyadic_wtrans2d(Wtrans2d wtrans, int level, int nrow, int ncol)
{
     if (_mw_alloc_wtrans2d_norient(wtrans,level,nrow,ncol,4,0) == NULL)
     {
	  mwerror(ERROR, 0,"[mw_alloc_dyadic_wtrans2d] Cannot alloc dyadic wtrans2d.\n");
	  return(NULL);
     }
     wtrans->type = mw_dyadic;
     return(wtrans);
}

/* Desallocate the memory used by a wtrans2d and the structure itself */

void mw_delete_wtrans2d(Wtrans2d wtrans)
{
     int j,d;

     if (wtrans == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_wtrans2d] cannot delete : wtrans2d structure is NULL\n");
	  return;
     }

     wtrans->images[0][0] = NULL;

     for (j=1;j<=wtrans->nlevel;j++) for (d=0;d<=wtrans->norient;d++)
					  if (wtrans->images[j][d] != NULL)
					  {
					       mw_delete_fimage(wtrans->images[j][d]);
					       wtrans->images[j][d] = NULL;
					  }

     for (j=0;j<mw_max_nfilter;j++) strcpy(wtrans->filter_name[j],"?");
     wtrans->type = wtrans->edges = wtrans->nlevel = wtrans->norient = 0;
     wtrans->nfilter = wtrans->nrow = wtrans->ncol = 0;
     wtrans->cmt[0] = wtrans->name[0] = '\0';
     free(wtrans);
     wtrans=NULL;  /* Useless but who knows ? */
}

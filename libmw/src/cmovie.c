/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  cmovie.c
   
  Vers. 1.1
  Author : Jacques Froment
  Basic memory routines for the cmovie internal type

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
#include "utils.h"
#include "cimage.h"

#include "cmovie.h"

/* creates a new cmovie structure */

Cmovie mw_new_cmovie(void)
{
     Cmovie movie;

     if(!(movie = (Cmovie) (malloc(sizeof(struct cmovie)))))
     {
	  mwerror(ERROR, 0, "[mw_new_cmovie] Not enough memory\n");
	  return(NULL);
     }

     movie->scale = 1.0;
     strcpy(movie->cmt,"?");
     strcpy(movie->name,"?");
     movie->first = NULL;

     return (movie);
}

/* desallocate the images in the cmovie structure and the structure itself */

void mw_delete_cmovie(Cmovie movie)
{
     Cimage image,image_next;

     if (movie == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_cmovie] cannot delete : cmovie structure is NULL\n");
	  return;
     }

     image = movie->first;
     while (image != NULL)
     {
	  image_next = image->next;
	  mw_delete_cimage(image);
	  image = image_next;
     }
     free(movie);
     movie=NULL;
}


/* Define the struct if not defined */
/* So you have to call it with image = mw_change_cimage(image,...) */

Cmovie mw_change_cmovie(Cmovie movie)
{
     if (movie == NULL) movie = mw_new_cmovie();
     return(movie);
}

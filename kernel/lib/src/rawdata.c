/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  rawdata.c
   
  Vers. 1.1
  Author : Jacques Froment
  Basic memory routines for the rawdata internal type
  This internal type allows to load into memory any kind of file.

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

/* creates a new rawdata structure */

Rawdata mw_new_rawdata(void)
{
     Rawdata rd;

     if(!(rd = (Rawdata) (malloc(sizeof(struct rawdata)))))
     {
	  mwerror(ERROR, 0, "[mw_new_rawdata] Not enough memory\n");
	  return(NULL);
     }

     rd->size = 0;
     rd->data = NULL;
     return (rd);
}

/* allocates the data array */ 

Rawdata mw_alloc_rawdata(Rawdata rd, int newsize)
{
     if (rd == NULL)
     {
	  mwerror(ERROR, 0, 
		  "[mw_alloc_rawdata] cannot alloc data : rawdata structure is NULL\n");
	  return(NULL);
     }
  
     if (rd->data != NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_alloc_rawdata] Attempts to alloc a rawdata which is already allocated\n");
	  return(NULL);
     }

     rd->data = (unsigned char *) malloc(newsize);
     if (rd->data == NULL)
     {
	  rd->size = 0;
	  mwerror(ERROR, 0,"[mw_alloc_rawdata] Not enough memory\n");
	  return(NULL);
     }
     rd->size = newsize;  
     return(rd);
}

/* desallocate the data array in the rawdata structure and the 
   structure itself 
*/

void mw_delete_rawdata(Rawdata rd)
{
     if (rd == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_rawdata] cannot delete : rawdata structure is NULL\n");
	  return;
     }
     if (rd->data != NULL) free(rd->data);
     rd->data = NULL;
     free(rd);
     rd=NULL;
}


/* Change the size of the allocated data array */
/* May define the struct if not defined */
/* So you have to call it with rd = mw_change_rawdata(rd,...) */

Rawdata mw_change_rawdata(Rawdata rd, int newsize)
{
     if (rd == NULL) rd = mw_new_rawdata();
     if (rd == NULL) return(NULL);

     if (newsize != rd->size)
     {
	  if (rd->data != NULL) 
	  {
	       free(rd->data);  
	       rd->data = NULL;
	  }
	  if (mw_alloc_rawdata(rd,newsize) == NULL)
	  {
	       mw_delete_rawdata(rd);
	       return(NULL);
	  }
     }
     else 
	  rd->size = newsize;
     return(rd);
}

/* Copy the data of a rawdata into another rawdata */

void mw_copy_rawdata(Rawdata in, Rawdata out)
{
     if ((!in) || (!out) || (!in->data) || (!out->data) 
	 || (in->size != out->size))
     {
	  mwerror(ERROR, 0,
		  "[mw_copy_rawdata] NULL input or output rd or rawdatas of different sizes\n");
	  return;
     }
     memcpy(out->data, in->data, in->size);
}


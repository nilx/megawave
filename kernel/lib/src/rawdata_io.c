/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   rawdata_io.c
   
   Vers. 1.0
   (C) 2000 Jacques Froment
   Input/Output functions for the rawdata structure
   This internal type allows to load into memory any kind of file.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef Linux
#include <unistd.h>
#endif

#include "mw.h"

/* Load rawdata from a file of any format */

Rawdata _mw_load_rawdata(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Rawdata rd;
  struct stat buf;
  int fsize;

  if ( (!(fp = fopen(fname, "r"))) || (fstat(fileno(fp),&buf) != 0) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }
  /* Size of the file = size of the data, in bytes */
  fsize = buf.st_size; 
  if (!(rd=mw_change_rawdata(NULL,fsize)))
    {
      mwerror(ERROR, 0,"Not enough memory to load rawdata file \"%s\" (%d bytes) !\n",fname,fsize);
      fclose(fp);
      return(NULL);
    }
  if (fread(rd->data,1,fsize,fp) != fsize)
    {
      mwerror(ERROR, 0,"Error while reading rawdata file \"%s\" !\n",fname);
      fclose(fp);
      mw_delete_rawdata(rd);
      return(NULL);
    }

  fclose(fp);
  return(rd);
}


/* Write rawdata file */  

short _mw_create_rawdata(fname,rd)

char  *fname;                        /* file name */
Rawdata rd;

{
  FILE *fp;

  if (rd == NULL)
    mwerror(INTERNAL,1,"[_mw_create_rawdata_mw2_rawdata] Cannot create file: Rawdata structure is NULL\n");

  if (rd->size <= 0)
    mwerror(INTERNAL,1,"[_mw_create_rawdata_mw2_rawdata] Cannot create file: Rawdata structure's size is %d !\n",rd->size);

  fp = fopen(fname, "w");
  if (fp == NULL) return(-1);
  
  if (fwrite(rd->data,1,rd->size,fp) != rd->size)
    {
      mwerror(ERROR, 0,"Error while writing rawdata file \"%s\" !\n",fname);
      fclose(fp);
      return(-1);
    }
  fclose(fp);
  return(0);
}


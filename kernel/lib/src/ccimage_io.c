/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ccimage_io.c
   
   Vers. 1.09
   (C) 1993-2002 Jacques Froment
   Input/Output private functions for the Ccimage structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>
#include <math.h>

#include "mw.h"

/*~~~~~~ MegaWave2 formats ~~~~~*/

/* Native formats (without conversion of the internal type) */

/* Return != NULL if load OK */

Ccimage _mw_ccimage_load_native(NomFic,Type)

char  *NomFic;                        /* Nom du fichier image */
char  *Type;                          /* Type de format du fichier */

{
  if (strcmp(Type,"PMC_C") == 0)
    /* PM format with pm_form=PM_C and pm_np = 3 */
    return((Ccimage) _mw_ccimage_load_pm(NomFic));

  if (strcmp(Type,"TIFFC") == 0)
    /* TIFF format with 24-bit color plane */
    return((Ccimage) _mw_ccimage_load_tiff(NomFic));

  return(NULL);
}

/* Return 0 if create OK */

short _mw_ccimage_create_native(NomFic,image,Type)

     char  *NomFic;                        /* Nom du fichier image */
     Ccimage image;
     char  *Type;                          /* Type de format du fichier */

{

  if (strcmp(Type,"TIFFC") == 0)
    /* TIFF format with 24-bit color plane */
    return(_mw_ccimage_create_tiff(NomFic,image));

  if (strcmp(Type,"PMC_C") == 0)
    /* PM format with pm_form=PM_C and pm_np 3 */
    return(_mw_ccimage_create_pm(NomFic,image));

  return(-1);
}

/* All available formats */

Ccimage _mw_ccimage_load_image(NomFic,Type)

char  *NomFic;                        /* Nom du fichier image */
char  *Type;                          /* Type de format du fichier */

{ 
  Ccimage im;
  char mtype[mw_mtype_size];
  int hsize;  /* Size of the header, in bytes */
  float version;/* Version number of the file format */

  _mw_get_file_type(NomFic,Type,mtype,&hsize,&version);

  /* First, try native formats */
  im = _mw_ccimage_load_native(NomFic,Type);
  if (im != NULL) return(im);

  /* If failed, try other formats with memory conversion */
  im = (Ccimage) _mw_load_etype_to_itype(NomFic,mtype,"ccimage",Type);
  if (im != NULL) return(im);

  if (Type[0]=='?')
    mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",NomFic);
  else
    mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Ccimage !\n",NomFic,Type);
}


short _mw_ccimage_create_image(NomFic,image,Type)

     char  *NomFic;                        /* Nom du fichier image */
     Ccimage image;
     char  *Type;                          /* Type de format du fichier */

{ 
  short ret;

  /* First, try native formats */
  ret = _mw_ccimage_create_native(NomFic,image,Type);
  if (ret == 0) return(0);

  /* If failed, try other formats with memory conversion */
  ret = _mw_create_etype_from_itype(NomFic,image,"ccimage",Type);
  if (ret == 0) return(0);

  /* Invalid Type should have been detected before, but we can arrive here because
     of a write failure (e.g. the output file name is write protected).
  */
  mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",NomFic);  
}


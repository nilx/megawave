/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ccimage_io.c
   
   Vers. 1.04
   (C) 1993-99 Jacques Froment
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

{ Cimage image_cimage;
  Ccimage image_ccimage;
  Cfimage image_cfimage;
  Fimage image_fimage;
  char mtype[TYPE_SIZE];

  _mw_get_file_type(NomFic,Type,mtype);

  /* First, try native formats */
  image_ccimage = _mw_ccimage_load_native(NomFic,Type);
  if (image_ccimage != NULL) return(image_ccimage);

  /* Other formats */
  image_cfimage = (Cfimage) _mw_cfimage_load_native(NomFic,Type);
  if (image_cfimage != NULL) 
    {
      image_ccimage = (Ccimage) mw_cfimage_to_ccimage(image_cfimage);
      mw_delete_cfimage(image_cfimage);
      return(image_ccimage);
    }

  image_cimage = (Cimage) _mw_cimage_load_native(NomFic,Type);
  if (image_cimage != NULL) 
    {
      image_ccimage = (Ccimage) mw_cimage_to_ccimage(image_cimage);
      mw_delete_cimage(image_cimage);
      return(image_ccimage);
    }

  image_fimage = (Fimage) _mw_fimage_load_native(NomFic,Type,NULL);
  if (image_fimage != NULL) 
    {
      image_cimage = (Cimage) mw_fimage_to_cimage(image_fimage);
      image_ccimage = (Ccimage) mw_cimage_to_ccimage(image_cimage);
      mw_delete_cimage(image_cimage);
      mw_delete_fimage(image_fimage);
      return(image_ccimage);
    }

  mwerror(INTERNAL, 1,"[_mw_ccimage_load_image] Invalid external type \"%s\" for the file \"%s\"\n",Type,NomFic);

}


short _mw_ccimage_create_image(NomFic,image,Type)

     char  *NomFic;                        /* Nom du fichier image */
     Ccimage image;
     char  *Type;                          /* Type de format du fichier */

{ 
  Cimage image_cimage;
  Fimage image_fimage;
  Cfimage image_cfimage;
  short ret;

  /* First, try native formats */
  ret = _mw_ccimage_create_native(NomFic,image,Type);
  if (ret == 0) return(0);

  /* Other formats */

  image_cfimage = (Cfimage) mw_ccimage_to_cfimage(image);
  ret = _mw_cfimage_create_native(NomFic,image_cfimage,Type);
  mw_delete_cfimage(image_cfimage);
  if (ret == 0) return(0);

  image_cimage = (Cimage) mw_ccimage_to_cimage(image);
  ret = _mw_cimage_create_native(NomFic,image_cimage,Type);
  if (ret == 0) { mw_delete_cimage(image_cimage); return(0); }

  image_fimage = (Fimage) mw_cimage_to_fimage(image_cimage);
  ret = _mw_fimage_create_native(NomFic,image_fimage,Type);
  mw_delete_fimage(image_fimage);
  mw_delete_cimage(image_cimage);
  if (ret == 0) return(0);

  mwerror(INTERNAL, 1,"[_mw_ccimage_create_image] Invalid external type \"%s\" for the file \"%s\"\n",Type,NomFic); 
}

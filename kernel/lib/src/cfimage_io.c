/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   cfimage_io.c
   
   Vers. 1.2
   (C) 1994-99 Jacques Froment
   Input/Output private functions for the Cfimage structure

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

#include "mw.h"

/*~~~~~~ MegaWave2 formats ~~~~~*/

/* Native formats (without conversion of the internal type) */

/* Return != NULL if load OK */

Cfimage _mw_cfimage_load_native(NomFic,Type)

char  *NomFic;                        /* Nom du fichier image */
char  *Type;                          /* Type de format du fichier */

{ 
  /* File formats with 3 color planes of float */
  if (strcmp(Type,"PMC_F") == 0)
    /* PM format with pm_form=PM_F and pm_np = 3 */
    return((Cfimage) _mw_cfimage_load_pm(NomFic));

  return(NULL);
}

/* Return 0 if create OK */

short _mw_cfimage_create_native(NomFic,image,Type)

     char  *NomFic;                        /* Nom du fichier image */
     Cfimage image;
     char  *Type;                          /* Type de format du fichier */

{ 
  /* File formats with 3 color planes of float */
  if (strcmp(Type,"PMC_F") == 0)
    /* PM format with pm_form=PM_F and pm_np 3 */
    return(_mw_cfimage_create_pm(NomFic,image));
  
  return(-1);
}


/* All available formats */

Cfimage _mw_cfimage_load_image(NomFic,Type)

char  *NomFic;                        /* Nom du fichier image */
char  *Type;                          /* Type de format du fichier */

{ 
  Cfimage image_cfimage;
  Ccimage image_ccimage;
  Cimage image_cimage;
  Fimage image_fimage;
  char mtype[TYPE_SIZE];
  
  _mw_get_file_type(NomFic,Type,mtype);
  
  /* First, try native formats */
  image_cfimage = _mw_cfimage_load_native(NomFic,Type);
  if (image_cfimage != NULL) return(image_cfimage);

  /* Other formats */

  image_ccimage = (Ccimage) _mw_ccimage_load_native(NomFic,Type);
  if (image_ccimage != NULL)
    {
      image_cfimage = (Cfimage) mw_ccimage_to_cfimage(image_ccimage);
      mw_delete_ccimage(image_ccimage);
      return(image_cfimage);
    }

  image_fimage = (Fimage) _mw_fimage_load_native(NomFic,Type);
  if (image_fimage != NULL)
    {
      image_cfimage = (Cfimage) mw_fimage_to_cfimage(image_fimage);
      mw_delete_fimage(image_fimage);
      return(image_cfimage);
    }

  image_cimage = (Cimage) _mw_cimage_load_native(NomFic,Type);
  if (image_cimage != NULL)
    {
      image_ccimage = (Ccimage) mw_cimage_to_ccimage(image_cimage);
      image_cfimage = (Cfimage) mw_ccimage_to_cfimage(image_ccimage);
      mw_delete_ccimage(image_ccimage);
      mw_delete_cimage(image_cimage);
      return(image_cfimage);
    }
  mwerror(INTERNAL, 1,"[_mw_cfimage_load_image] Invalid external type \"%s\" for the file \"%s\"\n",Type,NomFic);
}


short _mw_cfimage_create_image(NomFic,image,Type)

     char  *NomFic;                        /* Nom du fichier image */
     Cfimage image;
     char  *Type;                          /* Type de format du fichier */

{ 
  Cimage image_cimage;
  Fimage image_fimage;
  Ccimage image_ccimage;
  short ret;
  
  /* First, try native formats */
  ret = _mw_cfimage_create_native(NomFic,image,Type);
  if (ret == 0) return(0);

  /* Other formats */

  image_ccimage = (Ccimage) mw_cfimage_to_ccimage(image);
  ret = _mw_ccimage_create_native(NomFic,image_ccimage,Type);
  mw_delete_ccimage(image_ccimage);
  if (ret == 0) return(0);

  image_fimage = (Fimage) mw_cfimage_to_fimage(image);
  ret = _mw_fimage_create_native(NomFic,image_fimage,Type);
  if (ret == 0) { mw_delete_fimage(image_fimage); return(0);}

  image_cimage = (Cimage) mw_fimage_to_cimage(image_fimage);
  ret = _mw_cimage_create_native(NomFic,image_cimage,Type);
  mw_delete_cimage(image_cimage);
  mw_delete_fimage(image_fimage);
  if (ret == 0) return(0);

  mwerror(INTERNAL, 1,"[_mw_cfimage_create_image] Invalid external type \"%s\" for the file \"%s\"\n",Type,NomFic); 

}














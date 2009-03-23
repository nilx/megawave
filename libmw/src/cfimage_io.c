/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  cfimage_io.c
   
  Vers. 1.8
  (C) 1994-2002 Jacques Froment
  Input/Output private functions for the Cfimage structure

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <string.h>

#include "definitions.h"
#include "error.h"
#include "pm_io.h"
#include "file_type.h"
#include "type_conv.h"

#include "cfimage_io.h"

/*~~~~~~ MegaWave2 formats ~~~~~*/

/* Native formats (without conversion of the internal type) */

/* Return != NULL if load OK */

Cfimage _mw_cfimage_load_native(char *NomFic, char *Type)
{ 
     /* File formats with 3 color planes of float */
     if (strcmp(Type,"PMC_F") == 0)
	  /* PM format with pm_form=PM_F and pm_np = 3 */
	  return((Cfimage) _mw_cfimage_load_pm(NomFic));

     return(NULL);
}

/* Return 0 if create OK */

short _mw_cfimage_create_native(char *NomFic, Cfimage image, char *Type)
{ 
     /* File formats with 3 color planes of float */
     if (strcmp(Type,"PMC_F") == 0)
	  /* PM format with pm_form=PM_F and pm_np 3 */
	  return(_mw_cfimage_create_pm(NomFic,image));
  
     return(-1);
}


/* All available formats */

Cfimage _mw_cfimage_load_image(char *NomFic, char *Type)
{ 
     Cfimage im;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(NomFic,Type,mtype,&hsize,&version);
  
     /* First, try native formats */
     im = _mw_cfimage_load_native(NomFic,Type);
     if (im != NULL) return(im);

     /* If failed, try other formats with memory conversion */
     im = (Cfimage) _mw_load_etype_to_itype(NomFic,mtype,"cfimage",Type);
     if (im != NULL) return(im);

     if (Type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",NomFic);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Cfimage !\n",NomFic,Type);
     return NULL;
}


short _mw_cfimage_create_image(char *NomFic, Cfimage image, char *Type)
{ 
     short ret;
  
     /* First, try native formats */
     ret = _mw_cfimage_create_native(NomFic,image,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(NomFic,image,"cfimage",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",NomFic);  
     return -1;
}

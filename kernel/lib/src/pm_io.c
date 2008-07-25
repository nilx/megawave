/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  pm_io.c
   
  Vers. 1.08
  Author : Jacques Froment
  Part of code (C) 1989, 1990 by the University of Pennsylvania
  Input/Output functions for the PM file compatibility with MegaWave2

  Main changes :
  v1.08 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
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
#include "pm.h"

static char temp_char;
static int temp_int;

/*~~~~~ Load PM CHAR ~~~~~*/

Cimage _mw_cimage_load_pm(char *file)
{
     int	isize,cmtsize;
     char    need_flipping = 0;
     pmpic   *pm;
     FILE *fp;
     Cimage image;
  
     pm = (pmpic*)malloc(sizeof(pmpic));
     if (pm == NULL)
	  mwerror(FATAL,0,"Not enough memory to load any PM image !\n");

     mw_bzero(pm,sizeof(pmpic));
     pm->pm_id = PM_MAGICNO;
     pm->pm_np = pm->pm_nband = 1;
     pm->pm_nrow = pm->pm_ncol = 512;
     pm->pm_form = PM_C;
     pm->pm_cmtsize = 0;
     pm->pm_image = NULL;
     pm->pm_cmt = NULL;

     if (!(fp = fopen(file, "r")))
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",file);
	  fclose(fp);
	  free(pm);
	  return(NULL);
     }
  
     if (fread(pm,PM_IOHDR_SIZE,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }
  
     if (pm->pm_id != PM_MAGICNO)
     {
	  _mw_in_flip_b4(pm->pm_id);
	  if (pm->pm_id == PM_MAGICNO)
	       need_flipping = 1;
     }

     if (pm->pm_id != PM_MAGICNO)
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_load_pm] File \"%s\" is not in the PM format\n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }

     if (need_flipping)
     {
	  _mw_in_flip_b4(pm->pm_np);
	  _mw_in_flip_b4(pm->pm_nrow);
	  _mw_in_flip_b4(pm->pm_ncol);
	  _mw_in_flip_b4(pm->pm_nband);
	  _mw_in_flip_b4(pm->pm_form);
	  _mw_in_flip_b4(pm->pm_cmtsize);
     }

     if (pm->pm_form != PM_C)
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_load_pm] File \"%s\" is not in the CHAR PM fomat\n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }

     isize = pm_isize(pm);
     if (isize != (pm->pm_nrow * pm->pm_ncol * sizeof(char)))
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_load_pm] PM file \"%s\" cannot be loaded into a cimage\n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }

     image = mw_change_cimage(NULL,pm->pm_nrow,pm->pm_ncol);
     if (image == NULL)
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",file);

     if (isize)
     {
	  if (fread(image->gray,isize,1,fp) == 0)
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	       free(pm);
	       mw_delete_cimage(image);
	       fclose(fp);
	       return(NULL);
	  }
     }
    
     cmtsize = pm->pm_cmtsize;
     if (pm->pm_cmtsize > mw_cmtsize) cmtsize = mw_cmtsize;
  
     if (cmtsize)
     {
	  if (fread(image->cmt,cmtsize,1,fp) == 0)
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	       free(pm);
	       mw_delete_cimage(image);
	       fclose(fp);
	       return(NULL);
	  }
     }

     fclose(fp);

     _mw_flip_image((unsigned char *) image->gray,sizeof(char),
		    pm->pm_ncol,pm->pm_nrow,FALSE);

     free(pm);
     return(image);
}


/*~~~~~ Create PM CHAR ~~~~~*/

short _mw_cimage_create_pm(char *file, Cimage image)
{
     FILE *fp;
     pmpic   *pm;
     int isize;

     if (!(fp = fopen(file, "w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",file);
	  return(-1);
     }

     if ((image == NULL) || (image->gray == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_create_pm] NULL cimage or cimage plane\n");
	  return(-2);
     }

     pm = (pmpic*)malloc(sizeof(pmpic));
     if (pm == NULL)
	  mwerror(FATAL,0,"Not enough memory to create any PM image !\n");

     mw_bzero(pm,sizeof(pmpic));
     pm->pm_id = PM_MAGICNO;
     pm->pm_np = pm->pm_nband = 1;
     pm->pm_nrow = image->nrow;
     pm->pm_ncol = image->ncol;
     pm->pm_form = PM_C;
     pm->pm_cmtsize = strlen(image->cmt);
     pm->pm_image = NULL;
     pm->pm_cmt = NULL;

     if (fwrite(pm,PM_IOHDR_SIZE,1,fp) == 0)
     {
	  mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     isize = pm_isize(pm);
     if (isize != (image->nrow * image->ncol * sizeof(char)))
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_create_pm] Illegal image size\n");
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     if (fwrite(image->gray,isize,1,fp) == 0)
     {
	  mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     if (pm->pm_cmtsize)
     {
	  if (fwrite(image->cmt,pm->pm_cmtsize,1,fp) == 0)
	  {
	       mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	       free(pm);
	       fclose(fp);
	       return(-1);
	  }
     }
  
     free(pm);
     fclose(fp);
     return(0);
}


/*~~~~~ Load PM FLOAT ~~~~~*/

Fimage _mw_fimage_load_pm(char *file)
{
     Fimage image=NULL; 
     int	isize,cmtsize;
     char    need_flipping = FALSE;
     pmpic   *pm;
     FILE *fp;
   
     pm = (pmpic*)malloc(sizeof(pmpic));
     if (pm == NULL)
	  mwerror(FATAL,0,"Not enough memory to load any PM image !\n");

     mw_bzero(pm,sizeof(pmpic));
     pm->pm_id = PM_MAGICNO;
     pm->pm_np = pm->pm_nband = 1;
     pm->pm_nrow = pm->pm_ncol = 512;
     pm->pm_form = PM_C;
     pm->pm_cmtsize = 0;
     pm->pm_image = NULL;
     pm->pm_cmt = NULL;

     if (!(fp = fopen(file, "r")))
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",file);
	  fclose(fp);
	  free(pm);
	  return(NULL);
     }
  
     if (fread(pm,PM_IOHDR_SIZE,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }
  
     if (pm->pm_id != PM_MAGICNO)
     {
	  _mw_in_flip_b4(pm->pm_id);
	  if (pm->pm_id == PM_MAGICNO)
	       need_flipping = TRUE;
     }

     if (pm->pm_id != PM_MAGICNO)
     {
	  mwerror(INTERNAL, 0,"[_mw_fimage_load_pm] File \"%s\" is not in the PM format\"n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }

     if (need_flipping == TRUE)
     {
	  _mw_in_flip_b4(pm->pm_np);
	  _mw_in_flip_b4(pm->pm_nrow);
	  _mw_in_flip_b4(pm->pm_ncol);
	  _mw_in_flip_b4(pm->pm_nband);
	  _mw_in_flip_b4(pm->pm_form);
	  _mw_in_flip_b4(pm->pm_cmtsize);
     }

     if (pm->pm_form != PM_F)
     {
	  mwerror(INTERNAL, 0,"[_mw_fimage_load_pm] File \"%s\" is not in the FLOAT PM format\"n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }
  
     isize = pm_isize(pm);
     if (isize != (pm->pm_nrow * pm->pm_ncol * sizeof(float)))
     {
	  mwerror(INTERNAL, 0,"[_mw_fimage_load_pm] PM file \"%s\" cannot be loaded into a fimage\n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }

     image = mw_change_fimage(image,pm->pm_nrow,pm->pm_ncol);
     if (image == NULL)
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",file);

     if (isize)
     {
	  if (fread(image->gray,isize,1,fp) == 0)
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	       free(pm);
	       mw_delete_fimage(image);
	       fclose(fp);
	       return(NULL);
	  }
     }
    
     cmtsize = pm->pm_cmtsize;
     if (pm->pm_cmtsize > mw_cmtsize) cmtsize = mw_cmtsize;
  
     if (cmtsize)
     {
	  if (fread(image->cmt,cmtsize,1,fp) == 0)
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	       free(pm);
	       mw_delete_fimage(image);
	       fclose(fp);
	       return(NULL);
	  }
     }

     fclose(fp);
     _mw_flip_image((unsigned char *) image->gray,sizeof(float),
		    pm->pm_ncol,pm->pm_nrow,need_flipping);
     free(pm);
     return(image);
}


/*~~~~~ Create PM FLOAT ~~~~~*/

short _mw_fimage_create_pm(char *file, Fimage image)
{
     FILE *fp;
     pmpic   *pm;
     int isize;

     if (!(fp = fopen(file, "w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",file);
	  return(-1);
     }

     if ((image == NULL) || (image->gray == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_fimage_create_pm] NULL fimage or fimage plane\n");
	  return(-2);
     }

     pm = (pmpic*)malloc(sizeof(pmpic));
     if (pm == NULL)
	  mwerror(FATAL,0,"Not enough memory to create any PM image !\n");

     mw_bzero(pm,sizeof(pmpic));
     pm->pm_id = PM_MAGICNO;
     pm->pm_np = pm->pm_nband = 1;
     pm->pm_nrow = image->nrow;
     pm->pm_ncol = image->ncol;
     pm->pm_form = PM_F;
     pm->pm_cmtsize = strlen(image->cmt);
     pm->pm_image = NULL;
     pm->pm_cmt = NULL;

     if (fwrite(pm,PM_IOHDR_SIZE,1,fp) == 0)
     {
	  mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     isize = pm_isize(pm);
     if (isize != (image->nrow * image->ncol * sizeof(float)))
     {
	  mwerror(INTERNAL, 0,"[_mw_fimage_create_pm] Illegal image size\n");
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     if (fwrite(image->gray,isize,1,fp) == 0)
     {
	  mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     if (pm->pm_cmtsize)
     {
	  if (fwrite(image->cmt,pm->pm_cmtsize,1,fp) == 0)
	  {
	       mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	       free(pm);
	       fclose(fp);
	       return(-1);
	  }
     }
  
     free(pm);
     fclose(fp);
     return(0);
}



/*~~~~~ Load 24-bit color PM CHAR ~~~~~*/

Ccimage _mw_ccimage_load_pm(char *file)
{
     int isize,cmtsize;
     char need_flipping = 0;
     pmpic *pm;
     FILE *fp;
     Ccimage image;

     pm = (pmpic*)malloc(sizeof(pmpic));
     if (pm == NULL)
	  mwerror(FATAL,0,"Not enough memory to load any PM image !\n");

     mw_bzero(pm,sizeof(pmpic));
     pm->pm_id = PM_MAGICNO;
     pm->pm_np = pm->pm_nband = 1;
     pm->pm_nrow = pm->pm_ncol = 512;
     pm->pm_form = PM_C;
     pm->pm_cmtsize = 0;
     pm->pm_image = NULL;
     pm->pm_cmt = NULL;

     if (!(fp = fopen(file, "r")))
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",file);
	  fclose(fp);
	  free(pm);
	  return(NULL);
     }
  
     if (fread(pm,PM_IOHDR_SIZE,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }
  
     if (pm->pm_id != PM_MAGICNO)
     {
	  _mw_in_flip_b4(pm->pm_id);
	  if (pm->pm_id == PM_MAGICNO)
	       need_flipping = 1;
     }

     if (pm->pm_id != PM_MAGICNO)
     {
	  mwerror(INTERNAL, 0,"[_mw_ccimage_load_pm] File \"%s\" is not in the PM format\n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }

     if (need_flipping)
     {
	  _mw_in_flip_b4(pm->pm_np);
	  _mw_in_flip_b4(pm->pm_nrow);
	  _mw_in_flip_b4(pm->pm_ncol);
	  _mw_in_flip_b4(pm->pm_nband);
	  _mw_in_flip_b4(pm->pm_form);
	  _mw_in_flip_b4(pm->pm_cmtsize);
     }

     if (pm->pm_form != PM_C)
     {
	  mwerror(INTERNAL, 0,"[_mw_ccimage_load_pm] File \"%s\" is not in the CHAR PM fomat\n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }
  
     isize = pm_isize(pm);
     if (isize != (3 * pm->pm_nrow * pm->pm_ncol * sizeof(char)))
     {
	  mwerror(INTERNAL, 0,"[_mw_ccimage_load_pm] PM file \"%s\" cannot be loaded into a ccimage\n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }

     image = mw_change_ccimage(NULL,pm->pm_nrow,pm->pm_ncol);
     if (image == NULL)
     {
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",file);
	  return(NULL);
     }

     if (isize)
     {
	  if ( (fread(image->red,isize/3,1,fp) == 0)
	       || (fread(image->green,isize/3,1,fp) == 0)
	       || (fread(image->blue,isize/3,1,fp) == 0) )
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	       free(pm);
	       mw_delete_ccimage(image);
	       fclose(fp);
	       return(NULL);
	  }
     }
    
     cmtsize = pm->pm_cmtsize;
     if (pm->pm_cmtsize > mw_cmtsize) cmtsize = mw_cmtsize;
  
     if (cmtsize)
     {
	  if (fread(image->cmt,cmtsize,1,fp) == 0)
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	       free(pm);
	       mw_delete_ccimage(image);
	       fclose(fp);
	       return(NULL);
	  }
     }

     fclose(fp);

     _mw_flip_image((unsigned char *) image->red,sizeof(char),
		    pm->pm_ncol,pm->pm_nrow,FALSE);
     _mw_flip_image((unsigned char *) image->green,sizeof(char),
		    pm->pm_ncol,pm->pm_nrow,FALSE);
     _mw_flip_image((unsigned char *) image->blue,sizeof(char),
		    pm->pm_ncol,pm->pm_nrow,FALSE);

     free(pm);
     return(image);
}


/*~~~~~ Create 24-bit color PM CHAR ~~~~~*/

short _mw_ccimage_create_pm(char *file, Ccimage image)
{
     FILE *fp;
     pmpic   *pm;
     int isize;

     if (!(fp = fopen(file, "w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",file);
	  return(-1);
     }

     if ((image == NULL) || (image->red == NULL) || (image->green == NULL) 
	 || (image->blue == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_ccimage_create_pm] NULL ccimage or ccimage planes\n");
	  return(-2);
     }

     pm = (pmpic*)malloc(sizeof(pmpic));
     if (pm == NULL)
	  mwerror(FATAL,0,"Not enough memory to create any PM image !\n");

     mw_bzero(pm,sizeof(pmpic));
     pm->pm_id = PM_MAGICNO;
     pm->pm_np = 3;
     pm->pm_nband = 1;
     pm->pm_nrow = image->nrow;
     pm->pm_ncol = image->ncol;
     pm->pm_form = PM_C;
     pm->pm_cmtsize = strlen(image->cmt);
     pm->pm_image = NULL;
     pm->pm_cmt = NULL;

     if (fwrite(pm,PM_IOHDR_SIZE,1,fp) == 0)
     {
	  mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     isize = pm_isize(pm);
     if (isize != (3 * image->nrow * image->ncol * sizeof(char)))
     {
	  mwerror(INTERNAL, 0,"[_mw_ccimage_create_pm] Illegal image size\n");
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     if ( (fwrite(image->red,isize/3,1,fp) == 0) ||
	  (fwrite(image->green,isize/3,1,fp) == 0) ||
	  (fwrite(image->blue,isize/3,1,fp) == 0) )
     {
	  mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     if (pm->pm_cmtsize)
     {
	  if (fwrite(image->cmt,pm->pm_cmtsize,1,fp) == 0)
	  {
	       mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	       free(pm);
	       fclose(fp);
	       return(-1);
	  }
     }
  
     free(pm);
     fclose(fp);
     return(0);
}


/*~~~~~ Load color PM FLOAT ~~~~~*/

/* I guessed this format from the 24-bits format but I am not sure this is 
   really PM compatible... Have to check this ! 
*/

Cfimage _mw_cfimage_load_pm(char *file)
{
     int isize,cmtsize;
     char need_flipping = 0;
     pmpic *pm;
     FILE *fp;
     Cfimage image;

     pm = (pmpic*)malloc(sizeof(pmpic));
     if (pm == NULL)
	  mwerror(FATAL,0,"Not enough memory to load any PM image !\n");

     mw_bzero(pm,sizeof(pmpic));
     pm->pm_id = PM_MAGICNO;
     pm->pm_np = pm->pm_nband = 1;
     pm->pm_nrow = pm->pm_ncol = 512;
     pm->pm_form = PM_C;
     pm->pm_cmtsize = 0;
     pm->pm_image = NULL;
     pm->pm_cmt = NULL;

     if (!(fp = fopen(file, "r")))
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",file);
	  fclose(fp);
	  free(pm);
	  return(NULL);
     }
  
     if (fread(pm,PM_IOHDR_SIZE,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }
  
     if (pm->pm_id != PM_MAGICNO)
     {
	  _mw_in_flip_b4(pm->pm_id);
	  if (pm->pm_id == PM_MAGICNO)
	       need_flipping = 1;
     }

     if (pm->pm_id != PM_MAGICNO)
     {
	  mwerror(INTERNAL, 0,"[_mw_cfimage_load_pm] File \"%s\" is not in the PM format\n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }

     if (need_flipping)
     {
	  _mw_in_flip_b4(pm->pm_np);
	  _mw_in_flip_b4(pm->pm_nrow);
	  _mw_in_flip_b4(pm->pm_ncol);
	  _mw_in_flip_b4(pm->pm_nband);
	  _mw_in_flip_b4(pm->pm_form);
	  _mw_in_flip_b4(pm->pm_cmtsize);
     }

     image = mw_new_cfimage();
     if (image == NULL)
     {
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",file);
	  return(NULL);
     }

     switch(pm->pm_form ) /* Code FLOAT values and the color model */
     {
     case PM_F_RGB:
	  image->model=MODEL_RGB;
	  break;
     case PM_F_YUV:
	  image->model=MODEL_YUV;
	  break;
     case PM_F_HSI:
	  image->model=MODEL_HSI;
	  break;
     case PM_F_HSV:
	  image->model=MODEL_HSV;
	  break;      
     default:
	  mwerror(INTERNAL, 0,"[_mw_cfimage_load_pm] File \"%s\" is not in the FLOAT PM fomat or unknown color model\n",file);
     }

     isize = pm_isize(pm);
     if (isize != (3 * pm->pm_nrow * pm->pm_ncol * sizeof(float)))
     {
	  mwerror(INTERNAL, 0,"[_mw_cfimage_load_pm] PM file \"%s\" cannot be loaded into a cfimage\n",file);
	  free(pm);
	  fclose(fp);
	  return(NULL);
     }

     image = mw_change_cfimage(image,pm->pm_nrow,pm->pm_ncol);
     if (image == NULL)
     {
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",file);
	  return(NULL);
     }

     if (isize)
     {
	  if ( (fread(image->red,isize/3,1,fp) == 0)
	       || (fread(image->green,isize/3,1,fp) == 0)
	       || (fread(image->blue,isize/3,1,fp) == 0) )
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	       free(pm);
	       mw_delete_cfimage(image);
	       fclose(fp);
	       return(NULL);
	  }
     }
    
     cmtsize = pm->pm_cmtsize;
     if (pm->pm_cmtsize > mw_cmtsize) cmtsize = mw_cmtsize;
  
     if (cmtsize)
     {
	  if (fread(image->cmt,cmtsize,1,fp) == 0)
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a PM format or file corrupted !\n",file);      
	       free(pm);
	       mw_delete_cfimage(image);
	       fclose(fp);
	       return(NULL);
	  }
     }

     fclose(fp);

     _mw_flip_image((unsigned char *) image->red,sizeof(float),
		    pm->pm_ncol,pm->pm_nrow,need_flipping);
     _mw_flip_image((unsigned char *) image->green,sizeof(float),
		    pm->pm_ncol,pm->pm_nrow,need_flipping);
     _mw_flip_image((unsigned char *) image->blue,sizeof(float),
		    pm->pm_ncol,pm->pm_nrow,need_flipping);

     free(pm);
     return(image);
}


/*~~~~~ Create color PM FLOAT ~~~~~*/

/* I guessed this format from the 24-bits format but I am not sure this is 
   really PM compatible... Have to check this ! 
*/

short _mw_cfimage_create_pm(char *file, Cfimage image)
{
     FILE *fp;
     pmpic   *pm;
     int isize;

     if (!(fp = fopen(file, "w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",file);
	  return(-1);
     }

     if ((image == NULL) || (image->red == NULL) || (image->green == NULL) 
	 || (image->blue == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_cfimage_create_pm] NULL cfimage or cfimage planes\n");
	  return(-2);
     }

     pm = (pmpic*)malloc(sizeof(pmpic));
     if (pm == NULL)
	  mwerror(FATAL,0,"Not enough memory to create any PM image !\n");

     mw_bzero(pm,sizeof(pmpic));
     pm->pm_id = PM_MAGICNO;
     pm->pm_np = 3;
     pm->pm_nband = 1;
     pm->pm_nrow = image->nrow;
     pm->pm_ncol = image->ncol;

     switch(image->model) /* Code the color model */
     {
     case MODEL_RGB:
	  pm->pm_form = PM_F_RGB;
	  break;
     case MODEL_YUV:
	  pm->pm_form = PM_F_YUV;
	  break;
     case MODEL_HSI: 
	  pm->pm_form = PM_F_HSI;
	  break;
     case MODEL_HSV:
	  pm->pm_form = PM_F_HSV;
	  break;      
     default:
	  mwerror(WARNING,1,
		  "Unknown color model %d for output cfimage ! (record as RGB)\n",
		  image->model);
	  pm->pm_form = PM_F;
     }

     pm->pm_cmtsize = strlen(image->cmt);
     pm->pm_image = NULL;
     pm->pm_cmt = NULL;

     if (fwrite(pm,PM_IOHDR_SIZE,1,fp) == 0)
     {
	  mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     isize = pm_isize(pm);
     if (isize != (3 * image->nrow * image->ncol * sizeof(float)))
     {
	  mwerror(INTERNAL, 0,"[_mw_cfimage_create_pm] Illegal image size\n");
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     if ( (fwrite(image->red,isize/3,1,fp) == 0) ||
	  (fwrite(image->green,isize/3,1,fp) == 0) ||
	  (fwrite(image->blue,isize/3,1,fp) == 0) )
     {
	  mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	  free(pm);
	  fclose(fp);
	  return(-1);
     }

     if (pm->pm_cmtsize)
     {
	  if (fwrite(image->cmt,pm->pm_cmtsize,1,fp) == 0)
	  {
	       mwerror(ERROR,0,"Error while writing file \"%s\" !\n",file);
	       free(pm);
	       fclose(fp);
	       return(-1);
	  }
     }
  
     free(pm);
     fclose(fp);
     return(0);
}

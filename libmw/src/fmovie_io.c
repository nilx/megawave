/**
 * @file fmovie_io.c
 *
 * @version 1.5
 * @author Jacques Froment (1994 - 2004)
 * @author Nicolas Limare (2008 - 2009)
 *
 * input/output private functions for the Fmovie structure
 */

#include <stdio.h>
#include <string.h>

#include "libmw-defs.h"
#include "error.h"

#include "fimage.h"
#include "fimage_io.h"
#include "fmovie.h"
#include "ascii_file.h"

#include "fmovie_io.h"

extern int _mw_convert_struct_warning;

/* Ancien format (fichier d'entete vide) */

Fmovie _mw_fmovie_load_movie_old_format(char *NomFic, char *Type)
{
     FILE * fp;
     Fmovie movie;               
     Fimage image,image_next;
     char FicImage[BUFSIZ];
     char Ext[BUFSIZ];
     short num;
     /*short i;*/
      
     movie = NULL;
     strcpy(Type,"?");  /* Type a priori inconnu */
     Ext[0] = '\0';     /* Recherche d'une extension eventuelle */

     sprintf(FicImage,"%s_001",NomFic);
     if (NULL == (fp = fopen(FicImage, "r")))
	 mwerror(FATAL, 1, "First image file \"%s\" "
		 "not found or unreadable\n", FicImage);
     fclose(fp);
     image = (Fimage) _mw_fimage_load_image(FicImage,Type);
     if (image == NULL)
	 return(movie);

     movie = (Fmovie) mw_new_fmovie();
     movie->first = image;

     num = 1;
     do {
	  num++;
	  if (Ext[0] == '\0')
	       sprintf(FicImage,"%s_%3.3d",NomFic,num);
	  else
	       sprintf(FicImage,"%s_%3.3d.%s",NomFic,num,Ext);
	  if (NULL != (fp = fopen(FicImage, "r")))
	  {
	      fclose(fp);
	      if (_mw_convert_struct_warning >= 3)
		  _mw_convert_struct_warning = -1; /* Disable warnings */
	      image_next = (Fimage) _mw_fimage_load_image(FicImage,Type);
	      if (image_next != NULL) 
	      {
		  image->next = image_next;
		  image_next->previous = image;
		  image = image_next;
	      }
	  }
     } while (NULL != fp);

     image->next = NULL;
     (movie->first)->previous = NULL;

     _mw_convert_struct_warning = 0;  /* Set to 0 previous warning account */

     return(movie);
}

/* Nouveau format (avec fichier entete enumerant les fichiers images) */

Fmovie _mw_fmovie_load_movie(char *fname, char *Type)
{
     FILE *fp, *fp2;
     Fmovie movie;               
     Fimage image,image_next;
     char FicImage[BUFSIZ];
     char Fic[BUFSIZ];
     char PathName[BUFSIZ];
     int num,nimage;
     int ret,i;
      
     movie = NULL;
     strcpy(Type,"?");  /* Type a priori inconnu */
     _mw_convert_struct_warning = 0;  /* Set to 0 previous warning account */

     fp = fopen(fname, "r");
     if ((fp == NULL) || 
	 (_mw_fascii_search_string(fp,_MW_DATA_ASCII_FILE_HEADER) == EOF) )
     {
	  /* Essaye l'ancien format */
	  fclose(fp);
	  return(_mw_fmovie_load_movie_old_format(fname,Type));
     }

     if (_mw_fascii_search_string(fp,"def ?movie\n") == EOF)
     {
	  fclose(fp);
	  fp = fopen(fname, "r");
	  if (_mw_fascii_search_string(fp,"def ??movie\n") == EOF)
	       mwerror(ERROR, 0, "MegaWave2 Data Ascii File \"%s\" "
		       "does not contain a movie field !\n", fname);
     }
  
     if (_mw_fascii_get_field(fp,fname,"nimage:","%d\n",&nimage) != 1)
     {
	  fclose(fp);
	  return(NULL);
     }
  
     /* Get the pathname of fname */
     for (i=strlen(fname)-1;(i>=0) && (fname[i] != '/');i--) ;
     if (fname[i] == '/') 
     {
	  strncpy(PathName,fname,i+1); 
	  PathName[i+1] = '\0';
     }
     else strcpy(PathName,"./");

     num = 0;
     image = NULL;
     movie = NULL;
     do {
	  ret = fscanf(fp,"%s",Fic);
	  if ((ret == 1)&&(Fic[0] != '%')&&(Fic[0] != '#'))
	  {
	       if (Fic[0]=='/') /* Absolute pathname */
		    strcpy(FicImage,Fic);
	       else /* Relative pathname : add pathname of fname */
		    sprintf(FicImage,"%s%s",PathName,Fic);
	       if (NULL != (fp2 = fopen(FicImage, "r")))
	       {
		    fclose(fp2);
		    num++;
		    if (num == 1)  /* 1st image */
		    {
			 image = (Fimage) 
			     _mw_fimage_load_image(FicImage,Type);
			 if (image == NULL) return(movie);
			 movie = (Fmovie) mw_new_fmovie();
			 movie->first = image;
		    }
		    else
		    {
			 if (_mw_convert_struct_warning >= 3)
			      _mw_convert_struct_warning = -1; 
                              /* Disable warnings */
			 image_next = (Fimage) 
			     _mw_fimage_load_image(FicImage, Type);
			 if (image_next != NULL) 
			 {
			      image->next = image_next;
			      image_next->previous = image;
			      image = image_next;
			 }
		    }
	       }
	  } 
     } while ((ret == 1) && (NULL != fp2) && (num < nimage));

     if (image) image->next = NULL;
     if (movie) (movie->first)->previous = NULL;

     _mw_convert_struct_warning = 0;  /* Set to 0 previous warning account */

     if (num < nimage)
	  mwerror(WARNING, 0, "Only %d image(s) have been read "
		  "(%d expected):\n\t\tFile \"%s\" not found\n",
		  num, nimage, FicImage);
     return(movie);
}



short _mw_fmovie_create_movie(char *NomFic, Fmovie movie, char *Type)
{
     Fimage image;
     char FicImage1[BUFSIZ],FicImage2[BUFSIZ];
     char *BaseName,*c;
     int nimage,num;
     FILE *fp;

     if (movie == NULL) 
     {
	  mwerror(INTERNAL, 0,
		  "[_mw_fmovie_create_movie] Cannot create movie: "
		  "NULL fmovie.\n");
	  return(-1);
     }

     c = strrchr(NomFic,'/');
     if (c == NULL) BaseName = NomFic;
     else BaseName = c+1;

     /*
       printf("NomFic=%s Basename=%s\n",NomFic,BaseName);
     */

     fp =_mw_create_data_ascii_file(NomFic);
     if (fp == NULL) return(-1);

     fprintf(fp,"%%----- Fmovie -----\n");
     fprintf(fp,"def Fmovie\n");

     _mw_convert_struct_warning = 0;  /* Set to 0 previous warning account */

     /* Compute the number of images */
     nimage = 0;
     image = movie->first;
     while (image != NULL) 
     {
	  nimage++;
	  image = image->next;
     }
     fprintf(fp,"nimage: %d\n",nimage);
     if (nimage > 99999)
	  mwerror(INTERNAL, 1, "[_mw_fmovie_create_movie] "
		  "Too many images !\n");

     num = 0;
     image = movie->first;
     while (image != NULL)
     {
	  num++;
	  sprintf(FicImage1,"%s_%5.5d",NomFic,num);
	  sprintf(FicImage2,"%s_%5.5d",BaseName,num);
	  _mw_fimage_create_image(FicImage1,image,Type);
	  if (_mw_convert_struct_warning >= 3)
	      _mw_convert_struct_warning = -1;
	  fprintf(fp,"%s\n",FicImage2);
	  image = image->next;
     }
     fclose(fp);

     _mw_convert_struct_warning = 0;  /* Set to 0 previous warning account */
     return(0);
}

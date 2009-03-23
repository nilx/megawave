/**
 * @file cmovie_io.c
 *
 * @version 1.16
 * @author Jacques Froment (1993 - 2004)
 * @author Nicolas Limare (2008 - 2009)
 *
 * input/output private functions for the Cmovie structure
 */

#include <stdio.h>
#include <string.h>

#include "definitions.h"
#include "error.h"

#include "cimage_io.h"
#include "cmovie.h"
#include "ascii_file.h"
#include "file_type.h"
#include "type_conv.h"

#include "cmovie_io.h"

extern int _mw_convert_struct_warning;

/* Old format (empty header file) */

Cmovie _mw_cmovie_load_movie_old_format(char *NomFic, char *Type)
{
     FILE * fp;
     Cmovie movie;               
     Cimage image,image_next;
     char FicImage[BUFSIZ];
     char Ext[BUFSIZ];
     short num;
     /* short i; */
      
     movie = NULL;
     strcpy(Type,"?");  /* Type a priori inconnu */
     Ext[0] = '\0';     /* Recherche d'une extension eventuelle */

     sprintf(FicImage,"%s_001",NomFic);
     if (NULL == (fp = fopen(FicImage, "r")))
	  mwerror(FATAL, 1, "First image file \"%s\" not found "
		  "or unreadable\n",FicImage);
     fclose(fp);
     image = (Cimage) _mw_cimage_load_image(FicImage,Type);
     if (image == NULL) return(movie);

     movie = (Cmovie) mw_new_cmovie();
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
	       image_next = (Cimage) _mw_cimage_load_image(FicImage,Type);
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

/* Native formats (without conversion of the internal type) 
   Recognize old and new format (with header file listing image files)
*/

Cmovie _mw_cmovie_load_native(char *fname, char *Type)
{
     FILE * fp, * fp2 = NULL;
     Cmovie movie;               
     Cimage image,image_next;
     char FicImage[BUFSIZ];
     char Fic[BUFSIZ];
     char PathName[BUFSIZ];
     int num,nimage;
     int ret,i;
      
     movie = NULL;
     strcpy(Type,"?");  /* Unknown type */
     _mw_convert_struct_warning = 0;  /* Set to 0 previous warning account */

     fp = fopen(fname, "r");
     if ((fp == NULL) || 
	 (_mw_fascii_search_string(fp,_MW_DATA_ASCII_FILE_HEADER) == EOF))
     {
	  /* Essaye l'ancien format */
	  fclose(fp);
	  return(_mw_cmovie_load_movie_old_format(fname,Type));
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
	       fp2 = fopen(FicImage, "r");
	       if (NULL != fp2)
	       {
		    fclose(fp2);
		    num++;
		    if (num == 1)  /* 1st image */
		    {
			 image = (Cimage) _mw_cimage_load_image(FicImage,Type);
			 if (image == NULL) return(movie);
			 movie = (Cmovie) mw_new_cmovie();
			 movie->first = image;
		    }
		    else
		    {
			 if (_mw_convert_struct_warning >= 3)
			      _mw_convert_struct_warning = -1;
			 /* Disable warnings */
			 image_next = (Cimage) 
			     _mw_cimage_load_image(FicImage, Type);
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

/* All available formats */

Cmovie _mw_cmovie_load_movie(char *NomFic, char *Type)
{ 
     Cmovie mov;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(NomFic,Type,mtype,&hsize,&version);

     /* First, try native formats */
     mov = _mw_cmovie_load_native(NomFic,Type);
     if (mov != NULL) return(mov);

     /* If failed, try other formats with memory conversion */
     mov = (Cmovie) _mw_load_etype_to_itype(NomFic,mtype,"cmovie",Type);
     if (mov != NULL) return(mov);

     if (Type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",NomFic);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Cmovie !\n",NomFic,Type);
     return NULL;
}

short _mw_cmovie_create_movie(char *NomFic, Cmovie movie, char *Type)
{
     Cimage image;
     char FicImage1[BUFSIZ],FicImage2[BUFSIZ];
     char *BaseName,*c;
     int nimage,num;
     FILE *fp;

     if (movie == NULL) 
     {
	  mwerror(INTERNAL, 0,"[_mw_cmovie_create_movie] Cannot create movie:  NULL cmovie.\n");
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

     fprintf(fp,"%%----- Cmovie -----\n");
     fprintf(fp,"def Cmovie\n");

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
	  mwerror(INTERNAL,1,"[_mw_cmovie_create_movie] Too many images !\n");

     num = 0;
     image = movie->first;
     while (image != NULL)
     {
	  num++;
	  sprintf(FicImage1,"%s_%5.5d",NomFic,num);
	  sprintf(FicImage2,"%s_%5.5d",BaseName,num);
	  _mw_cimage_create_image(FicImage1,image,Type);
	  if (_mw_convert_struct_warning >= 3) _mw_convert_struct_warning = -1;
	  fprintf(fp,"%s\n",FicImage2);
	  image = image->next;
     }
     fclose(fp);

     _mw_convert_struct_warning = 0; 
     return(0);
}

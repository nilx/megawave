/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ccmovie_io.c
   
  Vers. 1.4
  (C) 1995-2004 Jacques Froment
  Input/Output private functions for the Ccmovie structure

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
/* FIXME : UNIX-centric */
#include <fcntl.h>
#include <unistd.h>

#include "libmw.h"
#include "utils.h"
#include "ccimage_io.h"
#include "ccmovie.h"
#include "ascii_file.h"
#include "file_type.h"
#include "type_conv.h"

#include "ccmovie_io.h"

extern int _mw_convert_struct_warning;

/* Old format (empty header file) */

Ccmovie _mw_ccmovie_load_movie_old_format(char *NomFic, char *Type)
{
     Ccmovie movie;               
     Ccimage image,image_next;
     char FicImage[BUFSIZ];
     char Ext[BUFSIZ];
     short f,num;
     /* short i; */
      
     movie = NULL;
     strcpy(Type,"?");  /* Type a priori inconnu */
     Ext[0] = '\0';     /* Recherche d'une extension eventuelle */

     sprintf(FicImage,"%s_001",NomFic);
     f = open(FicImage,O_RDONLY);
     /*
       if (f == -1) for (i=0; (ccimage_types[i] != NULL) && (f == -1); i++)
       {
       strcpy(Ext,ccimage_types[i]);
       sprintf(FicImage,"%s_001.%s",NomFic,Ext);
       f = open(FicImage,O_RDONLY);      
       if (f == -1) 
       {
       _mw_lower_type(Ext);
       sprintf(FicImage,"%s_001.%s",NomFic,Ext);
       f = open(FicImage,O_RDONLY);      
       }
       }
     */

     if (f == -1) 
	  mwerror(FATAL,1,"First image file \"%s\" not found or unreadable\n",FicImage);
     close(f);
     image = (Ccimage) _mw_ccimage_load_image(FicImage,Type);
     if (image == NULL) return(movie);

     movie = (Ccmovie) mw_new_ccmovie();
     movie->first = image;

     num = 1;
     do {
	  num++;
	  if (Ext[0] == '\0')
	       sprintf(FicImage,"%s_%3.3d",NomFic,num);
	  else
	       sprintf(FicImage,"%s_%3.3d.%s",NomFic,num,Ext);
	  f = open(FicImage,O_RDONLY);
	  if (f != -1) 
	  {
	       close(f);
	       if (_mw_convert_struct_warning >= 3)
		    _mw_convert_struct_warning = -1; /* Disable warnings */
	       image_next = (Ccimage) _mw_ccimage_load_image(FicImage,Type);
	       if (image_next != NULL) 
	       {
		    image->next = image_next;
		    image_next->previous = image;
		    image = image_next;
	       }
	  }
     } while (f != -1);

     image->next = NULL;
     (movie->first)->previous = NULL;

     _mw_convert_struct_warning = 0;  /* Set to 0 previous warning account */
     return(movie);
}

/* Native formats (without conversion of the internal type) 
   Recognize old and new format (with header file listing image files)
*/

Ccmovie _mw_ccmovie_load_native(char *fname, char *Type)
{
     FILE    *fp;

     Ccmovie movie;               
     Ccimage image,image_next;
     char FicImage[BUFSIZ];
     char Fic[BUFSIZ];
     char PathName[BUFSIZ];
     short f;
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
	  return(_mw_ccmovie_load_movie_old_format(fname,Type));
     }
     if (_mw_fascii_search_string(fp,"def ?movie\n") == EOF)
     {
	  fclose(fp);
	  fp = fopen(fname, "r");
	  if (_mw_fascii_search_string(fp,"def ??movie\n") == EOF)
	       mwerror(ERROR,0,"MegaWave2 Data Ascii File \"%s\" does not contain a movie field !\n",fname);
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
	       f = open(FicImage,O_RDONLY);
	       if (f != -1) 
	       {
		    close(f);
		    num++;
		    if (num == 1)  /* 1st image */
		    {
			 image = (Ccimage) _mw_ccimage_load_image(FicImage,Type);
			 if (image == NULL) return(movie);
			 movie = (Ccmovie) mw_new_ccmovie();
			 movie->first = image;
		    }
		    else
		    {
			 if (_mw_convert_struct_warning >= 3)
			      _mw_convert_struct_warning = -1; /* Disable warnings */
			 image_next = (Ccimage) _mw_ccimage_load_image(FicImage,Type);
			 if (image_next != NULL) 
			 {
			      image->next = image_next;
			      image_next->previous = image;
			      image = image_next;
			 }
		    }
	       }
	  } 
     } while ((ret == 1) && (f != -1) && (num < nimage));

     if (image) image->next = NULL;
     if (movie) (movie->first)->previous = NULL;

     _mw_convert_struct_warning = 0;  /* Set to 0 previous warning account */

     if (num < nimage)
	  mwerror(WARNING,0,"Only %d image(s) have been read (%d expected):\n\t\tFile \"%s\" not found\n",num,nimage,FicImage);

     return(movie);
}


/* All available formats */

Ccmovie _mw_ccmovie_load_movie(char *NomFic, char *Type)
{ 
     Ccmovie mov;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(NomFic,Type,mtype,&hsize,&version);

     /* First, try native formats */
     mov = _mw_ccmovie_load_native(NomFic,Type);
     if (mov != NULL) return(mov);

     /* If failed, try other formats with memory conversion */
     mov = (Ccmovie) _mw_load_etype_to_itype(NomFic,mtype,"ccmovie",Type);
     if (mov != NULL) return(mov);

     if (Type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",NomFic);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Ccmovie !\n",NomFic,Type);

     return NULL;
}

short _mw_ccmovie_create_movie(char *NomFic, Ccmovie movie, char *Type)
{
     Ccimage image;
     char FicImage1[BUFSIZ],FicImage2[BUFSIZ];
     char *BaseName,*c;
     char field[10];
     int nimage,num;
     FILE *fp;

     if (movie == NULL) 
     {
	  mwerror(INTERNAL, 0,"[_mw_ccmovie_create_movie] Cannot create movie:  NULL ccmovie.\n");
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

     fprintf(fp,"%%----- Ccmovie -----\n");
     fprintf(fp,"def Ccmovie\n");

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
     if (nimage < 100)
	  strcpy(field,"%s_%2.2d");
     else if (nimage < 1000)
	  strcpy(field,"%s_%3.3d");    
     else if (nimage < 10000)
	  strcpy(field,"%s_%4.4d");    
     else if (nimage < 100000)
	  strcpy(field,"%s_%5.5d");    
     else 
	  mwerror(INTERNAL,1,"[_mw_ccmovie_create_movie] Too many images !\n");

     num = 0;
     image = movie->first;
     while (image != NULL)
     {
	  num++;
	  sprintf(FicImage1,field,NomFic,num);
	  sprintf(FicImage2,field,BaseName,num);
	  _mw_ccimage_create_image(FicImage1,image,Type);
	  if (_mw_convert_struct_warning >= 3) _mw_convert_struct_warning = -1;
	  fprintf(fp,"%s\n",FicImage2);
	  image = image->next;
     }
     fclose(fp);

     _mw_convert_struct_warning = 0;  /* Set to 0 previous warning account */
     return(0);
}

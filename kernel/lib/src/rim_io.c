/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   rim_io.c
   
   Vers. 1.02
   Input/Output private functions for the Rim structure

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

/*~~~~~~ MegaWaveI formats ~~~~~*/

Rim _mw_rim_load_megawave1(NomFic,Type, Comment)

char  *NomFic;                        /* Nom du fichier image */
char  *Type;                          /* Type de format du fichier */
char  Comment[];                      /* Commentaires */

{ short   fic;                            /* fichier */
  unsigned short BufferEntete[4];         /* Buffer de lecture de l'entete */
  unsigned short bytesread;               /* Nbre de bytes lus */
  unsigned short minheader = 8; /* Nbre de bytes entete minimale du fichier */
  unsigned short header;        /* Nbre de bytes entete du format du fichier */
  long  TailleBuffer;                   /* Taille du Buffer en octets */
  short dx,dy;                          /* Taille de l'image du fichier */
  unsigned short taillezc;              /* Taille de la zone de commentaires */
  unsigned short n;                     /* Nbre de bytes a lire */
  long i,L;                             /* Indices courants sur l'image */
  
  Rim image;                /* Buffer memoire */
  Rim ptr;

  image = NULL;
  strcpy(Type,"?");  /* Type a priori inconnu */

       /* Ouverture du fichier */

  fic = open(NomFic, /* O_BINARY | */ O_RDONLY);
  if (fic == -1) mwerror(FATAL,1,"File \"%s\" not found or unreadable\n",NomFic);

         /* Lecture entete */
  bytesread = read(fic,BufferEntete,minheader);
  if (bytesread != minheader) 
    mwerror(FATAL,1,"Error while reading header of file \"%s\" (may be corrupted)\n",NomFic);

         /* Tests entete */

 if ((bytesread == minheader) && (BufferEntete[0] == 0x5249))
    {  

      /* RIM */
      strcpy(Type,"RIM");
      taillezc = _mw_get_flip_b2(BufferEntete[1]);
      dx = _mw_get_flip_b2(BufferEntete[2]);
      dy = _mw_get_flip_b2(BufferEntete[3]);
      header=64+taillezc;

      /* Read the comments */
      if (taillezc > 0)
	{
	  if (lseek(fic,64,L_SET) == -1L) 
	    mwerror(FATAL,1,"RIM image header file \"%s\" is corrupted\n",NomFic);

	  bytesread = read(fic,Comment,taillezc);
	  if (bytesread != taillezc) 
	    mwerror(FATAL,1,"RIM image header file \"%s\" is corrupted\n",NomFic);
	  Comment[taillezc] = '\0';
	}
      else Comment[0] = '\0';
    }

 else
 if ((bytesread == minheader) && (BufferEntete[0] == 0x494D))
     {  /* IMG */
      strcpy(Type,"IMG");
      dx = _mw_get_flip_b2(BufferEntete[2]);
      dy = _mw_get_flip_b2(BufferEntete[3]);
      Comment[0] = '\0';
      mwerror(FATAL,1,"The image file \"%s\" is not with FLOAT values\n",NomFic);
    }

 else
   if ((BufferEntete[3] == 1) && (BufferEntete[2] == 256))
     {  /* INR */
       strcpy(Type,"INR");
       dx = BufferEntete[0];
       dy = BufferEntete[1];
       Comment[0] = '\0';
       mwerror(FATAL,1,"The image file \"%s\" is not with FLOAT values\n",NomFic);
     }

   else
     if ((BufferEntete[2] == 8) && (BufferEntete[3] == 0))
       {  /* MTI */
	 strcpy(Type,"MTI");
	 dx = BufferEntete[0];
	 dy = BufferEntete[1];
	 Comment[0] = '\0';
	 mwerror(FATAL,1,"The image file \"%s\" is not with FLOAT values\n",NomFic);
       }
     else 
       mwerror(FATAL,1,"Format of the image file \"%s\" unknown\n",NomFic);
  
  /* Reservation memoire */

  image = (Rim) calloc((long)dx*dy+2,sizeof(Rim));
  if (image == NULL)
    {
      mwerror(ERROR, 0,"Not enough memory to load image \"%s\"\n",NomFic);
      return(image);
    }
  image[0] = (float) dx;
  image[1] = (float) dy;

  /* On se positionne sur le debut de la zone pixel (0,0) */

  if (lseek(fic,header,L_SET) == -1L) 
    {
      if (image != NULL) cfree(image);
      image = NULL;
      mwerror(FATAL,1,"Format of the image file \"%s\" unknown\n",NomFic);
      return(image);
    }

  ptr = image+2;
  i=0;
  L = sizeof(float) * ( (long) dx ) * dy;
  while (L>0) {
    if (L > 65000L) n = (65000/sizeof(float))*sizeof(float); else n = L;
    L -= n;
    bytesread = read(fic,ptr+i,n);
    if (bytesread != n) 
      {
	if (image != NULL) cfree(image);
	image = NULL;
	mwerror(FATAL,1,"Error while reading file \"%s\" (file may be corrupted)\n",NomFic); 
	return(image);
      }
    i += (n/sizeof(float));
  }

  close(fic);

  _mw_flip_image((unsigned char *) ptr, sizeof(float), dx,dy, FALSE);
  return(image);
}


short _mw_rim_create_megawave1(NomFic,image,Type,Comment)

     char  *NomFic;                        /* Nom du fichier image */
     Rim image;
     char  *Type;                          /* Type de format du fichier */
     char Comment[];

{
  int   fic;                            /* fichier */
  unsigned short byteswrite;            /* Nbre de bytes ecrits */
  unsigned short header;                /* Nbre de bytes entete du fichier */
  short dx,dy;                          /* Taille de l'image */
  unsigned short taillezc;              /* Taille de la zone de commentaires */
  unsigned short n;                     /* Nbre de bytes a lire */
  long i,l;                             /* Indices courants sur l'image */
  float *ptr; 

  dx = (short) image[0];
  dy = (short) image[1];

  if (Type[0] == '?') strcpy(Type,fimage_types[0]);
  if (strcmp(Type,"RIM") == 0) 
    {
      if (_MAKEHEADER_RIM(NomFic,dx,dy,Comment,0.0,0.0) != 0) 
	{
	  mwerror(FATAL,1,"Unable to write in the file \"%s\"\n",NomFic);
	  return(-1);
	}
      taillezc = strlen(Comment);
      header = 64;
    }
  else
    mwerror(INTERNAL,1,"[_mw_rim_create_image] Unknown format \"%s\"\n",
	    Type);

  if ( ( (fic = open(NomFic,O_WRONLY)) == -1) || 
      (lseek(fic,(long)taillezc+header,L_SET) != taillezc+header) )
    {
      mwerror(FATAL,1,"Unable to write in the file \"%s\"\n",NomFic);
      return(-1);
    }

  l = sizeof(float)*((long) dx) * dy;
  
  i=0;
  ptr = image+2;
  while (l>0) 
    {
      if (l > 65000L)  n = (65000/sizeof(float))*sizeof(float);
      else n = l;
      l -= n;
      byteswrite = write(fic,ptr+i,n);
      if (byteswrite != n) 
	{
	  mwerror(FATAL,1,"Error while writing in the file \"%s\"\n",NomFic);
	  return(-1);
	}
      i +=(n/sizeof(float));
  }
  close(fic);
  return(0);
}



/*~~~~~~ MegaWave2 formats ~~~~~*/

Rim _mw_rim_load_image(NomFic,Type,Comment)

char  *NomFic;                        /* Nom du fichier image */
char  *Type;                          /* Type de format du fichier */
char Comment[];

{ Rim image_rim;
  Img image_img;
  char mtype[TYPE_SIZE];
  
  _mw_get_file_type(NomFic,Type,mtype);
  
  if (strcmp(Type,"RIM") == 0)
    return(_mw_rim_load_megawave1(NomFic,Type,Comment));

  if ((strcmp(Type,"IMG") == 0)  || (Type[0] == '?'))
    /* ? Includes INR and MTI types (not supported by _mw_get_file_type()) */
    {
      image_img = (Img) _mw_img_load_megawave1(NomFic,Type, Comment);
      image_rim = (Rim) mw_img_to_rim(image_img);
      cfree(image_img);
      return(image_rim);
    }
  
  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,NomFic);  

}


short _mw_rim_create_image(NomFic,image,Type,Comment)

     char  *NomFic;                        /* Nom du fichier image */
     Rim image;
     char  *Type;                          /* Type de format du fichier */
     char Comment[];

{ Img image_img;

  if (strcmp(Type,"RIM") == 0)
    return(_mw_rim_create_megawave1(NomFic,image,Type,Comment));

  if ((strcmp(Type,"IMG") == 0) || (strcmp(Type,"INR") == 0) || 
      (strcmp(Type,"MTI") == 0))
    {
      image_img = (Img) mw_rim_to_img(image);
      cfree(image);
      return(_mw_img_create_megawave1(NomFic,image_img,Type,Comment));
    }
  
  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,NomFic);  
}




/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  img_io.c
   
  Vers. 1.04
  (C) 1993-2000 Jacques Froment
  Input/Output functions for the Img structure

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

Img _mw_img_load_megawave1(char * NomFic, char * Type, char * Comment)
{
     short   fic;                            /* fichier */
     unsigned short BufferEntete[4];         /* Buffer de lecture de l'entete */
     unsigned short bytesread;               /* Nbre de bytes lus */
     unsigned short minheader = 8; /* Nbre de bytes entete minimale du fichier */
     unsigned short header;        /* Nbre de bytes entete du format du fichier */
     long  TailleBuffer;                   /* Taille du Buffer en octets */
     short dx,dy;                          /* Taille de l'image du fichier */
     unsigned short taillezc;              /* Taille de la zone de commentaires */
     unsigned short n;                       /* Nbre de bytes a lire */
     long i,L;                               /* Indices courants sur l'image */
     unsigned char *image;                 /* Buffer memoire */

     image = NULL;
     strcpy(Type,"?");  /* Type a priori inconnu */

     /* Ouverture du fichier */

     fic = open(NomFic, /* O_BINARY | */ O_RDONLY);
     if (fic == -1) mwerror(FATAL, 0,"File \"%s\" not found or unreadable\n",NomFic);

     /* Lecture entete */
     bytesread = read(fic,BufferEntete,minheader);
     if (bytesread != minheader) 
     {
	  mwerror(ERROR, 0,"Error while reading header of file \"%s\" (may be corrupted)\n",NomFic);
	  return(NULL);
     }

     /* Tests entete */

     if ((bytesread == minheader) && (BufferEntete[0] == 0x494D))
     {  /* IMG */
	  strcpy(Type,"IMG");
	  taillezc = _mw_get_flip_b2(BufferEntete[1]);
	  dx = _mw_get_flip_b2(BufferEntete[2]);
	  dy = _mw_get_flip_b2(BufferEntete[3]);
	  header=64+taillezc;

	  /* Read the comments */
	  if (taillezc > 0)
	  {
	       if (lseek(fic,64,L_SET) == -1L) 
	       {
		    mwerror(ERROR, 0,"IMG image header file \"%s\" is corrupted\n",NomFic);
		    return(NULL);
	       }
	       bytesread = read(fic,Comment,taillezc);
	       if (bytesread != taillezc) 
	       {
		    mwerror(ERROR, 0,"IMG image header file \"%s\" is corrupted\n",NomFic);
		    return(NULL);
	       }
	       Comment[taillezc] = '\0';
	  }
	  else Comment[0] = '\0';
     }

     else
	  if ((bytesread == minheader) && (BufferEntete[0] == 0x5249))
	  {  /* RIM */
	       strcpy(Type,"RIM");
	       dx = _mw_get_flip_b2(BufferEntete[2]);
	       dy = _mw_get_flip_b2(BufferEntete[3]);
	       Comment[0] = '\0';
	       mwerror(ERROR, 0,"The image file \"%s\" is not with CHAR values\n",NomFic);
	       return(NULL);
	  }

	  else
	       if ((BufferEntete[3] == 1) && (BufferEntete[2] == 256))
	       {  /* INR */
		    strcpy(Type,"INR");
		    dx = BufferEntete[0];
		    dy = BufferEntete[1];
		    header=256;
		    Comment[0] = '\0';
	       }

	       else
		    if ((BufferEntete[2] == 8) && (BufferEntete[3] == 0))
		    {  /* MTI */
			 strcpy(Type,"MTI");
			 dx = BufferEntete[0];
			 dy = BufferEntete[1];
			 header=8;
			 Comment[0] = '\0';
		    }
		    else 
		    {
			 mwerror(ERROR, 0,"Format of the image file \"%s\" unknown\n",NomFic);
			 return(NULL);
		    }

     /* Reservation memoire */

     image = (Img) calloc((long)dx*dy+4,sizeof(unsigned char));
     if (image == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to load image \"%s\"\n",NomFic);
	  return(image);
     }

     *image = (unsigned char) dx & 255;
     *(image+1) = (unsigned char) (dx>>8);
     *(image+2) = (unsigned char) dy & 255;
     *(image+3) = (unsigned char ) (dy>>8);

     /* On se positionne sur le debut de la zone pixel (0,0) */

     if (lseek(fic,header,L_SET) == -1L) 
     {
	  if (image != NULL) cfree(image);
	  image = NULL;
	  mwerror(ERROR, 0,"Format of the image file \"%s\" unknown\n",NomFic);
	  return(image);
     }

     i=4L;
     L = ( (long) dx ) * dy;
     while (L>0) {
	  if (L > 65000L) n = 65000; else n = L;
	  L -= n;
	  bytesread = read(fic,image+i,n);
	  if (bytesread != n) 
	  {
	       if (image != NULL) cfree(image);
	       image = NULL;
	       mwerror(ERROR, 0,"Error while reading file \"%s\" (file may be corrupted)\n",NomFic); 
	       return(image);
	  }
	  i += bytesread;
     }

     close(fic);

     _mw_flip_image((unsigned char *) image+4,sizeof(char),dx,dy,FALSE);
     return(image);
}


short _mw_img_create_megawave1(char * NomFic, Img * image,
			       char * Type, char * Comment)
{
     if (Type[0] == '?') strcpy(Type,cimage_types[0]);

     if (strcmp(Type,"IMG") ==0)
	  return(CREATE_IMG(NomFic,image,32767,Comment));
     if (strcmp(Type,"MTI") ==0)
	  return(CREATE_MTI(NomFic,image,32767));
     if (strcmp(Type,"INR") ==0)
	  return(CREATE_INR(NomFic,image,32767));

     mwerror(INTERNAL,1,"[mw_img_create_megawave1] Unknown type \"%s\"\n",Type);
}   


/*~~~~~~ MegaWave2 formats ~~~~~*/

Img _mw_img_load_image(char * NomFic, char * Type, char Comment[])
{
     Rim image_rim;
     Img image_img;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(NomFic,Type,mtype,&hsize,&version);
  
     if ((strcmp(Type,"IMG") == 0) || (Type[0] == '?'))
	  /* ? Includes INR and MTI types (not supported by _mw_get_file_type()) */
	  return(_mw_img_load_megawave1(NomFic,Type,Comment));

     if (strcmp(Type,"RIM") == 0)
     {
	  image_rim = (Rim) _mw_rim_load_megawave1(NomFic,Type, Comment);
	  image_img = (Img) mw_rim_to_img(image_rim);
	  cfree(image_rim);
	  return(image_img);
     }

     mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,NomFic);  

}


short _mw_img_create_image(char * NomFic, Img image, char * Type,
			   char Comment[])
{
     Rim image_rim;

     if ((strcmp(Type,"IMG") == 0) || (strcmp(Type,"INR") == 0) || 
	 (strcmp(Type,"MTI") == 0))
	  return(_mw_img_create_megawave1(NomFic,image,Type,Comment));

     if (strcmp(Type,"RIM") == 0)
     {
	  image_rim = (Rim) mw_img_to_rim(image);
	  cfree(image);
	  return(_mw_rim_create_megawave1(NomFic,image_rim,Type,Comment));
     }
  
     mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,NomFic);  

}

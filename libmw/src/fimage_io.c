/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  fimage_io.c
   
  Vers. 1.12
  Author : Jacques Froment
  Input/Output private functions for the Fimage structure

  Main changes :
  v1.12 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <string.h>
#include <stdio.h>
/* FIXME : UNIX-centric */
#include <fcntl.h>
#include <unistd.h>

#include "libmw-defs.h"
#include "mw.h"
#include "fimage.h"
#include "mwio.h"
#include "file_type.h"
#include "type_conv.h"
#include "pm_io.h"

#include "fimage_io.h"

/*~~~~~~ MegaWaveI formats ~~~~~*/

#define PROTECT_NORMAL 0644  /* rw- pour l'utilisateur, r-- pour les autres */

/*--- Cree l'entete d'un fichier <NomFic> au format RIM, ayant      ---*/
/*--- pour dimension d'image <dx>,<dy>, pour commentaires <comment> ---*/
/*--- et pour bornes d'abcisse [a,b].                               ---*/
/* Signifie qu'au cas ou NomFic represente une serie de courbes 1D, */
/* la ligne y = 0 correspond a l'abcisse x = a et y = dy-1 a x = b. */
/* Retourne 0 si OK, -1 si creation impossible ou erreur d'ecriture */

static short _MAKEHEADER_RIM(char *NomFic, 
			     short int dx, short int dy, 
			     char *comment, float a, float b)
{
     short fic;
     unsigned short BufferEntete[160];       /* Buffer de lecture de l'entete */
     short i;
     unsigned short byteswrite;
     unsigned short header = 64;             /* Nbre de bytes entete du fichier */
     short lencomm;
     char A[20],B[20];     /* forme Ascii de a,b */
     short lenfloat = 14;  /* Longueur en bytes d'un float sous forme Ascii */

     fic = open(NomFic,O_WRONLY | O_CREAT | O_TRUNC,PROTECT_NORMAL);
     if (fic == -1) return(-1);

     BufferEntete[0] = 0x5249;
     lencomm = strlen(comment);
     BufferEntete[1] = _mw_get_flip_b2(lencomm);
     BufferEntete[2] = _mw_get_flip_b2(dx);
     BufferEntete[3] = _mw_get_flip_b2(dy);

     sprintf(A,"%f",a);
     sprintf(B,"%f",b);
     memcpy(&BufferEntete[4],A,lenfloat);
     memcpy(&BufferEntete[4+lenfloat/2],B,lenfloat);

     for (i=4+lenfloat;i<32; i++) BufferEntete[i] = 0;

     memcpy(&BufferEntete[32],comment,lencomm);
     byteswrite = write(fic,BufferEntete,header+lencomm);
     close(fic);
     if (byteswrite != header+lencomm) return(-1);
     return(0);
}


Fimage _mw_fimage_load_megawave1(char *NomFic, char *Type)
{
     short   fic;                            /* fichier */
     unsigned short BufferEntete[4];         /* Buffer de lecture de l'entete */
     unsigned short bytesread;               /* Nbre de bytes lus */
     unsigned short minheader = 8; /* Nbre de bytes entete minimale du fichier */
     unsigned short header;        /* Nbre de bytes entete du format du fichier */
     unsigned short dx,dy;                 /* Taille de l'image du fichier */
     unsigned short taillezc;              /* Taille de la zone de commentaires */
     unsigned short n;                     /* Nbre de bytes a lire */
     long i,L;                             /* Indices courants sur l'image */
  
     Fimage image;                /* Buffer memoire */
     char  Comment[mw_cmtsize];   /* Commentaires */
     float *ptr;

     unsigned char need_flipping = FALSE;

     image = NULL;
     strcpy(Type,"?");  /* Type a priori inconnu */

     /* Ouverture du fichier */

     fic = open(NomFic, /* O_BINARY | */ O_RDONLY);
     if (fic == -1) 
	  mwerror(FATAL,1,"File \"%s\" not found or unreadable\n",NomFic);

     /* Lecture entete */
     bytesread = read(fic,BufferEntete,minheader);
     if (bytesread != minheader) 
	  mwerror(FATAL,1,"Error while reading header of file \"%s\" (may be corrupted)\n",NomFic);

     /* Tests entete */

     if ((bytesread == minheader) && 
	 ((BufferEntete[0] == 0x5249)||(BufferEntete[0] == 0x4952)))
     {  
	  /* RIM */
	  strcpy(Type,"RIM");
	  if (BufferEntete[0] == 0x5249)
	  {
	       taillezc = _mw_get_flip_b2(BufferEntete[1]);
	       dx = _mw_get_flip_b2(BufferEntete[2]);
	       dy = _mw_get_flip_b2(BufferEntete[3]);
	  }
	  else
	  {
	       taillezc = BufferEntete[1];
	       dx = BufferEntete[2];
	       dy = BufferEntete[3];
	       need_flipping = TRUE;
	  }
	  header=64+taillezc;

	  /* Read the comments */
	  if (taillezc > 0)
	  {
	       if (lseek(fic,64,SEEK_SET) == -1L) 
		    mwerror(FATAL,1,"RIM image header file \"%s\" is corrupted\n",NomFic);
	       bytesread = read(fic,Comment,taillezc);
	       if (bytesread != taillezc) 
		    mwerror(FATAL,1,"RIM image header file \"%s\" is corrupted\n",NomFic);
	       Comment[taillezc] = '\0';
	  }
	  else Comment[0] = '\0';
     }

     else
	  if ((bytesread == minheader) && 
	      ((BufferEntete[0] == 0x494D)||(BufferEntete[0] == 0x4D49)))
	  {  /* IMG */
	       strcpy(Type,"IMG");
	       mwerror(FATAL,1,"The image file \"%s\" is not with FLOAT values\n",NomFic);
	  }

	  else
	       if ((BufferEntete[3] == 1) && (BufferEntete[2] == 256))
	       {  /* INR */
		    strcpy(Type,"INR");
		    mwerror(FATAL,1,"The image file \"%s\" is not with FLOAT values\n",NomFic);
	       }

	       else
		    if ((BufferEntete[2] == 8) && (BufferEntete[3] == 0))
		    {  /* MTI */
			 strcpy(Type,"MTI");
			 mwerror(FATAL,1,"The image file \"%s\" is not with FLOAT values\n",NomFic);
		    }
		    else 
			 mwerror(FATAL,1,"Format of the image file \"%s\" unknown\n",NomFic);
  
     /* Reservation memoire */

     image = mw_change_fimage(NULL,dy,dx);
     strcpy(image->cmt,Comment);

     /* On se positionne sur le debut de la zone pixel (0,0) */

     if (lseek(fic,header,SEEK_SET) == -1L) 
     {
	  mw_delete_fimage(image);
	  image = NULL;
	  mwerror(FATAL,1,"Format of the image file \"%s\" unknown\n",NomFic);
	  return(image);
     }

     ptr = image->gray;
     i=0;
     L = sizeof(float) * ( (long) dx ) * dy;
     while (L>0) {
	  if (L > 65000L) n = (65000/sizeof(float))*sizeof(float); else n = L;
	  L -= n;
	  bytesread = read(fic,ptr+i,n);
	  if (bytesread != n) 
	  {
	       mw_delete_fimage(image);
	       image = NULL;
	       mwerror(FATAL,1,"Error while reading file \"%s\" (file may be corrupted)\n",NomFic); 
	       return(image);
	  }
	  i += (n/sizeof(float));
     }

     close(fic);

     _mw_flip_image((unsigned char *) ptr, sizeof(float), dx,dy, need_flipping);
     return(image);
}


short _mw_fimage_create_megawave1(char *NomFic, Fimage image, char *Type)
{
     int   fic;                            /* fichier */
     unsigned short byteswrite;            /* Nbre de bytes ecrits */
     unsigned short header;                /* Nbre de bytes entete du fichier */
     unsigned short dx,dy;                 /* Taille de l'image */
     unsigned short taillezc;              /* Taille de la zone de commentaires */
     unsigned short n;                     /* Nbre de bytes a lire */
     long i,l;                             /* Indices courants sur l'image */
     float *ptr; 

     if ((image->ncol>=65536)||(image->nrow>=65536))
     {
	  mwerror(FATAL,1,"Image too big to be saved using %s external type !\n",
		  Type);
	  return(-1);
     }

     dx = image->ncol;
     dy = image->nrow;

     if (Type[0] == '?') _mw_put_range_type(Type,"fimage",1);
     if (strcmp(Type,"RIM") == 0) 
     {
	  if (_MAKEHEADER_RIM(NomFic,dx,dy,image->cmt,0.0,0.0) != 0) 
	  {
	       mwerror(FATAL,1,"Unable to write in the file \"%s\"\n",NomFic);
	       return(-1);
	  }
	  taillezc = strlen(image->cmt);
	  header = 64;
     }
     else
	  mwerror(INTERNAL,1,"[_mw_fimage_create_megawave1] Unknown format \"%s\"\n",
		  Type);

     if ( ( (fic = open(NomFic,O_WRONLY)) == -1) || 
	  (lseek(fic,(long)taillezc+header,SEEK_SET) != taillezc+header) )
     {
	  mwerror(FATAL,1,"Unable to write in the file \"%s\"\n",NomFic);
	  return(-1);
     }

     l = sizeof(float)*((long) dx) * dy;
  
     i=0;
     ptr = image->gray;
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

/* Native formats (without conversion of the internal type) */

/* Return != NULL if load OK */

Fimage _mw_fimage_load_native(char *NomFic, char *Type)
{ 
     if (strcmp(Type,"RIM") == 0)
	  return(_mw_fimage_load_megawave1(NomFic,Type));
  
     if (strcmp(Type,"PM_F") == 0)
	  /* PM format with pm_form=PM_F and pm_np = pm_nband = 1 */
	  return((Fimage) _mw_fimage_load_pm(NomFic));

     return(NULL);
}

/* Return 0 if create OK */

short _mw_fimage_create_native(char *NomFic, Fimage image, char *Type)
{ 
     if (strcmp(Type,"RIM") == 0)
     {
	  if ((image->ncol>=65536)||(image->nrow>=65536))
	  {
	       mwerror(WARNING,1,"Image too big to be saved using %s external type. Using PM_F...\n",Type);
	       strcpy(Type,"PM_F");
	  }
	  else return(_mw_fimage_create_megawave1(NomFic,image,Type));
     }

     if (strcmp(Type,"PM_F") == 0)
	  /* PM format with pm_form=PM_F and pm_np = pm_nband = 1 */
	  return( _mw_fimage_create_pm(NomFic,image));

     return(-1);
}


/* All available formats */

Fimage _mw_fimage_load_image(char *NomFic, char *Type)
{ 
     Fimage im;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(NomFic,Type,mtype,&hsize,&version);

     /* First, try native formats */
     im = _mw_fimage_load_native(NomFic,Type);
     if (im != NULL) return(im);

     /* If failed, try other formats with memory conversion */
     im = (Fimage) _mw_load_etype_to_itype(NomFic,mtype,"fimage",Type);
     if (im != NULL) return(im);

     if (Type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",NomFic);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Fimage !\n",NomFic,Type);
  
     return NULL;
}


short _mw_fimage_create_image(char *NomFic, Fimage image, char *Type)
{ 
     short ret;

     /* First, try native formats */
     ret = _mw_fimage_create_native(NomFic,image,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(NomFic,image,"fimage",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",NomFic);  
     return -1;
}

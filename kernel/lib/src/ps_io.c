/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ps_io.c
   
   Vers. 1.3
   (C) 1995-2000 Jacques Froment

   Input/Output functions for PostScript image file

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <time.h>

#include "mw.h"

/* Table de conversion Hexa -> Ascii */
char  hval[] = {'0', '1', '2', '3', '4', '5', '6',
		  '7', '8', '9','a', 'b', 'c', 'd', 'e', 'f'}; 
	

/*~~~~~ Load PS ~~~~~*/

Cimage _mw_cimage_load_ps(fname)

char *fname;

{
  Cimage image;
  FILE *fp;
  int Height,Width;

  fp = fopen(fname,"r");
  if (!fp) 
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      return(NULL);
  }
  
  mwerror(FATAL,0,"Sorry, PS file format for input not available ! (why don't you use Ghostscript ?)\n");

  /* TO DO */
  Height = Width = 0;
  
  image = mw_change_cimage(NULL,Height,Width);
  if (image == NULL)
    mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",fname);

  _mw_flip_image((unsigned char *) image->gray,sizeof(char),image->ncol,
		 image->nrow,FALSE);
  
  return(image);
}

/* Used by _mw_cimage_create_ps() : remove in s special characters which
   may cause an error to the PostScript printer.
*/

static void remove_special_char(s)

char *s;

{
  while (*s != '\0')
    {
      switch(*s)
	{
	case '(' : 
	  *s='[';
	  break;
	case ')':
	  *s=']';
	  break;
	case '"':
	  *s='\'';
	  break;
	}
      s++;
    }
}


/*~~~~~ Write PS ~~~~~*/

short _mw_cimage_create_ps(fname,image)

char *fname;
Cimage image;

{
  FILE *fp;
  int  PSfd;

#define   MAXBUFSIZE  10000  /* Taille max du Buffer fichier PostScript */
#define   PSHEADS     1000

  unsigned char	PSfbuf[MAXBUFSIZE]; /* Buffer fichier PostScript */
  char		headbuf[PSHEADS];   /* tableau commandes PS */
  char		hbuf[PSHEADS];	    /* tampon commandes PS */ 

  unsigned int  bufsize=MAXBUFSIZE;
  int pixoffs;
  int PSfcmd;			/* nbre caracteres commandes PS */

  /* PostScript Scale */
#define BGST 740
#define	LWST	540
#define XPIX	228
#define	YPIX	158

  /* Margins */
#define LM 30
#define UM 40

  float 	pagratio = (float) BGST / LWST; /* hauteur/largeur page */
  float 	PSsx;			/* echelle image */
  float 	PSsy;
  float 	PSorx;			/* origines image */
  float 	PSory;

  /* Number of lines */
  int	bdy;

  float	bsy;
  float	boy;
  int	cntdy = 0;
	
  /* indices boucle de conversion */
  unsigned int j;
  unsigned int pixcnt;

  /* Date */
  time_t        Date;                   /* Date et heure du systeme */
  struct tm     *Gmt;                   /* Convertion en structure GMT */

  /* Comments */
#define   MAXLCOM   80      /* Longueur maxi indicative d'une ligne de comm.*/
  char          lcom[MAXLCOM+30];
  char          *comm;
  int lg,i;
#define LgDep 45

  if ((image == NULL) || (image->gray == NULL))
    {
      mwerror(INTERNAL, 0,"[_mw_cimage_create_ps] NULL cimage or cimage plane\n");
      return(-2);
    }

  if (!(fp = fopen(fname, "w")))
    {
      mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",fname);
      return(-1);
    }
  PSfd = fileno(fp);

  /* Header */
  strcpy(headbuf,"%!PS-Adobe\n");
  strcat(headbuf,"%Creator : MegaWave2 (PS format)\n");
  strcat(headbuf,"newpath\n");

  sprintf(hbuf,"/Times-Roman findfont 15 scalefont setfont\n60 %hd moveto\n",
	  LgDep);
  strcat(headbuf,hbuf);

  time(&Date); 
  Gmt = gmtime(&Date);
  sprintf(hbuf, "(\"%s\") show\n350 %hd moveto\n",fname,LgDep);
  strcat(headbuf,hbuf);
  strcat(headbuf, "/Times-Roman findfont 8 scalefont setfont\n");
  sprintf(hbuf,"(%d/%d/%d) show\n",(*Gmt).tm_mday,(*Gmt).tm_mon+1,(*Gmt).tm_year);
  strcat(headbuf,hbuf);

  strcat(headbuf, "/Times-Roman findfont 15 scalefont setfont\n");
  sprintf(hbuf, "480 %hd moveto\n",LgDep);
  strcat(headbuf,hbuf);
  strcat(headbuf,"(MegaWave2) show\n");


  PSfcmd = strlen(headbuf);
  write(PSfd,headbuf,PSfcmd);
  
  /* Comments */
  strcpy(headbuf,"/Times-BoldItalic findfont 10 scalefont setfont\n");
  comm = (char *) image->cmt;

  lg = LgDep-20;
  while ( *comm != '\0'  ) 
    {
      i=MAXLCOM;
      while ( (i<strlen(comm)) && (comm[i] != ' ') ) i++;
      strncpy(lcom,comm,i);
      lcom[i++] = '\0';
      comm = (char *) &comm[i];
      sprintf(hbuf,"60 %d moveto\n",lg);
      strcat(headbuf,hbuf);
      remove_special_char(lcom);
      sprintf(hbuf,"(%s) show\n",lcom);
      strcat(headbuf,hbuf);
      lg -= 10;
    }
  PSfcmd = strlen(headbuf);
  write(PSfd,headbuf,PSfcmd);

  /* DATA */

  bdy = (bufsize / image->ncol) >> 1;
  if (bdy <= 0) 
    mwerror(INTERNAL,1,"[_mw_cimage_create_ps] Cannot write PS image with so many columns !\n");
    
  /* Remember original state */
  sprintf(headbuf,"/origstate save def\n");

  if (image->ncol > image->nrow) 
    /* LANDSCAPE */
    {	sprintf (headbuf, "/origstate save def\n-90 rotate\n");
	if (image->ncol >= (image->nrow * pagratio))
	  {	PSsx = BGST;
		PSsy = ((float) PSsx * image->nrow) / image->ncol;
		PSorx = - 800;
		PSory = LWST + LM - PSsy;
	      }
	else
	  {	PSsy = LWST;
		PSsx = ((float) PSsy * image->ncol) / image->nrow;
		PSorx = - 800;
		PSory = LM;
	      }
	}
  else
    /* PORTRAIT */
    {	sprintf (headbuf, "/origstate save def\n");
	if (image->ncol >= (image->nrow / pagratio))
	  {	PSsx = LWST;
		PSsy = ((float) PSsx * image->nrow) / image->ncol;
		PSorx = LM;
		PSory = BGST + UM - PSsy;
	      }
	else
	  {	PSsy = BGST;
		PSsx = ((float) PSsy * image->ncol) / image->nrow;
		PSorx = LM;
		PSory = UM;
	      }
      }

  while (cntdy < image->nrow)
    {
      /* Nbre de lignes imprimees par bloc, 
	 origine et echelle en y */
		
      if (cntdy == 0)
	{	if ((image->nrow - cntdy) < bdy)
		  bdy = image->nrow;
		bsy = PSsy * bdy / image->nrow;
		boy = PSory + PSsy * (1 - ((float) bdy / image->nrow));
	      }
      else
	{	PSorx = 0;
		boy = -1;
		PSsx = 1;
		bsy = 1;
		if ((image->nrow - cntdy) < bdy)
		  {	bsy *= (float) (image->nrow - cntdy) / bdy;
			boy *= (float) (image->nrow - cntdy) / bdy; 
			bdy = image->nrow - cntdy;
		      }
	      }
		
      /* Ecriture de l'en-tete du fichier PostScript */
      
      sprintf (hbuf, "%f %f translate\n", PSorx, boy);
      strcat (headbuf, hbuf);
      sprintf (hbuf, "%f %f scale\n", PSsx, bsy);
      strcat (headbuf, hbuf);
      sprintf (hbuf ,"%d %d %d ", image->ncol, bdy, 8);
      strcat (headbuf ,hbuf);
      sprintf (hbuf ,"[%d 0 0 -%d 0 %d] <", image->ncol, bdy, bdy);
      strcat (headbuf ,hbuf);
      
      PSfcmd = strlen (headbuf);
      write (PSfd, headbuf, PSfcmd);

      /* Lecture des pixels de l'image et conversion PostScript */

      pixoffs = (long) cntdy * image->ncol;
      pixcnt = 2 * bdy * image->ncol;
      for (j = 0; j < pixcnt; j+=2)
	{	
	  PSfbuf[j] = hval[image->gray[pixoffs]>>4];
	  PSfbuf[j+1]=hval[image->gray[pixoffs++]&15];
	}
      write (PSfd, PSfbuf, pixcnt);
      
      /* Fin commandes PostScript */	

      sprintf (headbuf, "> image\n");
      PSfcmd = strlen (headbuf);
      write (PSfd, headbuf, PSfcmd);
      sprintf (headbuf, "");
      cntdy += bdy;

    }
  /* Show the page and Restore original state */
  sprintf(headbuf,"showpage\norigstate restore\n");
  PSfcmd = strlen(headbuf);
  write(PSfd,headbuf,PSfcmd);
    
  fclose (fp);
  return (0);
}

 

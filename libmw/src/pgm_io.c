/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  pgm_io.c
   
  Vers. 1.1
  (C)1996-2002 Jacques Froment
  load/save functions for the PGM (portable graymap file) formats
  (gray levels being unsigned chars)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "definitions.h"
#include "error.h"

#include "cimage.h"
#include "mwio.h"

#include "pgm_io.h"

/*~~~~~~ Return the next integer item or a the appened comments lines ~~~~~*/

int _mw_pgm_get_next_item(FILE *fp, char *comment)
{
     int c,l,i;

     do
     {
	  c=getc(fp);
	  if (c==EOF) return(EOF);
	  if (c=='#') /* Read the comment line */
	  {
	       l=strlen(comment);
	       if (l < mw_cmtsize-1) comment[l++]= '/';
	       while ((c=getc(fp) != '\n')&&(c != EOF))
		    if (l < mw_cmtsize-1) comment[l++]=(char)c;
	       comment[l]='\0';
	  }
     }
     while ((c<'0')||(c>'9'));
  
     /* That's the beginning of the next integer */
     i = 0;
     do
     {
	  i = (i*10) + (c - '0');
	  c = getc(fp);
	  if (c==EOF) return(i);
     }
     while ((c>='0')&&(c<='9'));
     return(i);
}

/*~~~~~~ Load 8-bits PGM Ascii ~~~~~*/

Cimage _mw_cimage_load_pgma(char *file)
{
     FILE *fp;
     Cimage image;
     int ncol,nrow,l,c;
     int maxgl;
     char comment[mw_cmtsize];

     if (!(fp = fopen(file, "r")))
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",file);
	  fclose(fp);

	  return(NULL);
     }

     /* Check the header (P2) */
     if ((getc(fp)!='P') || (getc(fp)!='2'))
	  mwerror(INTERNAL,1,"[_mw_cimage_load_pgma] Error while reading file \"%s\": not a PGMA format !\n",file);

     comment[0]='\0';
     ncol=_mw_pgm_get_next_item(fp,comment);
     nrow=_mw_pgm_get_next_item(fp,comment);

     maxgl=_mw_pgm_get_next_item(fp,comment);
     if ((ncol<=0) || (nrow<=0) || (maxgl <=0)) 
     {
	  mwerror(ERROR,0,"Error while reading file \"%s\": bad PGMA format !\n",file);
	  fclose(fp);
	  return(NULL);
     }
     if (maxgl > 255)
     {
	  mwerror(ERROR,0,"Cannot load PGMA image with gray levels exceeding value 255\n");
	  fclose(fp);
	  return(NULL);
     }

     image = mw_change_cimage(NULL,nrow,ncol);
     if (image == NULL)
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",file);

     for (l=0;l<ncol*nrow;l++) 
     {
	  c=_mw_pgm_get_next_item(fp,comment);
	  if (c==EOF) 
	  {
	       mwerror(ERROR,0,"Error while reading PGMA image \"%s\": unexpected end of file !\n",file);
	       fclose(fp);
	       mw_delete_cimage(image);
	       return(NULL);
	  }
	  image->gray[l] = (unsigned char) c;
     }

     if (strlen(comment) > 0) strcpy(image->cmt,comment);
     _mw_flip_image((unsigned char *) image->gray,sizeof(char),ncol,nrow,FALSE);
     return(image);
}

/*~~~~~ Create 8-bits PGM Ascii ~~~~~*/

short _mw_cimage_create_pgma(char *file, Cimage image)
{
     FILE *fp;
     int l,ll;

     if (!(fp = fopen(file, "w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",file);
	  return(-1);
     }

     if ((image == NULL) || (image->gray == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_create_pgma] NULL cimage or cimage plane\n");
	  return(-2);
     }

     fprintf(fp,"P2\n");
     if ((strlen(image->cmt)>0)&&(image->cmt[0]!='?')) fprintf(fp,"# %s\n",image->cmt);
     fprintf(fp,"%d\n",image->ncol);
     fprintf(fp,"%d\n",image->nrow);
     fprintf(fp,"255\n");

     for (l=0,ll=0;l<image->ncol*image->nrow;l++) 
     {
	  fprintf(fp,"%3d ",image->gray[l]);
	  ll+=4;
	  if (ll > 60) { ll=0; fprintf(fp,"\n"); }
     }
     return(0);
}

/*~~~~~~ Load 8-bits PGM Raw Bits ~~~~~*/

Cimage _mw_cimage_load_pgmr(char *file)
{
     FILE *fp;
     Cimage image;
     int ncol,nrow;
     int maxgl;
     long size;
     char comment[mw_cmtsize];

     if (!(fp = fopen(file, "r")))
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",file);
	  fclose(fp);

	  return(NULL);
     }

     /* Check the header (P5) */
     if ((getc(fp)!='P') || (getc(fp)!='5'))
	  mwerror(INTERNAL,1,"[_mw_cimage_load_pgmr] Error while reading file \"%s\": not a PGMR format !\n",file);

     comment[0]='\0';
     ncol=_mw_pgm_get_next_item(fp,comment);
     nrow=_mw_pgm_get_next_item(fp,comment);

     maxgl=_mw_pgm_get_next_item(fp,comment);
     if ((ncol<=0) || (nrow<=0) || (maxgl <=0) || (maxgl > 255)) 
     {
	  mwerror(ERROR,0,"Error while reading file \"%s\": bad PGMR format !\n",file);
	  fclose(fp);
	  return(NULL);
     }

     image = mw_change_cimage(NULL,nrow,ncol);
     if (image == NULL)
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",file);

     size=ncol*nrow;
     /* FIXME: wrong types, dirty temporary fix */
     if (fread(image->gray,1,size,fp) != (unsigned int) size)
     {
	  mwerror(ERROR,0,"Error while reading PGMR image \"%s\": unexpected end of file !\n",file);
	  fclose(fp);
	  mw_delete_cimage(image);
	  return(NULL);
     }
    
     if (strlen(comment) > 0) strcpy(image->cmt,comment);
     _mw_flip_image((unsigned char *) image->gray,sizeof(char),ncol,nrow,FALSE);
     return(image);
}

/*~~~~~ Create 8-bits PGM Raw bits ~~~~~*/

short _mw_cimage_create_pgmr(char *file, Cimage image)
{
     FILE *fp;
     long size;

     if (!(fp = fopen(file, "w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",file);
	  return(-1);
     }

     if ((image == NULL) || (image->gray == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_create_pgmr] NULL cimage or cimage plane\n");
	  return(-2);
     }

     fprintf(fp,"P5\n");
     if ((strlen(image->cmt)>0)&&(image->cmt[0]!='?')) fprintf(fp,"# %s\n",image->cmt);
     fprintf(fp,"%d\n",image->ncol);
     fprintf(fp,"%d\n",image->nrow);
     fprintf(fp,"255\n");

     size = image->ncol * image->nrow;
     if ((size_t) size > fwrite(image->gray,1,size,fp))
     {
	 fprintf(stderr, "error while writing to disk");
	 exit(EXIT_FAILURE);
     }
     return(0);
}



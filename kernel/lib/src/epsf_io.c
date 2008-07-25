/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  epsf_io.c
   
  Vers. 0.0
  (C) 1999 Jacques Froment
  Parts of this code inspired from XV: Copyright 1989, 1994 by John Bradley.

  Input/Output functions for Encapsulated PostScript image file

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

/*~~~~~ Load EPSF ~~~~~*/

Cimage _mw_cimage_load_epsf(char *fname)
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
  
     mwerror(FATAL,0,"Sorry, EPSF file format for input not available ! (why don't you use Ghostscript ?)\n");

     /* TO DO */
     Height = Width = 0;
  
     image = mw_change_cimage(NULL,Height,Width);
     if (image == NULL)
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",fname);

     _mw_flip_image((unsigned char *) image->gray,sizeof(char),image->ncol,
		    image->nrow,FALSE);
  
     return(image);
}

/*~~~~~ Write EPSF ~~~~~*/

/* Write the binary image plane into file fp */

static void WriteBinaryPlane(FILE *fp, unsigned char *B, int dx, int dy)
{
     int err, i, j, lwidth;
     unsigned char outbyte, bitnum, bit;

     err = 0;
     for (i=0; i<dy && err != EOF; i++) 
     {
	  outbyte = bitnum = lwidth = 0;
	  for (j=0; j<dx && err != EOF; j++) 
	  {
	       bit = *B;
	       outbyte = (outbyte<<1) | ((bit)&0x01);
	       bitnum++;

	       if (bitnum==8) 
	       {
		    err = fprintf(fp,"%02x",outbyte);
		    lwidth+=2;
		    outbyte = bitnum = 0;
	       }

	       if (lwidth>=72 && j+1<dx) 
	       { fprintf(fp, "\n"); lwidth = 0; }
	       B++;
	  }

	  if (bitnum) 
	  {   /* few bits left over... */
	       for ( ; bitnum<8; bitnum++) outbyte <<= 1;
	       err = fprintf(fp,"%02x",outbyte);
	       lwidth+=2;
	  }
	  fprintf(fp, "\n");
     }
}


/*~~ Write function ~~*/

/* Paper size (Width,Height), in inches */
/* A4 */
#define PAPER_W 8.268 
#define PAPER_H 11.693

short _mw_cimage_create_epsf(char *fname, Cimage image)
{
     FILE *fp;
     int dx,dy;        /* Size of image, in pixels (= points) */
     float sx, sy; /* Size of image, in inches (72 dpi - dot per inch) */
     float tx,ty;      /* Top-left offset of image, in inch */
     int bx,by;        /* Bottom-left offset of image, in points (pixels) */
     unsigned char white;
     unsigned char *G; /* Binary image plane */
     int i,j,lwidth,err,len;

     if ((image == NULL) || (image->gray == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_create_epsf] NULL cimage or cimage plane\n");
	  return(-2);
     }

     dx = image->ncol;
     dy = image->nrow;
     /* Size of image, in inches (at 72 dpi - dot per inch) */
     sx= dx / 72.0;
     sy= dy / 72.0;  

     if (!(fp = fopen(fname, "w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",fname);
	  return(-1);
     }

     /* Header */
     fprintf(fp,"%%!PS-Adobe-2.0 EPSF-2.0\n");
     fprintf(fp,"%%%%Title: %s\n",fname);
     fprintf(fp,"%%%%Creator: MegaWave2 (EPSF format)\n");
  
     /* Write always in Portrait mode (just as a plain image format) */
     /* Top-left offset of image, in inches */
     tx = 0.0;
     ty = (PAPER_H - sy);

     /* Bottom-left offset of image, in points = in pixels (1in = 72pt) */
     bx = (int) (tx * 72.0 + 0.5);
     by = (int) ((PAPER_H - (ty + sy)) * 72.0 + 0.5);
     fprintf(fp,"%%%%BoundingBox: %d %d %d %d\n", bx, by, bx+dx, by+dy);

     fprintf(fp,"%%%%Pages: 1\n");
     fprintf(fp,"%%%%DocumentFonts:\n");
     fprintf(fp,"%%%%EndComments\n");
     fprintf(fp,"%%%%EndProlog\n\n");

     fprintf(fp,"%%%%Page: 1 1\n\n");
     fprintf(fp,"%% remember original state\n");
     fprintf(fp,"/origstate save def\n\n");
     fprintf(fp,"%% build a temporary dictionary\n");
     fprintf(fp,"20 dict begin\n\n");

     white=mw_isitbinary_cimage(image);
     if (white > 0) 
	  len = (dx+7)/8;
     else 
	  len=dx;
     fprintf(fp,"/pix %d string def\n\n",len);

     fprintf(fp,"/grays %d string def  %% space for gray scale line\n", dx);
     fprintf(fp,"/npixls 0 def\n");
     fprintf(fp,"/rgbindx 0 def\n\n");

     fprintf(fp,"%d %d translate\n\n",bx,by);
     fprintf(fp,"%d %d scale\n\n",image->ncol,dy);
  
     if (white > 0)
	  /* This is a binary image */
     {
	  fprintf(fp,"%% dimensions of data (binary image)\n");
	  fprintf(fp,"%d %d 1\n\n",dx,dy);
	  fprintf(fp,"%% mapping matrix\n");
	  fprintf(fp,"[%d 0 0 %d 0 %d]\n\n", dx, -dy, dy);
	  fprintf(fp,"{currentfile pix readhexstring pop}\n");
	  fprintf(fp,"image\n");
      
	  G=image->gray;
	  /* write the binary plane */
	  WriteBinaryPlane(fp, G, dx, dy);
     }
     else
	  /* Gray level image */
     {
	  fprintf(fp,"%d %d 8\t\t\t%% dimensions of data (8 bits gray levels)\n",
		  dx,dy);
	  fprintf(fp,"[%d 0 0 %d 0 %d]\t\t%% mapping matrix\n", dx, -dy, dy);
	  fprintf(fp,"{currentfile pix readhexstring pop}\n");
	  fprintf(fp,"image\n");
	  for (i=0, G=image->gray; i<dy && err != EOF; i++) 
	  {
	       lwidth = 0;
	       putc('\n',fp);
	       for (j=0; j<dx && err != EOF; j++) 
	       {
		    err = fprintf(fp,"%02x",*G);
		    lwidth+=2;
		    G++;
	       }
	       if (lwidth>70) { putc('\n',fp); lwidth = 0; }
	  }
     }
  
     fprintf(fp,"\n\nshowpage\n\n");
     fprintf(fp,"%% stop using temporary dictionary\n");
     fprintf(fp,"end\n\n");
     fprintf(fp,"%% restore original state\n");
     fprintf(fp,"origstate restore\n\n");
     fprintf(fp,"%%%%Trailer\n");

     fclose (fp);
     return (0);
}

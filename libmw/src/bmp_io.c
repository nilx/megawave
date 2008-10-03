/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   bmp_io.c
   
   Vers. 1.02
   Authors : Emmanuel Villeger and Jacques Froment
   Parts of this code inspired from XV: Copyright 1989, 1994 by John Bradley.

   Input/Output functions for the BMP file compatibility with MegaWave2

   Main changes :
   v1.02 (JF) : bug on size of scan line corrected, in case of 24-bits plane.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>

#include "libmw-defs.h"
#include "mw.h"

#include "cimage.h"
#include "ccimage.h"

#include "bmp_io.h"

/* some functions designed to read and write on file */

static unsigned int getshort(FILE * fp)
{
     int c,c1;
  
     if ((c  = getc(fp)) == EOF ||
	 (c1 = getc(fp)) == EOF)
	  return EOF;
  
     return ((unsigned int)c) + (((unsigned int)c1) << 8);
}

static unsigned int getint(FILE * fp)
{
     int c,c1,c2,c3;
  
     if ((c  = getc(fp)) == EOF ||
	 (c1 = getc(fp)) == EOF ||
	 (c2 = getc(fp)) == EOF ||
	 (c3 = getc(fp)) == EOF)
	  return EOF;
  
     return ((unsigned int)c) +
	  (((unsigned int)c1) << 8) + 
	  (((unsigned int)c2) << 16) +
	  (((unsigned int)c3) << 24);
}

static int putshort(int i, FILE * fp)
{
     int c,c1;
  
     c  =  ((unsigned int)i)     & 0xff;
     c1 = (((unsigned int)i)>>8) & 0xff;
  
     if (putc(c,fp) == EOF ||
	 putc(c1,fp) == EOF)
	  return EOF;
  
     return i;
}

static int putint(int i, FILE * fp)
{
     int c,c1,c2,c3;
  
     c  =  ((unsigned int)i)      & 0xff;  
     c1 = (((unsigned int)i)>>8)  & 0xff;
     c2 = (((unsigned int)i)>>16) & 0xff;
     c3 = (((unsigned int)i)>>24) & 0xff;

     if (putc(c, fp) == EOF ||
	 putc(c1,fp) == EOF ||
	 putc(c2,fp) == EOF ||
	 putc(c3,fp) == EOF)
	  return EOF;
  
     return i;
}

/*~~~~~ Read BMP header ~~~~~*/

/* Fill the corresponding variable and return the file pointer */

FILE * _mw_read_bmp_header(char * fname, 
			   unsigned int * nx, unsigned int * ny,
			   unsigned int * offset, unsigned int * size,
			   unsigned int * planes,
			   unsigned int * bitcount,
			   unsigned int * compression)
{
     int    c,c1;      /* character read                 */
     FILE *fp;

     if (!(fp = fopen(fname,"r"))) 
     {
	  mwerror(ERROR, 0,"BMP image file \"%s\" not found or unreadable !\n",fname);
	  return(NULL);
     }
  
     c  = getc(fp); /* bfType */
     c1 = getc(fp); /* bfType */
     if (c!='B' || c1!='M') 
     {
	  mwerror(ERROR, 0,"File \"%s\" not in BMP format !\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     *compression=0;
     /* FIXME: wrong types, dirty temporary fix */
     if (
	  getint(fp) == (unsigned int) EOF ||            /* bfSize      */
	  getshort(fp) == (unsigned int) EOF ||          /* bfReserved1 */
	  getshort(fp) == (unsigned int) EOF ||          /* bfReserved2 */
	  (*offset = getint(fp)) == (unsigned int) EOF || /* bfOffBits   */
	  (*size=getint(fp)) == (unsigned int) EOF        /* size   */
	  )
     {
	  mwerror(ERROR, 0,"Error while reading header of file \"%s\"... Not a BMP format or file corrupted !\n",fname);
	  fclose(fp);
	  return(NULL);
     }  

     if ((*size == 40 || *size == 64)) 
	  /* New BMP format with extended header */
     {
	  /* FIXME: wrong types, dirty temporary fix */
	  if (
	       (*nx = getint(fp)) == (unsigned int) EOF || /* biWidth  */
	       (*ny = getint(fp)) == (unsigned int) EOF ||   /* biHeight */
	       (*planes = getshort(fp))== (unsigned int)EOF ||
	       (*bitcount = getshort(fp))== (unsigned int)EOF ||    
	       (*compression=getint(fp))== (unsigned int)EOF /* New format may include compression */
	       )
	  {
	       mwerror(ERROR, 0,"Error while reading header of file \"%s\"... Not a new BMP format or file corrupted !\n",fname);
	       fclose(fp);
	       return(NULL);
	  }  

     }
     else
	  /* Old BMP format */
     {
	  mwerror(ERROR, 0,"Error while reading header of file \"%s\"... Unsupported old BMP format !\n",fname);
	  fclose(fp);
	  return(NULL);
     }
  
     return(fp);
}

/*~~~~~ Load BMP CHAR ~~~~~*/

Cimage _mw_cimage_load_bmp(char * file)
{
     Cimage image;     /* image to be loaded             */
     int    i,j;       /* indexes for loops              */
     int    allocsize; /* size of scan lines             */
     unsigned int   nx,ny;     /* image size                     */
     unsigned int   offset;    /* offset to the begining of data */
     FILE   *fp;       /* file to read                   */
     unsigned int size, planes, bitcount, compression;
     int c;

     fp=_mw_read_bmp_header(file,&nx,&ny,&offset,&size,&planes,&bitcount,&compression);
     if (!fp) return(NULL);

     if (planes!=1)
	  mwerror(INTERNAL, 1,"BMP image file \"%s\" : unexpected number of planes = %d\n",file,planes);
     if (bitcount!=8)
	  mwerror(INTERNAL, 1,"BMP image file \"%s\" : unexpected number of bits = %d\n",file,bitcount);
     if (compression!=0)
     {
	  mwerror(ERROR, 1,"BMP image file \"%s\" : cannot handle compression !\n",file);    
	  fclose(fp);
	  return(NULL);
     }

     allocsize = nx + ((nx&3) ? 4 - (nx&3) : 0);

     if (fseek(fp,offset,SEEK_SET)) 
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a BMP format or file corrupted !\n",file);
	  fclose(fp);
	  return(NULL);
     }

     if (!(image = mw_change_cimage(NULL,ny,nx)))
	  mwerror(FATAL,0,"Not enough memory to load BMP image \"%s\"\n",file);

     for (j = ny-1; j>=0; j--) 
     {
	  /* FIXME: wrong types, dirty temporary fix */
	  for (i = 0; i < (int) nx; i++)
	  {
	       if ((c = getc(fp)) == EOF) 
	       {
		    mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a BMP format or file corrupted !\n",file);
		    mw_delete_cimage(image);
		    image = NULL;
		    fclose(fp);
		    return(NULL);
	       }
	       else image->gray[i + j*nx] = c;
	  }
	  for (; i < allocsize; i++)
	       if (getc(fp) == EOF) 
	       {
		    mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a BMP format or file corrupted !\n",file);
		    mw_delete_cimage(image);
		    image = NULL;
		    fclose(fp);
		    return(NULL);
	       }
     }
  
     fclose(fp);
     return(image);
}

/*~~~~~ Create BMP CHAR ~~~~~*/

short _mw_cimage_create_bmp(char * file, Cimage image)
{
     int i,j;       /* indexes for loops  */
     int allocsize; /* size of scan lines */
     int nx,ny;     /* image size         */
     FILE *fp;      /* file for save      */
  
     if ((image == NULL) || (image->gray == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_create_pm] NULL cimage or cimage plane\n");
	  return(-2);
     }
     nx = image->ncol;
     ny = image->nrow;
  
     allocsize = nx + ((nx&3) ? 4-(nx&3) : 0);
  
     if (!(fp = fopen(file,"w")))
     {
	  mwerror(ERROR, 0,"Cannot create BMP file \"%s\"\n",file);
	  return(-1);
     }
  
     if (
	  /* BITMAPFILEHEADER */
    
	  putc('B',fp) == EOF ||                       /* bfType      */
	  putc('M',fp) == EOF ||                       /* bfType      */
	  putint(14+40+1024+ny*allocsize,fp) == EOF || /* bfSize      */
	  putshort(0,fp) == EOF ||                     /* bfReserved1 */
	  putshort(0,fp) == EOF ||                     /* bfReserved2 */
	  putint(14+40+1024,fp) == EOF ||              /* bfOffBits   */
    
	  /* BITMAPINFOHEADER */
    
	  putint(40,fp) == EOF ||           /* size : new WINDOWS format  */
	  putint(nx,fp) == EOF ||           /* biWidth         */
	  putint(ny,fp) == EOF ||           /* biHeight        */
	  putshort(1,fp) == EOF ||          /* biPlanes        */
	  putshort(8,fp) == EOF ||          /* biBitCount      */
	  putint(0,fp) == EOF ||            /* biCompression   */
	  putint(ny*allocsize,fp) == EOF || /* biSizeImage     */
	  putint(75 * 39,fp) == EOF ||      /* biXPelsPerMeter */
	  putint(75 * 39,fp) == EOF ||      /* biYPelsPerMeter */
	  putint(256,fp) == EOF ||          /* biClrUsed       */
	  putint(256,fp) == EOF             /* biClrImportant  */
	  ) {
	  mwerror(ERROR, 0,"Error while writing BMP file \"%s\" !\n",file);
	  fclose(fp);
	  return(-1);
     }
     /* RGBQUAD */
  
     for (i = 0; i < 256; i++) {
	  if (
	       putc(i,fp) == EOF || /* rgbBlue     */
	       putc(i,fp) == EOF || /* rgbGreen    */
	       putc(i,fp) == EOF || /* rgbRed      */
	       putc(0,fp) == EOF    /* rgbReserved */
	       ) {
	       mwerror(ERROR, 0,"Cannot write color map of BMP file \"%s\".\n",
		       file);
	       fclose(fp);
	       return(-2);
	  }
     }
  
     /* write the bitmap */
  
     for (j = ny-1; j >= 0 ; j--) {
	  for (i = 0; i < nx; i++)
	       if (putc(image->gray[i + j*nx],fp) == EOF) {
		    mwerror(ERROR,0,"Error while writing BMP file \"%s\" !\n",file);
		    fclose(fp);
		    return(-1);
	       }
	  for (; i < allocsize; i++)
	       if (putc(0,fp) == EOF) {
		    mwerror(ERROR,0,"Error while writing BMP file \"%s\" !\n",file);
		    fclose(fp);
		    return(-1);
	       }
     }
  
     /* close the file and return */
     fclose(fp);
     return(0);
}

/*~~~~~ Load 24-bit color BMP CHAR ~~~~~*/


Ccimage _mw_ccimage_load_bmp(char * file)
{
     Ccimage image;     /* image to be loaded             */
     int     i,j;       /* indexes for loops              */
     int     allocsize; /* size of scan lines             */
     unsigned int     nx,ny;     /* image size                     */
     unsigned int    offset;    /* offset to the begining of data */
     int     r,g,b;     /* character read                 */
     FILE    *fp;       /* file to read                   */
     unsigned int size, planes, bitcount, compression;

     fp=_mw_read_bmp_header(file,&nx,&ny,&offset,&size,&planes,&bitcount,&compression);
     if (!fp) return(NULL);

     if (planes!=1)
	  mwerror(INTERNAL, 1,"BMP image file \"%s\" : unexpected number of planes = %d\n",file,planes);
     if (bitcount!=24)
	  mwerror(INTERNAL, 1,"BMP image file \"%s\" : unexpected number of bits = %d\n",file,bitcount);
     if (compression!=0)
     {
	  mwerror(ERROR, 1,"BMP image file \"%s\" : cannot handle compression !\n",file);    
	  fclose(fp);
	  return(NULL);
     }
  
     if (!(image = mw_change_ccimage(NULL,ny,nx)))
	  mwerror(FATAL,0,"Not enough memory to load BMP image \"%s\"\n",file);
  
     /*  allocsize = nx + ((nx&3) ? 4 - (nx&3) : 0);*/
     allocsize=nx + ((4 - ((nx*3) % 4)) & 0x03);
  
     if (fseek(fp,offset,SEEK_SET)) {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a BMP format or file corrupted !\n",file);
	  mw_delete_ccimage(image);
	  image = NULL;
	  fclose(fp);
	  return(NULL);
     }
  
     for (j = ny-1; j>=0; j--) {
	  /* FIXME: wrong types, dirty temporary fix */
	  for (i = 0; i < (int) nx; i++)
	       if ((b = getc(fp)) == EOF ||
		   (g = getc(fp)) == EOF ||
		   (r = getc(fp)) == EOF) {
		    mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a BMP format or file corrupted !\n",file);
		    mw_delete_ccimage(image);
		    image = NULL;
		    fclose(fp);
		    return(NULL);
	       }
	       else {
		    image->red[i + j*nx]   = r;
		    image->green[i + j*nx] = g;
		    image->blue[i + j*nx]  = b;
	       }
	  for (; i < allocsize; i++)
	       if (getc(fp) == EOF) {
		    mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a BMP format or file corrupted !\n",file);
		    mw_delete_ccimage(image);
		    image = NULL;
		    fclose(fp);
		    return(NULL);
	       }
     }
  
     fclose(fp);
     return(image);
}

/*~~~~~ Create 24-bit color BMP CHAR ~~~~~*/

short _mw_ccimage_create_bmp(char * file, Ccimage image)
{
     int i,j;       /* indexes for loops  */
     int allocsize; /* size of scan lines */
     int nx,ny;     /* image size         */
     FILE *fp;      /* file for save      */
  
     if ((image == NULL)
	 || (image->red == NULL)
	 || (image->green == NULL)
	 || (image->blue == NULL))
	  mwerror(INTERNAL, 0,"[_mw_ccimage_create_bmp] NULL cimage or cimage plane\n");
     nx = image->ncol;
     ny = image->nrow;
     /*allocsize = nx + ((nx&3) ? 4-(nx&3) : 0);*/
     allocsize=nx + ((4 - ((nx*3) % 4)) & 0x03);
  
     if (!(fp = fopen(file,"w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",file);
	  return(-1);
     }
  
     if (
	  /* BITMAPFILEHEADER */
      
	  putc('B',fp) == EOF ||                  /* bfType      */
	  putc('M',fp) == EOF ||                  /* bfType      */
	  putint(14+40+ny*allocsize,fp) == EOF || /* bfSize      */
	  putshort(0,fp) == EOF ||                /* bfReserved1 */
	  putshort(0,fp) == EOF ||                /* bfReserved2 */
	  putint(14+40,fp) == EOF ||              /* bfOffBits   */
      
	  /* BITMAPINFOHEADER */
      
	  putint(40,fp) == EOF ||           /* size : new WINDOWS format  */
	  putint(nx,fp) == EOF ||           /* biWidth         */
	  putint(ny,fp) == EOF ||           /* biHeight        */
	  putshort(1,fp) == EOF ||          /* biPlanes        */
	  putshort(24,fp) == EOF ||         /* biBitCount      */
	  putint(0,fp) == EOF ||            /* biCompression   */
	  putint(ny*allocsize,fp) == EOF || /* biSizeImage     */
	  putint(75 * 39,fp) == EOF ||      /* biXPelsPerMeter */
	  putint(75 * 39,fp) == EOF ||      /* biYPelsPerMeter */
	  putint(0,fp) == EOF ||            /* biClrUsed       */
	  putint(0,fp) == EOF               /* biClrImportant  */
	  ) {
	  mwerror(ERROR, 0,"Error while writing file \"%s\" !\n",file);
	  fclose(fp);
	  return(-1);
     }
  
     for (j = ny-1; j >= 0 ; j--) {
	  for (i = 0; i < nx; i++) {
	       if (putc(image->blue[i + j*nx],fp)  == EOF ||
		   putc(image->green[i + j*nx],fp) == EOF ||
		   putc(image->red[i + j*nx],fp)   == EOF) {
		    mwerror(ERROR, 0,"Error while writing file \"%s\" !\n",file);
		    fclose(fp);
		    return(-1);
	       }
	  }
	  for (; i < allocsize; i++)
	       if (putc(0,fp) == EOF) {
		    mwerror(ERROR, 0,"Error while writing file \"%s\" !\n",file);
		    fclose(fp);
		    return(-1);
	       }
     }
  
     fclose(fp);
  
     return(0);
}

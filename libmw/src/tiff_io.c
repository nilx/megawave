/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  tiff_io.c
   
  Vers. 1.4
  Author : Jacques Froment
  Parts of this code inspired from XV: Copyright 1989, 1994 by John Bradley.

  Input/Output functions for the TIFF file compatibility with MegaWave2
  Need the 'libtiff' portions of Sam's TIFF distribution (ftp sgi.com).

  Main changes :
  v1.4 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)

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

#include "libmw-defs.h"
#include "error.h"

#include "mwio.h"
#include "cimage.h"
#include "ccimage.h"
#include "basic_conv.h"
/* FIXME : use standard headers */
#include "tiffio.h"

#include "tiff_io.h"

typedef int boolean;
typedef unsigned char byte;

/* Change u_short, u_char and u_int to u__short, u__char and 
   u__int to avoid conflict with same definition in include files.
*/
typedef unsigned short u__short;
typedef unsigned char u__char;
typedef unsigned int u__int;


#undef PARM
#define PARM(a) a

static int   loadImage   PARM((TIFF *, uint32, uint32, byte *, int));
static void  _TIFFerr    PARM((const char *, const char *, va_list));
static void  _TIFFwarn   PARM((const char *, const char *, va_list));

static char *filename;

static int   error_occurred;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ LOAD TIFF */

/*~~~~~ Load 24-bits or 8-bits color TIFF CHAR ~~~~~*/

Ccimage _mw_ccimage_load_tiff(char * fname)
{
     TIFF  *tif;
     uint32 w, h;
     short	 bps, spp, photo, orient;
     byte  *pic24;
     char  *desc;
     Ccimage image;
     char Comment[BUFSIZ];

     error_occurred = 0;
     Comment[0] = '\0';

     TIFFSetErrorHandler(_TIFFerr);
     TIFFSetWarningHandler(_TIFFwarn);

     tif=TIFFOpen(fname,"r");
     if (!tif) 
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  return(NULL);
     }

     /* flip orientation so that image comes in X order */
     TIFFGetFieldDefaulted(tif, TIFFTAG_ORIENTATION, &orient);
     switch (orient) {
     case ORIENTATION_TOPLEFT:
     case ORIENTATION_TOPRIGHT:
     case ORIENTATION_LEFTTOP:
     case ORIENTATION_RIGHTTOP:   orient = ORIENTATION_BOTLEFT;   break;

     case ORIENTATION_BOTRIGHT:
     case ORIENTATION_BOTLEFT:
     case ORIENTATION_RIGHTBOT:
     case ORIENTATION_LEFTBOT:    orient = ORIENTATION_TOPLEFT;   break;
     }

     TIFFSetField(tif, TIFFTAG_ORIENTATION, orient);

     TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
     TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
     TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bps);
     TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo);
     TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);

     if (spp <= 1) /* Nbre de plans */
     {
	  mwerror(INTERNAL, 0,"[_mw_ccimage_load_tiff] TIFF file \"%s\" is not a 24-bits color image\n",fname);
	  return(NULL);
     }

     /* allocate 24-bit image */
     pic24 = (byte *) malloc((size_t) w*h*3);
     if (!pic24) 
     {
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",fname);
	  return(NULL);
     }
     loadImage(tif, w, h, pic24, 0);

     /* try to get comments, if any */
     TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &desc);
     if (desc && strlen(desc) > (size_t) 0) 
     {
	  /* kludge:  tiff library seems to return bizarre comments */
	  if (strlen(desc)==4 && strcmp(desc, "\367\377\353\370")==0) {} 
	  else strcpy(Comment, desc);
     }
    
     TIFFClose(tif);

     if (error_occurred) 
     {
	  if (pic24) free(pic24);
	  return(NULL);
     }

     image = mw_change_ccimage(NULL,h,w);
     if (image == NULL)
     {
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",fname);
	  free(pic24);
	  return(NULL);
     }

     if (Comment[0] != '\0') strncpy(image->cmt,Comment,mw_cmtsize);
     _mw_1x24XV_to_3x8_ucharplanes(pic24,image->red,image->green,image->blue,w*h*3);

     _mw_flip_image((unsigned char *) image->red,sizeof(char),
		    image->ncol,image->nrow,FALSE);
     _mw_flip_image((unsigned char *) image->green,sizeof(char),
		    image->ncol,image->nrow,FALSE);
     _mw_flip_image((unsigned char *) image->blue,sizeof(char),
		    image->ncol,image->nrow,FALSE);
     free(pic24);

     return(image);
}  

/*~~~~~ Load 8-bit grey scale TIFF CHAR ~~~~~*/

Cimage _mw_cimage_load_tiff(char * fname)
{
     TIFF  *tif;
     uint32 w, h;
     short	 bps, spp, photo, orient;
     byte  *pic8;
     char  *desc;
     Cimage image;
     char Comment[BUFSIZ];

     error_occurred = 0;
     Comment[0] = '\0';

     TIFFSetErrorHandler(_TIFFerr);
     TIFFSetWarningHandler(_TIFFwarn);

     tif=TIFFOpen(fname,"r");
     if (!tif) 
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  return(NULL);
     }

     /* flip orientation so that image comes in X order */
     TIFFGetFieldDefaulted(tif, TIFFTAG_ORIENTATION, &orient);
     switch (orient) {
     case ORIENTATION_TOPLEFT:
     case ORIENTATION_TOPRIGHT:
     case ORIENTATION_LEFTTOP:
     case ORIENTATION_RIGHTTOP:   orient = ORIENTATION_BOTLEFT;   break;

     case ORIENTATION_BOTRIGHT:
     case ORIENTATION_BOTLEFT:
     case ORIENTATION_RIGHTBOT:
     case ORIENTATION_LEFTBOT:    orient = ORIENTATION_TOPLEFT;   break;
     }

     TIFFSetField(tif, TIFFTAG_ORIENTATION, orient);

     TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
     TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
     TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bps);
     TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo);
     TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);

     if (spp != 1)
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_load_tiff] TIFF file \"%s\" is not a 8-bits image\n",fname);
	  return(NULL);
     }

     if (photo == PHOTOMETRIC_PALETTE)
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_load_tiff] TIFF file \"%s\" is a 8-bits color image\n",fname);
	  return(NULL);
     }

     /* allocate 8-bit image */
     pic8 = (byte *) malloc((size_t) w*h);
     if (!pic8) 
     {
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",fname);
	  return(NULL);
     }
     loadImage(tif, w, h, pic8, 0);

     /* try to get comments, if any */
     TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &desc);
     if (desc && strlen(desc) > (size_t) 0) 
     {
	  /* kludge:  tiff library seems to return bizarre comments */
	  if (strlen(desc)==4 && strcmp(desc, "\367\377\353\370")==0) {} 
	  else strcpy(Comment, desc);
     }
    
     TIFFClose(tif);

     if (error_occurred) 
     {
	  if (pic8) free(pic8);
	  return(NULL);
     }

     image = mw_change_cimage(NULL,h,w);
     if (image == NULL)
     {
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",fname);
	  free(pic8);
	  return(NULL);
     }

     if (Comment[0] != '\0') strncpy(image->cmt,Comment,mw_cmtsize);
     memcpy(image->gray,pic8,w*h); 
     _mw_flip_image((unsigned char *) image->gray,sizeof(char),
		    image->ncol,image->nrow,FALSE);
  
     free(pic8);
     return(image);
}  


/* Error sent by libtiff */

static void _TIFFerr(const char * module, const char * fmt, va_list ap)
{
     char buf[2048],err[2048];
     char *cp = buf;

     if (module != NULL) {
	  sprintf(cp, "%s: ", module);
	  cp = strchr(cp, '\0');
     }

     vsprintf(cp, fmt, ap);
     strcat(cp, ".");

     sprintf(err,"[libtiff] %s\n",buf);
     mwerror(ERROR,0,err);
     error_occurred = 1;
}


/* Warning error sent by libtiff */
static void _TIFFwarn(const char * module, const char * fmt, va_list ap)
{
     char buf[2048],err[2048];
     char *cp = buf;

     if (module != NULL) {
	  sprintf(cp, "%s: ", module);
	  cp = strchr(cp, '\0');
     }
     cp = strchr(cp, '\0');
     vsprintf(cp, fmt, ap);
     strcat(cp, ".");

     sprintf(err,"[libtiff] %s\n",buf);
     mwerror(WARNING,0,err);
}

typedef	byte RGBvalue;

static	u__short bitspersample;
static	u__short samplesperpixel;
static	u__short photometric;
static	u__short orientation;

/* colormap for pallete images */
static	u__short *redcmap, *greencmap, *bluecmap;
static	int      stoponerr;			/* stop on read error */

/* YCbCr support */
static	u__short YCbCrHorizSampling;
static	u__short YCbCrVertSampling;
static	float   *YCbCrCoeffs;
static	float   *refBlackWhite;

static	byte **BWmap;
static	byte **PALmap;

typedef void (*tileContigRoutine)   PARM((byte*, u__char*, RGBvalue*, 
					  uint32, uint32, int, int));

typedef void (*tileSeparateRoutine) PARM((byte*, u__char*, u__char*, u__char*, 
					  RGBvalue*, uint32, uint32, int, int));


static int    gt                       PARM((TIFF *, uint32, uint32, byte *));
static uint32 setorientation           PARM((TIFF *, uint32));
static int    gtTileContig             PARM((TIFF *, byte *, RGBvalue *, 
					     uint32, uint32, int));
static int    gtTileSeparate           PARM((TIFF *, byte *, RGBvalue *, 
					     uint32, uint32, int));
static int    gtStripContig            PARM((TIFF *, byte *, RGBvalue *, 
					     uint32, uint32, int));
static int    gtStripSeparate          PARM((TIFF *, byte *, RGBvalue *, 
					     uint32, uint32, int));

static int    makebwmap                PARM((void));
static int    makecmap                 PARM((void));

static void   put8bitcmaptile          PARM((byte *, u__char *, RGBvalue *,
					     uint32, uint32, int, int));
static void   put4bitcmaptile          PARM((byte *, u__char *, RGBvalue *,
					     uint32, uint32, int, int));
static void   put2bitcmaptile          PARM((byte *, u__char *, RGBvalue *,
					     uint32, uint32, int, int));
static void   put1bitcmaptile          PARM((byte *, u__char *, RGBvalue *,
					     uint32, uint32, int, int));
static void   putgreytile              PARM((byte *, u__char *, RGBvalue *,
					     uint32, uint32, int, int));
static void   put1bitbwtile            PARM((byte *, u__char *, RGBvalue *,
					     uint32, uint32, int, int));
static void   put2bitbwtile            PARM((byte *, u__char *, RGBvalue *,
					     uint32, uint32, int, int));
static void   put4bitbwtile            PARM((byte *, u__char *, RGBvalue *,
					     uint32, uint32, int, int));
static void   put16bitbwtile           PARM((byte *, u__char *, RGBvalue *,
					     uint32, uint32, int, int));

static void   putRGBcontig8bittile     PARM((byte *, u__char *, RGBvalue *,
					     uint32, uint32, int, int));

static void   putRGBcontig16bittile    PARM((byte *, u__short *, RGBvalue *,
					     uint32, uint32, int, int));

static void   putRGBseparate8bittile   PARM((byte *, u__char *, u__char *, 
					     u__char *, RGBvalue *, 
					     uint32, uint32, int, int));

static void   putRGBseparate16bittile  PARM((byte *, u__short *, u__short *, 
					     u__short *, RGBvalue *, 
					     uint32, uint32, int, int));


static void   initYCbCrConversion     PARM((void));

static void   putRGBContigYCbCrClump  PARM((byte *, u__char *, int, int, 
					    uint32, int, int, int));

static void   putcontig8bitYCbCrtile  PARM((byte *, u__char *, RGBvalue *,
					    uint32, uint32, int, int));

static tileContigRoutine   pickTileContigCase   PARM((RGBvalue *));
static tileSeparateRoutine pickTileSeparateCase PARM((RGBvalue *));


/*******************************************/
static int loadImage(TIFF * tif, uint32 rwidth, uint32 rheight,
		     byte * raster, int stop)
{
     int ok;
     uint32 width, height;

     TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
     switch (bitspersample) {
     case 1: 
     case 2: 
     case 4:
     case 8: 
     case 16:  break;

     default:
	  TIFFError(TIFFFileName(tif),
		    "Sorry, can not handle %d-bit pictures", bitspersample);
	  return (0);
     }


     TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
     switch (samplesperpixel) {
     case 1: 
     case 3: 
     case 4:  break;

     default:
	  TIFFError(TIFFFileName(tif),
		    "Sorry, can not handle %d-channel images", samplesperpixel);
	  return (0);
     }


     if (!TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric)) {
	  switch (samplesperpixel) {
	  case 1:  photometric = PHOTOMETRIC_MINISBLACK;   break;

	  case 3:
	  case 4:  photometric = PHOTOMETRIC_RGB;          break;

	  default:
	       TIFFError(TIFFFileName(tif),
			 "Missing needed \"PhotometricInterpretation\" tag");
	       return (0);
	  }

	  TIFFWarning(TIFFFileName(tif),
		      "No \"PhotometricInterpretation\" tag, assuming %s\n",
		      photometric == PHOTOMETRIC_RGB ? "RGB" : "min-is-black");
     }

     /* XXX maybe should check photometric? */
     TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
     TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

     /* XXX verify rwidth and rheight against width and height */
     stoponerr = stop;
     BWmap = NULL;
     PALmap = NULL;
     ok = gt(tif, rwidth, height, raster + (rheight-height)*rwidth);
     if (BWmap)
	  free((char *)BWmap);
     if (PALmap)
	  free((char *)PALmap);
     return (ok);
}


/*******************************************/
static int gt(TIFF * tif, uint32 w, uint32 h, byte * raster)
{
     u__short minsamplevalue, maxsamplevalue, planarconfig;
     RGBvalue *Map;
     int bpp = 1, e;
     int x, range;

     TIFFGetFieldDefaulted(tif, TIFFTAG_MINSAMPLEVALUE, &minsamplevalue);
     TIFFGetFieldDefaulted(tif, TIFFTAG_MAXSAMPLEVALUE, &maxsamplevalue);
     Map = NULL;
  
     switch (photometric) {
     case PHOTOMETRIC_YCBCR:
	  TIFFGetFieldDefaulted(tif, TIFFTAG_YCBCRCOEFFICIENTS,
				&YCbCrCoeffs);
	  TIFFGetFieldDefaulted(tif, TIFFTAG_YCBCRSUBSAMPLING,
				&YCbCrHorizSampling, &YCbCrVertSampling);
	  TIFFGetFieldDefaulted(tif, TIFFTAG_REFERENCEBLACKWHITE,
				&refBlackWhite);
	  initYCbCrConversion();
	  /* fall thru... */
	
     case PHOTOMETRIC_RGB:
	  bpp *= 3;
	  if (minsamplevalue == 0 && maxsamplevalue == 255)
	       break;
	
	  /* fall thru... */
     case PHOTOMETRIC_MINISBLACK:
     case PHOTOMETRIC_MINISWHITE:
	  range = maxsamplevalue - minsamplevalue;
	  Map = (RGBvalue *)malloc((range + 1) * sizeof (RGBvalue));
	  if (Map == NULL) {
	       TIFFError(filename,
			 "No space for photometric conversion table");
	       return (0);
	  }

	  if (photometric == PHOTOMETRIC_MINISWHITE) {
	       for (x = 0; x <= range; x++)
		    Map[x] = (255*(range-x))/range;
	  } else {
	       for (x = 0; x <= range; x++)
		    Map[x] = (255*x)/range;
	  }

	  if (photometric != PHOTOMETRIC_RGB && bitspersample <= 8) {
	       /*
		* Use photometric mapping table to construct
		* unpacking tables for samples <= 8 bits.
		*/
	       if (!makebwmap())
		    return (0);
	       /* no longer need Map, free it */
	       free((char *)Map);
	       Map = NULL;
	  }
	  break;

     case PHOTOMETRIC_PALETTE:
	  if (!TIFFGetField(tif, TIFFTAG_COLORMAP,
			    &redcmap, &greencmap, &bluecmap)) {
	       TIFFError(filename,
			 "Missing required \"Colormap\" tag");
	       return (0);
	  }

	  if (bitspersample <= 8) {
	       /*
		* Use mapping table to construct
		* unpacking tables for samples < 8 bits.
		*/
	       if (!makecmap())
		    return (0);
	  }
	  break;

     default:
	  TIFFError(filename, "Unknown photometric tag %u", photometric);
	  return (0);
     }

     TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarconfig);
     if (planarconfig == PLANARCONFIG_SEPARATE && samplesperpixel > 1) {
	  e = TIFFIsTiled(tif) ? gtTileSeparate (tif, raster, Map, h, w, bpp) :
	       gtStripSeparate(tif, raster, Map, h, w, bpp);
     } else {
	  e = TIFFIsTiled(tif) ? gtTileContig (tif, raster, Map, h, w, bpp) :
	       gtStripContig(tif, raster, Map, h, w, bpp);
     }

     if (Map) free((char *)Map);
     return (e);
}


/*******************************************/
static uint32 setorientation(TIFF * tif, uint32 h)
{
     uint32 y;

     TIFFGetFieldDefaulted(tif, TIFFTAG_ORIENTATION, &orientation);
     switch (orientation) {
     case ORIENTATION_BOTRIGHT:
     case ORIENTATION_RIGHTBOT:	/* XXX */
     case ORIENTATION_LEFTBOT:	/* XXX */
	  TIFFWarning(filename, "using bottom-left orientation");
	  orientation = ORIENTATION_BOTLEFT;

	  /* fall thru... */
     case ORIENTATION_BOTLEFT:
	  y = 0;
	  break;

     case ORIENTATION_TOPRIGHT:
     case ORIENTATION_RIGHTTOP:	/* XXX */
     case ORIENTATION_LEFTTOP:	/* XXX */
     default:
	  TIFFWarning(filename, "using top-left orientation");
	  orientation = ORIENTATION_TOPLEFT;
	  /* fall thru... */
     case ORIENTATION_TOPLEFT:
	  y = h-1;
	  break;
     }
     return (y);
}




/*
 * Get an tile-organized image that has
 *	PlanarConfiguration contiguous if SamplesPerPixel > 1
 * or
 *	SamplesPerPixel == 1
 */	
/*******************************************/
static int gtTileContig(TIFF * tif, byte * raster, RGBvalue * Map,
			uint32 h, uint32 w, int bpp)
{
     uint32 col, row, y;
     uint32 tw, th;
     u__char *buf;
     int fromskew, toskew;
     u__int nrow;
     tileContigRoutine put;

     put = pickTileContigCase(Map);
     if (put == 0) return (0);

     buf = (u__char *) malloc((size_t) TIFFTileSize(tif));
     if (buf == 0) {
	  TIFFError(filename, "No space for tile buffer");
	  return (0);
     }

     TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tw);
     TIFFGetField(tif, TIFFTAG_TILELENGTH, &th);
     y = setorientation(tif, h);
     toskew = (orientation == ORIENTATION_TOPLEFT ? -tw + -w : -tw + w);

     for (row = 0; row < h; row += th) {
	  nrow = (row + th > h ? h - row : th);
	  for (col = 0; col < w; col += tw) {
	       if (TIFFReadTile(tif,buf,(uint32)col, (uint32)row, 0, 0) < 0
		   && stoponerr) break;

	       if (col + tw > w) {
		    /*
		     * Tile is clipped horizontally.  Calculate
		     * visible portion and skewing factors.
		     */
		    uint32 npix = w - col;
		    fromskew = tw - npix;
		    (*put)(raster + (y*w + col)*bpp, buf, Map,   npix, (uint32) nrow,
			   fromskew, (int) ((toskew + fromskew)*bpp));
	       } else
		    (*put)(raster + (y*w + col)*bpp, buf, Map,   tw,   (uint32) nrow,
			   0, (int) (toskew*bpp));
	  }

	  y += (orientation == ORIENTATION_TOPLEFT ? -nrow : nrow);
     }
     free(buf);
     return (1);
}




/*
 * Get an tile-organized image that has
 *	 SamplesPerPixel > 1
 *	 PlanarConfiguration separated
 * We assume that all such images are RGB.
 */	

/*******************************************/
static int gtTileSeparate(TIFF * tif, byte * raster, RGBvalue * Map,
			  uint32 h, uint32 w, int bpp)
{
     uint32 col, row, y;
     uint32 tw, th;
     u__char *buf;
     u__char *r, *g, *b;
     int tilesize;
     int fromskew, toskew;
     u__int nrow;
     tileSeparateRoutine put;
  
     put = pickTileSeparateCase(Map);
     if (put == 0) return (0);

     tilesize = TIFFTileSize(tif);
     buf = (u__char *)malloc((size_t) (3*tilesize));
     if (buf == 0) {
	  TIFFError(filename, "No space for tile buffer");
	  return (0);
     }
     r = buf;
     g = r + tilesize;
     b = g + tilesize;
     TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tw);
     TIFFGetField(tif, TIFFTAG_TILELENGTH, &th);
     y = setorientation(tif, h);
     toskew = (orientation == ORIENTATION_TOPLEFT ? -tw + -w : -tw + w);

     for (row = 0; row < h; row += th) {
	  nrow = (row + th > h ? h - row : th);
	  for (col = 0; col < w; col += tw) {
	       tsample_t band;

	       band = 0;
	       if (TIFFReadTile(tif, r, (uint32) col, (uint32) row,0, band) < 0
		   && stoponerr) break;

	       band = 1;
	       if (TIFFReadTile(tif, g, (uint32) col, (uint32) row,0, band) < 0
		   && stoponerr) break;

	       band = 2;
	       if (TIFFReadTile(tif, b, (uint32) col, (uint32) row,0, band) < 0
		   && stoponerr) break;

	       if (col + tw > w) {
		    /*
		     * Tile is clipped horizontally.  Calculate
		     * visible portion and skewing factors.
		     */
		    uint32 npix = w - col;
		    fromskew = tw - npix;
		    (*put)(raster + (y*w + col)*bpp, r, g, b, Map, npix, (uint32) nrow, 
			   fromskew, (int) ((toskew + fromskew)*bpp));
	       } else
		    (*put)(raster + (y*w + col)*bpp, r, g, b, Map, tw, (uint32) nrow, 
			   0, (int) (toskew*bpp));
	  }
	  y += (orientation == ORIENTATION_TOPLEFT ? -nrow : nrow);
     }
     free(buf);
     return (1);
}

/*
 * Get a strip-organized image that has
 *	PlanarConfiguration contiguous if SamplesPerPixel > 1
 * or
 *	SamplesPerPixel == 1
 */	
/*******************************************/
static int gtStripContig(TIFF * tif, byte * raster, RGBvalue * Map,
			 uint32 h, uint32 w, int bpp)
{
     uint32 row, y, nrow;
     u__char *buf;
     tileContigRoutine put;
     uint32 rowsperstrip;
     uint32 imagewidth;
     int scanline;
     int fromskew, toskew;
  
     put = pickTileContigCase(Map);
     if (put == 0)
	  return (0);
     buf = (u__char *) malloc((size_t) TIFFStripSize(tif));
     if (buf == 0) {
	  TIFFError(filename, "No space for strip buffer");
	  return (0);
     }
     y = setorientation(tif, h);
     toskew = (orientation == ORIENTATION_TOPLEFT ? -w + -w : -w + w);
     TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
     TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imagewidth);
     scanline = TIFFScanlineSize(tif);
     fromskew = (w < imagewidth ? imagewidth - w : 0);
     for (row = 0; row < h; row += rowsperstrip) {
	  nrow = (row + rowsperstrip > h ? h - row : rowsperstrip);
	  if (TIFFReadEncodedStrip(tif,
				   TIFFComputeStrip(tif, (uint32) row,(tsample_t) 0),
				   (tdata_t) buf, (tsize_t)(nrow*scanline)) < 0
	      && stoponerr) break;

	  (*put)(raster + y*w*bpp, buf, Map, w, nrow, fromskew, toskew*bpp);

	  y += (orientation == ORIENTATION_TOPLEFT ? -nrow : nrow);
     }
     free(buf);
     return (1);
}


/*
 * Get a strip-organized image with
 *	 SamplesPerPixel > 1
 *	 PlanarConfiguration separated
 * We assume that all such images are RGB.
 */
static int gtStripSeparate(TIFF * tif, byte * raster, RGBvalue * Map,
			   uint32 h, uint32 w, int bpp)
{
     u__char *buf;
     u__char *r, *g, *b;
     uint32 row, y, nrow;
     int scanline;
     tileSeparateRoutine put;
     uint32 rowsperstrip;
     uint32 imagewidth;
     u__int stripsize;
     int fromskew, toskew;
  
     stripsize = TIFFStripSize(tif);
     r = buf = (u__char *) malloc((size_t) 3*stripsize);
     if (buf == 0)
	  return (0);
     g = r + stripsize;
     b = g + stripsize;
     put = pickTileSeparateCase(Map);
     if (put == 0) {
	  TIFFError(filename, "Can not handle format");
	  return (0);
     }
     y = setorientation(tif, h);
     toskew = (orientation == ORIENTATION_TOPLEFT ? -w + -w : -w + w);
     TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
     TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &imagewidth);
     scanline = TIFFScanlineSize(tif);
     fromskew = (w < imagewidth ? imagewidth - w : 0);
     for (row = 0; row < h; row += rowsperstrip) {
	  tsample_t band;

	  nrow = (row + rowsperstrip > h ? h - row : rowsperstrip);
	  band = 0;
	  if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, (uint32) row, band),
				   (tdata_t) r, (tsize_t)(nrow*scanline)) < 0 
	      && stoponerr) break;

	  band = 1;
	  if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, (uint32) row, band),
				   (tdata_t) g, (tsize_t)(nrow*scanline)) < 0
	      && stoponerr) break;

	  band = 2;
	  if (TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, (uint32) row, band),
				   (tdata_t) b, (tsize_t)(nrow*scanline)) < 0 
	      && stoponerr) break;

	  (*put)(raster + y*w*bpp, r, g, b, Map, w, nrow, fromskew, toskew*bpp);

	  y += (orientation == ORIENTATION_TOPLEFT ? -nrow : nrow);
     }
     free(buf);
     return (1);
}


/*
 * Greyscale images with less than 8 bits/sample are handled
 * with a table to avoid lots of shifts and masks.  The table
 * is setup so that put*bwtile (below) can retrieve 8/bitspersample
 * pixel values simply by indexing into the table with one
 * number.
 */
static int makebwmap(void)
{
     register int i;
     int nsamples = 8 / bitspersample;
     register byte *p;
  
     BWmap = (byte **)malloc(
	  256*sizeof (byte *)+(256*nsamples*sizeof(byte)));
     if (BWmap == NULL) {
	  TIFFError(filename, "No space for B&W mapping table");
	  return (0);
     }
     p = (byte *)(BWmap + 256);
     for (i = 0; i < 256; i++) {
	  BWmap[i] = p;
	  switch (bitspersample) {
#define	GREY(x)	*p++ = x;
	  case 1:
	       GREY(i>>7);
	       GREY((i>>6)&1);
	       GREY((i>>5)&1);
	       GREY((i>>4)&1);
	       GREY((i>>3)&1);
	       GREY((i>>2)&1);
	       GREY((i>>1)&1);
	       GREY(i&1);
	       break;
	  case 2:
	       GREY(i>>6);
	       GREY((i>>4)&3);
	       GREY((i>>2)&3);
	       GREY(i&3);
	       break;
	  case 4:
	       GREY(i>>4);
	       GREY(i&0xf);
	       break;
	  case 8:
	       GREY(i);
	       break;
	  }
#undef	GREY
     }
     return (1);
}


/*
 * Palette images with <= 8 bits/sample are handled
 * with a table to avoid lots of shifts and masks.  The table
 * is setup so that put*cmaptile (below) can retrieve 8/bitspersample
 * pixel values simply by indexing into the table with one
 * number.
 */
static int makecmap(void)
{
     register int i;
     int nsamples = 8 / bitspersample;
     register byte *p;
  
     PALmap = (byte **)malloc(
	  256*sizeof (byte *)+(256*nsamples*sizeof(byte)));
     if (PALmap == NULL) {
	  TIFFError(filename, "No space for Palette mapping table");
	  return (0);
     }
     p = (byte *)(PALmap + 256);
     for (i = 0; i < 256; i++) {
	  PALmap[i] = p;
#define	CMAP(x)	*p++ = x;
	  switch (bitspersample) {
	  case 1:
	       CMAP(i>>7);
	       CMAP((i>>6)&1);
	       CMAP((i>>5)&1);
	       CMAP((i>>4)&1);
	       CMAP((i>>3)&1);
	       CMAP((i>>2)&1);
	       CMAP((i>>1)&1);
	       CMAP(i&1);
	       break;
	  case 2:
	       CMAP(i>>6);
	       CMAP((i>>4)&3);
	       CMAP((i>>2)&3);
	       CMAP(i&3);
	       break;
	  case 4:
	       CMAP(i>>4);
	       CMAP(i&0xf);
	       break;
	  case 8:
	       CMAP(i);
	       break;
	  }
#undef CMAP
     }
     return (1);
}


/*
 * The following routines move decoded data returned
 * from the TIFF library into rasters filled with packed
 * ABGR pixels (i.e. suitable for passing to lrecwrite.)
 *
 * The routines have been created according to the most
 * important cases and optimized.  pickTileContigCase and
 * pickTileSeparateCase analyze the parameters and select
 * the appropriate "put" routine to use.
 */

#define	REPEAT8(op)	REPEAT4(op); REPEAT4(op)
#define	REPEAT4(op)	REPEAT2(op); REPEAT2(op)
#define	REPEAT2(op)	op; op
#define	CASE8(x,op)				\
     switch (x) {				\
     case 7: op; case 6: op; case 5: op;	\
     case 4: op; case 3: op; case 2: op;	\
     case 1: op;				\
     }
#define	CASE4(x,op)	switch (x) { case 3: op; case 2: op; case 1: op; }

#define	UNROLL8(w, op1, op2) {			\
	  uint32 x;				\
	  for (x = w; x >= 8; x -= 8) {		\
	       op1;				\
	       REPEAT8(op2);			\
	  }					\
	  if (x > 0) {				\
	       op1;				\
	       CASE8(x,op2);			\
	  }					\
     }

#define	UNROLL4(w, op1, op2) {			\
	  register uint32 x;			\
	  for (x = w; x >= 4; x -= 4) {		\
	       op1;				\
	       REPEAT4(op2);			\
	  }					\
	  if (x > 0) {				\
	       op1;				\
	       CASE4(x,op2);			\
	  }					\
     }

#define	UNROLL2(w, op1, op2) {			\
	  register uint32 x;			\
	  for (x = w; x >= 2; x -= 2) {		\
	       op1;				\
	       REPEAT2(op2);			\
	  }					\
	  if (x) {				\
	       op1;				\
	       op2;				\
	  }					\
     }
			

#define	SKEW(r,g,b,skew)	{ r += skew; g += skew; b += skew; }



/*
 * 8-bit palette => colormap/RGB
 */
static void put8bitcmaptile(byte * cp, u__char * pp, RGBvalue * Map,
			    uint32 w, uint32 h, int fromskew, int toskew)
{
     /* FIXME : unused parameter */
     Map = Map;

     while (h-- > 0) {
	  UNROLL8(w, , *cp++ = PALmap[*pp++][0]);
	  cp += toskew;
	  pp += fromskew;
     }
}

/*
 * 4-bit palette => colormap/RGB
 */
static void put4bitcmaptile(byte * cp, u__char * pp, RGBvalue * Map,
			    uint32 w, uint32 h, int fromskew, int toskew)
{
     register byte *bw;
  
     /* FIXME : unused parameter */
     Map = Map;

     fromskew /= 2;
     while (h-- > 0) {
	  UNROLL2(w, bw = PALmap[*pp++], *cp++ = *bw++);
	  cp += toskew;
	  pp += fromskew;
     }
}


/*
 * 2-bit palette => colormap/RGB
 */
static void put2bitcmaptile(byte * cp, u__char * pp, RGBvalue * Map,
			    uint32 w, uint32 h, int fromskew, int toskew)
{
     register byte *bw;
  
     /* FIXME : unused parameter */
     Map = Map;

     fromskew /= 4;
     while (h-- > 0) {
	  UNROLL4(w, bw = PALmap[*pp++], *cp++ = *bw++);
	  cp += toskew;
	  pp += fromskew;
     }
}

/*
 * 1-bit palette => colormap/RGB
 */
static void put1bitcmaptile(byte * cp, u__char * pp, RGBvalue * Map,
			    uint32 w, uint32 h, int fromskew, int toskew)
{
     register byte *bw;
  
     /* FIXME : unused parameter */
     Map = Map;

     fromskew /= 8;
     while (h-- > 0) {
	  UNROLL8(w, bw = PALmap[*pp++], *cp++ = *bw++);
	  cp += toskew;
	  pp += fromskew;
     }
}


/*
 * 8-bit greyscale => colormap/RGB
 */
static void putgreytile(byte * cp, u__char * pp, RGBvalue * Map,
			uint32 w, uint32 h, int fromskew, int toskew)
{
     /* FIXME : unused parameter */
     Map = Map;

     while (h-- > 0) {
	  register uint32 x;
	  for (x = w; x-- > 0;)
	       *cp++ = BWmap[*pp++][0];
	  cp += toskew;
	  pp += fromskew;
     }
}


/*
 * 1-bit bilevel => colormap/RGB
 */
static void put1bitbwtile(byte * cp, u__char * pp, RGBvalue * Map,
			  uint32 w, uint32 h, int fromskew, int toskew)
{
     register byte *bw;
  
     /* FIXME : unused parameter */
     Map = Map;

     fromskew /= 8;
     while (h-- > 0) {
	  UNROLL8(w, bw = BWmap[*pp++], *cp++ = *bw++);
	  cp += toskew;
	  pp += fromskew;
     }
}

/*
 * 2-bit greyscale => colormap/RGB
 */
static void put2bitbwtile(byte * cp, u__char * pp, RGBvalue * Map,
			  uint32 w, uint32 h, int fromskew, int toskew)
{
     register byte *bw;
  
     /* FIXME : unused parameter */
     Map = Map;

     fromskew /= 4;
     while (h-- > 0) {
	  UNROLL4(w, bw = BWmap[*pp++], *cp++ = *bw++);
	  cp += toskew;
	  pp += fromskew;
     }
}

/*
 * 4-bit greyscale => colormap/RGB
 */
static void put4bitbwtile(byte * cp, u__char * pp, RGBvalue * Map,
			  uint32 w, uint32 h, int fromskew, int toskew)
{
     register byte *bw;
  
     /* FIXME : unused parameter */
     Map = Map;

     fromskew /= 2;
     while (h-- > 0) {
	  UNROLL2(w, bw = BWmap[*pp++], *cp++ = *bw++);
	  cp += toskew;
	  pp += fromskew;
     }
}

/*
 * 16-bit greyscale => colormap/RGB
 */
static void put16bitbwtile(byte * cp, u__char * pp, RGBvalue * Map,
			   uint32 w, uint32 h, int fromskew, int toskew)
{
     register uint32   x;
  
     while (h-- > 0) {
	  for (x=w; x>0; x--) {
	       *cp++ = Map[(pp[0] << 8) + pp[1]];
	       pp += 2;
	  }
	  cp += toskew;
	  pp += fromskew;
     }
}



/*
 * 8-bit packed samples => RGB
 */
static void putRGBcontig8bittile(byte * cp, u__char * pp, RGBvalue * Map,
				 uint32 w, uint32 h, int fromskew, int toskew)
{
     fromskew *= samplesperpixel;
     if (Map) {
	  while (h-- > 0) {
	       register uint32 x;
	       for (x = w; x-- > 0;) {
		    *cp++ = Map[pp[0]];
		    *cp++ = Map[pp[1]];
		    *cp++ = Map[pp[2]];
		    pp += samplesperpixel;
	       }
	       pp += fromskew;
	       cp += toskew;
	  }
     } else {
	  while (h-- > 0) {
	       UNROLL8(w, ,
		       *cp++ = pp[0];
		       *cp++ = pp[1];
		       *cp++ = pp[2];
		       pp += samplesperpixel);
	       cp += toskew;
	       pp += fromskew;
	  }
     }
}

/*
 * 16-bit packed samples => RGB
 */
static void putRGBcontig16bittile(byte * cp, u__short * pp, RGBvalue * Map,
				  uint32 w, uint32 h, int fromskew, int toskew)
{
     register u__int x;
  
     fromskew *= samplesperpixel;
     if (Map) {
	  while (h-- > 0) {
	       for (x = w; x-- > 0;) {
		    *cp++ = Map[pp[0]];
		    *cp++ = Map[pp[1]];
		    *cp++ = Map[pp[2]];
		    pp += samplesperpixel;
	       }
	       cp += toskew;
	       pp += fromskew;
	  }
     } else {
	  while (h-- > 0) {
	       for (x = w; x-- > 0;) {
		    *cp++ = pp[0];
		    *cp++ = pp[1];
		    *cp++ = pp[2];
		    pp += samplesperpixel;
	       }
	       cp += toskew;
	       pp += fromskew;
	  }
     }
}

/*
 * 8-bit unpacked samples => RGB
 */
static void putRGBseparate8bittile(byte * cp, 
				   u__char * r, u__char * g, u__char * b,
				   RGBvalue * Map, uint32 w, uint32 h,
				   int fromskew, int toskew)
{
     if (Map) {
	  while (h-- > 0) {
	       register uint32 x;
	       for (x = w; x > 0; x--) {
		    *cp++ = Map[*r++];
		    *cp++ = Map[*g++];
		    *cp++ = Map[*b++];
	       }
	       SKEW(r, g, b, fromskew);
	       cp += toskew;
	  }
     } else {
	  while (h-- > 0) {
	       UNROLL8(w, ,
		       *cp++ = *r++;
		       *cp++ = *g++;
		       *cp++ = *b++;
		    );
	       SKEW(r, g, b, fromskew);
	       cp += toskew;
	  }
     }
}

/*
 * 16-bit unpacked samples => RGB
 */
static void putRGBseparate16bittile(byte * cp, 
				    u__short * r, u__short * g, u__short * b,
				    RGBvalue * Map, uint32 w, uint32 h,
				    int fromskew, int toskew)
{
     uint32 x;
  
     if (Map) {
	  while (h-- > 0) {
	       for (x = w; x > 0; x--) {
		    *cp++ = Map[*r++];
		    *cp++ = Map[*g++];
		    *cp++ = Map[*b++];
	       }
	       SKEW(r, g, b, fromskew);
	       cp += toskew;
	  }
     } else {
	  while (h-- > 0) {
	       for (x = 0; x < w; x++) {
		    *cp++ = *r++;
		    *cp++ = *g++;
		    *cp++ = *b++;
	       }
	       SKEW(r, g, b, fromskew);
	       cp += toskew;
	  }
     }
}

#define Code2V(c, RB, RW, CR)  ((((c)-(int)RB)*(float)CR)/(float)(RW-RB))

#define	CLAMP(f,min,max)						\
     (int)((f)+.5 < (min) ? (min) : (f)+.5 > (max) ? (max) : (f)+.5)

#define	LumaRed		YCbCrCoeffs[0]
#define	LumaGreen	YCbCrCoeffs[1]
#define	LumaBlue	YCbCrCoeffs[2]

static	float D1, D2;
static	float D3, D4, D5;


static void initYCbCrConversion(void)
{
     D1 = 2 - 2*LumaRed;
     D2 = D1*LumaRed / LumaGreen;
     D3 = 2 - 2*LumaBlue;
     D4 = D2*LumaBlue / LumaGreen;
     D5 = 1.0 / LumaGreen;
}

static void putRGBContigYCbCrClump(byte * cp, u__char * pp, 
				   int cw, int ch, uint32 w, int n,
				   int fromskew, int toskew)
{
     float Cb, Cr;
     int j, k;
  
     Cb = Code2V(pp[n],   refBlackWhite[2], refBlackWhite[3], 127);
     Cr = Code2V(pp[n+1], refBlackWhite[4], refBlackWhite[5], 127);
     for (j = 0; j < ch; j++) {
	  for (k = 0; k < cw; k++) {
	       float Y, R, G, B;
	       Y = Code2V(*pp++,
			  refBlackWhite[0], refBlackWhite[1], 255);
	       R = Y + Cr*D1;
	       B = Y + Cb*D3;
	       G = Y*D5 - Cb*D4 - Cr*D2;
	       cp[3*k+0] = CLAMP(R,0,255);
	       cp[3*k+1] = CLAMP(G,0,255);
	       cp[3*k+2] = CLAMP(B,0,255);
	  }
	  cp += 3*w+toskew;
	  pp += fromskew;
     }
}

#undef LumaBlue
#undef LumaGreen
#undef LumaRed
#undef CLAMP
#undef Code2V


/*
 * 8-bit packed YCbCr samples => RGB
 */
static void putcontig8bitYCbCrtile(byte * cp, u__char * pp, RGBvalue * Map,
				   uint32 w, uint32 h, int fromskew, int toskew)
{
     u__int Coff = YCbCrVertSampling * YCbCrHorizSampling;
     byte *tp;
     uint32 x;
  
     /* FIXME : unused parameter */
     Map = Map;

     /* XXX adjust fromskew */
     while (h >= YCbCrVertSampling) {
	  tp = cp;
	  for (x = w; x >= YCbCrHorizSampling; x -= YCbCrHorizSampling) {
	       putRGBContigYCbCrClump(tp, pp, YCbCrHorizSampling, YCbCrVertSampling,
				      w, (int) Coff, 0, toskew);
	       tp += 3*YCbCrHorizSampling;
	       pp += Coff+2;
	  }
	  if (x > 0) {
	       putRGBContigYCbCrClump(tp, pp, (int) x, YCbCrVertSampling,
				      w, (int) Coff, (int)(YCbCrHorizSampling - x),
				      toskew);
	       pp += Coff+2;
	  }
	  cp += YCbCrVertSampling*(3*w + toskew);
	  pp += fromskew;
	  h -= YCbCrVertSampling;
     }
     if (h > 0) {
	  tp = cp;
	  for (x = w; x >= YCbCrHorizSampling; x -= YCbCrHorizSampling) {
	       putRGBContigYCbCrClump(tp, pp, YCbCrHorizSampling, (int) h,
				      w, (int) Coff, 0, toskew);
	       tp += 3*YCbCrHorizSampling;
	       pp += Coff+2;
	  }
	  if (x > 0)
	       putRGBContigYCbCrClump(tp, pp, (int) x, (int) h, w, 
				      (int)Coff, (int)(YCbCrHorizSampling-x),toskew);
     }
}

/*
 * Select the appropriate conversion routine for packed data.
 */
static tileContigRoutine pickTileContigCase(RGBvalue * Map)
{
     tileContigRoutine put = 0;
  
     /* FIXME : unused parameter */
     Map = Map;

     switch (photometric) {
     case PHOTOMETRIC_RGB:
	  switch (bitspersample) {
	  case 8:  put = (tileContigRoutine) putRGBcontig8bittile;   break;
	  case 16: put = (tileContigRoutine) putRGBcontig16bittile;  break;
	  }
	  break;
    
     case PHOTOMETRIC_PALETTE:
	  switch (bitspersample) {
	  case 8: put = put8bitcmaptile; break;
	  case 4: put = put4bitcmaptile; break;
	  case 2: put = put2bitcmaptile; break;
	  case 1: put = put1bitcmaptile; break;
	  }
	  break;

     case PHOTOMETRIC_MINISWHITE:
     case PHOTOMETRIC_MINISBLACK:
	  switch (bitspersample) {
	  case 16: put = put16bitbwtile; break;
	  case 8:  put = putgreytile;    break;
	  case 4:  put = put4bitbwtile;  break;
	  case 2:  put = put2bitbwtile;  break;
	  case 1:  put = put1bitbwtile;  break;
	  }
	  break;

     case PHOTOMETRIC_YCBCR:
	  switch (bitspersample) {
	  case 8: put = putcontig8bitYCbCrtile; break;
	  }
	  break;
     }

     if (put==0) TIFFError(filename, "Can not handle format");
     return (put);
}


/*
 * Select the appropriate conversion routine for unpacked data.
 *
 * NB: we assume that unpacked single channel data is directed
 *	 to the "packed routines.
 */
static tileSeparateRoutine pickTileSeparateCase(RGBvalue * Map)
{
     tileSeparateRoutine put = 0;
  
     /* FIXME : unused parameter */
     Map = Map;

     switch (photometric) {
     case PHOTOMETRIC_RGB:
	  switch (bitspersample) {
	  case  8: put = (tileSeparateRoutine) putRGBseparate8bittile;  break;
	  case 16: put = (tileSeparateRoutine) putRGBseparate16bittile; break;
	  }
	  break;
     }

     if (put==0) TIFFError(filename, "Can not handle format");
     return (put);
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ WRITE TIFF */

/*~~~~~ Write 24-bits color TIFF CHAR ~~~~~*/

short _mw_ccimage_create_tiff(char * fname, Ccimage image)
{
     TIFF *tif;
     byte *pic;
     unsigned int N;

     tif = TIFFOpen(fname, "w");
     if (!tif)
     {
	  mwerror(ERROR, 0,"Cannot create TIFF file \"%s\"\n",fname);
	  return(-1);
     }

     if ((image == NULL) || (image->red == NULL)  || 
	 (image->green == NULL) || (image->blue == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_ccimage_create_tiff] NULL image or image planes\n");
	  return(-2);
     }

     /* Size of the image */
     TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, image->ncol);
     TIFFSetField(tif, TIFFTAG_IMAGELENGTH, image->nrow);

     /* Compression Scheme (COMPRESSION_LZW,..) */
     TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

     /* Comment field */
     if (image->cmt && (strlen(image->cmt) > 0)) 
	  TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, image->cmt);

     TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
     TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
     TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, image->nrow);

     TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, (int)2);
     TIFFSetField(tif, TIFFTAG_XRESOLUTION, (float)1200.0);
     TIFFSetField(tif, TIFFTAG_YRESOLUTION, (float)1200.0);

     TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
     TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   8);
     TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,     PHOTOMETRIC_RGB);

     N = image->ncol * image->nrow * 3;
     pic = (byte *) malloc(N);
     if (!pic)
     {
	  mwerror(ERROR,0,"Not enough memory to create \"%s\"\n",fname);
	  TIFFClose(tif);
	  return(-3);
     }
  
     _mw_3x8_to_1x24XV_ucharplanes(image->red,image->green,image->blue,pic,N);
     TIFFWriteEncodedStrip(tif, 0, pic, N);
     free(pic);

     TIFFClose(tif);
     return(0);

}


/*~~~~~ Write 8-bits gray levels TIFF CHAR ~~~~~*/

short _mw_cimage_create_tiff(char * fname, Cimage image)
{
     TIFF *tif;

     tif = TIFFOpen(fname, "w");
     if (!tif)
     {
	  mwerror(ERROR, 0,"Cannot create TIFF file \"%s\"\n",fname);
	  return(-1);
     }

     if ((image == NULL) || (image->gray == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_create_tiff] NULL image or image plane\n");
	  return(-2);
     }

     /* Size of the image */
     TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, image->ncol);
     TIFFSetField(tif, TIFFTAG_IMAGELENGTH, image->nrow);

     /* Compression Scheme (COMPRESSION_LZW,..) */
     TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

     /* Comment field */
     if (image->cmt && (strlen(image->cmt) > 0)) 
	  TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, image->cmt);

     TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
     TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
     TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, image->nrow);

     TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, (int)2);
     TIFFSetField(tif, TIFFTAG_XRESOLUTION, (float)1200.0);
     TIFFSetField(tif, TIFFTAG_YRESOLUTION, (float)1200.0);

     TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
     TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   8);
     TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,     PHOTOMETRIC_MINISBLACK);

     TIFFWriteEncodedStrip(tif, 0, image->gray, image->nrow*image->ncol);
     TIFFClose(tif);
     return(0);

}

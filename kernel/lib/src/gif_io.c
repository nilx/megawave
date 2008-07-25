/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  gif_io.c
   
  Vers. 1.0
  Author : Jacques Froment
  Part of code
  (C) 1988, 1989 by Patrick J. Naughton, Michael Mauldin and David Rowley
  (C) 1989, 1990 by the University of Pennsylvania

  Input/Output functions for the GIF file compatibility with MegaWave2

  Main changes :
  v1.0 (JF): 
  - added include <string> (Linux 2.6.12 & gcc 4.0.2)
  - cast added in strncmp((char *)ptr, id, 6));
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

#include "mw.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ LOAD GIF */

typedef int boolean;
typedef unsigned char byte;

#define NEXTBYTE (*ptr++)
#define IMAGESEP 0x2c
#define EXTENSION 0x21
#define INTERLACEMASK 0x40
#define COLORMAPMASK 0x80

#define True 1
#define False 0

/* MONO returns total intensity of r,g,b components */
#define MONO(rd,gn,bl) (((rd)*11 + (gn)*16 + (bl)*5) >> 5)  /*.33R+ .5G+ .17B*/


int BitOffset = 0,		/* Bit Offset of next code */
     XC = 0, YC = 0,		/* Output X and Y coords of current pixel */
     Pass = 0,			/* Used by output routine if interlaced pic */
     OutCount = 0,		/* Decompressor output 'stack count' */
     RWidth, RHeight,		/* screen dimensions */
     Width, Height,		/* image dimensions */
     LeftOfs, TopOfs,		/* image offset */
     BitsPerPixel,		/* Bits per pixel, read from GIF header */
     BytesPerScanline,		/* bytes per scanline in output raster */
     ColorMapSize,		/* number of colors */
     Background,			/* background color */
     CodeSize,			/* Code size, read from GIF header */
     InitCodeSize,		/* Starting code size, used during Clear */
     Code,			/* Value returned by ReadCode */
     MaxCode,			/* limiting value for current code size */
     ClearCode,			/* GIF clear code */
     EOFCode,			/* GIF end-of-information code */
     CurCode, OldCode, InCode,	/* Decompressor variables */
     FirstFree,			/* First free code, generated per GIF spec */
     FreeCode,			/* Decompressor,next free slot in hash table */
     FinChar,			/* Decompressor variable */
     BitMask,			/* AND mask for data size */
     ReadMask,			/* Code AND mask for current code size */
     Misc;                       /* miscellaneous bits (interlace, local cmap)*/


boolean Interlace, HasColormap;

byte *RawGIF;			/* The heap array to hold it, raw */
byte *Raster;			/* The raster data stream, unblocked */
byte *pic;

/* The hash table used by the decompressor */
int Prefix[4096];
int Suffix[4096];

/* An output array used by the decompressor */
int OutCode[1025];

/* The colormap */
static byte r[256],g[256],b[256];


char *id = "GIF87a";
  
static int  ReadCode();
static void DoInterlace(byte);

int filesize;


/*~~~~~ Load GIF ~~~~~*/

/* GIF format defines a colormap even for gray levels images. */
/* So we convert the colormap to gray levels.                 */

Cimage _mw_cimage_load_gif(char * fname)
{
     Cimage image;
     FILE *fp;

     register byte  ch, ch1;
     register byte *ptr, *ptr1, *picptr;
     register int   i;
     int            npixels, maxpixels;
     float normaspect;    /* normal aspect ratio of this picture */
     int   not_a_gray_image;
  
     /* initialize variables */
     BitOffset = XC = YC = Pass = OutCount = npixels = maxpixels = 0;
     RawGIF = Raster = pic = NULL;
  
     fp = fopen(fname,"r");
     if (!fp) 
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  return(NULL);
     }
  
     /* find the size of the file */
     fseek(fp, 0L, 2);
     filesize = ftell(fp);
     fseek(fp, 0L, 0);
  
     /* the +256's are so we can read truncated GIF files without fear of 
	segmentation violation */
     if (!(ptr = RawGIF = (byte *) malloc(filesize+256)))
	  mwerror(FATAL,1,"Not enough memory to read any GIF file !\n");
  
     if (!(Raster = (byte *) malloc(filesize+256)))    
	  mwerror(FATAL,1,"Not enough memory to read any GIF file !\n");
  
     if ((fread(ptr, filesize, 1, fp) != 1) || (strncmp((char *)ptr, id, 6)))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"... Not a GIF format or file corrupted !\n",fname);
	  free(ptr);
	  free(Raster);
	  fclose(fp);
	  return(NULL);
     }
  
     ptr += 6;
  
     /* Get variables from the GIF screen descriptor */
  
     ch = NEXTBYTE;
     RWidth = ch + 0x100 * NEXTBYTE;	/* screen dimensions... not used. */
     ch = NEXTBYTE;
     RHeight = ch + 0x100 * NEXTBYTE;
  
     ch = NEXTBYTE;
     HasColormap = ((ch & COLORMAPMASK) ? True : False);
  
     BitsPerPixel = (ch & 7) + 1;
     ColorMapSize = 1 << BitsPerPixel; /* Number of colors */
     BitMask = ColorMapSize - 1;
  
     Background = NEXTBYTE;		/* background color... not used. */
  
     if (NEXTBYTE)		/* supposed to be NULL */
     {
	  mwerror(ERROR, 0,"GIF file \"%s\" corrupted !\n",fname);
	  free(ptr);
	  free(Raster);
	  fclose(fp);
	  return(NULL);
     }

     /* Read in global colormap. */
  
     if (HasColormap)
	  for (i=0; i<ColorMapSize; i++) {
	       r[i] = NEXTBYTE;
	       g[i] = NEXTBYTE;
	       b[i] = NEXTBYTE;
	  }

     else {  /* no colormap in GIF file */
	  /* Put Gray scale */
	  for (i=0; i<256; i++) {
	       r[i] = i; /*EGApalette[i&15][0];*/
	       g[i] = i; /*EGApalette[i&15][1];*/
	       b[i] = i; /*EGApalette[i&15][2];*/
	  }
     }


     while ( (i=NEXTBYTE) == EXTENSION) {  /* parse extension blocks */
	  int i, fn, blocksize, aspnum, aspden;

	  /* read extension block */
	  fn = NEXTBYTE;

	  do {
	       i = 0;  blocksize = NEXTBYTE;
	       while (i<blocksize) {
		    if (fn == 'R' && blocksize == 2) {   /* aspect ratio extension */
			 aspnum = NEXTBYTE;  i++;
			 aspden = NEXTBYTE;  i++;
			 if (aspden>0 && aspnum>0) 
			      normaspect = (float) aspnum / (float) aspden;
			 else { normaspect = 1.0;  aspnum = aspden = 1; }

		    }
		    else { NEXTBYTE;  i++; }
	       }
	  } while (blocksize);
     }


     /* Check for image seperator */
     if (i != IMAGESEP) 
     {
	  mwerror(ERROR, 0,"GIF file \"%s\" is corrupted (no image separator) !\n",fname);
	  free(ptr);
	  free(Raster);
	  fclose(fp);
	  return(NULL);
     }
  
     /* Now read in values from the image descriptor */
  
     ch = NEXTBYTE;
     LeftOfs = ch + 0x100 * NEXTBYTE;
     ch = NEXTBYTE;
     TopOfs = ch + 0x100 * NEXTBYTE;
     ch = NEXTBYTE;
     Width = ch + 0x100 * NEXTBYTE;
     ch = NEXTBYTE;
     Height = ch + 0x100 * NEXTBYTE;

     Misc = NEXTBYTE;
     Interlace = ((Misc & INTERLACEMASK) ? True : False);

     if (Misc & 0x80) {
	  for (i=0; i< 1 << ((Misc&7)+1); i++) {
	       r[i] = NEXTBYTE;
	       g[i] = NEXTBYTE;
	       b[i] = NEXTBYTE;
	  }
     }

     if (!HasColormap && !(Misc&0x80)) 
     {
	  /* no global or local colormap */
	  mwerror(WARNING, 0,"No colormap in the GIF file \"%s\": Assuming Gray scale.\n",fname);
     }
    
     /* Start reading the raster data. First we get the intial code size
      * and compute decompressor constant values, based on this code size.
      */
  
     CodeSize = NEXTBYTE;
     ClearCode = (1 << CodeSize);
     EOFCode = ClearCode + 1;
     FreeCode = FirstFree = ClearCode + 2;
  
     /* The GIF spec has it that the code size is the code size used to
      * compute the above values is the code size given in the file, but the
      * code size used in compression/decompression is the code size given in
      * the file plus one. (thus the ++).
      */
  
     CodeSize++;
     InitCodeSize = CodeSize;
     MaxCode = (1 << CodeSize);
     ReadMask = MaxCode - 1;

     /* UNBLOCK:
      * Read the raster data.  Here we just transpose it from the GIF array
      * to the Raster array, turning it from a series of blocks into one long
      * data stream, which makes life much easier for ReadCode().
      */
  
     ptr1 = Raster;
     do {
	  ch = ch1 = NEXTBYTE;
	  while (ch--) { *ptr1 = NEXTBYTE; ptr1++; }
	  if ((ptr - RawGIF) > filesize) {
	       mwerror(WARNING, 0,"GIF file \"%s\" may be truncated !\n",fname);
	       break;
	  }
     } while(ch1);
     free(RawGIF);	 RawGIF = NULL; 	/* We're done with the raw data now */

  
     /* memory allocation for the image */

     maxpixels = Width*Height;
     image = mw_change_cimage(NULL,Height,Width);
     if (image == NULL)
	  mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",fname);

     picptr = pic = (byte *) image->gray;

  
     /* Decompress the file, continuing until you see the GIF EOF code.
      * One obvious enhancement is to add checking for corrupt files here.
      */
  
     Code = ReadCode();
     while (Code != EOFCode) {
	  /* Clear code sets everything back to its initial value, then reads the
	   * immediately subsequent code as uncompressed data.
	   */

	  if (Code == ClearCode) {
	       CodeSize = InitCodeSize;
	       MaxCode = (1 << CodeSize);
	       ReadMask = MaxCode - 1;
	       FreeCode = FirstFree;
	       Code = ReadCode();
	       CurCode = OldCode = Code;
	       FinChar = CurCode & BitMask;
	       if (!Interlace) *picptr++ = FinChar;
	       else DoInterlace(FinChar);
	       npixels++;
	  }
	  else {
	       /* If not a clear code, must be data: save same as CurCode and InCode */

	       /* if we're at maxcode and didn't get a clear, stop loading */
	       if (FreeCode>=4096) { /* printf("freecode blew up\n"); */
		    break; }

	       CurCode = InCode = Code;
      
	       /* If greater or equal to FreeCode, not in the hash table yet;
		* repeat the last character decoded
		*/
      
	       if (CurCode >= FreeCode) {
		    CurCode = OldCode;
		    if (OutCount > 1024) {  /* printf("outcount1 blew up\n"); */ break; }
		    OutCode[OutCount++] = FinChar;
	       }
      
	       /* Unless this code is raw data, pursue the chain pointed to by CurCode
		* through the hash table to its end; each code in the chain puts its
		* associated output code on the output queue.
		*/
      
	       while (CurCode > BitMask) {
		    if (OutCount > 1024) break;   /* corrupt file */
		    OutCode[OutCount++] = Suffix[CurCode];
		    CurCode = Prefix[CurCode];
	       }
      
	       if (OutCount > 1024) { /* printf("outcount blew up\n"); */ break; }
      
	       /* The last code in the chain is treated as raw data. */
      
	       FinChar = CurCode & BitMask;
	       OutCode[OutCount++] = FinChar;
      
	       /* Now we put the data out to the Output routine.
		* It's been stacked LIFO, so deal with it that way...
		*/

	       /* safety thing:  prevent exceeding range of 'pic' */
	       if (npixels + OutCount > maxpixels) OutCount = maxpixels-npixels;
	
	       npixels += OutCount;
	       if (!Interlace) for (i=OutCount-1; i>=0; i--) *picptr++ = OutCode[i];
	       else  for (i=OutCount-1; i>=0; i--) DoInterlace(OutCode[i]);
	       OutCount = 0;

	       /* Build the hash table on-the-fly. No table is stored in the file. */
      
	       Prefix[FreeCode] = OldCode;
	       Suffix[FreeCode] = FinChar;
	       OldCode = InCode;
      
	       /* Point to the next slot in the table.  If we exceed the current
		* MaxCode value, increment the code size unless it's already 12.  If it
		* is, do nothing: the next code decompressed better be CLEAR
		*/
      
	       FreeCode++;
	       if (FreeCode >= MaxCode) {
		    if (CodeSize < 12) {
			 CodeSize++;
			 MaxCode *= 2;
			 ReadMask = (1 << CodeSize) - 1;
		    }
	       }
	  }
	  Code = ReadCode();
	  if (npixels >= maxpixels) break;
     }
     free(Raster);  Raster = NULL;
  
     if (npixels != maxpixels) {
	  mwerror(WARNING, 0,"GIF file \"%s\" may be truncated !\n",fname);
	  memset(pic+npixels, 0, maxpixels-npixels);  /* clear to EOBuffer */
     }

     fclose(fp);
  
     /* Put the colormap into the gray-levels image */

     not_a_gray_image = 0;
     for (i=0; i<ColorMapSize; i++) 
	  if ((r[i] != g[i]) || (g[i] != b[i])) not_a_gray_image = 1;
     if (not_a_gray_image != 0)
	  mwerror(WARNING,0,"Convert a %d colors image to a gray levels image\n",
		  ColorMapSize); 
  
     picptr = image->gray;
     for (i=1;i<=npixels; i++, picptr++ ) *picptr =
					       MONO(r[*picptr],g[*picptr],b[*picptr]);

     _mw_flip_image((unsigned char *) image->gray,sizeof(char),Width,Height,FALSE);
  
     return(image);
}


/* Fetch the next code from the raster data stream.  The codes can be
 * any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
 * maintain our location in the Raster array as a BIT Offset.  We compute
 * the byte Offset into the raster array by dividing this by 8, pick up
 * three bytes, compute the bit Offset into our 24-bit chunk, shift to
 * bring the desired code to the bottom, then mask it off and return it. 
 */

static int ReadCode()
{
     int RawCode, ByteOffset;
  
     ByteOffset = BitOffset / 8;
     RawCode = Raster[ByteOffset] + (Raster[ByteOffset + 1] << 8);
     if (CodeSize >= 8)
	  RawCode += (Raster[ByteOffset + 2] << 16);
     RawCode >>= (BitOffset % 8);
     BitOffset += CodeSize;

     return(RawCode & ReadMask);
}


/***************************/
static void DoInterlace(byte Index)
{
     static byte *ptr = NULL;
     static int   oldYC = -1;
  
     if (oldYC != YC) {  ptr = pic + YC * Width;  oldYC = YC; }
  
     if (YC<Height)
	  *ptr++ = Index;
  
     /* Update the X-coordinate, and if it overflows, update the Y-coordinate */
  
     if (++XC == Width) {
    
	  /* deal with the interlace as described in the GIF
	   * spec.  Put the decoded scan line out to the screen if we haven't gone
	   * past the bottom of it
	   */
    
	  XC = 0;
    
	  switch (Pass) {
	  case 0:
	       YC += 8;
	       if (YC >= Height) { Pass++; YC = 4; }
	       break;
      
	  case 1:
	       YC += 8;
	       if (YC >= Height) { Pass++; YC = 2; }
	       break;
      
	  case 2:
	       YC += 4;
	       if (YC >= Height) { Pass++; YC = 1; }
	       break;
      
	  case 3:
	       YC += 2;  break;
      
	  default:
	       break;
	  }
     }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ WRITE GIF */


typedef long int        count_int;

static int  curx, cury;
static long CountDown;
/* static int  Interlace; */
static byte bw[2] = {0, 0xff};

static void putword(int, FILE *);
static void compress(int, FILE *, byte *, int);
static void output(int);
static void cl_block(void);
static void cl_hash(count_int);
static void char_init(void);
static void char_out(int);
static void flush_char(void);

/*~~~~~ Write GIF ~~~~~*/

/* It generates a gray-levels colormap */

short _mw_cimage_create_gif(char * fname, Cimage image)
{
     FILE *fp;
     byte *pic;
     int   w,h;
     int   numcols;
     int RWidth, RHeight;
     int LeftOfs, TopOfs;
     int Resolution, ColorMapSize, InitCodeSize, Background, BitsPerPixel;
     int i,j;

     if ((image->ncol>=65536)||(image->nrow>=65536))
     {
	  mwerror(FATAL,1,"Image too big to be saved using GIF external type !\n");
	  return(-1);
     }
 
     if (!(fp = fopen(fname, "w")))
     {
	  mwerror(ERROR, 0,"Cannot create the file \"%s\"\n",fname);
	  return(-1);
     }

     if ((image == NULL) || (image->gray == NULL))
     {
	  mwerror(INTERNAL, 0,"[_mw_cimage_create_pm] NULL cimage or cimage plane\n");
	  return(-2);
     }

     w = image->ncol;
     h = image->nrow;
     numcols = 256;
     pic = image->gray;

     Interlace = 0;
     Background = 0;

     BitsPerPixel = 8;

     ColorMapSize = 1 << BitsPerPixel;
	
     RWidth  = Width  = w;
     RHeight = Height = h;
     LeftOfs = TopOfs = 0;
	
     Resolution = BitsPerPixel;

     CountDown = w * h;    /* # of pixels we'll be doing */

     if (BitsPerPixel <= 1) InitCodeSize = 2;
     else InitCodeSize = BitsPerPixel;

     curx = cury = 0;

     fwrite("GIF87a", 1, 6, fp);    /* the GIF magic number */

     putword(RWidth, fp);           /* screen descriptor */
     putword(RHeight, fp);

     i = 0x80;	                 /* Yes, there is a color map */
     i |= (8-1)<<4;                 /* OR in the color resolution (hardwired 8) */
     i |= (BitsPerPixel - 1);       /* OR in the # of bits per pixel */
     fputc(i,fp);          

     fputc(Background, fp);         /* background color */

     fputc(0, fp);                  /* future expansion byte */


     /* Put greyscale colormap */
     for (i=0; i<ColorMapSize; i++) 
     {
	  fputc(i, fp);
	  fputc(i, fp);
	  fputc(i, fp);
     }
     fputc( ',', fp );              /* image separator */

     /* Write the Image header */
     putword(LeftOfs, fp);
     putword(TopOfs,  fp);
     putword(Width,   fp);
     putword(Height,  fp);
     if (Interlace) fputc(0x40, fp);   /* Use Global Colormap, maybe Interlace */
     else fputc(0x00, fp);

     fputc(InitCodeSize, fp);
     compress(InitCodeSize+1, fp, pic, w*h);

     fputc(0,fp);                      /* Write out a Zero-length packet (EOF) */
     fputc(';',fp);                    /* Write GIF file terminator */

     fclose(fp);
     return (0);
}




/******************************/
static void putword(int w, FILE * fp)
{
     /* writes a 16-bit integer in GIF order (LSB first) */
     fputc(w & 0xff, fp);
     fputc((w>>8)&0xff, fp);
}




/***********************************************************************/


static unsigned long cur_accum = 0;
static int           cur_bits = 0;




#define min(a,b)        ((a>b) ? b : a)

#define BITS	12
#define MSDOS	1

#define HSIZE  5003            /* 80% occupancy */

typedef unsigned char   char_type;


static int n_bits;                   /* number of bits/code */
static int maxbits = BITS;           /* user settable max # bits/code */
static int maxcode;                  /* maximum code, given n_bits */
static int maxmaxcode = 1 << BITS;   /* NEVER generate this */

#define MAXCODE(n_bits)     ( (1 << (n_bits)) - 1)

static  count_int      htab [HSIZE];
static  unsigned short codetab [HSIZE];
#define HashTabOf(i)   htab[i]
#define CodeTabOf(i)   codetab[i]

static int hsize = HSIZE;            /* for dynamic table sizing */

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**BITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i) CodeTabOf(i)
#define tab_suffixof(i)        ((char_type *)(htab))[i]
#define de_stack               ((char_type *)&tab_suffixof(1<<BITS))

static int free_ent = 0;                  /* first unused entry */

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
static int clear_flg = 0;

static long int in_count = 1;            /* length of input */
static long int out_count = 0;           /* # of codes output (for debugging) */

/*
 * compress stdin to stdout
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the 
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

static int g_init_bits;
static FILE *g_outfile;

/* static int ClearCode; */
/* static int EOFCode;   */


/********************************************************/
static void compress(int init_bits, FILE * outfile, byte * data, int len)
{
     register long fcode;
     register int i = 0;
     register int c;
     register int ent;
     register int disp;
     register int hsize_reg;
     register int hshift;

     /*
      * Set up the globals:  g_init_bits - initial number of bits
      *                      g_outfile   - pointer to output file
      */
     g_init_bits = init_bits;
     g_outfile   = outfile;

     /* initialize 'compress' globals */
     maxbits = BITS;
     maxmaxcode = 1<<BITS;
     memset((char *) htab, 0, sizeof(htab));
     memset((char *) codetab, 0, sizeof(codetab));
     hsize = HSIZE;
     free_ent = 0;
     clear_flg = 0;
     in_count = 1;
     out_count = 0;
     cur_accum = 0;
     cur_bits = 0;


     /*
      * Set up the necessary values
      */
     out_count = 0;
     clear_flg = 0;
     in_count = 1;
     maxcode = MAXCODE(n_bits = g_init_bits);

     ClearCode = (1 << (init_bits - 1));
     EOFCode = ClearCode + 1;
     free_ent = ClearCode + 2;

     char_init();
     ent = *data++;  len--;

     hshift = 0;
     for ( fcode = (long) hsize;  fcode < 65536L; fcode *= 2L )
	  hshift++;
     hshift = 8 - hshift;                /* set hash code range bound */

     hsize_reg = hsize;
     cl_hash( (count_int) hsize_reg);            /* clear hash table */

     output(ClearCode);
    
     while (len) {
	  c = *data++;  len--;
	  in_count++;

	  fcode = (long) ( ( (long) c << maxbits) + ent);
	  i = (((int) c << hshift) ^ ent);    /* xor hashing */

	  if ( HashTabOf (i) == fcode ) {
	       ent = CodeTabOf (i);
	       continue;
	  }

	  else if ( (long)HashTabOf (i) < 0 )      /* empty slot */
	       goto nomatch;

	  disp = hsize_reg - i;           /* secondary hash (after G. Knott) */
	  if ( i == 0 )
	       disp = 1;

     probe:
	  if ( (i -= disp) < 0 )
	       i += hsize_reg;

	  if ( HashTabOf (i) == fcode ) {
	       ent = CodeTabOf (i);
	       continue;
	  }

	  if ( (long)HashTabOf (i) > 0 ) 
	       goto probe;

     nomatch:
	  output(ent);
	  out_count++;
	  ent = c;

	  if ( free_ent < maxmaxcode ) {
	       CodeTabOf (i) = free_ent++; /* code -> hashtable */
	       HashTabOf (i) = fcode;
	  }
	  else
	       cl_block();
     }

     /* Put out the final code */
     output(ent);
     out_count++;
     output(EOFCode);
}


/*****************************************************************
 * TAG( output )
 *
 * Output the given code.
 * Inputs:
 *      code:   A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *              that n_bits =< (long)wordsize - 1.
 * Outputs:
 *      Outputs code to the file.
 * Assumptions:
 *      Chars are 8 bits long.
 * Algorithm:
 *      Maintain a BITS character long buffer (so that 8 codes will
 * fit in it exactly).  Use the VAX insv instruction to insert each
 * code in turn.  When the buffer fills up empty it and start over.
 */

static
unsigned long masks[] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
			  0x001F, 0x003F, 0x007F, 0x00FF,
			  0x01FF, 0x03FF, 0x07FF, 0x0FFF,
			  0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

static void output(int code)
{
     cur_accum &= masks[cur_bits];

     if (cur_bits > 0)
	  cur_accum |= ((long)code << cur_bits);
     else
	  cur_accum = code;
	
     cur_bits += n_bits;

     while( cur_bits >= 8 ) {
	  char_out( (unsigned int) (cur_accum & 0xff) );
	  cur_accum >>= 8;
	  cur_bits -= 8;
     }

     /*
      * If the next entry is going to be too big for the code size,
      * then increase it, if possible.
      */

     if (free_ent > maxcode || clear_flg) {

	  if( clear_flg ) {
	       maxcode = MAXCODE (n_bits = g_init_bits);
	       clear_flg = 0;
	  }
	  else {
	       n_bits++;
	       if ( n_bits == maxbits )
		    maxcode = maxmaxcode;
	       else
		    maxcode = MAXCODE(n_bits);
	  }
     }
	
     if( code == EOFCode ) {
	  /* At EOF, write the rest of the buffer */
	  while( cur_bits > 0 ) {
	       char_out( (unsigned int)(cur_accum & 0xff) );
	       cur_accum >>= 8;
	       cur_bits -= 8;
	  }

	  flush_char();
	
	  fflush( g_outfile );

	  if( ferror( g_outfile ) )
	       mwerror(FATAL, 0,"Error while writing GIF file !\n");
     }
}


/********************************/
static void cl_block ()             /* table clear for block compress */
{
     /* Clear out the hash table */

     cl_hash ( (count_int) hsize );
     free_ent = ClearCode + 2;
     clear_flg = 1;

     output(ClearCode);
}


/********************************/
static void cl_hash(count_int hsize)          /* reset code table */
{
     register count_int * htab_p = htab+hsize;
     register long i;
     register long m1 = -1;

     i = hsize - 16;
     do {                            /* might use Sys V memset(3) here */
	  *(htab_p-16) = m1;
	  *(htab_p-15) = m1;
	  *(htab_p-14) = m1;
	  *(htab_p-13) = m1;
	  *(htab_p-12) = m1;
	  *(htab_p-11) = m1;
	  *(htab_p-10) = m1;
	  *(htab_p-9) = m1;
	  *(htab_p-8) = m1;
	  *(htab_p-7) = m1;
	  *(htab_p-6) = m1;
	  *(htab_p-5) = m1;
	  *(htab_p-4) = m1;
	  *(htab_p-3) = m1;
	  *(htab_p-2) = m1;
	  *(htab_p-1) = m1;
	  htab_p -= 16;
     } while ((i -= 16) >= 0);

     for ( i += 16; i > 0; i-- )
	  *--htab_p = m1;
}


/******************************************************************************
 *
 * GIF Specific routines
 *
 ******************************************************************************/

/*
 * Number of characters so far in this 'packet'
 */
static int a_count;

/*
 * Set up the 'byte output' routine
 */
static void char_init()
{
     a_count = 0;
}

/*
 * Define the storage for the packet accumulator
 */
static char accum[ 256 ];

/*
 * Add a character to the end of the current packet, and if it is 254
 * characters, flush the packet to disk.
 */
static void char_out(int c)
{
     accum[ a_count++ ] = c;
     if( a_count >= 254 ) 
	  flush_char();
}

/*
 * Flush the packet to disk, and reset the accumulator
 */
static void flush_char()
{
     if( a_count > 0 ) {
	  fputc( a_count, g_outfile );
	  fwrite( accum, 1, a_count, g_outfile );
	  a_count = 0;
     }
}	

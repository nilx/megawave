/*
 * jpeg_io.c
 *
 * part of this code is from XV
 * (C) 1989, 1994 by John Bradley,
 * and from the IJG JPEG (Independent JPEG Group) distribution
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* FIXME : avoid */
#include <setjmp.h>

#include "libmw-defs.h"
#include "mw.h"

#include "cimage.h"
#include "ccimage.h"
#include "basic_conv.h"
/* FIXME : use standard headers */
#include "jpeglib.h"

#include "jpeg_io.h"

/*-------- JPEG defined -------*/
 
typedef unsigned char byte;

struct my_error_mgr {
     struct jpeg_error_mgr pub;
     jmp_buf               setjmp_buffer;
};
 
typedef struct my_error_mgr *my_error_ptr;

#undef PARM
#define PARM(a) a

static void         mw_error_exit      PARM((j_common_ptr));
static void         mw_error_output    PARM((j_common_ptr));
static boolean      mw_process_comment PARM((j_decompress_ptr));

/* ----- Local variables ----- */

static char *comment;
static char *lfname;

/* ----- Local functions ----- */

/* This function is called when a fatal jpeglib error is detected :
   call mw_error_output and resume execution 
*/

static void mw_error_exit(j_common_ptr cinfo) 
{
     my_error_ptr myerr;

     myerr = (my_error_ptr) cinfo->err;
     /* Display error message and resume */
     (*cinfo->err->output_message)(cinfo);     
}

/* Print jpeglib error message */
static void mw_error_output(j_common_ptr cinfo) 
{
     my_error_ptr myerr;
     char         buffer[JMSG_LENGTH_MAX];

     myerr = (my_error_ptr) cinfo->err;
     (*cinfo->err->format_message)(cinfo, buffer);

     mwerror(ERROR,0,"JPEG image file \"%s\" : %s... Canceling I/O operation !\n", 
	     lfname, buffer); 
}

static unsigned int j_getc(j_decompress_ptr cinfo)
{
     struct jpeg_source_mgr *datasrc = cinfo->src;
  
     if (datasrc->bytes_in_buffer == 0)
     {
	  if (! (*datasrc->fill_input_buffer) (cinfo))
	       mwerror(INTERNAL,0,"[j_getc (jpeg_io.c)] : Cannot resume from this kind of JPEG lib error !\n");
     }
     datasrc->bytes_in_buffer--;
     return GETJOCTET(*datasrc->next_input_byte++);
}

static boolean mw_process_comment(j_decompress_ptr cinfo)
{
     int          length, hasnull;
     unsigned int ch;
     char         *oldsp, *sp;

     length  = j_getc(cinfo) << 8;
     length += j_getc(cinfo);
     length -= 2;                  /* discount the length word itself */

     if (!comment) {
	  comment = (char *) malloc((size_t) length + 1);
	  if (comment) comment[0] = '\0';
     }
     else comment = (char *) realloc(comment, strlen(comment) + length + 1);
     if (!comment) mwerror(FATAL,1,"Out of memory in mw_process_comment");
  
     oldsp = sp = comment + strlen(comment);
     hasnull = 0;

     while (length-- > 0) {
	  ch = j_getc(cinfo);
	  *sp++ = (char) ch;
	  if (ch == 0) hasnull = 1;
     }

     if (hasnull) sp = oldsp;       /* swallow comment blocks that have nulls */
     *sp++ = '\0';

     return TRUE;
}


/* Load JPEG/JFIF format into a Cimage */

Cimage _mw_cimage_load_jpeg(char * fname)
{
     Cimage image;
     struct jpeg_decompress_struct    cinfo;
     struct my_error_mgr              jerr;
     JSAMPROW                         rowptr[1];
     FILE                            *fp;
     static byte                     *pic;
     int                              w,h,bperpix;


     pic       = (byte *) NULL;
     comment   = (char *) NULL;
     lfname=fname;

     if ((fp = fopen(fname, "r")) == NULL)
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  return(NULL);
     }
  
     /* Initialize the JPEG decompression object with personal error handling. */
     cinfo.err = jpeg_std_error(&jerr.pub);
     jerr.pub.error_exit     = mw_error_exit;
     jerr.pub.output_message = mw_error_output;

     if (setjmp(jerr.setjmp_buffer)==1) 
	  /* What to do in case of error detected in jpeg library */
     {
	  jpeg_destroy_decompress(&cinfo);
	  fclose(fp);
	  return(NULL);
     }
  
     /* Allocate and initialize a JPEG decompression object */
     jpeg_create_decompress(&cinfo);

     /* Insert custom COM marker processor. */
     jpeg_set_marker_processor(&cinfo, JPEG_COM, mw_process_comment);

     /* Specify data source for decompression */
     jpeg_stdio_src(&cinfo, fp);

     /* Read file header, set default decompression parameters */
     (void) jpeg_read_header(&cinfo, TRUE);

     jpeg_calc_output_dimensions(&cinfo);
     w = cinfo.output_width;
     h = cinfo.output_height;
     bperpix = cinfo.output_components;
  
     if (cinfo.jpeg_color_space != JCS_GRAYSCALE) 
	  mwerror(INTERNAL,1,"[_mw_cimage_load_jpeg] Color JPEG image file \"%s\" should not be loaded by this function !\n",fname);

     if (bperpix!=1) 
	  mwerror(INTERNAL,1,"[_mw_cimage_load_jpeg] JPEG image file \"%s\" with %d planes should not be loaded by this function !\n",fname,bperpix);

     /* Ask the decoded image to be in Grayscale color space */
     cinfo.out_color_space = JCS_GRAYSCALE;
     cinfo.quantize_colors = FALSE;
     jpeg_calc_output_dimensions(&cinfo);  
    
     image = mw_change_cimage(NULL,h,w);
     if (image == NULL)
     {
	  mwerror(ERROR,0,"Not enough memory to load JPEG image file \"%s\" !\n",
		  fname);
	  jpeg_destroy_decompress(&cinfo);
	  fclose(fp);
	  if (comment) free(comment);
	  return(NULL);
     }
     pic = (byte *) image->gray;
  
     /* Start decompressor */
     jpeg_start_decompress(&cinfo);

     /* Process data */
     while (cinfo.output_scanline < cinfo.output_height) {
	  rowptr[0] = (JSAMPROW) &pic[cinfo.output_scanline * w * bperpix];
	  (void) jpeg_read_scanlines(&cinfo, rowptr, (JDIMENSION) 1);
     }

     /* Finish decompression and release memory.*/
     jpeg_finish_decompress(&cinfo);
     jpeg_destroy_decompress(&cinfo);
     fclose(fp);

     if (comment && (comment[0] != '\0')) strncpy(image->cmt,comment,mw_cmtsize);
     comment = (char *) NULL;
  
     return(image);
}

/* Load JPEG/JFIFC format into a Ccimage */

Ccimage _mw_ccimage_load_jpeg(char * fname)
{
     Ccimage image;
     struct jpeg_decompress_struct    cinfo;
     struct my_error_mgr              jerr;
     JSAMPROW                         rowptr[1];
     FILE                            *fp;
     static byte                     *pic;
     int                              w,h,bperpix;


     pic       = (byte *) NULL;
     comment   = (char *) NULL;
     lfname=fname;

     if ((fp = fopen(fname, "r")) == NULL)
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  return(NULL);
     }
  
     /* Initialize the JPEG decompression object with personal error handling. */
     cinfo.err = jpeg_std_error(&jerr.pub);
     jerr.pub.error_exit     = mw_error_exit;
     jerr.pub.output_message = mw_error_output;

     if (setjmp(jerr.setjmp_buffer)==1) 
	  /* What to do in case of error detected in jpeg library */
     {
	  jpeg_destroy_decompress(&cinfo);
	  fclose(fp);
	  return(NULL);
     }
  
     /* Allocate and initialize a JPEG decompression object */
     jpeg_create_decompress(&cinfo);

     /* Insert custom COM marker processor. */
     jpeg_set_marker_processor(&cinfo, JPEG_COM, mw_process_comment);

     /* Specify data source for decompression */
     jpeg_stdio_src(&cinfo, fp);

     /* Read file header, set default decompression parameters */
     (void) jpeg_read_header(&cinfo, TRUE);

     jpeg_calc_output_dimensions(&cinfo);
     w = cinfo.output_width;
     h = cinfo.output_height;
     bperpix = cinfo.output_components;
  
     if (cinfo.jpeg_color_space == JCS_GRAYSCALE) 
	  mwerror(INTERNAL,1,"[_mw_ccimage_load_jpeg] Grayscale JPEG image file \"%s\" should not be loaded by this function !\n",fname);

     if (bperpix!=3) 
	  mwerror(INTERNAL,1,"[_mw_ccimage_load_jpeg] JPEG image file \"%s\" with %d planes should not be loaded by this function !\n",fname,bperpix);
  
     /* Ask the decoded image to be in RGB color space */
     cinfo.out_color_space = JCS_RGB;
     cinfo.quantize_colors = FALSE;
     jpeg_calc_output_dimensions(&cinfo);  

     pic = (byte *) malloc((size_t) (w * h * bperpix));
     if (!pic) 
     {
	  mwerror(ERROR,0,"Not enough memory to load JPEG image file \"%s\" !\n",
		  fname);
	  jpeg_destroy_decompress(&cinfo);
	  fclose(fp);
	  if (comment) free(comment);
	  return(NULL);
     }
  
     /* Start decompressor */
     jpeg_start_decompress(&cinfo);

     /* Process data */
     while (cinfo.output_scanline < cinfo.output_height) {
	  rowptr[0] = (JSAMPROW) &pic[cinfo.output_scanline * w * bperpix];
	  (void) jpeg_read_scanlines(&cinfo, rowptr, (JDIMENSION) 1);
     }

     /* Finish decompression and release memory.*/
     jpeg_finish_decompress(&cinfo);
     jpeg_destroy_decompress(&cinfo);
     fclose(fp);

     image = mw_change_ccimage(NULL,h,w);
     if (image == NULL)
     {
	  mwerror(ERROR,0,"Not enough memory to load JPEG image file \"%s\" !\n",
		  fname);
	  free(pic);
	  return(NULL);
     }

     if (comment && (comment[0] != '\0')) strncpy(image->cmt,comment,mw_cmtsize);
     comment = (char *) NULL;
  
     _mw_1x24XV_to_3x8_ucharplanes(pic,image->red,image->green,image->blue,w*h*3);
     free(pic);

     return(image);
}

/* Write 8-bits gray level JPEG */

short _mw_cimage_create_jpeg(char * fname, Cimage image, char * Quality)
{
     FILE                            *fp;
     struct     jpeg_compress_struct cinfo;
     struct     my_error_mgr         jerr;
     JSAMPROW                        rowptr[1];
     int                             Q;
     /* FIXME : variable shadows, temporary dirty fix */
     char                            _comment[BUFSIZ];
     byte *pic;
  
     pic = NULL;

     if ((fp = fopen(fname, "w")) == NULL)
     {
	  mwerror(ERROR, 0,"Cannot open file \"%s\" for writting !\n",fname);
	  return(-1);
     }

     /* Initialize the JPEG decompression object with personal error handling. */
     cinfo.err = jpeg_std_error(&jerr.pub);
     jerr.pub.error_exit     = mw_error_exit;
     jerr.pub.output_message = mw_error_output;

     if (setjmp(jerr.setjmp_buffer)==1) 
	  /* What to do in case of error detected in jpeg library */
     {
	  jpeg_destroy_compress(&cinfo);
	  fclose(fp);
	  return(-2);
     }

     /* Allocate and initialize a JPEG compression object */  
     jpeg_create_compress(&cinfo);
     jpeg_stdio_dest(&cinfo, fp);

     cinfo.image_width  = image->ncol;
     cinfo.image_height = image->nrow;
     cinfo.input_components = 1;
     cinfo.in_color_space = JCS_GRAYSCALE;
  
     jpeg_set_defaults(&cinfo);
  
     /* Set Quality factor */
     if (Quality)
	  Q=atoi(Quality);
     else Q=100;
     if ((Q<=0)||(Q>100)) Q=100;
     jpeg_set_quality(&cinfo, Q, TRUE);
     mwerror(WARNING,1,"Image \"%s\" is loosely compressed (JPEG quality %d)\n",fname,Q);

     jpeg_start_compress(&cinfo, TRUE);

     /* Add comment */
     sprintf(_comment,"%s JPEG Quality %d", image->cmt,Q);
     jpeg_write_marker(&cinfo,JPEG_COM,(byte *) _comment,(unsigned int) strlen(_comment));

     /* Save the bitplanes */
     pic=image->gray;
     while (cinfo.next_scanline < cinfo.image_height) 
     {
	  rowptr[0] = (JSAMPROW) &pic[cinfo.next_scanline * image->ncol];
	  (void) jpeg_write_scanlines(&cinfo, rowptr, (JDIMENSION) 1);
     }
  
     jpeg_finish_compress(&cinfo);
     jpeg_destroy_compress(&cinfo);
     return(0);
}

/* Write 24-bits color JPEG CHAR */ 

short _mw_ccimage_create_jpeg(char * fname, Ccimage image, char * Quality)
{
     FILE                            *fp;
     struct     jpeg_compress_struct cinfo;
     struct     my_error_mgr         jerr;
     JSAMPROW                        rowptr[1];
     int                             Q;
     /* FIXME : variable shadows, temporary dirty fix */
     char                            _comment[BUFSIZ];
     byte *pic;
     unsigned int N;
  
     pic = NULL;

     if ((fp = fopen(fname, "w")) == NULL)
     {
	  mwerror(ERROR, 0,"Cannot open file \"%s\" for writting !\n",fname);
	  return(-1);
     }

     /* Initialize the JPEG decompression object with personal error handling. */
     cinfo.err = jpeg_std_error(&jerr.pub);
     jerr.pub.error_exit     = mw_error_exit;
     jerr.pub.output_message = mw_error_output;

     if (setjmp(jerr.setjmp_buffer)==1) 
	  /* What to do in case of error detected in jpeg library */
     {
	  jpeg_destroy_compress(&cinfo);
	  if (pic) free(pic);
	  fclose(fp);
	  return(-2);
     }

     /* Allocate and initialize a JPEG compression object */  
     jpeg_create_compress(&cinfo);
     jpeg_stdio_dest(&cinfo, fp);

     cinfo.image_width  = image->ncol;
     cinfo.image_height = image->nrow;
     cinfo.input_components = 3;
     cinfo.in_color_space = JCS_RGB;
  
     jpeg_set_defaults(&cinfo);
  
     /* Set Quality factor */
     if (Quality)
	  Q=atoi(Quality);
     else Q=100;
     if ((Q<=0)||(Q>100)) Q=100;
     jpeg_set_quality(&cinfo, Q, TRUE);
     mwerror(WARNING,1,"Image \"%s\" is loosely compressed (JPEG quality %d)\n",fname,Q);

     jpeg_start_compress(&cinfo, TRUE);

     /* Add comment */
     sprintf(_comment,"%s JPEG Quality %d",image->cmt,Q);
     jpeg_write_marker(&cinfo,JPEG_COM,(byte *) _comment,(unsigned int) strlen(_comment));

     /* Save the bitplanes */
     N = image->ncol * image->nrow * 3;
     pic = (byte *) malloc(N);
     if (!pic)
     {
	  mwerror(ERROR,0,"Not enough memory to create \"%s\"\n",fname);
	  jpeg_destroy_compress(&cinfo);
	  fclose(fp);      
	  return(-3);
     }
     _mw_3x8_to_1x24XV_ucharplanes(image->red,image->green,image->blue,pic,N);
     while (cinfo.next_scanline < cinfo.image_height) 
     {
	  rowptr[0] = (JSAMPROW) &pic[cinfo.next_scanline * image->ncol * 3];
	  (void) jpeg_write_scanlines(&cinfo, rowptr, (JDIMENSION) 1);
     }
  
     jpeg_finish_compress(&cinfo);
     jpeg_destroy_compress(&cinfo);
     free(pic);
     return(0);
}

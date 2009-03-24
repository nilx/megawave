/**
 * @file jpeg_io.c
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2009)
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <jpeglib.h>

#include "definitions.h"
#include "error.h"

#include "cimage.h"
#include "ccimage.h"

#include "jpeg_io.h"

/**
 * load a jpeg image into a RGB raster array,
 * for further use with _mw_xx_load_jpeg
 */
static JSAMPLE * _mw_jpeg_load_raster(JSAMPLE *raster,
				     JDIMENSION *width, JDIMENSION *height,
				     const char *fname)
{
    FILE * fp;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPLE *raster_ptr=NULL;
    JSAMPARRAY scanlines;
    JDIMENSION i=0;

    /* create the jpeg structure */
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    /* connect it to the input file */
    if (NULL == (fp = fopen(fname, "rb")))
	return NULL;
    jpeg_stdio_src(&cinfo, fp);

    /* collect image information */
    (void) jpeg_read_header(&cinfo, TRUE);
    *width = cinfo.image_width;
    *height = cinfo.image_height;

    /* allocate the raster storage */
    if ((double) INT_MAX < (double) *width
	|| (double) INT_MAX < (double) *height
	|| NULL == (raster = (JSAMPLE*) malloc(3 * *width * *height
					       * sizeof(JSAMPLE))))
    {
	fclose(fp);
	jpeg_abort_decompress(&cinfo);
	return NULL;
    }
    /* allocate the scan lines pointers */
    if (NULL == (scanlines = (JSAMPARRAY) malloc(*height * sizeof(JSAMPROW))))
    {
	free(raster);
	fclose(fp);
	jpeg_abort_decompress(&cinfo);
	return NULL;
    }
    /* fill the scan lines pointers */
    raster_ptr = raster;
    for (i = 0; i < *height; i++)
    {
	scanlines[i] = raster_ptr;
	raster_ptr += 3 * *width;
    }

    /* set decompression options */
    cinfo.out_color_space = JCS_RGB;
    cinfo.dct_method = JDCT_ISLOW;

    /* gather image raster data */
    (void) jpeg_start_decompress(&cinfo);
    while (cinfo.output_scanline < *height)
	if (1 > jpeg_read_scanlines(&cinfo,
				    scanlines + cinfo.output_scanline,
				    *height))
	{
	    free(raster);
	    fclose(fp);
	    jpeg_abort_decompress(&cinfo);
	    return NULL;
	}
    (void) jpeg_finish_decompress(&cinfo);
    fclose(fp);
    jpeg_destroy_decompress(&cinfo);

    return raster;
}

/**
 * load a jpeg image file to a Cimage struct
 */
/* FIXME : could be loaded directly into the Cimage */
Cimage _mw_cimage_load_jpeg(const char * fname)
{
    JSAMPLE *raster=NULL, *raster_ptr=NULL, *raster_end=NULL;
    JDIMENSION width=0, height=0;
    Cimage image=NULL;
    unsigned char *ptr=NULL;

    /* load the jpeg image into a RGB raster */
    if (NULL == (raster = _mw_jpeg_load_raster(raster, &width, &height, fname)))
	return NULL;

    /* allocate the final image structure */
    if (NULL == (image = mw_new_cimage())
	|| NULL == (image = mw_alloc_cimage(image, (int) height, (int) width)))
    {
	free(raster);
	return NULL;
    }

    /* load the raster into the gray planes */
    raster_ptr = raster;
    raster_end = raster_ptr + 3 * (size_t) width * (size_t) height;
    ptr = image->gray;
    while (raster_ptr < raster_end)
    {
	*ptr++ = (unsigned char) *raster_ptr;
	raster_ptr += 3;
    }

    /* free the raster and return the image */
    free(raster);

    return image;
}

/**
 * load a jpeg image file to a Ccimage struct
 */
Ccimage _mw_ccimage_load_jpeg(const char * fname)
{
    JSAMPLE *raster=NULL, *raster_ptr=NULL, *raster_end=NULL;
    JDIMENSION width=0, height=0;
    Ccimage image=NULL;
    unsigned char *ptrR=NULL, *ptrG=NULL, *ptrB=NULL;

    /* load the jpeg image into a RGB raster */
    if (NULL == (raster = _mw_jpeg_load_raster(raster, &width, &height, fname)))
	return NULL;

    /* allocate the final image structure */
    if (NULL == (image = mw_new_ccimage())
	|| NULL == (image = mw_alloc_ccimage(image, (int) height, (int) width)))
    {
	free(raster);
	return NULL;
    }

    /* load the raster into the color planes */
    raster_ptr = raster;
    raster_end = raster_ptr + 3 * (size_t) width * (size_t) height;
    ptrR = image->red;
    ptrG = image->green;
    ptrB = image->blue;
    while (raster_ptr < raster_end)
    {
	*ptrR++ = (unsigned char) *raster_ptr++;
	*ptrG++ = (unsigned char) *raster_ptr++;
	*ptrB++ = (unsigned char) *raster_ptr++;
    }

    /* free the raster and return the image */
    free(raster);

    return image;
}

/**
 * save a Cimage struct as a jpeg image file
 */
short _mw_cimage_create_jpeg(const char * fname, const Cimage image,
			     const char * quality)
{
    FILE * fp;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPLE *raster=NULL, *raster_ptr=NULL, *raster_end=NULL;
    JSAMPARRAY scanlines;
    unsigned char *ptr=NULL;
    JDIMENSION width=0, height=0, i=0;
    int iqual=100;

    /* ensure the image is correctly allocated */
    if ((NULL == image) || (NULL == image->gray)) 
	return -1;

    if (NULL != quality)
	iqual = atoi(quality);
    if (0 > iqual || 100 < iqual)
	return -1;
    /*
     * ensure the width and height are within the limits
     * and allocate the raster
     */
    if ((double) JPEG_MAX_DIMENSION < (double) image->ncol
	|| (double) JPEG_MAX_DIMENSION < (double) image->nrow
	|| NULL == (raster = (unsigned char *) 
		    malloc(image->nrow * image->ncol 
			   * sizeof(unsigned char))))
	return -1;

    width = (JDIMENSION) image->ncol;
    height = (JDIMENSION) image->nrow;

    /* convert the gray plane into the raster */
    ptr = image->gray;
    raster_ptr = raster;
    raster_end = raster_ptr + width * height;
    while (raster_ptr < raster_end)
	*raster_ptr++ = (JSAMPLE) *ptr++;

    /* allocate the scan lines pointers */
    if (NULL == (scanlines = (JSAMPARRAY) malloc(height * sizeof(JSAMPROW))))
    {
	free(raster);
	return -1;
    }
    /* fill the scan lines pointers */
    raster_ptr = raster;
    for (i = 0; i < height; i++)
    {
	scanlines[i] = raster_ptr;
	raster_ptr += width;
    }

    /* create the jpeg structure */
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    /* connect it to the output file */
    if (NULL == (fp = fopen(fname, "wb")))
	return -1;
    jpeg_stdio_dest(&cinfo, fp);

    /* set the compression parameters */
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;

    jpeg_set_defaults(&cinfo);
    /* set the compression quality */
    jpeg_set_quality(&cinfo, iqual, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height)
	(void) jpeg_write_scanlines(&cinfo, scanlines
				    + cinfo.next_scanline, 1);

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(fp);

    return 0;
}

/**
 * save a Ccimage struct as a jpeg image file
 */
short _mw_ccimage_create_jpeg(const char * fname, const Ccimage image,
			      const char * quality)
{
    FILE * fp;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPLE *raster=NULL, *raster_ptr=NULL, *raster_end=NULL;
    JSAMPARRAY scanlines;
    unsigned char *ptrR=NULL, *ptrG=NULL, *ptrB=NULL;
    JDIMENSION width=0, height=0, i=0;
    int iqual=100;

    /* ensure the image is correctly allocated */
    if ((NULL == image)
	|| (NULL == image->red) 
	|| (NULL == image->green) 
	|| (NULL == image->blue)) 
	return -1;

    if (NULL != quality)
	iqual = atoi(quality);
    if (0 > iqual || 100 < iqual)
	return -1;
    /*
     * ensure the width and height are within the limits
     * and allocate the raster
     */
    if ((double) JPEG_MAX_DIMENSION < (double) image->ncol
	|| (double) JPEG_MAX_DIMENSION < (double) image->nrow
	|| NULL == (raster = (unsigned char *) 
		    malloc(3 * image->nrow * image->ncol 
			   * sizeof(unsigned char))))
	return -1;

    width = (JDIMENSION) image->ncol;
    height = (JDIMENSION) image->nrow;

    /* convert the gray plane into the raster */
    ptrR = image->red;
    ptrG = image->green;
    ptrB = image->blue;
    raster_ptr = raster;
    raster_end = raster_ptr + 3 * width * height;
    while (raster_ptr < raster_end)
    {
	*raster_ptr++ = (JSAMPLE) *ptrR++;
	*raster_ptr++ = (JSAMPLE) *ptrG++;
	*raster_ptr++ = (JSAMPLE) *ptrB++;
    }

    /* allocate the scan lines pointers */
    if (NULL == (scanlines = (JSAMPARRAY) malloc(height * sizeof(JSAMPROW))))
    {
	free(raster);
	return -1;
    }
    /* fill the scan lines pointers */
    raster_ptr = raster;
    for (i = 0; i < height; i++)
    {
	scanlines[i] = raster_ptr;
	raster_ptr += 3 * width;
    }

    /* create the jpeg structure */
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    /* connect it to the output file */
    if (NULL == (fp = fopen(fname, "wb")))
	return -1;
    jpeg_stdio_dest(&cinfo, fp);

    /* set the compression parameters */
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    /* set the compression quality */
    jpeg_set_quality(&cinfo, iqual, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height)
	(void) jpeg_write_scanlines(&cinfo, scanlines
				    + cinfo.next_scanline, 1);

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(fp);

    return 0;
}

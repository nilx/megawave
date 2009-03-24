/**
 * @file tiff_io.c
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2009)
 */

#include <stdlib.h>
#include <limits.h>
#include <tiffio.h>

#include "definitions.h"

#include "cimage.h"
#include "ccimage.h"

#include "tiff_io.h"

/**
 * load a tiff image file to a Ccimage struct
 *
 * Loading the interlaced raster, then deinterlacing it uses
 * unnecessary memory, but this is the easy and safe way for the
 * moment.
 */
Ccimage _mw_ccimage_load_tiff(const char * fname)
{
    TIFF *tiffp=NULL;
    uint32 *raster=NULL, *raster_ptr=NULL, *raster_end=NULL;
    uint32 width=0, height=0;
    Ccimage image=NULL;
    unsigned char *ptrR=NULL, *ptrG=NULL, *ptrB=NULL;

    /* open the TIFF file and structure */
    if (NULL == (tiffp = TIFFOpen(fname, "r")))
	return NULL;

    /* read width and height and allocate the storage raster */
    if (1 != TIFFGetField(tiffp, TIFFTAG_IMAGEWIDTH, &width) 
	|| 1 != TIFFGetField(tiffp, TIFFTAG_IMAGELENGTH, &height)
	|| NULL == (raster = (uint32 *) malloc(width * height 
					       * sizeof(uint32))))
    {
	TIFFClose(tiffp);
	return NULL;
    }

    /* read the image into the raster */
    if (1 != TIFFReadRGBAImageOriented(tiffp, width, height, raster,
				       ORIENTATION_TOPLEFT, 1))
    {
	free(raster);
	TIFFClose(tiffp);
	return NULL;
    }

    /* close the TIFF file and structure */
    TIFFClose(tiffp);

    /*
     * ensure the width and height are within the limits (ccimage uses int)
     * and allocate the image
     */
    if (INT_MAX < width || INT_MAX < height
	|| NULL == (image = mw_new_ccimage())
	|| NULL == (image = mw_alloc_ccimage(image, (int) width, (int) height)))
    {
	free(raster);
	return NULL;
    }

    /* convert the RGBA raster mix into the red, greb, blue planes */
    raster_ptr = raster;
    raster_end = raster_ptr + (size_t) width * (size_t) height;
    ptrR = image->red;
    ptrG = image->green;
    ptrB = image->blue;
    while (raster_ptr < raster_end)
    {
	*ptrR++ = TIFFGetR(*raster_ptr);
	*ptrG++ = TIFFGetG(*raster_ptr);
	*ptrB++ = TIFFGetB(*raster_ptr++);
    }

    /* free the raster and return the image */
    free(raster);
    return image;
}  

/**
 * load a tiff image file to a Cimage struct
 *
 * Loading the interlaced raster, then deinterlacing it uses
 * unnecessary memory, but this is the easy and safe way for the
 * moment. We only read one channel, as all the channels are the same
 * for a greyscale image loaded as RGBA.
 *
 * @todo refactor with ccimage_load
 */
Cimage _mw_cimage_load_tiff(const char * fname)
{
    TIFF *tiffp=NULL;
    uint32 *raster=NULL, *raster_ptr=NULL, *raster_end=NULL;
    uint32 width=0, height=0;
    Cimage image=NULL;
    unsigned char *ptr=NULL;

    /* open the TIFF file and structure */
    if (NULL == (tiffp = TIFFOpen(fname, "r")))
	return NULL;

    /* read width and height and allocate the storage raster */
    if (1 != TIFFGetField(tiffp, TIFFTAG_IMAGEWIDTH, &width) 
	|| 1 != TIFFGetField(tiffp, TIFFTAG_IMAGELENGTH, &height)
	|| NULL == (raster = (uint32 *) malloc(width * height 
					       * sizeof(uint32))))
    {
	TIFFClose(tiffp);
	return NULL;
    }

    /* read the image into the raster */
    if (1 != TIFFReadRGBAImageOriented(tiffp, width, height, raster,
				       ORIENTATION_TOPLEFT, 1))
    {
	free(raster);
	TIFFClose(tiffp);
	return NULL;
    }

    /* close the TIFF file and structure */
    TIFFClose(tiffp);

    /*
     * ensure the width and height are within the limits (ccimage uses int)
     * and allocate the image
     */
    if (INT_MAX < width || INT_MAX < height
	|| NULL == (image = mw_new_cimage())
	|| NULL == (image = mw_alloc_cimage(image, (int) width, (int) height)))
    {
	free(raster);
	return NULL;
    }

    /* load the RGBA raster mix into the gray planes */
    raster_ptr = raster;
    raster_end = raster_ptr + (size_t) width * (size_t) height;
    ptr = image->gray;
    while (raster_ptr < raster_end)
	*ptr++ = TIFFGetR(*raster_ptr++);

    /* free the raster and return the image */
    free(raster);
    return image;
}  

/**
 * save a Ccimage struct as a tiff image file
 */
short _mw_ccimage_create_tiff(char * const fname, const Ccimage image)
{
    TIFF *tiffp=NULL;
    unsigned char *raster=NULL, *raster_ptr=NULL, *raster_end=NULL;
    unsigned char *ptrR=NULL, *ptrG=NULL, *ptrB=NULL;

    /* ensure the image is correctly allocated */
    if ((NULL == image) 
	|| (NULL == image->red) 
	|| (NULL == image->green) 
	|| (NULL == image->blue))
	return -1;

    /*
     * ensure the width and height are within the limits (tiff uses int32)
     * and allocate the raster
     */
    if (4294967295. < (float) image->ncol
	|| 4294967295. < (float) image->nrow
	|| NULL == (raster = (unsigned char *) 
		    malloc(3 * image->nrow * image->ncol 
			   * sizeof(unsigned char))))
	return -1;

    /* convert the red, greb, blue planes into the RGB mix raster*/
    ptrR = image->red;
    ptrG = image->green;
    ptrB = image->blue;
    raster_ptr = raster;
    raster_end = raster_ptr + 3 * image->ncol * image->nrow;
    while (raster_ptr < raster_end)
    {
	*raster_ptr++ = *ptrR++;
	*raster_ptr++ = *ptrG++;
	*raster_ptr++ = *ptrB++;
    }

    /* open the TIFF file and structure */
    if (NULL == (tiffp = TIFFOpen(fname, "w")))
    {
	free(raster);
	return -1;
    }

    /* insert tags into the TIFF structure */
    if (1 != TIFFSetField(tiffp, TIFFTAG_IMAGEWIDTH,
			  (uint32) image->ncol)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_IMAGELENGTH,
			     (uint32) image->nrow)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_ORIENTATION, 
			     ORIENTATION_TOPLEFT)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_SAMPLESPERPIXEL, 3)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_BITSPERSAMPLE,   8)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_PLANARCONFIG,
			     PLANARCONFIG_CONTIG)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_PHOTOMETRIC,
			     PHOTOMETRIC_RGB)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_ROWSPERSTRIP,
			     (uint32) image->nrow))
    {
	TIFFClose(tiffp);
	free(raster);
	return -1;
    }

    /* write the image as one single tile */
    if (3 * image->nrow * image->ncol
	!= TIFFWriteEncodedStrip(tiffp, (tstrip_t) 0, (tdata_t) raster, 
				 (tsize_t) 3 * image->nrow * image->ncol))
    {
	TIFFClose(tiffp);
	free(raster);
	return -1;
    }

    /* free the raster and tiff TIFF and structure, return success */
    free(raster);
    TIFFClose(tiffp);
    return 0;
}


/**
 * save a Cimage struct as a tiff image file
 *
 * @todo refactor with ccimage_create
 */
short _mw_cimage_create_tiff(const char * fname, const Cimage image)
{
    TIFF *tiffp=NULL;

    /* ensure the image is correctly allocated */
    if ((NULL == image) || (NULL == image->gray))
	return -1;

    /*
     * ensure the width and height are within the limits (tiff uses int32)
     * and open the TIFF file and structure
     */
    if (4294967295. < (float) image->ncol
	|| 4294967295. < (float) image->nrow
	|| NULL == (tiffp = TIFFOpen(fname, "w")))
	return -1;

    /* insert tags into the TIFF structure */
    if (1 != TIFFSetField(tiffp, TIFFTAG_IMAGEWIDTH,
			  (uint32) image->ncol)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_IMAGELENGTH,
			     (uint32) image->nrow)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_ORIENTATION, 
			     ORIENTATION_TOPLEFT)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_SAMPLESPERPIXEL, 1)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_BITSPERSAMPLE,   8)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_PLANARCONFIG,
			     PLANARCONFIG_CONTIG)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_PHOTOMETRIC,
			     PHOTOMETRIC_MINISBLACK)
	|| 1 != TIFFSetField(tiffp, TIFFTAG_ROWSPERSTRIP,
			     (uint32) image->nrow))
    {
	TIFFClose(tiffp);
	return -1;
    }

    /* write the image as one single tile */
    if (image->nrow * image->ncol
	!= TIFFWriteEncodedStrip(tiffp, (tstrip_t) 0, (tdata_t) image->gray, 
				 (tsize_t) image->nrow * image->ncol))
    {
	TIFFClose(tiffp);
	return -1;
    }

    /* free the raster and tiff TIFF and structure, return success */
    TIFFClose(tiffp);
    return 0;
}

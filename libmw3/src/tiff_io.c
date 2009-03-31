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
 * load a tiff image into a RGBA raster array,
 * for further use with _mw_xx_load_tiff
 */
static uint32 *_mw_tiff_load_raster(uint32 * raster,
                                    uint32 * width, uint32 * height,
                                    const char *fname)
{
    TIFF *tiffp = NULL;

    /* any non-allocated pointer should be NULL */
    if (NULL != raster)
        return NULL;

    /* open the TIFF file and structure */
    if (NULL == (tiffp = TIFFOpen(fname, "r")))
        return NULL;

    /* read width and height and allocate the storage raster */
    if (1 != TIFFGetField(tiffp, TIFFTAG_IMAGEWIDTH, width)
        || 1 != TIFFGetField(tiffp, TIFFTAG_IMAGELENGTH, height)
        || (double) INT_MAX < (double) *width
        || (double) INT_MAX < (double) *height
        || NULL == (raster = (uint32 *) malloc(*width * *height
                                               * sizeof(uint32))))
    {
        TIFFClose(tiffp);
        return NULL;
    }

    /* read the image into the raster */
    if (1 != TIFFReadRGBAImageOriented(tiffp, *width, *height, raster,
                                       ORIENTATION_TOPLEFT, 1))
    {
        free(raster);
        TIFFClose(tiffp);
        return NULL;
    }

    /* close the TIFF file and structure */
    TIFFClose(tiffp);

    return raster;
}

/**
 * load a tiff image file to a Ccimage struct
 *
 * Loading the interlaced raster, then deinterlacing it uses
 * unnecessary memory, but this is the easy and safe way for the
 * moment.
 */
Ccimage _mw_ccimage_load_tiff(const char *fname)
{
    uint32 *raster = NULL, *raster_ptr = NULL, *raster_end = NULL;
    uint32 width = 0, height = 0;
    Ccimage image = NULL;
    unsigned char *ptrR = NULL, *ptrG = NULL, *ptrB = NULL;

    /* load the TIFF into a RGBA raster */
    if (NULL ==
        (raster = _mw_tiff_load_raster(raster, &width, &height, fname)))
        return NULL;

    /*
     * ensure the width and height are within the limits (ccimage uses int)
     * and allocate the image
     */
    if (NULL == (image = mw_new_ccimage())
        || NULL == (image =
                    mw_alloc_ccimage(image, (int) height, (int) width)))
    {
        free(raster);
        return NULL;
    }

    /* convert the RGBA raster mix into the red, greb, blue planes */
    raster_ptr = raster;
    raster_end = raster_ptr + (size_t) width *(size_t) height;
    ptrR = image->red;
    ptrG = image->green;
    ptrB = image->blue;
    while (raster_ptr < raster_end)
    {
        *ptrR++ = (unsigned char) TIFFGetR(*raster_ptr);
        *ptrG++ = (unsigned char) TIFFGetG(*raster_ptr);
        *ptrB++ = (unsigned char) TIFFGetB(*raster_ptr++);
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
 */
Cimage _mw_cimage_load_tiff(const char *fname)
{
    uint32 *raster = NULL, *raster_ptr = NULL, *raster_end = NULL;
    uint32 width = 0, height = 0;
    Cimage image = NULL;
    unsigned char *ptr = NULL;

    /* load the TIFF into a RGBA raster */
    if (NULL ==
        (raster = _mw_tiff_load_raster(raster, &width, &height, fname)))
        return NULL;

    /*
     * ensure the width and height are within the limits (ccimage uses int)
     * and allocate the image
     */
    if (NULL == (image = mw_new_cimage())
        || NULL == (image =
                    mw_alloc_cimage(image, (int) height, (int) width)))
    {
        free(raster);
        return NULL;
    }

    /* load the first raster channel mix into the gray planes */
    raster_ptr = raster;
    raster_end = raster_ptr + (size_t) width *(size_t) height;
    ptr = image->gray;
    while (raster_ptr < raster_end)
        *ptr++ = (unsigned char) TIFFGetR(*raster_ptr++);

    /* free the raster and return the image */
    free(raster);
    return image;
}

/**
 * setup a TIFF structure, for further use with _mw_xx_create_tiff
 */
static TIFF *_mw_tiff_setup(uint32 width, uint32 height, const char *fname)
{
    TIFF *tiffp = NULL;

    /* open the TIFF file and structure */
    if (NULL == (tiffp = TIFFOpen(fname, "w")))
        return NULL;

    /* insert tags into the TIFF structure */
    if (1 != TIFFSetField(tiffp, TIFFTAG_IMAGEWIDTH, width)
        || 1 != TIFFSetField(tiffp, TIFFTAG_IMAGELENGTH, height)
        || 1 != TIFFSetField(tiffp, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT)
        || 1 != TIFFSetField(tiffp, TIFFTAG_BITSPERSAMPLE, 8)
        || 1 != TIFFSetField(tiffp, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG)
        || 1 != TIFFSetField(tiffp, TIFFTAG_ROWSPERSTRIP, height))
    {
        TIFFClose(tiffp);
        return NULL;
    }

    return tiffp;
}

/**
 * save a Ccimage struct as a tiff image file
 */
short _mw_ccimage_create_tiff(char *const fname, const Ccimage image)
{
    TIFF *tiffp = NULL;
    unsigned char *raster = NULL, *raster_ptr = NULL, *raster_end = NULL;
    unsigned char *ptrR = NULL, *ptrG = NULL, *ptrB = NULL;

    /* ensure the image is correctly allocated */
    if ((NULL == image)
        || (NULL == image->red)
        || (NULL == image->green) || (NULL == image->blue))
        return -1;

    /*
     * ensure the width and height are within the limits (tiff uses int32)
     * and allocate the raster
     */
    if (4294967295. < (double) image->ncol
        || 4294967295. < (double) image->nrow
        || NULL == (raster = (unsigned char *)
                    malloc(3 * image->nrow * image->ncol
                           * sizeof(unsigned char))))
        return -1;

    /* convert the red, greb, blue planes into the RGB mix raster */
    ptrR = image->red;
    ptrG = image->green;
    ptrB = image->blue;
    raster_ptr = raster;
    raster_end = raster_ptr + 3 * image->ncol * image->nrow;
    while (raster_ptr < raster_end)
    {
        *raster_ptr++ = (uint32) * ptrR++;
        *raster_ptr++ = (uint32) * ptrG++;
        *raster_ptr++ = (uint32) * ptrB++;
    }

    /* open and setup the TIFF structure */
    if (NULL == (tiffp = _mw_tiff_setup((uint32) image->ncol,
                                        (uint32) image->nrow, fname)))
    {
        free(raster);
        return -1;
    }

    /* insert specific tags into the TIFF structure */
    if (1 != TIFFSetField(tiffp, TIFFTAG_SAMPLESPERPIXEL, 3)
        || 1 != TIFFSetField(tiffp, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB))
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

    /* free the raster and TIFF structure, return success */
    TIFFClose(tiffp);
    free(raster);
    return 0;
}

/**
 * save a Cimage struct as a tiff image file
 */
short _mw_cimage_create_tiff(const char *fname, const Cimage image)
{
    TIFF *tiffp = NULL;

    /* ensure the image is correctly allocated */
    if ((NULL == image) || (NULL == image->gray))
        return -1;

    /*
     * ensure the width and height are within the limits
     * (tiff uses int32, 2^32 - 1 = 4294967295)
     * and open the TIFF file and structure
     */
    if (4294967295. < (double) image->ncol
        || 4294967295. < (double) image->nrow)
        return -1;

    /* open and setup the TIFF structure */
    if (NULL == (tiffp = _mw_tiff_setup((uint32) image->ncol,
                                        (uint32) image->nrow, fname)))
        return -1;

    /* insert specific tags into the TIFF structure */
    if (1 != TIFFSetField(tiffp, TIFFTAG_SAMPLESPERPIXEL, 1)
        || 1 != TIFFSetField(tiffp, TIFFTAG_PHOTOMETRIC,
                             PHOTOMETRIC_MINISBLACK))
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

    /* free the TIFF and structure, return success */
    TIFFClose(tiffp);
    return 0;
}

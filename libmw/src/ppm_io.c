/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ppm_io.c

  Vers. 1.0
  (C)2002 Jacques Froment
  load/save functions for the PPM (portable pixmap file) format
  Note : only the "raw" PPM format is supported, the "plain" (ascii) one
  being definitely too wasteful of space to record color images.
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "definitions.h"
#include "error.h"

#include "ppmr_io.h"
#include "pgm_io.h"
#include "ccimage.h"
#include "basic_conv.h"

#include "ppm_io.h"

/*~~~~~~ Load 24-bits color PPM Raw file ~~~~~*/

Ccimage _mw_ccimage_load_ppmr(char *file)
{
    FILE *fp;
    Ccimage image;
    int ncol, nrow;
    int maxgl;
    long size;
    char comment[mw_cmtsize];
    unsigned char *pic;

    if (!(fp = fopen(file, "r")))
    {
        mwerror(ERROR, 0, "File \"%s\" not found or unreadable\n", file);
        fclose(fp);

        return (NULL);
    }

    /* Check the header (P6) */
    if ((getc(fp) != 'P') || (getc(fp) != '6'))
        mwerror(INTERNAL, 1,
                "[_mw_ccimage_load_ppmr] Error while reading file \"%s\": "
                "not a PPM format !\n", file);

    comment[0] = '\0';
    ncol = _mw_pgm_get_next_item(fp, comment);
    nrow = _mw_pgm_get_next_item(fp, comment);

    maxgl = _mw_pgm_get_next_item(fp, comment);
    if ((ncol <= 0) || (nrow <= 0) || (maxgl <= 0) || (maxgl > 255))
    {
        mwerror(ERROR, 0, "Error while reading file \"%s\": "
                "bad PPM format !\n", file);
        fclose(fp);
        return (NULL);
    }

    size = 3 * ncol * nrow;
    image = mw_change_ccimage(NULL, nrow, ncol);
    pic = (unsigned char *) malloc(size);
    if ((!image) || (!pic))
        mwerror(FATAL, 0, "Not enough memory to load the image \"%s\"\n",
                file);

    /* FIXME: wrong types, dirty temporary fix */
    if (fread(pic, 1, size, fp) != (unsigned int) size)
    {
        mwerror(ERROR, 0,
                "Error while reading PPM image \"%s\": "
                "unexpected end of file !\n", file);
        fclose(fp);
        free(pic);
        mw_delete_ccimage(image);
        return (NULL);
    }
    fclose(fp);
    _mw_1x24XV_to_3x8_ucharplanes(pic, image->red, image->green, image->blue,
                                  size);
    free(pic);

    if (strlen(comment) > 0)
        strcpy(image->cmt, comment);

    return (image);
}

/*~~~~~ Create 24-bits color PPM Raw file ~~~~~*/

short _mw_ccimage_create_ppmr(char *file, Ccimage image)
{
    FILE *fp;
    long size;
    unsigned char *pic;

    if (!(fp = fopen(file, "w")))
    {
        mwerror(ERROR, 0, "Cannot create the file \"%s\"\n", file);
        return (-1);
    }

    if ((image == NULL) || (image->red == NULL) ||
        (image->green == NULL) || (image->blue == NULL))
    {
        mwerror(INTERNAL, 0,
                "[_mw_ccimage_create_ppmr] NULL image or image planes\n");
        return (-2);
    }

    fprintf(fp, "P6\n");
    if ((strlen(image->cmt) > 0) && (image->cmt[0] != '?'))
        fprintf(fp, "# %s\n", image->cmt);
    fprintf(fp, "%d\n", image->ncol);
    fprintf(fp, "%d\n", image->nrow);
    fprintf(fp, "255\n");

    size = image->ncol * image->nrow * 3;

    pic = (unsigned char *) malloc(size);
    if (!pic)
    {
        mwerror(ERROR, 0, "Not enough memory to create \"%s\"\n", file);
        fclose(fp);
        return (-3);
    }
    _mw_3x8_to_1x24XV_ucharplanes(image->red, image->green, image->blue, pic,
                                  size);
    if ((size_t) size > fwrite(pic, 1, size, fp))
    {
        fprintf(stderr, "error while writing to disk");
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    free(pic);
    return (0);
}

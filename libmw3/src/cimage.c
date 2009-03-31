/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  cimage.c

  Vers. 1.5
  Author : Jacques Froment
  Basic memory routines for the cimage internal type

  Main changes :
  v1.5 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <string.h>

#include "definitions.h"
#include "error.h"

#include "cimage.h"

/* creates a new cimage structure */

Cimage mw_new_cimage()
{
    Cimage image;

    if (!(image = (Cimage) (malloc(sizeof(struct cimage)))))
    {
        mwerror(ERROR, 0, "[mw_new_cimage] Not enough memory\n");
        return (NULL);
    }

    image->nrow = image->ncol = 0;
    image->allocsize = 0;
    image->firstcol = image->lastcol = image->firstrow = image->lastrow = 0.0;

    image->scale = 1.0;
    strcpy(image->cmt, "?");
    strcpy(image->name, "?");

    image->gray = NULL;

    image->previous = NULL;
    image->next = NULL;

    return (image);
}

/* allocates the gray array */

Cimage mw_alloc_cimage(Cimage image, int nrow, int ncol)
{
    int size;

    if (image == NULL)
    {
        mwerror(ERROR, 0,
                "[mw_alloc_cimage] cannot alloc plane : "
                "cimage structure is NULL\n");
        return (NULL);
    }

    size = nrow * ncol * sizeof(unsigned char);
    if (size <= 0)
    {
        mwerror(ERROR, 0,
                "[mw_alloc_cimage] Attempts to alloc a cimage "
                "with null size\n");
        return (NULL);
    }

    if (image->gray != NULL)
    {
        mwerror(ERROR, 0,
                "[mw_alloc_cimage] Attempts to alloc a cimage "
                "which is already allocated\n");
        return (NULL);
    }

    image->gray = (unsigned char *) malloc(size);
    if (image->gray == NULL)
    {
        image->nrow = image->ncol = 0;
        image->allocsize = 0;
        mwerror(ERROR, 0, "[mw_alloc_cimage] Not enough memory\n");
        return (NULL);
    }

    image->nrow = nrow;
    image->ncol = ncol;
    image->allocsize = size;
    return (image);
}

/* desallocate the array in the cimage structure and the structure itself */

void mw_delete_cimage(Cimage image)
{
    if (image == NULL)
    {
        mwerror(ERROR, 0,
                "[mw_delete_cimage] cannot delete : "
                "cimage structure is NULL\n");
        return;
    }
    if (image->gray != NULL)
        free(image->gray);
    image->gray = NULL;
    free(image);
    image = NULL;
}

/* Change the size of the allocated gray plane */
/* May define the struct if not defined */
/* So you have to call it with image = mw_change_cimage(image,...) */

Cimage mw_change_cimage(Cimage image, int nrow, int ncol)
{
    int size;

    if (image == NULL)
        image = mw_new_cimage();
    if (image == NULL)
        return (NULL);

    size = nrow * ncol * sizeof(unsigned char);
    if (size > image->allocsize)
    {
        if (image->gray != NULL)
        {
            free(image->gray);
            image->gray = NULL;
        }
        if (mw_alloc_cimage(image, nrow, ncol) == NULL)
        {
            mw_delete_cimage(image);
            return (NULL);
        }
    }
    else
    {
        image->nrow = nrow;
        image->ncol = ncol;
    }
    return (image);
}

/* Return the gray level value of a cimage at location (x,y) */
/* WARNING: this is a slow way to access to a pixel !        */

unsigned char mw_getdot_cimage(Cimage image, int x, int y)
{
    if ((!image) || (!image->gray))
    {
        mwerror(ERROR, 0,
                "[mw_getdot_cimage] NULL image struct "
                "or NULL gray plane... Return 0\n");
        return (0);

    }

    if ((x < 0) || (y < 0) || (x >= image->ncol) || (y >= image->nrow))
    {
        mwerror(ERROR, 0,
                "[mw_getdot_cimage] Point (%d,%d) out of image... Return 0\n",
                x, y);
        return (0);
    }

    return (image->gray[y * image->ncol + x]);
}

/* Set the gray level value of a cimage at location (x,y) */
/* WARNING: this is a slow way to access to a pixel !     */

void mw_plot_cimage(Cimage image, int x, int y, unsigned char v)
{
    if ((!image) || (!image->gray))
    {
        mwerror(ERROR, 0,
                "[mw_plot_cimage] NULL image struct or NULL gray plane\n");
        return;
    }

    if ((x < 0) || (y < 0) || (x >= image->ncol) || (y >= image->nrow))
    {
        mwerror(ERROR, 0,
                "[mw_plot_cimage] Point (%d,%d) out of image\n", x, y);
        return;
    }

    image->gray[y * image->ncol + x] = v;
}

/* Draw a connex line with gray level c between (a0,b0) and (a1,b1) */

void mw_draw_cimage(Cimage image, int a0, int b0, int a1, int b1,
                    unsigned char c)
{
    int bdx, bdy;
    int sx, sy, dx, dy, x, y, z;

    if ((!image) || (!image->gray))
    {
        mwerror(ERROR, 0,
                "[mw_draw_cimage] NULL image struct or NULL gray plane\n");
        return;
    }

    bdx = image->ncol;
    bdy = image->nrow;

    if (a0 < 0)
        a0 = 0;
    else if (a0 >= bdx)
        a0 = bdx - 1;
    if (a1 < 0)
    {
        a1 = 0;
        /*
         * if (a0 == 0) mwerror(ERROR, 0,
         * "[mw_draw_cimage] Illegal parameters (%d,%d)-(%d,%d)\n",
         * a0,b0,a1,b1);
         */
    }
    else if (a1 >= bdx)
    {
        a1 = bdx - 1;
        /*
         * if (a0==bdx-1) mwerror(ERROR, 0,
         * "[mw_draw_cimage] Illegal parameters (%d,%d)-(%d,%d)\n",
         * a0,b0,a1,b1);
         */
    }
    if (b0 < 0)
        b0 = 0;
    else if (b0 >= bdy)
        b0 = bdy - 1;
    if (b1 < 0)
    {
        b1 = 0;
        /*
         * if (b0==0)
         * mwerror(ERROR, 0,
         * "[mw_draw_cimage] Illegal parameters (%d,%d)-(%d,%d)\n",
         * a0,b0,a1,b1);
         */
    }
    else if (b1 >= bdy)
    {
        b1 = bdy - 1;
        /*
         * if (b0==bdy-1)
         * mwerror(ERROR, 0,
         * "[mw_draw_cimage] Illegal parameters (%d,%d)-(%d,%d)\n",
         * a0,b0,a1,b1);
         */
    }

    if (a0 < a1)
    {
        sx = 1;
        dx = a1 - a0;
    }
    else
    {
        sx = -1;
        dx = a0 - a1;
    }
    if (b0 < b1)
    {
        sy = 1;
        dy = b1 - b0;
    }
    else
    {
        sy = -1;
        dy = b0 - b1;
    }
    x = 0;
    y = 0;

    if (dx >= dy)
    {
        z = (-dx) / 2;
        while (abs(x) <= dx)
        {
            image->gray[(y + b0) * bdx + x + a0] = c;
            x += sx;
            z += dy;
            if (z > 0)
            {
                y += sy;
                z -= dx;
            }
        }
    }
    else
    {
        z = (-dy) / 2;
        while (abs(y) <= dy)
        {
            image->gray[(y + b0) * bdx + x + a0] = c;
            y += sy;
            z += dx;
            if (z > 0)
            {
                x += sx;
                z -= dy;
            }
        }
    }
}

/* Clear the gray plane of a cimage with the value v */

void mw_clear_cimage(Cimage image, unsigned char v)
{
    if ((!image) || (!image->gray))
    {
        mwerror(ERROR, 0,
                "[mw_clear_cimage] NULL image struct or NULL gray plane\n");
        return;
    }
    memset(image->gray, v, image->ncol * image->nrow);
}

/* Copy the gray plane of a cimage into another cimage */

void mw_copy_cimage(Cimage in, Cimage out)
{
    if ((!in) || (!out) || (!in->gray) || (!out->gray)
        || (in->ncol != out->ncol) || (in->nrow != out->nrow))
    {
        mwerror(ERROR, 0,
                "[mw_copy_cimage] NULL input or output image "
                "or images of different sizes\n");
        return;
    }

    strcpy(out->cmt, in->cmt);
    memcpy(out->gray, in->gray, in->ncol * in->nrow);
}

/* Return a tab T so that T[i][j] = image->gray[i*image->ncol+j] */

unsigned char **mw_newtab_gray_cimage(Cimage image)
{
    unsigned char **im;
    register unsigned long l;

    if ((!image) || (!image->gray))
    {
        mwerror(ERROR, 0,
                "[mw_newtab_gray_cimage] NULL image struct "
                "or NULL gray plane\n");
        return (NULL);
    }

    im = (unsigned char **) malloc(image->nrow * sizeof(unsigned char *));
    if (im == NULL)
    {
        mwerror(ERROR, 0, "[mw_newtab_gray_cimage] Not enough memory\n");
        return (NULL);
    }

    im[0] = image->gray;
    /* FIXME: wrong types, dirty temporary fix */
    for (l = 1; l < (unsigned long) image->nrow; l++)
        im[l] = im[l - 1] + image->ncol;

    return (im);
}

/* Return 0 is this is not a binary image, a value > 0 if it is one.
   The value > 0 corresponds to the maxima value (the unique value != 0)
*/

unsigned char mw_isitbinary_cimage(Cimage image)
{
    unsigned char *p;
    int l, min, max;

    if ((!image) || (!image->gray))
    {
        mwerror(ERROR, 0,
                "[mw_isitbinary_cimage] NULL image struct "
                "or NULL gray plane\n");
        return -1;
    }

    min = 1000;
    max = -1;
    for (l = 0, p = image->gray; l < image->ncol * image->nrow; l++, p++)
    {
        if (min > *p)
            min = *p;
        if (max < *p)
            max = *p;
    }
    if (min > 0)
        return (0);
    /* FIXME: wrong types, dirty temporary fix */
    for (l = 0, p = image->gray; l < image->ncol * image->nrow; l++, p++)
        if ((*p != min) && (*p != max))
            return (0);
    return (max);
}

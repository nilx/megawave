/**
 * @file wpack2d_io.c
 *
 * @version 1.0
 * @author Adrien Costagliola (2007)
 * @author David Serre (2007)
 * @author Francois Malgouyres (2007)
 * @author Jacques Froment (2007)
 * @author Nicolas Limare (2008 - 2009)
 *
 * input/output private functions for the Wpack2d structure
 */

#include <stdio.h>
#include <string.h>

#include "definitions.h"
#include "error.h"

#include "ascii_file.h"
#include "wpack2d.h"
#include "mwio.h"
#include "cimage.h"
#include "cimage_io.h"
#include "fsignal.h"
#include "fimage.h"
#include "fimage_io.h"
#include "file_type.h"
#include "type_conv.h"

#include "wpack2d_io.h"

extern int _mw_convert_struct_warning;

/*~~~~~~ Load Wpack2d using ASCII Format ~~~~~~*/

Wpack2d _mw_load_wpack2d_ascii(char *fname)
{
    FILE *fp, *fp2;
    Wpack2d pack = NULL;
    int fformat = 0;
    char buffer[BUFSIZ];
    int i, j, ret;
    char pn[BUFSIZ];            /* pathname of filenames to be loaded */
    char fn[BUFSIZ];            /* filenames */
    char ns1[mw_namesize], ns2[mw_namesize], nim[mw_namesize];
    Fsignal signal1, signal2;
    Cimage tree;
    char type[mw_ftype_size];
    char comment[mw_cmtsize];

    fp = _mw_open_data_ascii_file(fname);
    if (fp == NULL)
        return (NULL);

    if (_mw_fascii_search_string(fp, "def Wpack2d\n") == EOF)
    {
        /* Try old keyword Fpack */
        fclose(fp);
        fp = _mw_open_data_ascii_file(fname);
        if (fp == NULL)
            return (NULL);
        if (_mw_fascii_search_string(fp, "def Fpack\n") == EOF)
        {
            mwerror(ERROR, 0,
                    "No Wpack2d description found in the file \"%s\"\n",
                    fname);
            fclose(fp);
            return (NULL);
        }
        fformat = 1;
    }

    pack = mw_new_wpack2d();
    if (!pack)
    {
        fclose(fp);
        return (NULL);
    }

    if (fformat == 0)
    {
        if ((_mw_fascii_get_field(fp, fname,
                                  "name:", "%[^\n]", pack->name) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "comment:", "%[^\n]", pack->cmt) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "signal1:", "%[^\n]", ns1) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "signal2:", "%[^\n]", ns2) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "tree:", "%[^\n]", nim) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "img_ncol:", "%d\n",
                                     &pack->img_ncol) != 1)
            ||
            (_mw_fascii_get_field
             (fp, fname, "img_nrow:", "%d\n", &pack->img_nrow) != 1)
            || (_mw_fascii_get_field(fp, fname, "previous:", "%[^\n]", buffer)
                != 1)
            || (_mw_fascii_get_field(fp, fname, "next:", "%[^\n]", buffer) !=
                1))
        {
            mwerror(ERROR, 0,
                    "Missing mandatory field in the Wpack2d "
                    "description of file \"%s\" (fformat=%d)\n",
                    fname, fformat);
            mw_delete_wpack2d(pack);
            fclose(fp);
            return (NULL);
        }
    }
    else
        /* Old format (from fpack) */
    {
        if ((_mw_fascii_get_field(fp, fname,
                                  "name : ", "%[^\n]", pack->name) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "comment : ", "%[^\n]", pack->cmt) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "signal1 : ", "%[^\n]", ns1) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "signal2 : ", "%[^\n]", ns2) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "tree : ", "%[^\n]", nim) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "img_ncol : ", "%d\n",
                                     &pack->img_ncol) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "img_nrow : ", "%d\n",
                                     &pack->img_nrow) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "previous : ", "%[^\n]", buffer) != 1)
            || (_mw_fascii_get_field(fp, fname,
                                     "next : ", "%[^\n]", buffer) != 1))
        {
            mwerror(ERROR, 0,
                    "Missing mandatory field in the Wpack2d "
                    "description of file \"%s\" (fformat=%d)\n",
                    fname, fformat);
            mw_delete_wpack2d(pack);
            fclose(fp);
            return (NULL);
        }
    }

    /* Load signals and images */
    _mw_remove_first_spaces(ns1);
    _mw_remove_first_spaces(ns2);
    _mw_remove_first_spaces(nim);

    /* Get the pathname of fname */
    for (i = strlen(fname) - 1; (i >= 0) && (fname[i] != '/'); i--);
    if (fname[i] == '/')
    {
        strncpy(pn, fname, i + 1);
        pn[i + 1] = '\0';
    }
    else
        pn[0] = '\0';

    /* Load signal 1 */
    if (ns1[0] == '/')          /* Absolute pathname */
        strcpy(fn, ns1);
    else
    {
        /* Relative pathname : add pathname of fname and see if signal
         * 1 is found here */

        sprintf(fn, "%s%s", pn, ns1);
        if (NULL != (fp2 = fopen(fn, "r")))
            /* not found : do not add pathname
             * (file is probably in $MEGAWAV2/data) */
        {
            strcpy(fn, ns1);
            fclose(fp2);
        }

    }

    if (_mwload_fsignal(fn, type, comment, &signal1) != 0)
    {
        mwerror(ERROR, 0,
                "Cannot load signal1 (impulse response h) from file \"%s\"\n",
                fn);
        mw_delete_wpack2d(pack);
        fclose(fp);
        return (NULL);
    }
    strcpy(signal1->name, ns1);

    /* Load signal 2 */
    if (ns2[0] == '/')          /* Absolute pathname */
        strcpy(fn, ns2);
    else
    {
        /* Relative pathname : add pathname of fname and see if signal
         * 2 is found here */
        sprintf(fn, "%s%s", pn, ns2);
        if (NULL != (fp2 = fopen(fn, "r")))
            /* not found : do not add pathname
             * (file is probably in $MEGAWAV2/data) */
        {
            strcpy(fn, ns2);
            fclose(fp2);
        }
    }

    if (_mwload_fsignal(fn, type, comment, &signal2) != 0)
    {
        mwerror(ERROR, 0,
                "Cannot load signal2 (impulse response h~) from file \"%s\"\n",
                fn);
        mw_delete_wpack2d(pack);
        fclose(fp);
        return (NULL);
    }
    strcpy(signal2->name, ns2);

    /* Load tree */
    if (nim[0] == '/')          /* Absolute pathname */
        strcpy(fn, nim);
    else                        /* Relative pathname : add pathname of fname */
        sprintf(fn, "%s%s", pn, nim);
    tree = (Cimage) _mw_cimage_load_image(fn, type);
    if (!tree)
    {
        mwerror(ERROR, 0,
                "Cannot load the quad-tree image from file \"%s\"\n", fn);
        mw_delete_wpack2d(pack);
        fclose(fp);
        return (NULL);
    }
    strcpy(tree->name, nim);

    if (!mw_alloc_wpack2d
        (pack, tree, signal1, signal2, pack->img_nrow, pack->img_ncol))
    {
        mwerror(ERROR, 0, "Allocation of Wpack2d failed !n");
        mw_delete_wpack2d(pack);
        fclose(fp);
        return (NULL);
    }

    mw_delete_cimage(tree);
    tree = NULL;
    mw_delete_fsignal(signal2);
    signal2 = NULL;
    mw_delete_fsignal(signal1);
    signal1 = NULL;

    for (j = 0; j < pack->img_array_size; j++)
        if (pack->images[j])
        {
            do
                ret = fscanf(fp, "%s", nim);
            while ((ret != 1) || (nim[0] == '%') || (nim[0] == '#'));

            if (nim[0] == '/')  /* Absolute pathname */
                strcpy(fn, nim);
            else                /* Relative pathname : add pathname of fname */
                sprintf(fn, "%s%s", pn, nim);

            /* free memory previously allocated to pack->images[j] */
            mw_delete_fimage(pack->images[j]);

            pack->images[j] = (Fimage) _mw_fimage_load_image(fn, type);
            if (!pack->images[j])
            {
                mwerror(ERROR, 0,
                        "Cannot load the wavelet packet coeff. "
                        "image #%d from file \"%s\"\n", j, fn);
                mw_delete_wpack2d(pack);
                fclose(fp);
                return (NULL);
            }
        }

    return (pack);
}

/* Native formats (without conversion of the internal type) */

Wpack2d _mw_wpack2d_load_native(char *fname, char *type)
{
    if (strcmp(type, "A_WPACK2D") == 0)
        return ((Wpack2d) _mw_load_wpack2d_ascii(fname));

    return (NULL);
}

/*~~~~~ All available formats ~~~~~*/

Wpack2d _mw_load_wpack2d(char *fname, char *type)
{
    Wpack2d pack;
    char mtype[mw_mtype_size];
    int hsize;                  /* Size of the header, in bytes */
    float version;              /* Version number of the file format */

    _mw_get_file_type(fname, type, mtype, &hsize, &version);

    /* First, try native formats */
    pack = _mw_wpack2d_load_native(fname, type);
    if (pack != NULL)
        return (pack);

    /* If failed, try other formats with memory conversion */
    pack = (Wpack2d) _mw_load_etype_to_itype(fname, mtype, "wpack2d", type);
    if (pack != NULL)
        return (pack);

    if (type[0] == '?')
        mwerror(FATAL, 1, "Unknown external type for the file \"%s\"\n",
                fname);
    else
        mwerror(FATAL, 1,
                "External type of file \"%s\" is %s. "
                "I Don't know how to load such external type "
                "into a Wpack2d !\n", fname, type);
    return NULL;
}

/*~~~~~~ Write Wpack2d using ASCII Format ~~~~~~*/

short _mw_create_wpack2d_ascii(char *fname, Wpack2d pack)
{
    FILE *fp;
    int i, j;
    char im_fname[BUFSIZ];
    char im_bname[BUFSIZ];
    char bname[BUFSIZ];
    char dname[BUFSIZ];

    if (!pack)
    {
        mwerror(INTERNAL, 0,
                "[_mw_create_wpack2d_ascii] Cannot create A_WPACK2D file: "
                "NULL wpack2d structure.\n");
        return (-1);
    }

    fp = _mw_create_data_ascii_file(fname);
    if (fp == NULL)
        return (-1);

    /* if (strcmp(pack->name, WPACK2D_DEFAULT_NAME)==0) */
    /* Replace default name (in pack->name) to the basename of fname */
    _mw_dirbasename(fname, dname, bname);
    strncpy(pack->name, bname, mw_namesize);
    pack->name[mw_namesize - 1] = '\0';

    fprintf(fp, "%%----- A_WPACK2D -----\n");
    fprintf(fp, "def Wpack2d\n");

    fprintf(fp, "name: %s\n", pack->name);
    fprintf(fp, "comment: %s\n", pack->cmt);
    fprintf(fp, "signal1: %s\n", pack->signal1->name);
    fprintf(fp, "signal2: %s\n", pack->signal2->name);
    fprintf(fp, "tree: %s\n", pack->tree->name);
    sprintf(im_fname, "%s%s", dname, pack->tree->name);
    _mw_cimage_create_image(im_fname, pack->tree, "IMG");
    fprintf(fp, "img_ncol: %d\n", pack->img_ncol);
    fprintf(fp, "img_nrow: %d\n", pack->img_nrow);

    if (!pack->previous)
        fprintf(fp, "previous: null\n");
    else
        fprintf(fp, "previous: %s\n", pack->previous->name);
    if (!pack->next)
        fprintf(fp, "next: null\n");
    else
        fprintf(fp, "next: %s\n", pack->next->name);

    /* images saving */
    for (j = 0; j < pack->tree->nrow; j++)
        for (i = 0; i < pack->tree->ncol; i++)
            if (pack->images[i + j * pack->tree->ncol] != NULL)
            {
                sprintf(im_bname, "%s_%d_%d", pack->name, i, j);
                sprintf(im_fname, "%s_%d_%d", fname, i, j);
                fprintf(fp, "%s\n", im_bname);
                _mw_fimage_create_image(im_fname,
                                        pack->images[i +
                                                     j * pack->tree->ncol],
                                        "RIM");
            }

    fclose(fp);
    return (0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_wpack2d_create_native(char *fname, Wpack2d pack, char *Type)
{
    if (strcmp(Type, "A_WPACK2D") == 0)
        return (_mw_create_wpack2d_ascii(fname, pack));

    return (-1);
}

/*~~~~~ Write file in different formats ~~~~~*/

short _mw_create_wpack2d(char *fname, Wpack2d pack, char *Type)
{
    short ret;

    /* First, try native formats */
    ret = _mw_wpack2d_create_native(fname, pack, Type);
    if (ret == 0)
        return (0);

    /* If failed, try other formats with memory conversion */
    ret = _mw_create_etype_from_itype(fname, pack, "wpack2d", Type);
    if (ret == 0)
        return (0);

    /* Invalid Type should have been detected before, but we can
     * arrive here because of a write failure (e.g. the output file name
     * is write protected).
     */
    mwerror(FATAL, 1, "Cannot save \"%s\" : all write procedures failed !\n",
            fname);
    return -1;
}

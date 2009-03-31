/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {skeleton};
 version = {"2.2"};
 author = {"Denis Pasquignon"};
 function = {"compute the skeleton of a cimage"};
 usage = {
   'n':[iteration=2]->iteration      "number of iterations",
   'c':[infsup_iter=1]->infsup_iter  "number of iterations of infsup",
   'l':[extremite=10]->extremite     "stopping test for the extremities",
   'a'->average      "swap first and second level med and take the average",
   cimage_in->image  "input cimage",
   cmovie->cmovie    "masks sequence",
   cmovie<-output    "skeletons sequence of the image"
};
*/
/*----------------------------------------------------------------------
 v2.1: return result + external call (L.Moisan)
 v2.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"         /* for infsup() */

#define sqrt22 0.353553390593273564

                                       /* first define the max,min of
                                        * two quantities */
#define   MAX(A,B)          ((A) > (B)) ?  (A) : (B)
#define   MIN(A,B)          ((A) < (B)) ?  (A) : (B)
                                       /* now define the max,min of
                                        * three quantities */
                                       /* we use a temporary variable
                                        * to speed up    */
#define   MAX3(A,B,C)       (A) > (TMP= ((B)>(C)) ? (B):(C)) ? (A) : TMP
#define   MIN3(A,B,C)       (A) < (TMP= ((B)<(C)) ? (B):(C)) ? (A) : TMP

static void Final(Cimage pict, Cmovie sortie)
{
    register int i;
    Cimage image1, image2;
    int dx, dy, taille;
    register unsigned char *b;
    register unsigned char *a;

    image1 = NULL;
    dx = pict->ncol;
    dy = pict->nrow;
    image1 = mw_change_cimage(image1, dy, dx);
    if (sortie->first == NULL)
    {
        sortie->first = image1;
    }
    else
    {
        image2 = sortie->first;
        while (image2->next != NULL)
        {
            image2 = image2->next;
        }
        image2->next = image1;
        image1->previous = image2;
    }
    a = image1->gray;
    b = pict->gray;
    taille = dx * dy;
    for (i = 0; i < taille; i++)
    {
        if (*b < 0.0)
            *b = 0.0;
        if (*b > 255.0)
            *b = 255.0;
        *a = (unsigned char) (*b);
        a++;
        b++;
    }
}

                                 /* test de largeur                          */
                                 /* on a central element                     */

static int test_mcm(Cimage im, long unsigned int a)
{
    int test;
    int dx, dy;

    dx = im->ncol;
    dy = im->nrow;

    test = 1;

    /* configuration en y */

    if (im->gray[a] == 0)
    {

        if ((im->gray[a - dx - 1] == 0) && (im->gray[a - dx] == 255)
            && (im->gray[a - dx + 1] == 0)
            && (im->gray[a - 1] == 255) && (im->gray[a + 1] == 255)
            && (im->gray[a - 1 + dx] == 255) && (im->gray[a + dx] == 0)
            && (im->gray[a + dx + 1] == 255))

            test = 0;

        if ((im->gray[a - dx - 1] == 255) && (im->gray[a - dx] == 0)
            && (im->gray[a - dx + 1] == 255)
            && (im->gray[a - 1] == 255) && (im->gray[a + 1] == 0)
            && (im->gray[a - 1 + dx] == 0) && (im->gray[a + dx] == 255)
            && (im->gray[a + dx + 1] == 255))

            test = 0;

        if ((im->gray[a - dx - 1] == 255) && (im->gray[a - dx] == 255)
            && (im->gray[a - dx + 1] == 0)
            && (im->gray[a - 1] == 0) && (im->gray[a + 1] == 255)
            && (im->gray[a - 1 + dx] == 255) && (im->gray[a + dx] == 255)
            && (im->gray[a + dx + 1] == 0))

            test = 0;

        if ((im->gray[a - dx - 1] == 0) && (im->gray[a - dx] == 255)
            && (im->gray[a - dx + 1] == 255)
            && (im->gray[a - 1] == 255) && (im->gray[a + 1] == 0)
            && (im->gray[a - 1 + dx] == 255) && (im->gray[a + dx] == 0)
            && (im->gray[a + dx + 1] == 255))

            test = 0;

        if ((im->gray[a - dx - 1] == 255) && (im->gray[a - dx] == 0)
            && (im->gray[a - dx + 1] == 255)
            && (im->gray[a - 1] == 255) && (im->gray[a + 1] == 255)
            && (im->gray[a - 1 + dx] == 0) && (im->gray[a + dx] == 255)
            && (im->gray[a + dx + 1] == 0))

            test = 0;

        if ((im->gray[a - dx - 1] == 255) && (im->gray[a - dx] == 255)
            && (im->gray[a - dx + 1] == 0)
            && (im->gray[a - 1] == 0) && (im->gray[a + 1] == 255)
            && (im->gray[a - 1 + dx] == 255) && (im->gray[a + dx] == 0)
            && (im->gray[a + dx + 1] == 255))

            test = 0;

        if ((im->gray[a - dx - 1] == 0) && (im->gray[a - dx] == 255)
            && (im->gray[a - dx + 1] == 255)
            && (im->gray[a - 1] == 255) && (im->gray[a + 1] == 0)
            && (im->gray[a - 1 + dx] == 0) && (im->gray[a + dx] == 255)
            && (im->gray[a + dx + 1] == 255))

            test = 0;

        if ((im->gray[a - dx - 1] == 255) && (im->gray[a - dx] == 0)
            && (im->gray[a - dx + 1] == 255)
            && (im->gray[a - 1] == 0) && (im->gray[a + 1] == 255)
            && (im->gray[a - 1 + dx] == 255) && (im->gray[a + dx] == 255)
            && (im->gray[a + dx + 1] == 0))

            test = 0;

    }

    /* detection de ligne  */

    if (im->gray[a] == 0)
    {

        if ((im->gray[a - 1] == 255) && (im->gray[a - dx - 1] == 0)
            && (im->gray[a - dx] == 255))

            test = 0;

        if ((im->gray[a - 1 - dx] == 255) && (im->gray[a - dx] == 0)
            && (im->gray[a - dx + 1] == 255))

            test = 0;

        if ((im->gray[a - dx] == 255) && (im->gray[a - dx + 1] == 0)
            && (im->gray[a + 1] == 255))

            test = 0;

        if ((im->gray[a + 1 - dx] == 255) && (im->gray[a + 1] == 0)
            && (im->gray[a + dx + 1] == 255))

            test = 0;

        if ((im->gray[a + 1] == 255) && (im->gray[a + dx + 1] == 0)
            && (im->gray[a + dx] == 255))

            test = 0;

        if ((im->gray[a + 1 + dx] == 255) && (im->gray[a + dx] == 0)
            && (im->gray[a + dx - 1] == 255))

            test = 0;

        if ((im->gray[a + dx] == 255) && (im->gray[a + dx - 1] == 0)
            && (im->gray[a - 1] == 255))

            test = 0;

        if ((im->gray[a - 1 + dx] == 255) && (im->gray[a - 1] == 0)
            && (im->gray[a - dx - 1] == 255))

            test = 0;

    }

    return (test);

}

static int test_thinning(Cimage im, long unsigned int a)
{
    int test;
    int dx, dy;

    dx = im->ncol;
    dy = im->nrow;

    test = 0;

    /* premier masque et ses rotations */

    if ((im->gray[a - dx] == 255) &&
        (im->gray[a - 1] == 0) && (im->gray[a + 1] == 0) &&
        (im->gray[a - 1 + dx] == 0) && (im->gray[a + dx] == 0) &&
        (im->gray[a + dx + 1] == 0) && (im->gray[a + 2 * dx - 1] == 0) &&
        (im->gray[a + 2 * dx] == 0) && (im->gray[a + 2 * dx + 1] == 0))

        test = 1;

    if ((im->gray[a - dx - 2] == 0) &&
        (im->gray[a - dx - 1] == 0) && (im->gray[a - dx] == 0) &&
        (im->gray[a - 2] == 0) && (im->gray[a - 1] == 0) &&
        (im->gray[a + 1] == 255) && (im->gray[a + dx - 2] == 0) &&
        (im->gray[a + dx - 1] == 0) && (im->gray[a + dx] == 0))

        test = 1;

    if ((im->gray[a - 2 * dx - 1] == 0) &&
        (im->gray[a - 2 * dx] == 0) && (im->gray[a - 2 * dx + 1] == 0) &&
        (im->gray[a - 1 - dx] == 0) && (im->gray[a - dx] == 0) &&
        (im->gray[a - dx + 1] == 0) && (im->gray[a - 1] == 0) &&
        (im->gray[a + 1] == 0) && (im->gray[a + dx] == 255))

        test = 1;

    if ((im->gray[a - dx] == 0) &&
        (im->gray[a - dx + 1] == 0) && (im->gray[a - dx + 2] == 0) &&
        (im->gray[a - 1] == 255) && (im->gray[a + 1] == 0) &&
        (im->gray[a + 2] == 0) && (im->gray[a + dx] == 0) &&
        (im->gray[a + dx + 1] == 0) && (im->gray[a + dx + 2] == 0))

        test = 1;

    /* deuxieme masque et ses rotations */

    if ((im->gray[a - dx] == 255) && (im->gray[a - dx + 1] == 255) &&
        (im->gray[a - 2] == 0) && (im->gray[a - 1] == 0) &&
        (im->gray[a + 1] == 255) && (im->gray[a + dx - 2] == 0) &&
        (im->gray[a + dx - 1] == 0) && (im->gray[a + dx] == 0) &&
        (im->gray[a + 2 * dx - 2] == 0) && (im->gray[a + 2 * dx] == 0) &&
        (im->gray[a + 2 * dx - 1] == 0))

        test = 1;

    if ((im->gray[a - 2 * dx - 2] == 0) && (im->gray[a - 2 * dx - 1] == 0) &&
        (im->gray[a - 2 * dx] == 0) && (im->gray[a - dx - 2] == 0) &&
        (im->gray[a - dx - 1] == 0) && (im->gray[a - dx] == 0) &&
        (im->gray[a - 2] == 0) && (im->gray[a - 1] == 0) &&
        (im->gray[a + 1] == 255) &&
        (im->gray[a + dx] == 255) && (im->gray[a + dx + 1] == 255))

        test = 1;

    if ((im->gray[a - 2 * dx] == 0) && (im->gray[a - 2 * dx + 1] == 0) &&
        (im->gray[a - 2 * dx + 2] == 0) && (im->gray[a - dx] == 0) &&
        (im->gray[a - dx + 1] == 0) && (im->gray[a - dx + 2] == 0) &&
        (im->gray[a + 2] == 0) && (im->gray[a + 1] == 0) &&
        (im->gray[a - 1] == 255) &&
        (im->gray[a + dx] == 255) && (im->gray[a + dx - 1] == 255))

        test = 1;

    if ((im->gray[a + 2 * dx] == 0) && (im->gray[a + 2 * dx + 1] == 0) &&
        (im->gray[a + 2 * dx + 2] == 0) && (im->gray[a + dx] == 0) &&
        (im->gray[a + dx + 1] == 0) && (im->gray[a + dx + 2] == 0) &&
        (im->gray[a + 2] == 0) && (im->gray[a + 1] == 0) &&
        (im->gray[a - 1] == 255) &&
        (im->gray[a - dx] == 255) && (im->gray[a - dx - 1] == 255))

        test = 1;

    return (test);

}

static int test_carda(Cimage im, long unsigned int a, int l_min)
{
    int ll, cc;
    int l, c, dl, dc;
    int test, compte;
    int rayon, longueur;
    int dx, dy, ref, ref_vois;

    dx = im->ncol;
    dy = im->nrow;

    rayon = 2;
    longueur = 2 * rayon + 1;

    l = a / dx;
    c = a - l * dx;

    test = 1;

    if (im->gray[a] == 0)
    {

        compte = 0;

        for (dl = -rayon; dl <= rayon; dl++)
        {
            for (dc = -rayon; dc <= rayon; dc++)
            {
                ll = l + dl;
                cc = c + dc;

                if (ll >= 0 && ll < dy && cc >= 0 && cc < dx)
                {

                    ref = cc + ll * dx;
                    ref_vois = (dc + rayon) + (dl + rayon) * (2 * rayon + 1);

                    if (im->gray[ref] == 0)
                    {
                        compte = compte + 1;
                    }

                }

            }
        }

        if (compte < l_min)
            test = 0;

    }

    return (test);

}

                                 /* compute erosion                          */
                                 /* on a central element                     */
static int image_erosion(Cimage im, long unsigned int a, float r)
{
    int dl, dc, ll, cc, ir;
    int sup, new;
    int l, c;
    int dx, dy;

    dx = im->ncol;
    dy = im->nrow;

    ir = (int) (r + 0.99);

    sup = im->gray[a];

    l = a / dx;
    c = a - l * dx;

    for (dl = -ir; dl <= ir; dl++)
    {
        for (dc = -ir; dc <= ir; dc++)
        {

            ll = l + dl;
            cc = c + dc;
            if (ll >= 0 && ll < dy && cc >= 0 && cc < dx &&
                (float) (dl * dl + dc * dc) <= r * r)
            {

                new = im->gray[cc + ll * dx];

        /***** erosion -> sup *****/
                if (new > sup)
                    sup = new;

            }

        }
    }

    return (sup);

}

Cmovie skeleton(int *iteration, int *infsup_iter, int *extremite,
                char *average, Fmovie cmovie, Cimage image, Cmovie output)
{
    Cimage pict = NULL;
    Cimage im_final = NULL;
    Cimage im_inter = NULL;
    Cimage im_work = NULL;
    int iter = *iteration;
    int test_a, test_b, dx, dy, n;
    int nc, n_courbure;
    unsigned long a, size;
    float deginf;
    float degsup;
    int l_min = *extremite;
    float rad;

    im_final = mw_change_cimage(im_final, image->nrow, image->ncol);
    if (im_final == NULL)
        mwerror(FATAL, 1, "Not enough memory\n");

    im_inter = mw_change_cimage(im_inter, image->nrow, image->ncol);
    if (im_inter == NULL)
        mwerror(FATAL, 1, "Not enough memory\n");

    pict = mw_change_cimage(pict, image->nrow, image->ncol);
    if (pict == NULL)
        mwerror(FATAL, 1, "Not enough memory\n");

    im_work = mw_change_cimage(im_work, image->nrow, image->ncol);
    if (im_work == NULL)
        mwerror(FATAL, 1, "Not enough memory\n");

    mw_copy_cimage(image, pict);
    mw_copy_cimage(image, im_final);

    size = image->ncol * image->nrow;
    dx = image->ncol;
    dy = image->nrow;
    rad = 1;

    /* Final(pict,output); */

    deginf = 0.0;
    degsup = 1.0;

    while (iter-- > 0)
    {

        mwdebug("Remaining skeleton iterations : [%i]\n", iter + 1);
        printf("%i \n", iter + 1);
        n = *infsup_iter;
        nc = 1;

        for (n_courbure = 0; n_courbure < n; n_courbure++)
        {

            infsup(&nc, &deginf, &degsup, average, pict, cmovie, im_inter);
            infsup(&nc, &degsup, &deginf, average, im_inter, cmovie, im_work);

            for (a = 0; a < size; a++)
            {

                test_a = test_carda(im_final, a, l_min);
                test_b = test_mcm(im_final, a);

                if ((test_a == 1) && (test_b == 1))
                {
                    pict->gray[a] = im_work->gray[a];
                }
                else
                {
                    pict->gray[a] = im_final->gray[a];
                }
            }

            mw_copy_cimage(pict, im_final);

        }

        Final(im_final, output);

        for (a = 0; a < size; a++)
        {

            test_a = test_thinning(im_final, a);

            if (test_a == 1)
            {
                pict->gray[a] = image_erosion(im_final, a, rad);
            }
            else
            {
                pict->gray[a] = im_final->gray[a];
            }

        }

        mw_copy_cimage(pict, im_final);

        /* Final(pict,output); */

    }

    Final(pict, output);

    return (output);
}

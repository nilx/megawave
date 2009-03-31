/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {wp2doperate};
 author = {"Francois Malgouyres"};
 version = {"1.1"};
 function = {"Change an image by operating on its 2D-wavelet packet
             representation"};
 usage = {
     't':[translation=1]->translation
     "Repeats the process on several translations of the image and
     average (like cycle spinning), enter the number of horizontal and
     vertical translations to be performed",
     'h':threshold->threshold_hard
     "A hard thresholding is performed with given threshold",
     's':threshold->threshold_soft
     "A soft thresholding is performed with given threshold",
     'H':threshold->track_noise_hard
     "A hard noise tracking is performed with given threshold",
     'S':threshold->track_noise_soft
     "A soft noise tracking is performed with given threshold ",
     'L':level->track_noise_level
     "Maximum level for the noise tracking",
     'c':Fimage->pfilter
     "Eigenvalues of the operator (diagonal in the wavelet packet basis)",
     'C':level->convolution_level
     "All the trees between the input tree and the tree of full depth
     provided by this input  are considered for the convolution
     (eigenvalues provided by -c must correspond to this latter
     tree)",
     'b':   h_tilde->Ri_biortho
     "Impulse response of h_tilde (biorthogonal wavelet)",
     tree->tree
     "Cimage describing the tree",
     h->Ri
     "Impulse response of the filter h",
     input->input
     "Input Fimage to be modified",
     output<-output
     "Output Fimage containing the result of the operation"
};
*/
/*----------------------------------------------------------------------
 v1.1: adaptation from fpackOperate v1.0 (fpack) (JF)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for wp2dchangetree(), wp2dchangepack(),
                                 * wp2dmktree(), wp2ddecomp(), wp2drecomp() */

#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)])

/******************************************************************/
/******************************************************************/

static void test_input(Cimage tree, Fimage pfilter, int *translation,
                       float *threshold_soft, float *threshold_hard,
                       float *track_noise_hard, float *track_noise_soft,
                       int *track_noise_level, int *convolution_level)
{
    int i, size = tree->ncol * tree->nrow;
    int sup, sup_translation;
    int nbOption;

    /* avoids simultaneous use of soft and hard thresholding or noise
     * tracking */
    nbOption = 0;
    if (threshold_soft)
        nbOption++;
    if (threshold_hard)
        nbOption++;
    if (track_noise_hard)
        nbOption++;
    if (track_noise_soft)
        nbOption++;

    if (nbOption > 1)
        mwerror(FATAL, 1,
                "At most one option has to be chosen among -h, -s, -H, -S !\n");

    /*stop if no processing is actually performed */

    if (pfilter)
        nbOption++;

    if (nbOption == 0)
        mwerror(FATAL, 1,
                "At least one option has to be chosen among "
                "-h, -s, -H, -S, -c !\n");

    /* checks compatibility between  track_noise_level and other noise
     * tracking options */

    if ((track_noise_hard || track_noise_soft) && track_noise_level == NULL)
        mwerror(FATAL, 1,
                "Option -L should be specified "
                "when -H or -S are selected !\n");

    if (track_noise_hard == NULL && track_noise_soft == NULL
        && track_noise_level)
        mwerror(FATAL, 1,
                "Option -L is useless when neither "
                "option -H nor -S are selected !\n");

    /*checks tree */
    mw_checktree_wpack2d(tree);

    /*checks compatibility between convolution_level and pfilter */
    if (convolution_level && pfilter == NULL)
        mwerror(FATAL, 1, "option -C must be used with option -c !\n");

    /* compare tree and pfilter size */

    if (pfilter)
    {
        if (convolution_level)
        {
            /*i=power(2,*convolution_level); */
            i = 1 << (*convolution_level);
            if ((tree->ncol != i) || (tree->nrow != i))
                mwerror(FATAL, 1,
                        "The size of pfilter must be 2^convolution_level!\n");
        }
        else if ((tree->ncol != pfilter->ncol)
                 || (tree->nrow != pfilter->nrow))
            mwerror(FATAL, 1, "pfilter and tree must have the same size !\n");
    }

    /* checks if number of translation is OK */
    sup = tree->gray[0];
    for (i = 1; i < size; i++)
        if (sup < tree->gray[i])
            sup = tree->gray[i];

    /*sup_translation=power(2,sup); */
    sup_translation = 1 << sup;

    if ((*translation) <= 0 || *translation > sup_translation)
        mwerror(FATAL, 1,
                "translation (option -t) should be between 1 and %d ! \n"
                " Generaly, 2 is good enough.\n", sup_translation);

    /*end of input tests */

}

/******************************************************************/
static int modulo(int r, int n)
{
    if (0 <= r && r < n)
        return (r);
    else if (r < 0)
    {
        while (r < 0)
            r += n;
        return (r);
    }
    else
    {
        while (r >= n)
            r -= n;
        return (r);
    }

}

/******************************************************************/
static void translate_image(Fimage in, Fimage out, int tx, int ty)
{
    int x, y, sx = out->ncol, sy = out->nrow;
    int x1, y1;

    for (x = 0; x < sx; x++)
    {
        x1 = modulo(x + tx, sx);

        for (y = 0; y < sy; y++)
        {
            y1 = modulo(y + ty, sy);

            _(out, x, y) = _(in, x1, y1);
        }
    }

}

/***************************************************************/
/*returns the maximum level of the decomposition corresponding to tree */

static int treeLevel(Cimage tree)
{
    int i;
    int size = tree->ncol * tree->nrow;
    int level;

    level = tree->gray[0];
    for (i = 1; i < size; i++)
        if (level < tree->gray[i])
            level = tree->gray[i];

    return (level);
}

/***************************************************************/
/*returns the minimum level of the decomposition corresponding to tree */

static int treeMin(Cimage tree)
{
    int size = tree->ncol * tree->nrow;
    int i;
    int min;

    min = tree->gray[0];
    for (i = 1; i < size; i++)
        if (tree->gray[i] < min)
            min = tree->gray[i];

    return (min);
}

/******************************************************************/

static void threshold_hard_wavelet_packet_transform(Wpack2d pack,
                                                    float *threshold_hard)
{
    int k;
    int i, size;
    float tmp;
    float Threshold = *threshold_hard, minusThreshold = -(*threshold_hard);

    /*Notice : the resume is not modified */
    for (k = 1; k < pack->img_array_size; k++)
        if (pack->images[k])
        {
            size = pack->images[k]->ncol * pack->images[k]->nrow;

            for (i = 0; i < size; i++)
            {
                tmp = pack->images[k]->gray[i];

                if (minusThreshold < tmp && tmp < Threshold)
                    pack->images[k]->gray[i] = 0;
            }
        }

}

/******************************************************************/

static void threshold_soft_wavelet_packet_transform(Wpack2d pack,
                                                    float *threshold_soft)
{
    int k;
    int i, size;
    float tmp;
    float Threshold = *threshold_soft, minusThreshold = -(*threshold_soft);

    /*Notice : the resume is not modified */
    for (k = 1; k < pack->img_array_size; k++)
        if (pack->images[k])
        {
            size = pack->images[k]->ncol * pack->images[k]->nrow;

            for (i = 0; i < size; i++)
            {
                tmp = pack->images[k]->gray[i];

                if (tmp < minusThreshold)
                    pack->images[k]->gray[i] += Threshold;
                else if (tmp < Threshold)
                    pack->images[k]->gray[i] = 0;
                else
                    pack->images[k]->gray[i] += minusThreshold;
            }
        }

}

/******************************************************************/

static void convol_wavelet_packet_transform(Wpack2d pack, Fimage pfilter)
{
    int k;
    int i, size;
    float tmp;

    /* Notice : the resume is modified when pfilter->gray[0]=1 */
    for (k = 0; k < pack->img_array_size; k++)
        if (pack->images[k])
        {
            tmp = pfilter->gray[k];
            size = pack->images[k]->ncol * pack->images[k]->nrow;

            for (i = 0; i < size; i++)
                pack->images[k]->gray[i] *= tmp;

        }

}

/***************************************************************/
/*Computes  pfilter_remaining, tempTree, pfilter_to_perform*/
/* pfilter_remaining initialy corresponds to the tree tempTree */
/* At the end of the process, it corresponds to nextTree*/
/* At the end of the process, pfilter_to_perform will correspond to
 * the tree tempTree */

static void change_pfilter_and_tree(Fimage pfilter_remaining,
                                    Fimage pfilter_to_perform,
                                    Cimage tempTree, Cimage tree)
{
    Cimage nextTree;            /* contains the the output value of
                                 * tempTree, its size could be
                                 * smaller than the size of tempTree
                                 */
    Fimage next_pfilter_remaining;      /* contains the the output
                                         * value of pfilter_remaining, its
                                         * size could be smaller than the
                                         * size of nextTree */
    int kx, ky;                 /* indexes for nextTree and
                                 * next_pfilter_remaining */
    int kx1, ky1;               /* indexes for tempTree and
                                 * pfilter_to_perform */
    int extra;                  /* ratio between the size of tempTree
                                 * and the size of nextTree */
    int x, y;                   /* for tempTree and pfilter_to_perform */
    float min, tmp;
    int level;
    int levelMax;
    int jumpSmallTree, jumpLargeTree;
    char not_null = 1;

    /* computes the next tree : it less decomposed than 'tempTree' but
     * more decomposed than 'tree' */
    nextTree = mw_new_cimage();

    wp2dchangetree(tempTree, nextTree, NULL, &not_null, NULL, NULL,
                   NULL, NULL);
    /*avoids one decomposition, when possible */
    wp2dchangetree(nextTree, nextTree, NULL, NULL, NULL, NULL, tree, NULL);
    /* makes sure it is sufficiently decomposed */

    /* computes pfilter_to_perform and next_pfilter_remaining */

    /* memory allocation and initialisation */
    pfilter_to_perform =
        mw_change_fimage(pfilter_to_perform, tempTree->nrow, tempTree->ncol);
    if (pfilter_to_perform == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    next_pfilter_remaining =
        mw_change_fimage(NULL, nextTree->nrow, nextTree->ncol);
    if (next_pfilter_remaining == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    levelMax = treeLevel(tempTree);

    extra = tempTree->ncol / nextTree->ncol;

    /*Main Process */
    for (level = 1; level <= levelMax; level++)
    {
        /*jumpSmallTree = nextTree->ncol/power(2,level); */
        jumpSmallTree = nextTree->ncol >> level;
        jumpLargeTree = jumpSmallTree * extra;

        for (kx = 0, kx1 = 0; kx < nextTree->ncol;
             kx += jumpSmallTree, kx1 += jumpLargeTree)
            for (ky = 0, ky1 = 0; ky < nextTree->nrow;
                 ky += jumpSmallTree, ky1 += jumpLargeTree)
            {
                if (_(nextTree, kx, ky) == level
                    && _(nextTree, kx, ky) < _(tempTree, kx1, ky1))
                    /* this means
                     * _(tempTree,kx1+x,ky1+y) = _(nextTree,kx,ky)+1 ,
                     * for x and y between 0 and extra-1 */
                {
                    /* computes the value in next_pfilter_remaining */
                    /* it is the minimum, among the modulus
                     * of_(pfilter_to_perform ,kx1+x,ky1+y) */
                    /* for x and y between 0 and jumpLarge-1 */
                    /* which are larger than 1 */
                    tmp = _(pfilter_remaining, kx1, ky1);
                    min = (tmp > 0) ? tmp : -tmp;

                    for (x = 0; x < jumpLargeTree; x++)
                        for (y = 0; y < jumpLargeTree; y++)
                        {
                            tmp = _(pfilter_remaining, kx1 + x, ky1 + y);
                            tmp = (tmp > 0) ? tmp : -tmp;
                            if (tmp > 1)
                                min = (min < tmp) ? min : tmp;
                        }

                    for (x = 0; x < jumpSmallTree; x++)
                        for (y = 0; y < jumpSmallTree; y++)
                            _(next_pfilter_remaining, kx + x, ky + y) = min;

                    /*computes pfilter_to_perform */
                    min = 1. / min;

                    for (x = 0; x < jumpLargeTree; x++)
                        for (y = 0; y < jumpLargeTree; y++)
                            _(pfilter_to_perform, kx1 + x, ky1 + y) =
                                min * _(pfilter_remaining, kx1 + x, ky1 + y);

                }

                else if (_(nextTree, kx, ky) == level)
                    /* it means
                     * _(nextTree,kx,ky) == _(tempTree,kx1+x,ky1+y),
                     * for x and y between 0 and extra-1 */
                {
                    for (x = 0; x < jumpSmallTree; x++)
                        for (y = 0; y < jumpSmallTree; y++)
                            _(next_pfilter_remaining, kx + x, ky + y) =
                                _(pfilter_remaining, kx1, ky1);

                    for (x = 0; x < jumpLargeTree; x++)
                        for (y = 0; y < jumpLargeTree; y++)
                            _(pfilter_to_perform, kx1 + x, ky1 + y) = 1.;

                }
            }
    }
    /* the output are updated with the results */
    /* copy nextTree in tempTree */
    if (tempTree->ncol != nextTree->ncol)
    {
        tempTree = mw_change_cimage(tempTree, nextTree->nrow, nextTree->ncol);

        if (tempTree == NULL)
            mwerror(FATAL, 1, "Not enough memory !\n");
    }

    mw_copy_cimage(nextTree, tempTree);

    /* copy next_pfilter_remaining in pfilter_remaining */
    if (pfilter_remaining->ncol != next_pfilter_remaining->ncol)
    {
        pfilter_remaining =
            mw_change_fimage(pfilter_remaining, next_pfilter_remaining->nrow,
                             next_pfilter_remaining->ncol);

        if (pfilter_remaining == NULL)
            mwerror(FATAL, 1, "Not enough memory !\n");
    }

    mw_copy_fimage(next_pfilter_remaining, pfilter_remaining);

 /** free useless memory **/
    mw_delete_fimage(next_pfilter_remaining);
    next_pfilter_remaining = NULL;
    mw_delete_cimage(nextTree);
    nextTree = NULL;
}

/***************************************************************/
/*computes the difference between the coefficients of two Wpack2ds */
/* the tree, signals,... are assumed identical    */

static void wpack2d_diff(Wpack2d pack, Wpack2d temppack)
{
    int k;
    int i, size;

    for (k = 0; k < pack->img_array_size; k++)
        if (pack->images[k])
        {
            size = pack->images[k]->ncol * pack->images[k]->nrow;

            for (i = 0; i < size; i++)
                pack->images[k]->gray[i] -= temppack->images[k]->gray[i];

        }
}

/*****************************************************************/
/* removes the large coefficients from pack */

static void noise_search_hard(Wpack2d pack, float *track_noise_hard)
{
    int k;
    int i, size;
    float tmp;
    float Threshold = *track_noise_hard, minusThreshold =
        -(*track_noise_hard);

    /*Notice : the resume is not modified */
    for (k = 1; k < pack->img_array_size; k++)
        if (pack->images[k])
        {
            size = pack->images[k]->ncol * pack->images[k]->nrow;

            for (i = 0; i < size; i++)
            {
                tmp = pack->images[k]->gray[i];

                /*only small coefficients are kept */
                if (tmp < minusThreshold || Threshold < tmp)
                    pack->images[k]->gray[i] = 0;
            }
        }

}

/*****************************************************************/
/* removes large coefficients from pack */

static void noise_search_soft(Wpack2d pack, float *track_noise_soft)
{
    int k;
    int i, size;
    float tmp;
    float Threshold = *track_noise_soft, minusThreshold =
        -(*track_noise_soft);

    /*Notice : the resume is not modified */
    for (k = 1; k < pack->img_array_size; k++)
        if (pack->images[k])
        {
            size = pack->images[k]->ncol * pack->images[k]->nrow;

            for (i = 0; i < size; i++)
            {
                tmp = pack->images[k]->gray[i];

                /*only small coefficients are kept */
                if (tmp < minusThreshold)
                    pack->images[k]->gray[i] = minusThreshold;
                else if (Threshold < tmp)
                    pack->images[k]->gray[i] = Threshold;
            }
        }

}

/*****************************************************************/
/* removes the large coefficients from pack */
static void noise_search(Wpack2d pack, float *track_noise_hard,
                         float *track_noise_soft)
{
    if (track_noise_hard)
        noise_search_hard(pack, track_noise_hard);
    else if (track_noise_soft)
        noise_search_soft(pack, track_noise_soft);
}

/******************************************************************/
/* performs the noise tracking algorithm */

static void noise_tracking(Wpack2d pack, int *track_noise_level,
                           float *track_noise_hard, float *track_noise_soft)
{
    Cimage inOutTree;
    Cimage tmpTree;
    Wpack2d temppack;
    int nbIter = (*track_noise_level) - treeMin(pack->tree);
    int i;
    char not_null = 1;

    /* save tree, the tree of the wpack2d is the same at the beginning and
     * the end of this function */
    inOutTree = mw_change_cimage(NULL, pack->tree->nrow, pack->tree->ncol);
    if (inOutTree == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    mw_copy_cimage(pack->tree, inOutTree);

    /*memory allocation and initialisation */

    tmpTree = mw_change_cimage(NULL, pack->tree->nrow, pack->tree->ncol);
    if (tmpTree == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    mw_copy_cimage(pack->tree, tmpTree);

    temppack = mw_new_wpack2d();
    mw_copy_wpack2d(pack, temppack, 0);

    /* MAIN PROCESS */

    noise_search(temppack, track_noise_hard, track_noise_soft);

    for (i = 0; i < nbIter; i++)
    {
        wp2dchangetree(tmpTree, tmpTree, &not_null, NULL, NULL, NULL, NULL,
                       NULL);
        /*computes new wpack2d */
        wp2dchangepack(temppack, temppack, tmpTree);
        /*search for noise in this basis */
        noise_search(temppack, track_noise_hard, track_noise_soft);
    }

    /* the last resume is set to 0, so that it is preserved after the
     * difference with pack */
    mw_clear_fimage(temppack->images[0], 0);

    /*COME BACK TO THE INITIAL TREE */
    /*computes new wpack2d */
    wp2dchangepack(temppack, temppack, inOutTree);

    /* computes the difference between pack and the noise found during
     * the noise tracking process */
    wpack2d_diff(pack, temppack);

    /*FREE MEMORY */

    mw_delete_wpack2d(temppack);
    temppack = NULL;
    mw_delete_cimage(inOutTree);
    inOutTree = NULL;
    mw_delete_cimage(tmpTree);
    tmpTree = NULL;
}

/******************************************************************/

/*-------------------------------------------------------------*/
/*-------------  MAIN                ----------------------------*/
/*-------------------------------------------------------------*/

void wp2doperate(Fimage input, Fsignal Ri, Fsignal Ri_biortho, Cimage tree,
                 Fimage output, float *threshold_hard, float *threshold_soft,
                 int *translation, float *track_noise_hard,
                 float *track_noise_soft, int *track_noise_level,
                 Fimage pfilter, int *convolution_level)
{
    Wpack2d pack = NULL;
    Fimage work1, work2;
    Cimage tempTree;
    Fimage pfilter_remaining = NULL;
    Fimage pfilter_to_perform = NULL;
    int tx, ty;
    int i, size = input->ncol * input->nrow;
    float tmp;
    char not_null = 1;
    int nbIter, iter;

    /*test inputs */

    test_input(tree, pfilter, translation, threshold_hard, threshold_soft,
               track_noise_hard, track_noise_soft, track_noise_level,
               convolution_level);

 /** Memory allocation and initialisation **/

    if ((output = mw_change_fimage(output, input->nrow, input->ncol)) == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    if ((work1 = mw_change_fimage(NULL, input->nrow, input->ncol)) == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    if ((work2 = mw_change_fimage(NULL, input->nrow, input->ncol)) == NULL)
        mwerror(FATAL, 1, "Not enough memory !\n");

    /* copies pfilter in pfilter_remaining, sothat on
     * pfilter_remaining can eventually be modified */
    if (pfilter)
    {
        pfilter_remaining =
            mw_change_fimage(NULL, pfilter->nrow, pfilter->nrow);
        if (pfilter_remaining == NULL)
            mwerror(FATAL, 1, "Not enough memory !\n");

        mw_copy_fimage(pfilter, pfilter_remaining);
    }

    if (convolution_level)
        /* creates tempTree and pfilter_to_perform and computes the
         * number of iteration (one iteration per level) */
    {
        if ((tempTree = mw_new_cimage()) == NULL)
            mwerror(FATAL, 1, "Not enough memory !\n");
        wp2dmktree(*convolution_level, tempTree, NULL, NULL, NULL,
                   &not_null, NULL);
        /* full tree of depth convolution_level */

        nbIter = 1 + (*convolution_level) - treeMin(tree);

        pfilter_to_perform = mw_new_fimage();
        if (pfilter_to_perform == NULL)
            mwerror(FATAL, 1, "Not enough memory !\n");
    }
    else                        /*creates tempTree */
    {
        if ((tempTree =
             mw_change_cimage(NULL, tree->nrow, tree->ncol)) == NULL)
            mwerror(FATAL, 1, "Not enough memory !\n");

        mw_copy_cimage(tree, tempTree);

        nbIter = 1;
    }

    /*initializes the result */

    mw_clear_fimage(output, 0.0);

    /* MAIN Processes  */

    for (tx = 0; tx < (*translation); tx++)
        for (ty = 0; ty < (*translation); ty++)
        {                       /* translation of the input image */

            translate_image(input, work1, tx, ty);

            /*computation of the wavelet packet transform */
            pack = mw_new_wpack2d();
            if (!pack)
                mwerror(FATAL, 1, "Not enough memory !\n");
            wp2ddecomp(work1, Ri, Ri_biortho, pack, tempTree);

     /** Only iterates if convolution_level is on, otherwise nbIter=1 **/
            for (iter = 0; iter < nbIter; iter++)
            {
                /*  remove noise */

                if (threshold_hard)
                    threshold_hard_wavelet_packet_transform(pack,
                                                            threshold_hard);

                if (threshold_soft)
                    threshold_soft_wavelet_packet_transform(pack,
                                                            threshold_soft);

                if (track_noise_hard || track_noise_soft)
                    noise_tracking(pack, track_noise_level, track_noise_hard,
                                   track_noise_soft);

                /* apply operator diagonal in the wavelet packet domain */
                if (pfilter_remaining)
                {
                    if (iter == nbIter - 1)
                        convol_wavelet_packet_transform(pack,
                                                        pfilter_remaining);
                    else
                    {
                        change_pfilter_and_tree(pfilter_remaining,
                                                pfilter_to_perform, tempTree,
                                                tree);

                        convol_wavelet_packet_transform(pack,
                                                        pfilter_to_perform);

             /**Reconstruction**/

                        /*computes new wpack2d */
                        wp2dchangepack(pack, pack, tempTree);

                    }
                }
            }

            /*computes inverse wavelet packet transform */

            wp2drecomp(pack, work1);

            /* inverse translation of output image */

            translate_image(work1, work2, -tx, -ty);

            /*averaging */

            for (i = 0; i < size; i++)
                output->gray[i] += work2->gray[i];

        }

    /* End of MAIN processes */
    /* normalisation */

    if (*translation > 1)
    {
        tmp = 1. / (float) ((*translation) * (*translation));
        for (i = 0; i < size; i++)
            output->gray[i] *= tmp;
    }

 /*** Free memory ***/
    mw_delete_cimage(tempTree);
    if (pfilter_to_perform)
        mw_delete_fimage(pfilter_to_perform);
    if (pfilter_remaining)
        mw_delete_fimage(pfilter_remaining);
    mw_delete_fimage(work1);
    mw_delete_fimage(work2);
    mw_delete_wpack2d(pack);
}

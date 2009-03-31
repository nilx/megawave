/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {ccontrast_local};
 version = {"1.0"};
 author = {"Yan Jinhai"};
 function = {"Local contrast improvment"};
 usage = {
'd':[d=7]->d[0,7]  "locality parameter (subdivide [0,255] 2^d times, 0<=d<=7)",
'n'->n_flag        "to neutralize the anti-bright-spot device",
input->in          "input Cimage",
output<-out        "output Cimage"
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for extract_connex() */

#define GRAY_LEVEL(image, point) \
    ((image)->gray[(int)((point)->y)*(image)->ncol+(int)((point)->x)])

static int *histo_global;       /* old histogram */
static float *newht_global;     /* new histogram */

/*--------------- SOME BASIC FUNCTIONS ---------------------------*/

static int integer_sup(float f)
{
    int q;

    q = (int) f;
    if (q != f)
        q = q + 1;              /* above integer part */
    if (q < 0)
        q = 0;
    if (q > 255)
        q = 255;
    return q;
}

static int IPOWER(int a, int INT)       /* Integer Power */
{
    int i;

    if (INT < 1)
        i = 1;
    else
        i = a * IPOWER(a, INT - 1);
    return i;
}

/*---------- CONTRAST enhancing along a Curve (connected region) -----------*/

/*
   The image to be enhanced is 'fimage', its gray value is of float type,
   for the computing of its histogram, we use the original image 'image',
   since in the connected region 'curve', we have:

   fimage(point1)==fimage(point2) <=> image(point1)==image(point2)
*/
static void ccontrast_curve(Cimage image, Fimage fimage, Fcurve curve,
                            int *lambda1, int *lambda2, int *image_size,
                            char *n_flag)
                                /* original image */
                                /* image to be enhanced */
                                /* enhancing region */
                                /* enhancing gray value region */
{
    unsigned int h, histo_distr;
    unsigned int i, j, curve_area, size;
    float yd, epsilon;
    Point_fcurve p;

    for (i = 0; i < 256; i++)
    {
        histo_global[i] = 0;
        newht_global[i] = 0.0;
    }

    /* Compute the old histogram */
    epsilon = 0.0;
    for (p = curve->first; p; p = p->next)
    {
        histo_global[(unsigned int) (GRAY_LEVEL(image, p))]++;
        epsilon++;
    }
    epsilon = epsilon * 256.0 / (float) (*image_size * (*lambda2 - *lambda1));
    if (epsilon > 1.0)
        epsilon = 1.0;
    epsilon = 1.0 - epsilon;

    /* Compute the new histogram */
    /* firstly distribution 'curve_area' from 0 to i-1 */
    curve_area = 0;
    histo_distr = 0;
    for (i = 0; i < 256; i++)
        if (0 != (h = histo_global[i]))
        {
            newht_global[i] = (float) curve_area;
            curve_area += h;
            histo_distr++;
        }
    size = curve_area;
    /* secondly distribution 'curve_area' from i+1 to 255 */
    /* and the linear conbination  of the two areas */
    curve_area = 0;
    for (i = 0; i < 256; i++)
    {
        j = 255 - i;
        if (0 != (h = histo_global[j]))
        {
            yd = (newht_global[j] - (float) curve_area) / (float) size;
            yd = ((float) *lambda1) * (1.0 - yd) + ((float) *lambda2) * (1.0 +
                                                                         yd);
            yd = yd * 0.5;
            newht_global[j] = yd;
            curve_area += h;
        }
    }

    /* Process image */
    if (n_flag)
        for (p = curve->first; p; p = p->next)
            GRAY_LEVEL(fimage, p) = newht_global[GRAY_LEVEL(image, p)];
    else
        for (p = curve->first; p; p = p->next)
            GRAY_LEVEL(fimage, p) =
                (1.0 - epsilon) * newht_global[GRAY_LEVEL(image, p)] +
                epsilon * GRAY_LEVEL(fimage, p);
}

/*---------------------------------------------------------------------------*/
/*--------------------------- MAIN MODULE -----------------------------------*/
/*---------------------------------------------------------------------------*/

void ccontrast_local(Cimage in, Cimage out, int *d, char *n_flag)
{
    int j, k, k2, adr, size, lambda, lambda1, lambda2;
    int debug_gray1, debug_gray2, debug_gray3, debug_empty;
    float gray_level;
    Cimage inner;               /* for extract connected region */
    Fimage iterate;             /* for iteration */
    Fcurves curves;             /* pointer to connected regions */
    Fcurve curve;               /* pointer to one of the connected regions */
    Point_fcurve p;
    int nr = in->nrow;
    int nc = in->ncol;

    curves = mw_new_fcurves();

  /*--------------INITIALIZATION---------------------*/

    histo_global = (int *) calloc(256, sizeof(int));
    newht_global = (float *) calloc(256, sizeof(float));

    out = mw_change_cimage(out, nr, nc);
    inner = mw_change_cimage(NULL, nr, nc);
    iterate = mw_change_fimage(NULL, nr, nc);

    if (out == NULL || inner == NULL || iterate == NULL
        || !histo_global || !newht_global)
        mwerror(FATAL, 1, "Not enough memory. \n");

    size = nr * nc;
    for (adr = 0; adr < size; adr++)
        iterate->gray[adr] = (float) in->gray[adr];

    if (*d > 7)
        *d = 7;

    mwdebug("Remind you, the total iterate level=%d. \n", *d);

  /*--------- MAIN ITERATION --------*/

    for (k = 0; k <= *d; k++)
    {

        mwdebug("Remind you, iterating level %d. \n", k);

        k2 = IPOWER(2, k);
        lambda = 256 / k2;
        for (j = 0; j < k2; j++)
        {

      /*------ GRAY LEVEL SEGMENTATION ---------*/

            lambda1 = j * lambda;
            lambda2 = (j + 1) * lambda;

      /*------ EXTRACT CONNECTED COMPONENTS ----*/

            debug_empty = 0;
            for (adr = 0; adr < size; adr++)
            {
                gray_level = iterate->gray[adr];
                if (gray_level - (float) lambda1 < 0.0
                    || gray_level - (float) lambda2 >= 0.0)
                    inner->gray[adr] = 255;
                else
                {
                    inner->gray[adr] = 0;
                    debug_empty++;
                }
            }

            /* ------------ Begin debug for empty region ---------- */
            /* If it is a empty set for the points
             * with gray values in [lanbda1, lambda2),
             * we skip to the next step.
             */

            if (debug_empty != 0)
            {

                adr = 128;

                extract_connex(inner, curves, &adr);

                debug_gray3 = 0;
                for (curve = curves->first; curve; curve = curve->next)
                {
                    debug_gray1 = 0;
                    debug_gray2 = 0;
                    for (p = curve->first; p; p = p->next)
                    {
                        debug_gray1++;
                        gray_level = GRAY_LEVEL(iterate, p);
                        adr = GRAY_LEVEL(inner, p);
                        if (gray_level - (float) lambda1 < 0.0
                            || gray_level - (float) lambda2 >= 0.0)
                        {
                            debug_gray2++;
                            debug_gray3++;
                            if (adr == 0)
                                mwerror(FATAL, 1,
                                        "Some thing strange happened.\n");
                            else
                                mwerror(ERROR, 1,
                                        "\n inner gray=%d, iterated gray=%f.\n",
                                        adr, gray_level);
                        }
                    }
                    if (debug_gray2 > 0)
                        mwerror(ERROR, 1,
                                "\n %d points in this connected region "
                                "[%d, %d),\n %d points' gray value "
                                "out of range\n",
                                debug_gray1, lambda1, lambda2, debug_gray2);
                }
                if (debug_gray3 > 0)
                {
                    for (adr = 0; adr < size; adr++)
                        out->gray[adr] = inner->gray[adr];
                    mwerror(ERROR, 1,
                            "\n The inner image used for extract "
                            "connected regions has only "
                            "two gray values: 0 or 255,\n"
                            " we extracted the regions where "
                            "inner gray=0,\n"
                            " but we have found %d points "
                            "who's gray value=255,\n Help me!\n",
                            debug_gray3);
                    return;
                }

        /*-------------- end of Debug --------------------------------*/

        /*------- CONTRAST ENHANCE IN EACH CONNECTED COMPONENTS */

                for (curve = curves->first; curve; curve = curve->next)
                    ccontrast_curve(in, iterate, curve, &lambda1, &lambda2,
                                    &size, n_flag);

        /*--------------- End of debug for empty region --------------*/
            }
      /*------------------------------------------------------------*/
        }
    }
  /*----------------- END OF MAIN ITERATION ------------------------*/

    /* Output */

    for (adr = 0; adr < size; adr++)
    {
        gray_level = iterate->gray[adr];
        out->gray[adr] = integer_sup(gray_level);
    }

    mw_delete_fimage(iterate);
    mw_delete_cimage(inner);
    mw_delete_fcurves(curves);
}

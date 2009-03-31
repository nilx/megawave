/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {ll_boundaries};
  version = {"1.3"};
  author = {"Lionel Moisan"};
  function = {"Extract meaningful boundaries
              (contrasted level lines) from a Fimage"};
  usage = {
 'e':[eps=0]->eps     "-log10(max. number of false alarms)",
 'a'->all             "get all contrasted level lines (not only maximal ones)",
 'w'->weak            "select weak maximality",
 's':step->step       "quantization step (bilinear)",
 'p':p->precision     "sampling precision (bilinear)",
 't':tree->tree       "use a precomputed FLST tree",
 'z'->z               "use zero order instead of bilinear interpolation",
 in->in               "input Fimage",
 out<-ll_boundaries   "output boundaries (Flists)"
    };
*/
/*----------------------------------------------------------------------
 v1.1: added weak maximality, fixed Sonylogo 'type' bug (L.Moisan)
 v1.2: upgraded fhisto() call (L.Moisan)
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for fderiv(), flst(), flst_bilinear(),
                                 * flstb_quantize(), flst_boundary(),
                                 * flstb_boundary(), fsaddles(),
                                 * sintegral(), fhisto() */

#define HISTO_STEP 0.01

/*----- internal structure associated to each Shape -----*/
typedef struct mydata {
    int ndetect;
    float nfa;
    char type;
    /* inferior_type of nearest ascending meaningful boundary */
} *Mydata;

/*----- global variables -----*/
static Fimage image, NormOfDu;
static int precision1, precision2, max_only, zero_order;
static Fsignal logProbaOfDu;
static Shapes ref_tree;
static float **tabsaddles;
static Flists boundaries;
static Flist boundary;

/*===== Compute the minimum contrast and the length of the curve l =====*/

static float min_contrast(Flist l, float *length)
{
    double per;
    float mu, minmu, x, y, ox, oy;
    int i, ix, iy;

    per = 0.;
    minmu = FLT_MAX;

    for (i = 0; i < l->size; i++)
    {

        x = l->values[i * 2];
        y = l->values[i * 2 + 1];

        if (!zero_order)
        {
            if (i > 0)
                per +=
                    sqrt((double) (x - ox) * (x - ox) + (y - oy) * (y - oy));
            ox = x;
            oy = y;
        }

        ix = floor((double) x + .5) - 1;
        iy = floor((double) y + .5) - 1;
        if (ix >= 0 && iy >= 0 && ix < NormOfDu->ncol && iy < NormOfDu->nrow)
        {
            mu = NormOfDu->gray[NormOfDu->ncol * iy + ix];
            if (mu < minmu)
                minmu = mu;
        }
    }
    if (minmu == FLT_MAX)
        minmu = 0.;
    if (zero_order)
        *length = (float) l->size;
    else
        *length = (float) per;

    return (minmu);
}

/*===== compute NFA term associated to contrast mu =====*/

static float logH(float mu)
{
    int i;

    i = (int) (mu / logProbaOfDu->scale);
    if (i >= logProbaOfDu->size)
        i = logProbaOfDu->size - 1;
    return (logProbaOfDu->values[i]);
}

/*===== first pass: compute # meaningful sons w/o contrast reversal =====*/

static void update_mydata(Shape s, float threshold, char type)
{
    Shape t;
    float mu = 0.0, length = 0.0;
    struct mydata *sdata, *tdata;

    if (!s)
        return;

    sdata = (struct mydata *) malloc(sizeof(struct mydata));
    if (!sdata)
        mwerror(FATAL, 1, "Not enough memory\n");
    s->data = (void *) sdata;

    if (s->parent)
    {

        /* extract boundary */
        if (zero_order)
            flst_boundary(ref_tree, s, boundary);
        else
            /* FIXME: cast */
            flstb_boundary(&precision1, image, ref_tree, s, NULL,
                           boundary, (char *) tabsaddles);

    /*** compute nfa (-log10) ***/
        mu = min_contrast(boundary, &length);
        sdata->nfa = length * logH(mu) / (zero_order ? 3. : 2.);

        /* update type */
        if (sdata->nfa < threshold)
            sdata->type = s->inferior_type;
        else
            sdata->type = type;

    }
    else
    {

        sdata->nfa = threshold;
        sdata->type = s->inferior_type;

    }

    /* visit children and update ndetect from children */
    sdata->ndetect = 0;
    for (t = s->child; t; t = t->next_sibling)
    {
        update_mydata(t, threshold, sdata->type);
        tdata = (struct mydata *) t->data;
        if (tdata->nfa < threshold)
        {
            if (sdata->type == t->inferior_type)
                sdata->ndetect++;
        }
        else
            sdata->ndetect += tdata->ndetect;
    }
    if (sdata->nfa < threshold)
        mwdebug("contrast=%g length=%g nfa=%g (threshold=%g) ndetect=%d\n",
                mu, length, sdata->nfa, threshold, sdata->ndetect);
}

/*===== second pass : compute and store maximal meaningful boundaries =====*/

static void add_boundary(Shape s, float threshold, float *bestnfa_inf,
                         float bestnfa_sup, char type)
{
    Shape t;
    float nfa, new_bestnfa_inf, old_bestnfa_sup;
    struct mydata *sdata, *tdata;

    if (!s)
        return;

    if (s->parent)
    {

        sdata = (struct mydata *) s->data;

        nfa = sdata->nfa;

        /* update bestnfa_sup */
        if (nfa <= bestnfa_sup)
            bestnfa_sup = nfa;
        old_bestnfa_sup = bestnfa_sup;
        if (type != s->inferior_type || sdata->ndetect != 1)
            bestnfa_sup = threshold;

        /* visit children and update bestnfa_inf from children */
        new_bestnfa_inf = threshold;
        for (t = s->child; t; t = t->next_sibling)
        {
            tdata = (struct mydata *) t->data;
            *bestnfa_inf = threshold;
            add_boundary(t, threshold, bestnfa_inf, bestnfa_sup, sdata->type);
            if (*bestnfa_inf < new_bestnfa_inf
                && sdata->ndetect == 1 && sdata->type == tdata->type)
                new_bestnfa_inf = *bestnfa_inf;
        }
        *bestnfa_inf = new_bestnfa_inf;

        /* test if this shape has to be kept */
        if ((nfa < threshold && !max_only) ||
            (nfa < threshold && nfa <= old_bestnfa_sup && nfa < *bestnfa_inf))
        {

            /* store boundary */
            if (zero_order)
                flst_boundary(ref_tree, s, boundary);
            else
                /* FIXME: cast */
                flstb_boundary(&precision2, image, ref_tree, s, NULL,
                               boundary, (char *) tabsaddles);
            boundaries->list[boundaries->size++] =
                mw_copy_flist(boundary, NULL);
            printf("nfa=%f bestnfa_inf=%f bestnfa_sup=%f type=%d %d %d\n",
                   nfa, *bestnfa_inf, bestnfa_sup, type, sdata->type,
                   s->inferior_type);

        }
        else
            s->removed = (char) 1;
        if (nfa < *bestnfa_inf)
            *bestnfa_inf = nfa;

    }
    else
    {

        for (t = s->child; t; t = t->next_sibling)
            add_boundary(t, threshold, bestnfa_inf, bestnfa_sup,
                         s->inferior_type);

    }
}

/*===== simplified recursive algorithm for no or weak maximality =====*/

static void add_boundary_weak(Shape s, float threshold, float *bestnfa_inf,
                              float bestnfa_sup)
{
    Shape t;
    float mu, nfa, new_bestnfa_inf, new_bestnfa_sup, length;
    int nchildren;

    if (!s)
    {
        *bestnfa_inf = threshold;
        return;
    }

    for (nchildren = 0, t = s->child; t; t = t->next_sibling)
        nchildren++;

    if (s->parent)
    {

        /* extract boundary */
        if (zero_order)
            flst_boundary(ref_tree, s, boundary);
        else
            /* FIXME: cast */
            flstb_boundary(&precision1, image, ref_tree, s, NULL,
                           boundary, (char *) tabsaddles);

    /*** compute nfa (-log10) ***/
        mu = min_contrast(boundary, &length);
        nfa = length * logH(mu) / (zero_order ? 3. : 2.);

        /* check for 'type' chain break */
        if (s->parent->inferior_type != s->inferior_type)
            bestnfa_sup = threshold;

        /* compute new_bestnfa_sup */
        if (nchildren > 1)
            new_bestnfa_sup = threshold;
        else
            new_bestnfa_sup = bestnfa_sup;
        if (nfa < new_bestnfa_sup)
            new_bestnfa_sup = nfa;

        /* visit children and comput new_bestnfa_inf from children */
        new_bestnfa_inf = threshold;
        for (t = s->child; t; t = t->next_sibling)
        {
            add_boundary_weak(t, threshold, bestnfa_inf, new_bestnfa_sup);
            if (nchildren <= 1 && *bestnfa_inf < new_bestnfa_inf)
                new_bestnfa_inf = *bestnfa_inf;
        }

        /* test if this shape has to be kept */
        if (nfa < threshold && (!max_only ||
                                (nfa <= bestnfa_sup
                                 && nfa < new_bestnfa_inf)))
        {

            /* store boundary */
            if (zero_order)
                flst_boundary(ref_tree, s, boundary);
            else
                /* FIXME: cast */
                flstb_boundary(&precision2, image, ref_tree, s, NULL,
                               boundary, (char *) tabsaddles);
            boundaries->list[boundaries->size++] =
                mw_copy_flist(boundary, NULL);

        }
        else
            s->removed = (char) 1;

        *bestnfa_inf = new_bestnfa_inf;
        if (nfa < *bestnfa_inf)
            *bestnfa_inf = nfa;
        if (s->parent->inferior_type != s->inferior_type)
            *bestnfa_inf = threshold;

    }
    else
    {

        for (t = s->child; t; t = t->next_sibling)
            add_boundary_weak(t, threshold, bestnfa_inf, bestnfa_sup);

    }
}

/*------------------------------ MAIN MODULE ------------------------------*/

Flists ll_boundaries(Fimage in, Shapes tree, float *eps, char *all,
                     float *step, int *precision, char *z, char *weak)
{
    Fimage saddles = NULL, copy_in;
    float threshold, nfa_inf, nfa_sup, offset, histo_step, fzero;
    int newtree = 0, nsize, i;

    /* check consistency */
    if (all && weak)
        mwerror(WARNING, 0,
                "-w option is useless when -a option is selected\n");

    zero_order = (z != NULL);
    if (tree)
    {
        if (zero_order && tree->interpolation != 0)
            mwerror(FATAL, 1,
                    "Please use a zero order tree with -z option\n");
        if (!zero_order && tree->interpolation != 1)
            mwerror(FATAL, 1,
                    "Please use a bilinear tree without -z option\n");
    }
    if (zero_order && step)
        mwerror(WARNING, 0, "-s option have no effect with -z option\n");

    /* compute FLST if needed */
    if (!tree)
    {
        newtree = 1;
        tree = mw_new_shapes();
        copy_in = mw_change_fimage(NULL, in->nrow, in->ncol);
        if (!tree || !copy_in)
            mwerror(FATAL, 1, "Not enough memory");
        mw_copy_fimage(in, copy_in);
        if (zero_order)
            flst(NULL, copy_in, tree);
        else
            flst_bilinear(NULL, copy_in, tree);
        mw_delete_fimage(copy_in);
    }

    if (!zero_order)
    {
        /* bilinear case : compute saddle points and quantize FLST tree */
        ref_tree = mw_new_shapes();
        saddles = mw_new_fimage();
        if (!ref_tree || !saddles)
            mwerror(FATAL, 1, "Not enough memory");
        fsaddles(in, saddles);
        tabsaddles = mw_newtab_gray_fimage(saddles);
        if (!tabsaddles)
            mwerror(FATAL, 1, "Not enough memory");
        offset = 0.5;
        flstb_quantize(NULL, &offset, step, tree, ref_tree);
        if (newtree)
            mw_delete_shapes(tree);
    }
    else
        ref_tree = tree;

    mwdebug("Total number of shapes: %d\n", ref_tree->nb_shapes);

    /* initialization and memory allocation */
    image = in;
    precision1 = 2;
    precision2 = (precision ? *precision : 2);
    boundaries = mw_change_flists(NULL, ref_tree->nb_shapes - 1, 0);
    if (!boundaries)
        mwerror(FATAL, 1, "Not enough memory");
    if (all)
        max_only = 0;
    else
        max_only = 1;
    boundary = mw_change_flist(NULL, 2, 0, 2);

    /* compute NormOfDu */
    NormOfDu = mw_new_fimage();
    fzero = 0.;
    nsize = 3;
    fderiv(in, NULL, NULL, NULL, NULL, NULL, NULL, NormOfDu, NULL, &fzero,
           &nsize);

    /* compute logProbaOfDu */
    histo_step = HISTO_STEP;
    logProbaOfDu =
        fhisto(NormOfDu, NULL, &fzero, NULL, NULL, &histo_step, NULL, NULL);
    logProbaOfDu->values[0] = 0.;       /* because Du!=0 on level lines */
    sintegral(logProbaOfDu, (char *) 1, (char *) 1);
    for (i = 0; i < logProbaOfDu->size; i++)
        logProbaOfDu->values[i] =
            (float) log10((double) logProbaOfDu->values[i]);

    /* add boundaries recursively */
    threshold = -*eps - (float) log10((double) (ref_tree->nb_shapes));
    nfa_inf = nfa_sup = threshold;
    mwdebug("NFA threshold: %g\n", threshold);
    if (weak || all)
    {
        add_boundary_weak(ref_tree->the_shapes, threshold, &nfa_inf, nfa_sup);
    }
    else
    {
        update_mydata(ref_tree->the_shapes, threshold, 0);
        add_boundary(ref_tree->the_shapes, threshold, &nfa_inf, nfa_sup, 0);
    }
    printf("%d %sboundaries detected\n", boundaries->size,
           (all ? "" : "maximal "));

    /* free memory and exit */
    mw_delete_flist(boundary);
    mw_delete_fimage(NormOfDu);
    mw_delete_fsignal(logProbaOfDu);
    if (!zero_order)
    {
        free(tabsaddles);
        mw_delete_fimage(saddles);
        mw_delete_shapes(ref_tree);
    }

    return (boundaries);
}

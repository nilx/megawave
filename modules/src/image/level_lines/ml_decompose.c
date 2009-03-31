/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {ml_decompose};
 version = {"6.10"};
 author = {"Georges Koepfler"};
 function = {"Compute all morpho_lines of an image"};
 usage = {
  'c':m_image_in->m_image_in   "original image in Mimage structure",
  'o':[ml_opt=0]->ml_opt [0,2] "type of level lines: 0=upper, 1=lower, 2=iso",
  'm'->m_flag            "optimize memory occupation (allows no free(points))",
   image_in->image_in    "original image",
   m_image<-ml_decompose "mimage with all morpho_lines"
};
*/
/*----------------------------------------------------------------------
 v6.9: upgrade for new kernel and new fvalues() call (L.Moisan)
 v6.10 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#define NDEBUG                  /* comment this line out to enable assert() */
#include <assert.h>
#include "mw.h"
#include "mw-modules.h"         /* for ml_extract(), fvalues() */

#define POINT_OK(P,Y,X)  (((P)->x>=0)&&((P)->x<=X)&&((P)->y>=0)&&((P)->y<=Y))
#define BAD_POINT(P,Y,X) (!POINT_OK(P,Y,X))

static void llcheck(Mimage mimage)
{
    Point_curve point;
    Morpho_line ll;
    int NC, NL;

    NC = mimage->ncol;
    NL = mimage->nrow;
    for (ll = mimage->first_ml; ll; ll = ll->next)
    {
        point = ll->first_point;
        while (point != NULL)
        {
            if (BAD_POINT(point, NL, NC))
            {
                mwdebug
                    ("Morpho Line number %d :\n"
                     "   point->x=%d (NC=%d) \t point->y=%d (NL=%d)\n",
                     ll->num, point->x, NC, point->y, NL);
                mwerror(WARNING, 0, "[llcheck] Point out of image.\n");
            }
            point = point->next;
        }
    }
}

Mimage ml_decompose(Mimage m_image_in, int *ml_opt, char *m_flag,
                    Fimage image_in)
{
    Mimage m_image = NULL;
    Fsignal levels = NULL, tmp_levels;
    long l;
    char i_flag;                /* for values */

    m_image = mw_change_mimage(m_image);
    if (m_image == NULL)
        mwerror(FATAL, 1, "Not enough memory.\n");

    if (m_image_in)
    {
        if (m_image_in->first_ml != NULL)
            mwerror(WARNING, 1, "Level lines of m_image_in not copied!\n");
        if ((m_image_in->nrow != image_in->nrow) ||
            (m_image_in->ncol != image_in->ncol))
            mwerror(WARNING, 1,
                    "image_in and m_image_in not of same dimension!\n"
                    " Dimensions of image_in are kept.\n");
        m_image->first_fml = m_image_in->first_fml;
        m_image->first_ms = m_image_in->first_ms;
    }
    else
    {
        m_image->first_fml = NULL;
        m_image->first_ms = NULL;
    }
    m_image->nrow = image_in->nrow;
    m_image->ncol = image_in->ncol;

    /* construct levels according to the ml_opt (unoptimized but readable) */
    if (*ml_opt == 0)
    {
        tmp_levels = fvalues(NULL, NULL, NULL, image_in);
        m_image->minvalue = tmp_levels->values[0];
        m_image->maxvalue = tmp_levels->values[tmp_levels->size - 1];
        levels = mw_change_fsignal(NULL, tmp_levels->size - 1);
        for (l = 0; l < levels->size; l++)
            levels->values[l] = tmp_levels->values[l + 1];
        mw_delete_fsignal(tmp_levels);
    }
    if (*ml_opt == 1)
    {
        tmp_levels = fvalues(&i_flag, NULL, NULL, image_in);
        m_image->minvalue = tmp_levels->values[tmp_levels->size - 1];
        m_image->maxvalue = tmp_levels->values[0];
        levels = mw_change_fsignal(NULL, tmp_levels->size - 1);
        for (l = 0; l < levels->size; l++)
            levels->values[l] = tmp_levels->values[l + 1];
        mw_delete_fsignal(tmp_levels);
    }
    if (*ml_opt == 2)
    {
        levels = fvalues(NULL, NULL, NULL, image_in);
        m_image->minvalue = levels->values[0];
        m_image->maxvalue = levels->values[levels->size - 1];
    }
    if (2 * levels->size > m_image->nrow * m_image->ncol)
        printf("\n Warning : %d different pixel values (for %d pixels) !\n",
               levels->size, m_image->nrow * m_image->ncol);

    ml_extract(NULL, levels, ml_opt, NULL, m_flag, image_in, m_image);

    mw_delete_fsignal(levels);

    if (mwdbg == 1)
    {
        mwdebug("Checking mimage in ml_decompose (%d level lines)...\n",
                mw_num_morpho_line(m_image->first_ml));
        llcheck(m_image);
        mwdebug("End of checking mimage\n");
    }

    return (m_image);
}

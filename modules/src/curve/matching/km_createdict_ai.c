/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {km_createdict_ai};
version = {"1.1"};
author = {"Jose-Luis Lisani, Pablo Muse, Frederic Sur"};
function = {"Encode a list of curves into an affine-invariant dictionnary"};
usage = {
   'F':[FNorm=2.0]->FNorm    "length factor",
   'N':[NNorm=9]->NNorm      "number of points per code",
   list_curves->list_curves  "input list of curves (Flists)",
   dict<-dict                "output dictionary (Flists)"
        };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <math.h>
#include <time.h>
#include "mw.h"
#include "mw-modules.h"         /* for km_inflexionpoints(), km_flatpoints(),
                                 * km_bitangents(), km_codecurve_ai() */

#define _(a,i,j) ((a)->values[(i)*((a)->dim)+(j)])

/*------------------------------ MAIN MODULE ------------------------------*/

void km_createdict_ai(float *FNorm, int *NNorm, Flists list_curves,
                      Flists dict)
{
    Flist curveIP, curveFP, curveBP;
    Flists dictaux;
    int j, i, t1, t2, num_curves;
    float angle, dist;

    t1 = clock();
    num_curves = list_curves->size;
    angle = 0.4f;
    dist = 8.0f;

    if (2 * (*NNorm / 2) == *NNorm)
        mwerror(FATAL, 1, "error, NNorm is not odd\n");
    if ((dict = mw_change_flists(dict, num_curves * 50, 0)) == NULL)
        mwerror(FATAL, 1, "error, not enough memory\n");

    /* for each curve, compute IP, FP and BP,
     * and the add the codes to the dictionary */

    for (i = 0; i < num_curves; i++)
    {

        if ((curveIP = mw_new_flist()) == NULL)
            mwerror(FATAL, 1, "error, not enough memory\n");
        if ((curveFP = mw_new_flist()) == NULL)
            mwerror(FATAL, 1, "error, not enough memory\n");
        if ((curveBP = mw_new_flist()) == NULL)
            mwerror(FATAL, 1, "error, not enough memory\n");
        if ((dictaux = mw_new_flists()) == NULL)
            mwerror(FATAL, 1, "error, not enough memory\n");
        if (list_curves->list[i]->size == 0)
            continue;

        km_inflexionpoints(list_curves->list[i], curveIP);
        km_flatpoints(list_curves->list[i], curveIP, curveFP, angle, dist);
        km_bitangents(list_curves->list[i], curveIP, curveBP);

        km_codecurve_ai(list_curves->list[i], curveIP, curveFP, curveBP,
                        dictaux, i, *NNorm, *FNorm);

        for (j = 0; j < dictaux->size; j++)
        {
            dict->list[dict->size] = mw_copy_flist(dictaux->list[j], NULL);
            dict->size++;
            if (dict->size == dict->max_size)
                mw_enlarge_flists(dict);
        }
        mw_delete_flists(dictaux);
        mw_delete_flist(curveIP);
        mw_delete_flist(curveFP);
        mw_delete_flist(curveBP);
    }
    mw_realloc_flists(dict, dict->size);
    dict->data_size = sizeof(int);
    /*dict->data=(void*)(&list_curves->size); */
    dict->data = (void *) (NNorm);
    t2 = clock();

    mwdebug("number of words in the dictionary: %d\n", dict->size);
    mwdebug("elapsed time: %f\n", (float) (t2 - t1) / CLOCKS_PER_SEC);
}

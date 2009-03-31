/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  wtrans1d.c

  Vers. 1.2
  Author : Jacques Froment
  Basic memory routines for the wtrans1d internal type

  Main changes :
  v1.2 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
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

#include "fsignal.h"

#include "wtrans1d.h"

/* creates a new wtrans1d structure */

Wtrans1d mw_new_wtrans1d(void)
{
    Wtrans1d wtrans;
    int j, v;

    if (!(wtrans = (Wtrans1d) (malloc(sizeof(struct wtrans1d)))))
    {
        mwerror(ERROR, 0, "[mw_new_wtrans1d] Not enough memory\n");
        return (NULL);
    }

    wtrans->type = wtrans->edges = wtrans->nlevel = wtrans->complex = 0;
    wtrans->nfilter = wtrans->size = wtrans->nvoice = 0;

    strcpy(wtrans->cmt, "?");
    strcpy(wtrans->name, "?");

    for (j = 0; j < mw_max_nlevel; j++)
        for (v = 0; v < mw_max_nvoice; v++)
            wtrans->A[j][v] = wtrans->AP[j][v] = wtrans->D[j][v] =
                wtrans->DP[j][v] = NULL;

    for (j = 0; j < mw_max_nfilter_1d; j++)
        strcpy(wtrans->filter_name[j], "?");

    return (wtrans);
}

/* Alloc a wtrans1d for a wavelet decomposition with or without  */
/* a complex wavelet (internal use only).                        */

void *_mw_alloc_wtrans1d(Wtrans1d wtrans, int level, int voice,
                         int size, int complex, int sampling, int use_average)
{
    int N, j, v, jj, vv;

    if (wtrans == NULL)
    {
        mwerror(ERROR, 0,
                "cannot alloc wtrans1d: wtrans1d structure is NULL\n");
        return (NULL);
    }

    if (size <= 0)
    {
        mwerror(ERROR, 0,
                "cannot alloc wtrans1d: Illegal size of the original signal\n");
        return (NULL);
    }

    if (level > mw_max_nlevel)
    {
        mwerror(ERROR, 0,
                "cannot alloc wtrans1d: too many levels (%d) "
                "in the decomposition\n", level);
        return (NULL);
    }

    if (voice > mw_max_nvoice)
    {
        mwerror(ERROR, 0,
                "cannot alloc wtrans1d: too many voices "
                "per octave (%d) in the decomposition\n", voice);
        return (NULL);
    }

    if (sampling == 1)
    {
        /* ----- Orthonormal/Biorthonormal case ----- */

/*      if ((size % 2) != 0)
 *      {
 *      mwerror(ERROR, 0,
 *              "cannot alloc wtrans1d: Size of original signal "
 *              "is %d - not an even size -\n",size);
 *      return(NULL);
 *      }
 */
        N = size;
        for (j = 0; j <= level; j++, N /= 2)
            for (v = 0; v < voice; v++)
                if ((j > 0) || (v > 0))
                {
/*
 * if ((N % 2) != 0)
 * {
 * mwerror(ERROR, 0,
 *         "cannot alloc wtrans1d: Size of original signal is %d and max. "
 *         "level is %d\n",size,level);
 *
 * for (jj=0;jj<=j;jj++) for(vv=0;vv<voice;vv++) if ((jj>0)||(vv>0))
 * {
 * if (wtrans->A[jj][vv] != NULL)
 * {
 * mw_delete_fsignal(wtrans->A[jj][vv]);
 * wtrans->A[jj][vv] = NULL;
 * }
 * if (wtrans->AP[jj][vv] != NULL)
 * {
 * mw_delete_fsignal(wtrans->AP[jj][vv]);
 * wtrans->AP[jj][vv] = NULL;
 * }
 * if (wtrans->D[jj][vv] != NULL)
 * {
 * mw_delete_fsignal(wtrans->D[jj][vv]);
 * wtrans->D[jj][vv] = NULL;
 * }
 * if (wtrans->DP[jj][vv] != NULL)
 * {
 * mw_delete_fsignal(wtrans->DP[jj][vv]);
 * wtrans->DP[jj][vv] = NULL;
 * }
 * }
 * return(NULL);
 * }
 */
                    if (use_average == 1)
                        wtrans->A[j][v] = mw_change_fsignal(NULL, N);
                    wtrans->D[j][v] = mw_change_fsignal(NULL, N);
                    if (complex == 1)
                    {
                        if (use_average == 1)
                            wtrans->AP[j][v] = mw_change_fsignal(NULL, N);
                        wtrans->DP[j][v] = mw_change_fsignal(NULL, N);
                    }

                    if (((use_average == 1) && (wtrans->A[j][v] == NULL))
                        || (wtrans->D[j][v] == NULL) ||
                        ((complex == 1) && ((wtrans->DP[j][v] == NULL) ||
                                            ((use_average == 1)
                                             && (wtrans->AP[j][v] == NULL)))))
                    {
                        mwerror(ERROR, 0,
                                "cannot alloc wtrans1d: Not enough memory\n");
                        for (jj = 1; jj <= j; jj++)
                            for (vv = 0; vv < voice; vv++)
                            {
                                if (wtrans->A[jj][vv] != NULL)
                                {
                                    mw_delete_fsignal(wtrans->A[jj][vv]);
                                    wtrans->A[jj][vv] = NULL;
                                }
                                if (wtrans->D[jj][vv] != NULL)
                                {
                                    mw_delete_fsignal(wtrans->D[jj][vv]);
                                    wtrans->D[jj][vv] = NULL;
                                }
                                if (wtrans->AP[jj][vv] != NULL)
                                {
                                    mw_delete_fsignal(wtrans->AP[jj][vv]);
                                    wtrans->AP[jj][vv] = NULL;
                                }
                                if (wtrans->DP[jj][vv] != NULL)
                                {
                                    mw_delete_fsignal(wtrans->DP[jj][vv]);
                                    wtrans->DP[jj][vv] = NULL;
                                }
                            }
                        return (NULL);
                    }
                }
    }
    else
    {                           /* ----- Dyadic/Continuous case ----- */

        N = size;
        for (j = 0; j <= level; j++)
            for (v = 0; v < voice; v++)
                if ((j > 0) || (v > 0))
                {
                    if (use_average == 1)
                        wtrans->A[j][v] = mw_change_fsignal(NULL, N);
                    wtrans->D[j][v] = mw_change_fsignal(NULL, N);
                    if (complex == 1)
                    {
                        if (use_average == 1)
                            wtrans->AP[j][v] = mw_change_fsignal(NULL, N);
                        wtrans->DP[j][v] = mw_change_fsignal(NULL, N);
                    }

                    if (((use_average == 1) && (wtrans->A[j][v] == NULL))
                        || (wtrans->D[j][v] == NULL) ||
                        ((complex == 1) &&
                         ((wtrans->DP[j][v] == NULL) ||
                          ((use_average == 1)
                           && (wtrans->AP[j][v] == NULL)))))
                    {
                        mwerror(ERROR, 0,
                                "cannot alloc wtrans1d: Not enough memory\n");
                        for (jj = 1; jj <= j; jj++)
                            for (vv = 0; vv < voice; vv++)
                                if ((jj > 0) || (vv > 0))
                                {
                                    if (wtrans->A[jj][vv] != NULL)
                                    {
                                        mw_delete_fsignal(wtrans->A[jj][vv]);
                                        wtrans->A[jj][vv] = NULL;
                                    }
                                    if (wtrans->D[jj][vv] != NULL)
                                    {
                                        mw_delete_fsignal(wtrans->D[jj][vv]);
                                        wtrans->D[jj][vv] = NULL;
                                    }
                                    if (wtrans->AP[jj][vv] != NULL)
                                    {
                                        mw_delete_fsignal(wtrans->AP[jj][vv]);
                                        wtrans->AP[jj][vv] = NULL;
                                    }
                                    if (wtrans->DP[jj][vv] != NULL)
                                    {
                                        mw_delete_fsignal(wtrans->DP[jj][vv]);
                                        wtrans->DP[jj][vv] = NULL;
                                    }
                                }
                        return (NULL);
                    }
                }
    }

    wtrans->size = size;
    wtrans->nlevel = level;
    wtrans->nvoice = voice;
    wtrans->complex = complex;

    return (wtrans);
}

/* Alloc a wtrans1d for an orthonormal decomposition */

void *mw_alloc_ortho_wtrans1d(Wtrans1d wtrans, int level, int size)
{
    if (_mw_alloc_wtrans1d(wtrans, level, 1, size, 0, 1, 1) == NULL)
    {
        mwerror(ERROR, 0,
                "[mw_alloc_ortho_wtrans1d] Cannot alloc "
                "orthogonal wtrans1d.\n");
        return (NULL);
    }
    wtrans->type = mw_orthogonal;
    return (wtrans);
}

/* Alloc a wtrans1d for a biorthonormal decomposition */

void *mw_alloc_biortho_wtrans1d(Wtrans1d wtrans, int level, int size)
{
    if (_mw_alloc_wtrans1d(wtrans, level, 1, size, 0, 1, 1) == NULL)
    {
        mwerror(ERROR, 0,
                "[mw_alloc_biortho_wtrans1d] Cannot alloc "
                "biorthogonal wtrans1d.\n");
        return (NULL);
    }
    wtrans->type = mw_biorthogonal;
    return (wtrans);
}

/* Alloc a wtrans1d for a dyadic decomposition */

void *mw_alloc_dyadic_wtrans1d(Wtrans1d wtrans, int level, int size)
{
    if (_mw_alloc_wtrans1d(wtrans, level, 1, size, 0, 0, 1) == NULL)
    {
        mwerror(ERROR, 0,
                "[mw_alloc_dyadic_wtrans1d] Cannot alloc dyadic wtrans1d.\n");
        return (NULL);
    }
    wtrans->type = mw_dyadic;
    return (wtrans);
}

/* Alloc a wtrans1d for a continuous decomposition */

void *mw_alloc_continuous_wtrans1d(Wtrans1d wtrans,
                                   int level, int voice, int size,
                                   int complex)
{
    if (_mw_alloc_wtrans1d(wtrans, level, voice, size, complex, 0, 0) == NULL)
    {
        mwerror(ERROR, 0,
                "[mw_alloc_continuous_wtrans1d] Cannot alloc "
                "continuous wtrans1d.\n");
        return (NULL);
    }
    wtrans->type = mw_continuous;
    return (wtrans);
}

/* Desallocate the memory used by a wtrans1d and the structure itself */

void mw_delete_wtrans1d(Wtrans1d wtrans)
{
    int j, v;

    if (wtrans == NULL)
    {
        mwerror(ERROR, 0,
                "[mw_delete_wtrans1d] cannot delete : "
                "wtrans1d structure is NULL\n");
        return;
    }

    wtrans->A[0][0] = NULL;

    for (j = 0; j <= wtrans->nlevel; j++)
        for (v = 0; v < wtrans->nvoice; v++)
            if ((j > 0) || (v > 0))
            {
                if (wtrans->A[j][v] != NULL)
                {
                    mw_delete_fsignal(wtrans->A[j][v]);
                    wtrans->A[j][v] = NULL;
                }
                if (wtrans->D[j][v] != NULL)
                {
                    mw_delete_fsignal(wtrans->D[j][v]);
                    wtrans->D[j][v] = NULL;
                }
                if (wtrans->AP[j][v] != NULL)
                {
                    mw_delete_fsignal(wtrans->AP[j][v]);
                    wtrans->AP[j][v] = NULL;
                }
                if (wtrans->DP[j][v] != NULL)
                {
                    mw_delete_fsignal(wtrans->DP[j][v]);
                    wtrans->DP[j][v] = NULL;
                }
            }

    for (j = 0; j < mw_max_nfilter_1d; j++)
        strcpy(wtrans->filter_name[j], "?");

    wtrans->type = wtrans->edges = wtrans->nlevel = wtrans->complex = 0;
    wtrans->nfilter = wtrans->size = wtrans->nvoice = 0;

    wtrans->cmt[0] = wtrans->name[0] = '\0';
    free(wtrans);
    wtrans = NULL;              /* Useless but who knows ? */
}

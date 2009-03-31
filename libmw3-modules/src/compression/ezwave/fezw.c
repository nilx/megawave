/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {fezw};
version = {"1.32"};
author = {"Jean-Pierre D'Ales"};
function = {"Compress an image with EZW algorithm"};
usage = {
'r':NLevel->NumRec [1,20]
        "Number of level for wavelet tranform",
'e':EdgeIR->Edge_Ri
        "Impulse reponses of edge and preconditionning filters
        for orthogonal transform (fimage)",
'b':ImpulseResponse2->Ri2
        "Impulse response of filter 2 for biorthogonal transform (fsignal)",
'n':FilterNorm->FilterNorm [0,2]
        "Normalization mode for filter bank",
'w':WeightFac->WeightFac
        "Scaling factor for wavelet coefficients",
'd'->DistRate
        "Computes distorsion-rate function",
'R':TargetRate->Rate
        "Target Rate",
'P':TargetPSNR->PSNR
        "Target PSNR",
's':SelectArea->SelectedArea
        "Polygonal regions to be encoded
        with a different rate or PSNR (Polygons)",
'o':Compress<-Output
        "Output compressed Image (cimage)",
Image->Image
        "Input image (fimage)",
ImpulseResponse->Ri
        "Impulse response of inner filters (fsignal)",
        {
          Qimage<-QImage
                  "Output quantized image (fimage)"
        },
          notused->PtrDRC
                  "Distorsion rate curve"
        };
 */
/*----------------------------------------------------------------------
 v1.32: bugs on memory deallocation on Wtrans and QWtrans corrected (JF)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for ezw(), owave2(), iowave2(),
                                 * biowave2(), ibiowave2(), fmse() */

/*--- Constants ---*/

#define  NBIT_GREYVAL 8         /* Number of bit per pixels */
#define  MAX_GREYVAL 255.0      /* Maximum of grey levels in input image */
#define mw_drcurvesize 256

/*--- Special structures ---*/

typedef struct drcurve {
    int nrow;                   /* Number of rows (dy) */
    int ncol;                   /* Number of columns (dx) */
    int nbitplanes;             /* Number of bit per pixel */
    int npoints;                /* Number of points in the curve */
    float rate[mw_drcurvesize]; /* Bit rates */
    float cr[mw_drcurvesize];   /* Compression ratios */
    float mse[mw_drcurvesize];  /* Mean square error for correponding bit rate
                                 * and compression ratio */

} DRCurve;                      /* Distortion rate curves */

/*--- Global variables ---*/

static float targrate_dr[20];
static float targcr_dr[20];
static int count_dr;
static int max_count_dr;
static DRCurve *drc;            /* Distorsion rate curve */

static Fsignal ORI1, ORI2;      /* Non normalized filters */

static void INIT_RI(Fsignal ri1, Fsignal ri2)
{
    int i;

    ORI1 = mw_new_fsignal();
    if (mw_alloc_fsignal(ORI1, ri1->size) == NULL)
        mwerror(FATAL, 1, "Not enough memory for ri1!\n");

    ORI2 = mw_new_fsignal();
    if (mw_alloc_fsignal(ORI2, ri2->size) == NULL)
        mwerror(FATAL, 1, "Not enough memory for ri2!\n");

    for (i = 0; i < ri1->size; i++)
        ORI1->values[i] = ri1->values[i];

    for (i = 0; i < ri2->size; i++)
        ORI2->values[i] = ri2->values[i];

}

static void REFRESH_FILTERS(Fsignal ri1, Fsignal ri2)
{
    int i;

    for (i = 0; i < ri1->size; i++)
        ri1->values[i] = ORI1->values[i];

    for (i = 0; i < ri2->size; i++)
        ri2->values[i] = ORI2->values[i];

}

static void INIT_TARGNBIT_DR(Wtrans2d wtrans, char *ptrdrc)
{

    targrate_dr[0] = 0.008;
    targrate_dr[1] = 0.016;
    targrate_dr[2] = 0.032;
    targrate_dr[3] = 0.064;
    targrate_dr[4] = 0.08;
    targrate_dr[5] = 0.1;
    targrate_dr[6] = 0.13333;
    targrate_dr[7] = 0.16;
    targrate_dr[8] = 0.2;
    targrate_dr[9] = 0.26667;
    targrate_dr[10] = 0.4;
    targrate_dr[11] = 0.53333;
    targrate_dr[12] = 0.8;
    targrate_dr[13] = 1.6;

    targcr_dr[0] = 1000.0;
    targcr_dr[1] = 500.0;
    targcr_dr[2] = 250.0;
    targcr_dr[3] = 125.0;
    targcr_dr[4] = 100.0;
    targcr_dr[5] = 80.0;
    targcr_dr[6] = 60.0;
    targcr_dr[7] = 50.0;
    targcr_dr[8] = 40.0;
    targcr_dr[9] = 30.0;
    targcr_dr[10] = 20.0;
    targcr_dr[11] = 15.0;
    targcr_dr[12] = 10.0;
    targcr_dr[13] = 5.0;

    max_count_dr = 13;
    count_dr = 0;

    if (ptrdrc)
    {
        drc = (DRCurve *) ptrdrc;
        drc->npoints = max_count_dr + 1;
        drc->nbitplanes = NBIT_GREYVAL;
        drc->nrow = wtrans->nrow;
        drc->ncol = wtrans->ncol;

        drc->cr[0] = 1000.0;
        drc->cr[1] = 500.0;
        drc->cr[2] = 250.0;
        drc->cr[3] = 125.0;
        drc->cr[4] = 100.0;
        drc->cr[5] = 80.0;
        drc->cr[6] = 60.0;
        drc->cr[7] = 50.0;
        drc->cr[8] = 40.0;
        drc->cr[9] = 30.0;
        drc->cr[10] = 20.0;
        drc->cr[11] = 15.0;
        drc->cr[12] = 10.0;
        drc->cr[13] = 5.0;
    }
    else
        drc = NULL;
}

void
fezw(int *NumRec, Fimage Edge_Ri, Fsignal Ri2, int *FilterNorm,
     float *WeightFac, int *DistRate, float *Rate, float *PSNR,
     Polygons SelectedArea, Cimage Output, Fimage Image, Fsignal Ri,
     Fimage QImage, char *PtrDRC)
        /*--- Computes the orthogonal wavelet transform of image `Image` ---*/
                                /* Number of recursion (-j) */
                                /* Impulse responses of filters for special
                                 * edge processing (including preconditionning
                                 * matrices */
                                /* Impulse response of the low pass filter */
                                /* for synthesis */
                                /* Equal 0 if no normalisation of filter's tap
                                 *       1 if normalisation of the sum
                                 *       2 if normalistion of the square sum */
                                /* Weighting factor for wavelet coeff. */
                                /* Compute distortion-rate function */
                                /* Target Rate */
                                /* Target PSNR */
                                /* Polygnal regions to be encoded with
                                 * a special rate or PSNR */
                                /* Compressed `Image` */
                                /* Input image */
                                /* Impulse response of the low pass filter */
                                /* Output quantized image */
                                /* Distorsion rate curve */
{
    int J, Jx, Jy;              /* Current level of decomposition */
    int x;
    float targrate;             /* Target rate for vector quantization */
    Wtrans2d Wtrans = NULL;     /* Wavelet transform of Image */
    Wtrans2d QWtrans = NULL;    /* Quantized wavelet transform */
    Fimage QImage_dr;           /* Reconstructed image (for biorthogonal
                                 * wavelet transform and dist.rate curve) */
    int Haar;                   /* Continue decomposition with Haar wavelet
                                 * until haar level */
    int Edge;                   /* Edge processing mode */
    int FiltNorm;               /* Normalisation of filters */
    int Precond;                /* Preconditionning mode for orthogonal
                                 * transform */
    int Max_Count_AC;           /* Size of histogram for arithm. coding */
    double mse;                 /* Mean square error between
                                 * Image and Qimage_dr */
    double mrd;                 /* Maximal relative difference */
    double snr;                 /* Signal to noise ratio / `Image` */
    double psnr;                /* Peak signal to noise ratio / `Image` */
    char PsnrFlg = '1';         /* To compute 255-PSNR */

  /*--- Compute number of level for wavelet transform ---*/

    Jx = 0;
    x = Image->ncol;
    while (x > 1)
    {
        Jx++;
        x /= 2;
    }
    if (NumRec)
        if (*NumRec < Jx)
            Jx = *NumRec;

    Jy = 0;
    x = Image->nrow;
    while (x > 1)
    {
        Jy++;
        x /= 2;
    }

    if (Jy > Jx)
        J = Jx;
    else
        J = Jy;
    Haar = J;

  /*--- Wavelet decomposition ---*/

    if (Ri2)
    {
        Edge = 2;
        if (FilterNorm)
            FiltNorm = *FilterNorm;
        else
            FiltNorm = 1;
        INIT_RI(Ri, Ri2);
        Wtrans = mw_new_wtrans2d();
        if (!Wtrans)
            mwerror(FATAL, 1, "Not enough memory to create Wtrans !\n");
        biowave2(&J, &Haar, &Edge, &FiltNorm, Image, Wtrans, Ri, Ri2);
        REFRESH_FILTERS(Ri, Ri2);
    }
    else
    {
        if (Edge_Ri)
            Edge = 3;
        else
            Edge = 2;
        if (FilterNorm)
            FiltNorm = *FilterNorm;
        else
            FiltNorm = 2;
        Precond = 0;
        Wtrans = mw_new_wtrans2d();
        if (!Wtrans)
            mwerror(FATAL, 1, "Not enough memory to create Wtrans !\n");
        owave2(&J, &Haar, &Edge, &Precond, NULL, &FiltNorm, Image, Wtrans, Ri,
               Edge_Ri);
    }

  /*--- Apply EZW algorithm ---*/

    Max_Count_AC = 256;

    if (DistRate && Ri2)
    {

        INIT_TARGNBIT_DR(Wtrans, PtrDRC);
        QImage_dr = mw_new_fimage();

        for (count_dr = 0; count_dr <= max_count_dr; count_dr++)
        {
            targrate = targrate_dr[count_dr];
            QWtrans = mw_new_wtrans2d();
            if (!QWtrans)
                mwerror(FATAL, 1, "Not enough memory to create QWtrans !\n");
            ezw(NULL, NULL, WeightFac, NULL, &Max_Count_AC, NULL, &targrate,
                NULL, SelectedArea, NULL, Wtrans, QWtrans, PtrDRC);

            REFRESH_FILTERS(Ri, Ri2);
            ibiowave2(&J, &Haar, &Edge, &FiltNorm, QWtrans, QImage_dr, Ri,
                      Ri2);
            fmse(Image, QImage_dr, NULL, &PsnrFlg, &snr, &psnr, &mse, &mrd);
            if (PtrDRC)
                drc->mse[count_dr] = mse;
            else
                printf("%d\t%.2f\n", (int) targcr_dr[count_dr], psnr);
            mw_delete_wtrans2d(QWtrans);
            QWtrans = NULL;
        }

        mw_delete_fimage(QImage_dr);
        QImage_dr = NULL;

    }
    else
    {
        QWtrans = mw_new_wtrans2d();
        if (!QWtrans)
            mwerror(FATAL, 1, "Not enough memory to create QWtrans !\n");
        ezw(NULL, NULL, WeightFac, NULL, &Max_Count_AC, DistRate, Rate, PSNR,
            SelectedArea, Output, Wtrans, QWtrans, PtrDRC);
    }

  /*--- Wavelet reconstruction ---*/

    if (QImage)
    {
        if (Ri2)
        {
            ibiowave2(&J, &Haar, &Edge, &FiltNorm, QWtrans, QImage, Ri, Ri2);
            REFRESH_FILTERS(Ri, Ri2);
        }
        else
        {
            iowave2(&J, &Haar, &Edge, &Precond, NULL, &FiltNorm, QWtrans,
                    QImage, Ri, Edge_Ri);
        }
    }

    if (Wtrans)
        mw_delete_wtrans2d(Wtrans);
    if (QWtrans)
        mw_delete_wtrans2d(QWtrans);
}

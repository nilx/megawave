/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {fezw};
version = {"1.31"};
author = {"Jean-Pierre D'Ales"};
function = {"Compress an image with EZW algorithm"};
usage = {
'r':NLevel->NumRec [1,20]
	"Number of level for wavelet tranform", 
'e':EdgeIR->Edge_Ri
      	"Impulse reponses of edge and preconditionning filters for orthogonal transform (fimage)",
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
        "Polygonal regions to be encoded with a different rate or PSNR (Polygons)",
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


/*--- Include files UNIX C ---*/

#include <stdio.h>
#include <math.h>

/*--- Megawave2 library ---*/

#include  "mw.h"

/*--- Megawave2 modules definition ---*/

#ifdef __STDC__
void ezw(int *, int *, float *, float *, int *, int *, float *, float *, Polygons, Cimage, Wtrans2d, Wtrans2d, char *);
void owave2(int *, int *, int *, int *, int *, int *, Fimage, Wtrans2d, Fsignal, Fimage);
void iowave2(int *, int *, int *, int *, int *, int *, Wtrans2d, Fimage, Fsignal, Fimage);
void biowave2(int *, int *, int *, int *, Fimage, Wtrans2d, Fsignal, Fsignal);
void ibiowave2(int *, int *, int *, int *, Wtrans2d, Fimage, Fsignal, Fsignal);
void fmse(Fimage, Fimage, int *, char *, double *, double *, double *, double *);
#else
void ezw();
void owave2();
void iowave2();
void biowave2();
void ibiowave2();
void fmse();
#endif

/*--- Constants ---*/

#define  NBIT_GREYVAL 8     /* Number of bit per pixels */
#define  MAX_GREYVAL 255.0  /* Maximum of grey levels in input image */
#define mw_drcurvesize 256

/*--- Special structures ---*/

typedef struct drcurve {
  int nrow;                    /* Number of rows (dy) */
  int ncol;                    /* Number of columns (dx) */
  int nbitplanes;              /* Number of bit per pixel */
  int npoints;                 /* Number of points in the curve */
  float rate[mw_drcurvesize];  /* Bit rates */
  float cr[mw_drcurvesize];    /* Compression ratios */
  float mse[mw_drcurvesize];   /* Mean square error for correponding bit rate
				* and compression ratio */
  
} DRCurve;        /* Distortion rate curves */


/*--- Global variables ---*/ 

static float        targrate_dr[20];
static float        targcr_dr[20];
static int          count_dr;
static int          max_count_dr;
static DRCurve     *drc;                 /* Distorsion rate curve */

static Fsignal      ORI1, ORI2;          /* Non normalized filters */



static void
INIT_RI(ri1, ri2)

Fsignal ri1, ri2;

{
  int i;

  ORI1 = mw_new_fsignal();
  if (mw_alloc_fsignal(ORI1, ri1->size) == NULL)
    mwerror(FATAL, 1, "Not enough memory for ri1!\n");

  ORI2 = mw_new_fsignal();
  if (mw_alloc_fsignal(ORI2, ri2->size) == NULL)
    mwerror(FATAL, 1, "Not enough memory for ri2!\n");

  for (i=0; i<ri1->size; i++)
    ORI1->values[i] = ri1->values[i];

  for (i=0; i<ri2->size; i++)
    ORI2->values[i] = ri2->values[i];

}



static void
REFRESH_FILTERS(ri1, ri2)

Fsignal ri1, ri2;

{
  int i;


  for (i=0; i<ri1->size; i++)
    ri1->values[i] = ORI1->values[i];

  for (i=0; i<ri2->size; i++)
    ri2->values[i] = ORI2->values[i];

}



static void
INIT_TARGNBIT_DR(wtrans, ptrdrc)

Wtrans2d      wtrans;
char         *ptrdrc;

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

  if (ptrdrc) {
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
  } else
  drc = NULL;
}



void
fezw(NumRec, Edge_Ri, Ri2, FilterNorm, WeightFac, DistRate, Rate, PSNR, SelectedArea, Output, Image, Ri, QImage, PtrDRC)

	/*--- Computes the orthogonal wavelet transform of image `Image` ---*/

int        *NumRec;		/* Number of recursion (-j) */
Fimage	    Edge_Ri;		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
Fsignal     Ri2;		/* Impulse response of the low pass filter */
				/* for synthesis */
int        *FilterNorm;	        /* Equal 0 if no normalisation of filter's tap
			         *       1 if normalisation of the sum 
			         *       2 if normalistion of the square sum */
float      *WeightFac;          /* Weighting factor for wavelet coeff. */
int        *DistRate;           /* Compute distortion-rate function */
float      *Rate;               /* Target Rate */
float      *PSNR;               /* Target PSNR */
Polygons    SelectedArea;       /* Polygnal regions to be encoded with 
				 * a special rate or PSNR */
Cimage      Output;		/* Compressed `Image` */
Fimage      Image;		/* Input image */
Fsignal     Ri;			/* Impulse response of the low pass filter */
Fimage      QImage;		/* Output quantized image */
char       *PtrDRC;             /* Distorsion rate curve */

{
  int         J, Jx, Jy;       	/* Current level of decomposition */
  int         x;
  float       targrate;         /* Target rate for vector quantization */
  Wtrans2d    Wtrans, QWtrans;  /* Wavelet transform of Image, quantized 
				 * wavelet transform */
  Fimage      QImage_dr;        /* Reconstructed image (for biorthogonal 
				 * wavelet transform and dist.rate curve) */
  int         Haar;             /* Continue decomposition with Haar wavelet
				 * until haar level */
  int         Edge;	        /* Edge processing mode */
  int         FiltNorm;	        /* Normalisation of filters */
  int         Precond;          /* Preconditionning mode for orthogonal 
				 * transform */
  int         Max_Count_AC;     /* Size of histogram for arithm. coding */
  double      mse;	    /* Mean square error between Image and Qimage_dr */
  double      mrd;	    /* Maximal relative difference */	
  double      snr;	    /* Signal to noise ratio / `Image` */
  double      psnr;	    /* Peak signal to noise ratio / `Image` */
  char PsnrFlg = '1';       /* To compute 255-PSNR */

  /*--- Compute number of level for wavelet transform ---*/

  Jx = 0;
  x = Image->ncol;
  while (x > 1) {
    Jx++;
    x /= 2;
  }
  if (NumRec)
    if (*NumRec < Jx)
      Jx = *NumRec;
  
  Jy = 0;
  x = Image->nrow;
  while (x > 1) {
    Jy++;
    x /= 2;
  }

  if (Jy > Jx)
    J = Jx;
  else
    J = Jy;
  Haar = J;

  /*--- Wavelet decomposition ---*/

  if (Ri2) {
    Edge = 2;
    if (FilterNorm)
      FiltNorm = *FilterNorm;
    else
      FiltNorm = 1;
    INIT_RI(Ri, Ri2);
    Wtrans = mw_new_wtrans2d();
    biowave2(&J, &Haar, &Edge, &FiltNorm, Image, Wtrans, Ri, Ri2);
    REFRESH_FILTERS(Ri, Ri2);
  } else
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
      owave2(&J, &Haar, &Edge, &Precond, NULL, &FiltNorm, Image, Wtrans, Ri, Edge_Ri);
    }

  /*--- Apply EZW algorithm ---*/

  Max_Count_AC = 256;

  if (DistRate && Ri2) {

    INIT_TARGNBIT_DR(Wtrans, PtrDRC);    
    QImage_dr = mw_new_fimage();

    for (count_dr = 0; count_dr <= max_count_dr; count_dr++) {
      targrate = targrate_dr[count_dr];
      QWtrans = mw_new_wtrans2d();
      ezw(NULL, NULL, WeightFac, NULL, &Max_Count_AC, NULL, &targrate, NULL, SelectedArea, NULL, Wtrans, QWtrans, PtrDRC);

      REFRESH_FILTERS(Ri, Ri2);
      ibiowave2(&J, &Haar, &Edge, &FiltNorm, QWtrans, QImage_dr, Ri, Ri2);
      fmse(Image, QImage_dr, NULL, &PsnrFlg, &snr, &psnr, &mse, &mrd);
      if (PtrDRC) 
	drc->mse[count_dr] = mse;
      else
	printf("%d\t%.2f\n", (int) targcr_dr[count_dr], psnr);
      mw_delete_wtrans2d(QWtrans);
    }

    mw_delete_fimage(QImage_dr);

  } else
    {
      QWtrans = mw_new_wtrans2d();
      ezw(NULL, NULL, WeightFac, NULL, &Max_Count_AC, DistRate, Rate, PSNR, SelectedArea, Output, Wtrans, QWtrans, PtrDRC);
    }

  /*--- Wavelet reconstruction ---*/

  if (QImage)
    if (Ri2) {
      ibiowave2(&J, &Haar, &Edge, &FiltNorm, QWtrans, QImage, Ri, Ri2);
      REFRESH_FILTERS(Ri, Ri2);
    } else
      {
	iowave2(&J, &Haar, &Edge, &Precond, NULL, &FiltNorm, QWtrans, QImage, Ri, Edge_Ri);
      }
  

  mw_delete_wtrans2d(Wtrans);
  mw_delete_wtrans2d(QWtrans);
}

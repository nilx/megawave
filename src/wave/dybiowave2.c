/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {dybiowave2};
version = {"1.1"};
author = {"Jean-Pierre D'Ales"};
function = {"Computes the biorthogonal wavelet coefficients of an image without decimation"};
usage = {
'r':[NLevel=1]->NumRec [1,20]
	"Number of levels (default 1)", 
'd':[StopDecimLevel=2]->StopDecim [1,20]
	"Level for decimation stop (default 2)",
'o'->Ortho
	"Computes orthogonal coefficients", 
'e':[EdgeMode=2]->Edge [0,2]
	"Edge processing mode (0/1/2, default 2)", 
'n':[FiltNorm=0]->FilterNorm
	"Normalization mode for filter bank (default 0)", 
Image->Image
	"Input image (fimage)", 
WavTrans<-Output
	"Wavelet transform of Image (wtrans2d)", 
ImpulseResponse1->Ri1
	"Impulse response of filter 1 (fsignal)", 
ImpulseResponse2->Ri2
	"Impulse response of filter 2 (fsignal)"
	};
 */


/*--- Include files UNIX C ---*/
#include <stdio.h>
#include <math.h>

/*--- Megawave2 library ---*/
#include  "mw.h"

/*--- Megawave2 modules definition ---*/

extern void sconvolve();

/*--- Constants ---*/

static int DECIM=2;		/* Decimation's constant */
static int NOTDECIM = 1;
static int INTERPOL=1;		/* Interpolation rate */
static int REFLIR=1;	        /* Indicator for reflexion of filter's ir */
static int HIGH=1;		/* Index for high-pass filtering */
static int PROLONG=0;		/* Index for inverse preconditionning */



static void
COMMENT(result, image, edge, filternorm, ri1, ri2)

	/*--- Fill comment and other fields for result ---*/

    Wtrans2d    result;		/* Wavelet transform of the image `Image` */
    Fimage      image;		/* Input image */
    int 	edge;		/* Type of edge processing 
				 * (see `Edge` in `biowave2`) */
    int	       *filternorm;	/* Type of normalisation :
				 * equal 0 if normalisation of the sum of 
				 *         `ri`'s coefficients to 1.0
			 	 *       1 if normalisation of the squares' sum 
			 	 *         `ri`'s coefficients to 1.0 */
    Fsignal	ri1, ri2;	/* Impulse responses of the low-pass filters */

{
    result->edges = edge;
    result->type = mw_biorthogonal;
    result->nfilter = 2;
    strcpy(result->filter_name[0], ri1->name);
    strcpy(result->filter_name[1], ri2->name);
    strcpy(result->cmt, "Wav. Trans. of ");
    strcat(result->cmt, image->name);
    if(*filternorm >= 1)
	strcat(result->cmt, ", filters coef. norm.");

}



static void
COMLINE_ERR(ri1, ri2, edge, numrec, dy, dx)

	/*--- Detects errors and contradiction in command line ---*/

    Fsignal	ri1, ri2;	/* Impulse responses of the low-pass filters */
    int 	edge;		/* Type of edge processing 
				 * (see `Edge` in biowave2) */
    short	numrec;		/* Number of levels for decomposition */
    long	dx, dy;		/* Size of the image */

{

	/*--- Size of image ---*/

    if((dx>>(numrec - 1) < ri1->size) || (dy>>(numrec - 1) < ri1->size))
	mwerror(FATAL, 1, "The size of image must be greater than N where N is the size of the i.r.!\n");

    if((dx>>(numrec - 1) < ri2->size) || (dy>>(numrec - 1) < ri2->size))
	    mwerror(FATAL, 1, "The size of image must be greater than N where N is the size of the i.r.!\n");

    if (((dx % (1 << numrec)) != 0) || ((dy % (1 << numrec)) != 0))
	mwerror(FATAL, 1, "The horizontal and vertical dimension of image are not a multiple of 2^J!\n");

}




static void
ALLOC_WAV(wtrans, image, numrec, stopdecim)

Wtrans2d    wtrans;
Fimage      image;
int         numrec;
int         stopdecim;

{
  int             i, J;
  int             nrow, ncol;

  nrow = 2;
  ncol = 2;
    
  for (J = 1; J <= numrec; J++) {
    nrow *= 2;
    ncol *= 2;
  }
  if(mw_alloc_biortho_wtrans2d(wtrans, numrec, nrow, ncol)==NULL)
    mwerror(FATAL, 1, "Memory allocation refused for `WavTrans`!\n");

  nrow = image->nrow;
  ncol = image->ncol;

  for (J = 1; J <= numrec ; J++) {
    if (J < stopdecim) {
      nrow /= 2;
      ncol /= 2;
    }
    for (i = 0; i <= 3; i++) {
      if (mw_change_fimage(wtrans->images[J][i], nrow, ncol) == NULL)
	mwerror(FATAL, 1, "Not enough memory for wavelet transform buffer.\n");
    }
  }

  wtrans->images[0][0] = image;
  wtrans->nrow = image->nrow;
  wtrans->ncol = image->ncol;

}


static void
NORM_FIL(ri1, ri2, filternorm)

	/*--- Normalisation of the coefficients of the filter impulse 
	 *--- responses ---*/

    Fsignal	ri1, ri2;	/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
    int		*filternorm;	/* Type of normalisation :
				 * equal 0 if normalisation of the sum of 
				 *         `ri`'s coefficients to 1.0
			 	 *       1 if normalisation of the squares' sum 
			 	 *         `ri`'s coefficients to 1.0 */

{
    double	s1,s2;		/* Sums of coefficients of `ri1` and `ri2` */
    double	s;		/* Cross correlation of `ri1` and `ri2` */
    int         test;           /* Indicates biorthogonality of filters */
    short	i,j;
    short	shift1, shift2, shift, size;


  /*--- Test for biorthogonality of filters ---*/

  shift1 = (short) ri1->shift;
  shift2 = (short) ri2->shift;
  if(shift1 > shift2)
    shift = shift1;
  else
    shift = shift2;
  if(shift1 + ri1->size < shift2 + ri2->size)
    size = ri1->size;
  else
    size = ri2->size;

  test = 0;
  for (j=1; j <= - (long)  (shift1 + shift2) / 2.0; j++) {
    s = 0.0;
    for (i = shift; i < shift + size; i++)
      if (i - shift2 + 2 * j < ri2->size)
	s += ri1->values[i - shift1] * ri2->values[i - shift2 + 2 * j];
    if (s > 0.00001)
      test = 1;
  }

  if (test == 1)
    mwerror(WARNING, 0, "Filter bank is not biorthogonal!\n");

  /*--- Normalisation of cross correlation of filters (if selected) ---*/

  s = 0.0;
  if (*filternorm != 0) {
    if (*filternorm == 1) {

      /*--- Normalisation of the sum of the coefficients ---*/

      s1 = s2 = 0.0;
      for (i = 0; i < ri1->size; i++)
	s1 += ri1->values[i];
      for (i = 0; i < ri2->size; i++)
	s2 += ri2->values[i];
    } else
      {

	/*--- Normalisation of the sum of the coefficients' square ---*/

	s1 = s2 = 0.0;
	for (i = 0; i < ri1->size; i++)
	  s1 += ri1->values[i] * ri1->values[i];
	for (i = 0; i < ri2->size; i++)
	  s2 += ri2->values[i] * ri2->values[i];

	s1 = sqrt(s1);
	s2 = sqrt(s2);
      }

    for (i = 0; i < ri1->size; i++)
      ri1->values[i] /= s1;
    for (i = 0; i < ri2->size; i++)
      ri2->values[i] /= s2;

    for (i = shift; i < shift + size; i++)
      s += ri1->values[i - shift1] * ri2->values[i - shift2];
    s = sqrt(s);

    for (i = 0; i < ri1->size; i++)
      ri1->values[i] /= s;
    for (i = 0; i < ri2->size; i++)
      ri2->values[i] /= s;
  }

}



static void
COLUMN_WAVEL(Im, Im1, Im2, decim, nsubim, edge, ri1, ri2)

	/*--- Computes the 1-D wavelet transform of each column  ---*
	 *--- in image "Tab", puts the result in "Im1" and "Im2" ---*/

Fimage	    Im;		/* Input (wavelet transform along the lines
			 * of image or resume at level J-1) */
Fimage      Im1, Im2;	/* Low and high-pass filtered sub-images */
int         decim;      /* Flag of decimation for sconvolve */
int         nsubim;     /* Number of subimages */
int        *edge;	/* Type of edge processing (see `Edge` in biowave2) */
Fsignal     ri1, ri2;	/* Impulse responses of the low-pass filters */

{
  long        c, l;	/* Variables for line and column in resume or detail */
  long	      ldxc;	/* Index of element at line `l` and column `c` 
			 * in Im, Im1, or Im2 */
  long	      dx, dy;   /* Size of input (wavelet transform along the 
			 * lines of image or resume at level J-1) */
  long        dys;      /* Size of subimages */
  long        L2, L2s;
  int         s;        /* index of subimage */
  long        osi, osr;      /* origin of subimage in Im (Im1, Im2) */
  long        nsubdx;
  Fsignal     Tabin;	/* One column of Im, input for `convo_sig` */
  Fsignal     Tabout;	/* Output for module `sconvolve`, 
					 * wavelet transform of `Tabin` */

  dx = Im->ncol;
  dy = Im->nrow;
  dys = dy / nsubim;
  if (decim == 2) {
    L2 = dy / 2;
    L2s = dys / 2;
  } else
    {
      L2s = dys;
      L2 = dy;
    }
  nsubdx = nsubim * dx;

  /*--- Initialization of Tabin ---*/

  Tabin = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabin, dys) == NULL)
    mwerror(FATAL, 1, "Allocation of column buffer refused!\n");

  Tabout = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabout, L2s) == NULL)
    mwerror(FATAL, 1, "Allocation of column buffer refused!\n");

  if ((dx != Im1->ncol) || (dx != Im2->ncol))
    mwerror(WARNING, 0, "Bad number of columns in subimages!\n");

  for (s = 0; s < nsubim; s++) {
    osi = s * dx;
    osr = s * dx;
    for (c = 0; c < dx; c++) {
      ldxc = osi + c;
      for (l=0; l<dys; l++) {
	Tabin->values[l] = Im->gray[ldxc];
	ldxc += nsubdx;
      }

      /*--- Convolution with low-pass filter ---*/

      sconvolve(Tabin, Tabout, &decim, &INTERPOL, &REFLIR, NULL, edge, &PROLONG, ri2, NULL);

      ldxc = osr + c;
      for (l=0; l<L2s; l++) {
	Im1->gray[ldxc] = Tabout->values[l];
	ldxc += nsubdx;
      }

      /*--- Convolution with high-pass filter ---*/

      sconvolve(Tabin, Tabout, &decim, &INTERPOL, &REFLIR, &HIGH, edge, &PROLONG, ri1, NULL);

      ldxc = osr + c;
      for (l=0; l<L2s; l++) {
	Im2->gray[ldxc] = Tabout->values[l];
	ldxc += nsubdx;
      }
    }
  }

  mw_delete_fsignal(Tabin);
  mw_delete_fsignal(Tabout);

}




static void
WAVEL(wtrans, J, stopdecim, ortho, ri1, ri2, edge)

	/*----- Computes the wavelet decomposition of S -----*/

Wtrans2d    wtrans;		/* Wavelet transform */
int         J;		        /* Level of decomposition */
int         stopdecim;          /* Level where decimation is cancelled */
int        *ortho;              /* Computes orthogonal coefficients */
Fsignal     ri1, ri2;	        /* Impulse responses of the low-pass filters */
int        *edge;		/* Type of edge processing (see `Edge`
				 * in biowave2) */

{
  int         decim;            /* Flag of decimation for sconvolve */
  int         nsubim;           /* Number of subimages */
  long        c, l;		/* Indices for line and column in resume 
					 * or detail */
  long	      ldx, lK2;		/* Index of first element of line `l` */ 
  long	      dx, dy;		/* Size of image */
  long        K2, K2s;
  long        dxs;              /* Size of subimages */
  int         s;                /* index of subimage */
  long        osi, osr;         /* origin of subimage in Im (Im1, Im2) */
  long        dxK2s;
  Fsignal     Tabin;		/* One line of wtrans->image[J-1][0], 
					 * input for `sconvolve` */
  Fsignal     Tabout;		/* Output for module `sconvolve`, 
					 * wavelet transform of `Tabin` */
  Fimage	Tab;		/* Buffer for sum's separation */

  nsubim = 1;
  if (ortho)
    for (s = stopdecim; s < J; s++)
      nsubim *= 2;

  dx = wtrans->images[J-1][0]->ncol;
  dy = wtrans->images[J-1][0]->nrow;
  dxs = dx / nsubim;
  K2s = dxs / 2;
  K2 = dx / 2;
  if (J < stopdecim) {
    K2s = dxs / 2;
    K2 = dx / 2;
    decim = DECIM;
  } else
    {
      K2s = dxs;
      K2 = dx;
      decim = NOTDECIM;
    }

  /*--- Initialization of Tabin and Tab ---*/

  Tabin = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabin,dxs) == NULL)
    mwerror(FATAL, 1, "Allocation for line's buffer refused!\n");

  Tabout = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabout, K2s) == NULL)
    mwerror(FATAL, 1, "Allocation of column buffer refused!\n");

  Tab = mw_new_fimage();
  if (mw_alloc_fimage(Tab, dy, K2) == NULL)
    mwerror(FATAL, 1, "Allocation for sum's separation buffer refused!\n");

  /*--- Conmputation of S and D1 ---*/

  /*--- Convolution of rows with low-pass filter ---*/

  for (s = 0; s < nsubim; s++) {
    ldx = s;
    lK2 = s;
    for (l = 0; l < dy; l++) {
      for(c = 0; c < dxs; c++)
	Tabin->values[c] = wtrans->images[J-1][0]->gray[ldx + nsubim * c];

      sconvolve(Tabin, Tabout, &decim, &INTERPOL, &REFLIR, NULL, edge, &PROLONG, ri2, NULL);

      for (c = 0; c < K2s; c++)
	Tab->gray[lK2 + nsubim * c] = Tabout->values[c];

      ldx += dx;
      lK2 += K2;
    }
  }

  /*--- Convolution of column with low and high-pass filters ---*/

  COLUMN_WAVEL(Tab, wtrans->images[J][0], wtrans->images[J][1], decim, nsubim, edge, ri1, ri2);



  /*--- Conmputation of D2 and D3 ---*/

  /*--- Convolution of rows with high-pass filter ---*/

  for (s = 0; s < nsubim; s++) {
    ldx = s;
    lK2 = s;
    for (l = 0; l < dy; l++) {
      for(c = 0; c < dxs; c++)
	Tabin->values[c] = wtrans->images[J-1][0]->gray[ldx + nsubim * c];

      sconvolve(Tabin, Tabout, &decim, &INTERPOL, &REFLIR, &HIGH, edge, &PROLONG, ri1, NULL);

      for (c = 0; c < K2s; c++)
	Tab->gray[lK2 + nsubim * c] = Tabout->values[c];

      ldx += dx;
      lK2 += K2;
    }
  }

  /*--- Convolution of columns with low and high-pass filters ---*/

  COLUMN_WAVEL(Tab, wtrans->images[J][2], wtrans->images[J][3], decim, nsubim, edge, ri1, ri2);

  mw_delete_fimage(Tab);
  mw_delete_fsignal(Tabin);
  mw_delete_fsignal(Tabout);  

}




void
dybiowave2(NumRec, StopDecim, Ortho, Edge, FilterNorm, Image, Output, Ri1, Ri2)

  /*--- Computes the biorthogonal wavelet coefficients of image `Image` ---*/

int        *NumRec;	    /* Number of iteration (-j) */
int        *StopDecim;      /* Level where decimation is cancelled */
int        *Ortho;          /* Computes orthogonal coefficients */
int        *Edge;	    /* Equal 0 (default) if extension with 0 */
			    /* 1 if periodization */
			    /* 2 if reflexion */
int        *FilterNorm;	    /* Equal 1 if normalisation of filter's
				 * impulse responses (sum = 1.0) */
Fimage      Image;	    /* Input image */
Wtrans2d    Output;	    /* Wavelet transform of the image `Image` */
Fsignal     Ri1, Ri2;	    /* Impulse responses of the low pass filters */

{
  int         J;	      /* Current level of decomposition */

  /*--- Detection of errors in command line ---*/

  COMLINE_ERR(Ri1, Ri2, *Edge, *NumRec, Image->nrow, Image->ncol);

  /*--- Normalisation of filter banks ---*/

  NORM_FIL(Ri1, Ri2, FilterNorm);

  /*--- Memory allocation for wavelet transform Output ---*/

  ALLOC_WAV(Output, Image, *NumRec, *StopDecim);

  /*--- Write commentary for Output ---*/

  COMMENT(Output, Image, *Edge, FilterNorm, Ri1, Ri2);

  /*--- Wavelet decomposition ---*/

  for (J = 1; J <= *NumRec; J++)
    WAVEL(Output, J, *StopDecim, Ortho, Ri1, Ri2, Edge);
}

/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {biowave1};
version = {"1.3"};
author = {"Jean-Pierre D'Ales"};
function = {"Computes the biorthogonal wavelet transform of an 1D signal"};
usage = {
'r':[RecursNum=1]->NumRec [1,20]
	"Number of levels (default 1)", 
'e':[EdgeMode=2]->Edge [0,2]
	"Edge processing mode (0/1/2, default 2)", 
'n':[FilterNorm=0]->FilterNorm [0,2]
	"Normalization mode for filter bank (0/1/2, default 0)", 
Signal->Signal
	"Input signal (fsignal)", 
WavTrans<-Output
	"Wavelet transform of Signal (wtrans1d)", 
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

/*--- Megawave2 modules ---*/
extern void sconvolve();

/*--- Constants ---*/

static int DECIM=2;		/* Decimation's constant */
static int INTERPOL=1;		/* Interpolation rate */
static int REFLIR=1;	        /* Indicator for reflexion of filter's ir */
static int HIGH=1;		/* Index for high-pass filtering */
static int PROLONG=0;		/* Index for inverse preconditionning */




static void
COMLINE_ERR(ri1, ri2, edge, numrec, size)

	/*--- Detects errors and contradiction in command line ---*/

    Fsignal     ri1;		/* Impulse response of the low-pass filter 
			 	 * for decomposition */
    Fsignal     ri2;		/* Impulse responses of the low-pass filter 
			 	 * for synthesis */
    int 	edge;		/* Type of edge processing 
				 * (see `Edge` in biowave1) */
    short	numrec;		/* Number of levels for decomposition */
    long	size;		/* Size of the signal */

{


  /*--- Size of signal ---*/

  if((size>>(numrec - 1) < ri1->size) || (size>>(numrec - 1) < ri2->size))
    mwerror(FATAL, 1, "The size of signal must be greater than N where N is the size of the i.r.!\n");

  if ((edge == 1) && ((size % (1 << numrec)) != 0))
    mwerror(FATAL, 1, "The size of the signal is not a multiple of 2^J!\n");

}




static void
COMMENT(result, signal, edge, filternorm, ri1, ri2)

	/*--- Fill comment and other fields for result ---*/

    Wtrans1d    result;		/* Wavelet transform of the signal */
    Fsignal     signal;		/* Input signal */
    int 	edge;		/* Type of edge processing 
				 * (see `Edge` in `biowave1`) */
    int	       *filternorm;	/* Type of normalisation :
				 * equal 0 if normalisation of the sum of 
				 *         `ri`'s coefficients to 1.0
			 	 *       1 if normalisation of the squares' sum 
			 	 *         `ri`'s coefficients to 1.0 */
    Fsignal     ri1;		/* Impulse response of the low-pass filter 
			 	 * for decomposition */
    Fsignal     ri2;		/* Impulse responses of the low-pass filter 
			 	 * for synthesis */

{
  result->edges = edge;
  result->type = mw_biorthogonal;
  result->nfilter = 2;
  strcpy(result->filter_name[0], ri1->name);
  strcpy(result->filter_name[1], ri2->name);
  strcpy(result->cmt, "Wav. Trans. of ");
  strcat(result->cmt, signal->name);
  if(*filternorm >= 1)
    strcat(result->cmt, " filters coef. norm.");

}





static void
NORM_FIL(ri1, ri2, filternorm)

	/*--- Normalisation of the coefficients of the filter impulse 
	 *--- responses ---*/

    Fsignal	ri1, ri2;	/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
    int		filternorm;	/* Type of normalisation :
				 * equal 0 if normalisation of the sum of 
				 *         `ri`'s coefficients to 1.0
			 	 *       1 if normalisation of the squares' sum 
			 	 *         `ri`'s coefficients to 1.0 */

{
  double	s1,s2;		 /* Sums of coefficients of `ri1` and `ri2` */
  double	s;		 /* Cross correlation of `ri1` and `ri2` */
  int         test;	         /* Indicates biorthogonality of filters */
  short	i,j;
  short	shift1, shift2;



  /*--- Test for biorthogonality of filters ---*/

  shift1 = (short) ri1->shift;
  shift2 = (short) ri2->shift;
  test = 0;
  for (j=2; j < (long) shift2 - shift1 + ri2->size; j += 2) {
    s = 0.0;
    for (i = shift1; i < shift1 + ri1->size; i++)
      if ((i - shift2 + j < ri2->size) && (i - shift2 + j >= 0))
	s += ri1->values[i - shift1] * ri2->values[i - shift2 + j];
    if (s > 0.00001)
      test = 1;
  }

  for (j = -2; j >= (long) shift2 - shift1 - ri1->size; j -= 2) {
    s = 0.0;
    for (i = shift1; i < shift1 + ri1->size; i++)
      if ((i - shift2 + j < ri2->size) && (i - shift2 + j >= 0))
	s += ri1->values[i - shift1] * ri2->values[i - shift2 + j];
    if (s > 0.00001)
      test = 1;
  }

  if (test == 1) {
    mwerror(WARNING, 0, "Filter bank is not biorthogonal!\n");
    for (j=2; j < (long) shift2 - shift1 + ri2->size; j += 2) {
      s = 0.0;
      for (i = shift1; i < shift1 + ri1->size; i++)
	if ((i - shift2 + j < ri2->size) && (i - shift2 + j >= 0))
	  s += ri1->values[i - shift1] * ri2->values[i - shift2 + j];
      printf("s = %.8f\n", s);
    }
    for (j = -2; j >= (long) shift2 - shift1 - ri1->size; j -= 2) {
      s = 0.0;
      for (i = shift1; i < shift1 + ri1->size; i++)
	if ((i - shift2 + j < ri2->size) && (i - shift2 + j >= 0))
	  s += ri1->values[i - shift1] * ri2->values[i - shift2 + j];
      printf("s = %.8f\n", s);
    }
  }

  /*--- Normalisation of cross correlation of filters (if selected) ---*/

  s = 0.0;
  if (filternorm != 0) {
    if (filternorm == 1) {

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

    for (i = shift1; i < shift1 + ri1->size; i++)
      if ((i - shift2 < ri2->size) && (i - shift2 >= 0))
	s += ri1->values[i - shift1] * ri2->values[i - shift2];
    s = sqrt(s);

    for (i = 0; i < ri1->size; i++)
      ri1->values[i] /= s;
    for (i = 0; i < ri2->size; i++)
      ri2->values[i] /= s;
  }

}




static void
WAVEL(wtrans, J, ri1, ri2, edge)

	/*----- Computes the wavelet decomposition of S -----*/

Wtrans1d    wtrans;		/* Wavelet transform */
int         J;		        /* Level of decomposition */
Fsignal     ri1;		/* Impulse response of the low-pass filter 
			 	 * for decomposition */
Fsignal     ri2;		/* Impulse responses of the low-pass filter 
			 	 * for synthesis */
int        *edge;		/* Type of edge processing (see `Edge`
				 * in biowave1) */

{

  /*----- Computation of average at level J -----*/

  sconvolve(wtrans->A[J-1][0], wtrans->A[J][0], &DECIM, &INTERPOL, &REFLIR, NULL, edge, &PROLONG, ri2,NULL);

  /*----- Computation of detail at level J -----*/

  sconvolve(wtrans->A[J-1][0], wtrans->D[J][0], &DECIM, &INTERPOL, &REFLIR, &HIGH, edge, &PROLONG, ri1, NULL);

}




void
biowave1(NumRec, Edge, FilterNorm, Signal, Output, Ri1, Ri2)

    /*--- Computes the orthogonal wavelet transform of signal `Signal` ---*/

int        *NumRec;		/* Number of recursion (-j) */
int        *Edge;		/* Equal 0 (default) if extension with 0 
				 * 1 if periodization 
				 * 2 if reflexion */
int        *FilterNorm;	        /* Equal 1 if normalisation of filter's
				 * impulse responses (sum = 1.0) */
Fsignal     Signal;		/* Input signal */
Wtrans1d    Output;		/* Wavelet transform of the signal `Signal` */
Fsignal     Ri1;		/* Impulse response of the low pass filter 
				 * for decomposition */
Fsignal     Ri2;		/* Impulse response of the low pass filter */
				/* for synthesis */

{
  int         J;			/* Current level of decomposition */


  /*--- Detection of errors in command line ---*/

  COMLINE_ERR(Ri1, Ri2, *Edge, *NumRec, Signal->size);

  /*--- Normalisation of filter banks ---*/

  NORM_FIL(Ri1, Ri2, FilterNorm);

  /*--- Memory allocation for wavelet transform Output ---*/

  if(mw_alloc_ortho_wtrans1d(Output, *NumRec, Signal->size)==NULL)
    mwerror(FATAL, 1, "Allocation for wavelet transform refused!\n");

  Output->A[0][0] = Signal;

  /*--- Write commentary for Output ---*/

  COMMENT(Output, Signal, *Edge, FilterNorm, Ri1, Ri2);

  /*--- Wavelet decomposition ---*/

  for (J = 1; J <= *NumRec; J++)
    WAVEL(Output, J, Ri1, Ri2, Edge);

}

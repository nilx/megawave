/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {owave1};
version = {"1.3"};
author = {"Jean-Pierre D'Ales"};
function = {"Computes the orthogonal wavelet transform of an 1D signal"};
usage = {
 'r':[RecursNum=1]->NumRec [1,20]     "Number of levels", 
 'h':HaarLevel->Haar  	 "Continue decomposition with Haar until HaarLevel",
 'e':[EdgeMode=3]->Edge [0,3]         "Edge processing mode", 
 'p':[PrecondMode=0]->Precond [0,2]   "Edge preconditionning mode",
 'i'->Inverse                         "Invertible transform", 
 'n':[FilterNorm=2]->FilterNorm [0,2] "Filter taps normalization. 0: no normalization, 1: sum equal to 1.0, 2: squares sum equal to 1.0", 
 Signal->Signal       "Input signal", 
 WavTrans<-Output     "Wavelet transform of Signal", 
 ImpulseResponse->Ri  "Impulse response of inner filters", 
	{
	EdgeIR->Edge_Ri
	     "Impulse reponses of edge and preconditionning filters"
	}
	};
*/
/*----------------------------------------------------------------------
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for sconvolve(), precond1d() */

/*--- Constants ---*/

static int DECIM=2;		/* Decimation's constant */
static int INTERPOL=1;		/* Interpolation rate */
static int REFLIR=1;		/* Indicator for reflexion of filter's ir */
static int HIGH=1;		/* Index for high-pass filtering */
static int INVERSE=1;		/* Index for inverse preconditionning */
static int EDGE_HAAR=0;        /* Edge processing mode for haar filter */


static void
COMMENT(result, signal, edge, precond, filternorm, ri)

	/*--- Fill comment and other fields for result ---*/

Wtrans1d    result;		/* Wavelet transform of the signal `Signal` */
Fsignal     signal;		/* Input signal */
int 	edge;		        /* Type of edge processing 
				 * (see `Edge` in `owave1`) */
int		precond;	/* Type of edge preconditionning
				 * (see `Precond` in `owave1`) */
int	        filternorm;	/* Type of normalisation : see 'owave1' */
Fsignal	ri;		        /* Impulse response of the low-pass filter 
		       	 * (computation of the inner wavelet coefficients) */

{
  result->edges = edge;
  result->type = mw_orthogonal;
  result->nfilter = 1;
  strcpy(result->filter_name[0], ri->name);
  strcpy(result->cmt, "Wav. Trans. of ");
  strcat(result->cmt, signal->name);
  if(precond == 1)
    strcat(result->cmt, " with precond.");
  if(precond == 2)
    strcat(result->cmt, "/unprecond.");
  if(filternorm >= 1)
    strcat(result->cmt, " filters coef. norm.");

}




static void
COMLINE_ERR(ri, edge_ri, edge, precond, numrec, haar, inverse, size)

	/*--- Detects errors and contradiction in command line ---*/

Fsignal	ri;		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
Fimage	edge_ri;	/* Impulse responses of the edge filters
			 * (computation of wavelet coefficients near edges) */
int 	edge;		/* Type of edge processing 
			 * (see `Edge` in owave1) */
int	precond;	/* Type of edge preconditionning
			 * (see `Precond` in `owave1`) */
int    *numrec;		/* Number of levels for decomposition */
int    *haar;           /* Continue decomposition with Haar wavelet
			 * until ultimate level */
int    *inverse;	/* Performs invertible transform for edge
			 * processing mode 0 and 2 */
long	size;		/* Size of the signal */

{
    /*--- Invertible transform and edge processing mode ---*/

  if((edge_ri == NULL) && (edge == 3))
    mwerror(USAGE, 1, "Edge filters i.r. are needed!\n");

  if(((edge == 1) || (edge == 3)) && inverse)
    mwerror(WARNING, 0, "Edge processing mode is already invertible!\n");

    /*--- Preconditionning and special edge processing ---*/

  if((edge <= 2) && (precond >= 1))
    mwerror(USAGE, 1, "Preconditionning only with special edge processing\n");

    /*--- Size of image ---*/

  if(size>>(*numrec) < 1) {
    (*numrec)--;
    while (size>>(*numrec) < 1)
      (*numrec)--;
    mwerror(WARNING, 1, "The number of level is too big!\nNumRec is set to %d\n", *numrec);
  }

  if (*numrec <= 0)
    mwerror(FATAL, 1, "The size of signal must be greater than 2!\n");


  if (haar) {
    if(size>>(*haar) < 1) {
      (*haar)--;
      while (size>>(*haar) < 1)
	(*haar)--;
    }
  } else
    if(edge >= 1)
      if(size>>(*numrec) < ri->size) {
	(*numrec)--;
	while (size>>(*numrec) < ri->size)
	  (*numrec)--;
	mwerror(WARNING, 1, "The size of average must be greater than the size of the i.r.!\nNumRec is set to %d\n", *numrec);
      }

  if (((edge == 1) || (edge == 3)) && ((size % (1 << (*numrec))) != 0))
    mwerror(FATAL, 1, "The size of the signal is not a multiple of 2^NumRec!\n");

    /*--- Inner and edge filters compatibility ---*/

  if((edge == 3) && ((edge_ri->nrow != 4 * ri->size) || (edge_ri->ncol != 3 * ri->size / 2 - 1)))
    mwerror(FATAL, 1, "Inner and edge filters are not compatible!\n");

}




void
NORM_FIL(ri, edge_ri, filternorm, edge, haar_ri)

	/*--- Normalisation of the coefficients of the filter impulse 
	 *--- responses ---*/

    Fsignal	ri;		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
    Fimage	edge_ri;	/* Impulse responses of the edge filters
			 * (computation of wavelet coefficients near edges) */
    int	        filternorm;	/* Type of normalisation : see 'owave1' */
    int 	edge;		/* Type of edge processing 
				 * (see `Edge` in owave1) */
    Fsignal     haar_ri;        /* Impulse response for Haar filter */


{
  double	s,s2;	/* Sum of `ri`'s coefficients and sum's square root 
			 * of `ri`'s coefficients' squares */
  short	        i,j;



	/*--- Normalisation of the sum of the coefficients' squares ---*/

  if (filternorm >= 1) {
    s = s2 = 0.0;
    for (i = 0; i < ri->size; i++)
	s2 += ri->values[i] * ri->values[i];
    s = sqrt((double) s2);
    for (i = 0; i < ri->size; i++)
	ri->values[i] /= s;
  }

	/*--- Normalisation of the sum of the coefficients (if selected) ---*/

  s = 0.0;
  if (filternorm == 1) {
    for (i = 0; i < ri->size; i++)
      s += ri->values[i];
    for (i = 0; i < ri->size; i++)
      ri->values[i] /= s;
    if (edge == 3) {
      for (i = 0; i < edge_ri->nrow / 2; i++)
	for (j = 0; j < edge_ri->ncol; j++)
	  edge_ri->gray[edge_ri->ncol * i + j] /= s;
    }
  }

  if (haar_ri) {
    if (filternorm == 1) {
      haar_ri->values[0] = 0.5;
      haar_ri->values[1] = 0.5;
    } else
      {
	haar_ri->values[0] = sqrt((double) 0.5);
	haar_ri->values[1] = sqrt((double) 0.5);
      }
    haar_ri->shift = 0.0;
  }
}



void
HAAR_WAVEL1(wtrans, haar, filternorm)

    Wtrans1d    wtrans;		/* Wavelet transform */
    int         haar;           /* Continue decomposition with Haar wavelet
				 * until ultimate level */
    int	        filternorm;	/* Type of normalisation : see adapowave2 */

{
  int          j;
  int          numrec;
  int          size;
  int          size2;
  int          c;
  Fsignal      Tab;
  double       cnorm;

  numrec = j = wtrans->nlevel;
  size = wtrans->A[numrec][0]->size;

  if (filternorm == 1)
    cnorm = 0.5;
  else
    cnorm = sqrt((double) 0.5);

  Tab = mw_new_fsignal();
  if (mw_alloc_fsignal(Tab, size) == NULL)
    mwerror(FATAL, 1, "Allocation for sum's separation buffer refused!\n");

  while ((size >= 2) && (j < haar)) {

    size2 = size / 2;
    for (c = 0; c < size2; c++) {
      Tab->values[c] = wtrans->A[numrec][0]->values[2 * c] + wtrans->A[numrec][0]->values[2 * c + 1];
      Tab->values[size2 + c] = wtrans->A[numrec][0]->values[2 * c] - wtrans->A[numrec][0]->values[2 * c + 1];
    }

    for (c = 0; c < size; c++)
      wtrans->A[numrec][0]->values[c] = cnorm * Tab->values[c];

    size = size2;
    j++;
  }

  mw_delete_fsignal(Tab);
}



void
WAVEL(wtrans, J, int_ri, edge_ri, edge, inverse, haar_ri)

	/*----- Computes the wavelet decomposition of S -----*/

Wtrans1d    wtrans;		/* Wavelet transform */
int         J;			/* Level of decomposition */
Fsignal     int_ri;		/* Impulse response of the low-pass filter 
				 * (computation of the inner wavelet coefficients) */
Fimage      edge_ri;		/* Impulse responses of filters for special 
				 * edge processing */
int         edge;		/* Type of edge processing (see `Edge`
				 * in owave1) */
int	       *inverse;		/* Performs invertible transform for edge
				 * processing mode 0 and 2 */
Fsignal     haar_ri;        	/* Impulse response for Haar filter */

{
  int	    prolong;		/* Indicates signal extension 
				 * for invertibility */
  int       haarx;              /* Flag for use of Haar filter */

    if(!inverse || (edge == 1) || (edge == 3))
	prolong = 0;
    else
	prolong = 1;

  haarx = 0;
  if (haar_ri && (wtrans->A[J-1][0]->size >> 1 < int_ri->size))
    haarx = 1;

	/*----- Computation of average at level J -----*/

  if (haarx == 0)
    sconvolve(wtrans->A[J-1][0], wtrans->A[J][0], &DECIM, &INTERPOL, &REFLIR, NULL, &edge, &prolong, int_ri, edge_ri);
  else
    sconvolve(wtrans->A[J-1][0], wtrans->A[J][0], &DECIM, &INTERPOL, &REFLIR, NULL, &EDGE_HAAR, &prolong, haar_ri, NULL);

	/*----- Computation of detail at level J -----*/

  if (haarx == 0)
    sconvolve(wtrans->A[J-1][0], wtrans->D[J][0], &DECIM, &INTERPOL, &REFLIR, &HIGH, &edge, &prolong, int_ri, edge_ri);
  else
    sconvolve(wtrans->A[J-1][0], wtrans->D[J][0], &DECIM, &INTERPOL, &REFLIR, &HIGH, &EDGE_HAAR, &prolong, haar_ri, NULL);

}




void
owave1(NumRec, Haar, Edge, Precond, Inverse, FilterNorm, Signal, Output, Ri, Edge_Ri)

	/*--- Computes the orthogonal wavelet transform of signal `Signal` ---*/

int       *NumRec;		/* Number of recursion (-j) */
int       *Haar;                /* Continue decomposition with Haar wavelet
				 * until ultimate level */
int       *Edge;		/* Equal 0 if extension with 0 */
				/* 1 if periodization */
				/* 2 if reflexion */
				/* 3 (default) if special treatment of edges */
int       *Precond;		/* Equal 0 (default) if no
				 * (un)preconditionning 
				 * 1 if preconditionning only
				 * 2 if preconditionning and unpreconditionning
				 */
int	  *Inverse;		/* Performs invertible transform for edge
				 * processing mode 0 and 2 */
int       *FilterNorm;	        /* Type of normalisation :
			         * equal 0 if no normalisation 
			         *       1 if normalisation of the sum of
			         *         `ri`'s coefficients to 1.0 
			         *       2 if normalisation of the squares' sum 
			         *         `ri`'s coefficients to 1.0 */
Fsignal    Signal;		/* Input signal */
Wtrans1d   Output;		/* Wavelet transform of the signal `Signal` */
Fsignal    Ri;			/* Impulse response of the low pass filter */
Fimage     Edge_Ri;		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */

{
  int         J;		/* Current level of decomposition */
  Fsignal     Haar_Ri;        /* Impulse response for Haar filter */


  /*--- Detection of errors in command line ---*/

  COMLINE_ERR(Ri, Edge_Ri, *Edge, *Precond, NumRec, Haar, Inverse, Signal->size);

  /*--- Memory allocation for Haar filter ---*/

  if (Haar) {
    Haar_Ri = mw_new_fsignal();
    if (mw_alloc_fsignal(Haar_Ri, 2) == NULL)
      mwerror(FATAL, 1, "Not enough memory for Haar impulse response buffer.\n");
  } else
    Haar_Ri = NULL;

  /*--- Normalisation of filter banks ---*/

  NORM_FIL(Ri, Edge_Ri, *FilterNorm, *Edge, Haar_Ri);

  /*--- Memory allocation for wavelet transform ---*/

  if(mw_alloc_ortho_wtrans1d(Output, *NumRec, Signal->size)==NULL)
    mwerror(FATAL, 1, "Not enough memory for wavelet transform!\n");

  /*--- Write commentary for Result ---*/

  COMMENT(Output, Signal, *Edge, *Precond, *FilterNorm, Ri);

  /*--- Preconditionning of signal's edges (if selected) ---*/

  if (*Precond >= 1)
    precond1d(NULL, Signal, Signal, Edge_Ri);
	
  Output->A[0][0] = Signal;

  /*--- Wavelet decomposition ---*/

  for (J = 1; J <= *NumRec; J++)
    WAVEL(Output, J, Ri, Edge_Ri, *Edge, Inverse, Haar_Ri);

  if (Haar) {
    if (*Haar > *NumRec)
      HAAR_WAVEL1(Output, *Haar, *FilterNorm);
    mw_delete_fsignal(Haar_Ri);
  }

  /*--- Unpreconditionning of average's edges (if selected) ---*/

  if (*Precond == 2)
    precond1d(&INVERSE, Output->A[*NumRec][0], Output->A[*NumRec][0],  Edge_Ri);
}

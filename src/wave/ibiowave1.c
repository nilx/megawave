/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {ibiowave1};
version = {"1.3"};
author = {"Jean-Pierre D'Ales"};
function = {"Reconstructs a signal from a biorthogonal wavelet transform"};
usage = {
'r':[RecursNum=1]->NumRec [1,20]
	"Number of levels (default 1)", 
'e':[EdgeMode=2]->Edge [0,2]
	"Edge processing mode (0/1/2, default 2)", 
'n':[FilterNorm=0]->FilterNorm [0,2]
	"Normalization mode for filter bank (0/1/2, default 0)", 
WavTrans->Wtrans
	"Input wavelet transform (wtrans1d)", 
RecompSignal<-Output
	"Reconstructed signal (fsignal)", 
ImpulseResponse1->Ri1
	"Impulse response of filter 1 (fsignal)", 
ImpulseResponse2->Ri2
	"Impulse response of filter 2 (fsignal)" 
	};
*/


/*--- Include files UNIX C ---*/
#include <stdio.h>
#include <math.h>

/*--- Library megawave2 ---*/
#include  "mw.h"

/*--- Megawave2 modules ---*/
extern void sconvolve();

/*--- Constants ---*/

static int DECIM=1;		/* Decimation's constant */
static int INTERPOL=2;		/* Interpolation rate */
static int HIGH=1;		/* Index for high-pass filtering */
static int PROLONG=0;		/* Index for inverse preconditionning */



static void
COMMENT(result, wtrans, edge, filternorm, ri1, ri2)

	/*--- Fill comment and other fields for result ---*/

Fsignal     result;		/* Inverse wavelet transform of `wtrans` */
Wtrans1d    wtrans;		/* Input wavelet transform */
int 	    edge;		/* Type of edge processing 
				 * (see `Edge` in `ibiowave1`) */
int	   *filternorm;	        /* Type of normalisation :
				 * equal 0 if normalisation of the sum of 
				 *         `ri`'s coefficients to 1.0
			 	 *       1 if normalisation of the squares' sum 
			 	 *         `ri`'s coefficients to 1.0 */
Fsignal     ri1;		/* Impulse response of the low-pass filter 
			 	 * for decomposition */
Fsignal     ri2;		/* Impulse responses of the low-pass filter 
			 	 * for synthesis */

{
  strcpy(result->cmt, "Inv. wav. trans. of ");
  strcat(result->cmt, wtrans->name);
  switch(edge)
    {
  case 1:
      strcat(result->cmt, ", edges period.");
      break;
  case 2:
      strcat(result->cmt, ", edges reflected");
      break;
    }
  if (*filternorm >= 1)
    strcat(result->cmt, ", filters coef. norm.");

}



static void
COMLINE_ERR(wtrans, ri1, ri2, edge, numrec, size)

     /*--- Detects errors and contradiction in command line ---*/

Wtrans1d    wtrans;             /* Input wavelet transform */
Fsignal     ri1;		/* Impulse response of the low-pass filter 
			 	 * for decomposition */
Fsignal     ri2;		/* Impulse responses of the low-pass filter 
			 	 * for synthesis */
int 	    edge;		/* Type of edge processing 
				 * (see `Edge` in ibiowave1) */
int        *numrec;   	        /* Number of levels for decomposition */
long	    size;		/* Size of the signal */

{

  /*--- Size of image ---*/

  if((size>>(*numrec - 1) < ri1->size) || (size>>(*numrec - 1) < ri2->size))
    mwerror(FATAL, 1, "The size of signal must be greater than N where N is the size of the i.r.!\n");

  if ((edge == 1) && ((size % (1 << *numrec)) != 0))
    mwerror(FATAL, 1, "The size of the signal is not a multiple of 2^J!\n");

  /*--- Number of level ---*/

  if (wtrans->nlevel < *numrec) {
    mwerror(WARNING, 0, "Only %d levels in wavelet transform!\n", wtrans->nlevel);
    *numrec = wtrans->nlevel;
  }

}




static void
NORM_FIL(ri1, ri2, filternorm)

	/*--- Normalisation of the coefficients of the filter impulse 
	 *--- responses ---*/

Fsignal	ri1, ri2;	        /* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
int    	filternorm;	        /* Type of normalisation :
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
INV_WAVEL(J, wtrans, S, ri1, ri2, edge)	

	/*----- Computes average at level J-1 
	  ----- from average and details at level J -----*/

int         J;		        /* Current level of recomposition */
Wtrans1d    wtrans;		/* Wavelet transform */
Fsignal	    S;		        /* Reconstructed resume */
Fsignal	    ri1, ri2;	        /* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
int        *edge;		/* Type of edge processing (see `Edge`
				 * in ibiowave1) */

{
  long        c;		/* Index of the current point in `S` */
  Fsignal     Tabout;	        /* Output for module `sconvolve`, 
				 * wavelet transform of wtrans->A/D[J][0] */

  Tabout = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabout, S->size) == NULL)
    mwerror(FATAL, 1, "Allocation for line's buffer refused!\n");

  /*--- Reconstruction from the average ---*/

  sconvolve(wtrans->A[J][0], Tabout, &DECIM, &INTERPOL, NULL, NULL, edge, &PROLONG, ri1, NULL);

  for (c = 0; c < S->size; c++)
    S->values[c] = Tabout->values[c];

  /*--- Reconstruction from the detail ---*/

  sconvolve(wtrans->D[J][0], Tabout, &DECIM, &INTERPOL, NULL, &HIGH, edge, &PROLONG, ri2, NULL);


  /*--- Addition ---*/

  for (c = 0; c < S->size; c++)
    S->values[c] += Tabout->values[c];

  mw_delete_fsignal(Tabout);

}




void
ibiowave1(NumRec, Edge, FilterNorm, Wtrans, Output, Ri1, Ri2)
	
     /*--- Reconstructs an image from the wavelet teansform Wtrans ---*/

int        *NumRec;	        /* Number of recursion (-j) */
int        *Edge;		/* Equal 0 (default) if extension with 0 
				 * 1 if periodization */
				/* 2 if reflexion */
int        *FilterNorm;	        /* Equal 1 if normalisation of filter's
				 * impulse responses (sum = 1.0) */
Wtrans1d    Wtrans;		/* Wavelet transform (input) */
Fsignal     Output;		/* reconstructed signal (output) */
Fsignal     Ri1;		/* Impulse response of the low pass filter 
				 * for decomposition */
Fsignal     Ri2;		/* Impulse response of the low pass filter 
				 * for synthesis */

{
  int         J;		/* Current level of decomposition */


  /*--- Detection of errors in command line ---*/

  COMLINE_ERR(Wtrans, Ri1, Ri2, *Edge, NumRec, Wtrans->size);

  /*--- Memory allocation for inverse wavelet transform Output ---*/

  Output = mw_change_fsignal(Output, Wtrans->size);	
  if(Output==NULL)
    mwerror(FATAL, 1, "Allocation for reconstructed signal refused!\n");

  /*--- Normalisation of filter banks ---*/

  NORM_FIL(Ri1, Ri2, FilterNorm);


  /*--- Wavelet recomposition ---*/

  for (J = *NumRec; J > 1; J--)
    INV_WAVEL(J, Wtrans, Wtrans->A[J-1][0], Ri1, Ri2, Edge);

  INV_WAVEL(J, Wtrans, Output, Ri1, Ri2, Edge);

  /*--- Write commentary for Output ---*/

  COMMENT(Output, Wtrans, *Edge, FilterNorm, Ri1, Ri2);
}

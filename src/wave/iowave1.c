/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {iowave1};
version = {"1.2"};
author = {"Jean-Pierre D'Ales"};
function = {"Reconstructs a signal from an orthogonal wavelet transform"};
usage = {
'r':[RecursNum=1]->NumRec [1,20]
	"Number of levels (default 1)", 
'h':HaarLevel->Haar
	"Start reconstruction with Haar from HaarLevel",
'e':[EdgeMode=3]->Edge [0,3]
	"Edge processing mode (0/1/2/3, default 3)", 
'p':[PrecondMode=0]->Precond [0,2]
	"Edge preconditionning mode (0/1/2, default 0)", 
'i'->Inverse
	"Invertible transform", 
'n':[FilterNorm=2]->FilterNorm [0,2]
	"Filter taps normalization (0/1/2, default 2)", 
WavTrans->Wtrans
	"Input wavelet transform (wtrans1d)", 
RecompSignal<-Output
	"Reconstructed signal (fsignal)", 
ImpulseResponse->Ri
	"Impulse response of inner filters (fsignal)", 
	{
	EdgeIR->Edge_Ri
		"Impulse reponses of edge and preconditionning filters (fimage)"
	}
	};
*/


/*--- Include files UNIX C ---*/
#include <stdio.h>
#include <math.h>

/*--- Library megawave2 ---*/
#include  "mw.h"

/*--- Megawave2 modules ---*/
extern void sconvolve();
extern void precond1d();

/*--- Constants ---*/

static int DECIM=1;		/* Decimation's constant */
static int INTERPOL=2;		/* Interpolation rate */
static int HIGH=1;		/* Index for high-pass filtering */
static int INVERSE=1;		/* Index for inverse preconditionning */
static int EDGE_HAAR=0;   /* Index for edge processing mode with Haar filter */

static long	firstp;
static long	lastp[mw_max_nlevel];
static long	sizeav[mw_max_nlevel];



static void
COMLINE_ERR(wtrans, ri, edge_ri, edge, inverse, precond, numrec, haar)

	/*--- Detects errors and contradiction in command line ---*/

Wtrans1d     wtrans;		/* Wavelet transform */
Fsignal	     ri;		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
Fimage	     edge_ri;  	        /* Impulse responses of the edge filters
			 * (computation of wavelet coefficients near edges) */
int 	     edge;		/* Type of edge processing 
				 * (see `Edge` in iowave1) */
int	    *inverse;	        /* Performs invertible transform for edge
				 * processing mode 0 and 2 */
int	     precond;	        /* Type of edge preconditionning
				 * (see `Precond` in `iowave1`) */
int         *numrec;		/* Number of levels for decomposition */
int         *haar;               /* Reconstruct with Haar wavelet
				 * from haar level */

{
  long	size;		/* Size of the signal */

  size = wtrans->size;

  /*--- Number of levels ---*/

  if (wtrans->nlevel < *numrec) {
    mwerror(WARNING, 0, "Only %d levels in wavelet transform!\n", wtrans->nlevel);
    *numrec = wtrans->nlevel;
  }

  if (haar)
    if (size>>(*haar) < 1) {
      (*haar)--;
    while (size>>(*haar) < 1)
      (*haar)--;
    mwerror(WARNING, 1, "Size of average too small!\nHaar is set to %d\n", *haar);
  }

	/*--- Edge processing selection ---*/

  if((edge == 3) && (edge_ri == NULL))
    mwerror(USAGE, 1, "Edge filters i.r. are needed!\n");

  if(((edge == 1) || (edge == 3)) && inverse)
    mwerror(WARNING, 0, "Edge processing mode is already invertible!\n");

	/*--- Preconditionning and special edge processing ---*/

  if((edge != 3) && (precond >= 1))
    mwerror(USAGE, 1, "Preconditionning only with special edge processing\n");

	/*--- Size of average ---*/

  if(!haar) {
    if (wtrans->A[*numrec][0]->size < ri->size)
      mwerror(FATAL, 1, "The size of average must be greater than the size of the i.r.!\n");
  } 

	/*--- Inner and edge filters compatibility ---*/

    if((edge == 3) && ((edge_ri->nrow != 4 * ri->size) || (edge_ri->ncol != 3 * ri->size / 2 - 1)))
	mwerror(FATAL, 1, "Inner and edge filters are not compatible!\n");

}




static void
COMMENT(result, wtrans, edge, precond, filternorm, ri)

	/*--- Fill comment and other fields for result ---*/

Fsignal     result;		/* Inverse wavelet transform of `wtrans` */
Wtrans1d    wtrans;		/* Input wavelet transform */
int 	    edge;		/* Type of edge processing 
				 * (see `Edge` in `iowave1`) */
int	    precond;	        /* Type of edge preconditionning
				 * (see `Precond` in `iowave1`) */
int	   *filternorm;	        /* Type of normalisation :
				 * equal 0 if normalisation of the sum of 
				 *         `ri`'s coefficients to 1.0
			 	 *       1 if normalisation of the squares' sum 
			 	 *         `ri`'s coefficients to 1.0 */
Fsignal	ri;		        /* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */

{
  strcpy(result->cmt, "Inv. wav. trans. of ");
  strcat(result->cmt, wtrans->name);
  switch(edge)
    {
  case 0:
      strcat(result->cmt, ", edges zero padded");
      break;
  case 1:
      strcat(result->cmt, ", edges period.");
      break;
  case 2:
      strcat(result->cmt, ", edges reflected");
      break; 
  case 3:
      strcat(result->cmt, ", special edge proc.");
      break;
    }
  if(precond == 1)
    strcat(result->cmt, " with precond.");
  if(precond == 2)
    strcat(result->cmt, "/unprecond.");
  if(filternorm)
    strcat(result->cmt, ", filters coef. norm.");

}




static void
INIT_SHRINK(wtrans, ri, numrec, edge)

	/*--- Computation of indices for shrinkage ---*/

Wtrans1d	wtrans;		/* Wavelet transform */
Fsignal	        ri;		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
short	        numrec;		/* Number of levels for decomposition */
int             edge;		/* Type of edge processing
				 * (see `Edge` in iowave2) */

{
  short i,j;				/* Indices for orientation and level */
  short size;
  short firstphigh;

  if ((edge == 2) || (edge == 0)) {
    if ((1 - ri->size - (long) ri->shift) < 0) 
      firstp = - (1 - ri->size - (long) ri->shift) / 2;
    else
      firstp = (ri->size + (long) ri->shift) / 2;

    if (((long) ri->shift - 1) < 0) 
      firstphigh = - ((long) ri->shift - 1) / 2;
    else
      firstphigh = - (long) ri->shift / 2;

    sizeav[0] = wtrans->size;
    for (j=1; j<=numrec; j++) {
      lastp[j] = firstphigh + sizeav[j-1] / 2 - 1;
      sizeav[j] = firstphigh + (sizeav[j-1] - 3 + ri->size + (long) ri->shift) / 2 + 1;
      if (wtrans->D[j][0]->size != sizeav[j])
	mwerror(FATAL, 1, "Bad size for detail!\n");
      if (wtrans->D[j][0]->firstp != firstphigh)
	{
	  wtrans->D[j][0]->firstp = firstphigh;
	  mwerror(WARNING, 0, "Bad value for firstp in detail!\n");
	}

      lastp[j] = firstp + sizeav[j-1] / 2 - 1;
      sizeav[j] = firstp + (sizeav[j-1] - 1 - (long) ri->shift) / 2 + 1;

    }
    if (wtrans->A[numrec][0]->size != sizeav[numrec])
      mwerror(FATAL, 1, "Bad size for average!\n");

  }

}



static void
NORM_FIL(ri, edge_ri, filternorm, edge, haar_ri)

	/*--- Normalisation of the coefficients of the filter impulse 
	 *--- responses ---*/

Fsignal	ri;		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
Fimage	edge_ri;	/* Impulse responses of the edge filters
			 * (computation of wavelet coefficients near edges) */
int	filternorm;	/* Type of normalisation :
			 * equal 0 if normalisation of the sum of 
			 *         `ri`'s coefficients to 1.0
			 *       1 if normalisation of the squares' sum 
			 *         `ri`'s coefficients to 1.0 */
int 	edge;		/* Type of edge processing 
				 * (see `Edge` in iowave1) */
Fsignal haar_ri;        /* Impulse response for Haar filter */


{
  double	s,s2;	  /* Sum of `ri`'s coefficients and sum's square root 
			   * of `ri`'s coefficients' squares */
  short	i,j;


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
      ri->values[i] *= s;
    if (edge == 3)
      {
	for (i = 0; i < edge_ri->nrow / 2; i++)
	  for (j = 0; j < edge_ri->ncol; j++)
	    edge_ri->gray[edge_ri->ncol * i + j] *= s;
      }
  }

  if (haar_ri) {
    if (filternorm == 1) {
      haar_ri->values[0] = 1.0;
      haar_ri->values[1] = 1.0;
    } else
      {
	haar_ri->values[0] = sqrt((double) 0.5);
	haar_ri->values[1] = sqrt((double) 0.5);
      }
    haar_ri->shift = 0.0;
  }
}



static void
HAAR_INV_WAVEL1(wtrans, haar, filternorm)

Wtrans1d    wtrans;	    /* Wavelet transform */
int         haar;           /* Continue decomposition with Haar wavelet
			     * until ultimate level */
int	    filternorm;	    /* Type of normalisation : see adapowave2 */

{
  int          j;
  int          numrec;
  int          size, sizeav;
  int          c;
  Fsignal      Tab;
  double       cnorm;

  numrec = wtrans->nlevel;
  size = sizeav = wtrans->A[numrec][0]->size;
  for (j = numrec + 1; j <= haar; j++)
    size /= 2;

  if (filternorm == 1)
    cnorm = 1.0;
  else
    cnorm = sqrt((double) 0.5);

  Tab = mw_new_fsignal();
  if (mw_alloc_fsignal(Tab, sizeav) == NULL)
    mwerror(FATAL, 1, "Allocation for sum's separation buffer refused!\n");


  for (j = haar; j > numrec; j--) {
    for (c = 0; c < size; c++) {
	  Tab->values[2 * c] = wtrans->A[numrec][0]->values[c] + wtrans->A[numrec][0]->values[size + c];
	  Tab->values[2 * c + 1] = wtrans->A[numrec][0]->values[c] - wtrans->A[numrec][0]->values[size + c];
    }

    size *= 2;
    for (c = 0; c < size; c++)
	  wtrans->A[numrec][0]->values[c] = cnorm * Tab->values[c];
  }

}




static void
INV_WAVEL(J, wtrans, S, int_ri, inverse, edge, edge_ri, haar_ri)	

	/*----- Computes average at level J-1 
	  ----- from average and details at level J -----*/

    int         J;		/* Current level of recomposition */
    Wtrans1d	wtrans;		/* Wavelet transform */
    Fsignal	S;		/* Reconstructed resume */
    Fsignal     int_ri;		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
    int	       *inverse;	/* Performs invertible transform for edge
				 * processing mode 0 and 2 */
    int         edge;		/* Type of edge processing (see `Edge`
				 * in iowave1) */
    Fimage      edge_ri;	/* Impulse responses of filters for special 
				 * edge processing */
    Fsignal     haar_ri;        /* Impulse response for Haar filter */

{
    long        c;		/* Index of the current point in `S` */
    Fsignal     Tabout;		/* Output for module `sconvolve`, 
				 * wavelet transform of wtrans->A/D[J][0] */
    int 	prolong;	/* Indicates signal shrinkage 
				 * for invertibility */
    int         haars;          /* Flag for use of Haar filter */

  haars = 0;
  if (haar_ri && (wtrans->A[J][0]->size < int_ri->size))
    haars = 1;

  if(!inverse || (edge == 1) || (edge == 3) || (haars == 1)) {
    if (wtrans->A[J][0]->size != wtrans->D[J][0]->size)
      mwerror(FATAL,1, "Average and Detail are not of the same size!\n");
    prolong = 0;
  } else 
    {
      prolong = 2;
      wtrans->A[J][0]->firstp = firstp;
      wtrans->A[J][0]->lastp = lastp[J];
    }

    Tabout = mw_new_fsignal();
    if (mw_alloc_fsignal(Tabout,1) == NULL)
	mwerror(FATAL, 1, "Allocation for line's buffer refused!\n");


    /*--- Reconstruction from the average ---*/

  if (haars == 0)
    sconvolve(wtrans->A[J][0], Tabout, &DECIM, &INTERPOL, NULL, NULL, &edge, &prolong, int_ri, edge_ri);
  else
    sconvolve(wtrans->A[J][0], Tabout, &DECIM, &INTERPOL, NULL, NULL, &EDGE_HAAR, &prolong, haar_ri, NULL);

    for (c = 0; c < S->size; c++)
	S->values[c] = Tabout->values[c];

    /*--- Reconstruction from the detail ---*/

  if (haars == 0)
    sconvolve(wtrans->D[J][0], Tabout, &DECIM, &INTERPOL, NULL, &HIGH, &edge, &prolong, int_ri, edge_ri);
  else
    sconvolve(wtrans->D[J][0], Tabout, &DECIM, &INTERPOL, NULL, &HIGH, &EDGE_HAAR, &prolong, haar_ri, NULL);


    /*--- Addition ---*/

    for (c = 0; c < S->size; c++)
	S->values[c] += Tabout->values[c];

    mw_delete_fsignal(Tabout);

}




void
iowave1(NumRec, Haar, Edge, Precond, Inverse, FilterNorm, Wtrans, Output, Ri, Edge_Ri)
	
	/*--- Reconstructs an image from the wavelet teansform Wtrans ---*/

int        *NumRec;		/* Number of recursion (-j) */
int        *Haar;               /* Reconstruct with Haar wavelet
				 * from Haar level */
int        *Edge;		/* Equal 0 if extension with 0 
				 * 1 if periodization 
				 * 2 if reflexion 
				 * 3 (default) if special treatment of edges */
int        *Precond;         	/* Equal 0 (default) if no
				 *	   (un)preconditionning
				 * 	 1 if preconditionning only 
				 * 	 2 if preconditionning and
				 * 	   unpreconditionning */
int        *Inverse;	        /* Performs invertible transform for edge
				 * processing mode 0 and 2 */
int        *FilterNorm;		/* Equal 1 if normalisation of filter's
				 * impulse responses (sum = 1.0) */
Wtrans1d    Wtrans;		/* Input wavelet transform */
Fsignal     Output;		/* Reconstructed signal */
Fsignal     Ri;		        /* Impulse response of the low pass filter */
Fimage	Edge_Ri;	        /* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
{
  int         J;		/* Current level of decomposition */
  Fsignal     Haar_Ri;          /* Impulse response for Haar filter */


    /*--- Detection of errors in command line ---*/

    COMLINE_ERR(Wtrans, Ri, Edge_Ri, *Edge, Inverse, *Precond, NumRec, Haar);

    /*--- Memory allocation for inverse wavelet transform Output ---*/

    Output = mw_change_fsignal(Output, Wtrans->size);
    if(Output==NULL)
	mwerror(FATAL, 1, "Allocation for reconstructed signal refused!\n");

    /*--- Memory allocation for Haar filter ---*/

  if (Haar) {
    Haar_Ri = mw_new_fsignal();
    if (mw_alloc_fsignal(Haar_Ri, 2) == NULL)
      mwerror(FATAL, 1, "Not enough memory for Haar impulse response buffer.\n");
  } else
    Haar_Ri = NULL;

    /*--- Normalisation of filter banks ---*/

  NORM_FIL(Ri, Edge_Ri, *FilterNorm, *Edge, Haar_Ri);

    /*--- Computation of position indices for shrinkage ---*/

  if (Inverse)
    INIT_SHRINK(Wtrans, Ri, *NumRec, *Edge);

    /*--- Preconditionning of resume's edges (if selected) ---*/

  if (*Precond >= 2)
    precond1d(NULL, Wtrans->A[*NumRec][0], Wtrans->A[*NumRec][0], Edge_Ri);


    /*--- Wavelet recomposition ---*/

  if (Haar)
    if (*Haar > *NumRec)
      HAAR_INV_WAVEL1(Wtrans, *Haar, *FilterNorm);

    for (J = *NumRec; J > 1; J--)
	INV_WAVEL(J, Wtrans, Wtrans->A[J-1][0], Ri, Inverse, *Edge, Edge_Ri, Haar_Ri);

    INV_WAVEL(J, Wtrans, Output, Ri, Inverse, *Edge, Edge_Ri, Haar_Ri);

  if (Haar)
    mw_delete_fsignal(Haar_Ri);

    /*--- Unpreconditionning of image's edges (if selected) ---*/

    if (*Precond >= 1)
      precond1d(&INVERSE, Output, Output, Edge_Ri);

    /*--- Write commentary for Output ---*/

    COMMENT(Output, Wtrans, *Edge, *Precond, FilterNorm, Ri);

}

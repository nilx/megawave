/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {dyowave2};
version = {"1.1"};
author = {"Jean-Pierre D'Ales"};
function = {"Computes the orthogonal wavelet coefficients of an image without decimation"};
usage = {
'r':[NLevel=1]->NumRec [1,20]
	"Number of levels (default 1)", 
'd':[StopDecimLevel=2]->StopDecim [1,20]
	"Level for decimation stop (default 2)",
'o'->Ortho
	"Computes orthogonal coefficients", 
'e':[EdgeMode=3]->Edge [0,3]
	"Edge processing mode (0/1/2/3, default 3)", 
'p':[PrecondMode=0]->Precond [0,2]
	"Edge preconditionning mode (0/1/2, default 2)", 
'n':[FiltNorm=2]->FilterNorm [0,2]
	"Filter taps normalization. 0: no normalization, 1: sum equal to 1.0, 2: squares sum equal to 1.0 (default)", 
Image->Image
	"Input image (fimage)", 
WavTrans<-Output
	"Wavelet transform of Image (wtrans2d)", 
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

/*--- Megawave2 library ---*/
#include  "mw.h"

/*--- Megawave2 modules definition ---*/

extern void owave2();



static void
ALLOC_WAV(wtrans, image, numrec, stopdecim, edge)

Wtrans2d    wtrans;
Fimage      image;
int         numrec;
int         stopdecim;
int         edge;

{
  int             i, J;
  long            nrow, ncol;
  int             nrowshift, ncolshift;

  nrow = 2;
  ncol = 2;
  for (J = 1; J <= numrec; J++) {
    nrow *= 2;
    ncol *= 2;
  }
  if(mw_alloc_ortho_wtrans2d(wtrans, numrec, nrow, ncol)==NULL)
    mwerror(FATAL, 1, "Memory allocation refused for `WavTrans`!\n");

  nrow = image->nrow;
  ncol = image->ncol;
  for (J = 1; J < stopdecim; J++) {
    nrow /= 2;
    ncol /= 2;

    for (i = 0; i <= 3; i++)
      if (mw_change_fimage(wtrans->images[J][i], nrow, ncol) == NULL)
	mwerror(FATAL, 1, "Not enough memory for wavelet transform buffer.\n");
  }

  nrowshift = 1;
  ncolshift = 1;
  for (J = numrec; J >= stopdecim; J--) {
    nrowshift *= 2;
    ncolshift *= 2;
  }
  nrow -= nrowshift;
  ncol -= ncolshift;
  for (J = stopdecim; J <= numrec ; J++) {
    for (i = 0; i <= 3; i++)
      if (mw_change_fimage(wtrans->images[J][i], nrow, ncol) == NULL)
	mwerror(FATAL, 1, "Not enough memory for wavelet transform buffer.\n");
  }

  wtrans->images[0][0] = image;
  wtrans->nrow = image->nrow;
  wtrans->ncol = image->ncol;

}





static void
WAVEL(wtrans, numrec, stopdecim, ortho, filternorm, ri, edge_ri, edge, precond)

	/*----- Computes the wavelet decomposition of S -----*/

Wtrans2d    wtrans;		/* Wavelet transform */
int        *numrec;	        /* Level of decomposition */
int         stopdecim;          /* Level where decimation is cancelled */
int        *ortho;              /* Computes orthogonal coefficients */
int        *filternorm;
Fsignal     ri;		        /* Impulse response of the low-pass filter 
				 * (computation of the inner wavelet 
                                 * coefficients) */
Fimage      edge_ri;	        /* Impulse responses of filters for special 
				 * edge processing */
int        *edge;		/* Type of edge processing (see `Edge`
				 * in biowave2) */
int        *precond;

{
  int         i,J;
  int         decim;            /* Flag of decimation for sconvolve */
  int         nsubim;           /* Number of subimages */
  long        c, r;		/* Indices for line and column in resume 
					 * or detail */
  int         sc, sr;
  long	      rdx, rdxs;	/* Index of first element of line `l` */ 
  long	      dx, dy;		/* Size of image */
  long        dxs, dys;         /* Size of subimages */
  int         s;                /* index of subimage */
  Fimage      Tab;		/* Buffer for sum's separation */
  Wtrans2d    subwtrans;

  subwtrans = mw_new_wtrans2d();

  if (stopdecim > 1) {
    J = stopdecim - 1;
    owave2(&J, NULL, edge, precond, NULL, filternorm, wtrans->images[0][0], subwtrans, ri, edge_ri);
    for (J=1; J<stopdecim; J++)
      for (i=0; i<=3; i++) {
	rdx = 0;
	for (r=0;r<subwtrans->images[J][i]->nrow; r++) {
	  for (c=0;c<subwtrans->images[J][i]->ncol; c++)
	    wtrans->images[J][i]->gray[rdx+c] = subwtrans->images[J][i]->gray[rdx+c];
	  rdx += subwtrans->images[J][i]->ncol;
	}
      }
  }
  
  nsubim = 1;
  for (s = 1; s <= *numrec; s++)
      nsubim *= 2;

  dx = wtrans->images[0][0]->ncol;
  dy = wtrans->images[0][0]->nrow;
  dxs = dx - nsubim;
  dys = dy - nsubim;

  /*--- Initialization of Tab ---*/

  Tab = mw_new_fimage();
  if (mw_alloc_fimage(Tab, dys, dxs) == NULL)
    mwerror(FATAL, 1, "Allocation for sum's separation buffer refused!\n");

  nsubim = 1;
  for (s = stopdecim; s <= *numrec; s++)
      nsubim *= 2;

  for (sr = 0; sr < nsubim; sr++) 
    for (sc = 0; sc < nsubim; sc++) {
    
      mw_delete_wtrans2d(subwtrans);
      subwtrans = mw_new_wtrans2d();

      rdx = sr * dx;
      rdxs = 0;
      for (r = 0; r < dys; r++) {
	for(c = 0; c < dxs; c++)
	  Tab->gray[rdxs+c] = wtrans->images[0][0]->gray[rdx + sc + c];
	rdx += dx;
	rdxs += dxs;
      }

      owave2(numrec, NULL, edge, precond, NULL, filternorm, Tab, subwtrans, ri, edge_ri);

      s = 1;
      for (J = stopdecim; J <= *numrec; J++) {
	s *= 2;
	if ((sr < s) && (sc < s)) {
	  for (i=0; i<=3; i++) {
	    rdx = sr * wtrans->images[J][i]->ncol;
	    rdxs = 0;
	    for (r=0;r<subwtrans->images[J][i]->nrow; r++) {
	      for (c=0;c<subwtrans->images[J][i]->ncol; c++)
		wtrans->images[J][i]->gray[rdx+c*s+sc] = subwtrans->images[J][i]->gray[rdxs+c];
	      rdx += s * wtrans->images[J][i]->ncol;
	      rdxs += subwtrans->images[J][i]->ncol;
	    }
	  }
	}
      }
    }

  mw_delete_fimage(Tab);
  mw_delete_wtrans2d(subwtrans);

}




void
dyowave2(NumRec, StopDecim, Ortho, Edge, Precond, FilterNorm, Image, Output, Ri, Edge_Ri)

  /*--- Computes the orthogonal wavelet Coefficients of image `Image` ---*/

int        *NumRec;	    /* Number of iteration (-j) */
int        *StopDecim;      /* Level where decimation is cancelled */
int        *Ortho;          /* Computes orthogonal coefficients */
int        *Edge;	    /* Equal 0 (default) if extension with 0 */
			    /* 1 if periodization */
			    /* 2 if reflexion */
int        *Precond;	    /* Equal 0 (default) if no
			     * (un)preconditionning 
			     * 1 if preconditionning only
			     * 2 if preconditionning and unpreconditionning */
int        *FilterNorm;	    /* Equal 1 if normalisation of filter's
			     * impulse responses (sum = 1.0) */
Fimage      Image;	    /* Input image */
Wtrans2d    Output;	    /* Wavelet transform of the image `Image` */
Fsignal     Ri;		    /* Impulse response of the low pass filter */
Fimage	    Edge_Ri;	    /* Impulse responses of filters for special 
			     * edge processing (including preconditionning 
			     * matrices */

{

  /*--- Detection of errors in command line ---*/


  /*--- Memory allocation for wavelet transform Output ---*/

  ALLOC_WAV(Output, Image, *NumRec, *StopDecim, *Edge);

  /*--- Write commentary for Output ---*/


  /*--- Wavelet decomposition ---*/

    WAVEL(Output, NumRec, *StopDecim, Ortho, FilterNorm, Ri, Edge_Ri, Edge, Precond);
}

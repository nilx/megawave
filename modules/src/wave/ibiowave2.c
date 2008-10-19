/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {ibiowave2};
version = {"1.5"};
author = {"Jean-Pierre D'Ales"};
function = {"Reconstructs an image from a biorthogonal wavelet transform"};
usage = {
 'r':[NLevel=0]->NumRec [0,20]  "Start reconstruction from level NLevel", 
 'h':HaarNLevel->Haar           "Start reconstruction with Haar filter from level HaarNLevel down to level NLevel + 1",
 'e':[EdgeMode=2]->Edge [0,2]           "Edge processing mode", 
 'n':[FilterNorm=0]->FilterNorm [0,2]   "Normalization mode for filter bank",
 WavTrans->Wtrans         "Input wavelet transform (wtrans2d)", 
 RecompImage<-Output      "Output reconstructed image (fimage)", 
 ImpulseResponse1->Ri1    "Impulse response of filter 1 (fsignal)", 
 ImpulseResponse2->Ri2    "Impulse response of filter 2 (fsignal)" 
	};
*/
/*----------------------------------------------------------------------
 v1.5 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* sconvolve() */

/*--- Constants ---*/

#define MAX_NLEVEL 20

/*--- Global variables ---*/

static int DECIM=1;		/* Decimation's constant */
static int INTERPOL=2;		/* Interpolation rate */
static int HIGH=TRUE;		/* Index for high-pass filtering */
static int PROLONG=0;		/* Index for inverse preconditionning */
static int EDGE_HAAR=0;         /* Index for edge processing mode 
				 * with Haar filter */

static Fsignal      haar_ri;    /* Impulse response for Haar filter */
static double	    s1,s2;	/* Sums of coefficients of `ri1` and `ri2` */
static double	    shaar;      /* Sums of coefficients of haar filter */



static void
COMLINE_ERR(wtrans, ri1, ri2, edge, numrec, haar, haar_test)

	/*--- Detects errors and contradiction in command line ---*/

Wtrans2d	wtrans;		/* Wavelet transform */
Fsignal	        ri1, ri2;	/* Impulse responses of the low-pass filters */
int 	        edge;		/* Type of edge processing 
				 * (see `Edge` in biwav2d) */
int            *numrec;		/* Number of levels for decomposition */
int            *haar;           /* Reconstruct with Haar wavelet
				 * from haar level */
int            *haar_test;      /* Test value for use of Haar filter */

{
  long	dx, dy;				/* Size of the image */
  long	j, i;				/* Index for level and orientation */

  dx = wtrans->ncol;
  dy = wtrans->nrow;
  *haar_test = 0;

  /*--- Number of levels ---*/

  if (*numrec == 0)
    *numrec = wtrans->nlevel;
  if (wtrans->nlevel < *numrec) {
    mwerror(WARNING, 0, "NLevel is too large! -> set to %d\n", wtrans->nlevel);
    *numrec = wtrans->nlevel;
  }

  /*--- Size of image ---*/

  if (!haar) {
    if((dx>>(*numrec) < ri1->size) || (dy>>(*numrec) < ri1->size)) 
      *haar_test = 1;
    if((dx>>(*numrec) < ri2->size) || (dy>>(*numrec) < ri2->size))
      *haar_test = 1;
  } else
    *haar_test = 1;


  /*--- Size of detail and average sub-images ---*/

  for (j=1; j <= *numrec; j++) {
    if ((wtrans->images[j][1]->ncol != (dx + 1) / 2) || (wtrans->images[j][1]->nrow != dy / 2))
	mwerror(FATAL, 1, "The size of detail are incompatible!\n");
    if ((wtrans->images[j][2]->ncol != dx / 2) || (wtrans->images[j][2]->nrow != (dy + 1) / 2))
	mwerror(FATAL, 1, "The size of detail are incompatible!\n");
    if ((wtrans->images[j][3]->ncol != dx / 2) || (wtrans->images[j][3]->nrow != dy / 2))
	mwerror(FATAL, 1, "The size of detail are incompatible!\n");
    dx = (dx + 1) / 2;
    dy = (dy + 1) / 2;
  }
  if ((wtrans->images[*numrec][0]->ncol != dx) || (wtrans->images[*numrec][0]->nrow != dy))
    mwerror(FATAL, 1, "The size of average are incompatible!\n");

}



static void
COMMENT(result, wtrans, edge, filternorm, ri1, ri2)

	/*--- Fill comment and other fields for result ---*/

Fimage          result;		/* Inverse wavelet transform of `wtrans` */
Wtrans2d        wtrans;		/* Input wavelet transform */
int 	        edge;		/* Type of edge processing 
				 * (see `Edge` in `wav2d`) */
int	       *filternorm;	/* Type of normalisation :
				 * equal 0 if normalisation of the sum of 
				 *         `ri`'s coefficients to 1.0
			 	 *       1 if normalisation of the squares' 
			 	 *         sum of `ri`'s coefficients to 1.0 */
Fsignal	        ri1, ri2;	/* Impulse response of the low-pass filter 
			  * (computation of the inner wavelet coefficients) */

{
  strcpy(result->cmt, "Inv. wav. trans. of ");
  strcat(result->cmt, wtrans->name);
  switch(edge) {
 case 1:
    strcat(result->cmt, ", edges period.");
    break;
 case 2:
    strcat(result->cmt, ", edges reflected");
    break;
  }

  if(*filternorm >= 1)
    strcat(result->cmt, ", filters coef. norm.");

}




static void
NORM_FIL(ri1, ri2, filternorm, haar_test)

	/*--- Normalisation of the coefficients of the filter impulse 
	 *--- responses ---*/

Fsignal	        ri1, ri2;	/* Impulse response of the low-pass filter 
			   * (computation of the inner wavelet coefficients) */
int	        filternorm;	/* Type of normalisation : see ibiowave2 */
int             haar_test;      /* Test value for use of Haar filter */

{
  double	s;		/* Cross correlation of `ri1` and `ri2` */
  int           test;           /* Indicates biorthogonality of filters */
  int	        i,j;
  int	        shift1, shift2;


  /*--- Computation of cross correlation ---
    --- and test for biorthogonality of filters ---*/

  shift1 = (int) ri1->shift;
  shift2 = (int) ri2->shift;
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

  s = 0.0;
  for (i = shift1; i < shift1 + ri1->size; i++)
    if ((i - shift2 < ri2->size) && (i - shift2 >= 0))
      s += ri1->values[i - shift1] * ri2->values[i - shift2];

  /*--- Normalisation of the coefficients ---*/

  if (filternorm == 0) {
    for (i = 0; i < ri1->size; i++)
      ri1->values[i] /= s;
    for (i = 0; i < ri2->size; i++)
      ri2->values[i] /= s;
  } else
    {
      if (filternorm == 1) {

	/*--- Computation of the sum of the coefficients ---*/

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


      s = sqrt(s);
      s1 = sqrt(s1);
      s2 = sqrt(s2);
      for (i = 0; i < ri1->size; i++)
	ri1->values[i] *= s2 / s1 / s;
      for (i = 0; i < ri2->size; i++)
	ri2->values[i] *= s1 / s2 / s;
    }

  if (haar_test == 1) {
    if (filternorm == 3) {
      haar_ri->values[0] = 1.0;
      haar_ri->values[1] = 1.0;
      shaar = 1.0;
    } else
      {
	haar_ri->values[0] = sqrt((double) 0.5);
	haar_ri->values[1] = sqrt((double) 0.5);
	shaar = sqrt((double) 0.5);
      }
    haar_ri->shift = 0.0;
  }

  /*--- Compute multiplicative factor for last odd coefficient ---*/

  s1 = s2 = 0.0;
  for (i = 0; i < ri1->size; i++)
    s1 += ri1->values[i];
  for (i = 0; i < ri2->size; i++)
    s2 += ri2->values[i];

  if (filternorm == 0)
    s2 *= s;

  if (s2 < 0.00001)
    mwerror(WARNING, 0, "Sum of ri2 coefficient almost equal to 0!\n");
}



static void
HAAR_INV_WAVEL2(wtrans, numrec, haar, filternorm)

Wtrans2d    wtrans;		/* Wavelet transform */
int         numrec;             /* Use average sub-image at level numrec */
int         haar;               /* Reconstruct with Haar wavelet
				 * from Haar level */
int	    filternorm;	        /* Type of normalisation : see adapowave2 */

{
  int          j, jx, jy;
  int          dx2, dy2;
  int          dx[MAX_NLEVEL], dy[MAX_NLEVEL];
  int          r, c;
  Fimage       Tab;
  int          dxtab;
  double       cnorm;

  dx[numrec] = wtrans->images[numrec][0]->ncol;
  dy[numrec] = wtrans->images[numrec][0]->nrow;
  for (j = numrec + 1; j <= haar; j++) {
    dx[j] = dx[j - 1] / 2;
    if (dx[j - 1] % 2 == 1)
      dx[j]++;
    dy[j] = dy[j - 1] / 2;
    if (dy[j - 1] % 2 == 1)
      dy[j]++;
  }

  jx = numrec;
  while (dx[jx] >= 2) 
    jx++;
  
  jy = numrec;
  while (dy[jy] >= 2) 
    jy++;

  dxtab = dx[numrec];
  cnorm = sqrt((double) 0.5);

  Tab = mw_new_fimage();
  if (mw_alloc_fimage(Tab, dy[numrec], dx[numrec]) == NULL)
    mwerror(FATAL, 1, "Allocation for sum's separation buffer refused!\n");

  for (j = haar - 1; j >= numrec; j--) {

    if (j < jx) {
      dx2 = dx[j+1];
      if (dx[j] % 2 == 1)
	dx2--;
      for (r = 0; r < dy[j]; r++)
	for (c = 0; c < dx2; c++) {
	  Tab->gray[r * dxtab + 2 * c] = wtrans->images[numrec][0]->gray[r * dxtab + c] + wtrans->images[numrec][0]->gray[r * dxtab + dx[j+1] + c];
	  Tab->gray[r * dxtab + 2 * c + 1] = wtrans->images[numrec][0]->gray[r * dxtab + c] - wtrans->images[numrec][0]->gray[r * dxtab + dx[j+1] + c];
	}
      if (dx[j] % 2 == 1)
	for (r = 0; r < dy[j]; r++)
	  Tab->gray[r * dxtab + dx[j] - 1] = wtrans->images[numrec][0]->gray[r * dxtab + dx2];

      for (r = 0; r < dy[j]; r++)
	for (c = 0; c < dx2; c++) {
	  wtrans->images[numrec][0]->gray[r * dxtab + c] = cnorm * Tab->gray[r * dxtab + c];
	  wtrans->images[numrec][0]->gray[r * dxtab + dx2 + c] = cnorm * Tab->gray[r * dxtab + dx2 + c];
	}
      if (dx[j] % 2 == 1)
	for (r = 0; r < dy[j]; r++)
	  wtrans->images[numrec][0]->gray[r * dxtab + dx[j] - 1] = shaar * Tab->gray[r * dxtab + dx[j] - 1];
    }

    if (j < jy) {
      dy2 = dy[j+1];
      if (dy[j] % 2 == 1)
	dy2--;
      for (r = 0; r < dy2; r++)
	for (c = 0; c < dx[j]; c++) {
	  Tab->gray[2 * r * dxtab + c] = wtrans->images[numrec][0]->gray[r * dxtab + c] + wtrans->images[numrec][0]->gray[(r + dy[j+1]) * dxtab + c];
	  Tab->gray[(2 * r + 1) * dxtab + c] = wtrans->images[numrec][0]->gray[r * dxtab + c] - wtrans->images[numrec][0]->gray[(r + dy[j+1]) * dxtab + c];
	}
      if (dy[j] % 2 == 1)
	for (c = 0; c < dx[j]; c++) 
	  Tab->gray[(dy[j] - 1) * dxtab + c] = wtrans->images[numrec][0]->gray[dy2 * dxtab + c];

      for (r = 0; r < dy2; r++)
	for (c = 0; c < dx[j]; c++) {
	  wtrans->images[numrec][0]->gray[r * dxtab + c] = cnorm * Tab->gray[r * dxtab + c];
	  wtrans->images[numrec][0]->gray[(r + dy2) * dxtab + c] = cnorm * Tab->gray[(r + dy2) * dxtab + c];
	}
      if (dy[j] % 2 == 1)
	for (c = 0; c < dx[j]; c++) 
	  wtrans->images[numrec][0]->gray[(dy[j] - 1) * dxtab + c] = shaar * Tab->gray[(dy[j] - 1) * dxtab + c];
    }

  }

}



static void
RECOMP_LINES(Tab, A, J, haarx, band, ri, edge)

     /*--- Computes the inverse wavelet transform of `S` along the lines ---*/

Fimage	    Tab;		/* Wavelet transform's sub-image */
Fimage      A;		        /* Reconstructed image */
int         J;		        /* Current level of recomposition */
int         haarx;              /* Flag for use of Haar filter */
int	   *band;   	        /* Indicates reconstruction 
				 * with low or high-pass filters */
Fsignal     ri;		        /* Impulse response of the low-pass filter */
int        *edge;		/* Type of edge processing (see `Edge`
				 * in ibiowave2) */

{
  long        c, l;		/* Indices for line and column in resume 
				 * or detail */
  long	      ldx, lK2;		/* Index of first element of line `l`  
				 * in A and Tab */
  long	      K2, L2;	       	/* Size of the output (resume at level J-1) */
  long	      dx1, dx2;	       	/* Size of input and output for sconcolve */
  long	      dx, dy;	       	/* Size of input (resume or detail 
				 * at level J) */
  Fsignal     Tabin;		/* One line of Tab input for `convo_sig` */
  Fsignal     Tabout;		/* Output for module `sconvolve`, 
				 * wavelet transform of `Tabin` */


  dx = dx1 = Tab->ncol;
  dy = Tab->nrow;
  K2 = dx * 2;
  L2 = dy;
  if (K2 > A->ncol)
    dx1--;
  dx2 = dx1 * 2;

  /*--- Initialization of Tabin ---*/

  Tabin = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabin, dx1) == NULL)
    mwerror(FATAL, 1, "Allocation of line buffer refused!\n");

  Tabout = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabout, dx2) == NULL)
    mwerror(FATAL, 1, "Allocation for line's buffer refused!\n");

  ldx = lK2 = 0;
  for (l = 0; l < dy; l++) {
    for (c = 0; c < dx1; c++)
      Tabin->values[c] = Tab->gray[ldx + c];

    if (haarx == 0)
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, NULL, band, edge, &PROLONG, ri, NULL);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, NULL, band, &EDGE_HAAR, &PROLONG, haar_ri, NULL);

    for (c = 0; c < dx2; c++)
      A->gray[lK2 + c] += Tabout->values[c];

    /*--- Copy last point if number of lines is odd ---*/

    if (K2 > A->ncol)
    {
      if (haarx == 0)
	A->gray[lK2 + c] = Tab->gray[ldx + dx1] / s2;
      else
	A->gray[lK2 + c] = shaar * Tab->gray[ldx + dx1];
    }
    ldx += dx;
    lK2 += A->ncol;
  }

  mw_delete_fsignal(Tabin);
  mw_delete_fsignal(Tabout);
}



static void
RECOMP_COLUMNS(Tab, AD, J, haary, band, ri, edge)

     /*--- Computes the inverse wavelet transform along the lines ---*/

Fimage      Tab, AD;	        /* Reconstructed image and wavelet sub-image */
int         J;		        /* Current level of recomposition */
int         haary;              /* Flag for use of Haar filter */
int	   *band;	        /* Indicates reconstruction 
				 * with low or high-pass filters */
Fsignal     ri;		        /* Impulse response of the low-pass filter */
int        *edge;	  	/* Type of edge processing (see `Edge`
				 * in iwav2d) */
{
  long        c, l;		/* Variables for line and column in resume 
				 * or detail */
  long	      ldxc;	       	/* Index of element at line `l` and column `c` 
				 * in `Tab` or `AD` */
  long	      K2, L2;	       	/* Size of the output (resume at level J-1) */
  long	      dy1, dy2;	       	/* Size of input and output for sconcolve */
  long	      dx, dy;	       	/* Size of input (wavelet transform along the 
				 * lines of image or resume at level J-1) */
  Fsignal     Tabin;		/* One column of AD, input for `convo_sig` */
  Fsignal     Tabout;		/* Output for module `sconvolve`, 
				 * wavelet transform of `Tabin` */

  dx = AD->ncol;
  dy = dy1 = AD->nrow;
  K2 = dx;
  L2 = dy * 2;
  if (L2 > Tab->nrow) 
    dy1--;
  dy2 = dy1 * 2;

  /*--- Initialization of Tabin ---*/

  Tabin = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabin, dy1) == NULL)
    mwerror(FATAL, 1, "Allocation of line buffer refused!\n");

  Tabout = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabout, dy2) == NULL)
    mwerror(FATAL, 1, "Allocation for line's buffer refused!\n");

  for (c = 0; c < K2; c++) {
    ldxc = c;
    for (l = 0; l < dy1; l++) {
      Tabin->values[l] = AD->gray[ldxc];
      ldxc += dx;
    }

    if (haary == 0)
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, NULL, band, edge, &PROLONG, ri, NULL);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, NULL, band, &EDGE_HAAR, &PROLONG, haar_ri, NULL);

    ldxc = c;
    for (l=0; l<dy2; l++) {
      Tab->gray[ldxc] += Tabout->values[l];
      ldxc += Tab->ncol;
    }

    /*--- Copy last point if number of lines is odd ---*/

    if (L2 > Tab->nrow)
    {
      if (haary == 0)
	Tab->gray[ldxc] = AD->gray[dx * dy1 + c] / s2;      
      else
	Tab->gray[ldxc] = shaar * AD->gray[dx * dy1 + c];      
    }
  }

  mw_delete_fsignal(Tabin);
  mw_delete_fsignal(Tabout);
}




static void
INV_WAVEL(J, haar, wtrans, A, ri1, ri2, edge)	

	/*----- Computes average at level J-1 
	  ----- from average and details at level J -----*/

int         J;			/* Current level of recomposition */
int        *haar;               /* Reconstruct with Haar wavelet
				 * from Haar level */
Wtrans2d    wtrans;		/* Wavelet transform */
Fimage      A;		        /* Reconstructed average */
Fsignal     ri1, ri2;	        /* Impulse response of the low-pass filter */
int        *edge;		/* Type of edge processing (see `Edge`
				 * in ibiowave2) */

{
  long        c, l;	       	/* Indices for line and column in resume 
				 * or detail */
  long	      lK2;	       	/* Index of first element of line `l` in A */
  long	      ldx;     	       	/* Index of first element of line `l` in Tab */
  long	      K2, L2;	       	/* Size of the output (resume at level J-1) */
  long	      dx, dy;	       	/* Size of input (resume or detail 
				 * at level J) */
  long	      dx2, dy2;	       	/* Double of dx and dy */
  Fimage      Tab;	       	/* Real buffer for sum's separation */
  int         haarflag;         /* Flag for use of Haar filter */

  dx = wtrans->images[J][0]->ncol;
  dy = wtrans->images[J][0]->nrow;
  dx2 = K2 = dx * 2;
  dy2 = L2 = dy * 2;
  if (dx > wtrans->images[J][3]->ncol)
    K2--;
  if (dy > wtrans->images[J][3]->nrow)
    L2--;

  /*--- Initialization of A and Tab ---*/

  Tab = mw_new_fimage();
  if (mw_change_fimage(Tab, L2, dx) == NULL)
    mwerror(FATAL, 1, "Allocation of buffer for sum's separation refused!\n");

  if ((A->nrow != L2) || (A->ncol != K2)) {
    A = mw_change_fimage(A, L2, K2);
    if (A == NULL)
      mwerror(FATAL, 1, "Allocation of buffer for average at level %d refused!\n", J);
  }

  /*--- Initialization of A and Tab ---*/

  lK2 = 0;
  ldx = 0;
  for (l = 0; l < L2; l++) {
    for (c = 0; c < K2; c++)
      A->gray[lK2 + c] = 0.0;
    lK2 += K2;

    for (c = 0; c < dx; c++)
      Tab->gray[ldx + c] = 0.0;
    ldx += dx;
  }

  if ((L2 >> 1 < ri1->size) || (L2 >> 1 < ri2->size))
    haarflag = 1;
  else 
    haarflag = 0;

  /*--- Reconstruction along the columns from the average ---*/

  RECOMP_COLUMNS(Tab, wtrans->images[J][0], J, haarflag, NULL, ri1, edge);

  /*--- Reconstruction along the columns from the detail D1 ---*/

  RECOMP_COLUMNS(Tab, wtrans->images[J][1], J, haarflag, &HIGH, ri2, edge);

  /*--- Reconstruction along the lines from average and detail D1 ---*/

  if ((K2 >> 1 < ri1->size) || (K2 >> 1 < ri2->size))
    haarflag = 1;
  else 
    haarflag = 0;

  RECOMP_LINES(Tab, A, J, haarflag, NULL, ri1, edge);


  /*--- Initialization of Tab ---*/

  if (dx > wtrans->images[J][2]->ncol) {
    dx = wtrans->images[J][2]->ncol;
    if (mw_change_fimage(Tab, L2, dx) == NULL)
      mwerror(FATAL, 1, "Allocation of buffer for sum's separation refused!\n");
  }

  ldx = 0;
  for (l = 0; l < L2; l++) {
    for (c = 0; c < dx; c++)
      Tab->gray[ldx + c] = 0.0;
    ldx += dx;
  }

  if ((L2 >> 1 < ri1->size) || (L2 >> 1 < ri2->size))
    haarflag = 1;
  else 
    haarflag = 0;

  /*--- Reconstruction along the columns from the detail D2 ---*/

  RECOMP_COLUMNS(Tab, wtrans->images[J][2], J, haarflag, NULL, ri1, edge);

  /*--- Reconstruction along the columns from the detail D3 ---*/

  RECOMP_COLUMNS(Tab, wtrans->images[J][3], J, haarflag, &HIGH, ri2, edge);

  /*--- Reconstruction along the lines from details D2 and D3 ---*/

  if ((K2 >> 1 < ri1->size) || (K2 >> 1 < ri2->size))
    haarflag = 1;
  else 
    haarflag = 0;

  RECOMP_LINES(Tab, A, J, haarflag, &HIGH, ri2, edge);

  mw_delete_fimage(Tab);

}





void
ibiowave2(NumRec, Haar, Edge, FilterNorm, Wtrans, Output, Ri1, Ri2)
	
	/*--- Reconstructs an image from the wavelet teansform Wtrans ---*/

int        *NumRec;		/* Number de recursion (-j) */
int        *Haar;               /* Reconstruct with Haar wavelet
				 * from Haar level */
int        *Edge;		/* Equal 0 (default) if extension with 0 */
				/* 1 if periodization */
				/* 2 if reflexion */
int        *FilterNorm;	        /* Equal 0 if no normalisation of filter's tap
				 *       1 if normalisation of the sum 
				 *       2 if normalistion of the square sum */
Wtrans2d    Wtrans;		/* Wavelet transform (input) */
Fimage      Output;		/* reconstructed image (output) */
Fsignal     Ri1, Ri2;		/* Impulse responses of the low pass filters */

{
  int         J;		/* Current level of decomposition */
  int         nrow, ncol;       /* Number of lines and columns 
				 * in reconstructed image */
  int         haar_test;        /* Test value for use of Haar filter */

    /*--- Detection of errors in command line ---*/

  COMLINE_ERR(Wtrans, Ri1, Ri2, *Edge, NumRec, Haar, &haar_test);

  /*--- Memory allocation for Haar filter ---*/

  if (haar_test == 1) {
    haar_ri = mw_new_fsignal();
    if (mw_alloc_fsignal(haar_ri, 2) == NULL)
      mwerror(FATAL, 1, "Not enough memory for Haar impulse response buffer.\n");
  } else
    haar_ri = NULL;

    /*--- Normalisation of filter banks ---*/

  NORM_FIL(Ri1, Ri2, *FilterNorm, haar_test);

    /*--- Memory allocation for inverse wavelet transform Output ---*/

  nrow = Wtrans->nrow;
  ncol = Wtrans->ncol;
  Output = mw_change_fimage(Output, nrow, ncol);	
  if(Output==NULL)
    mwerror(FATAL, 1, "Memory allocation refused for `RecompImage`!\n");

    /*--- Wavelet recomposition ---*/

  if (Haar) 
    if (*Haar > *NumRec)
      HAAR_INV_WAVEL2(Wtrans, *NumRec, *Haar, *FilterNorm);

  for (J = *NumRec; J > 1; J--)
    INV_WAVEL(J, Haar, Wtrans, Wtrans->images[J-1][0], Ri1, Ri2, Edge);

  INV_WAVEL(J, Haar, Wtrans, Output, Ri1, Ri2, Edge);

  if (Haar) 
    mw_delete_fsignal(haar_ri);

    /*--- Write commentary for Output ---*/

  COMMENT(Output, Wtrans, *Edge, FilterNorm, Ri1, Ri2);

}

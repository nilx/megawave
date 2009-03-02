/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {iowave2};
version = {"1.4"};
author = {"Jean-Pierre D'Ales"};
function = {"Reconstructs an image from an orthogonal wavelet transform"};
usage = {
 'r':[NLevel=0]->NumRec [0,20]   "Start reconstruction from level NLevel", 
 'h':HaarNLevel->Haar            "Start reconstruction with Haar filter from level HaarNLevel down to level NLevel + 1",
 'e':[EdgeMode=3]->Edge [0,3]         "Edge processing mode", 
 'p':[PrecondMode=0]->Precond [0,2]   "Edge preconditionning mode", 
 'i'->Inverse                         "Invertible transform", 
 'n':[FilterNorm=2]->FilterNorm [0,2] "Filter taps normalization", 
 WavTrans->Wtrans       "Input wavelet transform (wtrans2d)",
 RecompImage<-Output    "Output reconstructed image (fimage)",
 ImpulseResponse->Ri    "Impulse response of inner filters (fsignal)", 
	{
	EdgeIR->Edge_Ri
	    "Impulse reponses of edge and preconditionning filters (fimage)"
	}
	};
*/
/*----------------------------------------------------------------------
 v1.4 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for sconvolve(), precond2d() */

/*--- Constants ---*/

#define MAX_NLEVEL 20

/*--- Global variables ---*/

static int DECIM=1;		/* Decimation's constant */
static int INTERPOL=2;		/* Interpolation rate */
static int HIGH=1;		/* Index for high-pass filtering */
static int INVERSE=0;		/* Index for inverse preconditionning */
static int EDGE_HAAR=0;         /* Index for edge processing mode 
				 * with Haar filter */

static Fsignal      haar_ri;    /* Impulse response for Haar filter */
static double	    shaar;      /* Sums of coefficients of haar filter */
static double	    s;     	/* Sums of coefficients of `ri` */

static long	firstplow, firstphigh;
static long	lastcol[mw_max_nlevel][4];
static long	lastrow[mw_max_nlevel][4];
static long	nrow[mw_max_nlevel];
static long	ncol[mw_max_nlevel];



static void
COMLINE_ERR(Wtrans2d wtrans, Fsignal ri, Fimage edge_ri, int edge, int *inverse, int precond, int *numrec, int *haar)

	/*--- Detects errors and contradiction in command line ---*/

            	       		/* Wavelet transform */
           	   		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
          	        	/* Impulse responses of the edge filters
			 * (computation of wavelet coefficients near edges) */
        	     		/* Type of edge processing 
				 * (see `Edge` in iowave2) */
       	                	/* Performs invertible transform for edge
				 * processing mode 0 and 2 */
       		        	/* Type of edge preconditionning
				 * (see `Precond` in `iowave2`) */
                       		/* Number of levels for decomposition */
                                /* Reconstruct with Haar wavelet
				 * from haar level */

{
  long	dx, dy;			/* Size of the image */

  dx = wtrans->ncol;
  dy = wtrans->nrow;

  /*--- Number of levels ---*/

  if (*numrec == 0) 
    *numrec = wtrans->nlevel;

  if (wtrans->nlevel < *numrec) {
    mwerror(WARNING, 0, "Only %d levels in wavelet transform!\n", wtrans->nlevel);
    *numrec = wtrans->nlevel;
  }

  /*--- Edge processing selection ---*/

  if((edge_ri==NULL) && (edge==3))
    mwerror(USAGE, 1, "Edge filters i.r. are needed!\n");

  if(((edge == 1) || (edge == 3)) && inverse)
    mwerror(WARNING, 0, "Edge processing mode is already invertible!\n");

  /*--- Preconditionning and special edge processing ---*/

  if((edge != 3) && (precond >= 1))
    mwerror(USAGE, 1, "Preconditionning only with special edge processing\n");

  /*--- Size of image ---*/

  if(!haar)
    if((dx>>(*numrec) < ri->size) || (dy>>(*numrec) < ri->size))
      mwerror(FATAL, 1, "The size of average must be greater than the size of the i.r.!\n");

  /*--- Inner and edge filters compatibility ---*/

  if((edge == 3) && ((edge_ri->nrow != 4 * ri->size) || (edge_ri->ncol != 3 * ri->size / 2 - 1)))
    mwerror(FATAL, 1, "Inner and edge filters are not compatible!\n");

}



static void
COMMENT(Fimage result, Wtrans2d wtrans, int edge, int precond, int *filternorm)

	/*--- Fill comment and other fields for result ---*/

                       		/* Inverse wavelet transform of `wtrans` */
                       		/* Input wavelet transform */
        	     		/* Type of edge processing 
				 * (see `Edge` in `wav2d`) */
       		        	/* Type of edge preconditionning
				 * (see `Precond` in `wav2d`) */
       	                   	/* Type of normalisation :
				 * equal 0 if normalisation of the sum of 
				 *         `ri`'s coefficients to 1.0
			 	 *       1 if normalisation of the squares' sum 
			 	 *         `ri`'s coefficients to 1.0 */
           	   		/* Impulse response of the low-pass filter 
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
    if(*filternorm >= 1)
	strcat(result->cmt, ", filters coef. norm.");

}



static void
INIT_SHRINK(Wtrans2d wtrans, Fsignal ri, short int numrec, int edge)

	/*--- Computation of indices for shrinkage ---*/

            	       		/* Wavelet transform */
           	   		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
         	       		/* Number of levels for decomposition */
                     		/* Type of edge processing
				 * (see `Edge` in iowave2) */

{
    short j;		/* Indices for orientation and level */

    if ((edge == 2) || (edge == 0))
    {
	if ((1 - ri->size - (long) ri->shift) < 0) 
	    firstplow = - (1 - ri->size - (long) ri->shift) / 2;
	else
	    firstplow = (ri->size + (long) ri->shift) / 2;

	if (((long) ri->shift - 1) < 0) 
	    firstphigh = - ((long) ri->shift - 1) / 2;
	else
	    firstphigh = - (long) ri->shift / 2;

	ncol[0] = wtrans->ncol;
	nrow[0] = wtrans->nrow;
	for (j=1; j<=numrec; j++)
	{
	    lastcol[j][0] = lastcol[j][1] = firstplow + ncol[j-1] / 2 - 1;
	    lastcol[j][2] = lastcol[j][3] = firstphigh + ncol[j-1] / 2 - 1;
	    ncol[j] = firstphigh + (ncol[j-1] - 3 + ri->size + (long) ri->shift) / 2 + 1;
	    if ((wtrans->images[j][2]->ncol != ncol[j]) || (wtrans->images[j][3]->ncol != ncol[j]))
		mwerror(FATAL, 1, "Bad number of column in detail!\n");

	    ncol[j] = firstplow + (ncol[j-1] - 1 - (long) ri->shift) / 2 + 1;
	    if (wtrans->images[j][1]->ncol != ncol[j])
		mwerror(FATAL, 1, "Bad number of column in detail!\n");

	    lastrow[j][0] = lastrow[j][2] = firstplow + nrow[j-1] / 2 - 1;
	    lastrow[j][1] = lastrow[j][3] = firstphigh + nrow[j-1] / 2 - 1;
	    nrow[j] = firstphigh + (nrow[j-1] - 3 + ri->size + (long) ri->shift) / 2 + 1;
	    if ((wtrans->images[j][1]->nrow != nrow[j]) || (wtrans->images[j][3]->nrow != nrow[j]))
		mwerror(FATAL, 1, "Bad number of row in detail!\n");

	    nrow[j] = firstphigh + (nrow[j-1] - 1 - (long) ri->shift) / 2 + 1;
	    if (wtrans->images[j][2]->nrow != nrow[j])
		mwerror(FATAL, 1, "Bad number of row in detail!\n");

	}
	if (wtrans->images[numrec][0]->ncol != ncol[numrec])
	    mwerror(FATAL, 1, "Bad number of column in average!\n");
	if (wtrans->images[numrec][0]->nrow != nrow[numrec])
	    mwerror(FATAL, 1, "Bad number of row in average!\n");

    }

}
	



static void
NORM_FIL(Fsignal ri, Fimage edge_ri, int filternorm, int edge, int *haar)

	/*--- Normalisation of the coefficients of the filter impulse 
	 *--- responses ---*/

       	              	/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
      	                /* Impulse responses of the edge filters
			 * (computation of wavelet coefficients near edges) */
   	                   /* Type of normalisation :
			    * equal 0 if normalisation of the sum of 
			    *         `ri`'s coefficients to 1.0
			    *       1 if normalisation of the squares' sum
			    *         of `ri`'s coefficients to 1.0 */
                    	 /* Type of edge processing
			  * (see `Edge` in iowave2) */
                         /* Reconstruct with Haar wavelet
			  * from Haar level */

{
  double s2;	        /* Sum of `ri`'s coefficients and sum's square root 
			 * of `ri`'s coefficients' squares */
  short i,j;		/* Indices for orientation and level */


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
    if (edge == 3) {
      for (i = 0; i < edge_ri->nrow / 2; i++)
	for (j = 0; j < edge_ri->ncol; j++)
	  edge_ri->gray[edge_ri->ncol * i + j] *= s;
    }
  }

  if (haar) {
    if (filternorm == 1) {
      haar_ri->values[0] = 1.0;
      haar_ri->values[1] = 1.0;
      shaar = 1.0;
    } else
      {
	haar_ri->values[0] = sqrt((double) 0.5);
	haar_ri->values[1] = sqrt((double) 0.5);
	shaar = sqrt((double) 2.0);
      }
    haar_ri->shift = 0.0;
  }

  /*--- Compute multiplicative factor for last odd coefficient ---*/

  if (filternorm != 1) {
    s = 0.0;
    for (i = 0; i < ri->size; i++)
      s += ri->values[i];
  }

  if (s < 0.00001)
    mwerror(WARNING, 0, "Sum of ri coefficient almost equal to 0!\n");
}


static void
HAAR_INV_WAVEL2(Wtrans2d wtrans, int numrec, int haar, int filternorm)

                   		/* Wavelet transform */
                                /* Use average sub-image at level numrec */
                                /* Reconstruct with Haar wavelet
				 * from Haar level */
   	               	        /* Type of normalisation : see adapowave2 */

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
  if (filternorm == 1)
    cnorm = 1.0;
  else
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
	  wtrans->images[numrec][0]->gray[r * dxtab + dx[j] - 1] = Tab->gray[r * dxtab + dx[j] - 1] / shaar;
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
	  wtrans->images[numrec][0]->gray[(dy[j] - 1) * dxtab + c] = Tab->gray[(dy[j] - 1) * dxtab + c] / shaar;
    }

  }

}



static void
RECOMP_LINES(Fimage Tab, Fimage A, int J, int *haar, int *band, Fsignal int_ri, int prolong, int *edge, Fimage edge_ri)

	/*--- Computes the inverse wavelet transform of `S` 
						along the lines ---*/

      	               	      /* Wavelet transform's sub-image */
              		      /* Reconstructed image */
              		      /* Current level of recomposition */
                              /* Reconstruct with Haar wavelet
			       * from Haar level */
   	         	      /* Indicates reconstruction 
			       * with low or high-pass filters */
                   	      /* Impulse response of the low-pass filter 
			   * (computation of the inner wavelet coefficients) */
    	            	      /* Indicates signal shrinkage 
			       * for invertibility */
                 	      /* Type of edge processing (see `Edge`
			       * in iowave2) */
                    	      /* Impulse responses of filters for special 
			       * edge processing */

{
  long        c, l;	        /* Indices for line and column in resume 
			         * or detail */
  long	      ldx, lK2;	        /* Index of first element of line `l`  
			         * in Tab and A */
  long	      K2, L2;		/* Size of the output (resume at level J-1) */
  long	      dx1, dx2;	       	/* Size of input and output for sconcolve */
  long	      dx, dy;	        /* Size of input (resume or detail 
			         * at level J) */
  Fsignal     Tabin;	        /* One line of Tab input for `convo_sig` */
  Fsignal     Tabout;	        /* Output for module `sconvolve`, 
			         * wavelet transform of `Tabin` */
  int         haarx;	        /* Flag for use of Haar filter */


  if (prolong == 0) {
    dx = dx1 = Tab->ncol;
    dy = Tab->nrow;
    K2 = dx * 2;
    L2 = dy;
    if (K2 > A->ncol)
      dx1--;
    dx2 = dx1 * 2;
  } else
    {
      dx = dx1 = Tab->ncol;
      dy = Tab->nrow;
      K2 = dx2 = A->ncol;
      L2 = A->nrow;

    }
  /*--- Initialization of Tabin and Tabout ---*/

  Tabin = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabin, dx1) == NULL)
    mwerror(FATAL, 1, "Allocation of line buffer refused!\n");

  Tabout = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabout, dx2) == NULL)
    mwerror(FATAL, 1, "Allocation of line buffer refused!\n");

  haarx = 0;
  if (haar && (dx1 < int_ri->size)) {
    haarx = 1;
    prolong = 0;
  }

  if (prolong == 2) {
    if (!band) {
      Tabin->firstp = firstplow;
      Tabin->lastp = lastcol[J][0];
    } else
      {
	Tabin->firstp = firstphigh;
	Tabin->lastp = lastcol[J][3];
      }

    Tabin->param = (float) (K2 % 2);
  }

  ldx = lK2 = 0;
  for (l = 0; l < dy; l++) {
    for (c = 0; c < dx1; c++)
      Tabin->values[c] = Tab->gray[ldx + c];

    if (haarx == 0)
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, NULL, band, edge, &prolong, int_ri, edge_ri);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, NULL, band, &EDGE_HAAR, &prolong, haar_ri, NULL);

    for (c = 0; c < dx2; c++)
      A->gray[lK2 + c] += Tabout->values[c];

    /*--- Copy last point if number of lines is odd ---*/

    if ((K2 > A->ncol) && (prolong == 0))
    {
      if (haar == 0)
	A->gray[lK2 + c] = Tab->gray[ldx + dx1] / s;
      else
	A->gray[lK2 + c] = Tab->gray[ldx + dx1] / shaar;
    }
    ldx += dx;
    lK2 += A->ncol;
  }

  mw_delete_fsignal(Tabin);
  mw_delete_fsignal(Tabout);
}



static void
RECOMP_COLUMNS(Fimage Tab, Fimage AD, int J, int *haar, int *band, Fsignal int_ri, int prolong, int *edge, Fimage edge_ri)

	/*--- Computes the inverse wavelet transform along the lines ---*/

                    	      /* Reconstructed image and wavelet sub-image */
              	 	      /* Current level of recomposition */
                              /* Reconstruct with Haar wavelet
			       * from Haar level */
   	         	      /* Indicates reconstruction 
			       * with low or high-pass filters */
                   	      /* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
    	            	      /* Indicates signal shrinkage 
			       * for invertibility */
                 	      /* Type of edge processing (see `Edge`
			       * in iowave2) */
                    	      /* Impulse responses of filters for special 
			       * edge processing */

{
  long          c, l;		/* Variables for line and column in resume 
				 * or detail */
  long	        ldxc;	       	/* Index of element at line `l` and column `c` 
			       	 * in `Tab` or `AD` */
  long	        K2, L2;	       	/* Size of the output (resume at level J-1) */
  long	        dy1, dy2;       /* Size of input and output for sconcolve */
  long	        dx, dy;	       	/* Size of input (wavelet transform along the 
			       	 * lines of image or resume at level J-1) */
  Fsignal       Tabin;	       	/* One column of AD, input for `convo_sig` */
  Fsignal       Tabout;	       	/* Output for module `sconvolve`, 
			       	 * wavelet transform of `Tabin` */
  int           haary;	       	/* Flag for use of Haar filter */


  if (prolong == 0) {
    dx = AD->ncol;
    dy = dy1 = AD->nrow;
    K2 = dx;
    L2 = dy * 2;
    if (L2 > Tab->nrow) 
      dy1--;
    dy2 = dy1 * 2;
  } else
    {
      dx = AD->ncol;
      dy = dy1 = AD->nrow;
      K2 = Tab->ncol;
      L2 = dy2 = Tab->nrow;
    }

  /*--- Initialization of Tabin ---*/

  Tabin = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabin, dy1) == NULL)
    mwerror(FATAL, 1, "Allocation of line buffer refused!\n");

  Tabout = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabout, dy2) == NULL)
    mwerror(FATAL, 1, "Allocation of line buffer refused!\n");

  haary = 0;
  if (haar && (dy1 < int_ri->size)) {
    haary = 1;
    prolong = 0;
  }

  if (prolong == 2) {
    if (!band) {
      Tabin->firstp = firstplow;
      Tabin->lastp = lastrow[J][0];
    } else
      {
	Tabin->firstp = firstphigh;
	Tabin->lastp = lastrow[J][3];
      }

    Tabin->param = (float) (L2 % 2);
  }

  for (c = 0; c < K2; c++) {
    ldxc = c;
    for (l = 0; l < dy1; l++) {
      Tabin->values[l] = AD->gray[ldxc];
      ldxc += dx;
    }

    if (haary == 0)
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, NULL, band, edge, &prolong, int_ri, edge_ri);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, NULL, band, &EDGE_HAAR, &prolong, haar_ri, NULL);

    ldxc = c;
    for (l=0; l<dy2; l++) {
      Tab->gray[ldxc] += Tabout->values[l];
      ldxc += Tab->ncol;
    }

    /*--- Copy last point if number of lines is odd ---*/

    if ((L2 > Tab->nrow) && (prolong == 0))
    {
      if (haar == 0)
	Tab->gray[ldxc] = AD->gray[dx * dy1 + c] / s;      
      else
	Tab->gray[ldxc] = AD->gray[dx * dy1 + c] / shaar;      
    }
  }

  mw_delete_fsignal(Tabin);
  mw_delete_fsignal(Tabout);
}




static void
INV_WAVEL(int J, int *haar, Wtrans2d wtrans, Fimage A, Fsignal int_ri, int *inverse, int *edge, Fimage edge_ri)	

	/*----- Computes average at level J-1 
	  ----- from average and details at level J -----*/

              		      /* Current level of recomposition */
                              /* Reconstruct with Haar wavelet
			       * from Haar level */
                   	      /* Wavelet transform */
      	  		      /* Reconstructed average */
                   	      /* Impulse response of the low-pass filter 
			       * (computation of the inner wavelet coefficients) */
   	            	      /* Performs invertible transform for edge
			       * processing mode 0 and 2 */
                 	      /* Type of edge processing (see `Edge`
			       * in iowave2) */
                    	      /* Impulse responses of filters for special 
			       * edge processing */

{
  long        c, l;	       	/* Indices for line and column in resume 
			       	 * or detail */
  long	      lK2;	       	/* Index of first element of line `l` in A */
  long	      ldx;	       	/* Index of first element of line `l` in Tab */
  long	      K2, L2;	       	/* Size of the output (resume at level J-1) */
  long	      dx, dy;	       	/* Size of input (resume or detail 
			       	 * at level J) */
  long	      dx2, dy2;	       	/* Double of dx and dy */
  Fimage      Tab;	       	/* Real buffer for sum's separation */
  int  	      prolong;	       	/* Indicates signal shrinkage 
			       	 * for invertibility */

  dx = wtrans->images[J][0]->ncol;
  dy = wtrans->images[J][0]->nrow;


  if(!inverse || (*edge == 1) || (*edge == 3)) {
    prolong = 0;
    dx2 = K2 = dx * 2;
    dy2 = L2 = dy * 2;
    if (dx > wtrans->images[J][3]->ncol)
      K2--;
    if (dy > wtrans->images[J][3]->nrow)
      L2--;
    if ((A->nrow != L2) || (A->ncol != K2)) {
      A = mw_change_fimage(A, L2, K2);
      if (A == NULL)
	mwerror(FATAL, 1, "Allocation of buffer for average at level %d refused!\n", J);
    }

  } else 
    {
      prolong = 2;
      dx2 = K2 = ncol[J-1];
      dy2 = L2 = nrow[J-1];
      A = mw_change_fimage(A, L2, K2);
      if (A == NULL)
	mwerror(FATAL, 1, "Reallocation of buffer for reconstructed average refused!\n");
    }

  /*--- Initialization of Tab ---*/

  Tab = mw_new_fimage();
  if (mw_alloc_fimage(Tab, L2, dx) == NULL)
    mwerror(FATAL, 1, "Allocation of buffer for sum's separation refused!\n");

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


  /*--- Reconstruction along the columns from the average ---*/

  RECOMP_COLUMNS(Tab, wtrans->images[J][0], J, haar, NULL, int_ri, prolong, edge, edge_ri);

  /*--- Reconstruction along the columns from the detail D1 ---*/

  RECOMP_COLUMNS(Tab, wtrans->images[J][1], J, haar, &HIGH, int_ri, prolong, edge, edge_ri);

  /*--- Reconstruction along the lines from average and detail D1 ---*/

  RECOMP_LINES(Tab, A, J, haar, NULL, int_ri, prolong, edge, edge_ri);


  /*--- Initialization of Tab ---*/

  if ((dx > wtrans->images[J][2]->ncol) && (prolong == 0)) {
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

  /*--- Reconstruction along the columns from the detail D2 ---*/

  RECOMP_COLUMNS(Tab, wtrans->images[J][2], J, haar, NULL, int_ri, prolong, edge, edge_ri);

  /*--- Reconstruction along the columns from the detail D3 ---*/

  RECOMP_COLUMNS(Tab, wtrans->images[J][3], J, haar, &HIGH, int_ri, prolong, edge, edge_ri);

  /*--- Reconstruction along the lines from details D2 and D3 ---*/

  RECOMP_LINES(Tab, A, J, haar, &HIGH, int_ri, prolong, edge, edge_ri);

  mw_delete_fimage(Tab);

}




void
iowave2(int *NumRec, int *Haar, int *Edge, int *Precond, int *Inverse, int *FilterNorm, Wtrans2d Wtrans, Fimage Output, Fsignal Ri, Fimage Edge_Ri)
	
	/*--- Reconstructs an image from the wavelet teansform Wtrans ---*/

                   		/* Number de recursion (-j) */
                                /* Reconstruct with Haar wavelet
				 * from Haar level */
                 		/* Equal 0 if extension with 0 
				 * 1 if periodization 
				 * 2 if reflexion 
				 * 3 (default) if special treatment of edges */
                    	        /* Equal 0 (default) if no
				 *	   (un)preconditionning
				 * 	 1 if preconditionning only 
				 * 	 2 if preconditionning and
				 * 	   unpreconditionning */
                    	        /* Performs invertible transform for edge
				 * processing mode 0 and 2 */
                       	        /* Equal 1 if normalisation of filter's
				 * impulse responses (sum = 1.0) */
                   		/* Wavelet transform (input) */
                   		/* Reconstructed image (output) */
               		        /* Impulse response of the low pass filter */
                    	        /* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
{
  int         J;		/* Current level of decomposition */


  /*--- Detection of errors in command line ---*/

  COMLINE_ERR(Wtrans, Ri, Edge_Ri, *Edge, Inverse, *Precond, NumRec, Haar);

  /*--- Memory allocation for inverse wavelet transform Result ---*/

  Output = mw_change_fimage(Output, Wtrans->nrow, Wtrans->ncol);
  if(Output == NULL)
    mwerror(FATAL, 1, "Allocation for reconstructed image refused!\n");

  /*--- Write commentary for Output ---*/

  COMMENT(Output, Wtrans, *Edge, *Precond, FilterNorm);

    /*--- Memory allocation for Haar filter ---*/

  if (Haar) {
    haar_ri = mw_new_fsignal();
    if (mw_alloc_fsignal(haar_ri, 2) == NULL)
      mwerror(FATAL, 1, "Not enough memory for Haar impulse response buffer.\n");
  }

  /*--- Normalisation of filter banks ---*/

  NORM_FIL(Ri, Edge_Ri, *FilterNorm, *Edge, Haar);

  /*--- Computation of position indices for shrinkage ---*/

  if (Inverse)
    INIT_SHRINK(Wtrans, Ri, *NumRec, *Edge);

  /*--- Preconditionning of resume's edges (if selected) ---*/

  if (*Precond >= 2)
    precond2d(NULL, Wtrans->images[*NumRec][0], Wtrans->images[*NumRec][0], Edge_Ri);


  /*--- Wavelet recomposition ---*/

  if (Haar) 
    if (*Haar > *NumRec)
      HAAR_INV_WAVEL2(Wtrans, *NumRec, *Haar, *FilterNorm);

  for (J = *NumRec; J > 1; J--)
    INV_WAVEL(J, Haar, Wtrans, Wtrans->images[J-1][0], Ri, Inverse, Edge, Edge_Ri);

  INV_WAVEL(J, Haar, Wtrans, Output, Ri, Inverse, Edge, Edge_Ri);

  if (Haar) 
    mw_delete_fsignal(haar_ri);

  /*--- Unpreconditionning of image's edges (if selected) ---*/

  if (*Precond >= 1)
    precond2d(&INVERSE, Output, Output, Edge_Ri);
}

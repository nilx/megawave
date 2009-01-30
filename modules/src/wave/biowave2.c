/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {biowave2};
version = {"1.5"};
author = {"Jean-Pierre D'Ales"};
function = {"Computes the biorthogonal wavelet transform of an image"};
usage = {
 'r':[NLevel=1]->NumRec [1,20]	"Number of levels", 
 'h':HaarNLevel->Haar            "Continue decomposition with Haar filter until level HaarNLevel",
 'e':[EdgeMode=2]->Edge [0,2]	"Edge processing mode", 
 'n':[FilterNorm=0]->FilterNorm [0,2]  "Normalization mode for filter bank", 
 Image->Image          "Input image (fimage)", 
 WavTrans<-Output      "Output wavelet transform of Image (wtrans2d)", 
 ImpulseResponse1->Ri1 "Impulse response of filter 1 (fsignal)", 
 ImpulseResponse2->Ri2 "Impulse response of filter 2 (fsignal)"
	};
*/
/*-------------------------------------------------------------------------
 v1.4: fixed bug with default number of levels (L.Moisan)
 v1.5 (04/2007): simplified header (LM)
-------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for sconvolve() */

/*--- Constants ---*/

/*--- Global variables ---*/

static int DECIM=2;		/* Decimation's constant */
static int INTERPOL=1;		/* Interpolation rate */
static int REFLIR=1;	        /* Indicator for reflexion of filter's ir */
static int HIGH=1;		/* Index for high-pass filtering */
static int PROLONG=0;		/* Index for inverse preconditionning */
static int EDGE_HAAR=0;         /* Index for edge processing mode 
				 * with Haar filter */

static Fsignal      haar_ri;    /* Impulse response for Haar filter */
static double	    shaar;      /* Sums of coefficients of haar filter */
static double	    s1,s2;     	/* Sums of coefficients of `ri1` and `ri2` */



static void
COMMENT(Wtrans2d result, Fimage image, int edge, int *filternorm, Fsignal ri1, Fsignal ri2)

	/*--- Fill comment and other fields for result ---*/

                   		/* Wavelet transform of the image `Image` */
                  		/* Input image */
    	         		/* Type of edge processing 
				 * (see `Edge` in `biowave2`) */
   	               	        /* Type of normalisation :
				 * equal 0 if normalisation of the sum of 
				 *         `ri`'s coefficients to 1.0
			 	 *       1 if normalisation of the squares' 
			 	 *         sum to 1.0 */
       	         	        /* Impulse responses of the low-pass filters */

{
  result->edges = edge;
  result->type = mw_biorthogonal;
  result->nfilter = 2;
  strcpy(result->filter_name[0], ri1->name);
  strcpy(result->filter_name[1], ri2->name);
  strcpy(result->cmt, "Wav. Trans. of ");
  strcat(result->cmt, image->name);
  if(filternorm)
    strcat(result->cmt, ", filters coef. norm.");

}



static void
COMLINE_ERR(Fsignal ri1, Fsignal ri2, int edge, int *numrec, int *haar, int dy, int dx)

	/*--- Detects errors and contradiction in command line ---*/

       	            	/* Impulse responses of the low-pass filters */
    	               	/* Type of edge processing 
			 * (see `Edge` in biowave2) */
                      	/* Number of levels for decomposition */
                        /* Continue decomposition with Haar wavelet
		         * until ultimate level */
   	               	/* Size of the image */

{

	/*--- Size of image ---*/

  if((dx>>(*numrec) < 1) || (dy>>(*numrec) < 1)) {
    mwerror(WARNING, 1, "The number of level is too big!\n");
    (*numrec)--;
    while ((dx>>(*numrec) < 1) || (dy>>(*numrec) < 1))
      (*numrec)--;
  }

  if (*numrec <= 0)
    mwerror(FATAL, 1, "The size of image must be greater than 2x2!\n");

  if (!haar)
    if (edge >= 1) {
      if((dx>>(*numrec) < ri1->size) || (dy>>(*numrec) < ri1->size))
	mwerror(FATAL, 1, "The size of image must be greater than the size of the i.r. 1!\n");

      if((dx>>(*numrec) < ri2->size) || (dy>>(*numrec) < ri2->size))
	mwerror(FATAL, 1, "The size of image must be greater than the size of the i.r. 2!\n");
    }

}




static void
NORM_FIL(Fsignal ri1, Fsignal ri2, int filternorm, int *haar)

	/*--- Normalisation of the coefficients of the filter impulse 
	 *--- responses ---*/

       	         	/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
   	           	/* Type of normalisation :
			 * equal 0 if normalisation of the sum of 
			 *         `ri`'s coefficients to 1.0
			 *       1 if normalisation of the squares' sum 
			 *         `ri`'s coefficients to 1.0 */
                        /* Continue decomposition with Haar wavelet
			 * until haar level */

{
  double	s;		 /* Cross correlation of `ri1` and `ri2` */
  int           test;	         /* Indicates biorthogonality of filters */
  short	        i,j;
  short	        shift1, shift2;



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

  if (haar) {
    if (filternorm == 3) {
      haar_ri->values[0] = 0.5;
      haar_ri->values[1] = 0.5;
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

  s1 = s2 = 0.0;
  for (i = 0; i < ri1->size; i++)
    s1 += ri1->values[i];
  for (i = 0; i < ri2->size; i++)
    s2 += ri2->values[i];

  if (s2 < 0.00001)
    mwerror(WARNING, 0, "Sum of ri2 coefficient almost equal to 0!\n");

}




static void
HAAR_WAVEL2(Wtrans2d wtrans, int numrec, int haar, int filternorm)

                    		/* Wavelet transform */
                                /* Level of average */
                                /* Continue decomposition with Haar wavelet
				 * until ultimate level */
   	                	/* Type of normalisation : see 'owave2' */

{
  int          j;
  int          dx, dy;
  int          dx2, dy2;
  int          dxres, dyres;
  int          r, c;
  Fimage       Tab;
  int          dxtab;
  double       cnorm;

  dx = dx2 = wtrans->images[numrec][0]->ncol;
  dy = dy2 = wtrans->images[numrec][0]->nrow;
  dxtab = dx;
  cnorm = sqrt((double) 0.5);

  Tab = mw_new_fimage();
  if (mw_alloc_fimage(Tab, dy, dx) == NULL)
    mwerror(FATAL, 1, "Allocation for sum's separation buffer refused!\n");

  j = numrec;
  while (((dx >= 2) || (dy >= 2)) && (j < haar)) {

    if (dx >= 2) {
      dx2 = dxres = dx / 2;
      if (dx % 2 == 1)
	dxres++;
      for (r = 0; r < dy; r++)
	for (c = 0; c < dx2; c++) {
	  Tab->gray[r * dxtab + c] = wtrans->images[numrec][0]->gray[r * dxtab + 2 * c] + wtrans->images[numrec][0]->gray[r * dxtab + 2 * c + 1];
	  Tab->gray[r * dxtab + dxres + c] = wtrans->images[numrec][0]->gray[r * dxtab + 2 * c] - wtrans->images[numrec][0]->gray[r * dxtab + 2 * c + 1];
	}
      if (dx % 2 == 1)
	for (r = 0; r < dy; r++)
	  Tab->gray[r * dxtab + dx2] = wtrans->images[numrec][0]->gray[r * dxtab + dx - 1];

      for (r = 0; r < dy; r++)
	for (c = 0; c < dx2; c++) {
	  wtrans->images[numrec][0]->gray[r * dxtab + c] = cnorm * Tab->gray[r * dxtab + c];
	  wtrans->images[numrec][0]->gray[r * dxtab + dxres + c] = cnorm * Tab->gray[r * dxtab + dxres + c];
	}
      if (dx % 2 == 1)
	for (r = 0; r < dy; r++)
	  wtrans->images[numrec][0]->gray[r * dxtab + dx2] = shaar * Tab->gray[r * dxtab + dx2];
      
    }

    if (dy >= 2) {
      dy2 = dyres = dy / 2;
      if (dy % 2 == 1)
	dyres++;
      for (r = 0; r < dy2; r++)
	for (c = 0; c < dx; c++) {
	  Tab->gray[r * dxtab + c] = wtrans->images[numrec][0]->gray[2 * r * dxtab + c] + wtrans->images[numrec][0]->gray[(2 * r + 1) * dxtab + c];
	  Tab->gray[(r + dyres) * dxtab + c] = wtrans->images[numrec][0]->gray[2 * r * dxtab + c] - wtrans->images[numrec][0]->gray[(2 * r + 1) * dxtab + c];
	}
      if (dy % 2 == 1)
	for (c = 0; c < dx; c++) 
	  Tab->gray[dy2 * dxtab + c] = wtrans->images[numrec][0]->gray[(dy - 1) * dxtab + c];

      for (r = 0; r < dy2; r++)
	for (c = 0; c < dx; c++) {
	  wtrans->images[numrec][0]->gray[r * dxtab + c] = cnorm * Tab->gray[r * dxtab + c];
	  wtrans->images[numrec][0]->gray[(r + dyres) * dxtab + c] = cnorm * Tab->gray[(r + dyres) * dxtab + c];
	}
      if (dy % 2 == 1)
	for (c = 0; c < dx; c++) 
	  wtrans->images[numrec][0]->gray[dy2 * dxtab + c] = shaar * Tab->gray[dy2 * dxtab + c];
    }

    dx = dxres;
    dy = dyres;
    j++;
  }

}


static void
COLUMN_WAVEL(Fimage Im, Fimage Im1, Fimage Im2, int J, int *haar, int *edge, Fsignal ri1, Fsignal ri2)

	/*--- Computes the 1-D wavelet transform of each column  ---*
	 *--- in image "Tab", puts the result in "Im1" and "Im2" ---*/

      	       		/* Input (wavelet transform along the lines
			 * of image or resume at level J-1) */
                     	/* Low and high-pass filtered sub-images */
              	        /* Level of decomposition */
                        /* Continue decomposition with Haar wavelet
			 * until haar level */

                      	/* Type of edge processing (see `Edge`
			 * in biowave2) */
                     	/* Impulse responses of the low-pass filters */

{
  int      c, l;		/* Variables for line and column in resume 
			       	 * or detail */
  long	    ldxc;      		/* Index of element at line `l` and column `c` 
			       	 * in Im, Im1, or Im2 */
  int	    K2, L2;	       	/* Size of the output (low-pass) */
  int       dy1, dy2;           /* Size of the input and output of sconvolve */
  int	    dx, dy;	       	/* Size of input (wavelet transform along the 
			       	 * lines of image or resume at level J-1) */
  Fsignal   Tabin;	       	/* One column of Im, input for `convo_sig` */
  Fsignal   Tabout;	       	/* Output for module `sconvolve`, 
			       	 * wavelet transform of `Tabin` */
  int       haary;	       	/* Flag for use of Haar filter */

  dx = Im->ncol;
  dy = dy1 = Im->nrow;
  K2 = dx;
  L2 = dy2 = dy >> 1;
  if (dy % 2 == 1) {
    L2++;
    dy1--;
  }

  haary = 0;
  if (haar && ((dy2 < ri1->size) || (dy2 < ri2->size)))
    haary = 1;

  /*--- Initialization of Tabin and Tabout ---*/

  Tabin = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabin, dy1) == NULL)
    mwerror(FATAL, 1, "Allocation of column buffer refused!\n");

  Tabout = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabout, dy2) == NULL)
    mwerror(FATAL, 1, "Allocation of column buffer refused!\n");

  /*--- Initialization of Im1 and Im2 ---*/

  if ((L2 != Im1->nrow) || (dx != Im1->ncol)) {
    Im1 = mw_change_fimage(Im1, L2, dx);
    if (Im1 == NULL)
      mwerror(FATAL, 1, "Reallocation for average buffer refused!\n");
  }

  if ((dy2 != Im2->nrow) || (dx != Im2->ncol)) {
    Im2 = mw_change_fimage(Im2, dy2, dx);
    if (Im2 == NULL)
      mwerror(FATAL, 1, "Reallocation for detail buffer refused!\n");
  }


  for (c = 0; c < K2; c++) {
    ldxc = c;
    for (l=0; l<dy1; l++) {
      Tabin->values[l] = Im->gray[ldxc];
      ldxc += dx;
    }

    /*--- Convolution with low-pass filter ---*/

    if (haary == 0)
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, NULL, edge, &PROLONG, ri2, NULL);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, NULL, &EDGE_HAAR, &PROLONG, haar_ri, NULL);

    /*--- Copy result in Im1 buffer --- */

    ldxc = c;
    for (l=0; l<dy2; l++) {
      Im1->gray[ldxc] = Tabout->values[l];
      ldxc += dx;
    }

    /*--- Copy last point if number of lines is odd ---*/

    if (L2 > dy2)
    {
      if (haary == 0)
	Im1->gray[ldxc] = s2 * Im->gray[dy1 * dx + c];
      else
	Im1->gray[ldxc] = shaar * Im->gray[dy1 * dx + c];
    }

    /*--- Convolution with high-pass filter ---*/

    if (haary == 0)
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, &HIGH, edge, &PROLONG, ri1, NULL);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, &HIGH, &EDGE_HAAR, &PROLONG, haar_ri, NULL);

    ldxc = c;
    for (l=0; l<dy2; l++) {
      Im2->gray[ldxc] = Tabout->values[l];
      ldxc += dx;
    }
  }

  mw_delete_fsignal(Tabin);
  mw_delete_fsignal(Tabout);

}




static void
WAVEL(Wtrans2d wtrans, int J, int *haar, Fsignal ri1, Fsignal ri2, int *edge)

	/*----- Computes the wavelet decomposition of S -----*/

                   	    /* Wavelet transform */
              		    /* Level of decomposition */
                            /* Continue decomposition with Haar wavelet
			     * until haar level */
                     	    /* Impulse responses of the low-pass filters */
                            /* Type of edge processing (see `Edge`
			     * in biowave2) */

{
  long       c, l;		/* Indices for line and column in resume 
			       	 * or detail */
  long	     ldx, lK2;	       	/* Index of first element of line `l`  
			       	 * in wtrans->image[J-1][0] and 
			       	 * and wtrans->image[J][] */
  long	     K2, L2;   		/* Size of the output (low-pass) */
  long       dx1, dx2;          /* Size of the input and output of sconvolve */
  long	     dx, dy;   		/* Size of input (image or resume 
			       	 * at level J-1) */
  Fsignal    Tabin;	       	/* One line of wtrans->image[J-1][0], 
			       	 * input for `sconvolve` */
  Fsignal    Tabout;	       	/* Output for module `sconvolve`, 
			       	 * wavelet transform of `Tabin` */
  Fimage     Tab;	       	/* Buffer for sum's separation */
  int        haarx;	       	/* Flag for use of Haar filter */


  dx = dx1 = wtrans->images[J-1][0]->ncol;
  dy = wtrans->images[J-1][0]->nrow;
  K2 = dx2 = dx >> 1;
  L2 = dy >> 1;
  if (dx %2 == 1) {
    K2++;
    dx1--;
  }

  haarx = 0;
  if (haar && ((dx2 < ri1->size) || (dx2 < ri2->size)))
    haarx = 1;

 /*--- Initialization of Tabin and Tab ---*/

  Tabin = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabin, dx1) == NULL)
    mwerror(FATAL, 1, "Allocation for line's buffer refused!\n");

  Tabout = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabout, dx2) == NULL)
    mwerror(FATAL, 1, "Allocation for line's buffer refused!\n");

  Tab = mw_new_fimage();
  if (mw_alloc_fimage(Tab, dy, K2) == NULL)
    mwerror(FATAL, 1, "Allocation for sum's separation buffer refused!\n");

  /*--- Conmputation of S and D1 ---*/

  /*--- Convolution of rows with low-pass filter ---*/

  ldx = lK2 = 0;
  for (l = 0; l < dy; l++) {
    for(c = 0; c < dx1; c++)
      Tabin->values[c] = wtrans->images[J-1][0]->gray[ldx+c];

    if (haarx == 0)
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, NULL, edge, &PROLONG, ri2, NULL);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, NULL, &EDGE_HAAR, &PROLONG, haar_ri, NULL);

    for (c = 0; c < dx2; c++)
      Tab->gray[lK2+c] = Tabout->values[c];

    /*--- Copy last point if number of columns is odd ---*/

    if (K2 > dx2)
    {
     if (haarx == 0)
       Tab->gray[lK2+c] = s2 * wtrans->images[J-1][0]->gray[ldx+dx1];
     else
       Tab->gray[lK2+c] = shaar * wtrans->images[J-1][0]->gray[ldx+dx1];
    }
    ldx += dx;
    lK2 += K2;
  }

  /*--- Convolution of column with low and high-pass filters ---*/

  COLUMN_WAVEL(Tab, wtrans->images[J][0], wtrans->images[J][1], J, haar, edge, ri1, ri2);



  /*--- Conmputation of D2 and D3 ---*/

  if (K2 > dx2) {
    Tab = mw_change_fimage(Tab, dy, dx2);
    if (Tab == NULL)
      mwerror(FATAL, 1, "Allocation for sum's separation buffer refused!\n");
  }

  /*--- Convolution of rows with high-pass filter ---*/

  ldx = lK2 = 0;
  for (l = 0; l < dy; l++) {
    for(c = 0; c < dx1; c++)
      Tabin->values[c] = wtrans->images[J-1][0]->gray[ldx+c];

    if (haarx == 0)
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, &HIGH, edge, &PROLONG, ri1, NULL);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, &HIGH, &EDGE_HAAR, &PROLONG, haar_ri, NULL);

    for (c=0; c<dx2; c++)
      Tab->gray[lK2+c] = Tabout->values[c];

    ldx += dx;
    lK2 += dx2;
  }


  /*--- Convolution of columns with low and high-pass filters ---*/

  COLUMN_WAVEL(Tab, wtrans->images[J][2], wtrans->images[J][3], J, haar, edge, ri1, ri2);

  mw_delete_fimage(Tab);
  mw_delete_fsignal(Tabin);
  mw_delete_fsignal(Tabout);  

}




void
biowave2(int *NumRec, int *Haar, int *Edge, int *FilterNorm, Fimage Image, Wtrans2d Output, Fsignal Ri1, Fsignal Ri2)

  /*--- Computes the biorthogonal wavelet transform of image `Image` ---*/

                   	      /* Number de recursion (-j) */
                              /* Continue decomposition with Haar wavelet
			       * until haar level */

                 	      /* Equal 0 (default) if extension with 0 */
			      /* 1 if periodization */
			      /* 2 if reflexion */
                       	      /* Equal 0 if no normalisation of filter's tap
			       *       1 if normalisation of the sum 
			       *       2 if normalistion of the square sum */
                  	      /* Input image */
                   	      /* Wavelet transform of the image `Image` */
                     	      /* Impulse responses of the low pass filters */

{
  int         J;			/* Current level of decomposition */

  /*--- Detection of errors in command line ---*/

  COMLINE_ERR(Ri1, Ri2, *Edge, NumRec, Haar, Image->nrow, Image->ncol);

    /*--- Memory allocation for Haar filter ---*/

  if (Haar) {
    haar_ri = mw_new_fsignal();
    if (mw_alloc_fsignal(haar_ri, 2) == NULL)
      mwerror(FATAL, 1, "Not enough memory for Haar impulse response buffer.\n");
  }

  /*--- Normalisation of filter banks ---*/

  NORM_FIL(Ri1, Ri2, *FilterNorm, Haar);

  /*--- Memory allocation for wavelet transform Output ---*/

  if(mw_alloc_biortho_wtrans2d(Output, *NumRec, Image->nrow, Image->ncol)==NULL)
    mwerror(FATAL, 1, "Memory allocation refused for `WavTrans`!\n");
  Output->images[0][0] = Image;

  /*--- Write commentary for Output ---*/

  COMMENT(Output, Image, *Edge, FilterNorm, Ri1, Ri2);

  /*--- Wavelet decomposition ---*/

  for (J = 1; J <= *NumRec; J++)
    WAVEL(Output, J, Haar, Ri1, Ri2, Edge);

  if (Haar) {
    if (*Haar > *NumRec)
      HAAR_WAVEL2(Output, *NumRec, *Haar, *FilterNorm);
    mw_delete_fsignal(haar_ri);
  }

}











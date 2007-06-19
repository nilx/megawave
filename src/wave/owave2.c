/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {owave2};
version = {"1.4"};
author = {"Jean-Pierre D'Ales"};
function = {"Computes the orthogonal wavelet transform of an image"};
usage = {
 'r':[NLevel=1]->NumRec [1,20]         "Number of levels", 
 'h':HaarNLevel->Haar "Continue decomposition with Haar filter until level HaarNLevel",
 'e':[EdgeMode=3]->Edge [0,3]          "Edge processing mode",
 'p':[PrecondMode=0]->Precond [0,2]    "Edge preconditionning mode",
 'i'->Inverse                          "Invertible transform", 
 'n':[FilterNorm=2]->FilterNorm [0,2]  "Filter taps normalization. 0: no normalization, 1: sum equal to 1.0, 2: squares sum equal to 1.0", 
 Image->Image         "Input image (fimage)", 
 WavTrans<-Output     "Output wavelet transform of Image (wtrans2d)", 
 ImpulseResponse->Ri  "Impulse response of inner filters (fsignal)", 
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
#include <math.h>
#include  "mw.h"

/*--- Megawave2 modules definition ---*/

extern void sconvolve();
extern void precond2d();

/*--- Constants ---*/

static int DECIM=2;	       	/* Decimation's constant */
static int INTERPOL=1;		/* Interpolation rate */
static int REFLIR=1;		/* Indicator for reflexion of filter's ir */
static int HIGH=1;		/* Index for high-pass filtering */
static int INVERSE=1;		/* Index for inverse preconditionning */
static int EDGE_HAAR=0;         /* Index for edge processing mode 
				 * with Haar filter */
static Fsignal      haar_ri;    /* Impulse response for Haar filter */
static double	    shaar;      /* Sums of coefficients of haar filter */
static double	    s;     	/* Sums of coefficients of `ri` */


static void
COMMENT(result, image, edge, precond, filternorm, ri)

	/*--- Fill comment and other fields for result ---*/

    Wtrans2d    result;		/* Wavelet transform of the image `Image` */
    Fimage      image;		/* Input image */
    int 	edge;		/* Type of edge processing 
				 * (see `Edge` in `owave2`) */
    int		precond;	/* Type of edge preconditionning
				 * (see `Precond` in `owave2`) */
    int	        filternorm;	/* Type of normalisation : see 'owave2' */
    Fsignal	ri;		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */

{
    result->edges = edge;
    result->type = mw_orthogonal;
    result->nfilter = 1;
    strcpy(result->filter_name[0], ri->name);
    strcpy(result->cmt, "Wav. Trans. of ");
    strcat(result->cmt, image->name);
    if(precond == 1)
	strcat(result->cmt, " with precond.");
    if(precond == 2)
	strcat(result->cmt, "/unprecond.");
    if(filternorm >= 1)
	strcat(result->cmt, " filters coef. norm.");

}





static void
COMLINE_ERR(ri, edge_ri, edge, inverse, precond, numrec, haar, dy, dx)

	/*--- Detects errors and contradiction in command line ---*/

    Fsignal	ri;		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
    Fimage	edge_ri;	/* Impulse responses of the edge filters
			 * (computation of wavelet coefficients near edges) */
    int 	edge;		/* Type of edge processing 
				 * (see `Edge` in `owave2`) */
    int	       *inverse;	/* Performs invertible transform for edge
				 * processing mode 0 and 2 */
    int		precond;	/* Type of edge preconditionning
				 * (see `Precond` in `owave2`) */
    int        *numrec;		/* Number of levels for decomposition */
    int        *haar;           /* Continue decomposition with Haar wavelet
				 * until ultimate level */
    long	dx, dy;		/* Size of the image */

{
	/*--- Edge processing selection ---*/

    if((edge_ri == NULL) && (edge == 3))
	mwerror(USAGE, 1, "Edge filters i.r. are needed!\n");

    if(((edge == 1) || (edge == 3)) && inverse)
	mwerror(WARNING, 0, "Edge processing mode is already invertible!\n");

	/*--- Preconditionning and special edge processing ---*/

    if((edge < 3) && (precond >= 1))
	mwerror(USAGE, 1, "Preconditionning only with special edge processing\n");

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
    if (edge >= 1)
      if((dx>>(*numrec) < ri->size) || (dy>>(*numrec) < ri->size))
	mwerror(FATAL, 1, "The size of image must be greater than N where N is the size of the i.r.!\n");

	/*--- Inner and edge filters compatibility ---*/

    if((edge == 3) && ((edge_ri->nrow != 4 * ri->size) || (edge_ri->ncol != 3 * ri->size / 2 - 1)))
	mwerror(FATAL, 1, "Inner and edge filters are not compatible!\n");

}





static void
NORM_FIL(ri, edge_ri, filternorm, edge, haar)

	/*--- Normalisation of the coefficients of the filter impulse 
	 *--- responses ---*/

    Fsignal	ri;		/* Impulse response of the low-pass filter 
			 * (computation of the inner wavelet coefficients) */
    Fimage	edge_ri;	/* Impulse responses of the edge filters
			 * (computation of wavelet coefficients near edges) */
    int	        filternorm;	/* Type of normalisation : see 'owave2' */
    int 	edge;		/* Type of edge processing 
				 * (see `Edge` in owave2) */
    int        *haar;           /* Continue decomposition with Haar wavelet
				 * until haar level */


{
  double	s2;		/* Sum square root of `ri`'s coefficients */
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
      ri->values[i] /= s;
    if (edge == 3) {
      for (i = 0; i < edge_ri->nrow / 2; i++)
	for (j = 0; j < edge_ri->ncol; j++)
	  edge_ri->gray[edge_ri->ncol * i + j] /= s;
    }
  }

  if (haar) {
    if (filternorm == 1) {
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

  s = 0.0;
  for (i = 0; i < ri->size; i++)
    s += ri->values[i];

  if (s < 0.00001)
    mwerror(WARNING, 0, "Sum of ri coefficient almost equal to 0!\n");
}




static void
HAAR_WAVEL2(wtrans, numrec, haar, filternorm)

Wtrans2d    wtrans;		/* Wavelet transform */
int         numrec;             /* Level of average */
int         haar;               /* Continue decomposition with Haar wavelet
				 * until ultimate level */
int	    filternorm;	        /* Type of normalisation : see 'owave2' */

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
  if (filternorm == 1)
    cnorm = 0.5;
  else
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
COLUMN_WAVEL(Im, Im1, Im2, J, haar, edge, prolong, int_ri, edge_ri)

	/*--- Computes the 1-D wavelet transform of each column  ---*
	 *--- in image "Tab", puts the result in "Im1" and "Im2" ---*/

Fimage	Im;		     /* Input (wavelet transform along the lines
			      * of image or resume at level J-1) */
Fimage      Im1, Im2;	     /* Low and high-pass filtered sub-images */
int         J;	             /* Level of decomposition */
int        *haar;            /* Continue decomposition with Haar wavelet
			      * until haar level */
int        *edge;	     /* Type of edge processing (see `Edge`
			      * in owave2) */
int	    prolong;         /* Indicates signal extension 
			      * for invertibility */
Fsignal     int_ri;	     /* Impulse response of the low-pass filter 
			  * (computation of the inner wavelet coefficients) */
Fimage      edge_ri;	     /* Impulse responses of filters for special 
			      * edge processing (including preconditionning 
			      * matrices */

{
  long      c, l;	       	/* Variables for line and column in resume 
			       	 * or detail */
  long	    ldxc;	       	/* Index of element at line `l` and column `c` 
			       	 * in Im, Im1, or Im2 */
  long	    K2, L2;	       	/* Size of the output (resume and detail 
			       	 * at level J) */
  int       dy1, dy2;           /* Size of the input and output of sconvolve */
  long	    dx, dy;	       	/* Size of input (wavelet transform along the 
			       	 * lines of image or resume at level J-1) */
  Fsignal   Tabin;	       	/* One column of Im, input for `convo_sig` */
  Fsignal   Tabout;	       	/* Output for module `sconvolve`, 
			       	 * wavelet transform of `Tabin` */
  int       risize;	       	/* Size of 'int_ri' */
  int       haary;	       	/* Flag for use of Haar filter */

  risize = int_ri->size;
  dx = Im->ncol;
  dy = dy1 = Im->nrow;

  if (prolong == 0) {
    K2 = dx;
    L2 = dy2 = dy >> 1;
    if (dy % 2 == 1) {
      L2++;
      dy1--;
    }

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

  } else
    {
      K2 = Im->lastcol;

      L2 = (dy + risize) / 2 - (1 - (dy + risize) / 2) * (long) fabs((double) ((risize + (long) int_ri->shift - 1) %2));
      Im1 = mw_change_fimage(Im1, L2, K2);

      L2 = (dy + risize) / 2 - (1 - (dy + risize) / 2) * (long) fabs((double) (((long) int_ri->shift - 1) %2));
      Im2 = mw_change_fimage(Im2, L2, K2);
      dy2 = L2;
    }

  haary = 0;
  if (haar && (dy >> 1 < int_ri->size))
    haary = 1;

  /*--- Initialization of Tabin and Tabout ---*/

  Tabin = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabin, dy1) == NULL)
    mwerror(FATAL, 1, "Allocation of line buffer refused!\n");

  Tabout = mw_new_fsignal();
  if (mw_alloc_fsignal(Tabout, dy2) == NULL)
    mwerror(FATAL, 1, "Allocation of line buffer refused!\n");

  for (c = 0; c < K2; c++) {
    ldxc = c;
    for (l=0; l<dy1; l++) {
      Tabin->values[l] = Im->gray[ldxc];
      ldxc += dx;
    }

    /*--- Convolution with low-pass filter ---*/

    if (haary == 0)
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, NULL, edge, &prolong, int_ri, edge_ri);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, NULL, &EDGE_HAAR, &prolong, haar_ri, NULL);

    /*--- Copy result in Im1 buffer --- */

    ldxc = c;
    for (l=0; l<dy2; l++) {
      Im1->gray[ldxc] = Tabout->values[l];
      ldxc += dx;
    }

    /*--- Copy last point if number of lines is odd ---*/

    if (prolong == 1) {
      Im1->firstrow = Tabout->firstp;
      Im1->lastrow = Tabout->lastp;
    } else
      if (L2 > dy2)
	if (haary == 0)
	  Im1->gray[ldxc] = s * Im->gray[dy1 * dx + c];
	else
	  Im1->gray[ldxc] = shaar * Im->gray[dy1 * dx + c];
      


    /*--- Convolution with high-pass filter ---*/

    if (haary == 0)
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, &HIGH, edge, &prolong, int_ri, edge_ri);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, &HIGH, &EDGE_HAAR, &prolong, haar_ri, NULL);

    ldxc = c;
    for (l=0; l<dy2; l++) {
      Im2->gray[ldxc] = Tabout->values[l];
      ldxc += K2;
    }
  }

  if (prolong == 1) {
      Im2->firstrow = Tabout->firstp;
      Im2->lastrow = Tabout->lastp;
    }

  mw_delete_fsignal(Tabin);
  mw_delete_fsignal(Tabout);

}




static void
WAVEL(wtrans, J, int_ri, edge_ri, edge, inverse, haar)

	/*----- Computes the wavelet decomposition of S -----*/

Wtrans2d    wtrans;	      /* Wavelet transform */
int         J;		      /* Level of decomposition */
Fsignal     int_ri;	      /* Impulse response of the low-pass filter 
			  * (computation of the inner wavelet coefficients) */
Fimage      edge_ri;	      /* Impulse responses of filters for special 
			       * edge processing */
int        *edge;	      /* Type of edge processing (see `Edge`
			       * in owave2) */
int	   *inverse;	      /* Performs invertible transform for edge
			       * processing mode 0 and 2 */
int        *haar;             /* Continue decomposition with Haar wavelet
			       * until haar level */

{
  int	     prolong;	       	/* Indicates signal extension 
			       	 * for invertibility */
  long       c, l;	       	/* Indices for line and column in resume 
			       	 * or detail */
  long	     ldx, lK2;	       	/* Index of first element of line `l`  
			       	 * in wtrans->image[J-1][0] and 
			       	 * and wtrans->image[J][] */
  long	     K2, L2;	       	/* Size of the output (resume and detail 
			       	 * at level J) */
  long       dx1, dx2;          /* Size of the input and output of sconvolve */
  long	     dx, dy;	       	/* Size of input (image or resume 
			       	 * at level J-1) */
  Fsignal    Tabin;	       	/* One line of wtrans->image[J-1][0], 
			       	 * input for `sconvolve` */
  Fsignal    Tabout;	       	/* Output for module `sconvolve`, 
			       	 * wavelet transform of `Tabin` */
  Fimage     Tab;	       	/* Buffer for sum's separation */
  int        haarx;	       	/* Flag for use of Haar filter */

  dx = dx1 = wtrans->images[J-1][0]->ncol;
  dy = wtrans->images[J-1][0]->nrow;

  if(!inverse || (*edge == 1) || (*edge == 3)) {
    prolong = 0;
    K2 = dx2 = dx >> 1;
    L2 = dy >> 1;
    if (dx %2 == 1) {
      K2++;
      dx1--;
    }
  } else
    {
      prolong = 1;
      K2 = dx2 = (dx + int_ri->size) >> 1;
      L2 = (dy + int_ri->size) >> 1;
    }

  haarx = 0;
  if (haar && (dx >> 1 < int_ri->size))
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
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, NULL, edge, &prolong, int_ri, edge_ri);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, NULL, &EDGE_HAAR, &prolong, haar_ri, NULL);

    for (c = 0; c < dx2; c++)
      Tab->gray[lK2+c] = Tabout->values[c];

    /*--- Copy last point if number of columns is odd ---*/

    if ((K2 > dx2) && (prolong == 0))
     if (haarx == 0)
       Tab->gray[lK2+c] = s * wtrans->images[J-1][0]->gray[ldx+dx1];
     else
       Tab->gray[lK2+c] = shaar * wtrans->images[J-1][0]->gray[ldx+dx1];

    ldx += dx;
    lK2 += K2;
  }

  /*--- Convolution of column with low and high-pass filters ---*/

  Tab->lastcol = Tabout->size;

  COLUMN_WAVEL(Tab, wtrans->images[J][0], wtrans->images[J][1], J, haar, edge, prolong, int_ri, edge_ri);

  if (prolong == 1) {
    wtrans->images[J][0]->firstcol = wtrans->images[J][1]->firstcol = Tabout->firstp;
    wtrans->images[J][0]->lastcol = wtrans->images[J][1]->lastcol = Tabout->lastp;
  } else

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
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, &HIGH, edge, &prolong,  int_ri, edge_ri);
    else
      sconvolve(Tabin, Tabout, &DECIM, &INTERPOL, &REFLIR, &HIGH, &EDGE_HAAR, &prolong,  haar_ri, NULL);

    for(c = 0; c < dx2; c++)
      Tab->gray[lK2+c] = Tabout->values[c];

    ldx += dx;
    lK2 += dx2;
  }


  /*--- Convolution of columns with low and high-pass filters ---*/

  Tab->lastcol = Tabout->size;

  COLUMN_WAVEL(Tab, wtrans->images[J][2], wtrans->images[J][3], J, haar, edge, prolong, int_ri, edge_ri);

  if (prolong == 1) {
    wtrans->images[J][2]->firstcol = wtrans->images[J][3]->firstcol = Tabout->firstp;
    wtrans->images[J][2]->lastcol = wtrans->images[J][3]->lastcol = Tabout->lastp;
  }

  mw_delete_fimage(Tab);
  mw_delete_fsignal(Tabin);
  mw_delete_fsignal(Tabout);  

}







void
owave2(NumRec, Haar, Edge, Precond, Inverse, FilterNorm, Image, Output, Ri, Edge_Ri)

	/*--- Computes the orthogonal wavelet transform of image `Image` ---*/

int        *NumRec;		/* Number of recursion (-j) */
int        *Haar;               /* Continue decomposition with Haar wavelet
				 * until Haar level */
int        *Edge;		/* Equal 0 if extension with 0 */
				/* 1 if periodization */
				/* 2 if reflexion */
				/* 3 (default) if special treatment of edges */
int        *Precond;		/* Equal 0 (default) if no
				 * (un)preconditionning 
				 * 1 if preconditionning only
				 * 2 if preconditionning and unpreconditionning
				 */
int	   *Inverse;		/* Performs invertible transform for edge
				 * processing mode 0 and 2 */
int        *FilterNorm;	        /* Type of normalisation :
			         * equal 0 if no normalisation 
			         *       1 if normalisation of the sum of
			         *         `ri`'s coefficients to 1.0 
			         *       2 if normalisation of the squares' sum 
			         *         `ri`'s coefficients to 1.0 */
Fimage      Image;		/* Input image */
Wtrans2d    Output;		/* Wavelet transform of the image `Image` */
Fsignal     Ri;			/* Impulse response of the low pass filter */
Fimage	    Edge_Ri;		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */

{
  int         J;			/* Current level of decomposition */

  /*--- Detection of errors in command line ---*/

  COMLINE_ERR(Ri, Edge_Ri, *Edge, Inverse, *Precond, NumRec, Haar, Image->nrow, Image->ncol);

  /*--- Memory allocation for Haar filter ---*/

  if (Haar) {
    haar_ri = mw_new_fsignal();
    if (mw_alloc_fsignal(haar_ri, 2) == NULL)
      mwerror(FATAL, 1, "Not enough memory for Haar impulse response buffer.\n");
  }

  /*--- Normalisation of filter banks ---*/

  NORM_FIL(Ri, Edge_Ri, *FilterNorm, *Edge, Haar);

  /*--- Memory allocation for wavelet transform Output ---*/

  if(mw_alloc_ortho_wtrans2d(Output, *NumRec, Image->nrow, Image->ncol)==NULL)
    mwerror(FATAL, 1, "Allocation of buffer for wavelet transform refused!\n");

  /*--- Write commentary for Output ---*/

  COMMENT(Output, Image, *Edge, *Precond, *FilterNorm, Ri);

  /*--- Preconditionning of image's edges (if selected) ---*/

  if (*Precond >= 1)
    precond2d(NULL, Image, Image, Edge_Ri);

  Output->images[0][0] = Image;

  /*--- Wavelet decomposition ---*/

  for (J = 1; J <= *NumRec; J++)
    WAVEL(Output, J, Ri, Edge_Ri, Edge, Inverse, Haar);

  if (Haar) {
    if (*Haar > *NumRec)
      HAAR_WAVEL2(Output, *NumRec, *Haar, *FilterNorm);
    mw_delete_fsignal(haar_ri);
  }

  /*--- Unpreconditionning of resume's edges (if selected) ---*/

  if (*Precond == 2)
    precond2d(&INVERSE, Output->images[*NumRec][0], Output->images[*NumRec][0],  Edge_Ri);
    
}

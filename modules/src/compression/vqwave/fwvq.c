/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {fwvq};
version = {"2.3"};
author = {"Jean-Pierre D'Ales"};
function = {"Compress an image with vector quantization of wavelet transform"};
usage = {
 'r':NLevel->NumRec [1,15]    "Number of level for wavelet tranform", 
 'e':EdgeIR->Edge_Ri          "Impulse reponses of edge and preconditionning filters for orthogonal transform (fimage)",
 'b':ImpulseResponse2->Ri2    "Impulse response of filter 2 for biorthogonal transform (fsignal)",
 'n':FilterNorm->FilterNorm [0,2]  "Normalization mode for filter bank", 
 'w':WeightFac->WeightFac          "Scaling factor for wavelet coefficients", 
 's':ScalQuant->NumRecScal [0,15]  "Use uniform scalar quantization for the resume at level NLevelWav if ScalQuant = NLevelWav or all subimages at levels larger than SalQuant otherwise", 
 'u':[UnifQuantStep=0]->NStep      "Use UnifQuantStep levels to uniformly scalar quantize the resume or subimages", 
 'm':[MultiCB=2]->MultiCB [1,2]    "1: approximative memory allocation procedure, 2: exhaustive memory allocation procedure",
 'x':CodeBook2->CodeBook2          "Sequence of codebooks for second class vectors (fimage)",
 'y':CodeBook3->CodeBook3          "Sequence of codebooks for third class vectors (fimage)",
 'A':ResCodeBook1->ResCodeBook1    "Sequence of codebooks for residu quantization after quantization with CodeBook1 (fimage)",
 'B':ResCodeBook2->ResCodeBook2    "Sequence of codebooks for residu quantization after quantization with CodeBook2 (fimage)",
 'C':ResCodeBook3->ResCodeBook3    "Sequence of codebooks for residu quantization after quantization with CodeBook3 (fimage)",
 'D':ResResCodeBook1->ResResCodeBook1   "Sequence of codebooks for residu quantization after quantization with CodeBook1 and ResCodeBook1 (fimage)",
 'E':ResResCodeBook2->ResResCodeBook2   "Sequence of codebooks for residu quantization after quantization with CodeBook2 and ResCodeBook2 (fimage)",
 'd'->DistRate             "Computes distorsion-rate function", 
 'R':TargetRate->TargRate  "Target Rate", 
 'o':Compress<-Output      "Compressed Image (cimage)", 
 Image->Image              "Input image (fimage)", 
 CodeBook1->CodeBook1      "First sequence of codebooks (fimage)", 
 ImpulseResponse->Ri       "Impulse response of inner filters (fsignal)", 
	{
	   Qimage<-QImage  "Quantized image (fimage)"
	}
	};
 */
/*----------------------------------------------------------------------
 v2.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fvq(), fscalq(), owave2(), iowave2(),
			 * biowave2(), ibiowave2(), fmse() */

static int count_cb(Fimage codebook);

/*--- Constants ---*/

#define MAX_SIZEO 32383          /* Maximum number of line and column for 
				  * output cimage (compressed file) */
#define MAX_LEVEL 6              /* Maximum number of level in wav. transf. */ 
#define NORIENT 4                /* Number of orientation in wav. transf. */
#define MAX_CB 50                /* Maximum number of codebooks for 
				  * a given sub-image and a given class */
#define MAX_WADAP 3              /* Maximum number of classes */
#define NUMSTEP_SCALQ 12         /* log of maximum number of steps for scalar 
				  * quantization */
#define NBIT_SIZEIM 15           /* Number of bits dedicated to encode 
				  * the dimensions of image */
#define NBIT_NLEVEL 4            /* Number of bits dedicated to encode the 
				  * number of levels in wavelet transf. */

/*--- Global variables ---*/

typedef float bufmse[MAX_WADAP][MAX_LEVEL][NORIENT][MAX_CB];
typedef float bufrl[MAX_WADAP][MAX_LEVEL][NORIENT];
typedef int   bufind[MAX_WADAP][MAX_LEVEL][NORIENT];

bufmse        bmse;            /* Buffer for m.s.e. for subimages 
				* and different codebooks */
bufmse        brate;           /* Buffer for rate for subimages 
				* and different codebooks */
bufrl         brl;             /* Buffer for classes map rates for 
				* subimages and different codebooks */
bufind        numcb;           /* Buffer for number of codebooks 
				* for each class and each subimage */
bufrl         bfacmse;         /* Buffer of multiplicative constants 
                                * for mse in approximative memory allocation */
static int    nadapcb[MAX_LEVEL][NORIENT];  /* Number of adapted classes 
				* for each level and orientation */
static float  targrate_dr[20]; /* Target bit rate for distortion-rate curve */
static int    count_dr;        /* Index of a point in distortion-rate curve */
static int    max_count_dr;    /* Number of points in distortion-rate curve */
static float  rateartot;       /* Rate with arithmetic coding 
				* for global image */
static int    ncodewords;      /* Number of codewords stored in compress */
static int    bits_to_go;      /* Number of free bits in the currently 
				* encoded codeword */
static int    buffer;          /* Bits to encode in the currently 
				* encoded codeword */
static unsigned char  *ptrc;   /* Pointer to compress->gray for next 
				* codewords to encode */
static Fsignal ORI1, ORI2;     /* Non normalized i.r. of wavelet filters */
static Fimage OEDGE_RI;        /* Non normalized edge filters */




static void
FCB2WCB(fcb, wcb)

  /*--- Translate a codebook sequence from the fimage ---*/
               /*--- to the wtrans2d format ---*/

Fimage        fcb;         /* Codebook in the fimage format */
Wtrans2d      wcb;         /* Codebook in the wtrans2d format */

{
  int           nlevel;
  int           j;	        /* Scale index in wav. transf. */
  int           i;	      	/* Orientation index */
  long          r, c;           /* Indices for row and columns in codebooks */
  long          nrow, ncol;
  long          nrowcb, ncolcb; /* Number of rows and columns in sub-image 
				 * codebook in wcb */
  int           width, height;  /* Height and width of vectors */
  long          incrc;

  /*--- Memory allocation and initialisation of wcb ---*/

  nrow = ncol = 2;
  nlevel = (int) fcb->gray[0];
  for (j = 1; j < nlevel; j++) {
    nrow *= 2;
    ncol *= 2;
  }
  
  if (mw_alloc_ortho_wtrans2d(wcb, nlevel, nrow, ncol) == NULL)
    mwerror(FATAL, 1, "Allocation of wavelet transform for codebook refused!\n");  

  /*--- Copy fcb to wcb ---*/

  nrow = fcb->nrow;
  ncol = fcb->ncol;
  incrc = 0;
  for (j = 1; j <= wcb->nlevel; j++)
    for (i = 0; i <= 3; i++) {
      nrowcb = (int) fcb->gray[ncol + (j - 1) * 4 + i];
      if (((float) nrowcb != fcb->gray[ncol + (j - 1) * 4 + i]) || (nrowcb < 0))
	mwerror(FATAL, 2, "Bad value for size of codebook for sub-image %d/%d.\n", j, i);
      ncolcb = (int) fcb->gray[2 * ncol + (j - 1) * 4 + i];
      if (((float) ncolcb != fcb->gray[2 * ncol + (j - 1) * 4 + i]) || (ncolcb < 0))
	mwerror(FATAL, 2, "Bad value for dimension of vector for sub-image %d/%d.\n", j, i);
      
      if ((nrowcb > 4) && (ncolcb > 0)) {
	height = (int) fcb->gray[(nrowcb + 1) * ncol + incrc];
	if (((float) height != fcb->gray[(nrowcb + 1) * ncol + incrc]) || (height <= 0))
	  mwerror(FATAL, 2, "Bad value for height of vector for sub-image %d/%d.\n", j, i);
	if (ncolcb % height != 0)
	  mwerror(FATAL, 2, "Dimension and height of vector for sub-image %d/%d are incompatible!\n", j, i);
	width = ncolcb / height;

	if (mw_change_fimage(wcb->images[j][i], nrowcb, ncolcb) == NULL)
	  mwerror(FATAL, 1, "Not enough memory for wavelet transform buffer for codebook.\n");
      
	wcb->images[j][i]->firstcol = width;
	wcb->images[j][i]->firstrow = height;
	for (r = 0; r < nrowcb; r++)
	  for (c = 0; c < ncolcb; c++)
	    wcb->images[j][i]->gray[r * ncolcb + c] = fcb->gray[(r + 3) * ncol + c + incrc]; 
	incrc += ncolcb;
      } else 
	{
	  if (mw_change_fimage(wcb->images[j][i], 1, 1) == NULL)
	    mwerror(FATAL, 1, "Not enough memory for wavelet transform buffer for codebook.\n");
	  
	  wcb->images[j][i]->gray[0] = 0.0;; 
	}
    }

}





static void
INIT_TARGNBIT_DR(wtrans)

Wtrans2d      wtrans;

{

  targrate_dr[0] = 0.008;
  targrate_dr[1] = 0.016;
  targrate_dr[2] = 0.032;
  targrate_dr[3] = 0.064;
  targrate_dr[4] = 0.08;
  targrate_dr[5] = 0.1;
  targrate_dr[6] = 0.13333;
  targrate_dr[7] = 0.16;
  targrate_dr[8] = 0.2;
  targrate_dr[9] = 0.26667;
  targrate_dr[10] = 0.4;
  targrate_dr[11] = 0.53333;
  targrate_dr[12] = 0.8;
  targrate_dr[13] = 1.6;

  max_count_dr = 13;
  count_dr = 0;

}


static void
INIT_RI(ri1, ri2, edge_ri)

Fsignal ri1, ri2;
Fimage  edge_ri;

{
  int i, j;

  ORI1 = mw_new_fsignal();
  if (mw_alloc_fsignal(ORI1, ri1->size) == NULL)
    mwerror(FATAL, 1, "Not enough memory for ri1!\n");

  for (i=0; i<ri1->size; i++)
    ORI1->values[i] = ri1->values[i];

  if (ri2) {
    ORI2 = mw_new_fsignal();
    if (mw_alloc_fsignal(ORI2, ri2->size) == NULL)
      mwerror(FATAL, 1, "Not enough memory for ri2!\n");

    for (i=0; i<ri2->size; i++)
      ORI2->values[i] = ri2->values[i];

  }
  
  if (edge_ri) {
    OEDGE_RI = mw_new_fimage();
    if (mw_alloc_fimage(OEDGE_RI, edge_ri->nrow, edge_ri->ncol) == NULL)
      mwerror(FATAL, 1, "Not enough memory for edge_ri!\n");

    for (j = 0; j < edge_ri->nrow; j++)
      for (i = 0; i < edge_ri->ncol; i++)
	OEDGE_RI->gray[j * edge_ri->ncol + i] = edge_ri->gray[j * edge_ri->ncol + i];
  }
}



static void
REFRESH_FILTERS(ri1, ri2, edge_ri)

Fsignal ri1, ri2;
Fimage  edge_ri;

{
  int i, j;


  for (i=0; i<ri1->size; i++)
    ri1->values[i] = ORI1->values[i];

  if (ri2)
    for (i=0; i<ri2->size; i++)
      ri2->values[i] = ORI2->values[i];

  if (edge_ri) 
    for (j = 0; j < edge_ri->nrow; j++)
      for (i = 0; i < edge_ri->ncol; i++)
	edge_ri->gray[j * edge_ri->ncol + i] = OEDGE_RI->gray[j * edge_ri->ncol + i];
}



static float
SPHERE_CONST(dim)

int   dim;

{
  float   r;

  if (dim <= 0)
    mwerror(FATAL, 4, "Negative dimension : %d!\n", dim);

  switch(dim) {

  case 1:
    r = 0.08333;
    break;

  case 2:
    r = 0.07958;
    break;

  case 3:
    r = 0.07697;
    break;

  case 4:
    r = 0.07503;
    break;

  case 5:
    r = 0.07352;
    break;

  case 6:
    r = 0.0723;
    break;

  case 7:
    r = 0.07135;
    break;

  case 8:
    r = 0.07045;
    break;

  case 16:
    r = 0.06657;
    break;
  }

  if (dim > 16)
    r = 0.0574;
  if ((dim < 16) && (dim > 8))
    r = 0.06657 + (0.07045 - 0.06657) * (16.0 - (float) dim) / 8.0;

  return(r);
}



static void 
COMPUTE_CONST(codebook, nlevel, nlevelscal)

Wtrans2d        codebook;
int             nlevel;
int             nlevelscal;

{
  int             j, i, nadap;

  for (j = 1; j <= nlevel; j++) 
    for (i = 0; i <= 3; i++)
      for (nadap = 0; nadap < nadapcb[j][i]; nadap++)
	bfacmse[nadap][j][i] = 12.0 * SPHERE_CONST(1);

  for (j = 1; j <= nlevelscal; j++) 
    for (i = 0; i <= 3; i++)
      if (codebook->images[j][i]) 
	if (codebook->images[j][i]->nrow > 6)
	  for (nadap = 0; nadap < nadapcb[j][i]; nadap++)
	    bfacmse[nadap][j][i] = 12.0 * SPHERE_CONST(codebook->images[j][i]->ncol);

  if (nlevelscal == nlevel)
    bfacmse[0][nlevel][0] = 12.0 * SPHERE_CONST(1);

  for (j = nlevelscal + 1; j <= nlevel; j++) 
    for (i = 0; i <= 3; i++)
      bfacmse[0][j][i] = 12.0 * SPHERE_CONST(1);
}




static void
clear_fimage(image, mse)
     Fimage         image;
     double        *mse;

{
  long              x;
  long              size;

  size = image->nrow * image->ncol;
  *mse = 0.0;
  for (x = 0; x < size; x++) {
    *mse += image->gray[x] * image->gray[x];
    image->gray[x] = 0.0;
  }
  *mse /= (double) size;
}



static void
fvariance(wtrans, j, i, cb1, cb2, cb3, var1, var2, var3, varthres, num1, num2, num3, numthres)

Wtrans2d        wtrans;
int             j, i;
Wtrans2d        cb1, cb2, cb3;
double         *var1, *var2, *var3, *varthres;
long	       *num1, *num2, *num3, *numthres;
    
{
  int             sizeb;
  int             height, width;
  int             rb, cb;
  int             r, c;
  int             nrowb, ncolb;
  int             nrow, ncol;
  float           thres1, thres2, thres3;
  double          mean, mean1, mean2, mean3, meanthres;
  double          energy;
  
  nrow = wtrans->images[j][i]->nrow;
  ncol = wtrans->images[j][i]->ncol;
  width = height = 1;
  sizeb = 1;
  thres1 = thres2 = thres3 = 0.0;
  if (cb1->nlevel >= j)
    if (cb1->images[j][i]->nrow > 4) {
      sizeb = cb1->images[j][i]->ncol;
      height = cb1->images[j][i]->gray[(cb1->images[j][i]->nrow - 2) * sizeb];
      width = sizeb / height;
      thres1 = cb1->images[j][i]->gray[(cb1->images[j][i]->nrow - 3) * sizeb];
      if (cb2) {
	thres2 = cb2->images[j][i]->gray[(cb2->images[j][i]->nrow - 3) * sizeb];
	if (cb3)
	  thres3 = cb3->images[j][i]->gray[(cb3->images[j][i]->nrow - 3) * sizeb];
      }
    }

  nrowb = wtrans->images[j][i]->nrow / height;
  ncolb = wtrans->images[j][i]->ncol / width;

  *var1 = *var2 = *var3 = *varthres = 0.0;
  mean1 = mean2 = mean3 = meanthres = 0.0;
  *num1 = *num2 = *num3 = *numthres = 0;
  for(r = 0; r < nrowb; r++) 
    for (c = 0; c < ncolb; c++) {
      energy = mean = 0.0;
      for (rb = 0; rb < height; rb++)
	for (cb = 0; cb < width; cb++) {
	  energy += wtrans->images[j][i]->gray[(r*height+rb) * ncol + c * width + cb] * wtrans->images[j][i]->gray[(r*height+rb) * ncol + c * width + cb];
	  mean += wtrans->images[j][i]->gray[(r*height+rb) * ncol + c * width + cb];
	}
      if (energy >= thres1) {
	*var1 += energy;
	mean1 += mean;
	(*num1)++;
      } else
	if (thres2 > 0.0)
	  if (energy >= thres2) {
	    *var2 += energy;
	    mean2 += mean;
	    (*num2)++;
	  } else
	    if (thres3 > 0.0)
	      if (energy >= thres3) {
		*var3 += energy;
		mean3 += mean;
		(*num3)++;
	      } else
		{
		  *varthres += energy;
		  meanthres += mean;
		  (*numthres)++;
		}
	    else
	      {
		*var3 += energy;
		mean3 += mean;
		(*num3)++;
	      }
	else
	  {
	    *var2 += energy;
	    mean2 += mean;
	    (*num2)++;
	  }
    }

  if (*num1 > 0) {
    *num1 *= sizeb;
    mean1 /= (double) *num1;
    *var1 /= (double) *num1;
    *var1 -= mean1 * mean1;
  }
  if (*num2 > 0) {
    *num2 *= sizeb;
    mean2 /= (double) *num2;
    *var2 /= (double) *num2;
    *var2 -= mean2 * mean2;
  }
  if (*num3 > 0) {
    *num3 *= sizeb;
    mean3 /= (double) *num3;
    *var3 /= (double) *num3;
    *var3 -= mean3 * mean3;
  }
  if (*numthres > 0) {
    *numthres *= sizeb;
    meanthres /= (double) *numthres;
    *varthres /= (double) *numthres;
    *varthres -= meanthres * meanthres;
  }
}




static double
det_energy(image)

Fimage     image;

{
  long x, size;
  double e;

  e = 0.0;
  size = image->nrow * image->ncol;
  for (x = 0; x < size; x++)
    e += image->gray[x] * image->gray[x];
  e /= (double) size;

  return(e);
}




static float
sqdist(image1, image2)

    Fimage          image1, image2;

{
    int             i;
    int             size;
    float           error, e;

    size = image1->nrow * image1->ncol;
    error = 0.0;
    for (i = 0; i < size; i++)
    {
	e = image1->gray[i] - image2->gray[i];
	error += e * e;
    }

    error /= (float) size;
    return (error);

}


static void
RESIZE_COMPRESS_WTRANS(compress)

Cimage           compress;

{
  int              i;
  int              ncolo, nrowo;
  long             size, ncodewords;
  int              mindif;


  ncolo = 1;
  nrowo = ncodewords = compress->firstcol;
  if (nrowo > MAX_SIZEO) {
    if ((int) sqrt((double) nrowo) + 1 > MAX_SIZEO)
      mwerror(FATAL, 2, "Number of codewords is too large!\n");
    i = 2;
    while (nrowo / i > MAX_SIZEO)
      i++;
    while ((nrowo % i != 0) && (i <= nrowo / i))
      i++;
    if (i <= nrowo / i) {
      nrowo /= i;
      ncolo = i;
    } else
      {
	i = 2;
	mindif = ncodewords;
	while (i <= nrowo / i) {
	  if (nrowo / i <= MAX_SIZEO) 
	    if ((nrowo / i + 1) * i - ncodewords < mindif) {
	      ncolo = i;
	      mindif = (nrowo / i + 1) * i - ncodewords;
	    }
	  i++;
	}
	nrowo = ncodewords / ncolo + 1;
	size = nrowo * ncolo;
	if (ncodewords >= size)
	  mwerror(WARNING, 0, "Something is wrong with output dimensions!\n");
      }
  }


  if (nrowo > ncolo) {
    mindif = ncolo;
    ncolo = nrowo;
    nrowo = mindif;
  }

  compress = mw_change_cimage(compress, nrowo, ncolo);
  compress->cmt[0]='\0';
  
}



static void
REALLOCATE_COMPRESS_WTRANS(compress)

Cimage           compress;

{
  int              i;
  Cimage           bufcomp;
  long             size;

  size = compress->ncol * compress->nrow;
  printf("Reallocation of Compress for wavelet coefficients vector quantization.\n");

  bufcomp = mw_new_cimage();
  bufcomp = mw_change_cimage(bufcomp, compress->nrow,  compress->ncol);
  if (bufcomp == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for reallocation buffer!\n");
  for (i = 0; i < size; i++)
    bufcomp->gray[i] = compress->gray[i];

  compress = mw_change_cimage(compress, compress->nrow * 2,  compress->ncol);
  if (compress == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for reallocated compress!\n");

  for (i = 0; i < size; i++) {
    compress->gray[i] = bufcomp->gray[i];
    compress->gray[i+size] = 0;
  }

  ptrc = compress->gray;
  for (i = 1; i < size; i++) 
    ptrc++;

  mw_delete_cimage(bufcomp);
}



static void
ADD_BIT_TO_COMPRESS_WTRANS(bit, compress)

int              bit;
Cimage           compress;

{
  buffer >>= 1;
  if (bit) 
    buffer += 128;
  bits_to_go -= 1;
  if (bits_to_go == 0) {
    *ptrc = buffer;
    ncodewords += 1;
    if (ncodewords == compress->ncol * compress->nrow)
      REALLOCATE_COMPRESS_WTRANS(compress);
    ptrc++;
    bits_to_go = 8;
    buffer = 0;
  }
}



static void
ENCODE_INT_WTRANS(symb, max, compress)

int            symb;         /* Value of symbol to write */
int            max;          /* Half of maximum value for symbol */
Cimage         compress;     /* Compressed file */

{

  while (max > 0) {
    if (symb >= max) {
      ADD_BIT_TO_COMPRESS_WTRANS(1, compress);
      symb = symb % max;
    } else
      ADD_BIT_TO_COMPRESS_WTRANS(0, compress);
    max /= 2;
  }

}


static void
INIT_ENCODING_WTRANS(nrow, ncol, nlevel, codebook1, nlevelscal, compress)

int          nrow, ncol;         /* Size of original image */
int          nlevel;             /* Number of levels in wavelet transform */
Wtrans2d     codebook1;
int          nlevelscal;
Cimage       compress;           /* Compressed file */

{
  int           i, j;

  if (compress) {

  /*--- Memory allocation for compressed image buffer ---*/

    compress = mw_change_cimage(compress, nrow,  ncol);
    if (compress == NULL)
      mwerror(FATAL, 1, "Memory allocation refused for `Compress`!\n");

    /*--- Init encoding ---*/

    compress->lastrow = 1;
    ptrc = compress->gray;
    bits_to_go = 8;
    buffer = 0;
    ncodewords = 0;

    /*--- Encode size of image ---*/

    ENCODE_INT_WTRANS(nrow, 1 << (NBIT_SIZEIM - 1), compress);
    ENCODE_INT_WTRANS(ncol, 1 << (NBIT_SIZEIM - 1), compress);

    /*--- Encode number of level in wavelet transform ---*/

    ENCODE_INT_WTRANS(nlevel, 1 << (NBIT_NLEVEL - 1), compress);

    /*--- Encode info for scalar / vector quantization ---*/

    for (j = 1; j <= codebook1->nlevel; j++)
      if (j <= nlevel)
	for (i = 1; i <= 3; i++)
	  if ((count_cb(codebook1->images[j][i]) <= 0) || (j > nlevelscal))
	    ENCODE_INT_WTRANS(0, 1, compress);
	  else
	    ENCODE_INT_WTRANS(1, 1, compress);
    for (j = codebook1->nlevel + 1; j <= nlevel; j++)
      for (i = 1; i <= 3; i++)
	ENCODE_INT_WTRANS(0, 1, compress);
    if ((nlevel >= nlevelscal) || (count_cb(codebook1->images[nlevel][0]) <= 0))
      ENCODE_INT_WTRANS(0, 1, compress);
    else
      ENCODE_INT_WTRANS(1, 1, compress);

    if (bits_to_go < 8) {
      ncodewords++;
      *ptrc = buffer >> bits_to_go;
    } else
      bits_to_go = 0;

    compress->firstrow = bits_to_go;
    compress->firstcol = ncodewords;
  } 

}




static void
extract_cb(codebook, n)

Fimage     codebook;
int        n;

{
  int      size, sizei, sizef;
  int      nrow;
  long     xshift;
  long     x, xi, xf;
  int      n1;
  
  sizei = floor(codebook->gray[(codebook->nrow - 5) * codebook->ncol]
		+ .5);
  sizef = floor(codebook->gray[(codebook->nrow - 6) * codebook->ncol]
		+ .5);
	       
  if (n == 0) {
    xi = sizef;
    xf = (sizef + sizei);
  } else
    {
      size = 1;
      while (size < sizei)
	size *= 2;
      if (size == sizei)
	size *= 2;
      xi = sizef + sizei;
      n1 = 1; 
      while ((n1 < n) && (size * 2 < sizef)) {
	xi += size;
	size *= 2;
	n1++;
      }
      if (n1 < n) {
	xi = 0;
	xf = sizef;
      } else
	xf = xi + size;
    }

  nrow = xf - xi + 4;
  xi *= codebook->ncol;
  xf *= codebook->ncol;

  for (x = xi; x < xf; x++)
    codebook->gray[x - xi] = codebook->gray[x];    

  xi = (codebook->nrow - 4) * codebook->ncol;
  xf = codebook->nrow * codebook->ncol;
  xshift = (nrow - codebook->nrow) * codebook->ncol;
  for (x = xi; x < xf; x++)
    codebook->gray[x + xshift] = codebook->gray[x];

  codebook = mw_change_fimage(codebook, nrow, codebook->ncol);
  if (codebook == NULL)
    mwerror(FATAL,1,"Not enough memory for extracted codebook.\n");
}




static int
count_cb(codebook)

Fimage     codebook;

{
  long      size, sizei, sizef;
  int      n;

  n = 0;
  if (codebook) 
    if (codebook->nrow > 4) {
      n = 1;
      if (codebook->nrow > 6) {
	sizei = floor(codebook->gray[(codebook->nrow - 5) *
				    codebook->ncol] + .5);
	sizef = floor(codebook->gray[(codebook->nrow - 6) *
				    codebook->ncol] + .5);
	if (((float) sizei == codebook->gray[(codebook->nrow - 5) * codebook->ncol]) && ((float) sizef == codebook->gray[(codebook->nrow - 6) * codebook->ncol]) && (sizei > 0) && (sizef >= sizef)) {
	  n = 2;
	  size = 1;
	  while (size <= sizei)
	    size *= 2;  
	  while (size < sizef) {
	    size *= 2;
	    n++;
	  }
	}
      }
    }

  return(n);
}




static void
compute_indexcb(indcb, targrate, rateheader, numrec, test_dr, multicb)

bufind    indcb;                /* Index of codebooks to be used */
float     targrate;
float     rateheader;           /* Bit rate to encode header */
int       numrec;
int       test_dr;
int       multicb;

{
  int      q, qmax;
  int      j, i, nadap;
  int      jmax, imax, nadapmax;
  float    effrate, diffrate, diffrate1;
  float    maxdiff, diff;

  indcb[0][numrec][0] = 0;
  for (j = 1; j <= numrec; j++) 
    for (i = 1; i <= 3; i++) 
      for (nadap = 0; nadap < nadapcb[j][i]; nadap++) 
	indcb[nadap][j][i] = 0;


  maxdiff = 1e30;
  effrate = rateheader + brate[0][numrec][0][indcb[0][numrec][0]];
  for (j = 1; j <= numrec; j++) 
    for (i = 1; i <= 3; i++)
      for (nadap = 0; nadap < nadapcb[j][i]; nadap++) 
	effrate += brate[nadap][j][i][indcb[nadap][j][i]];


  while (effrate < targrate) {
    maxdiff = 0.0;
    jmax = 0;
    for (j = 1; j <= numrec; j++) 
      for (i = 1; i <= 3; i++)
	for (nadap = 0; nadap < nadapcb[j][i]; nadap++) {
	  diffrate1 = 0.0;
	  if ((nadap >= 1) && (indcb[nadap][j][i] == 0))
	    if (indcb[nadap - 1][j][i] == 0)
	      diffrate1 = brl[nadap - 1][j][i];
	  for (q = indcb[nadap][j][i] + 1; q < numcb[nadap][j][i]; q++) {
	    diffrate = diffrate1 + brate[nadap][j][i][q] - brate[nadap][j][i][indcb[nadap][j][i]];
	    if (diffrate != 0.0) {
	      diff = (bmse[nadap][j][i][indcb[nadap][j][i]] - bmse[nadap][j][i][q]) / diffrate;
	      if ((diff > maxdiff) && (effrate + diffrate <= targrate)) {
		jmax = j;
		imax = i;
		nadapmax = nadap;
		qmax = q;
		maxdiff = diff;
	      }
	    }
	  }
	}

    for (q = indcb[0][numrec][0] + 1; q < numcb[0][numrec][0]; q++) {
      diffrate = brate[0][numrec][0][q] - brate[0][numrec][0][indcb[0][numrec][0]];
      if (diffrate != 0.0) {
	diff = (bmse[0][numrec][0][indcb[0][numrec][0]] - bmse[0][numrec][0][q]) / diffrate;
	if ((diff > maxdiff) && (effrate + diffrate <= targrate)) {
	  jmax = numrec;
	  imax = 0;
	  nadapmax = 0;
	  qmax = q;
	  maxdiff = diff;
	}
      }
    }

    if (jmax == 0)
      effrate = targrate + 1.0;
    else
      {
	indcb[nadapmax][jmax][imax] = qmax;
	effrate = rateheader;
	for (j = 1; j <= numrec; j++) 
	  for (i = 1; i <= 3; i++)
	    for (nadap = 0; nadap < nadapcb[j][i]; nadap++) {
	      effrate += brate[nadap][j][i][indcb[nadap][j][i]];
	      if ((nadap >= 1) && (indcb[nadap][j][i] > 0))
		if (indcb[nadap - 1][j][i] == 0)
		  effrate += brl[nadap - 1][j][i];	      
	    }
	effrate += brate[0][numrec][0][indcb[0][numrec][0]];
      }
  }    


}



static void
wvq_loc(NumRec, ScaleWeight, NumRecScal, NStep, MultiCB, TargRate, CodeBook2, CodeBook3, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResResCodeBook1, ResResCodeBook2, Compress, Wtrans, CodeBook1, Output)

int            *NumRec;               /* Quantize wavelet transform up 
				       * to level NumRec */
float          *ScaleWeight;          /* Apply a weighting of coefficients 
				       * before quantization */
int            *NumRecScal;           /* Scalar quantize wavelet subimages  
				       * at level NumRecScal and higher */
int            *NStep;                /* Number of steps in case of scalar 
				       * quantization of resume */
int            *MultiCB;              /* Not selected : one codebook per file
					 1: compute approx. dist-rate curves
					 2: compute exact dist-rate curves */
float          *TargRate;             /* Target bit rate */
Wtrans2d        CodeBook2, CodeBook3; /* Sequence of codebooks for adaptive 
				       * quantization */
Wtrans2d        ResCodeBook1, ResCodeBook2, ResCodeBook3; /* Sequence of 
				       * codebooks for residu quantization  
				       * after quantization with CodeBook 
				       * AdapCodeBook2 and AdapCodeBook3 */
Wtrans2d        ResResCodeBook1;      /* Sequence of codebooks for residu 
				       * quantization after quantization 
				       * with CodeBook and ResCodeBook1 */
Wtrans2d        ResResCodeBook2;      /* Sequence of codebooks for residu 
				       * quantization after quantization 
				       * with AdapCodeBook2 and ResCodeBook2 */
Cimage          Compress;	      /* Compressed representation of Wtrans */
Wtrans2d        Wtrans;               /* Input wavelet transform */
Wtrans2d        CodeBook1;            /* First sequence of codebooks for 
				       * for quantization */
Wtrans2d        Output;               /* Quantized wavelet transform (can be 
				       * reconstructed from compress */

{
  Fimage          AdapCB2, AdapCB3;   /* Codebooks for adaptive quantization 
				       * of details */
  Fimage          ResCB1, ResCB2, ResCB3; /* Codebooks for residual 
				       * quantization  of detail */
  Fimage          ResResCB1, ResResCB2; /* Codebooks for residual quantization 
				       * of details (second level) */
  int             NLevelScal;         /* Scalar quantize wavelet subimages  
				       * at level NLevelScal and higher */
  int             J;	              /* Scale index for subimages */
  int             i;                  /* Orientation index for subimages */
  int             nadap;              /* Adaptive class index for codebooks */
  int             q, q1, q2, q3;      /* Indices of codebooks used for 
				       * computation of distortion-rate 
				       * curves */
  int             ncb1, ncb2, ncb3;   /* Indices of codebooks 
				       * for quantization */
  int             nrescb1, nrescb2, nrescb3; /* Indices of residual codebooks 
				       * for quantization */
  int             nresrescb1, nresrescb2; /* Indices of residual codebooks 
				       * for quantization */
  int            *ptrncb1, *ptrncb2, *ptrncb3; /* Pointers to indices 
				       * of codebooks for quantization */
  int            *ptrnrescb1, *ptrnrescb2, *ptrnrescb3; /* Pointers to indices 
				       * of codebooks for quantization */
  int            *ptrnresrescb1, *ptrnresrescb2; /* Pointers to indices 
				       * of codebooks for quantization */
  int             indext;
  int             test_dr;
  double          var1, var2, var3, var4; /* Varainces of different classes 
				       * in a subimage */
  double          variance;            /* Total variance of a subimage */
  long            num1, num2, num3, num4, num; /* Number of vectors in a 
					* subimage belonging to given class */
  long            size;                /* Size of original image */
  float           allocweight;
  float           frac;                /* Inverse of subsampling factor 
					* in different subimages */
  double	  Ent, RateAr;         /* Entropy and rate with arithmetic 
				        * coding for subimages */
  double	  SNR;
  double          msetot, MSE;         /* m.s.e. for global image 
					* and subimages */
  int             bitmapcode;          /* Encode bitmap even if all codebooks 
					* have size 1 */
  bufind          indcb;               /* Buffer for indices of codebooks */
  bufind          numcb1, numcb2;      /* Buffer for number of codebooks 
				        * for each class and each subimage */
  int             oldnumcb, resnumcb;
  int             sizeb;               /* Dimaension of vectors */
  int            *Print, PPrint;       /* Control of info print by fvq */
  int             Print_Scal;          /* control info print by fscalq */
  int             testprint;           /* test value for info print */
  int             smallheader;         /* Flag to specify small header 
					* for fvq and fscalq */
  float           rateheader;      /* Bit rate to encode header */
  int             center;              /* Flag to specify centering of  
					* quantization grid for fscalq */
  int             nstep_dr;               /* Number of step for scalar 
					* quantization */

  if (*NumRec <= 0)
    *NumRec = CodeBook1->nlevel;
  if (*NumRec > Wtrans->nlevel)
    *NumRec = Wtrans->nlevel;

  if (NumRecScal)
    NLevelScal = *NumRecScal;
  else
    NLevelScal = *NumRec + 1;

  size = Wtrans->nrow * Wtrans->ncol;

  rateheader = ((double) 2.0 * NBIT_SIZEIM + NBIT_NLEVEL + 1.0 + 3.0 * (double) *NumRec) / (double) size;

  /*--- Memory allocation for wavelet transform Output ---*/

  if (mw_alloc_ortho_wtrans2d(Output, *NumRec, Wtrans->nrow, Wtrans->ncol) == NULL)
    mwerror(FATAL, 1, "Allocation of buffer for wavelet transform refused!");

  /*--- Set indices for codebooks to 0 ---*/

  indcb[0][*NumRec][0] = 0;
  for (J = 1; J <= *NumRec; J++) 
    for (i = 1; i <= 3; i++) 
      for (nadap = 0; nadap < MAX_WADAP; nadap++) 
	indcb[nadap][J][i] = 0;

  if (MultiCB) {

    /*--- Search best memory allocation between subimages ---*/

    if (*MultiCB != 3) {
   
      /*--- Set number of codebooks to 0 for each subimage ---*/
                  /*--- and each adaptive class ---*/

      numcb[0][*NumRec][0] = numcb1[0][*NumRec][0] = numcb2[0][*NumRec][0] = 0;
      for (J = 1; J <= *NumRec; J++) 
	for (i = 1; i <= 3; i++) 
	  for (nadap = 0; nadap < nadapcb[J][i]; nadap++) 
	    numcb[nadap][J][i] = numcb1[nadap][J][i] = numcb2[nadap][J][i] = 0;
    }

    if (*MultiCB % 3 == 1) {

      /*--- Compute approximated distorsion rate curves for details ---*/

      COMPUTE_CONST(CodeBook1, *NumRec, NLevelScal);

      frac = 1.0;
      for (J = 1; J <= *NumRec; J++) {
	frac /= 4.0;
	for (i = 1; i <= 3; i++) {

	  /*--- Compute variance of coefficients in each adaptive class ---*/

	  fvariance(Wtrans, J, i, CodeBook1, CodeBook2, CodeBook3, &var1, &var2, &var3, &var4, &num1, &num2, &num3, &num4);

	  /*--- Compute number of codebooks in each adaptive class ---*/

	  numcb[0][J][i] = count_cb(CodeBook1->images[J][i]);
	  if ((numcb[0][J][i] <= 0) || (J > NLevelScal)) {
	    sizeb = 1;
	    numcb1[0][J][i] = numcb2[0][J][i] = numcb[0][J][i] = NUMSTEP_SCALQ;
	    nadapcb[J][i] = 1;
	    brl[0][J][i] = 0.0;
	    variance = 0.0;
	    var1 = num1 * var1 + num2 * var2 + num3 * var3 + num4 * var4;
	    num1 += num2 + num3 + num4;
	    var1 /= (double) num1;
	  } else
	    {
	      sizeb = CodeBook1->images[J][i]->ncol;
	      numcb1[0][J][i] = numcb[0][J][i];
	      if (ResCodeBook1) {
		numcb[0][J][i] += count_cb(ResCodeBook1->images[J][i]);
		numcb2[0][J][i] = numcb[0][J][i];
		if (ResResCodeBook1) 
		  numcb[0][J][i] += count_cb(ResResCodeBook1->images[J][i]);
	      } else
		numcb2[0][J][i] = numcb[0][J][i];
	    

	      if (nadapcb[J][i] > 1) {
		numcb[1][J][i] = count_cb(CodeBook2->images[J][i]);
		numcb1[1][J][i] = numcb[1][J][i];
		if (ResCodeBook2) {
		  numcb[1][J][i] += count_cb(ResCodeBook2->images[J][i]);
		  numcb2[1][J][i] = numcb[1][J][i];
		  if (ResResCodeBook2) 
		    numcb[1][J][i] += count_cb(ResResCodeBook2->images[J][i]);
		} else
		  numcb2[1][J][i] = numcb[1][J][i];
	      }

	      if (nadapcb[J][i] > 2) {
		numcb[2][J][i] = count_cb(CodeBook3->images[J][i]);
		numcb1[2][J][i] = numcb[2][J][i];
		if (ResCodeBook2) 
		  numcb[2][J][i] += count_cb(ResCodeBook3->images[J][i]);
		numcb2[2][J][i] = numcb[2][J][i];
	      }
	    

	      /*--- Total variance of subimage ---*/

	      variance = (num2 * var2 + num3 * var3 + num4 * var4) / (double) size;
 
	      /*--- Compute approximate m.s.e. and rates for coding ---*/
	                  /*--- of first class vectors ---*/

	      /*--- Compute approximate rate for coding classes map ---*/

	      if (num2 > 0) {
		brl[0][J][i] = (num1 + num2 + num3 + num4) * log10((double) (num1 + num2 + num3 + num4));
		if (num1 > 0)
		  brl[0][J][i] -= num1 * log10((double) num1);
		brl[0][J][i] -= (num2 + num3 + num4) * log10((double) (num2 + num3 + num4));
		brl[0][J][i] /= ((double) num1 + num2 + num3 + num4) * log10((double) 2.0) * sizeb;
		brl[0][J][i] *= frac;
	      } else 
		brl[0][J][i] = 0.0;

	    }

	  /*--- Compute approximate m.s.e. and rate for coding ---*/
	             /*--- of quantization symbols ---*/

	  brate[0][J][i][0] = 0.0;
	  bmse[0][J][i][0] = num1 * var1 / (double) size + variance;
	  for (q = 1; q < numcb1[0][J][i]; q++) {
	    brate[0][J][i][q] = num1 * q / (double) size / (double) sizeb + brl[0][J][i];
	    bmse[0][J][i][q] = num1 * var1 * bfacmse[0][J][i] * pow((double) 2.0, - (double) 2.0 * q / (double) sizeb) / (double) size + variance;
	  }
	  for (q = numcb1[0][J][i]; q < numcb2[0][J][i]; q++) {
	    brate[0][J][i][q] = num1 * (q - 1.0) / (double) size / (double) sizeb + brl[0][J][i];
	    bmse[0][J][i][q] = num1 * var1 * bfacmse[0][J][i] * pow((double) 2.0, - (double) 2.0 * (q - 1.0) / (double) sizeb) / (double) size + variance;
	  }
	  for (q = numcb2[0][J][i]; q < numcb[0][J][i]; q++) {
	    brate[0][J][i][q] = num1 * (q - 2.0) / (double) size / (double) sizeb + brl[0][J][i];
	    bmse[0][J][i][q] = num1 * var1 * bfacmse[0][J][i] * pow((double) 2.0, - (double) 2.0 * (q - 2.0) / (double) sizeb) / (double) size + variance;
	  }

	  for (nadap = 1; nadap < nadapcb[J][i]; nadap++) {

	  /*--- Compute approximate m.s.e. and rates for coding ---*/
	           /*--- of subsequent classes vectors ---*/

	  /*--- Compute approximate rate for coding classes map ---*/

	    if (nadap == 1) {
	      num = num2;
	      variance = num * var2 / (double) size;

	      if (num3 > 0) {
		brl[1][J][i] = (num1 + num2 + num3 + num4) * log10((double) (num1 + num2 + num3 + num4));
		if (num1 > 0)
		  brl[1][J][i] -= num1 * log10((double) num1);
		if (num2 > 0)
		  brl[1][J][i] -= num2 * log10((double) num2);
		brl[1][J][i] -= (num3 + num4) * log10((double) (num3 + num4));
		brl[1][J][i] /= ((double) num1 + num2 + num3 + num4) * log10((double) 2.0) * sizeb;
		brl[1][J][i] *= frac;
		brl[1][J][i] -= brl[0][J][i];
	      } else
		brl[1][J][i] = 0.0;

	    } else
	      if (nadap == 2) {
		num = num3;
		variance = num * var3 / (double) size;
		brl[2][J][i] = 0.0;
	      } else
		{
		  num = num4;
		  variance = num * var4 / (double) size;
		  brl[3][J][i] = 0.0;
		}

	  /*--- Compute approximate m.s.e. and rate for coding ---*/
	             /*--- of quantization symbols ---*/

	    brate[nadap][J][i][0] = 0.0;
	    bmse[nadap][J][i][0] = 0.0;
	    for (q = 1; q < numcb1[nadap][J][i]; q++) {
	      brate[nadap][J][i][q] = num * q / (double) size / (double) sizeb + brl[nadap][J][i];
	      bmse[nadap][J][i][q] = variance * bfacmse[nadap][J][i] * (pow((double) 2.0, - (double) 2.0 * q / (double) sizeb) - 1.0);
	    }
	    for (q = numcb1[nadap][J][i]; q < numcb2[nadap][J][i]; q++) {
	      brate[nadap][J][i][q] = num * (q - 1.0) / (double) size / (double) sizeb + brl[nadap][J][i];
	      bmse[nadap][J][i][q] = variance * bfacmse[nadap][J][i] * (pow((double) 2.0, - (double) 2.0 * (q - 1.0) / (double) sizeb) - 1.0);
	    }
	    for (q = numcb2[nadap][J][i]; q < numcb[nadap][J][i]; q++) {
	      brate[nadap][J][i][q] = num * (q - 2.0) / (double) size / (double) sizeb + brl[nadap][J][i];
	      bmse[nadap][J][i][q] = variance * bfacmse[nadap][J][i] * (pow((double) 2.0, - (double) 2.0 * (q - 2.0) / (double) sizeb) - 1.0);
	    }
	  }
	}
      }

      /*--- Compute approximated distorsion rate curves for average ---*/

      /*--- Compute variance of coefficients ---*/

      fvariance(Wtrans, *NumRec, 0, CodeBook1, CodeBook2, CodeBook3, &var1, &var2, &var3, &var4, &num1, &num2, &num3, &num4);

      /*--- Compute number of codebooks and prepare codebooks ---*/

      numcb[0][*NumRec][0] = count_cb(CodeBook1->images[*NumRec][0]);
      if ((numcb[0][*NumRec][0] <= 0) || (*NumRec >= NLevelScal)) {
	sizeb = 1;
	numcb1[0][*NumRec][0] = numcb[0][*NumRec][0] = NUMSTEP_SCALQ;
      } else
	{
	  sizeb = CodeBook1->images[*NumRec][0]->ncol;
	  numcb1[0][*NumRec][0] = numcb[0][*NumRec][0];
	  if (ResCodeBook1) {
	    numcb[0][*NumRec][0] += count_cb(ResCodeBook1->images[*NumRec][0]);
	    numcb2[0][*NumRec][0] = numcb[0][*NumRec][0];
	    if (ResResCodeBook1)
	      numcb[0][*NumRec][0] += count_cb(ResResCodeBook1->images[*NumRec][0]);
	  } else
	    numcb2[0][*NumRec][0] = numcb[0][*NumRec][0];
	}

      /*--- Compute approximate m.s.e. and rate for coding ---*/
                 /*--- of quantization symbols ---*/

      for (q = 0; q < numcb1[0][*NumRec][0]; q++) {
	brate[0][*NumRec][0][q] = num1 * q / (double) size / (double) sizeb;
	bmse[0][*NumRec][0][q] = num1 * var1 * bfacmse[0][*NumRec][0] * pow((double) 2.0, - (double) 2.0 * q / (double) sizeb) / (double) size;
      }
      for (q = numcb1[0][*NumRec][0]; q < numcb2[0][*NumRec][0]; q++) {
	brate[0][*NumRec][0][q] = num1 * (q - 1.0)  / (double) size / (double) sizeb;
	bmse[0][*NumRec][0][q] = num1 * var1 * bfacmse[0][*NumRec][0] * pow((double) 2.0, - (double) 2.0 * (q - 1.0) / (double) sizeb) / (double) size;
      }
      for (q = numcb2[0][*NumRec][0]; q < numcb[0][*NumRec][0]; q++) {
	brate[0][*NumRec][0][q] = num1 * (q - 2.0)  / (double) size / (double) sizeb;
	bmse[0][*NumRec][0][q] = num1 * var1 * bfacmse[0][*NumRec][0] * pow((double) 2.0, - (double) 2.0 * (q - 2.0) / (double) sizeb) / (double) size;
      }

    } else
      if (*MultiCB % 3 == 2) {

	/*--- Compute exact distorsion rate curves for details ---*/

	PPrint = 2;
	Print = &PPrint;
	frac = 1.0;
	for (J = 1; J <= *NumRec; J++) {
	  frac /= 4.0;
	  for (i = 1; i <= 3; i++) {
	    numcb[0][J][i] = count_cb(CodeBook1->images[J][i]);
	    if ((numcb[0][J][i] <= 0) || (J > NLevelScal)) {
	      sizeb = 1;
	      numcb[0][J][i] = NUMSTEP_SCALQ;
	      brl[0][J][i] = 0.0;
	      nadapcb[J][i] = 1;
	      for (q = 0; q < numcb[0][J][i]; q++) {
		if (q > 0)
		  /* FIXME: really? op precedence? */
		  nstep_dr = 1 + (1 << (q+1));
		else
		  nstep_dr = 1;
		fscalq(Print, &smallheader, &nstep_dr, NULL, &center, NULL, Wtrans->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);
		bmse[0][J][i][q] = frac * MSE;
		brate[0][J][i][q] = frac * RateAr;
	      }
	    } else
	      {
		sizeb = CodeBook1->images[J][i]->ncol;
		for (q = 0; q < numcb[0][J][i]; q++) {
		  fvq(Print, NULL, &bitmapcode, NULL, &q, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, Wtrans->images[J][i], CodeBook1->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);
		  if ((q == 0) && (CodeBook1->images[J][i]->gray[(CodeBook1->images[J][i]->nrow - 3) * sizeb] != 0.0)) {
		    brate[0][J][i][q] = 0.0;
		    bmse[0][J][i][q] = frac * det_energy(Wtrans->images[J][i]);
		    brl[0][J][i] = frac * RateAr;
		  } else
		    {
		      bmse[0][J][i][q] = frac * MSE;
		      brate[0][J][i][q] = frac * RateAr;
		    }
		}

		if (ResCodeBook1)
		  if ((ResCodeBook1->images[J][i]->nrow > 6) && (ResCodeBook1->images[J][i]->ncol == sizeb)) {
		    q1 = numcb[0][J][i] - 1;
		    oldnumcb = numcb[0][J][i];
		    resnumcb = count_cb(ResCodeBook1->images[J][i]);
		    numcb[0][J][i] += resnumcb;
		    for (q = 0; q < resnumcb; q++) {
		      fvq(Print, NULL, &bitmapcode, NULL, &q1, NULL, NULL, NULL, NULL, NULL, NULL, &q, NULL, NULL, NULL, ResCodeBook1->images[J][i], NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, Wtrans->images[J][i], CodeBook1->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);
		      bmse[0][J][i][oldnumcb + q] = frac * MSE;
		      brate[0][J][i][oldnumcb + q] = frac * RateAr;
		    }

		    if (ResResCodeBook1)
		      if ((ResResCodeBook1->images[J][i]->nrow > 6) && (ResResCodeBook1->images[J][i]->ncol == sizeb)) {
			q2 = resnumcb - 1;
			oldnumcb = numcb[0][J][i];
			resnumcb = count_cb(ResResCodeBook1->images[J][i]);
			numcb[0][J][i] += resnumcb;
			for (q = 0; q < resnumcb; q++) {
			  fvq(Print, NULL, &bitmapcode, NULL, &q1, NULL, NULL, NULL, NULL, NULL, NULL, &q2, NULL, NULL, NULL, ResCodeBook1->images[J][i], NULL, NULL, NULL, &q, NULL, ResResCodeBook1->images[J][i], NULL, NULL, Wtrans->images[J][i], CodeBook1->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);
			  bmse[0][J][i][oldnumcb + q] = frac * MSE;
			  brate[0][J][i][oldnumcb + q] = frac * RateAr;
			}

		      }
		  }

		if (CodeBook2 && (nadapcb[J][i] >= 2)) 
		  if ((CodeBook2->images[J][i]->nrow > 6) && (CodeBook2->images[J][i]->ncol == sizeb)) {
		    q1 = 1;
		    numcb[1][J][i] = count_cb(CodeBook2->images[J][i]);
		    for (q = 0; q < numcb[1][J][i]; q++) {
		      fvq(Print, NULL, &bitmapcode, NULL, &q1, &q, NULL, NULL, CodeBook2->images[J][i], NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, Wtrans->images[J][i], CodeBook1->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);
		      if ((q == 0) && (CodeBook2->images[J][i]->gray[(CodeBook2->images[J][i]->nrow - 3) * sizeb] != 0.0)) {
			brate[1][J][i][q] = 0.0;
			bmse[1][J][i][q] = 0.0;
			brl[1][J][i] = frac * RateAr - brate[0][J][i][q1];
		      } else
			{
			  bmse[1][J][i][q] = frac * MSE - bmse[0][J][i][q1];
			  brate[1][J][i][q] = frac * RateAr - brate[0][J][i][q1];
			}
		    }

		    if (ResCodeBook2)
		      if ((ResCodeBook2->images[J][i]->nrow > 6) && (ResCodeBook2->images[J][i]->ncol == sizeb)) {
			q2 = numcb[1][J][i] - 1;
			oldnumcb = numcb[1][J][i];
			resnumcb = count_cb(ResCodeBook2->images[J][i]);
			numcb[1][J][i] += resnumcb;
			for (q = 0; q < resnumcb; q++) {
			  fvq(Print, NULL, &bitmapcode, NULL, &q1, &q2, NULL, NULL, CodeBook2->images[J][i], NULL, NULL, NULL, &q, NULL, NULL, NULL, ResCodeBook2->images[J][i], NULL, NULL, NULL, NULL, NULL, NULL, NULL, Wtrans->images[J][i], CodeBook1->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);
			  bmse[1][J][i][oldnumcb + q] = frac * MSE - bmse[0][J][i][q1];
			  brate[1][J][i][oldnumcb + q] = frac * RateAr - brate[0][J][i][q1];
			}

			if (ResResCodeBook2)
			  if ((ResResCodeBook2->images[J][i]->nrow > 6) && (ResResCodeBook2->images[J][i]->ncol == sizeb)) {
			    q3 = resnumcb - 1;
			    oldnumcb = numcb[1][J][i];
			    resnumcb = count_cb(ResResCodeBook2->images[J][i]);
			    numcb[1][J][i] += resnumcb;
			    for (q = 0; q < resnumcb; q++) {
			      fvq(Print, NULL, &bitmapcode, NULL, &q1, &q2, NULL, NULL, CodeBook2->images[J][i], NULL, NULL, NULL, &q3, NULL, NULL, NULL, ResCodeBook2->images[J][i], NULL, NULL, NULL, &q, NULL, ResResCodeBook2->images[J][i], NULL, Wtrans->images[J][i], CodeBook1->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);
			      bmse[1][J][i][oldnumcb + q] = frac * MSE - bmse[0][J][i][q1];
			      brate[1][J][i][oldnumcb + q] = frac * RateAr - brate[0][J][i][q1];
			    }
			  }		
		      }		
	    
		    if (CodeBook3 && (nadapcb[J][i] >= 3)) 
		      if ((CodeBook3->images[J][i]->nrow > 6) && (CodeBook3->images[J][i]->ncol == sizeb)) {
			q2 = 1;
			numcb[2][J][i] = count_cb(CodeBook3->images[J][i]);
			for (q = 0; q < numcb[2][J][i]; q++) {
			  fvq(Print, NULL, &bitmapcode, NULL, &q1, &q2, &q, NULL, CodeBook2->images[J][i], CodeBook3->images[J][i], NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, Wtrans->images[J][i], CodeBook1->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);
			  bmse[2][J][i][q] = frac * MSE - bmse[0][J][i][q1] - bmse[1][J][i][q2];
			  brate[2][J][i][q] = frac * RateAr - brate[0][J][i][q1] - brate[1][J][i][q2];
			}

			if (ResCodeBook3)
			  if ((ResCodeBook3->images[J][i]->nrow > 6) && (ResCodeBook3->images[J][i]->ncol == sizeb)) {
			    q3 = numcb[2][J][i] - 1;
			    oldnumcb = numcb[2][J][i];
			    resnumcb = count_cb(ResCodeBook3->images[J][i]);
			    numcb[2][J][i] += resnumcb;
			    for (q = 0; q < resnumcb; q++) {
			      fvq(Print, NULL, &bitmapcode, NULL, &q1, &q2, &q3, NULL, CodeBook2->images[J][i], CodeBook3->images[J][i], NULL, NULL, NULL, &q, NULL, NULL, NULL, ResCodeBook3->images[J][i], NULL, NULL, NULL, NULL, NULL, NULL, Wtrans->images[J][i], CodeBook1->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);
			      bmse[2][J][i][oldnumcb + q] = frac * MSE - bmse[0][J][i][q1] - bmse[1][J][i][q2];
			      brate[2][J][i][oldnumcb + q] = frac * RateAr - brate[0][J][i][q1] - brate[1][J][i][q2];
			    }
			
			  }
		      }
		  }
	      }
	  }
	}

	/*--- Compute distorsion rate curves for average ---*/

	numcb[0][*NumRec][0] = count_cb(CodeBook1->images[*NumRec][0]);
	if ((numcb[0][*NumRec][0] <= 0) || (*NumRec >= NLevelScal)) {
	  sizeb = 1;
	  numcb[0][*NumRec][0] = NUMSTEP_SCALQ;
	  for (q = 0, nstep_dr = 1; q < numcb[0][*NumRec][0]; q++) {
	    fscalq(Print, &smallheader, &nstep_dr, NULL, NULL, NULL, Wtrans->images[*NumRec][0], Output->images[*NumRec][0], &MSE, &SNR, &Ent, &RateAr);
	    bmse[0][*NumRec][0][q] = frac * MSE;
	    brate[0][*NumRec][0][q] = frac * RateAr;
	    nstep_dr *= 2;
	  }
	} else
	  {
	    sizeb = CodeBook1->images[*NumRec][0]->ncol;
	    for (q = 0; q < numcb[0][*NumRec][0]; q++) {
	      fvq(Print, NULL, NULL, NULL, &q, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, Wtrans->images[*NumRec][0], CodeBook1->images[*NumRec][0], Output->images[*NumRec][0], &MSE, &SNR, &Ent, &RateAr);
	      bmse[0][*NumRec][0][q] = frac * MSE;
	      brate[0][*NumRec][0][q] = frac * RateAr;
	    }
	    if (ResCodeBook1->images[*NumRec][0])
	      if ((ResCodeBook1->images[*NumRec][0]->nrow > 6) && (ResCodeBook1->images[*NumRec][0]->ncol == sizeb)) {
		q1 = numcb[0][*NumRec][0] - 1;
		oldnumcb = numcb[0][*NumRec][0];
		resnumcb = count_cb(ResCodeBook1->images[*NumRec][0]);
		numcb[0][*NumRec][0] += resnumcb;
		for (q = 0; q < resnumcb; q++) {
		  fvq(Print, NULL, NULL, NULL, &q1, NULL, NULL, NULL, NULL, NULL, NULL, &q, NULL, NULL, NULL, ResCodeBook1->images[*NumRec][0], NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, Wtrans->images[*NumRec][0], CodeBook1->images[*NumRec][0], Output->images[*NumRec][0], &MSE, &SNR, &Ent, &RateAr);
		  bmse[0][*NumRec][0][oldnumcb + q] = frac * MSE;
		  brate[0][*NumRec][0][oldnumcb + q] = frac * RateAr;
		}

		if (ResResCodeBook1->images[*NumRec][0])
		  if ((ResResCodeBook1->images[*NumRec][0]->nrow > 6) && (ResResCodeBook1->images[*NumRec][0]->ncol == sizeb)) {
		    q2 = resnumcb - 1;
		    oldnumcb = numcb[0][*NumRec][0];
		    resnumcb = count_cb(ResResCodeBook1->images[*NumRec][0]);
		    numcb[0][*NumRec][0] += resnumcb;
		    for (q = 0; q < resnumcb; q++) {
		      fvq(Print, NULL, NULL, NULL, &q1, NULL, NULL, NULL, NULL, NULL, NULL, &q2, NULL, NULL, NULL, ResCodeBook1->images[*NumRec][0], NULL, NULL, NULL, &q, NULL, ResResCodeBook1->images[*NumRec][0], NULL, NULL, Wtrans->images[*NumRec][0], CodeBook1->images[*NumRec][0], Output->images[*NumRec][0], &MSE, &SNR, &Ent, &RateAr);
		      bmse[0][*NumRec][0][oldnumcb + q] = frac * MSE;
		      brate[0][*NumRec][0][oldnumcb + q] = frac * RateAr;
		    }
		  }		
	      }		
	  }
      }	
		
    /*--- Multiply m.s.e. errors by a weight in order to get better ---*/
                    /*--- psychovisual quality ---*/

    if (ScaleWeight)  
      if (*ScaleWeight > 0.0) {
	allocweight = 1.0;
	for (J = 2; J <= *NumRec; J++) {
	  allocweight *= *ScaleWeight;
	  for (i = 1; i <= 3; i++) 
	    for (nadap = 0; nadap < nadapcb[J][i]; nadap++) 
	      for (q = 0; q < numcb[nadap][J][i]; q++) 
		bmse[nadap][J][i][q] *= allocweight;
		
	}
	for (q = 0; q < numcb[0][*NumRec][0]; q++) 
	  bmse[0][*NumRec][0][q] *= allocweight;
	  
      }

    if (!TargRate) {

      if (*MultiCB <= 2) {

	/*--- Compute distorsion-rate curve for the wavelet transform ---*/

	test_dr = 0;
	for (q = 0; q <= max_count_dr; q++) {

	  /*--- Compute indices of codebooks for optimal allocation ---*/ 

	  compute_indexcb(indcb, targrate_dr[q], rateheader, *NumRec, test_dr, *MultiCB);

	  /*--- Compute resulting m.s.e. and rate ---*/

	  MSE = 0.0;
	  RateAr = rateheader;
	  for (J = 1; J <= *NumRec; J++) 
	    for (i = 1; i <= 3; i++) {
	      for (nadap = 0; nadap < nadapcb[J][i]; nadap++) {
		MSE += bmse[nadap][J][i][indcb[nadap][J][i]];
		RateAr += brate[nadap][J][i][indcb[nadap][J][i]];
		if ((nadap > 1) && (indcb[nadap][J][i] > 0))
		  if (indcb[nadap - 1][J][i] == 0)
		    RateAr += brl[nadap - 1][J][i];
	      }
	    }
	  MSE += bmse[0][*NumRec][0][indcb[0][*NumRec][0]];
	  RateAr += brate[0][*NumRec][0][indcb[0][*NumRec][0]];
	  printf("%.4f  %.2f\n", RateAr, 10.0 * log10((double) 255.0 * 255.0 / MSE));		
	}
      }

    } else
      {
	/*--- Compute indices of codebooks for optimal allocation ---*/ 
	        /*--- and for a specified target bitrate ---*/

	test_dr = 0;
	compute_indexcb(indcb, *TargRate, rateheader, *NumRec, test_dr, *MultiCB);

      }
  }


  if (TargRate || !MultiCB) {

    /*--- Perform vector quantization using indices of codebooks ---*/ 
                      /*--- computed above ---*/
  
    testprint = 0;
    if (TargRate)
      testprint = 1;
    if (!MultiCB)
      testprint = 1;
    else
      if (*MultiCB != 3)
	testprint = 1;
      else
	testprint = 0;
    rateartot = 0.0;
    PPrint = 1;
    if (testprint == 0)
      PPrint = 2;
    Print = &PPrint;
    Print_Scal = 2;

    INIT_ENCODING_WTRANS(Wtrans->nrow, Wtrans->ncol, *NumRec, CodeBook1, NLevelScal, Compress);
     
    /*--- Quantization of details ----*/

    for (J = 1; J <= *NumRec; J++) {

      if (CodeBook1->images[J][1]) 
	if (CodeBook1->images[J][1]->nrow > 4) {
	  indext = (CodeBook1->images[J][1]->nrow - 3) * CodeBook1->images[J][1]->ncol;
	  if ((CodeBook1->images[J][1]->gray[indext] > 0.0) && (testprint == 1))
	    printf("\nLevel %d :\n", J);
	}

      for (i = 1; i <= 3; i++) {

	if ((count_cb(CodeBook1->images[J][i]) <= 0) || (J > NLevelScal)) {

	  /*--- Scalar quantization ---*/ 
	  
	  if (TargRate) {
	    nstep_dr = 1;
	    if (indcb[0][J][i] > 0)
	      /* FIXME: really? op precedence? */
	      nstep_dr = 1 + (1 << (indcb[0][J][i] + 1));
	  } else
	    if (*NStep > 0)
	      nstep_dr = *NStep >> (*NumRec - J + 1);
	    else
	      nstep_dr = 1 << (*NumRec - J);

	  fscalq(&Print_Scal, &smallheader, &nstep_dr, NULL, &center, Compress, Wtrans->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);

	} else
	  {

	    /*--- Vector quantization ---*/ 
	    
	    indext = (CodeBook1->images[J][i]->nrow - 3) * CodeBook1->images[J][i]->ncol;
	    if ((CodeBook1->images[J][i]->gray[indext] > 0.0) && (testprint == 1))
	      printf("Detail %d :\n", i);

	    if (MultiCB) {
	  
	      AdapCB2 = AdapCB3 = NULL;
	      ResCB1 = ResResCB1 = NULL;
	      ResCB2 = ResResCB2 = NULL;
	      ResCB3 = NULL;

	      /*--- Prepare indices for codebooks for details ---*/ 

	      ptrncb1 = &ncb1;
	      ptrnrescb1 = &nrescb1;
	      ptrnresrescb1 = &nresrescb1;
	      oldnumcb = count_cb(CodeBook1->images[J][i]);
	      if (indcb[0][J][i] < oldnumcb) {
		ncb1 = indcb[0][J][i];
		ptrnrescb1 = ptrnresrescb1 = NULL;
	      } else
		{
		  ResCB1 = ResCodeBook1->images[J][i];
		  ncb1 = oldnumcb - 1;
		  resnumcb = indcb[0][J][i] - oldnumcb;
		  oldnumcb = count_cb(ResCB1);
		  if (resnumcb < oldnumcb) {
		    nrescb1 = resnumcb;
		    ptrnresrescb1 = NULL;
		  } else
		    {
		      ResResCB1 = ResResCodeBook1->images[J][i];
		      nrescb1 = oldnumcb - 1;
		      resnumcb -= oldnumcb;
		      oldnumcb = count_cb(ResResCB1);
		      if (resnumcb < oldnumcb) 
			nresrescb1 = resnumcb;
		      else
			mwerror(FATAL, 2, "Something wrong with value of codebook index for detail %d/%d.\n", J, i);		
		    }
		}
    
	      if (CodeBook2 && (nadapcb[J][i] >= 2) && (indcb[1][J][i] > 0)) {
		AdapCB2 = CodeBook2->images[J][i];
		ptrncb2 = &ncb2;
		ptrnrescb2 = &nrescb2;
		ptrnresrescb2 = &nresrescb2;
		oldnumcb = count_cb(AdapCB2);
		if (indcb[1][J][i] < oldnumcb) {
		  ncb2 = indcb[1][J][i];
		  ptrnrescb2 = ptrnresrescb2 = NULL;
		} else
		  {
		    ResCB2 = ResCodeBook2->images[J][i];
		    ncb2 = oldnumcb - 1;
		    resnumcb = indcb[1][J][i] - oldnumcb;
		    oldnumcb = count_cb(ResCB2);
		    if (resnumcb < oldnumcb) {
		      nrescb2 = resnumcb;
		      ptrnresrescb2 = NULL;
		    } else
		      {
			ResResCB2 = ResResCodeBook2->images[J][i];
			nrescb2 = oldnumcb - 1;
			resnumcb -= oldnumcb;
			oldnumcb = count_cb(ResResCB2);
			if (resnumcb < oldnumcb) 
			  nresrescb2 = resnumcb;
			else
			  mwerror(FATAL, 2, "Something wrong with value of second class codebook index for detail %d/%d.\n", J, i);
		      }
		  }

		if (CodeBook3 && (nadapcb[J][i] >= 3) && (indcb[2][J][i] > 0)) {
		  AdapCB3 = CodeBook3->images[J][i];
		  ptrncb3 = &ncb3;
		  ptrnrescb3 = &nrescb3;
		  oldnumcb = count_cb(AdapCB3);
		  if (indcb[2][J][i] < oldnumcb) {
		    ncb3 = indcb[2][J][i];
		    ptrnrescb3 = NULL;
		  } else
		    {
		      ResCB3 = ResCodeBook3->images[J][i];
		      ncb3 = oldnumcb - 1;
		      resnumcb = indcb[2][J][i] - oldnumcb;
		      oldnumcb = count_cb(ResCB3);
		      if (resnumcb < oldnumcb) 
			nrescb3 = resnumcb;
		      else
			mwerror(FATAL, 2, "Something wrong with value of third class codebook index for detail %d %d.\n", J, i);
		    }
		} else
		  ptrncb3 = ptrnrescb3 = NULL;
	      } else
		{
		  ptrncb2 = ptrnrescb2 = ptrnresrescb2 = NULL;
		  ptrncb3 = ptrnrescb3 = NULL;
		}
	    } else
	      {
		ptrncb1 = ptrnrescb1 = ptrnresrescb1 = NULL;
		ptrncb2 = ptrnrescb2 = ptrnresrescb2 = NULL;
		ptrncb3 = ptrnrescb3 = NULL;
		ResCB1 = ResCodeBook1->images[J][i];
		ResResCB1 = ResResCodeBook1->images[J][i];
		AdapCB2 = CodeBook2->images[J][i];
		ResCB2 = ResCodeBook2->images[J][i];
		ResResCB2 = ResResCodeBook2->images[J][i];
		AdapCB3 = CodeBook3->images[J][i];
		ResCB3 = ResCodeBook3->images[J][i];
	      }
      
	    fvq(Print, &smallheader, NULL, NULL, ptrncb1, ptrncb2, ptrncb3, NULL, AdapCB2, AdapCB3, NULL, ptrnrescb1, ptrnrescb2, ptrnrescb3, NULL, ResCB1, ResCB2, ResCB3, NULL, ptrnresrescb1, ptrnresrescb2, ResResCB1, ResResCB2, Compress, Wtrans->images[J][i], CodeBook1->images[J][i], Output->images[J][i], &MSE, &SNR, &Ent, &RateAr);
	  }

	brate[0][J][i][0] = RateAr;
	bmse[0][J][i][0] = MSE;

      }
    }
    
    /*--- Quantization of resume ---*/

    if ((count_cb(CodeBook1->images[*NumRec][0]) <= 0) || (*NumRec >= NLevelScal)) {
      
      /*--- Scalar quantization ---*/ 

      if (TargRate)
	nstep_dr = 1<<indcb[0][*NumRec][0];
      else
	if (*NStep > 0)
	  nstep_dr = *NStep;
	else
	  nstep_dr = 1 << (*NumRec + 1);

      fscalq(&Print_Scal, &smallheader, &nstep_dr, NULL, NULL, Compress, Wtrans->images[*NumRec][0], Output->images[*NumRec][0], &MSE, &SNR, &Ent, &RateAr);

    } else
      {
	
	/*--- Vector quantization ---*/

	if (MultiCB) {

	  /*--- Prepare indices for codebooks for average ---*/ 

	  ptrncb1 = &ncb1;
	  ptrnrescb1 = &nrescb1;
	  ptrnresrescb1 = &nresrescb1;
	  oldnumcb = count_cb(CodeBook1->images[*NumRec][0]);
	  if (indcb[0][*NumRec][0] < oldnumcb) {
	    ncb1 = indcb[0][*NumRec][0];
	    ptrnrescb1 = ptrnresrescb1 = NULL;
	  } else
	    {
	      ncb1 = oldnumcb - 1;
	      resnumcb = indcb[0][*NumRec][0] - oldnumcb;
	      oldnumcb = count_cb(ResCodeBook1->images[*NumRec][0]);
	      if (resnumcb < oldnumcb) {
		nrescb1 = resnumcb;
		ptrnresrescb1 = NULL;
	      } else
		{
		  nrescb1 = oldnumcb - 1;
		  resnumcb -= oldnumcb;
		  oldnumcb = count_cb(ResResCodeBook1->images[*NumRec][0]);
		  if (resnumcb < oldnumcb)
		    nresrescb1 = resnumcb;
		  else
		    mwerror(FATAL, 2, "Something wrong with value of codebook index for average.\n");
		}
	    }
		
	}
    
	fvq(Print, &smallheader, NULL, NULL, ptrncb1, NULL, NULL, NULL, NULL, NULL, NULL, ptrnrescb1, NULL, NULL, NULL, ResCodeBook1->images[*NumRec][0], NULL, NULL, NULL, ptrnresrescb1, NULL, ResResCodeBook1->images[*NumRec][0], NULL, Compress, Wtrans->images[*NumRec][0], CodeBook1->images[*NumRec][0], Output->images[*NumRec][0], &MSE, &SNR, &Ent, &RateAr);
      
      }

       /*--- Compute resulting distorsion and rate, ---*/
    /*--- print info on quantization of each sub-image ---*/

    brate[0][*NumRec][0][0] = RateAr;
    bmse[0][*NumRec][0][0] = MSE;
    
    msetot = bmse[0][*NumRec][0][0] * (float) Output->images[*NumRec][0]->nrow * Output->images[*NumRec][0]->nrow;
    
    rateartot = brate[0][*NumRec][0][0] * (float) Output->images[*NumRec][0]->nrow * Output->images[*NumRec][0]->nrow;
    
    for (J = 1; J <= *NumRec; J++) {
      if (testprint == 1) {
	printf("\nLevel %d:\n", J);
	printf("Codebook index \t Vector size \t Entropy \t mse \n");
      }
      for (i = 1; i <= 3; i++) {
	rateartot += brate[0][J][i][0] * (float) Wtrans->images[J][i]->nrow * Wtrans->images[J][i]->ncol;
	msetot += bmse[0][J][i][0] * (float) Wtrans->images[J][i]->nrow * Wtrans->images[J][i]->ncol;
	if (testprint == 1) {
	  if (CodeBook3)
	    printf("  %2d/%2d/%d\t     %d", indcb[0][J][i], indcb[1][J][i], indcb[2][J][i], CodeBook1->images[J][i]->ncol);
	  else
	    if (CodeBook2)
	      printf("   %2d/%d \t     %d", indcb[0][J][i], indcb[1][J][i], CodeBook1->images[J][i]->ncol);
	    else
	      printf("     %2d\t\t     %d", indcb[0][J][i], CodeBook1->images[J][i]->ncol);

	  printf("\t\t  %.3f \t%.3f \n", brate[0][J][i][0], bmse[0][J][i][0]);
	}
      }
    }

    rateartot /= (float) Wtrans->nrow * Wtrans->ncol;
    rateartot += rateheader;
    msetot /= (float) Wtrans->nrow * Wtrans->ncol;

    if (testprint == 1) {
      printf("\nAver. Entropy  = %f (%d),  aver. m.s.e. = %f\n", brate[0][*NumRec][0][0], indcb[0][*NumRec][0], bmse[0][*NumRec][0][0]);
      printf("\nTotal Entropy  = %f,  total m.s.e. = %f,  total PSNR = %f\n", rateartot, msetot, 10.0 * log10(255.0 * 255.0 / msetot));
    }
    
    if (Compress)
      RESIZE_COMPRESS_WTRANS(Compress);

  }
}





static void
fwvq_wcb(NumRec, Edge_Ri, Ri2, FilterNorm, WeightFac, NumRecScal, NStep, MultiCB, CodeBook2, CodeBook3, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResResCodeBook1, ResResCodeBook2, DistRate, TargRate, Output, Image, CodeBook1, Ri, QImage)

	/*--- Computes the orthogonal wavelet transform of image `Image` ---*/

int        *NumRec;		/* Number of recursion (-j) */
Fimage	    Edge_Ri;		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
Fsignal     Ri2;		/* Impulse response of the low pass filter */
				/* for synthesis */
int        *FilterNorm;	        /* Equal 0 if no normalisation of filter's tap
			         *       1 if normalisation of the sum 
			         *       2 if normalistion of the square sum */
float      *WeightFac;          /* Weighting factor for wavelet coeff. */
int        *NumRecScal;         /* Scalar quantize wavelet subimages  
				 * at level NumRecScal and higher */
int        *NStep;              /* Number of steps in case of scalar 
				 * quantizarion of resume */
int        *MultiCB;            /* Not selected : one codebook per file
					 1: compute approx. dist-rate curves
					 2: compute exact dist-rate curves */
Wtrans2d    CodeBook2, CodeBook3; /* Sequence of codebooks for adaptive 
			         * quantization */
Wtrans2d    ResCodeBook1, ResCodeBook2, ResCodeBook3; /* Sequence of 
			         * codebooks for residu quantization  
				 * after quantization with CodeBook 
				 * AdapCodeBook2 and AdapCodeBook3 */
Wtrans2d    ResResCodeBook1;    /* Sequence of codebooks for residu 
				 * quantization after quantization 
				 * with CodeBook and ResCodeBook1 */
Wtrans2d    ResResCodeBook2;    /* Sequence of codebooks for residu 
				 * quantization after quantization 
				 * with AdapCodeBook2 and ResCodeBook2 */
int        *DistRate;           /* Compute distortion-rate function */
float      *TargRate;           /* Target bit rate */
Cimage      Output;		/* Compressed `Image` */
Fimage      Image;		/* Input image */
Wtrans2d    CodeBook1;          /* First sequence of codebooks for 
			         * for quantization */
Fsignal     Ri;			/* Impulse response of the low pass filter */
Fimage      QImage;		/* Output quantized image */

{
  int         NumLev;           /* Number of  level of decomposition */
  int         J;	        /* Scale index for subimages */
  int         i;                /* Orientation index for subimages */
  int         nadap;            /* Adaptive class index for codebooks */
  int         q;                /* Indices of codebooks used for 
				 * computation of distortion-rate 
				 * curves */
  int         nrow, ncol;       /* Size of input image */
  bufmse      loc_bmse;         /* Local buffer for m.s.e. for subimages 
				 * and different codebooks */
  bufmse      loc_brate;        /* Local buffer for rate for subimages 
				 * and different codebooks */
  bufrl       loc_brl;          /* Local buffer for classes map rates for 
				 * subimages and different codebooks */
  float       targrate;         /* Target rate for vector quantization */
  Wtrans2d    Wtrans, QWtrans;  /* Wavelet transform of Image, quantized 
				 * wavelet transform */
  Fimage      QImage_dr;        /* Reconstructed image (for biorthogonal 
				 * wavelet transform and dist.rate curve) */
  int         multicb_dr;       /* flag for memory allocation in wvq */
  int        *multicb_null;     /* flag for multi codebooks in wvq */
  int         Haar;             /* Continue decomposition with Haar wavelet
				 * until haar level */
  int         Edge;	        /* Edge processing mode */
  int         FiltNorm;	        /* Normalisation of filters */
  int         Precond;          /* Preconditionning mode for orthogonal 
				 * transform */
  double      mse;	    /* Mean square error between Image and Qimage_dr */
  double      mrd;	    /* Maximal relative difference */	
  double      snr;	    /* Signal to noise ratio / `Image` */
  double      psnr;	    /* Peak signal to noise ratio / `Image` */
  char PsnrFlg = '1';       /* To compute 255-PSNR */

  /*--- Compute number of level for wavelet transform ---*/

  if (NumRec)
    NumLev = *NumRec;
  else 
    NumLev = CodeBook1->nlevel;

  nrow = Image->nrow;
  ncol = Image->ncol;
  while ((nrow % (1<<NumLev) != 0) && (ncol % (1<<NumLev)))
    NumLev--;
  Haar = NumLev;

  if (NumLev == 0)
    mwerror(FATAL, 2, "The dimensions of input image must be even!\n");

  if (!DistRate)
    printf("Number of levels : %d\n", NumLev);
  else
    if (MultiCB)
      multicb_dr = *MultiCB + 3;
    else
      multicb_dr = 5;

  /*--- Wavelet decomposition ---*/

  if (Ri2) {
    Edge = 2;
    if (FilterNorm)
      FiltNorm = *FilterNorm;
    else
      FiltNorm = 1;
    INIT_RI(Ri, Ri2, NULL);

    Wtrans = mw_new_wtrans2d();
    biowave2(&NumLev, &Haar, &Edge, &FiltNorm, Image, Wtrans, Ri, Ri2);
  } else
    {
      if (Edge_Ri)
	Edge = 3;
      else
	Edge = 1;
      if (FilterNorm)
	FiltNorm = *FilterNorm;
      else
	FiltNorm = 2;
      Precond = 0;

      INIT_RI(Ri, NULL, Edge_Ri);
      owave2(&NumLev, &Haar, &Edge, &Precond, NULL, &FiltNorm, Image, Wtrans, Ri, Edge_Ri);
    }

  /*--- Perform vector quantization ---*/

  /*--- Compute number of adaptive classes for each level ---*/
  
  for (J = 1; J <= NumLev; J++) {
    nadapcb[J][0] = 1;
    for (i = 1; i <= 3; i++) {
      nadapcb[J][i] = 1;
      if (CodeBook2) 
	if (CodeBook2->nlevel >= J) 
	  if (CodeBook2->images[J][i]->nrow > 6) {
	    nadapcb[J][i]++;
	    if (CodeBook3)
	      if (CodeBook3->nlevel >= J) 
		if (CodeBook3->images[J][i]->nrow > 6) 
		  nadapcb[J][i]++;
	  }
    }
  }

  /*--- Set target bit rates for R-D curve ---*/ 

  if (DistRate) 
    INIT_TARGNBIT_DR(Wtrans);    

  if (DistRate && (Ri2 || (multicb_dr == 4) || WeightFac || (FiltNorm == 1))) {

    /*--- Compute R-D curve in case of biorthogonal transform ---*/

    QImage_dr = mw_new_fimage();
    QWtrans = mw_new_wtrans2d();

    wvq_loc(&NumLev, WeightFac, NumRecScal, NStep, &multicb_dr, NULL, CodeBook2, CodeBook3, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResResCodeBook1, ResResCodeBook2, NULL, Wtrans, CodeBook1, QWtrans);

    for (J = 1; J <= NumLev; J++) 
      for (i = 0; i <= 3; i++) 
	for (nadap = 0; nadap < MAX_WADAP; nadap++) {
	  for (q = 0; q <= numcb[nadap][J][i]; q++) {
	    loc_bmse[nadap][J][i][q] = bmse[nadap][J][i][q];
	    loc_brate[nadap][J][i][q] = brate[nadap][J][i][q];
	  }	    
	  loc_brl[nadap][J][i] = brl[nadap][J][i];
	}

    mw_delete_wtrans2d(QWtrans);
    
    multicb_dr = 3;
    for (count_dr = 0; count_dr <= max_count_dr; count_dr++) {
      targrate = targrate_dr[count_dr];
      QWtrans = mw_new_wtrans2d();

      wvq_loc(&NumLev, WeightFac, NumRecScal, NStep, &multicb_dr, &targrate, CodeBook2, CodeBook3, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResResCodeBook1, ResResCodeBook2, NULL, Wtrans, CodeBook1, QWtrans);

      for (J = 1; J <= NumLev; J++) 
	for (i = 0; i <= 3; i++) 
	  for (nadap = 0; nadap < MAX_WADAP; nadap++) {
	    for (q = 0; q <= numcb[nadap][J][i]; q++) {
	      bmse[nadap][J][i][q] = loc_bmse[nadap][J][i][q];
	      brate[nadap][J][i][q] = loc_brate[nadap][J][i][q];
	    }	    
	    brl[nadap][J][i] = loc_brl[nadap][J][i];
	  }

      if (Ri2) {
	REFRESH_FILTERS(Ri, Ri2, NULL);
	ibiowave2(&NumLev, &Haar, &Edge, &FiltNorm, QWtrans, QImage_dr, Ri, Ri2);
      } else
	{
	  REFRESH_FILTERS(Ri, NULL, Edge_Ri);
	  iowave2(&NumLev, &Haar, &Edge, &Precond, NULL, &FiltNorm, QWtrans, QImage_dr, Ri, Edge_Ri);
	}

      fmse(Image, QImage_dr, NULL, &PsnrFlg, &snr, &psnr, &mse, &mrd);
      printf("%.4f %.2f\n", rateartot, psnr);
      mw_delete_wtrans2d(QWtrans);
    }

    mw_delete_fimage(QImage_dr);

  } else
    {

      /*--- Compute R-D curve in case of orthogonal transform ---*/
                /*--- or compress wavelet transform ---*/

      QWtrans = mw_new_wtrans2d();
      if (TargRate || DistRate)
	multicb_null = MultiCB;
      else
	multicb_null = NULL;
      wvq_loc(&NumLev, WeightFac, NumRecScal, NStep, multicb_null, TargRate, CodeBook2, CodeBook3, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResResCodeBook1, ResResCodeBook2, Output, Wtrans, CodeBook1, QWtrans);
    }

  /*--- Wavelet reconstruction ---*/

  if (QImage && !DistRate)
    if (Ri2) {
      REFRESH_FILTERS(Ri, Ri2, NULL);
      ibiowave2(&NumLev, &Haar, &Edge, &FiltNorm, QWtrans, QImage, Ri, Ri2);
      fmse(Image, QImage, NULL, &PsnrFlg, &snr, &psnr, &mse, &mrd);
      printf("True PSNR = %.2f\n", psnr);	
    } else
      {
	if (FiltNorm == 1)
	  REFRESH_FILTERS(Ri, NULL, Edge_Ri);
	iowave2(&NumLev, &Haar, &Edge, &Precond, NULL, &FiltNorm, QWtrans, QImage, Ri, Edge_Ri);

	if (WeightFac || (FiltNorm == 1)) {
	  fmse(Image, QImage, NULL, &PsnrFlg, &snr, &psnr, &mse, &mrd);
	  printf("True PSNR = %.2f\n", psnr);	
	}
      }
  else
    QImage = NULL;
  
  if (QWtrans)
    mw_delete_wtrans2d(QWtrans);
  mw_delete_wtrans2d(Wtrans);
}





void
fwvq(NumRec, Edge_Ri, Ri2, FilterNorm, WeightFac, NumRecScal, NStep, MultiCB, CodeBook2, CodeBook3, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResResCodeBook1, ResResCodeBook2, DistRate, TargRate, Output, Image, CodeBook1, Ri, QImage)

	/*--- Computes the orthogonal wavelet transform of image `Image` ---*/

int        *NumRec;		/* Number of recursion (-j) */
Fimage	    Edge_Ri;		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
Fsignal     Ri2;		/* Impulse response of the low pass filter */
				/* for synthesis */
int        *FilterNorm;	        /* Equal 0 if no normalisation of filter's tap
			         *       1 if normalisation of the sum 
			         *       2 if normalistion of the square sum */
float      *WeightFac;          /* Weighting factor for wavelet coeff. */
int        *NumRecScal;         /* Scalar quantize wavelet subimages  
				 * at level NumRecScal and higher */
int        *NStep;              /* Number of steps in case of scalar 
				 * quantizarion of resume */
int        *MultiCB;            /* Not selected : one codebook per file
					 1: compute approx. dist-rate curves
					 2: compute exact dist-rate curves */
Fimage	    CodeBook2, CodeBook3; /* Sequence of codebooks for adaptive 
			         * quantization */
Fimage	    ResCodeBook1, ResCodeBook2, ResCodeBook3; /* Sequence of 
			         * codebooks for residu quantization  
				 * after quantization with CodeBook 
				 * AdapCodeBook2 and AdapCodeBook3 */
Fimage	    ResResCodeBook1;    /* Sequence of codebooks for residu 
				 * quantization after quantization 
				 * with CodeBook and ResCodeBook1 */
Fimage	    ResResCodeBook2;    /* Sequence of codebooks for residu 
				 * quantization after quantization 
				 * with AdapCodeBook2 and ResCodeBook2 */
int        *DistRate;           /* Compute distortion-rate function */
float      *TargRate;           /* Target bit rate */
Cimage      Output;		/* Compressed `Image` */
Fimage      Image;		/* Input image */
Fimage	    CodeBook1;          /* First sequence of codebooks for 
			         * for quantization */
Fsignal     Ri;			/* Impulse response of the low pass filter */
Fimage      QImage;		/* Output quantized image */

{
  Wtrans2d    WCodeBook1, WCodeBook2, WCodeBook3;  /* Sequence of codebooks 
                                 * extracted form CodeBook1, CodeBook2 
				 * and CodeBook3 */
  Wtrans2d    WResCodeBook1, WResCodeBook2, WResCodeBook3; /* Sequence of 
                                 * codebooks extracted form ResCodeBook1, 
				 * ResCodeBook2 and ResCodeBook3 */ 
  Wtrans2d    WResResCodeBook1, WResResCodeBook2; /* Sequence of 
                                 * codebooks extracted form ResResCodeBook1, 
				 * and ResResCodeBook2 */ 

  /*--- Modification of format for codebooks ---*/

  WCodeBook2 = WCodeBook3 = NULL;
  WResCodeBook1 = WResCodeBook2 = WResCodeBook3 = NULL;
  WResResCodeBook1 = WResResCodeBook2 = NULL;

  WCodeBook1 = mw_new_wtrans2d();
  FCB2WCB(CodeBook1, WCodeBook1);
  if (CodeBook2) {
    WCodeBook2 = mw_new_wtrans2d();
    FCB2WCB(CodeBook2, WCodeBook2);
  }
  if (CodeBook3) {
    WCodeBook3 = mw_new_wtrans2d();
    FCB2WCB(CodeBook3, WCodeBook3);
  }
  if (ResCodeBook1) {
    WResCodeBook1 = mw_new_wtrans2d();
    FCB2WCB(ResCodeBook1, WResCodeBook1);
  }
  if (ResCodeBook2) {
    WResCodeBook2 = mw_new_wtrans2d();
    FCB2WCB(ResCodeBook2, WResCodeBook2);
  }
  if (ResCodeBook3) {
    WResCodeBook3 = mw_new_wtrans2d();
    FCB2WCB(ResCodeBook3, WResCodeBook3);
  }
  if (ResResCodeBook1) {
    WResResCodeBook1 = mw_new_wtrans2d();
    FCB2WCB(ResResCodeBook1, WResResCodeBook1);
  }
  if (ResResCodeBook2) {
    WResResCodeBook2 = mw_new_wtrans2d();
    FCB2WCB(ResResCodeBook2, WResResCodeBook2);
  }

  /*--- Wavelet transform of the input image and vector quantization ---*/
                       /*--- of wavelet coefficients ---*/

  fwvq_wcb(NumRec, Edge_Ri, Ri2, FilterNorm, WeightFac, NumRecScal, NStep, MultiCB, WCodeBook2, WCodeBook3, WResCodeBook1, WResCodeBook2, WResCodeBook3, WResResCodeBook1, WResResCodeBook2, DistRate, TargRate, Output, Image, WCodeBook1, Ri, QImage);

}

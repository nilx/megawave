/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {fwivq};
version = {"2.0"};
author = {"Jean-Pierre D'Ales"};
function = {"Decompress an image compressed by vector quantization of its wavelet transform"};
usage = {
 'e':EdgeIR->Edge_Ri
      	"Impulse reponses of edge and preconditionning filters for orthogonal transform (fimage)",
 'b':ImpulseResponse2->Ri2
	"Impulse response of filter 2 for biorthogonal transform (fsignal)",
 'n':FilterNorm->FilterNorm [0,2]
	"Normalization mode for filter bank", 
 'w':WeightFac->WeightFac
        "Scaling factor for wavelet coefficients", 
 'x':CodeBook2->CodeBook2
	"Sequence of codebooks for second class (fimage)",
 'y':CodeBook3->CodeBook3
	"Sequence of codebooks for third class (fimage)",
 'A':ResCodeBook1->ResCodeBook1
	"Sequence of codebooks for residu quantization after quantization with CodeBook1 (fimage)",
 'B':ResCodeBook2->ResCodeBook2
	"Sequence of codebooks for residu quantization after quantization with CodeBook2 (fimage)",
 'C':ResCodeBook3->ResCodeBook3
	"Sequence of codebooks for residu quantization after quantization with CodeBook3 (fimage)",
 'D':ResResCodeBook1->ResResCodeBook1
	"Sequence of codebooks for residu quantization after quantization with CodeBook1 and ResCodeBook1 (fimage)",
 'E':ResResCodeBook2->ResResCodeBook2
	"Sequence of codebooks for residu quantization after quantization with CodeBook2 and ResCodeBook2 (fimage)",

 Cimage->Compress      "Input string of codewords (cimage)",
 CodeBook1->CodeBook1  "Sequence of codebooks for first class (fimage)", 
 ImpulseResponse->Ri   "Impulse response of inner filters (fsignal)", 
 Result<-Output        "Reconstructed image (fimage)"

};
*/


#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for iowave2(), ibiowave2(), fivq(), fiscalq() */

/*--- Constants ---*/

#define MAX_LEVEL 6              /* Maximum number of level in wav. transf. */ 
#define NORIENT 4                /* Number of orientation in wav. transf. */
#define NBIT_SIZEIM 15           /* Number of bits dedicated to encode 
				  * the dimensions of image */
#define NBIT_NLEVEL 4            /* Number of bits dedicated to encode the 
				  * number of levels in wavelet transf. */

static int      sizecomp;         /* Total number of 8 bits-codewords stored 
				   * in compress */
static int      ncwreadow;        /* Number of codewords stored in compress */
static int      bits_to_go;       /* Number of free bits in the currently 
				   * encoded codeword */
static int      buffer_wtrans;    /* Bits to encode in the currently 
				   * encoded codeword */
static unsigned char  *ptrc;      /* Pointer to compress->gray for next 
				   * codewords to encode */


static void
FCB2WCB(Fimage fcb, Wtrans2d wcb)
{
  int           nlevel;
  int           j;	        /* Scale index in wav. transf. */
  int           i;	      	/* Orientation index */
  long          r, c;
  long          nrow, ncol;
  long          nrowcb, ncolcb;
  int           width, height;  /* Height and width of vectors */
  long          incrc;

  nrow = ncol = 2;
  nlevel = (int) fcb->gray[0];
  for (j = 1; j < nlevel; j++) {
    nrow *= 2;
    ncol *= 2;
  }
  
  if (mw_alloc_ortho_wtrans2d(wcb, nlevel, nrow, ncol) == NULL)
    mwerror(FATAL, 1, "Allocation of wavelet transform for codebook refused!\n");  

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



static int 
READ_BIT_WTRANS(void)
{
  int           bit;              /* Value of read and returned bit */

  if (bits_to_go == 0) {
    if (ncwreadow == sizecomp) 
      mwerror(FATAL, 1, "Buffer ended to soon while reading header for wavelet transform!\n");
    else 
      {
	ptrc++;
	buffer_wtrans = *ptrc;
	bits_to_go = 8;
	ncwreadow += 1;
      }
  }

  bit = buffer_wtrans&1;
  buffer_wtrans >>= 1;
  bits_to_go -= 1;
  return bit;
}



static void
DECODE_INT_WTRANS(int *symb, int max)

                             /* Value of read symbol */
                             /* Half of maximum value for symbol */
  
{
  int bit;

  *symb = 0;
  while (max > 0) {
    *symb <<= 1;
    bit = READ_BIT_WTRANS();
    if (bit)
      *symb += 1;
    max /= 2;
  }
   
}




static void
READ_HEADER_WTRANS(int *nrow, int *ncol, int *nlevel, int (*scalvec)[4], Cimage compress)

                                 /* Size of image */
                                 /* Number of levels in wavelet transform */
                                         
                                 /* Compressed wavelet transform */

{
  int       i, j;
  int       n;
 
  /*--- Init decoding of compress ---*/

  sizecomp = compress->nrow * compress->ncol;
  ncwreadow = 1;
  ptrc = compress->gray;
  bits_to_go = 8;
  buffer_wtrans = *ptrc;

  /*--- Read size of image ---*/

  DECODE_INT_WTRANS(nrow, 1 << (NBIT_SIZEIM - 1));
  DECODE_INT_WTRANS(ncol, 1 << (NBIT_SIZEIM - 1));
  printf("Size of image : %d, %d\n", *nrow, *ncol);

  /*--- Read number of levels in wavelet transform ---*/

  DECODE_INT_WTRANS(nlevel, 1 << (NBIT_NLEVEL - 1));
  printf("Number of levels : %d\n", *nlevel);

  if ((*nlevel == 0) || (*nrow == 0) || (*ncol == 0))
    mwerror(FATAL, 4, "Bad value for size of image or number of levels!\n");

    /*--- Read info for scalar / vector quantization ---*/

  for (j = 1; j <= *nlevel; j++)
    for (i = 1; i <= 3; i++) {
      DECODE_INT_WTRANS(&n, 1);
      scalvec[j][i] = n;
    }

  DECODE_INT_WTRANS(&n, 1);
  scalvec[*nlevel][0] = n;

  if (bits_to_go == 0) 
    ncwreadow++;
  compress->firstrow = bits_to_go;
  compress->firstcol = ncwreadow;
}





static void
wivq_loc(Wtrans2d CodeBook2, Wtrans2d CodeBook3, Wtrans2d ResCodeBook1, Wtrans2d ResCodeBook2, Wtrans2d ResCodeBook3, Wtrans2d ResResCodeBook1, Wtrans2d ResResCodeBook2, Cimage Compress, Wtrans2d CodeBook1, Wtrans2d Output)

                                      /* Apply a weighting of coefficients 
				       * before quantization */
                                      /* Sequence of codebooks for adaptive 
				       * quantization */
                                                          /* Sequence of 
				       * codebooks for residu quantization  
				       * after quantization with CodeBook 
				       * AdapCodeBook2 and AdapCodeBook3 */
                                      /* Sequence of codebooks for residu 
				       * quantization after quantization 
				       * with CodeBook and ResCodeBook1 */
                                      /* Sequence of codebooks for residu 
				       * quantization after quantization 
				       * with AdapCodeBook2 and ResCodeBook2 */
                         	      /* Input compressed representation */
                                      /* First sequence of codebooks for 
				       * for quantization */
                                      /* Reconstructed wavelet transform */

{
  int             NRow, NCol;         /* Number of rows and columns in image */
  int             NLevel;             /* Number of levels in wav. transf. */
  int             J;	              /* Level index for subimages */
  int             i;                  /* Orientation index for subimages */
  int             Print;              /* Control of information print 
				       * in fivq */
  Fimage          ResCB, ResResCB;    /* Residual codebooks for average */
  Fimage          AdapCB2, AdapCB3;   /* Codebooks for adaptive quantization 
				       * of details */
  Fimage          ResCB1, ResCB2, ResCB3; /* Codebooks for residual 
				       * quantization  of detail */
  Fimage          ResResCB1, ResResCB2; /* Codebooks for residual quantization 
				       * of details (second level) */
  double          RateAr;             /* Bit rate for sub-images */
  int             ScalVec[MAX_LEVEL][NORIENT];


  /*--- Read information in heaeder of compressed file ---*/

  READ_HEADER_WTRANS(&NRow, &NCol, &NLevel, ScalVec, Compress); 

  /*--- Memory allocation for wavelet transform Output ---*/

  if (mw_alloc_ortho_wtrans2d(Output, NLevel, NRow, NCol) == NULL)
    mwerror(FATAL, 1, "Allocation of buffer for wavelet transform refused!");

  /*--- Reconstruction of details ---*/

  for (J = 1; J <= NLevel; J++) {
    
    NRow /= 2;
    NCol /= 2;
    
    for (i = 1; i <= 3; i++) {

      printf("Reconstruction of sub-image %d/%d\n", J, i);
      if (ScalVec[J][i] == 0)
	fiscalq(&Print, &NRow, &NCol, Compress, Output->images[J][i], &RateAr);
      else
	{

	  if (ResCodeBook1)
	    ResCB1 = ResCodeBook1->images[J][i];
	  else
	    ResCB1 = NULL;
	  if (ResResCodeBook1)
	    ResResCB1 = ResResCodeBook1->images[J][i];
	  else
	    ResResCB1 = NULL;
	  if (CodeBook2)
	    AdapCB2 = CodeBook2->images[J][i];
	  else
	    AdapCB2 = NULL;
	  if (ResCodeBook2)
	    ResCB2 = ResCodeBook2->images[J][i];
	  else
	    ResCB2 = NULL;
	  if (ResResCodeBook2)
	    ResResCB2 = ResResCodeBook2->images[J][i];
	  else
	    ResResCB2 = NULL;
	  if (CodeBook3)
	    AdapCB3 = CodeBook3->images[J][i];
	  else
	    AdapCB3 = NULL;
	  if (ResCodeBook3)
	    ResCB3 = ResCodeBook3->images[J][i];
	  else
	    ResCB3 = NULL;

	  fivq(&Print, &NRow, &NCol, AdapCB2, AdapCB3, NULL, ResCB1, ResCB2, ResCB3, NULL, ResResCB1, ResResCB2, Compress, CodeBook1->images[J][i], Output->images[J][i], &RateAr);
	}
    }
  }

    /*--- Reconstruction of average ---*/

  printf("Reconstruction of average sub-image\n");
  if (ScalVec[NLevel][0] == 0)
    fiscalq(&Print, &NRow, &NCol, Compress, Output->images[NLevel][0], &RateAr);
  else
    {

      if (ResCodeBook1)
	ResCB = ResCodeBook1->images[NLevel][0];
      else
	ResCB = NULL;
      if (ResResCodeBook1)
	ResResCB = ResResCodeBook1->images[NLevel][0];
      else
	ResResCB = NULL;
     
      fivq(&Print, &NRow, &NCol, NULL, NULL, NULL, ResCB, NULL, NULL, NULL, ResResCB, NULL, Compress, CodeBook1->images[NLevel][0], Output->images[NLevel][0], &RateAr);
    }
}



static void
fwivq_wcb(Wtrans2d CodeBook2, Wtrans2d CodeBook3, Wtrans2d ResCodeBook1, Wtrans2d ResCodeBook2, Wtrans2d ResCodeBook3, Wtrans2d ResResCodeBook1, Wtrans2d ResResCodeBook2, Fimage Edge_Ri, Fsignal Ri2, int *FilterNorm, Cimage Compress, Wtrans2d CodeBook1, Fimage Output, Fsignal Ri)

	/*--- Computes the orthogonal wavelet transform of image `Image` ---*/

                                  /* Sequence of codebooks for adaptive 
				 * quantization */
                                                      /* Sequence of codebooks 
			         * for residu quantization after quantization 
				 * with CodeBook AdapCodeBook2 
				 * and AdapCodeBook3 */
                                /* Sequence of codebooks for residu 
				 * quantization after quantization 
				 * with CodeBook and ResCodeBook1 */
                                /* Sequence of codebooks for residu 
				 * quantization after quantization 
				 * with AdapCodeBook2 and ResCodeBook2 */
      	            		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
                		/* Impulse response of the low pass filter */
				/* for synthesis */
                       	        /* Equal 0 if no normalisation of filter's tap
			         *       1 if normalisation of the sum 
			         *       2 if normalistion of the square sum */
                                /* Weighting factor for wavelet coeff. */
                     		/* Input compressed image */
                                /* First sequence of codebooks for 
				 * for quantization */
                   		/* Reconstructed image */
               			/* Impulse response of the low pass filter */

{
  int         J;       	        /* Current level of decomposition */
  Wtrans2d    QWtrans;          /* Reconstructed wavelet transform */
  int         Haar;             /* Continue decomposition with Haar wavelet
				 * until haar level */
  int         Edge;	        /* Edge processing mode */
  int         FiltNorm;	        /* Normalisation of filters */
  int         Precond;          /* Preconditionning mode for orthogonal 
				 * transform */

  
  /*--- Reconstruct wavelet transform ---*/

  QWtrans = mw_new_wtrans2d();

  wivq_loc(CodeBook2, CodeBook3, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResResCodeBook1, ResResCodeBook2, Compress, CodeBook1, QWtrans);

  /*--- Inverse wavelet transform ---*/
  
  J = Haar = QWtrans->nlevel;

  if (Ri2) {
    Edge = 2;
    if (FilterNorm)
      FiltNorm = *FilterNorm;
    else
      FiltNorm = 1;
    ibiowave2(&J, &Haar, &Edge, &FiltNorm, QWtrans, Output, Ri, Ri2);
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
      iowave2(&J, &Haar, &Edge, &Precond, NULL, &FiltNorm, QWtrans, Output, Ri, Edge_Ri);
    }

  /*--- Delete reconstructed wavelet transform ---*/

  mw_delete_wtrans2d(QWtrans);

}




void
fwivq(Fimage Edge_Ri, Fsignal Ri2, int *FilterNorm, float *WeightFac, Fimage CodeBook2, Fimage CodeBook3, Fimage ResCodeBook1, Fimage ResCodeBook2, Fimage ResCodeBook3, Fimage ResResCodeBook1, Fimage ResResCodeBook2, Cimage Compress, Fimage CodeBook1, Fsignal Ri, Fimage Output)

	/*--- Computes the orthogonal wavelet transform of image `Image` ---*/

      	            		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
                		/* Impulse response of the low pass filter */
				/* for synthesis */
                       	        /* Equal 0 if no normalisation of filter's tap
			         *       1 if normalisation of the sum 
			         *       2 if normalistion of the square sum */
                                /* Weighting factor for wavelet coeff. */
                                  /* Sequence of codebooks for adaptive 
				 * quantization */
      	                                              /* Sequence of codebooks 
			         * for residu quantization after quantization 
				 * with CodeBook AdapCodeBook2 
				 * and AdapCodeBook3 */
      	                        /* Sequence of codebooks for residu 
				 * quantization after quantization 
				 * with CodeBook and ResCodeBook1 */
      	                        /* Sequence of codebooks for residu 
				 * quantization after quantization 
				 * with AdapCodeBook2 and ResCodeBook2 */
                     		/* Input compressed image */
      	                        /* First sequence of codebooks for 
				 * for quantization */
                   		/* Reconstructed image */
               			/* Impulse response of the low pass filter */

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

  /* FIXME : unused parameter */
  WeightFac = WeightFac;

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

  /*--- Reconstruction of the image from the Compress file ---*/

  fwivq_wcb(WCodeBook2, WCodeBook3, WResCodeBook1, WResCodeBook2, WResCodeBook3, WResResCodeBook1, WResResCodeBook2, Edge_Ri, Ri2, FilterNorm, Compress, WCodeBook1, Output, Ri);

}

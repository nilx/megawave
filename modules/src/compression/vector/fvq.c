/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {fvq};
version = {"2.3"};
author = {"Jean-Pierre D'Ales"};
function = {"Performs the vector quantization of an image"};
usage = {
 'p':PrintSNR->PrintSNR    " 1 -> print info on classified VQ",
 'h'->SmallHeader          "Insert only a reduced header at top of Compress",
 'M'->BitMapCode           "Encode bitmap even if all codebooks have size 1",
 'm'->RateDist             "Compute rate distortion curve",
 'n':NCB1->NCB1            "Index of codebook in CodeBook1",
 'X':NCB2->NCB2            "Index of codebook in CodeBook2",
 'Y':NCB3->NCB3            "Index of codebook in CodeBook3",
 'Z':NCB4->NCB4            "Index of codebook in CodeBook4",
 'x':CodeBook2->CodeBook2  "Sequence of codebooks for second class (fimage)",
 'y':CodeBook3->CodeBook3  "Sequence of codebooks for third class (fimage)",
 'z':CodeBook4->CodeBook4  "Sequence of codebooks for fourth class (fimage)",
 'A':NResCB1->NResCB1      "Index of codebook in ResCodeBook1",
 'B':NResCB2->NResCB2      "Index of codebook in ResCodeBook2",
 'C':NResCB3->NResCB3      "Index of codebook in ResCodeBook3",
 'D':NResCB4->NResCB4      "Index of codebook in CodeBook4",
 'a':ResCodeBook1->ResCodeBook1  "Codebook for residu quantization after quantization with CodeBook1 (fimage)",
 'b':ResCodeBook2->ResCodeBook2  "Codebook for residu quantization after quantization with CodeBook2 (fimage)",
 'c':ResCodeBook3->ResCodeBook3  "Codebook for residu quantization after quantization with CodeBook3 (fimage)",
 'd':ResCodeBook4->ResCodeBook4  "Codebook for residu quantization after quantization with CodeBook4 (fimage)",
 'E':NResResCB1->NResResCB1      "Index of codebook in ResResCodeBook1",
 'F':NResResCB2->NResResCB2      "Index of codebook in ResResCodeBook2",
 'e':ResResCodeBook1->ResResCodeBook1  "Codebook for residu quantization after quantization with CodeBook1 and ResCodeBook1 (fimage)",
 'f':ResResCodeBook2->ResResCodeBook2  "Codebook for residu quantization after quantization with CodeBook2 and ResCodeBook2 (fimage)",
 'o':Compress<-Compress    "Compressed representation of Image (cimage)",
 Image->Image              "Input Fimage (fimage)", 
 CodeBook1->CodeBook1      "Sequence of codebooks for first class (fimage)", 
 QImage<-Result            "Quantized image (fimage)",
   { 
     MSE<-MSE              "MSE", 
     SNR<-SNR              "SNR",
     Entropy<-Entropy      "Entropy",
     RateAr<-RateAr        "RateAr"
   }
};
 */
/*----------------------------------------------------------------------
 v2.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include  "mw.h"

/*--- Megawave2 modules definition ---*/

extern void arencode2();
extern void entropy();

/*--- Constants ---*/

#define MAX_CB 50                /* Maximum number of codebooks for 
				  * a given sub-image and a given class */
#define MAX_ADAP 4               /* Maximum number of classes */
#define NBIT_SIZEIM 15           /* Number of bits dedicated to encode 
				  * the dimensions of image */
#define MAX_SIZEIM ((long) (1 << NBIT_SIZEIM) - 1)  /* Maximum number of line 
				  * and column for images */

/*--- Global variables ---*/

typedef float   bufmse[MAX_ADAP][MAX_CB];
typedef int     bufind[MAX_ADAP];

static double   var, energy;      /* Variance and energy of image */
static long     height, width;    /* Height and width of vectors */
static float    targrate_dr[50];  /* Target rates for dist-rate curve */
static int      max_count_dr;     /* Number of points for dist-rate curve */
static long     effnbit;          /* Total number of bits stored in compress */
static int      ncodewords;       /* Number of codewords stored in compress */
static int      bits_to_go;       /* Number of free bits in the currently 
				   * encoded codeword */
static int      buffer;           /* Bits to encode in the currently 
				   * encoded codeword */
static unsigned char *ptrc;       /* Pointer to compress->gray for next 
				   * codewords to encode */
static int      nadapcb;          /* Number of levels for adaptive 
				   * quantization */


static void
CHECK_INPUT(ratedist, ncb1, codebook1, codebook2, codebook3, codebook4, nrescb1, rescodebook1, rescodebook2, rescodebook3, rescodebook4, nresrescb1, resrescodebook1, resrescodebook2, image)

int             ratedist;
int            *ncb1;
Fimage          codebook1, codebook2, codebook3, codebook4; /* Codebooks for 
                                 * different classes */
int            *nrescb1;
Fimage          rescodebook1, rescodebook2, rescodebook3, rescodebook4; 
                                /* Residu codebooks for different classes */
int            *nresrescb1;
Fimage          resrescodebook1, resrescodebook2; /* Residu codebooks 
				 * for different classes */
Fimage          image;          /* Input image */

{
  int           sizeb;
  float         testsize;           /* Control for codebook structure */  

  /*--- Test dimensions of vectors and size of codebook files ---*/ 

  if (codebook1->nrow <= 4) 
    mwerror(FATAL, 2, "Bad number (<= 4) of row in CodeBook1!\n");

  sizeb = codebook1->ncol;
  height = floor(codebook1->gray[(codebook1->nrow - 2) * sizeb] + .5);
  if (((float) height != codebook1->gray[(codebook1->nrow - 2) * sizeb]) || (height <= 0))
    mwerror(WARNING, 0, "Bad value for height of vectors in CodeBook1!\n");
  if (sizeb % height != 0)
    mwerror(FATAL, 2, "Dimension and height of vector are incompatible!");
  width = sizeb / height;

  if (codebook2) {
    if (codebook2->nrow <= 4) 
      mwerror(FATAL, 2, "Bad number (<= 4) of row in CodeBook2!\n");
    if (codebook2->ncol != sizeb)
      mwerror(FATAL, 2, "Numbers of columns are different in CodeBook1 and CodeBook2!\n");
    if ((float) height != codebook2->gray[(codebook2->nrow - 2) * sizeb])
      mwerror(WARNING, 0, "Bad value for height of vectors in CodeBook2!\n");
  }

  if (codebook3) {
    if (codebook3->nrow <= 4) 
      mwerror(FATAL, 2, "Bad number (<= 4) of row in CodeBook3!\n");
    if (codebook3->ncol != sizeb)
      mwerror(FATAL, 2, "Numbers of columns are different in CodeBook1 and CodeBook3!\n");
    if ((float) height != codebook3->gray[(codebook3->nrow - 2) * sizeb])
      mwerror(WARNING, 0, "Bad value for height of vectors in CodeBook3!\n");
  }

  if (codebook4) {
    if (codebook4->nrow <= 4) 
      mwerror(FATAL, 2, "Bad number (<= 4) of row in CodeBook4!\n");
    if (codebook4->ncol != sizeb)
      mwerror(FATAL, 2, "Numbers of columns are different in CodeBook1 and CodeBook4!\n");
    if ((float) height != codebook4->gray[(codebook4->nrow - 2) * sizeb])
      mwerror(WARNING, 0, "Bad value for height of vectors in CodeBook4!\n");
  }

  if (rescodebook1) {
    if (rescodebook1->nrow <= 4) 
      mwerror(FATAL, 2, "Bad number (<= 4) of row in ResCodeBook1!\n");
    if (rescodebook1->ncol != sizeb)
      mwerror(FATAL, 2, "Numbers of columns are different in ResCodeBook1 and CodeBook1!\n");
    if ((float) height != rescodebook1->gray[(rescodebook1->nrow - 2) * sizeb])
      mwerror(WARNING, 0, "Bad value for height of vectors in ResCodeBook1!\n");
  }

  if (rescodebook2) {
    if (rescodebook2->nrow <= 4) 
      mwerror(FATAL, 2, "Bad number (<= 4) of row in ResCodeBook2!\n");
    if (rescodebook2->ncol != sizeb)
      mwerror(FATAL, 2, "Numbers of columns are different in CodeBook1 and ResCodeBook2!\n");
    if ((float) height != rescodebook2->gray[(rescodebook2->nrow - 2) * sizeb])
      mwerror(WARNING, 0, "Bad value for height of vectors in ResCodeBook2!\n");
  }

  if (rescodebook3) {
    if (rescodebook3->nrow <= 4) 
      mwerror(FATAL, 2, "Bad number (<= 4) of row in ResCodeBook3!\n");
    if (rescodebook3->ncol != sizeb)
      mwerror(FATAL, 2, "Numbers of columns are different in CodeBook1 and ResCodeBook3!\n");
    if ((float) height != rescodebook3->gray[(rescodebook3->nrow - 2) * sizeb])
      mwerror(WARNING, 0, "Bad value for height of vectors in ResCodeBook3!\n");
  }

  if (rescodebook4) {
    if (rescodebook4->nrow <= 4) 
      mwerror(FATAL, 2, "Bad number (<= 4) of row in ResCodeBook4!\n");
    if (rescodebook4->ncol != sizeb)
      mwerror(FATAL, 2, "Numbers of columns are different in CodeBook1 and ResCodeBook4!\n");
    if ((float) height != rescodebook4->gray[(rescodebook4->nrow - 2) * sizeb])
      mwerror(WARNING, 0, "Bad value for height of vectors in ResCodeBook4!\n");
  }

  if (resrescodebook1) {
    if (resrescodebook1->nrow <= 4) 
      mwerror(FATAL, 2, "Bad number (<= 4) of row in ResResCodeBook1!\n");
    if (resrescodebook1->ncol != sizeb)
      mwerror(FATAL, 2, "Numbers of columns are different in ResResCodeBook1 and CodeBook1!\n");
    if ((float) height != resrescodebook1->gray[(resrescodebook1->nrow - 2) * sizeb])
      mwerror(WARNING, 0, "Bad value for height of vectors in ResResCodeBook1!\n");
  }

  if (resrescodebook2) {
    if (resrescodebook2->nrow <= 4) 
      mwerror(FATAL, 2, "Bad number (<= 4) of row in ResResCodeBook2!\n");
    if (resrescodebook2->ncol != sizeb)
      mwerror(FATAL, 2, "Numbers of columns are different in CodeBook1 and ResResCodeBook2!\n");
    if ((float) height != resrescodebook2->gray[(resrescodebook2->nrow - 2) * sizeb])
      mwerror(WARNING, 0, "Bad value for height of vectors in ResResCodeBook2!\n");
  }

  /*--- Test if codebook files contain one or several codebooks ---*/ 

  if (ratedist || ncb1 || nrescb1 || nresrescb1) {
    if (codebook1->nrow <= 6) 
      mwerror(FATAL, 2, "Bad number (<= 6) of row in CodeBook1!\n");

    testsize = (float) floor(codebook1->gray[(codebook1->nrow - 6) *
					     sizeb] + .5);
    if ((codebook1->gray[(codebook1->nrow - 6) * sizeb] != testsize) || (testsize < 1.0)) 
      mwerror(FATAL, 2, "Bad final size in CodeBook1!\n");

    testsize = (float) floor(codebook1->gray[(codebook1->nrow - 5) *
					     sizeb] + .5);
    if ((codebook1->gray[(codebook1->nrow - 5) * sizeb] != testsize) || (testsize < 1.0)) 
      mwerror(FATAL, 2, "Bad initial size in CodeBook1!\n");

    if (codebook2) {
      if (codebook2->nrow <= 6) 
	mwerror(FATAL, 2, "Bad number (<= 6) of row in CodeBook2!\n");

      testsize = (float) floor(codebook2->gray[(codebook2->nrow - 6) *
					       sizeb] + .5);
      if ((codebook2->gray[(codebook2->nrow - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 0, "Bad final size in CodeBook2!\n");

      testsize = (float) floor(codebook2->gray[(codebook2->nrow - 5) *
					       sizeb] + .5);
      if ((codebook2->gray[(codebook2->nrow - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(WARNING, 0, "Bad initial size in CodeBook2!\n");
    }

    if (codebook3) {
      if (codebook3->nrow <= 6) 
	mwerror(FATAL, 2, "Bad number (<= 6) of row in CodeBook3!\n");

      testsize = (float) floor(codebook3->gray[(codebook3->nrow - 6) *
					       sizeb] + .5);
      if ((codebook3->gray[(codebook3->nrow - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 0, "Bad final size in CodeBook3!\n");

      testsize = (float) floor(codebook3->gray[(codebook3->nrow - 5) *
					       sizeb] + .5);
      if ((codebook3->gray[(codebook3->nrow - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(WARNING, 0, "Bad initial size in CodeBook3!\n");
    }

    if (codebook4) {
      if (codebook4->nrow <= 6) 
	mwerror(FATAL, 2, "Bad number (<= 6) of row in CodeBook4!\n");

      testsize = (float) floor(codebook4->gray[(codebook4->nrow - 6) *
					       sizeb] + .5);
      if ((codebook4->gray[(codebook4->nrow - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 0, "Bad final size in CodeBook4!\n");

      testsize = (float) floor(codebook4->gray[(codebook4->nrow - 5) *
					       sizeb] + .5);
      if ((codebook4->gray[(codebook4->nrow - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(WARNING, 0, "Bad initial size in CodeBook4!\n");
    }

    if (rescodebook1) {
      if (rescodebook1->nrow <= 6) 
	mwerror(FATAL, 2, "Bad number (<= 6) of row in ResCodeBook1!\n");

      testsize = (float) floor(rescodebook1->gray[(rescodebook1->nrow
						   - 6) * sizeb] + .5);
      if ((rescodebook1->gray[(rescodebook1->nrow - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 2, "Bad final size in ResCodeBook1!\n");

      testsize = (float) floor(rescodebook1->gray[(rescodebook1->nrow
						   - 5) * sizeb] + .5);
      if ((rescodebook1->gray[(rescodebook1->nrow - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 2, "Bad initial size in ResCodeBook1!\n");
    }

    if (rescodebook2) {
      if (rescodebook2->nrow <= 6) 
	mwerror(FATAL, 2, "Bad number (<= 6) of row in ResCodeBook2!\n");

      testsize = (float) floor(rescodebook2->gray[(rescodebook2->nrow
						   - 6) * sizeb] + .5);
      if ((rescodebook2->gray[(rescodebook2->nrow - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 0, "Bad final size in ResCodeBook2!\n");

      testsize = (float) floor(rescodebook2->gray[(rescodebook2->nrow
						   - 5) * sizeb] + .5);
      if ((rescodebook2->gray[(rescodebook2->nrow - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(WARNING, 0, "Bad initial size in ResCodeBook2!\n");
    }

    if (rescodebook3) {
      if (rescodebook3->nrow <= 6) 
	mwerror(FATAL, 2, "Bad number (<= 6) of row in ResCodeBook3!\n");

      testsize = (float) floor(rescodebook3->gray[(rescodebook3->nrow
						   - 6) * sizeb] + .5);
      if ((rescodebook3->gray[(rescodebook3->nrow - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 0, "Bad final size in ResCodeBook3!\n");

      testsize = (float) floor(rescodebook3->gray[(rescodebook3->nrow
						   - 5) * sizeb] + .5);
      if ((rescodebook3->gray[(rescodebook3->nrow - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(WARNING, 0, "Bad initial size in ResCodeBook3!\n");
    }

    if (rescodebook4) {
      if (rescodebook4->nrow <= 6) 
	mwerror(FATAL, 2, "Bad number (<= 6) of row in ResCodeBook4!\n");

      testsize = (float) floor(rescodebook4->gray[(rescodebook4->nrow
						   - 6) * sizeb] + .5);
      if ((rescodebook4->gray[(rescodebook4->nrow - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 0, "Bad final size in ResCodeBook4!\n");

      testsize = (float) floor(rescodebook4->gray[(rescodebook4->nrow
						   - 5) * sizeb] + .5);
      if ((rescodebook4->gray[(rescodebook4->nrow - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(WARNING, 0, "Bad initial size in ResCodeBook4!\n");
    }

    if (resrescodebook1) {
      if (resrescodebook1->nrow <= 6) 
	mwerror(FATAL, 2, "Bad number (<= 6) of row in ResResCodeBook1!\n");

      testsize = (float)
      floor(resrescodebook1->gray[(resrescodebook1->nrow - 6) * sizeb]
	    + .5);
      if ((resrescodebook1->gray[(resrescodebook1->nrow - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 2, "Bad final size in ResResCodeBook1!\n");

      testsize = (float)
      floor(resrescodebook1->gray[(resrescodebook1->nrow - 5) * sizeb]
	    + .5);
      if ((resrescodebook1->gray[(resrescodebook1->nrow - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 2, "Bad initial size in ResResCodeBook1!\n");
    }

    if (resrescodebook2) {
      if (resrescodebook2->nrow <= 6) 
	mwerror(FATAL, 2, "Bad number (<= 6) of row in ResResCodeBook2!\n");

      testsize = (float)
	   floor(resrescodebook2->gray[(resrescodebook2->nrow - 6) * sizeb]
		 + .5);
      if ((resrescodebook2->gray[(resrescodebook2->nrow - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(FATAL, 0, "Bad final size in ResResCodeBook2!\n");

      testsize = (float)
      floor(resrescodebook2->gray[(resrescodebook2->nrow - 5) * sizeb]
	    + .5);
      if ((resrescodebook2->gray[(resrescodebook2->nrow - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	mwerror(WARNING, 0, "Bad initial size in ResResCodeBook2!\n");
    }
  }

  /*---Check Threshold value in codebook files ---*/

  if (codebook1->gray[(codebook1->nrow - 3) * sizeb] < 0.0)
    mwerror(WARNING, 0, "Negative value for threshold in CodeBook1!\n");
  if (codebook2) {
    if (codebook2->gray[(codebook2->nrow - 4) * sizeb] < 0.0)
      mwerror(WARNING, 0, "Negative value for upper threshold in CodeBook2!\n");
    if (codebook2->gray[(codebook2->nrow - 3) * sizeb] < 0.0)
      mwerror(WARNING, 0, "Negative value for lower threshold in CodeBook2!\n");
    if (codebook2->gray[(codebook2->nrow - 4) * sizeb] != codebook1->gray[(codebook1->nrow - 3) * sizeb])
      mwerror(WARNING, 0, "Lower and upper threshold not equal in CodeBook1 and CodeBook2!\n");

    if (codebook3) {
      if (codebook3->gray[(codebook3->nrow - 4) * sizeb] < 0.0)
	mwerror(WARNING, 0, "Negative value for upper threshold in CodeBook3!\n");
      if (codebook3->gray[(codebook3->nrow - 3) * sizeb] < 0.0)
	mwerror(WARNING, 0, "Negative value for lower threshold in CodeBook3!\n");
      if (codebook3->gray[(codebook3->nrow - 4) * sizeb] != codebook2->gray[(codebook2->nrow - 3) * sizeb])
	mwerror(WARNING, 0, "Lower and upper threshold not equal in CodeBook2 and CodeBook3!\n");

      if (codebook4) {
	if (codebook4->gray[(codebook4->nrow - 4) * sizeb] < 0.0)
	  mwerror(WARNING, 0, "Negative value for upper threshold in CodeBook4!\n");
	if (codebook4->gray[(codebook4->nrow - 3) * sizeb] < 0.0)
	  mwerror(WARNING, 0, "Negative value for lower threshold in CodeBook4!\n");
	if (codebook4->gray[(codebook4->nrow - 4) * sizeb] != codebook3->gray[(codebook3->nrow - 3) * sizeb])
	  mwerror(WARNING, 0, "Lower and upper threshold not equal in CodeBook3 and CodeBook4!\n");
      }
    }
  }

  /*--- Check compatiblity of image and vector sizes ---*/

  if ((image->nrow % height != 0) || (image->ncol % width != 0))
    mwerror(FATAL, 2, "Image dimensions are not multiples of vector dimensions!\n");
    
  /*--- Check size of image ---*/

  if ((image->nrow > MAX_SIZEIM) || (image->ncol > MAX_SIZEIM))
    mwerror(FATAL, 2, "Image dimensions are too large!\n");

}



static void
INIT_TARGRATE_DR()

{

  targrate_dr[0] = 0.008;
  targrate_dr[1] = 0.016;
  targrate_dr[2] = 0.020;
  targrate_dr[3] = 0.024;
  targrate_dr[4] = 0.028;
  targrate_dr[5] = 0.032;
  targrate_dr[6] = 0.036;
  targrate_dr[7] = 0.040;
  targrate_dr[8] = 0.044;
  targrate_dr[9] = 0.048;
  targrate_dr[10] = 0.056;
  targrate_dr[11] = 0.064;
  targrate_dr[12] = 0.072;
  targrate_dr[13] = 0.08;
  targrate_dr[14] = 0.09;
  targrate_dr[15] = 0.1;
  targrate_dr[16] = 0.11;
  targrate_dr[17] = 0.125;
  targrate_dr[18] = 0.15;
  targrate_dr[19] = 0.175;
  targrate_dr[20] = 0.2;
  targrate_dr[21] = 0.225;
  targrate_dr[22] = 0.25;
  targrate_dr[23] = 0.275;
  targrate_dr[24] = 0.3;
  targrate_dr[25] = 0.325;
  targrate_dr[26] = 0.35;
  targrate_dr[27] = 0.375;
  targrate_dr[28] = 0.4;
  targrate_dr[29] = 0.45;
  targrate_dr[30] = 0.5;
  targrate_dr[31] = 0.55;
  targrate_dr[32] = 0.60;
  targrate_dr[33] = 0.7;
  targrate_dr[34] = 0.8;
  targrate_dr[35] = 1.0;
  targrate_dr[36] = 1.25;
  targrate_dr[37] = 1.5;
  targrate_dr[38] = 1.75;
  targrate_dr[39] = 2.0;
  targrate_dr[40] = 2.25;
  targrate_dr[41] = 2.5;
  targrate_dr[42] = 3.0;
  targrate_dr[43] = 3.5;
  targrate_dr[44] = 4.0;
  targrate_dr[45] = 4.5;
  targrate_dr[46] = 5.0;
  targrate_dr[47] = 5.5;
  targrate_dr[48] = 6.0;

  max_count_dr = 48;

}


static void
clear_histo(histo)

Fsignal histo;

{
  int i;

  for (i=0;i<histo->size;i++)
    histo->values[i]=0.0;
}




static void
Variance(image)

Fimage          image;
    
{
  int             r, c;
  long	rdx;
  long	dx, dy;			/* Size of image */
  double	mean;		/* Mean value of `Img1` */
  double        f, mean2;

  dx = image->ncol;
  dy = image->nrow;

  mean = 0.0;
  rdx = 0;
  for(r = 0; r < dy; r++) {
    for(c = 0; c < dx; c++)
      mean += image->gray[rdx + c];
    rdx += dx;
  }
  mean /= (double) dx * dy;
  mean2 = mean * mean;

  var = energy = 0.0;
  rdx = 0;
  for(r = 0; r < dy; r++) {
    for(c = 0; c < dx; c++) {
      f = image->gray[rdx + c] * image->gray[rdx + c];
      energy += f;
      var += f;
    }
    rdx += dx;
  }
  var /= (double) dx * dy;
  energy /= (double) dx * dy;
  var -= mean2;
}



static void
RESIZE_COMPRESS_FIMAGE(compress)

Cimage           compress;

{
  int              i;
  int              ncolo, nrowo;
  long             size;
  int              mindif;

  if (bits_to_go < 8) {
    *ptrc = buffer >> bits_to_go;
    ncodewords += 1;
  } else
    bits_to_go = 0;

  ncolo = 1;
  nrowo = ncodewords;
  if (nrowo > MAX_SIZEIM) {
    if ((int) sqrt((double) nrowo) + 1 > MAX_SIZEIM)
      mwerror(FATAL, 2, "Number of codewords is too large!\n");
    i = 2;
    while (nrowo / i > MAX_SIZEIM)
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
	  if (nrowo / i <= MAX_SIZEIM) 
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

  if (compress->lastrow == 0)
    compress = mw_change_cimage(compress, nrowo, ncolo);
  compress->cmt[0]='\0';
  
  compress->firstrow = bits_to_go;
  compress->firstcol = ncodewords;
}


static void
REALLOCATE_COMPRESS_FIMAGE(compress)

Cimage           compress;

{
  int              i;
  Cimage           bufcomp;
  long             size;

  size = compress->ncol * compress->nrow;
  printf("Reallocation of Compress for fimage vector quantization.\n");

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
ADD_BIT_TO_COMPRESS_FIMAGE(bit, compress)

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
      REALLOCATE_COMPRESS_FIMAGE(compress);
    ptrc++;
    bits_to_go = 8;
    buffer = 0;
  }
}



static void
ENCODE_INT_FIMAGE(symb, max, compress)

int            symb;         /* Value of symbol to write */
int            max;          /* Half of maximum value for symbol */
Cimage         compress;     /* Compressed file */

{

  while (max > 0) {
    if (symb >= max) {
      ADD_BIT_TO_COMPRESS_FIMAGE(1, compress);
      symb = symb % max;
    } else
      ADD_BIT_TO_COMPRESS_FIMAGE(0, compress);
    max /= 2;
  }

}




static void
INIT_ENCODING_FIMAGE(smallheader, nrow, ncol, testmulticb, indexcb, compress)

int         *smallheader;        /* Put size of image in header iff NULL */ 
int          nrow, ncol;         /* Size of image */
int          testmulticb;        /* Control for muliple codebooks per class */
bufind       indexcb;            /* Indices of selected codebooks 
				  * for quantization */
Cimage       compress;           /* Compressed file */

{
  int     n;

  effnbit = 4;
  if (testmulticb == 1)
    effnbit += nadapcb * 6;
  if (!smallheader) 
    effnbit += 2 * NBIT_SIZEIM;

  if (compress) {

    /*--- Init encoding ---*/

    if (compress->gray) {

      bits_to_go = compress->firstrow;
      ncodewords = compress->firstcol;
      if ((bits_to_go < 0) || (bits_to_go >= 8))
	mwerror(FATAL, 4, "Wrong value for compress->firstrow! (%d)\n", bits_to_go);
      if (bits_to_go == 0) {
	ptrc = compress->gray + ncodewords;
	buffer = 0;
	bits_to_go = 8;
      } else
	{
	  ncodewords--;
	  ptrc = compress->gray + ncodewords;
	  buffer = *ptrc << bits_to_go;
	}
    } else
      {

	/*--- Memory allocation for compressed image buffer ---*/

	compress = mw_change_cimage(compress, nrow,  ncol);
	if (compress == NULL)
	  mwerror(FATAL, 1, "Memory allocation refused for `Compress`!\n");

	ptrc = compress->gray;
	bits_to_go = 8;
	buffer = 0;
	ncodewords = 0;
      } 

    /*--- Encode header ---*/

    if (!smallheader) {

      /*--- Encode size of image ---*/

      ENCODE_INT_FIMAGE(nrow, 1 << (NBIT_SIZEIM - 1), compress);
      ENCODE_INT_FIMAGE(ncol, 1 << (NBIT_SIZEIM - 1), compress);
    }

    /*--- Encode number of classes in vector quantization ---*/

    ENCODE_INT_FIMAGE(nadapcb, 4, compress);

    /*--- Check if there are different codebooks for each class ---*/

    ENCODE_INT_FIMAGE(testmulticb, 1, compress);

    if (testmulticb == 1) 
      
      /*--- Encode index of codebook for each class ---*/

      for (n = 0; n < nadapcb; n++)
	ENCODE_INT_FIMAGE(indexcb[n], 32, compress);
    
  } 

}




static void
ADD_BUF_TO_COMPRESS_FIMAGE(compress, bufcomp)

Cimage          compress;       /* Compressed image */
Cimage          bufcomp;        /* Buffer for compressed image */

{
  int bit;
  int b;
  int buf;
  int c, size;

  size = bufcomp->nrow * bufcomp->ncol;

  for (c = 0; c < size - 1; c++) {
    buf = bufcomp->gray[c];
    for (b = 0; b <= 7; b++) {
      bit = buf&1;
      buf >>= 1;
      ADD_BIT_TO_COMPRESS_FIMAGE(bit, compress);
      effnbit++;
    }
  }

  buf = bufcomp->gray[size - 1];
  for (b = 0; b < bufcomp->firstrow; b++) {
    bit = buf&1;
    buf >>= 1;
    ADD_BIT_TO_COMPRESS_FIMAGE(bit, compress);
    effnbit++;
  }
}






static void
compute_rates(symbol, sizec, recfl, histo, rate, ratearc, ent, compress)

Fimage          symbol;
int             sizec;
int            *recfl;
Fsignal         histo;
double         *rate;
double         *ratearc;
double	       *ent;
Cimage          compress;       /* Compressed image */

{
  double          e;            /* Rate with arithmetic coding */
  double          er;           /* Rate with no coding */
  int            *predic;       /* Control predicitive coding */
  int             nsymb;
  int             size;
  Cimage          bufcomp;             /* Buffer for compressed image */

  size = symbol->nrow * symbol->ncol;
  entropy(histo, &e);
  *ent += e * size;

  if (compress)
    bufcomp = mw_new_cimage();
  else
    bufcomp = NULL;

  nsymb = sizec;
  if (sizec <= 32)
    predic = &nsymb;
  else
    predic = NULL;

  if (sizec > 1)
    arencode2(&sizec, NULL, &nsymb, NULL, predic, NULL, NULL, symbol, &e, bufcomp);
  else
    e = 0.0;

  er = log10((double) sizec) / log10((double) 2.0);
  if (e > er)
    *ratearc += er * size;
  else
    *ratearc += e * size;
    
  *rate += log10((double) sizec) * size;

  if (compress && (sizec > 1))
    ADD_BUF_TO_COMPRESS_FIMAGE(compress, bufcomp);
}



static void
extract_symbol(symbol, symbex, indcb, index, size, recfl, recfac)

Fimage          symbol, symbex;
Fimage          indcb;
float           index;
long            size;
int            *recfl;
int             recfac;

{
  register float  *ptrs, *ptrse;
  long             sizet;
  long             x;
  long             sizecheck;

  sizet = symbol->nrow * symbol->ncol;

  symbex = mw_change_fimage(symbex, 1, size);
  if (symbex == NULL)
    mwerror(FATAL, 1, "Not enough memory for symbol buffer\n");

  ptrs = symbol->gray;
  ptrse = symbex->gray;
  if (indcb) {
    sizecheck = 0;
    for (x = 0; x < sizet; x++, ptrs++)
      if (indcb->gray[x] == index) {
	*ptrse = *ptrs;
	ptrse++;
	sizecheck++;
      }
    if (sizecheck != size)
      mwerror(WARNING, 0, "Something wrong with indcb.\n");
  } else
    for (x = 0; x < sizet; x++, ptrs++, ptrse++)
      *ptrse = *ptrs;

  ptrse = symbex->gray;

  if (recfac > 0)
    if (recfl)
      for (x = 0; x < size; x++, ptrse++)
	*ptrse = (float) ((int) floor(*ptrse + .5) / recfac);
    else
      for (x = 0; x < size; x++, ptrse++)
	*ptrse = (float) ((int) floor(*ptrse + .5) % recfac);
    
}



static void
indexcb_adap(indexcb, nadap, qprec, numcb, bmse, effmse, minmse, brate, brl, effrate, targrate, testopt, opt)

bufind    indexcb;               /* Indices of selected codebooks 
				  * for quantization */
int       nadap;
int       qprec;
bufind    numcb;
bufmse    bmse;
float     effmse, *minmse;
bufmse    brate;
float     brl[MAX_ADAP];
float     effrate, targrate;
int      *testopt, opt;

{
  int      q;
  float    neweffrate, neweffmse;

  if (nadap == nadapcb - 1) {
    for (q = numcb[nadap] - 1; q >= 0; q--) {
      neweffmse = effmse + bmse[nadap][q];
      neweffrate = effrate + brate[nadap][q];
      if ((q > 0) && (qprec == 0))
	neweffrate += brl[nadap - 1];
      if ((neweffrate <= targrate) && (neweffmse < *minmse)) {
	*testopt = opt - 1;
	indexcb[nadap] = q;
	*minmse = neweffmse;
      }
    }
  } else
    {
      for (q = numcb[nadap] - 1; q >= 0; q--) {
	neweffmse = effmse + bmse[nadap][q]; 
	neweffrate = effrate + brate[nadap][q]; 
	if ((q > 0) && (qprec == 0))
	  neweffrate += brl[nadap - 1];
	if ((neweffrate <= targrate) && (neweffmse < *minmse)) {
	  indexcb_adap(indexcb, nadap + 1, q, numcb, bmse, neweffmse, minmse, brate, brl, neweffrate, targrate, testopt, opt + 1);
	  if (*testopt == opt) {
	    (*testopt)--;
	    indexcb[nadap] = q;
	  }
	}
      }
    }
  
}



static void
compute_indexcb(indexcb, numcb, bmse, brate, brl, targrate, test_dr)

bufind    indexcb;
bufind    numcb;
bufmse    bmse;
bufmse    brate;
float     brl[MAX_ADAP];
float     targrate;
int       test_dr;

{
  int      q;
  int      nadap;
  float    effrate;
  float    effmse, minmse;
  int      testopt, opt;

  for (nadap = 0; nadap < nadapcb; nadap++) 
    indexcb[nadap] = 0;

  minmse = 0.0;
  for (nadap = 0; nadap < nadapcb; nadap++) 
    minmse += bmse[nadap][indexcb[nadap]];
  nadap = 0;
  testopt = 0;
  opt = 1;
  for (q = numcb[0] - 1; q >= 0; q--) {
    effmse = bmse[0][q];
    effrate = brate[0][q];
    if (nadapcb == 1) {
      if ((effrate <= targrate) && (effmse < minmse)) {
	indexcb[nadap] = q;
	minmse = effmse;
      }
    } else
      {
	indexcb_adap(indexcb, nadap + 1, q, numcb, bmse, effmse, &minmse, brate, brl, effrate, targrate, &testopt, opt + 1);
	if (testopt == opt) {
	  indexcb[nadap] = q;
	  testopt--;
	}
      }
  }


  if (test_dr == 1) {
    for (nadap = 0; nadap < nadapcb; nadap++) 
      printf("%2d ", indexcb[nadap]);
    printf("\n");
  }
}




static int
count_cb(codebook)

Fimage     codebook;

{
  long      size, sizei, sizef;
  int      n;

  if (codebook) {
    sizei = floor(codebook->gray[(codebook->nrow - 5) *
				 codebook->ncol] + .5);
    sizef = floor(codebook->gray[(codebook->nrow - 6) *
				 codebook->ncol] + .5);
    n = 2;
    size = 1;
    while (size <= sizei)
      size *= 2;  
    while (size < sizef) {
      size *= 2;
      n++;
    }
  } else
    n = 0;

  return(n);
}




static void
extract_cb(codebook, cb, n)

Fimage     codebook, cb;
int        n;

{
  long      size, sizei, sizef;
  long     xshift;
  long     x, xi, xf;
  int      n1;
  
  sizei = floor(codebook->gray[(codebook->nrow - 5) * codebook->ncol]
		+ .5);
  sizef = floor(codebook->gray[(codebook->nrow - 6) * codebook->ncol]
		+ .5);
	       
  if (n == 0) {
    xi = sizef;
    xf = sizef + sizei;
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

  cb = mw_change_fimage(cb, xf - xi + 4, codebook->ncol);
  if (cb == NULL)
    mwerror(FATAL,1,"Not enough memory for extracted codebook.\n");

  xi *= codebook->ncol;
  xf *= codebook->ncol;

  for (x = xi; x < xf; x++)
    cb->gray[x - xi] = codebook->gray[x];    

  xi = (codebook->nrow - 4) * codebook->ncol;
  xf = codebook->nrow * codebook->ncol;
  xshift = (cb->nrow - codebook->nrow) * cb->ncol;
  for (x = xi; x < xf; x++)
    cb->gray[x + xshift] = codebook->gray[x];

}


static void
THRESBLOCK(rcol, image)

long     rcol;
Fimage   image;                   /* Input image */

{
  long     r,c,i;

  i = rcol;
  for (r = 0; r < height; r++) {
    for (c = 0; c < width; c++, i++)
      image->gray[i] = 0.0;
    i += image->ncol - width;
  }
}



static void
block_vq_adap(image, i, j, codebook, rescodebook, resrescodebook, result, symbol, symbres, symbresres, recfac, histo, reshisto, resreshisto, bmse)

Fimage          image;          /* Input image */
long	        i, j;           /* Row and column indices of block */
Fimage          codebook;       /* Codebook */
Fimage          rescodebook, resrescodebook;  /* Codebooks for residus */
Fimage          result;         /* Quantized image */
Fimage          symbol, symbres, symbresres; /* Buffers of symbols of 
				 * reproducing blocks */
long            recfac;
Fsignal	        histo;          /* Histogram of symbols */
Fsignal	        reshisto, resreshisto; /* Histogram of symbols for residus */
float	       *bmse;           /* Quantization mean square error */

{
  int             rb, cb;       /* Coordinates of coefficients in block */
  int		  rbi, obi;	/* Index of the first component of
				 * (the line of) a block in image */
  int             l;            /* Index of block in codebook */
  int             obcb, rbcol;	/* Index of the first component of
				 * (the line of) a block in codebook */
  int             index;        /* Index of reproducing block */
  float           minerr, err, e; /* quantization error for one block */
  int             sizev, sizec; /* Size of block and codebook */
  int             nrow, ncol;   /* Size of image */
    
  sizec = codebook->nrow - 4;
  sizev = codebook->ncol;
  nrow = image->nrow;
  ncol = image->ncol;
  minerr = 1e30;
  obi = i * height * image->ncol + j * width;
  obcb = 0;

  for (l = 0; l < sizec; l++) {
    rbi = obi;
    err = 0.0;
    rbcol = obcb;
    rb = 0;
    while ((rb < height) && (err<minerr)) {
      for (cb = 0; cb < width; cb++) {
	e = codebook->gray[rbcol + cb] - image->gray[rbi + cb];
	err += e * e;
      }
      rbcol += width;
      rbi += ncol;
      rb++;
    }
    if (minerr > err) {
      index = l;
      minerr = err;
    }
    obcb += sizev;
  }
    
  histo->values[index]++;
  if (recfac >= 1)
    symbol->gray[i * symbol->ncol + j] += recfac * index;
  else
    symbol->gray[i * symbol->ncol + j] = index;

  if (rescodebook) {
    rbi = obi;
    rbcol = index * sizev;
    for (rb = 0; rb < height; rb++) {
      for (cb = 0; cb < width; cb++) {
	image->gray[rbi + cb] -= codebook->gray[rbcol + cb];
      }
      rbi += ncol;
      rbcol += width;
    }

    if (symbres)
      block_vq_adap(image, i, j, rescodebook, resrescodebook, NULL, result, symbres, symbresres, NULL, 0L, reshisto, resreshisto, NULL, bmse);
    else
      block_vq_adap(image, i, j, rescodebook, resrescodebook, NULL, result, symbol, NULL, NULL, recfac * sizec, reshisto, resreshisto, NULL, bmse);

    rbi = obi;
    rbcol = index * sizev;
    for (rb = 0; rb < height; rb++) {
      for (cb = 0; cb < width; cb++) {
	result->gray[rbi + cb] += codebook->gray[rbcol + cb];
	image->gray[rbi + cb] += codebook->gray[rbcol + cb];
      }
      rbi += ncol;
      rbcol += width;
    }    

  } else
    {
      rbi = obi;
      rbcol = index * sizev;
      for (rb = 0; rb < height; rb++)
	{
	  for (cb = 0; cb < width; cb++)
	    {
	      result->gray[rbi + cb] = codebook->gray[rbcol + cb];
	    }
	  rbi += ncol;
	  rbcol += width;
	}
      *bmse = minerr;
    }

}




static void
FULLSEARCH(image, codebook, rescodebook, resrescodebook, compress, result, mse, rate, ratearc, ent)

Fimage          image;          /* Input image */
Fimage          codebook;       /* Codebook */
Fimage          rescodebook, resrescodebook; /* Codebooks for residus */
Cimage          compress;       /* Compressed image */
Fimage          result;         /* Quantized image */
double	       *mse;
double         *rate;
double         *ratearc;
double	       *ent;

{
  int             r, c;
  int             nrow, ncol;
  int             nrowb, ncolb;
  int             sizec, sizecres, sizecresres;
  long            x, size;
  Fsignal	  histo;
  Fsignal         reshisto, resreshisto;
  float           bmse;
  Fimage          symbol, symbres, symbresres;

  nrow = image->nrow;
  ncol = image->ncol;
  nrowb = nrow / height;
  ncolb = ncol / width;
  size = nrowb * ncolb;
  sizec = codebook->nrow - 4;
  *rate = *ratearc = *ent = 0.0;
  *mse = 0.0;
   
  histo = mw_new_fsignal();
  histo = mw_change_fsignal(histo, sizec);
  if (histo == NULL)
    mwerror(FATAL,1,"Not enough memory for histogram.\n");
  clear_histo(histo);
  symbol = mw_new_fimage();
  symbol = mw_change_fimage(symbol, nrowb, ncolb);
  if (symbol == NULL)
    mwerror(FATAL, 1, "Not enough memory for symbol buffer\n");

  if (rescodebook) {
    sizecres = rescodebook->nrow - 4;
    reshisto = mw_new_fsignal();
    if (mw_alloc_fsignal(reshisto, sizecres) == NULL)
      mwerror(FATAL, 1, "Not enough memory for histogram of index\n");
    clear_histo(reshisto);
    symbres = mw_new_fimage();
    symbres = mw_change_fimage(symbres, nrowb, ncolb);
    if (symbres == NULL)
      mwerror(FATAL, 1, "Not enough memory for symbol buffer\n");

    if (resrescodebook) {
      sizecresres = resrescodebook->nrow - 4;
      resreshisto = mw_new_fsignal();
      if (mw_alloc_fsignal(resreshisto, sizecresres) == NULL)
	mwerror(FATAL, 1, "Not enough memory for histogram of index\n");
      clear_histo(resreshisto);
      symbresres = mw_new_fimage();
      symbresres = mw_change_fimage(symbresres, nrowb, ncolb);
      if (symbresres == NULL)
	mwerror(FATAL, 1, "Not enough memory for symbol buffer\n");
    }
  }

  for(r=0;r<nrowb;r++)
    for(c=0;c<ncolb;c++) {
      block_vq_adap(image, r, c, codebook, rescodebook, resrescodebook, result, symbol, symbres, symbresres, 0L, histo, reshisto, resreshisto, &bmse);

      *mse += bmse;
    }

  *mse /= (float) nrow * ncol;

  compute_rates(symbol, sizec, NULL, histo, rate, ratearc, ent, compress);
  if (rescodebook) {
    compute_rates(symbres, sizecres, NULL, reshisto, rate, ratearc, ent, compress);
    if (resrescodebook) { 
      compute_rates(symbresres, sizecresres, NULL, resreshisto, rate, ratearc, ent, compress);
      mw_delete_fimage(symbresres);
      mw_delete_fsignal(resreshisto);
    }
    mw_delete_fimage(symbres);
    mw_delete_fsignal(reshisto);
  } 
    
  *ent /= (double) nrow * ncol;
  *ratearc /= (double) nrow * ncol;
  *rate /= (double) nrow * ncol * log10((double) 2.0);;
  if (*ratearc > *rate)
    *ratearc = *rate;

  mw_delete_fsignal(histo);
  mw_delete_fimage(symbol);

}





static void
FULLSEARCH_ADAP(printsnr, bitmapcode, image, codebook1, codebook2, codebook3, codebook4, rescodebook1, rescodebook2, rescodebook3, rescodebook4, resrescodebook1, resrescodebook2, compress, result, mse, rate, ratearc, ent)

int            *printsnr;       /* Control info print on SNR */       
int            *bitmapcode;     /* Encode bitmap even if all codebooks 
				 * have size 1 */
Fimage          image;          /* Input image */
Fimage          codebook1, codebook2, codebook3, codebook4; /* Codebooks for 
                                 * different classes */
Fimage          rescodebook1, rescodebook2, rescodebook3, rescodebook4; 
                                /* Residu codebooks for different classes */
Fimage          resrescodebook1, resrescodebook2; /* Residu codebooks 
				 * for different classes */
Cimage          compress;       /* Compressed file */
Fimage          result;         /* Quantized image */ 
double	       *mse;
double         *rate;
double         *ratearc;
double	       *ent;

{
  int             r, c;
  int             rb, cb, ov, ovi, ovcb;
  int             l;
  int             rcol;
  int             index;
  int             sizev, sizec1, sizec2, sizec3, sizec4;
  int             nrowb, ncolb;
  int             nrow, ncol;
  int             sizecres;
  long            x, size, sizeb;
  long	          num1, num2, num3, num4, numthres, nsig;
  float	          thres1, thres2, thres3;
  float	          benergy;			/* Block benergy */
  Fsignal         histo1, histo2, histo3, histo4;
  Fsignal         reshisto1, reshisto2, reshisto3, reshisto4;
  Fsignal         resreshisto1, resreshisto2;
  float	          mse1, mse2, mse3, mse4, msethres;
  float	          bmse;			/* Block mean square error */
  double	  e;
  Fimage	  indexcb;              /* Codebook index buffer */
  float           indexcbval[5];        /* Symbol value for each codebook */
  long            caphisto;
  int            *predic;
  int             nsymb;
  long            recfac;
  Fimage          symbol, symbex, symbres, symbresres;
  Cimage          bufcomp;             /* Buffer for compressed image */

  sizev = codebook1->ncol;
  sizec1 = codebook1->nrow - 4;
  sizec2 = sizec3 = sizec4 = 0;
  nrow = image->nrow;
  ncol = image->ncol;
  nrowb = nrow / height;
  ncolb = ncol / width;
  sizeb = nrowb * ncolb;
  size = nrow * ncol;
  num1 = num2 = num3 = num4 = numthres = 0;
  mse1 = mse2 = mse3 = mse4 = msethres = 0.0;
  *rate = *ratearc = *ent = 0.0;
  thres1 = codebook1->gray[(sizec1 + 1) * sizev];
  thres2 = thres3 = 0.0;
  nsymb = 2;
  symbres = symbresres = NULL;
  indexcbval[0] = 0.0;
  indexcbval[1] = 1.0;
  indexcbval[2] = 2.0;
  indexcbval[3] = 3.0;
  indexcbval[4] = 0.0;
  if (codebook2) 
    sizec2 = codebook2->nrow - 4;
  if (codebook3) 
    sizec3 = codebook3->nrow - 4;
  if (codebook4) 
    sizec4 = codebook4->nrow - 4;

  if ((sizec1 > 1) || (sizec2 > 1) || (sizec3 > 1) || (sizec4 > 1) || bitmapcode) {

    /*--- Memory allocation for list of codewords ---*/

    symbol = mw_new_fimage();
    symbol = mw_change_fimage(symbol, nrowb, ncolb);
    if (symbol == NULL)
      mwerror(FATAL, 1, "Not enough memory for symbol buffer\n");
    for (x = 0; x < sizeb; x++)
      symbol->gray[x] = 0.0;

    symbex = mw_new_fimage();
    symbres = mw_new_fimage();

    /*--- Memory allocation for histogram of residu quantized levels ---*/

    histo1 = mw_new_fsignal();
    histo1 = mw_change_fsignal(histo1, sizec1);
    if (histo1 == NULL)
      mwerror(FATAL,1,"Not enough memory for histogram of symbols 1.\n");
    clear_histo(histo1);

    if (rescodebook1) {
      reshisto1 = mw_new_fsignal();
      if (mw_alloc_fsignal(reshisto1, rescodebook1->nrow - 4) == NULL)
	mwerror(FATAL, 1, "Not enough memory for histogram of symbols 1 (residu)\n");
      clear_histo(reshisto1);
      symbres = mw_change_fimage(symbres, nrowb, ncolb);
      if (symbres == NULL)
	mwerror(FATAL, 1, "Not enough memory for symbol buffer 1\n");
      for (x = 0; x < sizeb; x++)
	symbres->gray[x] = 0.0;
  
      if (resrescodebook1) {
	resreshisto1 = mw_new_fsignal();
	if (mw_alloc_fsignal(resreshisto1, resrescodebook1->nrow - 4) == NULL)
	  mwerror(FATAL, 1, "Not enough memory for histogram of symbols 1 (second level residu)\n");
	clear_histo(resreshisto1);
	symbresres = mw_change_fimage(symbresres, nrowb, ncolb);
	if (symbresres == NULL)
	  mwerror(FATAL, 1, "Not enough memory for symbol buffer 1\n");
	for (x = 0; x < sizeb; x++)
	  symbresres->gray[x] = 0.0;
      }
    }
    
    /*--- Memory allocation for histograms of adaptative quantized levels ---*/

    if (codebook2) {
      thres2 = codebook2->gray[(sizec2 + 1) * sizev];
      if (thres2 > 0.0)
	nsymb++;
      else 
	indexcbval[2] = 0.0;
      histo2 = mw_new_fsignal();
      if (mw_alloc_fsignal(histo2, sizec2) == NULL)
	mwerror(FATAL, 1, "Not enough memory for histogram of symbols 2\n");
      clear_histo(histo2);

      if (rescodebook2) {
	reshisto2 = mw_new_fsignal();
	if (mw_alloc_fsignal(reshisto2, rescodebook2->nrow - 4) == NULL)
	  mwerror(FATAL, 1, "Not enough memory for histogram of symbols 2 (residu)\n");
	clear_histo(reshisto2);
	if (symbres->nrow == 0) {
	  symbres = mw_change_fimage(symbres, nrowb, ncolb);
	  if (symbres == NULL)
	    mwerror(FATAL, 1, "Not enough memory for symbol buffer 2\n");
	  for (x = 0; x < sizeb; x++)
	    symbres->gray[x] = 0.0;
	}

	if (resrescodebook2) {
	  resreshisto2 = mw_new_fsignal();
	  if (mw_alloc_fsignal(resreshisto2, resrescodebook2->nrow - 4) == NULL)
	    mwerror(FATAL, 1, "Not enough memory for histogram of symbols 2 (second level residu)\n");
	  clear_histo(resreshisto2);
	  if (symbresres->nrow == 0) {
	    symbresres = mw_change_fimage(symbresres, nrowb, ncolb);
	    if (symbresres == NULL)
	      mwerror(FATAL, 1, "Not enough memory for symbol buffer 2\n");
	    for (x = 0; x < sizeb; x++)
	      symbresres->gray[x] = 0.0;
	  }
	}
      }
    }

    if (codebook3) {
      thres3 = codebook3->gray[(sizec3 + 1) * sizev];
      if (thres3 > 0.0)
	nsymb++;
      else 
	indexcbval[3] = 0.0;
      histo3 = mw_new_fsignal();
      if (mw_alloc_fsignal(histo3, sizec3) == NULL)
	mwerror(FATAL, 1, "Not enough memory for histogram of symbols 3\n");
      clear_histo(histo3);

      if (rescodebook3) {
	reshisto3 = mw_new_fsignal();
	if (mw_alloc_fsignal(reshisto3, rescodebook3->nrow - 4) == NULL)
	  mwerror(FATAL, 1, "Not enough memory for histogram of symbols 3 (residu)\n");
	clear_histo(reshisto3);
	if (symbres->nrow == 0) {
	  symbres = mw_change_fimage(symbres, nrowb, ncolb);
	  if (symbres == NULL)
	    mwerror(FATAL, 1, "Not enough memory for symbol buffer 3\n");
	  for (x = 0; x < sizeb; x++)
	    symbres->gray[x] = 0.0;
	}
      }
    }

    if (codebook4) {
      histo4 = mw_new_fsignal();
      if (mw_alloc_fsignal(histo4, sizec4) == NULL)
	clear_histo(histo4);
      mwerror(FATAL, 1, "Not enough memory for histogram of symbols 4\n");

      if (rescodebook4) {
	reshisto4 = mw_new_fsignal();
	if (mw_alloc_fsignal(reshisto4, rescodebook4->nrow - 4) == NULL)
	  mwerror(FATAL, 1, "Not enough memory for histogram of symbols 4 (residu)\n");
	clear_histo(reshisto4);
	if (symbres->nrow == 0) {
	  symbres = mw_change_fimage(symbres, nrowb, ncolb);
	  if (symbres == NULL)
	    mwerror(FATAL, 1, "Not enough memory for symbol buffer 4\n");
	  for (x = 0; x < sizeb; x++)
	    symbres->gray[x] = 0.0;
	}
      }
    }
    
    if (symbres->nrow == 0) 
      symbres = NULL;
    if (symbresres->nrow == 0) 
      symbresres = NULL;
  
    /*--- Memory allocation for adapted codebook indices ---*/

    indexcb = mw_new_fimage();
    indexcb = mw_change_fimage(indexcb, nrowb, ncolb);
    if (indexcb == NULL)
      mwerror(FATAL,1,"Not enough memory for codebook index buffer!\n");
    
    /*--- Quantization of image ---*/

    for(rb=0;rb<nrowb;rb++)
      for(cb=0;cb<ncolb;cb++) {
	rcol = rb * height * ncol + cb * width;

	/*--- Computation of block benergy ---*/

	benergy = 0.0;
	for (r = 0; r < height; r++) {
	  for (c = 0; c < width; c++) {
	    benergy += image->gray[rcol + c] * image->gray[rcol + c];
	  }
	  rcol += ncol;
	}
	rcol -= height * ncol;

	if (benergy >= thres1) {

	  /*--- Quantization with first codebook ---*/

	  block_vq_adap(image, rb, cb, codebook1, rescodebook1, resrescodebook1, result, symbol, symbres, symbresres, 0L, histo1, reshisto1, resreshisto1, &bmse);
	  indexcb->gray[rb * ncolb + cb] = 1.0;
	  mse1 += bmse;
	  num1++;
	} else 
	  if (codebook2) {
	    if (benergy >= thres2) {

	      /*--- Quantization with second codebook ---*/

	      block_vq_adap(image, rb, cb, codebook2, rescodebook2, resrescodebook2, result, symbol, symbres, symbresres, 0L, histo2, reshisto2, resreshisto2, &bmse);
	      indexcb->gray[rb * ncolb + cb] = indexcbval[2];
	      mse2 += bmse;
	      num2++;
	    } else
	      if (codebook3) {
		if (benergy >= thres3) {

		  /*--- Quantization with third codebook ---*/

		  block_vq_adap(image, rb, cb, codebook3, rescodebook3, NULL, result, symbol, symbres, NULL, 0L, histo3, reshisto3, NULL, &bmse);
		  indexcb->gray[rb * ncolb + cb] = indexcbval[3];
		  mse3 += bmse;
		  num3++;
		} else
		  {
		    if (codebook4) {

		      /*--- Quantization with fourth codebook ---*/

		      block_vq_adap(image, rb, cb, codebook4, rescodebook4, NULL, result, symbol, symbres, NULL, 0L, histo4, reshisto4, NULL, &bmse);
		      mse4 += bmse;
		      num4++;
		    } else
		      {
			msethres += benergy;
			numthres++;
			THRESBLOCK(rcol, result);
		      }
		    indexcb->gray[rb * ncolb + cb] = 0.0;
		  }
	      } else
		{
		  indexcb->gray[rb * ncolb + cb] = 0.0;
		  msethres += benergy;
		  numthres++;
		  THRESBLOCK(rcol, result);
		}
	  } else
	    {
	      indexcb->gray[rb * ncolb + cb] = 0.0;
	      msethres += benergy;
	      numthres++;
	      THRESBLOCK(rcol, result);
	    }
      }

    *mse = (mse1 + mse2 + mse3 + mse4 + msethres) / (double) size;

    /*--- Computation of entropic rates of codebook index buffer ---*/

    *ent = (double) sizeb * log((double) sizeb);
    if (numthres > 0)
      *ent -= (double) numthres * log((double) numthres);
    if (num1 > 0)
      *ent -= (double) num1 * log((double) num1);
    if (num2 > 0)
      *ent -= (double) num2 * log((double) num2);
    if (num3 > 0)
      *ent -= (double) num3 * log((double) num3);
    if (num4 > 0)
      *ent -= (double) num4 * log((double) num4);

    predic = &nsymb;
    caphisto = 256;

    if (compress)
      bufcomp = mw_new_cimage();
    else
      bufcomp = NULL;
    arencode2(&nsymb, NULL, &nsymb, NULL, predic, NULL, NULL, indexcb, &e, bufcomp); 
    if (compress)
      ADD_BUF_TO_COMPRESS_FIMAGE(compress, bufcomp);

    *rate = 0.0;
    *ratearc = e * sizeb;
    e = *ratearc;

    if (numthres>0)
      msethres /= (float) numthres * sizev;
    if (printsnr)
      if (*printsnr == 1)
	printf("Run length: block number = %d,  rate = %.4f, threshold mse = %.2f\n", numthres, e / (double) size, msethres);


    /*--- Computation of entropy of quantized levels ---*/

    if (num1>0) {
      recfac = 0;
      if (resrescodebook1) {
	extract_symbol(symbresres, symbex, indexcb, 1.0, num1, NULL, recfac);
	compute_rates(symbex, resrescodebook1->nrow - 4, NULL, resreshisto1, rate, ratearc, ent, compress);
	mw_delete_fsignal(resreshisto1);
      } 
      if (rescodebook1) {
	extract_symbol(symbres, symbex, indexcb, 1.0, num1, NULL, recfac);
	compute_rates(symbex, rescodebook1->nrow - 4, NULL, reshisto1, rate, ratearc, ent, compress);
	mw_delete_fsignal(reshisto1);
      } 
      extract_symbol(symbol, symbex, indexcb, 1.0, num1, NULL, recfac);
      compute_rates(symbex, sizec1, NULL, histo1, rate, ratearc, ent, compress);
      mw_delete_fsignal(histo1);

      mse1 /= (float) num1 * sizev;
    }

    if (printsnr) 
      if (*printsnr == 1) {
	e = *ratearc - e;
	printf("cb1: block number = %d,  entropy = %.4f,  mse = %.2f\n", num1, e / (double) size, mse1);
	e = *ratearc;
      }

    if (codebook2) {
      if (num2 > 0) {
	recfac = 0;
	if (resrescodebook2) {
	  extract_symbol(symbresres, symbex, indexcb, indexcbval[2], num2, NULL, recfac);
	  compute_rates(symbex, resrescodebook2->nrow - 4, NULL, resreshisto2, rate, ratearc, ent, compress);
	  mw_delete_fsignal(resreshisto2);
	} 
	if (rescodebook2) {
	  extract_symbol(symbres, symbex, indexcb, indexcbval[2], num2, NULL, recfac);
	  compute_rates(symbex, rescodebook2->nrow - 4, NULL, reshisto2, rate, ratearc, ent, compress);
	  mw_delete_fsignal(reshisto2);
	}
	extract_symbol(symbol, symbex, indexcb, indexcbval[2], num2, NULL, recfac);
	compute_rates(symbex, sizec2, NULL, histo2, rate, ratearc, ent, compress);

	mse2 /= (float) num2 * sizev;
      }

      if (printsnr) 
	if (*printsnr == 1) {
	  e = *ratearc - e;
	  printf("cb2: block number = %d,  entropy = %.4f,  mse = %.2f\n", num2, e / (double) size, mse2);
	  e = *ratearc;
	}
      mw_delete_fsignal(histo2);
    }

    if (codebook3) {
      if (num3 > 0) {
	recfac = 0;
	if (rescodebook3) {
	  extract_symbol(symbres, symbex, indexcb, indexcbval[3], num3, NULL, recfac);
	  compute_rates(symbex, rescodebook3->nrow - 4, NULL, reshisto3, rate, ratearc, ent, compress);
	  mw_delete_fsignal(reshisto3);
	}
	extract_symbol(symbol, symbex, indexcb, indexcbval[3], num3, NULL, recfac);
	compute_rates(symbex, sizec3, NULL, histo3, rate, ratearc, ent, compress);

	mse3 /= (float) num3 * sizev;
      }

      if (printsnr) 
	if (*printsnr == 1) {
	  e = *ratearc - e;      
	  printf("cb3: block number = %d,  entropy = %.4f,  mse = %.2f\n", num3, e / (double) size, mse3);
	  e = *ratearc;
	}
      mw_delete_fsignal(histo3);
    }

    if (codebook4 && (thres3 > 0.0)) {
      if (num4 > 0) {
	recfac = 0;
	if (rescodebook4) {
	  extract_symbol(symbres, symbex, indexcb, 0.0, num4, NULL, recfac);
	  compute_rates(symbex, rescodebook4->nrow - 4, NULL, reshisto4, rate, ratearc, ent, compress);
	  mw_delete_fsignal(reshisto4);
	}
	extract_symbol(symbol, symbex, indexcb, 0.0, num4, NULL, recfac);
	compute_rates(symbex, sizec4, NULL, histo4, rate, ratearc, ent, compress);

	mse4 /= (float) num4 * sizev;
      }

      if (printsnr) 
	if (*printsnr == 1) {
	  e = *ratearc - e;      
	  printf("cb4: block number = %d,  entropy = %.2f,  mse = %.4f\n", num4, e / (double) size, mse4);
	}
      mw_delete_fsignal(histo4);
    }
    if (symbresres)
      mw_delete_fimage(symbresres);
    if (symbres)
      mw_delete_fimage(symbres);
    mw_delete_fimage(symbex);
    mw_delete_fimage(symbol);

    *ent /= (double) size;
    *ratearc /= (double) size;
    *rate += log10((double) nsymb) * sizeb;
    *rate /= (double) size * log10((double) 2.0);
  } else
    {
      for (x = 0; x < size; x++) {
	result->gray[x] = 0.0;
	msethres += image->gray[x] * image->gray[x];
      }
      *mse = msethres / (double) size;
      numthres = size;
    }

}



static void
VQ(printsnr, bitmapcode, image, testmulticb, ncb1, ncb2, ncb3, ncb4, codebook1, codebook2, codebook3, codebook4, nrescb1, nrescb2, nrescb3, nrescb4, rescodebook1, rescodebook2, rescodebook3, rescodebook4, nresrescb1, nresrescb2, resrescodebook1, resrescodebook2, compress, result, mse, snr, rate, ratearc, ent)

int            *printsnr;           /* Control info print on SNR */       
int            *bitmapcode;         /* Encode bitmap even if all codebooks 
				     * have size 1 */
Fimage          image;
int             testmulticb;
int             ncb1, ncb2, ncb3, ncb4;
Fimage          codebook1, codebook2, codebook3, codebook4;
int             nrescb1, nrescb2, nrescb3, nrescb4;
Fimage          rescodebook1, rescodebook2, rescodebook3, rescodebook4;
int             nresrescb1, nresrescb2;
Fimage          resrescodebook1, resrescodebook2;
Cimage          compress;       /* Compressed image */
Fimage          result;
double	       *mse;
double	       *snr;		/* Signal to noise ratio */
double         *rate;
double         *ratearc;
double	       *ent;

{
  Fimage          cb1, cb2, cb3, cb4;
  Fimage          rescb1, rescb2, rescb3, rescb4;
  Fimage          resrescb1, resrescb2;
  int             testsearch;

  if (testmulticb >= 1) {
    if (nrescb1 > ncb1)
      nrescb1 = ncb1;
    ncb1 -= count_cb(codebook1);
    if (bitmapcode || (nrescb1 >= 0)) {
      cb1 = mw_new_fimage();
      extract_cb(codebook1, cb1, nrescb1);
    } else
      cb1 = NULL;
    if (codebook2) {
      if (nrescb2 > ncb2)
	nrescb2 = ncb2;
      ncb2 -= count_cb(codebook2);
      if (nrescb2 >= 0) {
	cb2 = mw_new_fimage();
	extract_cb(codebook2, cb2, nrescb2);
      } else
	cb2 = NULL;
    } else
      cb2 = NULL;
    if (codebook3) {
      if (nrescb3 > ncb3)
	nrescb3 = ncb3;
      ncb3 -= count_cb(codebook3);
      if (nrescb3 >= 0) {
	cb3 = mw_new_fimage();
	extract_cb(codebook3, cb3, nrescb3);
      } else
	cb3 = NULL;
    } else
      cb3 = NULL;
    if (codebook4) {
      if (nrescb4 > ncb4)
	nrescb4 = ncb4;
      ncb4 -= count_cb(codebook4);
      if (nrescb4 >= 0) {
	cb4 = mw_new_fimage();
	extract_cb(codebook4, cb4, nrescb4);
      } else
	cb4 = NULL;
    } else
      cb4 = NULL;

    if (rescodebook1 && (ncb1 >= 0)) {
      if (nresrescb1 > ncb1)
	nresrescb1 = ncb1;
      ncb1 -= count_cb(rescodebook1);
      rescb1 = mw_new_fimage();
      extract_cb(rescodebook1, rescb1, nresrescb1);
      if (resrescodebook1 && (ncb1 >= 0)) {
	resrescb1 = mw_new_fimage();
	extract_cb(resrescodebook1, resrescb1, ncb1);	  
      } else
	resrescb1 = NULL;      
    } else
      rescb1 = resrescb1 = NULL;
    if (rescodebook2 && (ncb2 >= 0)) {
      if (nresrescb2 > ncb2)
	nresrescb2 = ncb2;
      ncb2 -= count_cb(rescodebook2);
      rescb2 = mw_new_fimage();
      extract_cb(rescodebook2, rescb2, nresrescb2);
      if (resrescodebook2 && (ncb2 >= 0)) {
	resrescb2 = mw_new_fimage();
	extract_cb(resrescodebook2, resrescb2, ncb2);	  
      } else
	resrescb2 = NULL;
    } else
      rescb2 = resrescb2 = NULL;
    if (rescodebook3 && (ncb3 >= 0)) {
      rescb3 = mw_new_fimage();
      extract_cb(rescodebook3, rescb3, ncb3);
    } else
      rescb3 = NULL;
    if (rescodebook4 && (ncb4 >= 0)) {
      rescb4 = mw_new_fimage();
      extract_cb(rescodebook4, rescb4, ncb4);
    } else
      rescb4 = NULL;

    if ((testmulticb >= 2) && !printsnr)
      printsnr = &testmulticb;

    if (cb1->gray[(cb1->nrow - 3) * cb1->ncol] == 0.0)
      FULLSEARCH(image, cb1, rescb1, resrescb1, compress, result, mse, rate, ratearc, ent);
    else
      FULLSEARCH_ADAP(printsnr, bitmapcode, image, cb1, cb2, cb3, cb4, rescb1, rescb2, rescb3, rescb4, resrescb1, resrescb2, compress, result, mse, rate, ratearc, ent);
    
    if (rescb4)
      mw_delete_fimage(rescb4);
    if (rescb3)
      mw_delete_fimage(rescb3);
    if (resrescb2)
      mw_delete_fimage(resrescb2);
    if (rescb2)
      mw_delete_fimage(rescb2);
    if (resrescb1)
      mw_delete_fimage(resrescb1);
    if (rescb1)
      mw_delete_fimage(rescb1);
    if (cb4)
      mw_delete_fimage(cb4);
    if (cb3)
      mw_delete_fimage(cb3);
    if (cb2)
      mw_delete_fimage(cb2);
    if (cb1)
      mw_delete_fimage(cb1);

  } else
    if (codebook1->gray[(codebook1->nrow - 3) * codebook1->ncol] == 0.0)
      FULLSEARCH(image, codebook1, rescodebook1, resrescodebook1, compress, result, mse, rate, ratearc, ent);
    else
      FULLSEARCH_ADAP(printsnr, bitmapcode, image, codebook1, codebook2, codebook3, codebook4, rescodebook1, rescodebook2, rescodebook3, rescodebook4, resrescodebook1, resrescodebook2, compress, result, mse, rate, ratearc, ent);

  if ((*mse > 0.0) && (var > 0.0))
    *snr = 10.0 * log10(var / *mse);
  else
    *snr = 150.0;
}



void
fvq(PrintSNR, SmallHeader, BitMapCode, RateDist, NCB1, NCB2, NCB3, NCB4, CodeBook2, CodeBook3, CodeBook4, NResCB1, NResCB2, NResCB3, NResCB4, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResCodeBook4, NResResCB1, NResResCB2, ResResCodeBook1, ResResCodeBook2, Compress, Image, CodeBook1, Result, MSE, SNR, Entropy, RateAr)

int        *PrintSNR;           /* Control info print on SNR */       
int        *SmallHeader;        /* Do not specify size of image in header */
int        *BitMapCode;         /* Encode bitmap even if all codebooks 
				 * have size 1 */
int        *RateDist;           /* Flag for computation of rate-distortion 
				 * curves */
int        *NCB1, *NCB2, *NCB3, *NCB4;  /* Index of codebook to be used in 
				 * CodeBook1, ..., CodeBook4 buffers */
Fimage      CodeBook2, CodeBook3, CodeBook4; /* Buffers for second, third 
				 * and fourth codebook */
int        *NResCB1, *NResCB2, *NResCB3, *NResCB4; /* Index of codebook 
				  * to be used in ResCodeBook1, ..., 
				  * ResCodeBook4 buffers */
Fimage      ResCodeBook1, ResCodeBook2, ResCodeBook3, ResCodeBook4;
                                /* Buffers for first, ..., fourth residu 
				 * codebook */
int        *NResResCB1, *NResResCB2; /* Index of codebook to be used in 
				 * ResResCodeBook1 and ResResCodeBook2 
				 * buffers */
Fimage      ResResCodeBook1, ResResCodeBook2; /* Buffers for first and second 
				 * second level residu codebook */
Cimage      Compress;		/* Compressed `Image` */
Fimage      Image;              /* Input image */
Fimage      CodeBook1;          /* Buffer for first codebook */
Fimage      Result;             /* Quantized image */
double	   *MSE;		/* Mean square error between 
    					 * original and quantized image */
double     *SNR;		/* Signal to noise ratio */
double     *Entropy;            /* Entropic rate */
double     *RateAr;             /* Arithmetic coding rate */


{
  int             n1, n2, n3, n4;     /* Indices of codebooks for different 
				       * classes */
  int             ninit1, ninit2, ninit3, ninit4; /* Initial values for n1, 
                                       * n2, n3 and n4 */
  int             nfin1, nfin2, nfin3, nfin4; /* Final values for n1, 
                                       * n2, n3 and n4 */
  int             nres1, nres2, nres3, nres4; /* Indices of residus codebooks 
				       * for different classes */
  int             nresres1, nresres2; /* Indices of residus codebooks 
				       * for different classes */
  int             testmulticb;        /* Control for multiple codebooks and 
				       * computation of dist. rate curve */
  double          Rate, ent, Ratearc; /* Bit rates */
  double          mse, snr;           /* m.s.e. and snr */
  bufmse          bmse, brate, bratearc, bent; /* Buffer of m.s.e and rates 
				       * for different classes and different 
				       * size of codebooks */
  bufind          indcb, numcb;       /* Indices and number of codebooks */
  float           brl[MAX_ADAP];      /* Rate for encoding of codebook 
				       * indices */
  int             q, nadap;           /* Indices of codebook and class */
  int             test_dr;            /* Control dist. rate curve */

  /*--- Check input data ---*/

  CHECK_INPUT(RateDist, NCB1, CodeBook1, CodeBook2, CodeBook3, CodeBook4, NResCB1, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResCodeBook4, NResResCB1, ResResCodeBook1, ResResCodeBook2, Image);

  /*--- Memory allocation for quantized image ---*/

  Result = mw_change_fimage(Result, Image->nrow, Image->ncol);
  if (Result == NULL)
    mwerror(FATAL,1,"Not enough memory for quantized image.\n");

  /*--- Computes variance of image (for SNR) --- */

  Variance(Image);

  /*--- Test if codebook files contain one or several codebooks ---*/

  if (RateDist || NCB1 || NResCB1 || NResResCB1)
    testmulticb = 1;
  else
    testmulticb = 0;

  /*--- Compute maximum number of codebooks ---*/

  nadapcb = 1;
  if (CodeBook2) {
    nadapcb++;
    if (CodeBook3) {
      nadapcb++;
      if (CodeBook4)
	nadapcb++;
    }
  }

  if (testmulticb == 1) {

    /*--- Search index of codebook to be used in codebook files ---*/

    ninit1 = ninit2 = ninit3 = ninit4 = 0;
    nfin1 = nfin2 = nfin3 = nfin4 = 0;
    nres1 = count_cb(CodeBook1) - 1;
    nres2 = count_cb(CodeBook2) - 1;
    nres3 = count_cb(CodeBook3) - 1;
    nres4 = count_cb(CodeBook4) - 1;
    nresres1 = count_cb(ResCodeBook1) - 1;
    nresres2 = count_cb(ResCodeBook2) - 1;

    if (RateDist) {
      nfin1 = count_cb(CodeBook1);
      nfin2 = count_cb(CodeBook2);
      nfin3 = count_cb(CodeBook3);
      nfin4 = count_cb(CodeBook4);
      nfin1 += count_cb(ResCodeBook1);
      nfin2 += count_cb(ResCodeBook2);
      nfin3 += count_cb(ResCodeBook3);
      nfin4 += count_cb(ResCodeBook4);
      nfin1 += count_cb(ResResCodeBook1);
      nfin2 += count_cb(ResResCodeBook2);
      numcb[0] = nfin1;
      numcb[1] = nfin2;
      numcb[2] = nfin3;
      numcb[3] = nfin4;
      testmulticb = 2;
    }

    if (NCB1) {
      if (*NCB1 > nres1) {
	mwerror(WARNING, 0, "Value of NCB1 too large!\n\tNCB1 set to %d\n", nres1);
	*NCB1 = nres1;
      }
      nres1 = *NCB1;
      ninit1 = *NCB1;
      nfin1 = *NCB1 + 1;
    }
    if (NCB2) {
      if (*NCB2 > nres2) {
	mwerror(WARNING, 0, "Value of NCB2 too large!\n\tNCB2 set to %d\n", nres2);
	*NCB2 = nres2;
      }
      nres2 = *NCB2;
      ninit2 = nfin2 = *NCB2;
      if (!ResCodeBook2)
	ninit2 = nfin2 = *NCB2;
    }
    if (NCB3) {
      if (*NCB3 > nres3) {
	mwerror(WARNING, 0, "Value of NCB3 too large!\n\tNCB3 set to %d\n", nres3);
	*NCB3 = nres3;
      }
      nres3 = *NCB3;
      ninit3 = nfin3 = *NCB3;
      if (!ResCodeBook3) 
	ninit3 = nfin3 = *NCB3;
    }
    if (NCB4) {
      if (*NCB4 > nres4) {
	mwerror(WARNING, 0, "Value of NCB4 too large!\n\tNCB4 set to %d\n", nres4);
	*NCB4 = nres4;
      }
      nres4 = *NCB4;
      ninit4 = nfin4 = *NCB4;
      if (!ResCodeBook4) 
	ninit4 = nfin4 = *NCB4;
    }

    if (NResCB1) {
      if (*NResCB1 > nresres1) {
	mwerror(WARNING, 0, "Value of NResCB1 too large!\n\tNResCB1 set to %d\n", nresres1);
	*NResCB1 = nresres1;
      }
      nresres1 = *NResCB1;
      ninit1 = count_cb(CodeBook1) + *NResCB1;
      nfin1 = count_cb(CodeBook1) + *NResCB1 + 1;
    }
    if (NResCB2) {
      if (*NResCB2 > nresres2) {
	mwerror(WARNING, 0, "Value of NResCB2 too large!\n\tNResCB2 set to %d\n", nresres2);
	*NResCB2 = nresres2;
      }
      nresres2 = *NResCB2;
      ninit2 = nfin2 = count_cb(CodeBook2) + *NResCB2;
    }
    if (NResCB3) {
      ninit3 = nfin3 = count_cb(CodeBook3) + *NResCB3;
    }
    if (NResCB4) {
      ninit4 = nfin4 = count_cb(CodeBook4) + *NResCB4;
    }

    if (NResResCB1) {
      ninit1 = count_cb(CodeBook1) + count_cb(ResCodeBook1) + *NResResCB1;
      nfin1 = count_cb(CodeBook1) + count_cb(ResCodeBook1) + *NResResCB1 + 1;
    }
    if (NResResCB2) {
      ninit2 = count_cb(CodeBook2) + count_cb(ResCodeBook2) + *NResResCB2;
      nfin2 = count_cb(CodeBook2) + count_cb(ResCodeBook2) + *NResResCB2 + 1;
    }

  }    

  if (testmulticb <= 1) {

    /*--- Codebook buffers contain only one codebook, or indices ---*/
             /*---  of codebooks are specifoed by user ---*/

    indcb[0] = ninit1;
    indcb[1] = ninit2;
    indcb[2] = ninit3;
    indcb[3] = ninit4;
    INIT_ENCODING_FIMAGE(SmallHeader, Image->nrow, Image->ncol, testmulticb, indcb, Compress);

    if (testmulticb == 0) {
      VQ(PrintSNR, NULL, Image, testmulticb, 0, 0, 0, 0, CodeBook1, CodeBook2, CodeBook3, CodeBook4, 0, 0, 0, 0, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResCodeBook4, 0, 0, ResResCodeBook1, ResResCodeBook2, Compress, Result, &mse, &snr, &Rate, &Ratearc, &ent);

    } else
      {
	
	VQ(PrintSNR, BitMapCode, Image, testmulticb, ninit1, ninit2, ninit3, ninit4, CodeBook1, CodeBook2, CodeBook3, CodeBook4, nres1, nres2, nres3, nres4, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResCodeBook4, nresres1, nresres2, ResResCodeBook1, ResResCodeBook2, Compress, Result, &mse, &snr, &Rate, &Ratearc, &ent);
      }

    if (!PrintSNR) 
      printf("%.4f  %.4f  %.4f  %.2f\n", Rate, Ratearc, ent, snr);
	
    if (Compress)
      RESIZE_COMPRESS_FIMAGE(Compress);
  
  } else
    {

      /*--- Compute rate and distortion associated to each codebook ---*/

      for (n1 = ninit1; n1 < nfin1; n1++) {
	VQ(PrintSNR, &n1, Image, testmulticb, n1, ninit2, ninit3, ninit4, CodeBook1, NULL, NULL, NULL, nres1, nres2, nres3, nres4, ResCodeBook1, NULL, NULL, NULL, nresres1, nresres2, ResResCodeBook1, ResResCodeBook2, NULL, Result, &mse, &snr, &Rate, &Ratearc, &ent);
	if ((n1 == 0) && (CodeBook1->gray[(CodeBook1->nrow - 3) * CodeBook1->ncol] > 0.0)) {
	  bmse[0][n1] = energy;
	  brate[0][n1] = 0.0;
	  bratearc[0][n1] = 0.0;
	  bent[0][n1] = 0.0;
	  brl[0] = Ratearc;
	} else
	  {
	    bmse[0][n1] = mse;
	    brate[0][n1] = Rate;
	    bratearc[0][n1] = Ratearc;
	    bent[0][n1] = ent;
	  }
	if (PrintSNR) 
	  if (*PrintSNR != 1)
	    printf("%.4f  %.4f  %.4f  %.2f\n", Rate, Ratearc, ent, snr);
      }
 
      ninit1 = 1;
      for (n2 = ninit2; n2 < nfin2; n2++) {
	VQ(PrintSNR, NULL, Image, testmulticb, ninit1, n2, ninit3, ninit4, CodeBook1, CodeBook2, NULL, NULL, nres1, nres2, nres3, nres4, ResCodeBook1, ResCodeBook2, NULL, NULL, nresres1, nresres2, ResResCodeBook1, ResResCodeBook2, NULL, Result, &mse, &snr, &Rate, &Ratearc, &ent);
	if (PrintSNR) 
	  if (*PrintSNR != 1)
	    printf("%.4f  %.4f  %.4f  %.2f\n", Rate, Ratearc, ent, snr);
	if ((n2 == 0) && (CodeBook2->gray[(CodeBook2->nrow - 3) * CodeBook2->ncol] > 0.0)) {
	  bmse[1][n2] = 0.0;
	  brate[1][n2] = 0.0;
	  bratearc[1][n2] = 0.0;
	  bent[1][n2] = 0.0;
	  brl[1] = Ratearc - bratearc[0][ninit1];
	} else
	  {
	    bmse[1][n2] = mse - bmse[0][ninit1];
	    brate[1][n2] = Rate - brate[0][ninit1];
	    bratearc[1][n2] = Ratearc - bratearc[0][ninit1];
	    bent[1][n2] = ent - bent[0][ninit1];
	  }
      }

      ninit2 = 1;
      for (n3 = ninit3; n3 < nfin3; n3++) {
	VQ(PrintSNR, NULL, BitMapCode, Image, testmulticb, ninit1, ninit2, n3, ninit4, CodeBook1, CodeBook2, CodeBook3, NULL, nres1, nres2, nres3, nres4, ResCodeBook1, ResCodeBook2, ResCodeBook3, NULL, nresres1, nresres2, ResResCodeBook1, ResResCodeBook2, NULL, Result, &mse, &snr, &Rate, &Ratearc, &ent);
	if (PrintSNR) 
	  if (*PrintSNR != 1)
	    printf("%.4f  %.4f  %.4f  %.2f\n", Rate, Ratearc, ent, snr);
	if ((n3 == 0) && (CodeBook3->gray[(CodeBook3->nrow - 3) * CodeBook3->ncol] > 0.0)) {
	  bmse[2][n3] = 0.0;
	  brate[2][n3] = 0.0;
	  bratearc[2][n3] = 0.0;
	  bent[2][n3] = 0.0;
	  brl[2] = Ratearc - bratearc[0][ninit1] - bratearc[1][ninit2];
	} else
	  {
	    bmse[2][n3] = mse - bmse[0][ninit1] - bmse[1][ninit2];
	    brate[2][n3] = Rate - brate[0][ninit1] - brate[1][ninit2];
	    bratearc[2][n3] = Ratearc - bratearc[0][ninit1] - bratearc[1][ninit2];
	    bent[2][n3] = ent - bent[0][ninit1] - bent[1][ninit2];
	  }
      }

      ninit3 = 1;
      for (n4 = ninit4; n4 < nfin4; n4++) {
	VQ(PrintSNR, NULL, Image, testmulticb, ninit1, ninit2, ninit3, n4, CodeBook1, CodeBook2, CodeBook3, CodeBook4, nres1, nres2, nres3, nres4, ResCodeBook1, ResCodeBook2, ResCodeBook3, ResCodeBook4, nresres1, nresres2, ResResCodeBook1, ResResCodeBook2, NULL, Result, &mse, &snr, &Rate, &Ratearc, &ent);
	if (PrintSNR)
	  if (*PrintSNR != 1)
	    printf("%.4f  %.4f  %.4f  %.2f\n", Rate, Ratearc, ent, snr);
	bmse[3][n4] = mse - bmse[0][ninit1] - bmse[1][ninit2] - bmse[2][ninit3];
	brate[3][n4] = Rate - brate[0][ninit1] - brate[1][ninit2] - brate[2][ninit3];
	bratearc[3][n4] = Ratearc - bratearc[0][ninit1] - bratearc[1][ninit2] - bratearc[2][ninit3];
	bent[3][n4] = ent - bent[0][ninit1] - bent[1][ninit2] - bent[2][ninit3];
      }

      /*--- Compute rate distortion curve ---*/

      INIT_TARGRATE_DR();
      test_dr = 0;
      for (q = 0; q <= max_count_dr; q++) {
	compute_indexcb(indcb, numcb, bmse, bratearc, brl, targrate_dr[q], test_dr);
	mse = 0.0;
	Ratearc = 0.0;
	for (nadap = 0; nadap < nadapcb; nadap++) {
	  mse += bmse[nadap][indcb[nadap]];
	  Ratearc += bratearc[nadap][indcb[nadap]];
	  if ((nadap > 1) && (indcb[nadap] > 0))
	    if (indcb[nadap-1] == 0)
	      Ratearc += brl[nadap - 1];
		  
	}
	/*
	   for (nadap = 0; nadap < nadapcb; nadap++) 
	   printf("%d  ", indcb[nadap]);
	   */
	printf("%.4f  %.2f\n", Ratearc, 10.0 * log10((double) var / mse));
      }

    }

  if (MSE) {
    *MSE = mse;
    *SNR = snr;
    *Entropy = ent;
    *RateAr = Ratearc;
  }
}

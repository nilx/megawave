/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
name = {fscalq};
author = {"Jean-Pierre D'Ales"};
version = {"2.05"};
function = {"Scalar quantization of an image"};
usage = {
 'p'->PrintSNR           "Do not print info on SNR",
 'h'->SmallHeader        "Insert only a reduced header at top of Compress",
 'n':StepNum->NStep      "Number of quantization steps", 
 's':StepSize->SStep     "Size of quantization steps", 
 'c'->Center             "0 is a quantization step",
 'o':Compress<-Compress  "Compressed representation of Image",
 Input->Image            "Input image (fimage)", 
 QImage<-Result          "Output quantized image (fimage)", 
  { 
    MSE<-MSE             "MSR", 
    SNR<-SNR             "SNR",
    Entropy<-Ent         "Ent",
    RateAr<-RateAr       "RateAr"
  }
	};
 */

/*----------------------------------------------------------------------
  v2.05 (JF) revision according to the light preprocessor :
      1/ allocation of <Ent> and <RateAr> added to avoid core dump
         with the right process of optional arguments (light preprocessor),
	 when no optional arguments are given. This is a temporary 
         solution : the module's header needs to be rewritten !
      2/ bad call to fmse() corrected.
----------------------------------------------------------------------*/
 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for arencode2(), entropy(), fmse() */

/*--- Constants ---*/

#define MAX_SIZEO 32383          /* Maximum number of line and column for 
				  * output cimage (compressed file) */
#define LOG_STEP 4.0             /* Logarithmic step for float encoding */
#define NBIT_LOG 4               /* Number of bits dedicated to encode 
				  * the logarithmic value of a float */
#define NBIT_STEP 12             /* Number of bits dedicated to encode 
				  * the quantization step of a float */
#define NBIT_SIZEIM 16           /* Number of bits dedicated to encode 
				  * the dimensions of image */
#define NBIT_NSTEP  13           /* Number of bits dedicated to encode 
				  * the number of quantization steps */
#define NBIT_MINSTEP 16          /* Number of bits dedicated to encode 
				  * the lowest quantization step */
#define MAX_NSTEP ((1 << NBIT_NSTEP) - 1)  /* Maximum number 
				  * of quantization steps */
#define MAX_MINSTEP ((1 << NBIT_MINSTEP) - 1)  /* Maximum value of lowest 
				  * quantization step */


static long     effnbit;          /* Total number of bits stored in compress */
static int      ncodewords;       /* Number of codewords stored in compress */
static int      bits_to_go;       /* Number of free bits in the currently 
				   * encoded codeword */
static int      buffer;           /* Bits to encode in the currently 
				   * encoded codeword */
static unsigned char *ptrc;       /* Pointer to compress->gray for next 
				   * codewords to encode */
float           min, max;	  /* Minimum and maximum values of gray level
				   * in `Image` */




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
  printf("Reallocation of Compress for fimage scalar quantization.\n");

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
ENCODE_INT_FIMAGE(symb, max_symb, compress)

int            symb;         /* Value of symbol to write */
int            max_symb;     /* Half of maximum value for symbol */
Cimage         compress;     /* Compressed file */

{

  while (max_symb > 0) {
    if (symb >= max_symb) {
      ADD_BIT_TO_COMPRESS_FIMAGE(1, compress);
      symb = symb % max_symb;
    } else
      ADD_BIT_TO_COMPRESS_FIMAGE(0, compress);
    max_symb /= 2;
  }

}




static int
FLOAT2INT(f, nbitlog, nbitstep, testoverflow)

float   *f;
int      nbitlog, nbitstep;
int     *testoverflow;

{
  int n;
  int logf;
  double logstep, fstep;
  float  fabsf;

  *testoverflow = 0;
  fabsf = fabs(*f);

  if (fabsf < 1.0) {
    n = fabsf * (double) ((int) 1<<nbitstep);
    fabsf = ((double) n + 1.0) / (double) ((int) 1<<nbitstep);
  } else
    {
      logstep = LOG_STEP;
      logf = (int) floor(log((double) fabsf) / log((double) 2.0) / logstep) + 1;
      if (logf >= 1<<nbitlog) {
	logf = (1 << nbitlog) - 1;
	*testoverflow = 1;
      }
      fstep = ((double) ((int) 1 << ((int) logstep * logf)) - ((int) 1 << ((int) logstep * (logf - 1)))) / (double) ((int) 1 << nbitstep);
      n = (1 << nbitstep) * logf + ((double) fabsf - ((int) 1 << ((int) logstep * (logf - 1)))) / fstep;
      if (n >= 1<<(nbitstep + nbitlog))
	n = (1<<(nbitstep + nbitlog)) - 1;
      fabsf = ((int) 1 << ((int) logstep * (logf - 1))) + (n - ((int) 1 << nbitstep) * logf + 1.0) * fstep; 
    }
  if (*f < 0.0)
    *f = - fabsf;
  else
    *f = fabsf;

  return(n);
}



static void
INIT_ENCODING_FIMAGE(smallheader, nrow, ncol, c_stepsize, nstep, center, minstep, ashift, c_ashift, compress)

int         *smallheader;        /* Put size of image in header iff NULL */ 
int          nrow, ncol;         /* Size of image */
int          c_stepsize;	 /* Cell width code */
int          nstep;              /* Number of quantization level */
int         *center;             /* Flag for centering of quantization step */
float        ashift;  
int          c_ashift;           /* Index of lowest quantization step shift */
int          minstep;            /* Index of lowest cell */
Cimage       compress;           /* Compressed file */

{

  effnbit = NBIT_NSTEP + 1;
  if (nstep > 1) {
    effnbit += NBIT_LOG + NBIT_STEP;
    if (center)
      effnbit += NBIT_MINSTEP + 1;
    else
      effnbit += NBIT_LOG + NBIT_STEP + 1;
  } else
    if (!center)
      effnbit += NBIT_LOG + NBIT_STEP + 1;

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

    /*--- Encode of quantization steps ---*/

    ENCODE_INT_FIMAGE(nstep, 1 << (NBIT_NSTEP - 1), compress);

    /*--- Check if there are different codebooks for each class ---*/

    if (center) {
      ENCODE_INT_FIMAGE(1, 1, compress);
      if (nstep > 1)
      {
	if (minstep >= 0) {
	  ENCODE_INT_FIMAGE(0, 1, compress);
	  ENCODE_INT_FIMAGE(minstep, 1 << (NBIT_MINSTEP - 1), compress);
	} else
	  {
	    ENCODE_INT_FIMAGE(1, 1, compress);
	    ENCODE_INT_FIMAGE(-minstep, 1 << (NBIT_MINSTEP - 1), compress);
	  }
      }
    } else
      {
	ENCODE_INT_FIMAGE(0, 1, compress);
	if (ashift >= 0) 
	  ENCODE_INT_FIMAGE(0, 1, compress);
	else
	  ENCODE_INT_FIMAGE(1, 1, compress);
	ENCODE_INT_FIMAGE(c_ashift, 1 << (NBIT_LOG + NBIT_STEP - 1), compress);
      } 

    /*--- Encode quantization step width ---*/

    if (nstep > 1)
      ENCODE_INT_FIMAGE(c_stepsize, 1 << (NBIT_LOG + NBIT_STEP - 1), compress);

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
min_max(image)
     
Fimage          image;

{
    long            i;
    long            size;	/* Number of pixel in `image` */

    size = image->nrow * image->ncol;

    min =  max = image->gray[0];
    for (i = 1; i < size; i++)
	if (min > image->gray[i])
	    min = image->gray[i];
	else if (max < image->gray[i])
	    max = image->gray[i];
}



static void
unif_quant(printsnr, smallheader, nstep, sstep, center, image, result, compress, ent, ratear)

int        *printsnr;           /* Control info print on SNR */       
int        *smallheader;        /* Put size of image in header iff NULL */ 
int        *nstep;              /* Size of histogram */
float      *sstep;              /* Step size for quantization */
int        *center;             /* Flag for centering of quantization step */
Fimage      image, result;
Cimage      compress;		/* Compressed `Image` */
double     *ent;            /* Entropic rate */
double     *ratear;             /* Arithmetic coding rate */

{
  float         stepsize;	 /* Cell width */
  float         ashift, sshift;  
  int           step, minstep;   /* Index of cell / min cell */
  int           hsize;           /* Number of quantization level */
  int           c_ashift, c_stepsize;
  long          i;
  long          isize;	         /* Number of pixel in `image` */
  float         test, testmin, testmax;
  int          *predic;          /* Control predicitive coding */
  double        e;               /* Rate with arithmetic coding */
  double        er;              /* Rate with no coding */
  Fsignal       histo;           /* Histogram for quantized image */
  Fimage        symbol;          /* Qunatization symbols buffer */
  Cimage        bufcomp;         /* Buffer for compressed image */
  int           testoverflow;    /* test for overflow in encoding of stepsize 
				  * or ashift */

  if (!ent) mwerror(FATAL,1,"NULL <ent> in unif_quant()\n");

  min_max(image);
  isize = (long) image->nrow * image->ncol;
  
  /*--- Memory allocation and initialisation of symbol buffer ---*/

  symbol = mw_new_fimage();
  symbol = mw_change_fimage(symbol, image->nrow, image->ncol);
  if (symbol == NULL)
    mwerror(FATAL, 1, "Not enough memory for symbol buffer.\n");

  /*--- Computes size of histogram and step size for quantization ---*/

  if (sstep) {
    if (*sstep <= 0.0)
      mwerror(FATAL, 2, "StepSize must be positive!\n");
    stepsize = *sstep;
    c_stepsize = FLOAT2INT(&stepsize, NBIT_LOG, NBIT_STEP, &testoverflow);
    if (!printsnr)
      printf("Step width : %.6f\n", stepsize);

    if (center)
      test = 1.0 + floor(max / stepsize + .5) 
	   - floor(min / stepsize + .5);
    else
      {
	test = floor((max - min) / stepsize) + 1.0;
	sshift = min - (test - (max - min) / stepsize) / 2.0;
	ashift = sshift + stepsize / 2.0; 
      }

    hsize = (int) floor(test + .5);
    if (hsize > MAX_NSTEP)
      mwerror(FATAL, 2, "StepSize is to small => nstep is too large!\n");

  } else
    {
      hsize = isize;
      if (hsize > MAX_NSTEP)
	hsize = MAX_NSTEP;
      if (nstep) {
	if (*nstep > 0)
	  hsize = *nstep;
	else
	  mwerror(WARNING, 0, "StepNum must be positive!\n");
	if (hsize > MAX_NSTEP) {
	  hsize = MAX_NSTEP;
	  mwerror(WARNING, 0, "StepNum is too large! -> set to %d\n", (int) MAX_NSTEP);
	}	  
      }
      if (center) {

	if (hsize > 1) {
	  stepsize = (max - min) / (float) (hsize - 1.0);
	  testmin = floor(min / stepsize + .5) - 0.5;
	  testmax = floor(max / stepsize + .5) + 0.5;
	  stepsize = max / testmax;
	  if (testmin * stepsize > min)
	    stepsize = min / testmin;
	} else
	  if (max >= -min)
	    stepsize = 2.0 * max;
	  else
	    stepsize = - 2.0 * min;

      } else
	{
	  stepsize = (max - min) / ((float) hsize - 0.01);
	  sshift = min;
	  ashift = min + stepsize / 2.0;
	}

      c_stepsize = FLOAT2INT(&stepsize, NBIT_LOG, NBIT_STEP, &testoverflow);
      if (!printsnr)
	printf("Step width : %.6f\n", stepsize);
    }

  if (testoverflow == 1)
    mwerror(WARNING, 3, "Overflow in encoding of stepsize!\n");

  if (!printsnr)
    printf("Number of steps : %d\n", hsize);

  /*--- Memory allocation and initialisation of histogramm ---*/

  histo = mw_new_fsignal();
  histo = mw_change_fsignal(histo, hsize);
  if(histo == NULL)
    mwerror(FATAL, 1, "Not enough memory for histogram fsignal\n");

  for(i=0;i<hsize;i++)
    histo->values[i] = 0.0;
  

  /*--- Quantization of image ---*/

  if (center) {
    minstep = (long) floor(min/stepsize + .5);
    if ((minstep < - MAX_MINSTEP) || (minstep > MAX_MINSTEP))
      mwerror(FATAL, 3, "Bad value for minstep!\n");
    if (!printsnr)
      printf("Lowest / highest steps : %d / %d\n", minstep, 
	     (int) floor(max/stepsize + .5));
    for(i=0;i<isize;i++) {
      step = (int) floor(image->gray[i] / stepsize + .5);
      result->gray[i] = (float) step * stepsize;
      step -= minstep;
      if ((step<0)||(step>=hsize)) {
	printf("xf = %.3f, xi = %d\n", image->gray[i] / stepsize, step);
	if (step < 0)
	  step = 0;
	if (step >= hsize)
	  step = hsize - 1;
      }
      symbol->gray[i] = step;
      histo->values[step] += 1.0;
    }
    histo->shift = minstep * stepsize;
  } else
    {
      c_ashift = FLOAT2INT(&ashift, NBIT_LOG, NBIT_STEP, &testoverflow);
      if (testoverflow == 1)
	mwerror(WARNING, 3, "Overflow in encoding of ashift!\n");
      
      for(i=0;i<isize;i++)  {
	step = (long) ((image->gray[i] - sshift) / stepsize);
	if ((step<0)||(step>=hsize)) {
	  printf("xf = %.3f, xi = %d\n", (image->gray[i] - sshift) / stepsize, step);
	  if (step < 0)
	    step = 0;
	  if (step >= hsize)
	    step = hsize - 1;
	}
 	result->gray[i] = step * stepsize + ashift;
	symbol->gray[i] = step;
	histo->values[step] += 1.0;
      }
      histo->shift = ashift;
    }

  histo->scale = (max - min) / (float) hsize;
  histo->sgrate = stepsize;

  entropy(histo, ent);
  printf("entropy : ent=%g\n",*ent);

  mw_delete_fsignal(histo);

  INIT_ENCODING_FIMAGE(smallheader, image->nrow, image->ncol, c_stepsize, hsize, center, minstep, ashift, c_ashift, compress);

  if (compress)
    bufcomp = mw_new_cimage();
  else
    bufcomp = NULL;

  if (hsize <= 32)
    predic = &hsize;
  else
    predic = NULL;

  if (hsize > 1)
    arencode2(&hsize, NULL, &hsize, NULL, predic, NULL, NULL, symbol, &e, bufcomp);
  else
    e = 0.0;

  er = log10((double) hsize) / log10((double) 2.0);

  if (e > er)
    *ratear = er + (effnbit / (double) isize);
  else
    *ratear = e + (effnbit / (double) isize);

  /* To check, value of *ent bugged  on Linux with cmw2 -O */
  *ent += ((double) effnbit) / isize;
    
  if (compress && (hsize > 1))
    ADD_BUF_TO_COMPRESS_FIMAGE(compress, bufcomp);
 
  if (compress)
    RESIZE_COMPRESS_FIMAGE(compress);

  mw_delete_fimage(symbol);
}




void
fscalq(PrintSNR, SmallHeader, NStep, SStep, Center, Compress, Image, Result, MSE, SNR, Ent, RateAr)

int        *PrintSNR;           /* Control info print on SNR */       
int        *SmallHeader;        /* Do not specify size of image in header */
int        *NStep;              /* Size of histogram */
float      *SStep;              /* Step size for histogram */
int        *Center;             /* Interval centered at zero */
Cimage      Compress;		/* Compressed `Image` */
Fimage      Image;              /* Input image */
Fimage      Result;             /* Quantized image */
double	   *MSE;		/* Mean square error between 
				 * original and quantized image */
double     *SNR;		/* Signal to noise ratio */
double     *Ent;                /* Entropic rate */
double     *RateAr;             /* Arithmetic coding rate */

{
  double      psnr;		/* Peak signal to noise ratio */
  double      mrd;		/* Root mean square error between 
    					 * original and quantized image */
  char PsnrFlg = '1';           /* To compute 255-PSNR */

  /* TO AVOID CORE DUMP WHEN NO OPTIONAL ARGUMENTS ARE GIVEN.
     Should be rewritten.
  */
  if (!Ent) Ent=(double *) malloc(sizeof(double));
  if (!RateAr) RateAr=(double *) malloc(sizeof(double));

  if (NStep && SStep) 
    mwerror(FATAL, 2, "Flag -n and -s are incompatible.\n");

  Result = mw_change_fimage(Result, Image->nrow, Image->ncol);
  if (Result == NULL)
    mwerror(FATAL, 1, "Not enough memory.\n");
  
  unif_quant(PrintSNR, SmallHeader, NStep, SStep, Center, Image, Result, Compress, Ent, RateAr);
  
  if (MSE) fmse(Image, Result, NULL, &PsnrFlg, SNR, &psnr, MSE, &mrd);

  if (!PrintSNR) 
    printf("%.4f %.4f %.2f\n", *Ent, *RateAr, psnr);
  if (PrintSNR)
    if (*PrintSNR == 1) 
      printf("%.4f %.4f %.2f\n", *Ent, *RateAr, psnr);

}

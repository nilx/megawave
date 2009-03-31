/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {fiscalq};
author = {"Jean-Pierre D'Ales"};
version = {"2.02"};
function = {"reconstruction of a scalar quantized image"};
usage = {
 'p'->Print          "Do not print info on reconstruction process",
 'r':NRow->NRow      "Number of rows in reconstructed image",
 'h':NCol->NCol      "Number of columns in reconstructed image",
 Compress->Compress  "Compressed image (cimage)",
 Result<-Result      "Reconstructed image (fimage)",
   {
     Rate<-Rate      "Compression rate"
   }
};
*/
/*----------------------------------------------------------------------
 v2.02 (JF) revision according to the light preprocessor :
        allocation of <Rate> added to avoid core dump
        with the right process of optional arguments (light preprocessor),
        when no optional arguments are given. This is a temporary
        solution : the module's header needs to be rewritten !
----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for ardecode2() */

/*--- Constants ---*/

#define LOG_STEP 4.0            /* Logarithmic step for float encoding */
#define NBIT_LOG 4              /* Number of bits dedicated to encode
                                 * the logarithmic value of a float */
#define NBIT_STEP 12            /* Number of bits dedicated to encode
                                 * the quantization step of a float */
#define NBIT_SIZEIM 16          /* Number of bits dedicated to encode
                                 * the dimensions of image */
#define NBIT_NSTEP  13          /* Number of bits dedicated to encode
                                 * the number of quantization steps */
#define NBIT_MINSTEP 16         /* Number of bits dedicated to encode
                                 * the lowest quantization step */

static int ncwreadfi;           /* Number of 8 bits-codewords read
                                 * in compress */
static int sizecomp;            /* Total number of 8 bits-codewords stored
                                 * in compress */
static int bits_to_go;          /* Number of free bits in the currently
                                 * encoded codeword */
static int buffer;              /* Bits to encode in the currently
                                 * encoded codeword */
static unsigned char *ptrc;     /* Pointer to compress->gray for next
                                 * codewords to decode */

static int READ_BIT_FIMAGE(void)
{
    int bit;                    /* Value of read and returned bit */

    if (bits_to_go == 0)
    {
        if (ncwreadfi == sizecomp)
            mwerror(FATAL, 1,
                    "Buffer ended to soon while readind header for image!\n");
        else
        {
            ptrc++;
            buffer = *ptrc;
            bits_to_go = 8;
            ncwreadfi += 1;
        }
    }

    bit = buffer & 1;
    buffer >>= 1;
    bits_to_go -= 1;
    return bit;
}

static void DECODE_INT_FIMAGE(int *symb, int max)
                             /* Value of read symbol */
                             /* Half of maximum value for symbol */
{
    int bit;

    *symb = 0;
    while (max > 0)
    {
        *symb <<= 1;
        bit = READ_BIT_FIMAGE();
        if (bit)
            *symb += 1;
        max /= 2;
    }

}

static float INT2FLOAT(int n, int nbitstep)
{
    float f;
    int logf;
    double logstep, fstep;

    logf = n / ((int) 1 << nbitstep);
    if (logf == 0)
        f = (double) n / ((int) 1 << nbitstep);
    else
    {
        logstep = LOG_STEP;
        fstep =
            ((double) ((int) 1 << ((int) logstep * logf)) -
             ((int) 1 << ((int) logstep * (logf - 1)))) /
            (double) ((int) 1 << nbitstep);
        f = ((int) 1 << ((int) logstep * (logf - 1))) + (n -
                                                         (1 << nbitstep) *
                                                         logf + 1.0) * fstep;
    }
    return (f);
}

static void
READ_HEADER_FIMAGE(int *print, int *nrow, int *ncol, int *nrow1, int *ncol1,
                   float *stepsize, int *nstep, int *center, int *minstep,
                   float *ashift, double *rate, Cimage compress)
                                 /* Do not print the size of image */
                                 /* Size of image specified by command line */
                                 /* Size of image */
                                 /* Cell width */
                                 /* Lowest quantization step shift */
                                 /* Number of quantization level */
                                 /* Flag for centering of quantization step */
                                 /* Index of lowest cell */
                                 /* Bit rate in compress */
                                 /* Compressed image */
{
    int c_stepsize;             /* Cell width code */
    int c_ashift;               /* Index of lowest quantization step shift */
    int n;

  /*--- Init decoding of compress ---*/

    sizecomp = compress->nrow * compress->ncol;
    if (compress->firstcol > 0)
    {
        ncwreadfi = compress->firstcol;
        ptrc = compress->gray + ncwreadfi - 1;
    }
    else
    {
        ncwreadfi = 1;
        ptrc = compress->gray;
    }
    if (compress->firstrow > 0)
    {
        bits_to_go = compress->firstrow;
        buffer = *ptrc >> (8 - bits_to_go);
    }
    else
    {
        bits_to_go = 8;
        buffer = *ptrc;
    }

    *rate = ((double) NBIT_NSTEP + 1.0) / ((double) 8.0 * *nrow1 * *ncol1);

  /*--- Read size of image ---*/

    if (nrow)
        *ncol1 = *nrow1 = *nrow;
    else if (!ncol)
        DECODE_INT_FIMAGE(nrow1, 1 << (NBIT_SIZEIM - 1));
    if (ncol)
    {
        *ncol1 = *ncol;
        if (!nrow)
            *nrow1 = *ncol;
    }
    else if (!nrow)
        DECODE_INT_FIMAGE(ncol1, 1 << (NBIT_SIZEIM - 1));
    if (!print)
        printf("Size of image : %d, %d\n", *nrow1, *ncol1);
    if (!ncol && !nrow)
        *rate += (double) NBIT_SIZEIM / (4.0 * *nrow1 * *ncol1);

  /*--- Read number of quantization steps ---*/

    DECODE_INT_FIMAGE(nstep, 1 << (NBIT_NSTEP - 1));

  /*--- Check if quantization steps have been centered ---*/

    DECODE_INT_FIMAGE(center, 1);

    if (!print)
        printf("Number of steps : %d\n", *nstep);

    if (*center == 1)
    {

        if (*nstep > 1)
        {

      /*--- Read sign of lower quantization step ---*/

            DECODE_INT_FIMAGE(&n, 1);

      /*--- Read index of lower quantization step ---*/

            DECODE_INT_FIMAGE(minstep, 1 << (NBIT_MINSTEP - 1));

            if (n == 1)
                *minstep = -*minstep;

            if (!print)
                printf
                    ("Quantization steps centered\nIndex of lowest step : %d\n",
                     *minstep);

            *rate =
                ((double) NBIT_MINSTEP +
                 1.0) / ((double) 8.0 * *nrow1 * *ncol1);

        }

    }
    else
    {

      /*--- Read sign of shift of lower quantization step  ---*/

        DECODE_INT_FIMAGE(&n, 1);

      /*--- Read index of shift of lower quantization step  ---*/

        DECODE_INT_FIMAGE(&c_ashift, 1 << (NBIT_LOG + NBIT_STEP - 1));
        *ashift = INT2FLOAT(c_ashift, NBIT_STEP);

        if (n == 1)
            *ashift = -*ashift;

        if (!print)
            printf
                ("Quantization steps not centered\n"
                 "Shift for lowest step : %.6f\n", *ashift);

        *rate =
            ((double) NBIT_LOG + NBIT_STEP +
             1.0) / ((double) 8.0 * *nrow1 * *ncol1);

    }

    if (*nstep > 1)
    {

    /*--- Read quantization step width ---*/

        DECODE_INT_FIMAGE(&c_stepsize, 1 << (NBIT_LOG + NBIT_STEP - 1));
        *stepsize = INT2FLOAT(c_stepsize, NBIT_STEP);

        if (!print)
            printf("Step width : %.6f\n", *stepsize);

        *rate =
            ((double) NBIT_LOG +
             NBIT_STEP) / ((double) 8.0 * *nrow1 * *ncol1);

    }
    else
        *stepsize = 0.0;

    if (bits_to_go == 0)
        ncwreadfi++;

    compress->firstrow = bits_to_go;
    compress->firstcol = ncwreadfi;
}

static void
SCALAR_RECONSTRUCT(int *print, int *nrow, int *ncol, Cimage compress,
                   Fimage result, double *rate)
                                /* Control info print */
                                /* Number of rows and columns in
                                 * reconstructed image (if not selected,
                                 * info is read in header of Compress) */
                                /* Reconstructed image */
                                /* Compressed `Image` */
                                /* Bit rate */
{
    int center;                 /* Flag for centering of quantization step */
    float stepsize;             /* Cell width */
    float ashift = 0.0;
    int step, minstep = 0;      /* Index of cell, min cell */
    int nstep;                  /* Number of quantization level */
    long i;
    long isize;                 /* Number of pixel in `image` */
    int nrow1, ncol1;           /* Number of rows and columns in
                                 * reconstructed image (if not selected,
                                 * info is read in header of Compress) */
    int *predic, predic1;
    double rate1;               /* Rate for each codebook */
    Fimage symbol;              /* Qunatization symbols buffer */

  /*--- Read information in heaeder of compressed file ---*/

    READ_HEADER_FIMAGE(print, nrow, ncol, &nrow1, &ncol1, &stepsize, &nstep,
                       &center, &minstep, &ashift, rate, compress);

  /*--- Memory allocation for quantized image ---*/

    result = mw_change_fimage(result, nrow1, ncol1);
    if (result == NULL)
        mwerror(FATAL, 1, "Not enough memory for reconstructed image.\n");
    isize = (long) nrow1 *ncol1;

  /*--- Decoding of symbol buffer ---*/

    if (nstep <= 32)
        predic1 = 1;
    else
        predic1 = 0;
    predic = &predic1;

    if (nstep > 1)
    {
        symbol = mw_new_fimage();
        ardecode2(&nstep, &nrow1, &nstep, NULL, predic, NULL, compress,
                  &rate1, symbol);
        *rate = rate1;
    }
    else
        symbol = NULL;

  /*--- Reconstruction of image ---*/

    if (nstep > 1)
        if (center)
        {
            for (i = 0; i < isize; i++)
            {
                step = floor(symbol->gray[i] + .5) + minstep;
                result->gray[i] = (float) step *stepsize;
            }
        }
        else
        {
            for (i = 0; i < isize; i++)
            {
                step = floor(symbol->gray[i] + .5);
                result->gray[i] = step * stepsize + ashift;
            }
        }
    else if (center)
        for (i = 0; i < isize; i++)
            result->gray[i] = (float) 0.0;
    else
        for (i = 0; i < isize; i++)
            result->gray[i] = ashift;

    if (symbol)
        mw_delete_fimage(symbol);
}

void
fiscalq(int *Print, int *NRow, int *NCol, Cimage Compress, Fimage Result,
        double *Rate)
                                /* Control of information print */
                                /* Number of rows and columns in
                                 * reconstructed image (if not selected,
                                 * info is read in header of Compress) */
                                /* Input compressed image */
                                /* Reconstructed image */
                                /* Bit rate for Compress */
{

    if (!Rate)
        Rate = (double *) malloc(sizeof(double));
    SCALAR_RECONSTRUCT(Print, NRow, NCol, Compress, Result, Rate);

}

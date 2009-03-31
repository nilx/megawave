/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {fivq};
version = {"1.00"};
author = {"Jean-Pierre D'Ales"};
function = {"Reconstruction of a vector quantized image"};
usage = {
'p':Print->Print
        "Do not print info on reconstrction process",
'r':NRow->NRow
        "Number of rows in reconstructed image",
'h':NCol->NCol
        "Number of columns in reconstructed image",
'x':CodeBook2->CodeBook2
        "Sequence of codebooks for second class (fimage)",
'y':CodeBook3->CodeBook3
        "Sequence of codebooks for third class (fimage)",
'z':CodeBook4->CodeBook4
        "Sequence of codebooks for fourth class (fimage)",
'a':ResCodeBook1->ResCodeBook1
        "Codebook for residu quantization
        after CodeBook1 quantization (fimage)",
'b':ResCodeBook2->ResCodeBook2
        "Codebook for residu quantization
        after CodeBook2 quantization (fimage)",
'c':ResCodeBook3->ResCodeBook3
        "Codebook for residu quantization
        after CodeBook3 quantization (fimage)",
'd':ResCodeBook4->ResCodeBook4
        "Codebook for residu quantization
        after CodeBook4 quantization (fimage)",
'e':ResResCodeBook1->ResResCodeBook1
        "Codebook for residu quantization
        after CodeBook1 and ResCodeBook1 quantization (fimage)",
'f':ResResCodeBook2->ResResCodeBook2
        "Codebook for residu quantization
        after CodeBook2 and ResCodeBook2 quantization (fimage)",
Compress->Compress
        "Compressed image (cimage)",
CodeBook1->CodeBook1
        "Sequence of codebooks for first class (fimage)",
Result<-Result
        "Reconstructed image (fimage)",
  {
    Rate<-Rate
        "Compression rate"
  }
};
 */

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for ardecode2() */

/*--- Constants ---*/

#define MAX_ADAP 4              /* Maximum number of classes */
#define NBIT_SIZEIM 15          /* Number of bits dedicated to encode
                                 * the dimensions of image */

/*--- Global variables ---*/

typedef int bufind[MAX_ADAP];

static long height, width;      /* Height and width of vectors */
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
static int nadapcb;             /* Number of levels for adaptive
                                 * quantization */

static void
CHECK_INPUT(int testmulticb, Fimage codebook1, Fimage codebook2,
            Fimage codebook3, Fimage codebook4, Fimage rescodebook1,
            Fimage rescodebook2, Fimage rescodebook3, Fimage rescodebook4,
            Fimage resrescodebook1, Fimage resrescodebook2, Fimage image)
/* Codebooks for different classes */
/* Residu codebooks for different classes */
/* Residu codebooks
 * for different classes */
/* Input image */
{
    int sizeb;
    float testsize;             /* Control for codebook structure */

  /*--- Test dimensions of vectors and size of codebook files ---*/

    if (codebook1->nrow <= 4)
        mwerror(FATAL, 2, "Bad number (<= 4) of row in CodeBook1!\n");

    sizeb = codebook1->ncol;
    height = floor(codebook1->gray[(codebook1->nrow - 2) * sizeb] + .5);
    if (((float) height != codebook1->gray[(codebook1->nrow - 2) * sizeb])
        || (height <= 0))
        mwerror(WARNING, 0,
                "Bad value for height of vectors in CodeBook1!\n");
    if (sizeb % height != 0)
        mwerror(FATAL, 2, "Dimension and height of vector are incompatible!");
    width = sizeb / height;

    if (codebook2)
    {
        if (codebook2->nrow <= 4)
            mwerror(FATAL, 2, "Bad number (<= 4) of row in CodeBook2!\n");
        if (codebook2->ncol != sizeb)
            mwerror(FATAL, 2,
                    "Numbers of columns are different "
                    "in CodeBook1 and CodeBook2!\n");
        if ((float) height != codebook2->gray[(codebook2->nrow - 2) * sizeb])
            mwerror(WARNING, 0,
                    "Bad value for height of vectors in CodeBook2!\n");
    }

    if (codebook3)
    {
        if (codebook3->nrow <= 4)
            mwerror(FATAL, 2, "Bad number (<= 4) of row in CodeBook3!\n");
        if (codebook3->ncol != sizeb)
            mwerror(FATAL, 2,
                    "Numbers of columns are different "
                    "in CodeBook1 and CodeBook3!\n");
        if ((float) height != codebook3->gray[(codebook3->nrow - 2) * sizeb])
            mwerror(WARNING, 0,
                    "Bad value for height of vectors in CodeBook3!\n");
    }

    if (codebook4)
    {
        if (codebook4->nrow <= 4)
            mwerror(FATAL, 2, "Bad number (<= 4) of row in CodeBook4!\n");
        if (codebook4->ncol != sizeb)
            mwerror(FATAL, 2,
                    "Numbers of columns are different "
                    "in CodeBook1 and CodeBook4!\n");
        if ((float) height != codebook4->gray[(codebook4->nrow - 2) * sizeb])
            mwerror(WARNING, 0,
                    "Bad value for height of vectors in CodeBook4!\n");
    }

    if (rescodebook1)
    {
        if (rescodebook1->nrow <= 4)
            mwerror(FATAL, 2, "Bad number (<= 4) of row in ResCodeBook1!\n");
        if (rescodebook1->ncol != sizeb)
            mwerror(FATAL, 2,
                    "Numbers of columns are different "
                    "in ResCodeBook1 and CodeBook1!\n");
        if ((float) height !=
            rescodebook1->gray[(rescodebook1->nrow - 2) * sizeb])
            mwerror(WARNING, 0,
                    "Bad value for height of vectors in ResCodeBook1!\n");
    }

    if (rescodebook2)
    {
        if (rescodebook2->nrow <= 4)
            mwerror(FATAL, 2, "Bad number (<= 4) of row in ResCodeBook2!\n");
        if (rescodebook2->ncol != sizeb)
            mwerror(FATAL, 2,
                    "Numbers of columns are different "
                    "in CodeBook1 and ResCodeBook2!\n");
        if ((float) height !=
            rescodebook2->gray[(rescodebook2->nrow - 2) * sizeb])
            mwerror(WARNING, 0,
                    "Bad value for height of vectors in ResCodeBook2!\n");
    }

    if (rescodebook3)
    {
        if (rescodebook3->nrow <= 4)
            mwerror(FATAL, 2, "Bad number (<= 4) of row in ResCodeBook3!\n");
        if (rescodebook3->ncol != sizeb)
            mwerror(FATAL, 2,
                    "Numbers of columns are different "
                    "in CodeBook1 and ResCodeBook3!\n");
        if ((float) height !=
            rescodebook3->gray[(rescodebook3->nrow - 2) * sizeb])
            mwerror(WARNING, 0,
                    "Bad value for height of vectors in ResCodeBook3!\n");
    }

    if (rescodebook4)
    {
        if (rescodebook4->nrow <= 4)
            mwerror(FATAL, 2, "Bad number (<= 4) of row in ResCodeBook4!\n");
        if (rescodebook4->ncol != sizeb)
            mwerror(FATAL, 2,
                    "Numbers of columns are different "
                    "in CodeBook1 and ResCodeBook4!\n");
        if ((float) height !=
            rescodebook4->gray[(rescodebook4->nrow - 2) * sizeb])
            mwerror(WARNING, 0,
                    "Bad value for height of vectors in ResCodeBook4!\n");
    }

    if (resrescodebook1)
    {
        if (resrescodebook1->nrow <= 4)
            mwerror(FATAL, 2,
                    "Bad number (<= 4) of row in ResResCodeBook1!\n");
        if (resrescodebook1->ncol != sizeb)
            mwerror(FATAL, 2,
                    "Numbers of columns are different "
                    "in ResResCodeBook1 and CodeBook1!\n");
        if ((float) height !=
            resrescodebook1->gray[(resrescodebook1->nrow - 2) * sizeb])
            mwerror(WARNING, 0,
                    "Bad value for height of vectors in ResResCodeBook1!\n");
    }

    if (resrescodebook2)
    {
        if (resrescodebook2->nrow <= 4)
            mwerror(FATAL, 2,
                    "Bad number (<= 4) of row in ResResCodeBook2!\n");
        if (resrescodebook2->ncol != sizeb)
            mwerror(FATAL, 2,
                    "Numbers of columns are different "
                    "in CodeBook1 and ResResCodeBook2!\n");
        if ((float) height !=
            resrescodebook2->gray[(resrescodebook2->nrow - 2) * sizeb])
            mwerror(WARNING, 0,
                    "Bad value for height of vectors in ResResCodeBook2!\n");
    }

  /*--- Test if codebook files contain one or several codebooks ---*/

    if (testmulticb == 1)
    {
        if (codebook1->nrow <= 6)
            mwerror(FATAL, 2, "Bad number (<= 6) of row in CodeBook1!\n");

        testsize = (float) floor(codebook1->gray[(codebook1->nrow - 6) *
                                                 sizeb] + .5);
        if ((codebook1->gray[(codebook1->nrow - 6) * sizeb] != testsize)
            || (testsize < 1.0))
            mwerror(FATAL, 2, "Bad final size in CodeBook1!\n");

        testsize = (float) floor(codebook1->gray[(codebook1->nrow - 5) *
                                                 sizeb] + .5);
        if ((codebook1->gray[(codebook1->nrow - 5) * sizeb] != testsize)
            || (testsize < 1.0))
            mwerror(FATAL, 2, "Bad initial size in CodeBook1!\n");

        if (codebook2)
        {
            if (codebook2->nrow <= 6)
                mwerror(FATAL, 2, "Bad number (<= 6) of row in CodeBook2!\n");

            testsize = (float) floor(codebook2->gray[(codebook2->nrow - 6) *
                                                     sizeb] + .5);
            if ((codebook2->gray[(codebook2->nrow - 6) * sizeb] != testsize)
                || (testsize < 1.0))
                mwerror(FATAL, 0, "Bad final size in CodeBook2!\n");

            testsize = (float) floor(codebook2->gray[(codebook2->nrow - 5) *
                                                     sizeb] + .5);
            if ((codebook2->gray[(codebook2->nrow - 5) * sizeb] != testsize)
                || (testsize < 1.0))
                mwerror(WARNING, 0, "Bad initial size in CodeBook2!\n");
        }

        if (codebook3)
        {
            if (codebook3->nrow <= 6)
                mwerror(FATAL, 2, "Bad number (<= 6) of row in CodeBook3!\n");

            testsize = (float) floor(codebook3->gray[(codebook3->nrow - 6) *
                                                     sizeb] + .5);
            if ((codebook3->gray[(codebook3->nrow - 6) * sizeb] != testsize)
                || (testsize < 1.0))
                mwerror(FATAL, 0, "Bad final size in CodeBook3!\n");

            testsize = (float) floor(codebook3->gray[(codebook3->nrow - 5) *
                                                     sizeb] + .5);
            if ((codebook3->gray[(codebook3->nrow - 5) * sizeb] != testsize)
                || (testsize < 1.0))
                mwerror(WARNING, 0, "Bad initial size in CodeBook3!\n");
        }

        if (codebook4)
        {
            if (codebook4->nrow <= 6)
                mwerror(FATAL, 2, "Bad number (<= 6) of row in CodeBook4!\n");

            testsize = (float) floor(codebook4->gray[(codebook4->nrow - 6) *
                                                     sizeb] + .5);
            if ((codebook4->gray[(codebook4->nrow - 6) * sizeb] != testsize)
                || (testsize < 1.0))
                mwerror(FATAL, 0, "Bad final size in CodeBook4!\n");

            testsize = (float) floor(codebook4->gray[(codebook4->nrow - 5) *
                                                     sizeb] + .5);
            if ((codebook4->gray[(codebook4->nrow - 5) * sizeb] != testsize)
                || (testsize < 1.0))
                mwerror(WARNING, 0, "Bad initial size in CodeBook4!\n");
        }

        if (rescodebook1)
        {
            if (rescodebook1->nrow <= 6)
                mwerror(FATAL, 2,
                        "Bad number (<= 6) of row in ResCodeBook1!\n");

            testsize = (float) floor(rescodebook1->gray[(rescodebook1->nrow
                                                         - 6) * sizeb] + .5);
            if ((rescodebook1->gray[(rescodebook1->nrow - 6) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(FATAL, 2, "Bad final size in ResCodeBook1!\n");

            testsize = (float) floor(rescodebook1->gray[(rescodebook1->nrow
                                                         - 5) * sizeb] + .5);
            if ((rescodebook1->gray[(rescodebook1->nrow - 5) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(FATAL, 2, "Bad initial size in ResCodeBook1!\n");
        }

        if (rescodebook2)
        {
            if (rescodebook2->nrow <= 6)
                mwerror(FATAL, 2,
                        "Bad number (<= 6) of row in ResCodeBook2!\n");

            testsize = (float) floor(rescodebook2->gray[(rescodebook2->nrow
                                                         - 6) * sizeb] + .5);
            if ((rescodebook2->gray[(rescodebook2->nrow - 6) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(FATAL, 0, "Bad final size in ResCodeBook2!\n");

            testsize = (float) floor(rescodebook2->gray[(rescodebook2->nrow
                                                         - 5) * sizeb] + .5);
            if ((rescodebook2->gray[(rescodebook2->nrow - 5) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(WARNING, 0, "Bad initial size in ResCodeBook2!\n");
        }

        if (rescodebook3)
        {
            if (rescodebook3->nrow <= 6)
                mwerror(FATAL, 2,
                        "Bad number (<= 6) of row in ResCodeBook3!\n");

            testsize = (float) floor(rescodebook3->gray[(rescodebook3->nrow
                                                         - 6) * sizeb] + .5);
            if ((rescodebook3->gray[(rescodebook3->nrow - 6) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(FATAL, 0, "Bad final size in ResCodeBook3!\n");

            testsize = (float) floor(rescodebook3->gray[(rescodebook3->nrow
                                                         - 5) * sizeb] + .5);
            if ((rescodebook3->gray[(rescodebook3->nrow - 5) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(WARNING, 0, "Bad initial size in ResCodeBook3!\n");
        }

        if (rescodebook4)
        {
            if (rescodebook4->nrow <= 6)
                mwerror(FATAL, 2,
                        "Bad number (<= 6) of row in ResCodeBook4!\n");

            testsize = (float) floor(rescodebook4->gray[(rescodebook4->nrow
                                                         - 6) * sizeb] + .5);
            if ((rescodebook4->gray[(rescodebook4->nrow - 6) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(FATAL, 0, "Bad final size in ResCodeBook4!\n");

            testsize = (float) floor(rescodebook4->gray[(rescodebook4->nrow
                                                         - 5) * sizeb] + .5);
            if ((rescodebook4->gray[(rescodebook4->nrow - 5) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(WARNING, 0, "Bad initial size in ResCodeBook4!\n");
        }

        if (resrescodebook1)
        {
            if (resrescodebook1->nrow <= 6)
                mwerror(FATAL, 2,
                        "Bad number (<= 6) of row in ResResCodeBook1!\n");

            testsize = (float)
                floor(resrescodebook1->gray
                      [(resrescodebook1->nrow - 6) * sizeb] + .5);
            if ((resrescodebook1->gray[(resrescodebook1->nrow - 6) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(FATAL, 2, "Bad final size in ResResCodeBook1!\n");

            testsize = (float)
                floor(resrescodebook1->gray
                      [(resrescodebook1->nrow - 5) * sizeb] + .5);
            if ((resrescodebook1->gray[(resrescodebook1->nrow - 5) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(FATAL, 2, "Bad initial size in ResResCodeBook1!\n");
        }

        if (resrescodebook2)
        {
            if (resrescodebook2->nrow <= 6)
                mwerror(FATAL, 2,
                        "Bad number (<= 6) of row in ResResCodeBook2!\n");

            testsize = (float)
                floor(resrescodebook2->gray
                      [(resrescodebook2->nrow - 6) * sizeb] + .5);
            if ((resrescodebook2->gray[(resrescodebook2->nrow - 6) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(FATAL, 0, "Bad final size in ResResCodeBook2!\n");

            testsize = (float)
                floor(resrescodebook2->gray
                      [(resrescodebook2->nrow - 5) * sizeb] + .5);
            if ((resrescodebook2->gray[(resrescodebook2->nrow - 5) * sizeb] !=
                 testsize) || (testsize < 1.0))
                mwerror(WARNING, 0, "Bad initial size in ResResCodeBook2!\n");
        }
    }

  /*---Check Threshold value in codebook files ---*/

    if (codebook1->gray[(codebook1->nrow - 3) * sizeb] < 0.0)
        mwerror(WARNING, 0, "Negative value for threshold in CodeBook1!\n");
    if (codebook2)
    {
        if (codebook2->gray[(codebook2->nrow - 4) * sizeb] < 0.0)
            mwerror(WARNING, 0,
                    "Negative value for upper threshold in CodeBook2!\n");
        if (codebook2->gray[(codebook2->nrow - 3) * sizeb] < 0.0)
            mwerror(WARNING, 0,
                    "Negative value for lower threshold in CodeBook2!\n");
        if (codebook2->gray[(codebook2->nrow - 4) * sizeb] !=
            codebook1->gray[(codebook1->nrow - 3) * sizeb])
            mwerror(WARNING, 0,
                    "Lower and upper threshold not equal "
                    "in CodeBook1 and CodeBook2!\n");

        if (codebook3)
        {
            if (codebook3->gray[(codebook3->nrow - 4) * sizeb] < 0.0)
                mwerror(WARNING, 0,
                        "Negative value for upper threshold in CodeBook3!\n");
            if (codebook3->gray[(codebook3->nrow - 3) * sizeb] < 0.0)
                mwerror(WARNING, 0,
                        "Negative value for lower threshold in CodeBook3!\n");
            if (codebook3->gray[(codebook3->nrow - 4) * sizeb] !=
                codebook2->gray[(codebook2->nrow - 3) * sizeb])
                mwerror(WARNING, 0,
                        "Lower and upper threshold not equal "
                        "in CodeBook2 and CodeBook3!\n");

            if (codebook4)
            {
                if (codebook4->gray[(codebook4->nrow - 4) * sizeb] < 0.0)
                    mwerror(WARNING, 0,
                            "Negative value for upper threshold "
                            "in CodeBook4!\n");
                if (codebook4->gray[(codebook4->nrow - 3) * sizeb] < 0.0)
                    mwerror(WARNING, 0,
                            "Negative value for lower threshold "
                            "in CodeBook4!\n");
                if (codebook4->gray[(codebook4->nrow - 4) * sizeb] !=
                    codebook3->gray[(codebook3->nrow - 3) * sizeb])
                    mwerror(WARNING, 0,
                            "Lower and upper threshold not equal "
                            "in CodeBook3 and CodeBook4!\n");
            }
        }
    }

  /*--- Check compatiblity of image and vector sizes ---*/

    if ((image->nrow % height != 0) || (image->ncol % width != 0))
        mwerror(FATAL, 2,
                "Image dimensions are not multiples of vector dimensions!\n");

}

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

static void
READ_HEADER_FIMAGE(int *print, int *nrow, int *ncol, int *nrow1, int *ncol1,
                   int *testmulticb, double *rate, int *indexcb,
                   Cimage compress)
                                 /* Do not print the size of image */
                                 /* Size of image specified by command line */
                                 /* Size of image */
                                 /* Flag for multiple codebooks in codebook
                                  * files */
                                 /* Bit rate in compress */
                                 /* Indices of selected codebooks
                                  * for quantization */
                                 /* Compressed image */
{
    int n;
    int ind;

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

    *rate = 0.0;

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
        *rate += (double) NBIT_SIZEIM / ((double) 4.0 * *nrow1 * *ncol1);

  /*--- Read number of classes in vector quantization ---*/

    DECODE_INT_FIMAGE(&nadapcb, 4);
    if (!print)
        printf("Number of codebooks : %d\n", nadapcb);

  /*--- Check if there are different codebooks for each class ---*/

    DECODE_INT_FIMAGE(testmulticb, 1);
    *rate += (double) 0.5 / (*nrow1 * *ncol1);
    if (*testmulticb == 1)
    {

    /*--- Read index of codebook for each class ---*/

        if (!print)
            printf("Indices of codebooks : ");
        for (n = 0; n < nadapcb; n++)
        {
            DECODE_INT_FIMAGE(&ind, 32);
            indexcb[n] = ind;
            if (!print)
                printf(" %d,", ind);
        }
        for (n = nadapcb; n < MAX_ADAP; n++)
            indexcb[n] = 0;

        if (!print)
            printf("\n");
        *rate += (double) nadapcb *0.75 / (*nrow1 * *ncol1);
    }

    if (bits_to_go == 0)
        ncwreadfi++;

    compress->firstrow = bits_to_go;
    compress->firstcol = ncwreadfi;
}

static int count_cb(Fimage codebook)
{
    long size, sizei, sizef;
    int n;

    if (codebook)
    {
        sizei = floor(codebook->gray[(codebook->nrow - 5) *
                                     codebook->ncol] + .5);
        sizef = floor(codebook->gray[(codebook->nrow - 6) *
                                     codebook->ncol] + .5);
        n = 2;
        size = 1;
        while (size <= sizei)
            size *= 2;
        while (size < sizef)
        {
            size *= 2;
            n++;
        }
    }
    else
        n = 0;

    return (n);
}

static void extract_cb(Fimage codebook, Fimage cb, int n)
{
    long size, sizei, sizef;
    long xshift;
    long x, xi, xf;
    int n1;

    sizei = floor(codebook->gray[(codebook->nrow - 5) * codebook->ncol] + .5);
    sizef = floor(codebook->gray[(codebook->nrow - 6) * codebook->ncol] + .5);

    if (n == 0)
    {
        xi = sizef;
        xf = sizef + sizei;
    }
    else
    {
        size = 1;
        while (size < sizei)
            size *= 2;
        if (size == sizei)
            size *= 2;
        xi = sizef + sizei;
        n1 = 1;
        while ((n1 < n) && (size * 2 < sizef))
        {
            xi += size;
            size *= 2;
            n1++;
        }
        if (n1 < n)
        {
            xi = 0;
            xf = sizef;
        }
        else
            xf = xi + size;
    }

    cb = mw_change_fimage(cb, xf - xi + 4, codebook->ncol);
    if (cb == NULL)
        mwerror(FATAL, 1, "Not enough memory for extracted codebook.\n");

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
BLOCK_RECONSTRUCT(int i, int j, Fimage codebook, Fimage rescodebook,
                  Fimage resrescodebook, Fimage result, Fimage symbol,
                  Fimage symbres, Fimage symbresres, long int recfac)
                                /* Row and column indices of block */
                                /* Codebook for reconstruction */
                                             /* First and second level
                                              * residual codebooks */
                                /* Reconstructed image */
                                             /* Buffer of symbols of vectors
                                              * to be used for reconstruction */
{
    int rb, cb;                 /* Indices for row and column of pixel
                                 * in the block */
    int obr, rbr;               /* Index of the first component of
                                 * (the line of) the block in result */
    int obcb, rbcb;             /* Index of the first component of
                                 * (the line of) a block in codebook */
    int sizec;                  /* Size of codebook */
    int ncol;                   /* Number of columns in result */

    sizec = codebook->nrow - 4;
    ncol = result->ncol;
    obr = i * height * result->ncol + j * width;
    if (sizec > 1)
        if (recfac >= 1)
            obcb =
                (((int) symbol->gray[symbol->firstrow]) / recfac) *
                codebook->ncol;
        else
            obcb = (int) symbol->gray[symbol->firstrow] * codebook->ncol;
    else
        obcb = 0;

    rb = 0;
    rbr = obr;
    rbcb = obcb;
    while (rb < height)
    {
        for (cb = 0; cb < width; cb++)
            result->gray[rbr + cb] += codebook->gray[rbcb + cb];
        rbcb += width;
        rbr += ncol;
        rb++;
    }

    if (rescodebook)
    {
        if (symbres || symbresres)
            BLOCK_RECONSTRUCT(i, j, rescodebook, resrescodebook, NULL, result,
                              symbres, symbresres, NULL, recfac * sizec);
        else
            BLOCK_RECONSTRUCT(i, j, rescodebook, resrescodebook, NULL, result,
                              symbol, NULL, NULL, recfac * sizec);
    }
    if (symbol && (sizec > 1))
        symbol->firstrow += 1;
}

static void
PLAIN_RECONSTRUCT(Fimage codebook, Fimage rescodebook, Fimage resrescodebook,
                  Cimage compress, Fimage result, double *rate)
                                /* Type of lossless encoding applied
                                 * to symbols*/
                                /* Codebook for reconstruction */
                                /* Compressed image */
                                /* Reconstructed image */
                                /* Bit rate in compress */
{
    int r, c;
    int nrow, ncol;
    int nrowb, ncolb;
    int sizec, sizecres, sizecresres;
    Fimage symbol, symbres, symbresres;
    int *predic, predic1;
    double rate1;               /* Rate for each codebook */

    nrow = result->nrow;
    ncol = result->ncol;
    nrowb = nrow / height;
    ncolb = ncol / width;
    sizec = codebook->nrow - 4;
    symbol = symbres = symbresres = NULL;

    if (rescodebook)
    {
        sizecres = rescodebook->nrow - 4;

        if (resrescodebook)
        {
            sizecresres = resrescodebook->nrow - 4;
        }
    }

    if (sizec <= 32)
        predic1 = 1;
    else
        predic1 = 0;
    predic = &predic1;

    if (sizec > 1)
    {
        symbol = mw_new_fimage();
        ardecode2(&sizec, &nrowb, &sizec, NULL, predic, NULL, compress,
                  &rate1, symbol);
        if (symbol->ncol != ncolb)
            mwerror(FATAL, 3, "Bad number of column for symbol buffer!\n");
        *rate = rate1;
        symbol->firstrow = 0;
    }

    if (rescodebook)
    {

        if (sizecres <= 32)
            predic1 = 1;
        else
            predic1 = 0;

        if (sizecres > 1)
        {
            symbres = mw_new_fimage();
            ardecode2(&sizec, &nrowb, &sizecres, NULL, predic, NULL, compress,
                      &rate1, symbres);
            if (symbres->ncol != ncolb)
                mwerror(FATAL, 3,
                        "Bad number of column for symbol buffer (residues)!\n");
            *rate += rate1;
            symbres->firstrow = 0;
        }

        if (resrescodebook)
        {

            if (sizecresres <= 32)
                predic1 = 1;
            else
                predic1 = 0;

            if (sizecresres > 1)
            {
                symbresres = mw_new_fimage();
                ardecode2(&sizec, &nrowb, &sizecresres, NULL, predic, NULL,
                          compress, &rate1, symbresres);
                if (symbresres->ncol != ncolb)
                    mwerror(FATAL, 3,
                            "Bad number of column "
                            "for symbol buffer (residues)!\n");
                *rate += rate1;
                symbresres->firstrow = 0;
            }

        }
    }

    for (r = 0; r < nrowb; r++)
        for (c = 0; c < ncolb; c++)
            BLOCK_RECONSTRUCT(r, c, codebook, rescodebook, resrescodebook,
                              result, symbol, symbres, symbresres, 0L);

    if (symbres)
    {
        if (symbresres)
            mw_delete_fimage(symbresres);
        mw_delete_fimage(symbres);
    }
    if (symbol)
        mw_delete_fimage(symbol);

}

static void
ADAP_RECONSTRUCT(Fimage codebook1, Fimage codebook2, Fimage codebook3,
                 Fimage codebook4, Fimage rescodebook1, Fimage rescodebook2,
                 Fimage rescodebook3, Fimage rescodebook4,
                 Fimage resrescodebook1, Fimage resrescodebook2,
                 Cimage compress, Fimage result, double *rate)
/* Type of lossless encoding applied to symbols*/
/* Codebooks for reconstruction */
/* Compressed image */
/* Reconstructed image */
/* Bit rate in compress */
{
    int rb, cb;
    int sizev;
    int nrowb, ncolb;
    int nrow, ncol;
    long x, sizeb;
    long num1, num2, num3, num4, numthres;
    float thres1, thres2, thres3;
    double rate1;               /* Rate for each codebook */
    Fimage indexcb;             /* Codebook index buffer */
    float indexcbval[5];        /* Symbol value for each codebook */
    int *predic, predic1;
    int nsymb;
    Fimage symbol, symbres, symbresres;
    int testthres;
    int testsymb;

    sizev = codebook1->ncol;
    nrow = result->nrow;
    ncol = result->ncol;
    nrowb = nrow / height;
    ncolb = ncol / width;
    sizeb = nrowb * ncolb;
    num1 = num2 = num3 = num4 = numthres = 0;
    nsymb = 2;
    thres1 = codebook1->gray[(codebook1->nrow - 3) * sizev];
    thres2 = thres3 = 0.0;
    testthres = 0;
    symbol = symbres = symbresres = NULL;
    indexcbval[0] = 0.0;
    indexcbval[1] = 1.0;
    indexcbval[2] = 2.0;
    indexcbval[3] = 3.0;
    indexcbval[4] = 0.0;

  /*--- Compute number of codebooks ---*/

    if (codebook2)
    {
        thres2 = codebook2->gray[(codebook2->nrow - 3) * sizev];
        if (thres2 > 0.0)
            nsymb++;
        else
            indexcbval[2] = 0.0;
        if ((thres2 > 0.0) && !codebook3)
            testthres = 1;

    }
    else
        testthres = 1;

    if (codebook3)
    {
        thres3 = codebook3->gray[(codebook3->nrow - 3) * sizev];
        if (thres3 > 0.0)
            nsymb++;
        else
            indexcbval[3] = 0.0;
        if ((thres3 > 0.0) && !codebook4)
            testthres = 1;

    }

  /*--- Declaration of buffer for symbols buffers ---*/

    testsymb = 0;
    if (codebook1->nrow - 4 > 1)
        testsymb = 1;
    else if (codebook2)
        if (codebook2->nrow - 4 > 1)
            testsymb = 1;
    if (codebook3)
        if (codebook3->nrow - 4 > 1)
            testsymb = 1;
    if (codebook4)
        if (codebook4->nrow - 4 > 1)
            testsymb = 1;
    if (testsymb == 1)
        symbol = mw_new_fimage();

    if (rescodebook1)
        if (rescodebook1->nrow - 4 > 1)
            testsymb = 2;
    if (rescodebook2)
        if (rescodebook2->nrow - 4 > 1)
            testsymb = 2;
    if (rescodebook3)
        if (rescodebook3->nrow - 4 > 1)
            testsymb = 2;
    if (rescodebook4)
        if (rescodebook4->nrow - 4 > 1)
            testsymb = 2;
    if (testsymb == 2)
        symbres = mw_new_fimage();

    if (resrescodebook1)
        if (resrescodebook1->nrow - 4 > 1)
            testsymb = 3;
    if (resrescodebook2)
        if (resrescodebook2->nrow - 4 > 1)
            testsymb = 3;
    if (testsymb == 3)
        symbresres = mw_new_fimage();

  /*--- Declaration of buffer for adapted codebook indices ---*/

    if (testsymb >= 1)
    {

        indexcb = mw_new_fimage();

    /*--- Decoding adapted codebook indices ---*/

        predic1 = 1;
        predic = &predic1;

        ardecode2(&nsymb, &nrowb, &nsymb, NULL, predic, NULL, compress,
                  &rate1, indexcb);
        if (indexcb->ncol != ncolb)
            mwerror(FATAL, 3,
                    "Bad number of column for symbol buffer "
                    "(codebook indices)!\n");
        *rate += rate1;

    /*--- Compute number of thresholded blocks ---*/

        if (testthres == 1)
        {
            for (x = 0; x < sizeb; x++)
                if (indexcb->gray[x] == 0.0)
                    numthres++;
                else if (indexcb->gray[x] == 1.0)
                    num1++;
                else if (indexcb->gray[x] == 2.0)
                    num2++;
                else if (indexcb->gray[x] == 3.0)
                    num3++;
                else
                    mwerror(FATAL, 2,
                            "Something wrong in codebook index buffer!\n");
        }
        else
            for (x = 0; x < sizeb; x++)
                if (indexcb->gray[x] == indexcbval[1])
                    num1++;
                else if (indexcb->gray[x] == indexcbval[2])
                    num2++;
                else if (indexcb->gray[x] == indexcbval[3])
                    num3++;
                else if (indexcb->gray[x] == indexcbval[4])
                    num4++;
                else
                    mwerror(FATAL, 2,
                            "Something wrong in codebook index buffer!\n");

    /*--- Decoding first codebook symbols ---*/

        if (resrescodebook1)
        {
            nsymb = resrescodebook1->nrow - 4;
            if (nsymb <= 32)
                predic1 = 1;
            else
                predic1 = 0;

            if ((nsymb > 1) && (num1 > 0))
            {
                ardecode2(&nsymb, NULL, &nsymb, NULL, predic, NULL, compress,
                          &rate1, symbresres);
                if (symbresres->nrow * symbresres->ncol != num1)
                    mwerror(FATAL, 2,
                            "Bad number of symbols "
                            "for codebook 1 (second level residu) %d/ %d!\n",
                            symbresres->nrow * symbresres->ncol, num1);

                symbresres->firstrow = 0;
                *rate += rate1 * num1 / sizeb;
            }
        }

        if (rescodebook1)
        {
            nsymb = rescodebook1->nrow - 4;
            if (nsymb <= 32)
                predic1 = 1;
            else
                predic1 = 0;

            if ((nsymb > 1) && (num1 > 0))
            {
                ardecode2(&nsymb, NULL, &nsymb, NULL, predic, NULL, compress,
                          &rate1, symbres);
                if (symbres->nrow * symbres->ncol != num1)
                    mwerror(FATAL, 2,
                            "Bad number of symbols for codebook 1 (residu)!\n");
                symbres->firstrow = 0;
                *rate += rate1 * num1 / sizeb;
            }
        }

        nsymb = codebook1->nrow - 4;
        if (nsymb <= 32)
            predic1 = 1;
        else
            predic1 = 0;

        if ((nsymb > 1) && (num1 > 0))
        {
            ardecode2(&nsymb, NULL, &nsymb, NULL, predic, NULL, compress,
                      &rate1, symbol);
            if (symbol->nrow * symbol->ncol != num1)
                mwerror(FATAL, 2, "Bad number of symbols for codebook 1!\n");
            symbol->firstrow = 0;
            *rate += rate1 * num1 / sizeb;
        }

    /*--- First pass for reconstruction of image ---*/

        x = 0;
        for (rb = 0; rb < nrowb; rb++)
            for (cb = 0; cb < ncolb; cb++, x++)
                if (indexcb->gray[x] == indexcbval[1])
                    BLOCK_RECONSTRUCT(rb, cb, codebook1, rescodebook1,
                                      resrescodebook1, result, symbol,
                                      symbres, symbresres, 0L);

        if (codebook2)
        {

      /*--- Decoding second codebook symbols ---*/

            if (resrescodebook2)
            {
                nsymb = resrescodebook2->nrow - 4;
                if (nsymb <= 32)
                    predic1 = 1;
                else
                    predic1 = 0;

                if ((nsymb > 1) && (num2 > 0))
                {
                    ardecode2(&nsymb, NULL, &nsymb, NULL, predic, NULL,
                              compress, &rate1, symbresres);
                    if (symbresres->nrow * symbresres->ncol != num2)
                        mwerror(FATAL, 2,
                                "Bad number of symbols "
                                "for codebook 2 (second level residu)!\n");
                    symbresres->firstrow = 0;
                    *rate += rate1 * num2 / sizeb;
                }
            }

            if (rescodebook2)
            {
                nsymb = rescodebook2->nrow - 4;
                if (nsymb <= 32)
                    predic1 = 1;
                else
                    predic1 = 0;

                if ((nsymb > 1) && (num2 > 0))
                {
                    ardecode2(&nsymb, NULL, &nsymb, NULL, predic, NULL,
                              compress, &rate1, symbres);
                    if (symbres->nrow * symbres->ncol != num2)
                        mwerror(FATAL, 2,
                                "Bad number of symbols "
                                "for codebook 2 (residu)!\n");
                    symbres->firstrow = 0;
                    *rate += rate1 * num2 / sizeb;
                }
            }

            nsymb = codebook2->nrow - 4;
            if (nsymb <= 32)
                predic1 = 1;
            else
                predic1 = 0;

            if ((nsymb > 1) && (num2 > 0))
            {
                ardecode2(&nsymb, NULL, &nsymb, NULL, predic, NULL, compress,
                          &rate1, symbol);
                if (symbol->nrow * symbol->ncol != num2)
                    mwerror(FATAL, 2,
                            "Bad number of symbols " "for codebook 2!\n");
                symbol->firstrow = 0;
                *rate += rate1 * num2 / sizeb;
            }

      /*--- Second pass for reconstruction of image ---*/

            x = 0;
            for (rb = 0; rb < nrowb; rb++)
                for (cb = 0; cb < ncolb; cb++, x++)
                    if (indexcb->gray[x] == indexcbval[2])
                        BLOCK_RECONSTRUCT(rb, cb, codebook2, rescodebook2,
                                          resrescodebook2, result, symbol,
                                          symbres, symbresres, 0L);

        }

        if (codebook3)
        {

      /*--- Decoding third codebook symbols ---*/

            if (rescodebook3)
            {
                nsymb = rescodebook3->nrow - 4;
                if (nsymb <= 32)
                    predic1 = 1;
                else
                    predic1 = 0;

                if ((nsymb > 1) && (num3 > 0))
                {
                    ardecode2(&nsymb, NULL, &nsymb, NULL, predic, NULL,
                              compress, &rate1, symbres);
                    if (symbres->nrow * symbres->ncol != num3)
                        mwerror(FATAL, 2,
                                "Bad number of symbols "
                                "for codebook 3 (residu)!\n");
                    symbres->firstrow = 0;
                    *rate += rate1 * num3 / sizeb;
                }
            }

            nsymb = codebook3->nrow - 4;
            if (nsymb <= 32)
                predic1 = 1;
            else
                predic1 = 0;

            if ((nsymb > 1) && (num3 > 0))
            {
                ardecode2(&nsymb, NULL, &nsymb, NULL, predic, NULL, compress,
                          &rate1, symbol);
                if (symbol->nrow * symbol->ncol != num3)
                    mwerror(FATAL, 2,
                            "Bad number of symbols for codebook 3!\n");
                symbol->firstrow = 0;
                *rate += rate1 * num3 / sizeb;
            }

      /*--- Third pass for reconstruction of image ---*/

            x = 0;
            for (rb = 0; rb < nrowb; rb++)
                for (cb = 0; cb < ncolb; cb++, x++)
                    if (indexcb->gray[x] == indexcbval[3])
                        BLOCK_RECONSTRUCT(rb, cb, codebook3, rescodebook3,
                                          NULL, result, symbol, symbres, NULL,
                                          0L);

        }

        if (codebook4 && (testthres == 0))
        {

      /*--- Decoding fourth codebook symbols ---*/

            if (rescodebook4)
            {
                nsymb = rescodebook4->nrow - 4;
                if (nsymb <= 32)
                    predic1 = 1;
                else
                    predic1 = 0;

                if ((nsymb > 1) && (num4 > 0))
                {
                    ardecode2(&nsymb, NULL, &nsymb, NULL, predic, NULL,
                              compress, &rate1, symbres);
                    if (symbres->nrow * symbres->ncol != num4)
                        mwerror(FATAL, 2,
                                "Bad number of symbols "
                                "for codebook 4 (residu)!\n");
                    symbres->firstrow = 0;
                    *rate += rate1 * num4 / sizeb;
                }
            }

            nsymb = codebook4->nrow - 4;
            if (nsymb <= 32)
                predic1 = 1;
            else
                predic1 = 0;

            if ((nsymb > 1) && (num4 > 0))
            {
                ardecode2(&nsymb, NULL, &nsymb, NULL, predic, NULL, compress,
                          &rate1, symbol);
                if (symbol->nrow * symbol->ncol != num4)
                    mwerror(FATAL, 2,
                            "Bad number of symbols for codebook 4!\n");
                symbol->firstrow = 0;
                *rate += rate1 * num4 / sizeb;
            }

      /*--- Fourth pass for reconstruction of image ---*/

            x = 0;
            for (rb = 0; rb < nrowb; rb++)
                for (cb = 0; cb < ncolb; cb++, x++)

                    if (indexcb->gray[x] == indexcbval[4])
                        BLOCK_RECONSTRUCT(rb, cb, codebook4, rescodebook4,
                                          NULL, result, symbol, symbres, NULL,
                                          0L);

        }

    /*--- Free buffer for list of symbols and codebook indices ---*/

        mw_delete_fimage(indexcb);
        if (symbresres)
            mw_delete_fimage(symbresres);
        if (symbres)
            mw_delete_fimage(symbres);
        if (symbol)
            mw_delete_fimage(symbol);

    }
}

static void
VECT_RECONSTRUCT(int testmulticb, int *indcb, Fimage codebook1,
                 Fimage codebook2, Fimage codebook3, Fimage codebook4,
                 Fimage rescodebook1, Fimage rescodebook2,
                 Fimage rescodebook3, Fimage rescodebook4,
                 Fimage resrescodebook1, Fimage resrescodebook2,
                 Cimage compress, Fimage result, double *rate)
/* Control for multiple codebooks */
/* Indices of codebook for each class */
/* Buffers of codebooks for reconstruction */
/* Buffers of residu codebooks for reconstruction */
/* Buffers of residu codebooks for reconstruction */
/* Compressed image */
/* Reconstructed image */
/* Bit rate in compress */
{
    Fimage cb1, cb2, cb3, cb4;  /* Codebooks for reconstruction */
    Fimage rescb1, rescb2, rescb3, rescb4;      /* Residu codebooks
                                                 * for reconstruction */
    Fimage resrescb1, resrescb2;        /* Residu codebooks
                                         * for reconstruction */
    bufind numcb;               /* Number of codebooks for each class */
    int n1, n2, n3, n4;         /* Indices of codebooks */
    int nres1, nres2, nres3, nres4;     /* Indices of residu codebooks */
    int nresres1, nresres2;     /* Indices of residu codebooks */

    if (testmulticb >= 1)
    {

        rescb1 = resrescb1 = NULL;
        cb2 = rescb2 = resrescb2 = NULL;
        cb3 = rescb3 = NULL;
        cb4 = rescb4 = NULL;

    /*--- Extract codebooks from CodeBook1, ResCodeBook1 ---*/
                 /*--- and ResResCodeBook1 ---*/

        nres1 = nresres1 = -1;
        numcb[0] = count_cb(codebook1);
        if (indcb[0] >= numcb[0])
        {
            indcb[0] -= numcb[0];
            n1 = numcb[0] - 1;
            if (rescodebook1)
            {
                numcb[0] = count_cb(rescodebook1);
                if (indcb[0] >= numcb[0])
                {
                    indcb[0] -= numcb[0];
                    nres1 = numcb[0] - 1;
                    if (resrescodebook1)
                    {
                        numcb[0] = count_cb(resrescodebook1);
                        if (indcb[0] >= numcb[0])
                        {
                            mwerror(FATAL, 0,
                                    "Index for first class codebook "
                                    "is too large : %d!\n",
                                    indcb[0] + count_cb(codebook1) +
                                    count_cb(rescodebook1));
                            nresres1 = numcb[0] - 1;
                        }
                        else
                            nresres1 = indcb[0];
                    }
                    else
                        mwerror(FATAL, 0, "Missing ResResCodeBook1!\n");

                }
                else
                    nres1 = indcb[0];
            }
            else
                mwerror(FATAL, 0, "Missing ResCodeBook1!\n");
        }
        else
            n1 = indcb[0];

        cb1 = mw_new_fimage();
        extract_cb(codebook1, cb1, n1);
        if (nres1 >= 0)
        {
            rescb1 = mw_new_fimage();
            extract_cb(rescodebook1, rescb1, nres1);
            if (nresres1 >= 0)
            {
                resrescb1 = mw_new_fimage();
                extract_cb(resrescodebook1, resrescb1, nresres1);
            }
        }

        if (!codebook2 && (indcb[1] > 0))
            mwerror(FATAL, 2, "Missing CodeBook2!\n");

        if ((codebook2) && (nadapcb > 1))
        {

      /*--- Extract codebooks from CodeBook2, ResCodeBook2 ---*/
      /*--- and ResResCodeBook2 ---*/

            nres2 = nresres2 = -1;
            numcb[1] = count_cb(codebook2);
            if (indcb[1] >= numcb[1])
            {
                indcb[1] -= numcb[1];
                n2 = numcb[1] - 1;
                if (rescodebook2)
                {
                    numcb[1] = count_cb(rescodebook2);
                    if (indcb[1] >= numcb[1])
                    {
                        indcb[1] -= numcb[1];
                        nres2 = numcb[1] - 1;
                        if (resrescodebook2)
                        {
                            numcb[1] = count_cb(resrescodebook2);
                            if (indcb[1] >= numcb[1])
                            {
                                mwerror(FATAL, 2,
                                        "Index for second class codebook "
                                        "is too large : %d!\n",
                                        indcb[1] + count_cb(codebook2) +
                                        count_cb(rescodebook2));
                                nresres2 = numcb[1] - 1;
                            }
                            else
                                nresres2 = indcb[1];
                        }
                        else
                            mwerror(FATAL, 2, "Missing ResResCodeBook2!\n");

                    }
                    else
                        nres2 = indcb[1];
                }
                else
                    mwerror(FATAL, 2, "Missing ResCodeBook2!\n");
            }
            else
                n2 = indcb[1];

            cb2 = mw_new_fimage();
            extract_cb(codebook2, cb2, n2);
            if (nres2 >= 0)
            {
                rescb2 = mw_new_fimage();
                extract_cb(rescodebook2, rescb2, nres2);
                if (nresres2 >= 0)
                {
                    resrescb2 = mw_new_fimage();
                    extract_cb(resrescodebook2, resrescb2, nresres2);
                }
            }

            if (!codebook3 && (indcb[2] > 0))
                mwerror(FATAL, 2, "Missing CodeBook3!\n");

            if ((codebook3) && (nadapcb > 2))
            {

        /*--- Extract codebooks from CodeBook3 and RecCodeBook3 ---*/

                nres3 = -1;
                numcb[2] = count_cb(codebook3);
                if (indcb[2] >= numcb[2])
                {
                    indcb[2] -= numcb[2];
                    n3 = numcb[2] - 1;
                    if (rescodebook3)
                    {
                        numcb[2] = count_cb(rescodebook3);
                        if (indcb[2] >= numcb[2])
                        {
                            mwerror(FATAL, 2,
                                    "Index for third class codebook "
                                    "is too large : %d!\n",
                                    indcb[2] + count_cb(codebook3));
                            nres3 = numcb[2] - 1;
                        }
                        else
                            nres3 = indcb[2];
                    }
                    else
                        mwerror(FATAL, 2, "Missing ResCodeBook3!\n");
                }
                else
                    n3 = indcb[2];

                cb3 = mw_new_fimage();
                extract_cb(codebook3, cb3, n3);
                if (nres3 >= 0)
                {
                    rescb3 = mw_new_fimage();
                    extract_cb(rescodebook3, rescb3, nres3);
                }

                if (!codebook4 && (indcb[3] > 0))
                    mwerror(FATAL, 2, "Missing CodeBook4!\n");

                if ((codebook4) && (nadapcb > 3))
                {

          /*--- Extract codebooks from CodeBook4 and RecCodeBook4 ---*/

                    nres4 = 0;
                    numcb[3] = count_cb(codebook4);
                    if (indcb[3] >= numcb[3])
                    {
                        indcb[3] -= numcb[3];
                        n4 = numcb[3] - 1;
                        if (rescodebook4)
                        {
                            numcb[3] = count_cb(rescodebook4);
                            if (indcb[3] >= numcb[3])
                            {
                                mwerror(FATAL, 2,
                                        "Index for fourth class codebook "
                                        "is too large : %d!\n",
                                        indcb[3] + count_cb(codebook4));
                                nres4 = numcb[3] - 1;
                            }
                            else
                                nres4 = indcb[3];
                        }
                        else
                            mwerror(FATAL, 2, "Missing ResCodeBook4!\n");
                    }
                    else
                        n4 = indcb[3];

                    cb4 = mw_new_fimage();
                    extract_cb(codebook4, cb4, n4);
                    if (nres4 >= 0)
                    {
                        rescb4 = mw_new_fimage();
                        extract_cb(rescodebook4, rescb4, nres4);
                    }
                }
            }
        }

        if (cb1->gray[(cb1->nrow - 3) * cb1->ncol] == 0.0)
            PLAIN_RECONSTRUCT(cb1, rescb1, resrescb1, compress, result, rate);
        else
            ADAP_RECONSTRUCT(cb1, cb2, cb3, cb4, rescb1, rescb2, rescb3,
                             rescb4, resrescb1, resrescb2, compress, result,
                             rate);

        if (rescb4)
            mw_delete_fimage(rescb4);
        if (cb4)
            mw_delete_fimage(cb4);
        if (rescb3)
            mw_delete_fimage(rescb3);
        if (cb3)
            mw_delete_fimage(cb3);
        if (resrescb2)
            mw_delete_fimage(resrescb2);
        if (rescb2)
            mw_delete_fimage(rescb2);
        if (cb2)
            mw_delete_fimage(cb2);
        if (resrescb1)
            mw_delete_fimage(resrescb1);
        if (rescb1)
            mw_delete_fimage(rescb1);
        if (cb1)
            mw_delete_fimage(cb1);

    }
    else
    {
        if (codebook1->gray[(codebook1->nrow - 3) * codebook1->ncol] == 0.0)
            PLAIN_RECONSTRUCT(codebook1, rescodebook1, resrescodebook1,
                              compress, result, rate);
        else
            ADAP_RECONSTRUCT(codebook1, codebook2, codebook3, codebook4,
                             rescodebook1, rescodebook2, rescodebook3,
                             rescodebook4, resrescodebook1, resrescodebook2,
                             compress, result, rate);
    }

}

void
fivq(int *Print, int *NRow, int *NCol, Fimage CodeBook2, Fimage CodeBook3,
     Fimage CodeBook4, Fimage ResCodeBook1, Fimage ResCodeBook2,
     Fimage ResCodeBook3, Fimage ResCodeBook4, Fimage ResResCodeBook1,
     Fimage ResResCodeBook2, Cimage Compress, Fimage CodeBook1, Fimage Result,
     double *Rate)
                                /* Control of information print */
                                /* Number of rows and columns in
                                 * reconstructed image (if not selected,
                                 * info is read in header of Compress) */
                                             /* Codebooks for classified
                                              * vector quantization */
                                /* Input compressed image */
                                /* First codebook for reconstruction */
                                /* Reconstructed image */
                                /* Bit rate for Compress */
{
    int testmulticb;            /* Control for multiple codebooks */
    bufind indcb;               /* Indices of codebook for each class */
    int nrow1, ncol1;           /* Size of image */
    int r, c;                   /* Indeices for row and column in image */

  /*--- Read information in heaeder of compressed file ---*/

    READ_HEADER_FIMAGE(Print, NRow, NCol, &nrow1, &ncol1, &testmulticb, Rate,
                       indcb, Compress);

  /*--- Memory allocation for quantized image ---*/

    Result = mw_change_fimage(Result, nrow1, ncol1);
    if (Result == NULL)
        mwerror(FATAL, 1, "Not enough memory for quantized image.\n");
    for (r = 0; r < nrow1; r++)
        for (c = 0; c < ncol1; c++)
            Result->gray[r * ncol1 + c] = 0.0;

  /*--- Check input data ---*/

    CHECK_INPUT(testmulticb, CodeBook1, CodeBook2, CodeBook3, CodeBook4,
                ResCodeBook1, ResCodeBook2, ResCodeBook3, ResCodeBook4,
                ResResCodeBook1, ResResCodeBook2, Result);

  /*--- Recoonstruct image ---*/

    VECT_RECONSTRUCT(testmulticb, indcb, CodeBook1, CodeBook2, CodeBook3,
                     CodeBook4, ResCodeBook1, ResCodeBook2, ResCodeBook3,
                     ResCodeBook4, ResResCodeBook1, ResResCodeBook2, Compress,
                     Result, Rate);

}

/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {cfezw};
version = {"1.21"};
author = {"Jean-Pierre D'Ales"};
function = {"Compress a color fimage with EZW algorithm"};
usage = {
 'r':NLevel->NumRec [1,20]         "Number of level for wavelet tranform",
 'e':EdgeIR->Edge_Ri               "Impulse reponses of edge and
                                    preconditionning filters
                                    (orthogonal transform)",
 'b':ImpulseResponse2->Ri2         "Impulse response of filter 2
                                    (biorthogonal transform)",
 'n':FilterNorm->FilterNorm [0,2]  "Normalization mode for filter bank",
 'w':WeightFac->WeightFac          "Scaling factor for wavelet coefficients",
 'd'->DistRate                     "Computes distorsion-rate function",
 'R':TargetRate->Rate              "Target total rate",
 'G':Greenrate->GRate  "Portion of target Rate for green channel (percentile)",
 'B':Bluerate->BRate   "Portion of target Rate for blue channel (percentile)",
 'P':TargetPSNR->PSNR  "Target PSNR",
 'c':Conv->Conv [0,2]  "0 : convert from RGB to YUV",
 's':SelectArea->SelectedArea      "Polygonal regions to be encoded
                                    with a different rate or PSNR",
 'o':Cimage<-Output    "Output compressed Image",
 Image->Image          "Input image",
 ImpulseResponse->Ri   "Impulse response of inner filters",
   {
     Cfimage<-QImage   "Output quantized image"
   }
};
*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"         /* for fezw(), cfchgchannels() */

/*--- Constants ---*/

#define NBIT_GREYVAL 8          /* Number of bit per pixels per channel */
#define MAX_GREYVAL 255.0       /* Maximum of grey levels in input image */
#define RED 0
#define GREEN 1
#define BLUE 2
#define NBIT_SIZEIM 15          /* Number of bits dedicated to encode
                                 * the dimensions of image */
#define MAX_SIZEIM ((long) (1 << NBIT_SIZEIM) - 1)
/* Maximum number of line and column for images */
#define mw_drcurvesize 256

/*--- Special structures ---*/

typedef struct drcurve {
    int nrow;                   /* Number of rows (dy) */
    int ncol;                   /* Number of columns (dx) */
    int nbitplanes;             /* Number of bit per pixel */
    int npoints;                /* Number of points in the curve */
    float rate[mw_drcurvesize]; /* Bit rates */
    float cr[mw_drcurvesize];   /* Compression ratios */
    float mse[mw_drcurvesize];  /* Mean square error for correponding bit rate
                                 * and compression ratio */

} DRCurve;                      /* Distortion rate curves */

/*--- Global variables ---*/

static long effnbit;            /* Total number of bits stored in compress */
static int ncodewords;          /* Number of codewords stored in compress */
static int bits_to_go;          /* Number of free bits in the currently
                                 * encoded codeword */
static int buffer;              /* Bits to encode in the currently
                                 * encoded codeword */
static unsigned char *ptrc;     /* Pointer to compress->gray for next
                                 * codewords to encode */
/* DRCurve         drc; */
/* Distorsion rate curve */

static void RESIZE_COMPRESS_FIMAGE(Cimage compress)
{
    int i;
    int ncolo, nrowo;
    long size;
    int mindif;

    if (bits_to_go < 8)
    {
        *ptrc = buffer >> bits_to_go;
        ncodewords += 1;
    }
    else
        bits_to_go = 0;

    ncolo = 1;
    nrowo = ncodewords;
    if (nrowo > MAX_SIZEIM)
    {
        if (sqrt((double) nrowo) + 1 > MAX_SIZEIM)
            mwerror(FATAL, 2, "Number of codewords is too large!\n");
        i = 2;
        while (nrowo / i > MAX_SIZEIM)
            i++;
        while ((nrowo % i != 0) && (i <= nrowo / i))
            i++;
        if (i <= nrowo / i)
        {
            nrowo /= i;
            ncolo = i;
        }
        else
        {
            i = 2;
            mindif = ncodewords;
            while (i <= nrowo / i)
            {
                if (nrowo / i <= MAX_SIZEIM)
                    if ((nrowo / i + 1) * i - ncodewords < mindif)
                    {
                        ncolo = i;
                        mindif = (nrowo / i + 1) * i - ncodewords;
                    }
                i++;
            }
            nrowo = ncodewords / ncolo + 1;
            size = nrowo * ncolo;
            if (ncodewords >= size)
                mwerror(WARNING, 0,
                        "Something is wrong with output dimensions!\n");
        }
    }

    if (nrowo > ncolo)
    {
        mindif = ncolo;
        ncolo = nrowo;
        nrowo = mindif;
    }

    if (compress->lastrow == 0)
        compress = mw_change_cimage(compress, nrowo, ncolo);
    compress->cmt[0] = '\0';

    compress->firstrow = bits_to_go;
    compress->firstcol = ncodewords;
}

static void REALLOCATE_COMPRESS_FIMAGE(Cimage compress)
{
    int i;
    Cimage bufcomp;
    long size;

    size = compress->ncol * compress->nrow;
    printf("Reallocation of Compress for fimage vector quantization.\n");

    bufcomp = mw_new_cimage();
    bufcomp = mw_change_cimage(bufcomp, compress->nrow, compress->ncol);
    if (bufcomp == NULL)
        mwerror(FATAL, 1,
                "Memory allocation refused for reallocation buffer!\n");
    for (i = 0; i < size; i++)
        bufcomp->gray[i] = compress->gray[i];

    compress = mw_change_cimage(compress, compress->nrow * 2, compress->ncol);
    if (compress == NULL)
        mwerror(FATAL, 1,
                "Memory allocation refused for reallocated compress!\n");

    for (i = 0; i < size; i++)
    {
        compress->gray[i] = bufcomp->gray[i];
        compress->gray[i + size] = 0;
    }

    ptrc = compress->gray;
    for (i = 1; i < size; i++)
        ptrc++;

    mw_delete_cimage(bufcomp);
}

static void ADD_BIT_TO_COMPRESS_FIMAGE(int bit, Cimage compress)
{
    buffer >>= 1;
    if (bit)
        buffer += 128;
    bits_to_go -= 1;
    if (bits_to_go == 0)
    {
        *ptrc = buffer;
        ncodewords += 1;
        if (ncodewords == compress->ncol * compress->nrow)
            REALLOCATE_COMPRESS_FIMAGE(compress);
        ptrc++;
        bits_to_go = 8;
        buffer = 0;
    }
}

static void ADD_BUF_TO_COMPRESS_FIMAGE(Cimage compress, Cimage bufcomp)
                                /* Compressed image */
                                /* Buffer for compressed image */
{
    int bit;
    int b;
    int buf;
    int c, size;

    size = bufcomp->nrow * bufcomp->ncol;

    for (c = 0; c < size - 1; c++)
    {
        buf = bufcomp->gray[c];
        for (b = 0; b <= 7; b++)
        {
            bit = buf & 1;
            buf >>= 1;
            ADD_BIT_TO_COMPRESS_FIMAGE(bit, compress);
            effnbit++;
        }
    }

    buf = bufcomp->gray[size - 1];
    for (b = 0; b < bufcomp->firstrow; b++)
    {
        bit = buf & 1;
        buf >>= 1;
        ADD_BIT_TO_COMPRESS_FIMAGE(bit, compress);
        effnbit++;
    }
}

static void
COMPUTE_CHANNEL_PSNR(int *conv, float *psnr, float *gpsnr, float *bpsnr,
                     float *redpsnr, float *greenpsnr, float *bluepsnr)
                                /* Target Psnr */
                                /* Portion of target psnr dedicated to green
                                 * channel */
                                /* Portion of target psnr dedicated to blue
                                 * channel */
                                            /* Target rates for each channel */
{
    if (bpsnr && gpsnr)
    {
        if (*bpsnr + *gpsnr > 100.0)
            mwerror(FATAL, 2, "BPsnr and GPsnr are too large!\n");
        *redpsnr = *psnr * (100.0 - (*bpsnr + *gpsnr)) * 0.03;
        *greenpsnr = *psnr * *gpsnr * 0.03;
        *bluepsnr = *psnr * *bpsnr * 0.03;
    }
    else
    {
        if (conv)
        {
            if (*conv == 0)
            {
                *redpsnr = *psnr - 5.0;
                *bluepsnr = *psnr - 5.0;
                *greenpsnr = *psnr;
            }
        }
        else
        {
            *redpsnr = *psnr;
            *bluepsnr = *psnr;
            *greenpsnr = *psnr;
        }
    }

}

static void
COMPUTE_CHANNEL_RATES(int *conv, float *rate, float *grate, float *brate,
                      float *redrate, float *greenrate, float *bluerate)
                                /* Target Rate */
                                /* Portion of target rate dedicated to green
                                 * channel */
                                /* Portion of target rate dedicated to blue
                                 * channel */
                                            /* Target rates for each channel */
{

    if (brate && grate)
    {
        if (*brate + *grate > 100.0)
            mwerror(FATAL, 2, "BRate and GRate are too large!\n");
        *redrate = *rate * (100.0 - (*brate + *grate)) * 0.03;
        *greenrate = *rate * *grate * 0.03;
        *bluerate = *rate * *brate * 0.03;
    }
    else
    {
        if (conv)
        {
            if (*conv == 0)
            {
                *redrate = 0.15 * *rate;
                *bluerate = 0.15 * *rate;
                *greenrate = 2.7 * *rate;
            }
        }
        else
        {
            *redrate = *rate;
            *bluerate = *rate;
            *greenrate = *rate;
        }
    }

}

static void COPY_CHANNEL2FIMAGE(Cfimage image, Fimage chimage, int color)
                                /* Input color image */
                                /* Original channel image */
                                /* Index of channel */
{
    int x, size;
    register float *ptr1, *ptr2;

    size = image->nrow * image->ncol;
    ptr2 = chimage->gray;
    if (color == RED)
        ptr1 = image->red;
    else if (color == GREEN)
        ptr1 = image->green;
    else
        ptr1 = image->blue;
    for (x = 0; x < size; x++, ptr1++, ptr2++)
        *ptr2 = *ptr1;

}

static void COPY_FIMAGE2CHANNEL(Cfimage image, Fimage chimage, int color)
                                /* Output color image */
                                /* Original channel image */
                                /* Index of channel */
{
    int x, size;
    register float *ptr1, *ptr2;

    size = image->nrow * image->ncol;
    ptr2 = chimage->gray;
    if (color == RED)
        ptr1 = image->red;
    else if (color == GREEN)
        ptr1 = image->green;
    else
        ptr1 = image->blue;
    for (x = 0; x < size; x++, ptr1++, ptr2++)
        *ptr1 = *ptr2;

}

void
cfezw(int *NumRec, Fimage Edge_Ri, Fsignal Ri2, int *FilterNorm,
      float *WeightFac, int *DistRate, float *Rate, float *GRate,
      float *BRate, float *PSNR, int *Conv, Polygons SelectedArea,
      Cimage Output, Cfimage Image, Fsignal Ri, Cfimage QImage)
        /*--- Computes the orthogonal wavelet transform of image `Image` ---*/
                                /* Number of recursion (-j) */
                                /* Impulse responses of filters for special
                                 * edge processing (including preconditionning
                                 * matrices */
                                /* Impulse response of the low pass filter */
                                /* for synthesis */
                                /* Equal 0 if no normalisation of filter's tap
                                 *       1 if normalisation of the sum
                                 *       2 if normalistion of the square sum */
                                /* Weighting factor for wavelet coeff. */
                                /* Compute distortion-rate function */
                                /* Target Rate */
                                /* Portion of target rate dedicated to green
                                 * channel */
                                /* Portion of target rate dedicated to blue
                                 * channel */
                                /* Target PSNR */
                                /* Conversion type */
                                /* Polygnal regions to be encoded with
                                 * a special rate or PSNR */
                                /* Compressed `Image` */
                                /* Input color image */
                                /* Impulse response of the low pass filter */
                                /* Output quantized image */
{
    float redrate, greenrate, bluerate; /* Target rates for each channel */
    float *RedR, *GreenR, *BlueR;       /* Target rates for each channel */
    float redpsnr, greenpsnr, bluepsnr; /* Target psnrs for each channel */
    float *RedP, *GreenP, *BlueP;       /* Target psnrs for each channel */
    Fimage ChImage;             /* Original channel image */
    Fimage QChImage;            /* Reconstructed channel image */
    Cimage bufcomp;             /* Buffer for compressed image */
    int Norm;                   /* Flag for normalisation
                                 * in color conversion */
    int Inverse;                /* Flag for inversion in color conversion */
    DRCurve drcchannel;         /* Distorsion rate curve for one channel */
    DRCurve drctotal;           /* Distorsion rate curve for total image */
    int count_dr;               /* Index of point in rate distortion curve */

  /*--- Change coordinate system for colors ---*/

    if (Conv)
        cfchgchannels(Conv, NULL, &Norm, Image, Image);

  /*--- Memory allocation for quantized color image ---*/

    if (QImage)
    {
        QImage = mw_change_cfimage(QImage, Image->nrow, Image->ncol);
        if (QImage == NULL)
            mwerror(FATAL, 1,
                    "Not enough memory for quantized color image!\n");
        QChImage = mw_new_fimage();
    }
    else
        QChImage = NULL;

  /*--- Memory allocation for original channel image ---*/

    ChImage = mw_new_fimage();
    ChImage = mw_change_fimage(ChImage, Image->nrow, Image->ncol);
    if (ChImage == NULL)
        mwerror(FATAL, 1, "Not enough memory for original channel image!\n");

  /*--- Initialisation of buffer for compressed file ---*/

    if (Output)
    {
        bufcomp = mw_new_cimage();
        Output = mw_change_cimage(Output, Image->nrow, Image->ncol);
        if (Output == NULL)
            mwerror(FATAL, 1, "Memory allocation refused for `Output`!\n");
        ptrc = Output->gray;
        bits_to_go = 8;
        buffer = 0;
        ncodewords = 0;
    }
    else
        bufcomp = NULL;

  /*--- Compute target rates for each channel ---*/

    RedR = GreenR = BlueR = RedP = GreenP = BlueP = NULL;
    if (Rate)
    {
        COMPUTE_CHANNEL_RATES(Conv, Rate, GRate, BRate, &redrate, &greenrate,
                              &bluerate);
        RedR = &redrate;
        GreenR = &greenrate;
        BlueR = &bluerate;
    }
    else if (PSNR)
    {
        COMPUTE_CHANNEL_PSNR(Conv, PSNR, GRate, BRate, &redpsnr, &greenpsnr,
                             &bluepsnr);
        RedP = &redpsnr;
        GreenP = &greenpsnr;
        BlueP = &bluepsnr;
    }

  /*--- Compress red channel ---*/

    COPY_CHANNEL2FIMAGE(Image, ChImage, RED);
    fezw(NumRec, Edge_Ri, Ri2, FilterNorm, WeightFac, DistRate, RedR, RedP,
         SelectedArea, bufcomp, ChImage, Ri, QChImage, (char *) &drcchannel);
    if (QImage)
        COPY_FIMAGE2CHANNEL(QImage, QChImage, RED);
    if (Output)
        ADD_BUF_TO_COMPRESS_FIMAGE(Output, bufcomp);
    if (DistRate)
    {
        drctotal.ncol = Image->ncol;
        drctotal.nrow = Image->nrow;
        drctotal.npoints = drcchannel.npoints;
        for (count_dr = 0; count_dr < drcchannel.npoints; count_dr++)
        {
            drctotal.cr[count_dr] = drcchannel.cr[count_dr];
            drctotal.rate[count_dr] = drcchannel.rate[count_dr];
            drctotal.mse[count_dr] = drcchannel.mse[count_dr];
        }
    }

  /*--- Compress green channel ---*/

    COPY_CHANNEL2FIMAGE(Image, ChImage, GREEN);
    fezw(NumRec, Edge_Ri, Ri2, FilterNorm, WeightFac, DistRate, GreenR,
         GreenP, SelectedArea, bufcomp, ChImage, Ri, QChImage,
         (char *) &drcchannel);
    if (QImage)
        COPY_FIMAGE2CHANNEL(QImage, QChImage, GREEN);
    if (Output)
        ADD_BUF_TO_COMPRESS_FIMAGE(Output, bufcomp);
    if (DistRate)
        for (count_dr = 0; count_dr < drcchannel.npoints; count_dr++)
            drctotal.mse[count_dr] += drcchannel.mse[count_dr];

  /*--- Compress blue channel ---*/

    COPY_CHANNEL2FIMAGE(Image, ChImage, BLUE);
    fezw(NumRec, Edge_Ri, Ri2, FilterNorm, WeightFac, DistRate, BlueR, BlueP,
         SelectedArea, bufcomp, ChImage, Ri, QChImage, (char *) &drcchannel);
    if (QImage)
        COPY_FIMAGE2CHANNEL(QImage, QChImage, BLUE);
    if (Output)
        ADD_BUF_TO_COMPRESS_FIMAGE(Output, bufcomp);
    if (DistRate)
        for (count_dr = 0; count_dr < drcchannel.npoints; count_dr++)
        {
            drctotal.mse[count_dr] += drcchannel.mse[count_dr];
            drctotal.mse[count_dr] /= 3.0;
            printf("%d\t%.2f\n", (int) drctotal.cr[count_dr],
                   10.0 * log10((double) MAX_GREYVAL * MAX_GREYVAL /
                                drctotal.mse[count_dr]));
        }
    if (Output)
        RESIZE_COMPRESS_FIMAGE(Output);

    if (Conv && QImage)
        cfchgchannels(Conv, &Inverse, &Norm, QImage, QImage);
}

/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {flbg};
author = {"Jean-Pierre D'Ales"};
function = {"Generates a (sequence of) codebook(s) from a training set of images using LBG (generalized Lloyd) algorithm"};
version = {"2.3"};
usage = {
 's':[CodeBookSize=1]->Size    "Size of output codebook", 
 'w':[VectorWidth=2]->Width    "Width of vectors", 
 'h':[VectorHeight=2]->Height  "Height of vectors", 
 'l'->Lap                      "Take overlapping vectors in training images", 
 'd':[Decim=1]->Decim          "Decimation factor in training images (for wavelet transform)",
 'e':[Edge=0]->Edge            "Do not take overlapping vectors if the distance to an edge is smaller than Edge",
 'W':Weight->Weight            "Weighting factors for the components of vector (fsignal)",
 'M'->MultiCB                  "Generate codebooks of size equal to a power of two and smaller than Size",
 'p'->PrintSNR                 "Do not print information",
 'A':TrainImage2->Image2       "Training image (fimage)",
 'B':TrainImage3->Image3       "Training image (fimage)",
 'C':TrainImage4->Image4       "Training image (fimage)",
 'D':TrainImage5->Image5       "Training image (fimage)",
 'E':TrainImage6->Image6       "Training image (fimage)",
 'F':TrainImage7->Image7       "Training image (fimage)",
 'G':TrainImage8->Image8       "Training image (fimage)",
 'i':InitRandCB->InitRandCB    "Initiate algorithm with a randomly drawn codebook of size InitRandCB",
 'r':[RepeatRand=1]->RepeatRand  "Run the algorithm RepeatRand times (irrelevant if InitRandCB is smaller than 1)",
 'f':NResCB->NResCB            "Index of first residual codebook (ResCodeBook)",
 'g':NResResCB->NResResCB      "Index of second residual codebook (ResResCodeBook)",
 'a':ResCodeBook->ResCodeBook  "First residual codebook (fimage)",
 'b':ResResCodeBook->ResResCodeBook  "Second residual codebook (fimage)",
 TrainImage1->Image1           "Training image (fimage)",
 CodeBook<-CodeBook            "Output codebook (fimage)",
   { 
     MSE<-MSE                  "Quantization M.S.E. for training set"
   }
};
*/
/*----------------------------------------------------------------------
 v2.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for mk_trainset(), mk_codebook(), flbg_train() */

/*--- Constants ---*/

static void
VARMEAN_TS(Fimage trainset, double *mean, double *var)

  /*--- Compute mean and variance of vectors in training set ---*/

                         /* Training set of vectors for LBG algorithm */
                         /* Mean and variance of vectors in training set */

{
  register float  *ptrts;
  long             i;
  long             size;

  *var = *mean = 0.0;
  size = trainset->nrow * trainset->ncol;

  ptrts = trainset->gray;
  for (i=0; i<size; i++, ptrts++)
    *mean += *ptrts;
  *mean /= (double) size;

  ptrts = trainset->gray;
  for (i=0; i<size; i++, ptrts++)
    *var += (*ptrts - *mean) * (*ptrts - *mean);
  *var /= (double) size;

}




void
flbg(int *Size, int *Width, int *Height, int *Lap, int *Decim, int *Edge, Fsignal Weight, int *MultiCB, int *PrintSNR, Fimage Image2, Fimage Image3, Fimage Image4, Fimage Image5, Fimage Image6, Fimage Image7, Fimage Image8, int *InitRandCB, int *RepeatRand, int *NResCB, int *NResResCB, Fimage ResCodeBook, Fimage ResResCodeBook, Fimage Image1, Fimage CodeBook, float *MSE)

                            /* Size of codebook */
                            /* Width  and height of block */
                            /* Take overlapping vectors in training images */
                            /* Space between pixels in a vector */
                            /* No overlapping on edges */
                            /* Coordinates weights in block */
                            /* Generates all code books of size equal 
			     * to a power of 2 */
                            /* Print SNR and rate */
                                                                   
                            /* Image for training set */
                            /* Size of initial codebook (randomly drawn) */
                            /* Repeat process with random initialization */
                                /* Size for residual codebooks */
                                              /* Residual codebooks */
                            /* First image for training set */
                            /* Generated codebook */
     	                    /* Quantization m.s.e. for training set */


{
  Fimage          TrainSet;     /* Training set of vectors for LBG algorithm */
  float           msetr;        /* Quantization m.s.e. of trainin set with 
				 * generated codebook */
  Fimage          InitCodeBook; /* Initial codebook */
  int             sizev;        /* Size of vectors */
  int             r;          
  double          Mean, Variance;   /* Mean and variance of vectors 
				 * in training set */

  TrainSet = mw_new_fimage();

  mk_trainset(Width, Height, Lap, Decim, Edge, NULL, NULL, NULL, NULL, Image2, Image3, Image4, Image5, Image6, Image7, Image8, NULL, NULL, NULL, Image1, TrainSet);

  if (InitRandCB) {
    VARMEAN_TS(TrainSet, &Mean, &Variance);
    sizev = *Width * *Height;
    InitCodeBook = mw_new_fimage();

    for (r=0; r<*RepeatRand; r++) {
      mk_codebook(&sizev, &Mean, &Variance, InitRandCB, &sizev, InitCodeBook);
      flbg_train(Size, Weight, MultiCB, InitCodeBook, NResCB, NResResCB, ResCodeBook, ResResCodeBook, PrintSNR, TrainSet, &msetr, CodeBook);
    }
  } else
    {
      InitCodeBook = NULL;

      flbg_train(Size, Weight, MultiCB, InitCodeBook, NResCB, NResResCB, ResCodeBook, ResResCodeBook, PrintSNR, TrainSet, &msetr, CodeBook);
    }

  CodeBook->firstcol = *Width;
  CodeBook->firstrow = *Height;
  CodeBook->gray[(CodeBook->nrow - 2) * CodeBook->ncol] = *Height;

  mw_delete_fimage(TrainSet);
  
  if (MSE)
    *MSE = msetr;
}

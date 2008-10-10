/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {flbg_adap};
 version = {"2.1"};
 author = {"Jean-Pierre D'Ales"};
 function = {"Generates a (sequence of) codebook(s) adapted for classified V.Q. from a training set of images using LBG algorithm"};
 usage = {
   's':[CBSize1=1]->Size         "Size of output codebook for first class", 
   'w':[VectorWidth=2]->Width    "Width of vectors", 
   'h':[VectorHeight=2]->Height  "Height of vectors", 
   'l'->Lap                      "Take overlapping vectors in training images",
   'd':[Decim=1]->Decim          "Decimation factor in training images (for wavelet transform)",
   'e':[Edge=0]->Edge            "Do not take overlapping vectors if the distance to an edge is smaller than Edge",
   'S':ThresVal1->ThresVal1      "First threshold value for classified VQ",
   'T':ThresVal2->ThresVal2      "Second threshold value for classified VQ",
   'U':ThresVal3->ThresVal3      "Third threshold value for classified VQ",
   'W':Weight->Weight       "Weighting factors for the components of vector (fsignal)",
   'M'->MultiCB             "Generate codebooks of size equal to a power of two and smaller than Size",
   'p'->PrintSNR            "Do not print information",
   't':CBSize2->Size2       "Size of output codebook for second class", 
   'u':CBSize3->Size3       "Size of output codebook for third class", 
   'v':CBSize4->Size4       "Size of output codebook for fourth class", 
   'A':TrainImage2->Image2  "Training image (fimage)",
   'B':TrainImage3->Image3  "Training image (fimage)",
   'C':TrainImage4->Image4  "Training image (fimage)",
   'D':TrainImage5->Image5  "Training image (fimage)",
   'E':TrainImage6->Image6  "Training image (fimage)",
   'F':TrainImage7->Image7  "Training image (fimage)",
   'G':TrainImage8->Image8  "Training image (fimage)",
   'x':Output2<-Output2     "Resulting codebook set for second class (fimage)",
   'y':Output3<-Output3     "Resulting codebook set for third class (fimage)",
   'z':Output4<-Output4     "Resulting codebook set for fourth class (fimage)",
   TrainImage1->Image1      "Training image (fimage)", 
   Output1<-Output          "Resulting codebook set for first class (fimage)"
	};
*/
/*----------------------------------------------------------------------
 v2.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* mk_trainset(), flbg_train() */

/*--- Constants ---*/

static void
orderf(a,b)

  /*--- Put *a and *b in a decreasing order ---*/

 float	*a, *b;
 
{
  float c;
  
  if (*a < *b) {
    c = *a;
    *a = *b;
    *b = c;
  }
}


static void
ORDER_THRES(thres1, thres2, thres3)

  /*--- Put *thres1, *thres2 and *thres3 (when they are not NULL) ---*/
                     /*--- in a decreasing order ---*/

float	**thres1, **thres2, **thres3;  /* threshold values for block energy */

{
  float    thres;
  float   *ptrthres;
    
  ptrthres = &thres;
    
  if (*thres1) {
    if (*thres2) {
      orderf(*thres1, *thres2);
      if (*thres3) {
	orderf(*thres1, *thres3);
	orderf(*thres2, *thres3);
      }
    } else 
      if (*thres3) {
	*thres2 = *thres3;
	*thres3 = NULL;
	orderf(*thres1, *thres2);
      }
	
  } else 
    if (*thres2) {
      *thres1 = *thres2;
      if (*thres3) {
	*thres2 = *thres3;
	orderf(*thres1, *thres2);
	*thres3 = NULL;
      } else
	*thres2 = NULL;
    } else 
      if (*thres3) {
	*thres1 = *thres3;
    	*thres3 = NULL;
      }

}




void
flbg_adap(Size, Width, Height, Lap, Decim, Edge, ThresVal1, ThresVal2, ThresVal3, Weight, MultiCB, PrintSNR, Size2, Size3, Size4, Image2, Image3, Image4, Image5, Image6, Image7, Image8, Output2, Output3, Output4, Image1, Output)

int        *Size;           /* Size of codebook */
int        *Width, *Height; /* Width  and height of block */
int        *Lap;            /* Take overlapping vectors in training images */
int        *Decim;          /* Space between pixels in a vector */
int        *Edge;           /* No overlapping on edges */
float      *ThresVal1, *ThresVal2, *ThresVal3;	/* threshold values  
				                 * for block energy */
Fsignal     Weight;         /* Coordinates weights in block */
int        *MultiCB;        /* Generates all code books of size equal 
			     * to a power of 2 */
int        *PrintSNR;       /* Print SNR and rate */
int	   *Size2, *Size3, *Size4;     /* Size of adapted codebook */
Fimage      Image2, Image3, Image4, Image5, Image6, Image7, Image8;    
                            /* Images for training set */
Fimage      Output2, Output3, Output4; /* Adapted codebooks */
Fimage      Image1;         /* First image for training set */
Fimage      Output;         /* Generated codebook */


{
  Fimage    TrainSet1, TrainSet2, TrainSet3, TrainSet4; /* Training sets 
			     * of blocks extracted form training images 
			     * for the different adaptive classes of vectors */
  float	    MSE;

  if (Weight) 
    if (Weight->size != *Width * *Height)
      mwerror(FATAL, 1, "Number of weights is different from size of block!\n");

  /*--- Initilize training sets ---*/

  TrainSet1 = mw_new_fimage();
  if (Size2)
    TrainSet2 = mw_new_fimage();
  else
    TrainSet2 = NULL;
  if (Size3)
    TrainSet3 = mw_new_fimage();
  else
    TrainSet3 = NULL;
  if (Size4)
    TrainSet4 = mw_new_fimage();
  else
    TrainSet4 = NULL;
    
  ORDER_THRES(&ThresVal1, &ThresVal2, &ThresVal3);
    
  /*--- Compute training sets ---*/ 

  mk_trainset(Width, Height, Lap, Decim, Edge, ThresVal1, ThresVal2, ThresVal3, Size, Image2, Image3, Image4, Image5, Image6, Image7, Image8, TrainSet2, TrainSet3, TrainSet4, Image1, TrainSet1);

  if (TrainSet4) {

    /*--- Compute codebook for fourth adaptive class ---*/
    /*--- (blocks whose energy is less than *ThresVal3) ---*/

    flbg_train(Size4, Weight, MultiCB, NULL, NULL, NULL, NULL, NULL, PrintSNR, TrainSet4, &MSE, Output4);
    
    mw_delete_fimage(TrainSet4);
    Output4->gray[(Output4->nrow - 4) * Output4->ncol] = *ThresVal3;
    Output4->gray[(Output4->nrow - 3) * Output4->ncol] = 0.0;
    Output4->gray[(Output4->nrow - 2) * Output4->ncol] = *Height;
    Output4->firstcol = *Width;
    Output4->firstrow = *Height;

    printf("m.s.e. 4 = %f\n", MSE);
  }

  if (TrainSet3) {

  /*--- Compute codebook for third adaptive class (blocks whose energy ---*/
       /*--- is less than *ThresVal2 and greater than *ThresVal3) ---*/

    flbg_train(Size3, Weight, MultiCB, NULL, NULL, NULL, NULL, NULL, PrintSNR, TrainSet3, &MSE, Output3);

    mw_delete_fimage(TrainSet3);
    Output3->gray[(Output3->nrow - 4) * Output3->ncol] = *ThresVal2;
    if (ThresVal3)
      Output3->gray[(Output3->nrow - 3) * Output3->ncol] = *ThresVal3;
    Output3->gray[(Output3->nrow - 2) * Output3->ncol] = *Height;
    Output3->firstcol = *Width;
    Output3->firstrow = *Height;

    printf("m.s.e. 3 = %f\n", MSE);
  }
    
  if (TrainSet2) {

  /*--- Compute codebook for second adaptive class (blocks whose energy ---*/
       /*--- is less than *ThresVal1 and greater than *ThresVal2) ---*/

    flbg_train(Size2, Weight, MultiCB, NULL, NULL, NULL, NULL, NULL, PrintSNR, TrainSet2, &MSE, Output2);

    mw_delete_fimage(TrainSet2);
    Output2->gray[(Output2->nrow - 4) * Output2->ncol] = *ThresVal1;
    if (ThresVal2)
      Output2->gray[(Output2->nrow - 3) * Output2->ncol] = *ThresVal2;
    Output2->gray[(Output2->nrow - 2) * Output2->ncol] = *Height;
    Output2->firstcol = *Width;
    Output2->firstrow = *Height;

    printf("m.s.e. 2 = %f\n", MSE);
  }
    

  /*--- Compute codebook for first adaptive class (blocks whose energy ---*/
                  /*--- is greater than *ThresVal1) ---*/

  flbg_train(Size, Weight, MultiCB, NULL, NULL, NULL, NULL, NULL, PrintSNR, TrainSet1, &MSE, Output);
   
  mw_delete_fimage(TrainSet1);

  Output->gray[(Output->nrow - 4) * Output->ncol] = Output->gray[(Output->nrow - 2) * Output->ncol + 1] = 0.0;
  if (ThresVal1)
    Output->gray[(Output->nrow - 3) * Output->ncol] = *ThresVal1;
  Output->gray[(Output->nrow - 2) * Output->ncol] = *Height;
  Output->firstcol = *Width;
  Output->firstrow = *Height;

  printf("m.s.e. 1 = %f\n", MSE);

    
}

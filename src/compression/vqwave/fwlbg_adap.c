/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {fwlbg_adap};
version = {"2.00"};
author = {"Jean-Pierre D'Ales"};
function = {"Generates training set(s) (starting from image(s)) and constructs a codebook sequence for wavelet transform vector quantization using LBG algorithm"};
usage = {
'r':[NLevel=1]->NumRecMax
	"Quantization is performed on MaxLevel scales", 
'q':Level->Level [1,16]
	"Create codebook(s) at scale Level", 
'o':Orient->Orient [0,3]
	"Create codebook(s) for orientation Orient",
'e':EdgeIR->Edge_Ri
      	"Impulse reponses of edge and preconditionning filters for orthogonal transform (fimage)",
'b':ImpulseResponse2->Ri2
	"Impulse response of filter 2 for biorthogonal transform (fsignal)",
'n':FilterNorm->FilterNorm [0,2]
	"Normalization mode for filter bank", 
'd':[StopDecimLevel=2]->StopDecim [1,20]
	"Level for decimation stop (default : 2)",
'w':[VectorWidth=2]->Width
	"Width of vectors (default : 2)", 
'h':[VectorHeight=2]->Height
	"Height of vectors (default : 2)", 
'M'->MultiCB
        "Generate codebooks of size equal to a power of two",
'l'->Lap
	"Take overlapping vectors in training images", 
's':Sizec1->Sizec1
	"Size of output codebook for first class", 
't':Size2->Sizec2
	"Size of output codebook for second class", 
'u':Size3->Sizec3
	"Size of output codebook for third class", 
'S':ThresVal1->ThresVal1
	"First threshold value for classified VQ",
'T':ThresVal2->ThresVal2
	"Second threshold value for classified VQ",
'U':ThresVal3->ThresVal3
	"Third threshold value for classified VQ",
'O':OldCodeBook->OldCodeBook
	"Modify the first class codebook sequence (fimage)",
'X':OldAdapCodeBook1->OldAdapCodeBook2
	"Modify the second class codebook sequence (fimage)",
'Y':OldAdapCodeBook2->OldAdapCodeBook3
	"Modify the third class codebook sequence (fimage)",
'x':AdapCodeBook2<-Output2
	"Sequence of second class codebooks (fimage)",
'y':AdapCodeBook3<-Output3
	"Sequence of third class codebooks (fimage)",
'A':TrainImage2->Image2
	"Training image (fimage)",
'B':TrainImage3->Image3
	"Training image (fimage)",
'C':TrainImage4->Image4
	"Training image (fimage)",
'Q':ResCodeBook->ResCodeBook
	"Generate codebook(s) for residu of quantization with ResCodeBook (fimage)",
'R':ResResCodeBook->ResResCodeBook
	"Generate codebook(s) for residu of quantization with ResCodeBook and ResResCodeBook (fimage)",
TrainImage1->Image1
	"Training image (fimage)", 
ImpulseResponse->Ri
	"Impulse response of inner filters (fsignal)", 
CodeBook1<-Output1
	"Sequence of first class codebooks (fimage)"
	};
*/


/*--- Include files UNIX C ---*/
#include <math.h>

/*--- Megawave2 library ---*/
#include  "mw.h"

/*--- Megawave2 modules definition ---*/

extern void wlbg_adap();
extern void dybiowave2();
extern void dyowave2();

/*--- Constants ---*/

/*--- Global variables ---*/ 

static Fsignal      ORI1, ORI2;          /* Non normalized filters */


static void
INIT_RI(ri1, ri2)

Fsignal ri1, ri2;

{
  int i;

  ORI1 = mw_new_fsignal();
  if (mw_alloc_fsignal(ORI1, ri1->size) == NULL)
    mwerror(FATAL, 1, "Not enough memory for ri1!\n");

  ORI2 = mw_new_fsignal();
  if (mw_alloc_fsignal(ORI2, ri2->size) == NULL)
    mwerror(FATAL, 1, "Not enough memory for ri2!\n");

  for (i=0; i<ri1->size; i++)
    ORI1->values[i] = ri1->values[i];

  for (i=0; i<ri2->size; i++)
    ORI2->values[i] = ri2->values[i];

}



static void
REFRESH_FILTERS(ri1, ri2)

Fsignal ri1, ri2;

{
  int i;


  for (i=0; i<ri1->size; i++)
    ri1->values[i] = ORI1->values[i];

  for (i=0; i<ri2->size; i++)
    ri2->values[i] = ORI2->values[i];

}



static void
WCB2FCB(wcb, fcb)

  /*--- Translate a codebook sequence from the wtrans2d ---*/
                 /*--- to the fimage format ---*/

Wtrans2d      wcb;         /* Codebook in the wtrans2d format */
Fimage        fcb;         /* Codebook in the fimage format */

{
  int           j;	        /* Scale index in wav. transf. */
  int           i;	      	/* Orientation index */
  long          r, c;           /* Indices for row and columns in codebooks */
  long          nrow, ncol;     /* Number of rows and columns in fcb */
  long          nrowcb, ncolcb; /* Number of rows and columns in sub-image 
				 * codebook in wcb */
  long          incrc;

  /*--- Memory allocation and initialisation of fcb ---*/

  nrow = ncol = 0;
  for (j = 1; j <= wcb->nlevel; j++)
    for (i = 0; i <= 3; i++) 
      if (wcb->images[j][i]) {
	if (wcb->images[j][i]->nrow > nrow)
	  nrow = wcb->images[j][i]->nrow;
	if (wcb->images[j][i]->ncol > 1)
	  ncol += wcb->images[j][i]->ncol;
      }
  nrow += 3;
  if (ncol < 4 * wcb->nlevel)
    ncol = 4 * wcb->nlevel;

  if (mw_change_fimage(fcb, nrow, ncol) == NULL)
    mwerror(FATAL, 1, "Not enough memory for fimage buffer for codebook.\n");

  /*--- Copy wcb to fcb ---*/

  incrc = 0;
  fcb->gray[0] = wcb->nlevel;
  for (j = 1; j <= wcb->nlevel; j++)
    for (i = 0; i <= 3; i++) {
      fcb->gray[ncol + (j - 1) * 4 + i] = 0;
      fcb->gray[2 * ncol + (j - 1) * 4 + i] = 0;

      if (wcb->images[j][i]) {
	nrowcb = wcb->images[j][i]->nrow;
	ncolcb = wcb->images[j][i]->ncol;
	if (nrowcb > 1) {
	  fcb->gray[ncol + (j - 1) * 4 + i] = nrowcb;
	  fcb->gray[2 * ncol + (j - 1) * 4 + i] = ncolcb;

	  for (r = 0; r < nrowcb; r++)
	    for (c = 0; c < ncolcb; c++) 
	      fcb->gray[(r + 3) * ncol + c + incrc] = wcb->images[j][i]->gray[r * ncolcb + c]; 

	  for (r = nrowcb + 3; r < nrow; r++)
	    for (c = 0; c < ncolcb; c++) 
	      fcb->gray[r * ncol + c + incrc] = 0.0;
	  incrc += ncolcb;
	}
      } 
    }

  for (c = 1; c < ncol; c++)
    fcb->gray[c] = 0.0;
  for (c = 4 * wcb->nlevel; c < ncol; c++) 
    fcb->gray[c + ncol] = fcb->gray[c + 2 * ncol] = 0.0;
}



static void
FCB2WCB(fcb, wcb)

  /*--- Translate a codebook sequence from the fimage ---*/
               /*--- to the wtrans2d format ---*/

Fimage        fcb;         /* Codebook in the fimage format */
Wtrans2d      wcb;         /* Codebook in the wtrans2d format */

{
  int           nlevel;
  int           j;	        /* Scale index in wav. transf. */
  int           i;	        /* Orientation index */
  long          r, c;           /* Indices for row and columns in codebooks */
  long          nrow, ncol;
  long          nrowcb, ncolcb; /* Number of rows and columns in sub-image 
				 * codebook in wcb */
  int           height, width;  /* Height and width of vectors */
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
fwlbg_adap_wcb(NumRecMax, Level, Orient, Edge_Ri, Ri2, FilterNorm, StopDecim, Width, Height, MultiCB, Lap, Sizec1, Sizec2, Sizec3, ThresVal1, ThresVal2, ThresVal3, OldCodeBook, OldAdapCodeBook2, OldAdapCodeBook3, Output2, Output3, Image2, Image3, Image4, ResCodeBook, ResResCodeBook, Image1, Ri, Output1)

int        *NumRecMax;          /* Number of level in Output1 */
int        *Level;              /* Generate codebook only at one scale */
int        *Orient;             /* Generate codebook only in one orientation */
Fimage	    Edge_Ri;		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
Fsignal     Ri2;		/* Impulse response of the low pass filter */
				/* for synthesis */
int        *FilterNorm;	        /* Equal 0 if no normalisation of filter's tap
			         *       1 if normalisation of the sum 
			         *       2 if normalistion of the square sum */
int        *StopDecim;          /* Level where decimation is cancelled */
int        *Width;              /* Width of block */
int        *Height;             /* Height of block */
int        *MultiCB;            /* Generates all code books of size equal 
				 * to a power of 2 */
int        *Lap;                /* Take overlapping vectors 
				 * in training images */
int        *Sizec1, *Sizec2, *Sizec3; /* Size of first, second and third 
				 * codebooks  */
float      *ThresVal1, *ThresVal2, *ThresVal3;	/* threshold values  
    			         * for block energy */
Wtrans2d    OldCodeBook;        /* Modified sequence of first class codebook */
Wtrans2d    OldAdapCodeBook2, OldAdapCodeBook3; /* Modified sequence 
				 * of second and third class codebooks */
Wtrans2d   *Output2, *Output3;  /* Generated sequences of second and third 
				 * class codebook */
Fimage      Image2, Image3, Image4;  /* Images for training set */
Wtrans2d    ResCodeBook, ResResCodeBook; /* Generate resdu codebook after 
				 * quantization with ResCodeBook 
				 * and ResResCodeBook */ 
Fimage      Image1;             /* First image for training set */
Fsignal     Ri;			/* Impulse response of the low pass filter */
Wtrans2d   *Output1;            /* Sequence of generated codebooks */

{
  int           J;	        /* Current level of decomposition */
  int           J1, J2;         /* Initial and final level of decomposition */
  int           i;	      	/* Orientation index */
  int           i1, i2;      	/* Initial and final orientation index */
  Wtrans2d      TrainWtrans1, TrainWtrans2, TrainWtrans3, TrainWtrans4; 
                                /* Wavelet transforms for training set */
  int           Edge;	        /* Edge processing mode */
  int           FiltNorm;       /* Normalisation of filters */
  int           Precond;        /* Preconditionning mode for orthogonal 
				 * transform */
  int           NumRec;         /* Number of level in training wav. transf. */
  int           Ortho;          /* Computes orthogonal coefficients 
				 * for dy(o)wave */

  /*--- Wavelet decompositions of training images ---*/

  NumRec = *NumRecMax;
  if (Level)
    NumRec = *Level;
  TrainWtrans2 = TrainWtrans3 = TrainWtrans4 = NULL;

  if (Ri2) {
    Edge = 2;
    if (FilterNorm)
      FiltNorm = *FilterNorm;
    else
      FiltNorm = 1;
    INIT_RI(Ri, Ri2); 

    TrainWtrans1 = mw_new_wtrans2d();
    dybiowave2(&NumRec, StopDecim, &Ortho, &Edge, &FiltNorm, Image1, TrainWtrans1, Ri, Ri2);

    if (Image2) {
      REFRESH_FILTERS(Ri, Ri2);
      TrainWtrans2 = mw_new_wtrans2d();
      dybiowave2(&NumRec, StopDecim, &Ortho, &Edge, &FiltNorm, Image2, TrainWtrans2, Ri, Ri2);
    } 

    if (Image3) {
      REFRESH_FILTERS(Ri, Ri2);
      TrainWtrans3 = mw_new_wtrans2d();
      dybiowave2(&NumRec, StopDecim, &Ortho, &Edge, &FiltNorm, Image3, TrainWtrans3, Ri, Ri2);
    }

    if (Image4) {
      REFRESH_FILTERS(Ri, Ri2);
      TrainWtrans4 = mw_new_wtrans2d();
      dybiowave2(&NumRec, StopDecim, &Ortho, &Edge, &FiltNorm, Image4, TrainWtrans4, Ri, Ri2);      
    }
  } else
    {
      if (Edge_Ri)
	Edge = 3;
      else
	Edge = 2;
      if (FilterNorm)
	FiltNorm = *FilterNorm;
      else
	FiltNorm = 2;
      Precond = 0;

      TrainWtrans1 = mw_new_wtrans2d();
      dyowave2(&NumRec, StopDecim, &Ortho, &Edge, &Precond, &FiltNorm, Image1, TrainWtrans1, Ri, Edge_Ri);

      if (Image2) {
	TrainWtrans2 = mw_new_wtrans2d();
	dyowave2(&NumRec, StopDecim, &Ortho, &Edge, &Precond, &FiltNorm, Image2, TrainWtrans2, Ri, Edge_Ri);
      }

      if (Image3) {
	TrainWtrans3 = mw_new_wtrans2d();
	dyowave2(&NumRec, StopDecim, &Ortho, &Edge, &Precond, &FiltNorm, Image3, TrainWtrans3, Ri, Edge_Ri);
      }

      if (Image4) {
	TrainWtrans4 = mw_new_wtrans2d();
	dyowave2(&NumRec, StopDecim, &Ortho, &Edge, &Precond, &FiltNorm, Image4, TrainWtrans4, Ri, Edge_Ri);
      }
    }

  /*--- Construction of codebook(s) ---*/

  Edge = (Ri->size + 1) / 2;
  wlbg_adap(NumRecMax, Level, Orient, StopDecim, Lap, &Edge, Width, Height, MultiCB, Sizec1, Sizec2, Sizec3, ThresVal1, ThresVal2, ThresVal3, OldCodeBook, OldAdapCodeBook2, OldAdapCodeBook3, Output2, Output3, TrainWtrans2, TrainWtrans3, TrainWtrans4, ResCodeBook, ResResCodeBook, TrainWtrans1, Output1);

}



void
fwlbg_adap(NumRecMax, Level, Orient, Edge_Ri, Ri2, FilterNorm, StopDecim, Width, Height, MultiCB, Lap, Sizec1, Sizec2, Sizec3, ThresVal1, ThresVal2, ThresVal3, OldCodeBook, OldAdapCodeBook2, OldAdapCodeBook3, Output2, Output3, Image2, Image3, Image4, ResCodeBook, ResResCodeBook, Image1, Ri, Output1)

int        *NumRecMax;          /* Number of level in Output1 */
int        *Level;              /* Generate codebook only at one scale */
int        *Orient;             /* Generate codebook only in one orientation */
Fimage	    Edge_Ri;		/* Impulse responses of filters for special 
				 * edge processing (including preconditionning 
				 * matrices */
Fsignal     Ri2;		/* Impulse response of the low pass filter */
				/* for synthesis */
int        *FilterNorm;	        /* Equal 0 if no normalisation of filter's tap
			         *       1 if normalisation of the sum 
			         *       2 if normalistion of the square sum */
int        *StopDecim;      /* Level where decimation is cancelled */
int        *Width;              /* Width of block */
int        *Height;             /* Height of block */
int        *MultiCB;            /* Generates all code books of size equal 
				 * to a power of 2 */
int        *Lap;                /* Take overlapping vectors 
				 * in training images */
int        *Sizec1, *Sizec2, *Sizec3; /* Size of first, second and third 
				 * codebooks  */
float      *ThresVal1, *ThresVal2, *ThresVal3;	/* threshold values  
    			         * for block energy */
Fimage      OldCodeBook;        /* Modified sequence of first class codebook */
Fimage      OldAdapCodeBook2, OldAdapCodeBook3; /* Modified sequence 
				 * of second and third class codebooks */
Fimage      Output2, Output3;  /* Generated sequences of second and third 
				 * class codebook */
Fimage      Image2, Image3, Image4;  /* Images for training set */
Fimage      ResCodeBook, ResResCodeBook; /* Generate resdu codebook after 
				 * quantization with ResCodeBook 
				 * and ResResCodeBook */ 
Fimage      Image1;             /* First image for training set */
Fsignal     Ri;			/* Impulse response of the low pass filter */
Fimage      Output1;            /* Sequence of generated codebooks */

{
  Wtrans2d    WOldCodeBook;     /* Modified sequence of first class codebook */
  Wtrans2d    WOldAdapCodeBook2, WOldAdapCodeBook3; /* Modified sequence 
				 * of second and third class codebooks */
  Wtrans2d    WOutput2, WOutput3;  /* Generated sequences of second and third 
				 * class codebook */
  Wtrans2d    WResCodeBook, WResResCodeBook; /* Generate residu codebook after 
				 * quantization with ResCodeBook 
				 * and ResResCodeBook */ 
  Wtrans2d    WOutput1;            /* Sequence of generated codebooks */

  /*--- Modification of format for old codebooks ---*/

  WOldCodeBook = WOldAdapCodeBook2 = WOldAdapCodeBook3 = NULL;
  WResCodeBook = WResResCodeBook = NULL;

  if (OldCodeBook) {
    WOldCodeBook = mw_new_wtrans2d();
    FCB2WCB(OldCodeBook, WOldCodeBook);
  }
  if (OldAdapCodeBook2) {
    WOldAdapCodeBook2 = mw_new_wtrans2d();
    FCB2WCB(OldAdapCodeBook2, WOldAdapCodeBook2);
  }
  if (OldAdapCodeBook3) {
    WOldAdapCodeBook3 = mw_new_wtrans2d();
    FCB2WCB(OldAdapCodeBook3, WOldAdapCodeBook3);
  }

  /*--- Modification of format for codebooks for multistage quantization ---*/

  if (ResCodeBook) {
    WResCodeBook = mw_new_wtrans2d();
    FCB2WCB(ResCodeBook, WResCodeBook);
  }
  if (ResResCodeBook) {
    WResResCodeBook = mw_new_wtrans2d();
    FCB2WCB(ResResCodeBook, WResResCodeBook);
  }

  /*--- Construction of new codebooks ---*/

  fwlbg_adap_wcb(NumRecMax, Level, Orient, Edge_Ri, Ri2, FilterNorm, StopDecim, Width, Height, MultiCB, Lap, Sizec1, Sizec2, Sizec3, ThresVal1, ThresVal2, ThresVal3, WOldCodeBook, WOldAdapCodeBook2, WOldAdapCodeBook3, &WOutput2, &WOutput3, Image2, Image3, Image4, WResCodeBook, WResResCodeBook, Image1, Ri, &WOutput1);

  /*--- Modification of format for output codebooks ---*/

  WCB2FCB(WOutput1, Output1);
  if (Output2)
    WCB2FCB(WOutput2, Output2);
  if (Output3)
    WCB2FCB(WOutput3, Output3);
   
}



/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {wlbg_adap};
version = {"2.00"};
author = {"Jean-Pierre D'Ales"};
function = {"Generates training set(s) (starting from wavelet transform(s)) and constructs a codebook sequence for wavelet transform vector quantization using LBG algorithm"};
usage = {
'r':[MaxLevel=1]->NumRecMax [1,16]
	"Quantization is performed on MaxLevel scales", 
'q':Level->Level [1,16]
	"Create codebook(s) at scale Level", 
'o':Orient->Orient [0,3]
	"Create codebook(s) for orientation Orient",
'd':[Dyad=16]->Dyadic [1,16]
	"Training wav. trans. are dyadic from scale Dyad",
'l'->Lap
	"Take overlapping vectors in training images", 
'e':[Edge=0]->Edge
        "Do not take overlapping vectors if the distance to an edge is smaller than Edge (default : 0)",
'w':[VectorWidth=2]->Width
	"Width of vectors (default : 2)", 
'h':[VectorHeight=2]->Height
	"Height of vectors (default : 2)", 
'M'->MultiCB
        "Generate codebooks of size equal to a power of two",
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
	"Modify the first class codebook sequence (wtrans2d)",
'X':OldAdapCodeBook1->OldAdapCodeBook2
	"Modify the second class codebook sequence (wtrans2d)",
'Y':OldAdapCodeBook2->OldAdapCodeBook3
	"Modify the third class codebook sequence (wtrans2d)",
'x':AdapCodeBook2<-Output2
	"Sequence of second class codebooks (wtrans2d)",
'y':AdapCodeBook3<-Output3
	"Sequence of third class codebooks (wtrans2d)",
'A':TrainWavTrans2->TrainWtrans2
	"Training wavelet transform (wtrans2d)", 
'B':TrainWavTrans3->TrainWtrans3
	"Training wavelet transform (wtrans2d)", 
'C':TrainWavTrans4->TrainWtrans4
	"Training wavelet transform (wtrans2d)", 
'Q':ResCodeBook->ResCodeBook
	"Generate codebook(s) for residu of quantization with ResCodeBook (wtrans2d)",
'R':ResResCodeBook->ResResCodeBook
	"Generate codebook(s) for residu of quantization with ResCodeBook and ResResCodeBook (wtrans2d)",
TrainWavTrans1->TrainWtrans1
	"Training wavelet transform (wtrans2d)", 
CodeBook1<-Output1
	"Sequence of first class codebooks (wtrans2d)"
	};
*/


/*--- Include files UNIX C ---*/

/*--- Megawave2 library ---*/
#include  "mw.h"

/*--- Megawave2 modules definition ---*/

#ifdef __STDC__
void flbg(int *, int *, int *, int *, int *, int *, Fsignal, int *, int *, Fimage, Fimage, Fimage, Fimage, Fimage, Fimage, Fimage, int *, int *, int *, int *, Fimage, Fimage, Fimage, Fimage, float *);
void flbg_adap(int *, int *, int *, int *, int *, int *, float *, float *, float *, Fsignal, int *, int *, int *, int *, int *, Fimage, Fimage, Fimage, Fimage, Fimage, Fimage, Fimage, Fimage, Fimage, Fimage, Fimage, Fimage);
#else
void flbg();
void flbg_adap();
#endif

/*--- Constants ---*/



static void
WSUBIMCHANGE(codebook, nlevel)

Wtrans2d      codebook;
int           nlevel;

{
  int     i, j;

  for (j = nlevel + 1; j <= codebook->nlevel; j++)
    for (i = 0; i <= 3; i++)
      if (mw_change_fimage(codebook->images[j][i], 1, 1) == NULL)
	mwerror(FATAL, 1, "Not enough memory for wavelet transform buffer for codebook.\n");

}


static int
count_cb(codebook)

  /*--- Count and return the number of codebooks contained ---*/
             /*--- in the fimage 'codebook' ---*/

Fimage     codebook;

{
  int      size, sizei, sizef;
  int      n;

  if (codebook) {
    sizei = nint(codebook->gray[(codebook->nrow - 5) * codebook->ncol]);
    sizef = nint(codebook->gray[(codebook->nrow - 6) * codebook->ncol]);
    n = 2;
    size = 1;
    while (size <= sizei)
      size *= 2;  
    while (size < sizef) {
      size *= 2;
      n++;
    }
  } else
    n = 1;

  return(n);
}




void
wlbg_adap(NumRecMax, Level, Orient, Dyadic, Lap, Edge, Width, Height, MultiCB, Sizec1, Sizec2, Sizec3, ThresVal1, ThresVal2, ThresVal3, OldCodeBook, OldAdapCodeBook2, OldAdapCodeBook3, Output2, Output3, TrainWtrans2, TrainWtrans3, TrainWtrans4, ResCodeBook, ResResCodeBook, TrainWtrans1, Output1)

int        *NumRecMax;          /* Number of level in Output1 */
int        *Level;              /* Generate codebook only at one scale */
int        *Orient;             /* Generate codebook only in one orientation */
int        *Dyadic;             /* Training set is formed of dyadic 
				 * wav. trans. from level Dyadic */
int        *Lap;                /* Take overlapping vectors 
				 * in training images */
int        *Edge;               /* Overlapping inhibited near edges */
int        *Width;              /* Width of block */
int        *Height;             /* Height of block */
int        *MultiCB;            /* Generates all code books of size equal 
				 * to a power of 2 */
int        *Sizec1, *Sizec2, *Sizec3; /* Size of first, second and third 
				 * codebooks  */
float      *ThresVal1, *ThresVal2, *ThresVal3;	/* threshold values  
    			         * for block energy */
Wtrans2d    OldCodeBook;        /* Modified sequence of first class codebook */
Wtrans2d    OldAdapCodeBook2, OldAdapCodeBook3; /* Modified sequence 
				 * of second and third class codebooks */
Wtrans2d   *Output2, *Output3;  /* Generated sequences of second and third 
				 * class codebook */
Wtrans2d    TrainWtrans2, TrainWtrans3, TrainWtrans4; /* Wavelet 
			         * transforms for training set */
Wtrans2d    ResCodeBook, ResResCodeBook; /* Generate resdu codebook after 
				 * quantization with ResCodeBook 
				 * and ResResCodeBook */ 
Wtrans2d    TrainWtrans1;       /* First wavelet transform for training set */
Wtrans2d   *Output1;            /* Sequence of generated codebooks */

{
  int             J;	        /* Current level of decomposition */
  int             J1, J2;       /* Initial and final level of decomposition */
  int             i;	      	/* Orientation index */
  int             i1, i2;      	/* Initial and final orientation index */
  int             testres;
  int             nres, nresres;  /* Index of codebook to use in ResCodeBook 
				 * and ResResCodeBook */ 
  int            *pnres, *pnresres;  /* Pointers to mres and nresres */
  float           testsize;
  int             sizec, sizeb;
  int             Print;
  int             decim;
  int		  numrec;
  long            nrow, ncol;
  float	          mse;
  float           thres1, thres2, thres3;
  Fimage          ResCB, ResResCB;  /* Residual codebooks for subimages */
  Fimage          AdapCB2, AdapCB3; /* Codebooks for adaptive quantization 
				     * of details */
  Fimage          TrainIm1, TrainIm2, TrainIm3, TrainIm4;   /* Training 
			          * sub-images of wavelet coefficients */

  /*--- Memory allocation or initialization for Output1 ---*/

  nrow = 2;
  ncol = 2;
  numrec = *NumRecMax;
  if (OldCodeBook && (*NumRecMax < OldCodeBook->nlevel))
    numrec = OldCodeBook->nlevel;
    
  for (J = 1; J < numrec; J++) {
    nrow *= 2;
    ncol *= 2;
  }

  if (OldCodeBook) {
    if (numrec > OldCodeBook->nlevel) {
      *Output1 = mw_new_wtrans2d(); 
      if (mw_alloc_ortho_wtrans2d(*Output1, numrec, nrow, ncol) == NULL)
	mwerror(FATAL, 1, "Allocation of wavelet transform for codebook refused!\n");
      for (J = 1; J <= OldCodeBook->nlevel; J++) 
	for (i = 0; i <= 3; i++)
	  (*Output1)->images[J][i] = OldCodeBook->images[J][i];
      WSUBIMCHANGE(*Output1, OldCodeBook->nlevel);
    } else
      *Output1 = OldCodeBook;
  } else 
    { 
      *Output1 = mw_new_wtrans2d(); 
      if (mw_alloc_ortho_wtrans2d(*Output1, numrec, nrow, ncol) == NULL)
	mwerror(FATAL, 1, "Allocation of wavelet transform for codebook refused!\n");
      WSUBIMCHANGE(*Output1, 0);
    }

  /*--- Memory allocation or initialization for Output2 ---*/

  if (OldAdapCodeBook2) {
    if (numrec > OldAdapCodeBook2->nlevel) {
      *Output2 = mw_new_wtrans2d(); 
      if (mw_alloc_ortho_wtrans2d(*Output2, numrec, nrow, ncol) == NULL)
	mwerror(FATAL, 1, "Allocation of wavelet transform for adaptive codebook 2 refused!\n");
      for (J = 1; J <= OldAdapCodeBook2->nlevel; J++) 
	for (i = 0; i <= 3; i++)
	  (*Output2)->images[J][i] = OldAdapCodeBook2->images[J][i];
      WSUBIMCHANGE(*Output2, OldAdapCodeBook2->nlevel);
    } else
      *Output2 = OldAdapCodeBook2; 
  } else
    {
      if (Sizec2) {
	*Output2 = mw_new_wtrans2d(); 
	if (mw_alloc_ortho_wtrans2d(*Output2, numrec, nrow, ncol) == NULL)
	  mwerror(FATAL, 1, "Allocation of wavelet transform for adaptive codebook 2 refused!\n");
	WSUBIMCHANGE(*Output2, 0);
      } else 
	*Output2 = NULL;
    }

  /*--- Memory allocation or initialization for Output3 ---*/

  if (OldAdapCodeBook3) {
    if (numrec > OldAdapCodeBook3->nlevel) {
      *Output3 = mw_new_wtrans2d(); 
      if (mw_alloc_ortho_wtrans2d(*Output3, numrec, nrow, ncol) == NULL)
	mwerror(FATAL, 1, "Allocation of wavelet transform for adaptive codebook 3 refused!\n");
      for (J = 1; J <= OldAdapCodeBook3->nlevel; J++) 
	for (i = 0; i <= 3; i++)
	  (*Output3)->images[J][i] = OldAdapCodeBook3->images[J][i];
      WSUBIMCHANGE(*Output3, OldAdapCodeBook3->nlevel);
    } else
    *Output3 = OldAdapCodeBook3; 
  } else
    {
      if (Sizec3) {
	*Output3 = mw_new_wtrans2d(); 
	if (mw_alloc_ortho_wtrans2d(*Output3, numrec, nrow, ncol) == NULL)
	  mwerror(FATAL, 1, "Allocation of wavelet transform for adaptive codebook 3 refused!\n");
	WSUBIMCHANGE(*Output3, 0);
      } else 
	*Output3 = NULL;
    }

  if (*Output2) 
    ResCodeBook = ResResCodeBook = NULL;

  if (ThresVal1)
    thres1 = *ThresVal1;
  if (ThresVal2)
    thres2 = *ThresVal2;
  if (ThresVal3)
    thres3 = *ThresVal3;  

  /*--- Determine which sub-image(s) should be considered ---*/

  decim = 1;
  if (Level) {
    J1 = *Level;
    J2 = *Level;
  } else
    {
      J1 = 1;
      J2 = numrec;
    }

  if (Orient) {
    i1 = *Orient;
    i2 = *Orient;
  } else
    {
      i1 = 1;
      i2 = 3;
    }

  for (J = *Dyadic; J <= J2; J++)
    decim *= 2;

  nrow = TrainWtrans1->nrow;
  ncol = TrainWtrans1->ncol;
  for (J = 1; J <= J2; J++) {
    nrow /= 2;
    ncol /= 2;
  }

  if (!Orient) 
    if (Level) {
      if (*Level == numrec)
	i1 = 0;
    } else
      {

	/*--- Genarate codebook for average sub-image at level numrec ---*/

	testres  = 1;
	if (ResCodeBook) {

	  /*--- Check that ResCodeBook content is consistent ---*/
	  /*--- If not, do not generate codebook for average ---*/

	  sizeb = ResCodeBook->images[J2][0]->ncol;
	  sizec = ResCodeBook->images[J2][0]->nrow;
	  if (sizec <= 4)
	    testres = 0;
	  if (MultiCB) {
	    testsize = (float) nint(ResCodeBook->images[J2][0]->gray[(sizec - 6) * sizeb]);
	    if ((ResCodeBook->images[J2][0]->gray[(sizec - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	      testres = 0;
	    testsize = (float) nint(ResCodeBook->images[J2][0]->gray[(sizec - 5) * sizeb]);
	    if ((ResCodeBook->images[J2][0]->gray[(sizec - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	      testres = 0;
	  }
	}
	if (ResResCodeBook) {

	  /*--- Check that ResResCodeBook content is consistent ---*/
	    /*--- If not, do not generate codebook for average ---*/

	  sizeb = ResResCodeBook->images[J2][0]->ncol;
	  sizec = ResResCodeBook->images[J2][0]->nrow;
	  if (sizec <= 4)
	    testres = 0;
	  if (MultiCB) {
	    testsize = (float) nint(ResResCodeBook->images[J2][0]->gray[(sizec - 6) * sizeb]);
	    if ((ResResCodeBook->images[J2][0]->gray[(sizec - 6) * sizeb] != testsize) || (testsize < 1.0)) 
	      testres = 0;
	    testsize = (float) nint(ResResCodeBook->images[J2][0]->gray[(sizec - 5) * sizeb]);
	    if ((ResResCodeBook->images[J2][0]->gray[(sizec - 5) * sizeb] != testsize) || (testsize < 1.0)) 
	      testres = 0;
	  }
	}

	if (testres == 1) {
	  pnres = pnresres = NULL;
	  if (MultiCB)
	    if (ResCodeBook) {
	      nres = count_cb(ResCodeBook->images[J2][0]) - 1;
	      pnres = &nres;
	      if (ResResCodeBook) {
		nresres = count_cb(ResResCodeBook->images[J2][0]) - 1;
		pnresres = &nresres;
	      }
	    }

	  if (ResCodeBook)
	    ResCB = ResCodeBook->images[J2][0];
	  else
	    ResCB = NULL;
	  if (ResResCodeBook)
	    ResResCB = ResResCodeBook->images[J2][0];
	  else
	    ResResCB = NULL;
	  TrainIm1 = TrainWtrans1->images[J2][0];
	  if (TrainWtrans2)
	    TrainIm2 = TrainWtrans2->images[J2][0];
	  else
	    TrainIm2 = NULL;
	  if (TrainWtrans3)
	    TrainIm3 = TrainWtrans3->images[J2][0];
	  else
	    TrainIm3 = NULL;
	  if (TrainWtrans4)
	    TrainIm4 = TrainWtrans4->images[J2][0];
	  else
	    TrainIm4 = NULL;

	  flbg(Sizec1, Width, Height, Lap, &decim, Edge, NULL, MultiCB, &Print, TrainIm2, TrainIm3, TrainIm4, NULL, NULL, NULL, NULL, NULL, NULL, pnres, pnresres, ResCB, ResResCB, TrainIm1, (*Output1)->images[J2][0], &mse);
	  printf("m.s.e. 0 = %f\n", mse);
	}
      }

  for (J = J2; J >= J1; J--) {
    
    printf("\nLevel %d:\n", J);
 
    for (i = i1; i <= i2; i++) {
      
      /*--- Genarate codebook for sub-image at level J and orientation i ---*/

      TrainIm1 = TrainWtrans1->images[J][i];
      if (TrainWtrans2)
	TrainIm2 = TrainWtrans2->images[J][i];
      else
	TrainIm2 = NULL;
      if (TrainWtrans3)
	TrainIm3 = TrainWtrans3->images[J][i];
      else
	TrainIm3 = NULL;
      if (TrainWtrans4)
	TrainIm4 = TrainWtrans4->images[J][i];
      else
	TrainIm4 = NULL;

      if (ThresVal1 && (i > 0)) {

	/*--- Generate adaptive codebooks for classified quantization ---*/

	if (*Output2)
	  AdapCB2 = (*Output2)->images[J][i];
	else
	  AdapCB2 = NULL;
	if (*Output3)
	  AdapCB3 = (*Output3)->images[J][i];
	else
	  AdapCB3 = NULL;

	flbg_adap(Sizec1, Width, Height, Lap, &decim, Edge, ThresVal1, ThresVal2, ThresVal3, NULL, MultiCB, &Print, Sizec2, Sizec3, NULL, TrainIm2, TrainIm3, TrainIm4, NULL, NULL, NULL, NULL, AdapCB2, AdapCB3, NULL, TrainIm1, (*Output1)->images[J][i]);
      } else
	{

	  /*--- Generate codebooks for plain vector quantization ---*/

	  testres  = 1;
	  if (ResCodeBook) {

	    /*--- Check that ResCodeBook content is consistent ---*/
	   /*--- If not, do not generate codebook for sub-image ---*/

	    sizeb = ResCodeBook->images[J][i]->ncol;
	    sizec = ResCodeBook->images[J][i]->nrow;
	    if (sizec <= 4)
	      testres = 0;
	    if (MultiCB) {
	      testsize = (float) nint(ResCodeBook->images[J][i]->gray[(sizec - 6) * sizeb]);
	      if ((ResCodeBook->images[J][i]->gray[(sizec - 6) * sizeb] != testsize) || (testsize < 1.0)) 
		testres = 0;
	      testsize = (float) nint(ResCodeBook->images[J][i]->gray[(sizec - 5) * sizeb]);
	      if ((ResCodeBook->images[J][i]->gray[(sizec - 5) * sizeb] != testsize) || (testsize < 1.0)) 
		testres = 0;
	    }
	  }
	  if (ResResCodeBook) {

	    /*--- Check that ResResCodeBook content is consistent ---*/
	     /*--- If not, do not generate codebook for sub-image ---*/

	    sizeb = ResResCodeBook->images[J][i]->ncol;
	    sizec = ResResCodeBook->images[J][i]->nrow;
	    if (sizec <= 4)
	      testres = 0;
	    if (MultiCB) {
	      testsize = (float) nint(ResResCodeBook->images[J][i]->gray[(sizec - 6) * sizeb]);
	      if ((ResResCodeBook->images[J][i]->gray[(sizec - 6) * sizeb] != testsize) || (testsize < 1.0)) 
		testres = 0;
	      testsize = (float) nint(ResResCodeBook->images[J][i]->gray[(sizec - 5) * sizeb]);
	      if ((ResResCodeBook->images[J][i]->gray[(sizec - 5) * sizeb] != testsize) || (testsize < 1.0)) 
		testres = 0;
	    }
	  }

	  if (testres == 1) {
	    pnres = pnresres = NULL;
	    if (MultiCB)
	      if (ResCodeBook) {
		nres = count_cb(ResCodeBook->images[J][i]) - 1;
		pnres = &nres;
		if (ResResCodeBook) {
		  nresres = count_cb(ResResCodeBook->images[J][i]) - 1;
		  pnresres = &nresres;
		}
	      }

	    if (ResCodeBook)
	      ResCB = ResCodeBook->images[J][i];
	    else
	      ResCB = NULL;
	    if (ResResCodeBook)
	      ResResCB = ResResCodeBook->images[J][i];
	    else
	      ResResCB = NULL;

	    flbg(Sizec1, Width, Height, Lap, &decim, Edge, NULL, MultiCB, &Print, TrainIm2, TrainIm3, TrainIm4, NULL, NULL, NULL, NULL, NULL, NULL, pnres, pnresres, ResCB, ResResCB, TrainIm1, (*Output1)->images[J][i], &mse);
	    printf("m.s.e. %d = %f\n", i, mse);
	  }
	}

      if (ThresVal1)
	*ThresVal1 = thres1;
      if (ThresVal2)
	*ThresVal2 = thres2;
      if (ThresVal3)
	*ThresVal3 = thres3;
    }

    nrow *= 2;
    if ((*Width <= 2) && (nrow >= 128))
      *Width *= 2;
    ncol *= 2;
    if ((*Height <= 2) && (ncol >= 128))
      *Height *= 2;

    if (decim > 1)
      decim /= 2;

  }

}

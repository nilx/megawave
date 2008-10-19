/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {mk_trainset};
version = {"1.1"};
author = {"Jean-Pierre D'Ales"};
function = {"Generates a training set of rectangular blocks from images"};
usage = {
 'w':[VectorWidth=2]->Width    "Width of blocks", 
 'h':[VectorHeight=2]->Height  "Height of blocks", 
 'l'->Lap                      "Take overlapping blocks in training images", 
 'd':[Decim=1]->Decim          "Decimation factor in training images (for wavelet transforms)",
 'e':[Edge=0]->Edge         "Do not take overlapping vectors if the distance to an edge is smaller than Edge",
 't':ThresVal1->ThresVal1   "First threshold value for classified VQ",
 'u':ThresVal2->ThresVal2   "Second threshold value for classified VQ",
 'v':ThresVal3->ThresVal3   "Third threshold value for classified VQ",
 'f':SizeCB->SizeCB         "Size of Codebook",
 'a':Image2->Image2         "Training image (fimage)",
 'b':Image3->Image3         "Training image (fimage)",
 'c':Image4->Image4         "Training image (fimage)",
 'A':Image5->Image5         "Training image (fimage)",
 'B':Image6->Image6         "Training image (fimage)",
 'C':Image7->Image7         "Training image (fimage)",
 'D':Image8->Image8         "Training image (fimage)",
 'm':TrainingSet2<-Result2  "Resulting training set for second class (fimage)",
 'n':TrainingSet3<-Result3  "Resulting training set for third class (fimage)",
 'o':TrainingSet4<-Result4  "Resulting training set for fourth class (fimage)",
 Image->Image               "Training image (fimage)", 
 TrainingSet<-Result        "Resulting training set for first class (fimage)"
	};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include  "mw.h"

/*--- Constants ---*/

#define FAC_SIZE_CBTS  10    /* Minimum value for size of tr. set / size of 
			      * codebook */
#define MAX_NITER 20



static void
THRES_ADAP(width, height, thresval1, thresval2, thresval3, image, image2, image3, image4, image5, image6, image7, image8)

  /*--- Compute threshold values fo adaptive quantization ---*/

int         width, height;                      /* Width and height of block */
float	   *thresval1, *thresval2, *thresval3;  /* Block energy thresholds 
						 * between codebooks */
Fimage      image, image2, image3, image4, image5, image6, image7, image8;
                                                /* Training images */

{
  double    energy;
  long      x, size, sizetot;
  int       sizeb;		/* Size of blocks */

  energy = 0.0;
  sizeb = width * height;

  size = sizetot = image->nrow * image->ncol;
  for (x = 0; x < size; x++)
    energy += image->gray[x] * image->gray[x];

  if (image2) {
    size = image2->nrow * image2->ncol;
    for (x = 0; x < size; x++)
      energy += image2->gray[x] * image2->gray[x];
    sizetot += size;
  }
  if (image3) {
    size = image3->nrow * image3->ncol;
    for (x = 0; x < size; x++)
      energy += image3->gray[x] * image3->gray[x];
    sizetot += size;
  }
  if (image4) {
    size = image4->nrow * image4->ncol;
    for (x = 0; x < size; x++)
      energy += image4->gray[x] * image4->gray[x];
    sizetot += size;
  }
  if (image5) {
    size = image5->nrow * image5->ncol;
    for (x = 0; x < size; x++)
      energy += image5->gray[x] * image5->gray[x];
    sizetot += size;
  }
  if (image6) {
    size = image6->nrow * image6->ncol;
    for (x = 0; x < size; x++)
      energy += image6->gray[x] * image6->gray[x];
    sizetot += size;
  }
  if (image7) {
    size = image7->nrow * image7->ncol;
    for (x = 0; x < size; x++)
      energy += image7->gray[x] * image7->gray[x];
    sizetot += size;
  }
  if (image8) {
    size = image8->nrow * image8->ncol;
    for (x = 0; x < size; x++)
      energy += image8->gray[x] * image8->gray[x];
    sizetot += size;
  }

  energy /= (double) sizetot;

  if (*thresval1 > 0.0)
    *thresval1 *= sizeb * energy;
  else
    *thresval1 = 3.0 * sizeb * energy;

  if (thresval2)
  {
    if (*thresval2 > 0.0)
      *thresval2 *= sizeb * energy;
    else
      *thresval2 = 1.75 * sizeb * energy;
  }
  if (thresval3) 
  {
    if (*thresval3 > 0.0)
      *thresval3 *= sizeb * energy;
    else
      *thresval3 = sizeb * energy;
  }
}




static long
SIZE_TS(imgsize, vecsize, lap, decim, edge)

  /*--- Compute and return number of training vectors extracted ---*/
          /*--- along the lines or columns of an image ---*/

long	    imgsize;	       	/* Number of row or column in image */
long	    vecsize;	       	/* Number of row or column in blocks */
int        *lap;                /* Take overlapping blocks 
				 * in training images */
int         decim;              /* Space between pixels in a vector */
int         edge;               /* No overlapping on edges */

{
    long	    size;
    
    if (lap)
      if (edge>1) {
	if (2*edge >= imgsize / decim)
	  size = (imgsize / decim / vecsize) * decim;
	else
	  size = (imgsize / decim - (vecsize - 1 + 2 * (edge - (edge + vecsize - 1) / vecsize))) * decim;
      } else
	size = (imgsize / decim - (vecsize - 1)) * decim;
    else
	size = (imgsize / decim / vecsize) * decim;
	
    return(size);
}




static unsigned char
SWITCH_TS(energy, tv1, tv2, tv3)

  /*--- Return the adaptive class according to the comparison ---*/ 
        /*--- between energy value and threshold values ---*/

float	    energy;           /*--- Energy of the block ---*/
float	   *tv1, *tv2, *tv3;  /*--- Threshold values ---*/

{
  unsigned char i;
    
  if (energy >= *tv1)
    i = 1;
  else
    if (tv2) {
      if (energy >= *tv2)
	i = 2;
      else
	if (tv3) {
	  if (energy >= *tv3)
	    i = 3;
	  else
	    i = 4;
	} else
	  i = 3;
    } else
      i = 2;
    
  return(i);
}




static double
ENERGY_BLOCK(image, decim, width, height, ldxi, imgj)

  /*--- Compute the energy of a block ---*/

Fimage      image;            /* Training image */
int         decim;            /* Space between pixels in a vector */
int         width, height;    /* Width and height of block */
long	    ldxi, imgj;

{
  long      r, c;
  long      incldxi;
  double    energy;

  energy = 0.0;
  incldxi = decim * image->ncol;
  for (r = 0; r < height; r++) {
    for (c = 0; c < width; c++)
      energy += image->gray[ldxi + imgj + decim * c] * image->gray[ldxi + imgj + decim * c];
    ldxi += incldxi;
  }

  return(energy);
}




static void
SEARCH_LINE_TS(image, ldxi, indts, t, width, height, lap, decim, edge, thresval1, thresval2, thresval3)

  /*--- Search for the adaptive class of the blocks in a given line ---*/

Fimage      image;            /* Training image */
long	    ldxi;             /* index of the first pixel of the line 
			       * in image */
Cimage	    indts;	      /* Index of training set for image block 't' */
long       *t;                /* index of the vector currently added 
			       * to the training set */
int         width, height;    /* Width and height of block */
int        *lap;              /* Flag for taking overlapping 
			       * blocks in training images */
int         decim;            /* Space between pixels in a vector */
int         edge;             /* No overlapping on edges */
float	   *thresval1, *thresval2, *thresval3;  /* Block energy thresholds 
						 * between codebooks */

{
  long      j;
  long	    imgj;
  int       incj;
  long      dx;
  double    energy;
  int       d;

  if (lap) { 
    incj = decim;
    dx = image->ncol / decim;
  } else
    {
      incj = width * decim;
      dx = image->ncol / decim / width;
    }

  for (d=0; d<decim; d++) {
    if (lap) {
      imgj = d;
      for (j = 0; j < (edge + width - 1) / width; j++) {
	energy = ENERGY_BLOCK(image, decim, width, height, ldxi, imgj);
	indts->gray[*t] = SWITCH_TS(energy, thresval1, thresval2, thresval3);
	imgj += decim * width;
	(*t)++;
      }
    }

    imgj = d + edge * decim;
    while (imgj + decim * (width - 1) < image->ncol - edge * decim + d) {
      energy = ENERGY_BLOCK(image, decim, width, height, ldxi, imgj);
      indts->gray[*t] = SWITCH_TS(energy, thresval1, thresval2, thresval3);
      imgj += incj;
      (*t)++;
    }
  
    if (lap) {
      imgj = d + image->ncol - decim * width;
      for (j = dx - 1; j > dx - (edge + width - 1) / width - 1; j--) {
	energy = ENERGY_BLOCK(image, decim, width, height, ldxi, imgj);
	indts->gray[*t] = SWITCH_TS(energy, thresval1, thresval2, thresval3);
	imgj -= decim * width;
	(*t)++;
      }
    }
  }
}



static void
SEARCH_TS(image, indts, width, height, lap, decim, edge, thresval1, thresval2, thresval3)

  /*--- Search for the adaptive class of the blocks in a given image ---*/

Fimage      image;            /* Training image */
Cimage	    indts;	      /* Index of training set for image block 't' */
int         width, height;    /* Width and height of block */
int        *lap;              /* Flag for taking overlapping 
			       * blocks in training images */
int         decim;            /* Space between pixels in a vector */
int         edge;             /* No overlapping on edges */
float	   *thresval1, *thresval2, *thresval3;  /* Block energy thresholds 
						 * between codebooks */

{
  long      dy;
  long      i;
  long	    t;
  int       inci;
  long	    incldxi;
  long	    ldxi;         /* index of the first pixel of the line in image */
  int       d;

  t = indts->firstrow;
  if (lap) {
    incldxi = decim * image->ncol;
    dy = image->nrow / decim;
    inci = decim;
  } else
    {
      inci = decim * height;
      incldxi = decim * height * image->ncol;
      dy = image->nrow / decim / width;
    }

  for (d=0; d<decim; d++) {
    if (lap) {
      ldxi = d * image->ncol;
      for (i = 0; i < (edge + height - 1) / height; i++) {
	SEARCH_LINE_TS(image, ldxi, indts, &t, width, height, lap, decim, edge, thresval1, thresval2, thresval3);
	ldxi += decim * height * image->ncol;
      }
    }

    ldxi = (d + edge * decim) * image->ncol;
    i = d + edge * decim;
    while (i + decim * (height - 1) < image->nrow - edge * decim + d) {
      SEARCH_LINE_TS(image, ldxi, indts, &t, width, height, lap, decim, edge, thresval1, thresval2, thresval3);
      ldxi += incldxi;
      i += inci;
    }
	
    if (lap) {
      ldxi = (d + image->nrow - decim * height) * image->ncol;
      for (i = dy - 1; i > dy - (edge + height - 1) / height - 1; i--) {
	SEARCH_LINE_TS(image, ldxi, indts, &t, width, height, lap, decim, edge, thresval1, thresval2, thresval3);
	ldxi -= decim * height * image->ncol;
      }
    }
  }

  indts->firstrow = t;

}




static void
COPY_BLOCK(image, indts, t, decim, result, result2, result3, result4, width, height, ldxi, imgj, ldxr, ldxr2, ldxr3, ldxr4)

  /*--- Extract a block from 'image' and copy it on specified ---*/
                /*--- training set ---*/

Fimage      image;            /* Training image */
Cimage	    indts;	      /* Index of training set for image block 't' */
long	    t;                /* Index of block im image */
int         decim;            /* Space between pixels in a vector */
Fimage      result, result2, result3, result4; /* Resulting training sets */
int         width, height;    /* Width and height of block */
long	    ldxi, imgj;       /* index of the first pixel of the line 
			       * and first pixel of the block in image */
long       *ldxr, *ldxr2, *ldxr3, *ldxr4;

{
  long            r, c;
    
  if (indts) 
    switch (indts->gray[t]) {
  case 1:
      for (r = 0; r < height; r++) {
	for (c = 0; c < width; c++)
	  result->gray[*ldxr + r * width + c] = image->gray[ldxi + imgj + decim * c];
	ldxi += decim * image->ncol;
      }
      *ldxr += result->ncol;
      break;    

  case 2:
      if (result2) {
	for (r = 0; r < height; r++) {
	  for (c = 0; c < width; c++)
	    result2->gray[*ldxr2 + r * width + c] = image->gray[ldxi + imgj + decim * c];
	  ldxi += decim * image->ncol;
	}
	*ldxr2 += result->ncol;
      }
      break;    

  case 3:
      if (result3) {
	for (r = 0; r < height; r++) {
	  for (c = 0; c < width; c++)
	    result3->gray[*ldxr3 + r * width + c] = image->gray[ldxi + imgj + decim * c];
	  ldxi += decim * image->ncol;
	}
	*ldxr3 += result->ncol;
      }
      break;    

  case 4:
      if (result4) {
	for (r = 0; r < height; r++) {
	  for (c = 0; c < width; c++)
	    result4->gray[*ldxr4 + r * width + c] = image->gray[ldxi + imgj + decim * c];
	  ldxi += decim * image->ncol;
	}
	*ldxr4 += result->ncol;
      }
      break;
    }  
  else
    {
      for (r = 0; r < height; r++) {
	for (c = 0; c < width; c++)
	  result->gray[*ldxr + r * width + c] = image->gray[ldxi + imgj + decim * c];
	ldxi += decim * image->ncol;
      }
      *ldxr += result->ncol;
    }
}




static void
COPY_LINE_OF_BLOCK(image, indts, lap, decim, edge, result, result2, result3, result4, width, height, t, ldxi, ldxr, ldxr2, ldxr3, ldxr4)

  /*--- Extract blocks from a line in 'image' and copy them on specified ---*/
                     /*--- training set ---*/

Fimage      image;            /* Training image */
Cimage	    indts;	      /* Index of training set for image block 't' */
int        *lap;              /* Take overlapping blocks 
			       * in training images */
int         decim;            /* Space between pixels in a vector */
int         edge;             /* No overlapping on edges */
Fimage      result, result2, result3, result4; /* Resulting training sets */
int         width, height;    /* Width and height of block */
long	   *t;                /* Index of block in training set */
long	    ldxi;             /* index of the first pixel of the line 
			       * in image */
long       *ldxr, *ldxr2, *ldxr3, *ldxr4;  /* Indices in training sets 
			       * of the first point of the vector 
			       * currently added */

{
  long            j;
  long            dx;
  long	          imgj;       /* index of the first pixel of the block 
			       * in image */
  int             incj;
  int             d;

  if (lap) { 
    incj = decim;
    dx = image->ncol / decim;
  } else
    {
      incj = width * decim;
      dx = image->ncol / decim / width;
    }

  for (d=0; d<decim; d++) {
    if (lap) {
      imgj = d;
      for (j = 0; j < (edge + width - 1) / width; j++) {
	COPY_BLOCK(image, indts, *t, decim, result, result2, result3, result4, width, height, ldxi, imgj, ldxr, ldxr2, ldxr3, ldxr4);
	imgj += decim * width;
	(*t)++;
      }
    }

    imgj = d + edge * decim;
    while (imgj + decim * (width - 1) < image->ncol - edge * decim + d) {
      COPY_BLOCK(image, indts, *t, decim, result, result2, result3, result4, width, height, ldxi, imgj, ldxr, ldxr2, ldxr3, ldxr4);
      imgj += incj;
      (*t)++;
    }
  
    if (lap) {
      imgj = d + image->ncol - decim * width;
      for (j = dx - 1; j > dx - (edge + width - 1) / width - 1; j--) {
	COPY_BLOCK(image, indts, *t, decim, result, result2, result3, result4, width, height, ldxi, imgj, ldxr, ldxr2, ldxr3, ldxr4);
	imgj -= decim * width;
	(*t)++;
      }
    }
  }
}


static void
ADD_IMAGE_ADAP(width, height, t, ldxr, ldxr2, ldxr3, ldxr4, lap, decim, edge, image, indts, result, result2, result3, result4)

  /*--- Extract vectors from an image and add them to the training set(s) ---*/

int             width, height;  /* Width and height of block */
long	       *t;
long           *ldxr, *ldxr2, *ldxr3, *ldxr4;  /* Indices in training sets 
				 * of the first point of the vector 
				 * currently added */
int            *lap;            /* Take overlapping blocks 
				 * in training images */
int             decim;          /* Space between pixels in a vector */
int             edge;           /* No overlapping on edges */
Fimage          image;          /* Input image */
Cimage          indts;		/* Index of training set for an image block */
Fimage          result, result2, result3, result4;

{
  long      sizeb;	      /* Size of blocks */
  long      dy;
  long      i;
  int       inci;
  long	    ldxi;             /* index of the first pixel of the line 
			       * in image */
  long	    incldxi;
  int       d;

  sizeb = height * width;

  if (lap) {
    inci = decim;
    incldxi = decim * image->ncol;
    dy = image->nrow / decim;
  } else
    {
      inci = decim * height;
      incldxi = decim * height * image->ncol;
      dy = image->nrow / decim / height;
    }

  for (d=0; d<decim; d++) {
    if (lap) {
      ldxi = d * image->ncol;
      for (i = 0; i < (edge + height - 1) / height; i++) {
	COPY_LINE_OF_BLOCK(image, indts, lap, decim, edge, result, result2, result3, result4, width, height, t, ldxi, ldxr, ldxr2, ldxr3, ldxr4);
	ldxi += decim * height * image->ncol;
      }
    }

    ldxi = (d + edge * decim) * image->ncol;
    i = d + edge * decim;
    while (i + decim * (height - 1) < image->nrow - edge * decim + d) {
      COPY_LINE_OF_BLOCK(image, indts, lap, decim, edge, result, result2, result3, result4, width, height, t, ldxi, ldxr, ldxr2, ldxr3, ldxr4);
      ldxi += incldxi;
      i += inci;
    }
	
    if (lap) {
      ldxi = (d + image->nrow - decim * height) * image->ncol;
      for (i = dy - 1; i > dy - (edge + height - 1) / height - 1; i--) {
	COPY_LINE_OF_BLOCK(image, indts, lap, decim, edge, result, result2, result3, result4, width, height, t, ldxi, ldxr, ldxr2, ldxr3, ldxr4);
	ldxi -= decim * height * image->ncol;
      }
    }
  }
}




static void
MK_TRAINSET_ADAP(width, height, lap, decim, edge, thresval1, thresval2, thresval3, sizecb, image, image2, image3, image4, image5, image6, image7, image8, result, result2, result3, result4)

  /*--- Construct adaptive training set(s) ---*/

int         width, height;      /* Width and height of block */
int        *lap;                /* Flag for taking overlapping 
				 * blocks in training images */
int         decim;              /* Space between pixels in a vector */
int         edge;               /* No overlapping on edges */
float	   *thresval1, *thresval2, *thresval3;  /* Block energy thresholds 
						 * between codebooks */
int        *sizecb;             /* Size of codebook */
Fimage      image, image2, image3, image4, image5, image6, image7, image8;
                                /* Training images */
Fimage      result, result2, result3, result4;  /* Resulting codebook */


{
  long      sizeb;		/* Size of blocks */
  long      sizet, sizet2, sizet3, sizet4; /* size of training sets */
  long      dx, dy;
  long	    imgj, t;
  long	    ldxi, ldxr, ldxr2, ldxr3, ldxr4;
  Cimage    indts;		/* Index of training set for an image block */
  int       testadap;
  int       niter;
  int       min_trainsize;

  sizeb = height * width;
  if (!lap)
    edge = 0;
  if (sizecb)
    min_trainsize = FAC_SIZE_CBTS * *sizecb;
  else
    min_trainsize = FAC_SIZE_CBTS * 2;

  THRES_ADAP(width, height, thresval1, thresval2, thresval3, image, image2, image3, image4, image5, image6, image7, image8);
    
  dx = SIZE_TS(image->nrow, height, lap, decim, edge);
  dy = SIZE_TS(image->ncol, width, lap, decim, edge);
  sizet = dx * dy;

  if (image2 != NULL) {
    dx = SIZE_TS(image2->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image2->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }
	
  if (image3 != NULL) {
    dx = SIZE_TS(image3->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image3->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }

  if (image4 != NULL) {
    dx = SIZE_TS(image4->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image4->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }
    
  if (image5 != NULL) {
    dx = SIZE_TS(image5->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image5->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }

  if (image6 != NULL) {
    dx = SIZE_TS(image6->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image6->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }
    
  if (image7 != NULL) {
    dx = SIZE_TS(image7->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image7->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }

  if (image8 != NULL) {
    dx = SIZE_TS(image8->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image8->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }
        
  indts = mw_new_cimage();
  if (mw_alloc_cimage(indts, sizet, 1) == NULL)
    mwerror(FATAL, 1, "Not enough memory for training set index buffer\n");

  printf("\n");
  sizet = 0;
  niter = 0;
  while ((sizet == 0) && (niter < MAX_NITER)) {
    niter++;
    printf("Thres1 = %.2f  ", *thresval1);
    if (thresval2)
      printf("Thres2 = %.2f  ", *thresval2);
    if (thresval3)
      printf("Thres3 = %.2f  ", *thresval3);
    printf("\n");

    indts->firstrow = 0;
    SEARCH_TS(image, indts, width, height, lap, decim, edge, thresval1, thresval2, thresval3);
	    
    if (image2 != NULL) 
      SEARCH_TS(image2, indts, width, height, lap, decim, edge, thresval1, thresval2, thresval3);

    if (image3 != NULL) 
      SEARCH_TS(image3, indts, width, height, lap, decim, edge, thresval1, thresval2, thresval3);
		    
    if (image4 != NULL) 
      SEARCH_TS(image4, indts, width, height, lap, decim, edge, thresval1, thresval2, thresval3);

    if (image5 != NULL) 
      SEARCH_TS(image5, indts, width, height, lap, decim, edge, thresval1, thresval2, thresval3);
		    
    if (image6 != NULL) 
      SEARCH_TS(image6, indts, width, height, lap, decim, edge, thresval1, thresval2, thresval3);

    if (image7 != NULL) 
      SEARCH_TS(image7, indts, width, height, lap, decim, edge, thresval1, thresval2, thresval3);
		    
    if (image8 != NULL) 
      SEARCH_TS(image8, indts, width, height, lap, decim, edge, thresval1, thresval2, thresval3);
	
    sizet = sizet2 = sizet3 = sizet4 = 0;
    for (t = 0; t < indts->nrow; t++)
      switch (indts->gray[t]) {
    case 1:
	sizet++;
	break;    
    case 2:
	sizet2++;
	break;    
    case 3:
	sizet3++;
	break;    
    case 4:
	sizet4++;
	break;
      }    
    
    if (sizet < min_trainsize) {
      if (niter == MAX_NITER) 
	mwerror(WARNING, 0, "Not enough vectors in training set 1\n");
      else
	{
	  sizet = 0;
	  *thresval1 /= 1.5;
	  if (thresval2)
	    *thresval2 /= 1.5;
	  if (thresval3)
	    *thresval3 /= 1.5;
	}
    } else
      if (thresval2 && (sizet2 < min_trainsize)) {
	if (niter == MAX_NITER) 
	  mwerror(WARNING, 0, "Not enough vectors in training set 2\n");
	else
	  {
	    sizet = 0;
	    *thresval2 /= 1.5;
	    if (thresval3)
	      *thresval3 /= 1.5;
	  }
      } else
	if (thresval3 && (sizet3 < min_trainsize)) {
	  if (niter == MAX_NITER) 
	    mwerror(WARNING, 0, "Not enough vectors in training set 3\n");
	  else
	    {
	      sizet = 0;
	      *thresval3 /= 1.5;
	    }
	}
  }

  result = mw_change_fimage(result, sizet, sizeb);
  if (result == NULL)
    mwerror(FATAL, 1, "Not enough memory for training set\n");

  printf("Size of training set 1 : %ld\n", sizet);

  if (result2) {
    result2 = mw_change_fimage(result2, sizet2, sizeb);
    if (result2 == NULL)
      mwerror(FATAL, 1, "Not enough memory for training set 2\n");
    printf("Size of training set 2 : %ld\n", sizet2);
  }
    
  if (result3) {
    result3 = mw_change_fimage(result3, sizet3, sizeb);
    if (result3 == NULL)
      mwerror(FATAL, 1, "Not enough memory for training set 3\n");
    printf("Size of training set 3 : %ld\n", sizet3);
  }

  if (result4) {
    result3 = mw_change_fimage(result4, sizet4, sizeb);
    if (result4 == NULL)
      mwerror(FATAL, 1, "Not enough memory for training set 4\n");
    printf("Size of training set 4 : %ld\n", sizet4);
  }


  ldxr = ldxr2 = ldxr3 = ldxr4 = t = 0;

  ADD_IMAGE_ADAP(width, height, &t, &ldxr, &ldxr2, &ldxr3, &ldxr4, lap, decim, edge, image, indts, result, result2, result3, result4);

  if (image2 != NULL)
    ADD_IMAGE_ADAP(width, height, &t, &ldxr, &ldxr2, &ldxr3, &ldxr4, lap, decim, edge, image2, indts, result, result2, result3, result4);

  if (image3 != NULL)
    ADD_IMAGE_ADAP(width, height, &t, &ldxr, &ldxr2, &ldxr3, &ldxr4, lap, decim, edge, image3, indts, result, result2, result3, result4);

  if (image4 != NULL) 
    ADD_IMAGE_ADAP(width, height, &t, &ldxr, &ldxr2, &ldxr3, &ldxr4, lap, decim, edge, image4, indts, result, result2, result3, result4);
    
  if (image5 != NULL) 
    ADD_IMAGE_ADAP(width, height, &t, &ldxr, &ldxr2, &ldxr3, &ldxr4, lap, decim, edge, image5, indts, result, result2, result3, result4);
    
  if (image6 != NULL) 
    ADD_IMAGE_ADAP(width, height, &t, &ldxr, &ldxr2, &ldxr3, &ldxr4, lap, decim, edge, image6, indts, result, result2, result3, result4);
    
  if (image7 != NULL) 
    ADD_IMAGE_ADAP(width, height, &t, &ldxr, &ldxr2, &ldxr3, &ldxr4, lap, decim, edge, image7, indts, result, result2, result3, result4);
    
  if (image8 != NULL) 
    ADD_IMAGE_ADAP(width, height, &t, &ldxr, &ldxr2, &ldxr3, &ldxr4, lap, decim, edge, image8, indts, result, result2, result3, result4);
    
  mw_delete_cimage(indts);
}



     
static void
MK_TRAINSET(width, height, lap, decim, edge, image, image2, image3, image4, image5, image6, image7, image8, result)

  /*--- Construct non adaptive training set ---*/

int             width, height;    /* Width and height of block */
int            *lap;              /* Take overlapping blocks 
				   * in training images */
int             decim;            /* Space between pixels in a vector */
int             edge;             /* No overlapping on edges */
Fimage          image, image2, image3, image4, image5, image6, image7, image8;
Fimage          result;


{
  long      sizeb;         /* Size of blocks */
  long      sizet;         /* Size of training set */
  long      dx, dy;        /* Dimensions of training images */
  long	    ldxr;          /* Index in training set of the first point 
			    * of the vector currently added */
  long      dum;

  sizeb = height * width;
  if (!lap)
    edge = 0;

  dx = SIZE_TS(image->nrow, height, lap, decim, edge);
  dy = SIZE_TS(image->ncol, width, lap, decim, edge);
  sizet = dx * dy;

  if (image2 != NULL) {
    dx = SIZE_TS(image2->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image2->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }
	
  if (image3 != NULL) {
    dx = SIZE_TS(image3->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image3->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }

  if (image4 != NULL) {
    dx = SIZE_TS(image4->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image4->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }
    
  if (image5 != NULL) {
    dx = SIZE_TS(image5->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image5->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }

  if (image6 != NULL) {
    dx = SIZE_TS(image6->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image6->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }
    
  if (image7 != NULL) {
    dx = SIZE_TS(image7->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image7->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }

  if (image8 != NULL) {
    dx = SIZE_TS(image8->nrow, height, lap, decim, edge);
    dy = SIZE_TS(image8->ncol, width, lap, decim, edge);
    sizet += dx * dy;
  }
    
  result = mw_change_fimage(result, sizet, sizeb);
  if (result == NULL)
    mwerror(FATAL, 1, "Not enough memory for training set\n");

  dum = ldxr = 0;
  ADD_IMAGE_ADAP(width, height, &dum, &ldxr, NULL, NULL, NULL, lap, decim, edge, image, NULL, result, NULL, NULL, NULL);

  if (image2 != NULL) 
    ADD_IMAGE_ADAP(width, height, &dum, &ldxr, NULL, NULL, NULL, lap, decim, edge, image2, NULL, result, NULL, NULL, NULL);
	
  if (image3 != NULL) 
    ADD_IMAGE_ADAP(width, height, &dum, &ldxr, NULL, NULL, NULL, lap, decim, edge, image3, NULL, result, NULL, NULL, NULL);
	
  if (image4 != NULL) 
    ADD_IMAGE_ADAP(width, height, &dum, &ldxr, NULL, NULL, NULL, lap, decim, edge, image4, NULL, result, NULL, NULL, NULL);
	
  if (image5 != NULL) 
    ADD_IMAGE_ADAP(width, height, &dum, &ldxr, NULL, NULL, NULL, lap, decim, edge, image5, NULL, result, NULL, NULL, NULL);
	
  if (image6 != NULL) 
    ADD_IMAGE_ADAP(width, height, &dum, &ldxr, NULL, NULL, NULL, lap, decim, edge, image6, NULL, result, NULL, NULL, NULL);
	
  if (image7 != NULL) 
    ADD_IMAGE_ADAP(width, height, &dum, &ldxr, NULL, NULL, NULL, lap, decim, edge, image7, NULL, result, NULL, NULL, NULL);
	
  if (image8 != NULL) 
    ADD_IMAGE_ADAP(width, height, &dum, &ldxr, NULL, NULL, NULL, lap, decim, edge, image8, NULL, result, NULL, NULL, NULL);
	

}





void
mk_trainset(Width, Height, Lap, Decim, Edge, ThresVal1, ThresVal2, ThresVal3, SizeCB, Image2, Image3, Image4, Image5, Image6, Image7, Image8, Result2, Result3, Result4, Image, Result)

int        *Width, *Height;		/* Dimensions of blocks */
int        *Lap;                        /* Flag for taking overlapping 
					 * blocks in training images */
int        *Decim;          /* Space between pixels in a vector */
int        *Edge;                       /* No overlapping on edges */
float	   *ThresVal1, *ThresVal2, *ThresVal3;	/* threshold values  
    					 * for block energy */
int        *SizeCB;                     /* Size of codebook */
Fimage      Image2, Image3, Image4, Image5, Image6, Image7, Image8;     
                                        /* Training images */
Fimage      Result2, Result3, Result4;  /* Resulting adapted codeooks */
Fimage      Image;                      /* Training image */
Fimage      Result;                     /* Resulting adapted codeook */

{

  if (ThresVal1)
    MK_TRAINSET_ADAP(*Width, *Height, Lap, *Decim, *Edge, ThresVal1, ThresVal2, ThresVal3, SizeCB, Image, Image2, Image3, Image4, Image5, Image6, Image7, Image8, Result, Result2, Result3, Result4);
  else
    MK_TRAINSET(*Width, *Height, Lap, *Decim, *Edge, Image, Image2, Image3, Image4, Image5, Image6, Image7, Image8, Result);

}

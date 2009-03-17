/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {flbg_train};
version = {"2.00"};
author = {"Jean-Pierre D'Ales"};
function = {"Generates a sequence of codebooks from a training set of vectors using LBG (generalized Lloyd) algorithm"};
usage = {
 's':[CodeBookSize=1]->Size          "Size of output codebook", 
 'W':Weight->Weight                  "Weighting factors for the components of vector (fsignal)",
 'M'->MultiCB                        "Generate a sequence of codebooks of size equal to a power of two and <= Size",
 'i':InitCodeBook->InitCodeBook      "Initial codebook (fimage)",
 'p'->PrintSNR                       "Print number of iterations for each loop instead of distortion rate results",
 'f':NResCB->NResCB                  "Index of first residual codebook (in ResCodeBook)",
 'g':NResResCB->NResResCB            "Index of second residual codebook (in ResResCodeBook)",
 'a':ResCodeBook->ResCodeBook        "First residual codebook (fimage)",
 'b':ResResCodeBook->ResResCodeBook  "Second residual codebook (fimage)",
 TrainSet->TrainSet                  "Training set of vectors (fimage)",
 MSE<-MSE                            "Quantization mean square error", 
 CodeBook<-CodeBook                  "Output codebook (fimage)"
};
*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

/*--- Constants ---*/

#define Min(x,y) ((x) >= (y) ? (y) : (x))
#define STOP_THRESHOLD 1e-1
#define MAXSIZE_SPLITBUF 5000000

/*--- Global variables ---*/ 

static double   dist;                /* Quantization m.s.e. for training set */
static int	oldsizec;            /* Size of codebook at former iteration */
static int	nitersplit;          /* Number of successive iterations 
				      * without increase of codebook size */
static long    *weightcell;          /* Number of tr. set vectors in codebook 
				      * cells */
static double   varts;               /* Variance of training set */
static double   raterec, entropyrec; /* Fixed and entropic rate */



static void
ENERGY_TS(Fimage trainset)

                                /* Input training set */

{
  register float *ptrts;
  long            t, size;
  double          meants;
  
  varts = meants = 0.0;
  size = trainset->nrow * trainset->ncol;
  
  for (t=0, ptrts = trainset->gray; t<size; t++, ptrts++)
    meants += *ptrts;
  meants /= (double) size;

  for (t=0, ptrts = trainset->gray; t<size; t++, ptrts++)
    varts += (*ptrts - meants) * (*ptrts - meants);
  varts /= (double) size;

}



static double
sqdist(Fimage trainset, Fimage codebook, Fsignal bweight, short int *cell)

                                 /* Input training set */
                                 /* Generated codebook */
                                 /* Coordinates weights in block */
                     

{
  int             t, c;
  long            ts;
  int             sizeb, sizet;
  double          error, e;

  sizeb = trainset->ncol;
  sizet = trainset->nrow;
  error = 0.0;
  ts = 0;
  for (t = 0; t < sizet; t++) {
    if (bweight)
	for (c = 0; c < sizeb; c++) {
	    e = codebook->gray[sizeb * cell[t] + c] - trainset->gray[ts + c];
	    error += bweight->values[c] * e * e;
	}
    else
	for (c = 0; c < sizeb; c++) {
	    e = codebook->gray[sizeb * cell[t] + c] - trainset->gray[ts + c];
	    error += e * e;
	}
	ts += sizeb;
    }

    return (error / (double) (sizet * sizeb));

}



static void
copy_multicb(Fimage codebook, int size)
{
  long            sizeb, sizec;  
  register float *ptrcb1, *ptrcb2;
  long            x;

  sizeb = codebook->ncol;
  sizec = (size + codebook->firstrow) * sizeb;
  ptrcb1 = codebook->gray;
  ptrcb2 = codebook->gray + (codebook->firstrow * sizeb);
  for (x = codebook->firstrow * sizeb; x < sizec; x++, ptrcb1++, ptrcb2++)
    *ptrcb2 = *ptrcb1;

  codebook->firstrow += size;
}



static void
extract_cb(Fimage codebook, int n)

  /*---extract codebook number n of the sequence of codebooks contained ---*/
          /*--- in 'codebook', put the result in 'codebook' ---*/

                    
             

{
  long      size, sizei, sizef;
  long     xshift;
  long     x, xi, xf;
  int      n1;
  
  sizei = floor(codebook->gray[(codebook->nrow - 5) * codebook->ncol]
		+ .5);
  sizef = floor(codebook->gray[(codebook->nrow - 6) * codebook->ncol]
		+ .5);

  if (((float) sizei == codebook->gray[(codebook->nrow - 5) * codebook->ncol]) || ((float) sizef == codebook->gray[(codebook->nrow - 6) * codebook->ncol])) {

    if (n == 0) {
      xi = sizef;
      xf = (sizef + sizei);
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

    xi *= codebook->ncol;
    xf *= codebook->ncol;
    for (x = xi; x < xf; x++)
      codebook->gray[x - xi] = codebook->gray[x];    

    xshift = (codebook->nrow - 4) * codebook->ncol - xf + xi;
    xi = xf - xi;
    xf = xi + 4 * codebook->ncol;
    for (x = xi; x < xf; x++)
      codebook->gray[x] = codebook->gray[x + xshift];
    codebook = mw_change_fimage(codebook, xf / codebook->ncol, codebook->ncol);
    if (codebook == NULL)
      mwerror(FATAL,1,"Not enough memory for extracted codebook.\n");
  } else
    mwerror(WARNING, 0, "ResCodeBook or ResResCodeBook is not a multiple codebook file!\n");
}


static void 
trainset_vq(Fimage trainset, Fimage codebook, Fsignal bweight, int *printsnr)

  /*--- Computation of residus of quantization of trainset vectors ---*/
  /*--- with codebook. The resulting vectors are stored in trainset ---*/

                                 /* Input training set and resulting training 
				  * set of residus after quantization 
				  * with codebook */
                                 /* Codebooks to be used for 
				  * quantization of training set */
                                 /* Coordinates weights in block */
                         

{
  long      t, t1, c, n, nc=0;
  long      tsizeb, tsizet1;
  long      nsizeb;
  int       sizet, sizeb, sizec;  
  float	    err, cerr, diff;
  float	    energy;
  float	    thressup, thresmin;
  double    rate;
  long      totaltest;
  long     *cweight;
    
  sizet = trainset->nrow;
  sizeb = trainset->ncol;
  sizec = codebook->nrow - 4;
  if (sizec <= 0) 
    mwerror(FATAL, 1, "Something wrong with residual codebook");

  /*--- Discard blocks of training set not in threshold range ---*/ 

  if ((codebook->gray[sizec * sizeb] != 0.0) || (codebook->gray[(sizec + 1) * sizeb] != 0.0)) {
    thressup = codebook->gray[sizec * sizeb];
    thresmin = codebook->gray[(sizec + 1) * sizeb];

    t1 = 0;
    tsizeb = tsizet1 = 0;
    for (t=0;t<sizet;t++) {
      energy = 0.0;
      if (bweight)
	for (c=0;c<sizeb;c++)
	  energy += bweight->values[c] * trainset->gray[tsizeb + c] * trainset->gray[tsizeb + c];
      else
	for (c=0;c<sizeb;c++)
	  energy += trainset->gray[tsizeb + c] * trainset->gray[tsizeb + c];
      if ((energy >= thresmin) && ((energy < thressup) || (thressup ==0.0))) {
	for (c=0;c<sizeb;c++)
	  trainset->gray[tsizet1 + c] = trainset->gray[tsizeb + c];
	t1++;
	tsizet1 += sizeb;
      }
      tsizeb += sizeb;
    }

    sizet = t1;
    if (sizet != trainset->nrow) {
      trainset = mw_change_fimage(trainset, sizet, sizeb);
      if (trainset == NULL)
        mwerror(FATAL, 1, "Reallocation of training set refused.\n");
    }

    if (printsnr)
      ENERGY_TS(trainset);
  }

  cweight = (long *) malloc(sizeof(long) * sizec);
  if (cweight == NULL)
    mwerror(FATAL, 1, "Not enough memory for weightcell buffer!\n");
  for (n = 0; n < sizec; n++)
    cweight[n] = 0;

  /*--- Quantize training set ---*/

  tsizeb = 0;
  for (t=0;t<sizet;t++){

    /*--- Search for nearest vector in codebook ---*/ 

    cerr = 1e30;
    nsizeb = 0;
    for (n=0;n<sizec;n++) {
      err = 0.0;
      if (bweight)
	for (c=0;c<sizeb;c++) {
	  diff = trainset->gray[tsizeb + c] - codebook->gray[nsizeb + c];
	  err += bweight->values[c] * diff * diff;
	} 
      else
	for (c=0;c<sizeb;c++) {
	  diff = trainset->gray[tsizeb + c] - codebook->gray[nsizeb + c];
	  err += diff * diff;
	} 

      if (err<cerr) {
	nc = n;
	cerr = err;
      }
      nsizeb += sizeb;
    }

    /*--- Quantize block ---*/

    nsizeb = nc * sizeb;
    cweight[nc]++;
    for (c=0;c<sizeb;c++)
      trainset->gray[tsizeb + c] -= codebook->gray[nsizeb + c];
    tsizeb += sizeb;
  }

  /*--- Computes rate ---*/

  if (printsnr) {
    rate = (double) sizet * log((double) sizet);
    totaltest = 0;
    for (c=0; c<sizec; c++) {
      if (cweight[c] > 0) 
	rate -= (double) cweight[c] * log((double) cweight[c]);
      totaltest += cweight[c];
    }
    if (totaltest != trainset->nrow) {
      mwerror(WARNING, 0, "Something wrong with weightcell buffer!\n");
      printf("Number of vector in tr.set : %d,  sum of weightcell : %ld\n",
	     sizet, totaltest);
    }
    rate /= (double) sizet * sizeb * log((double) 2.0);
    entropyrec += rate;
    raterec += log((double) sizec) / ((double) sizeb * log((double) 2.0));
  }

  free(cweight);
}



	
static void
split(Fimage trainset, Fimage codebook, Fsignal bweight, short int *cell, int *sizec, int sizecf, int *printsnr)

  /*--- Split all or some of the vectors in codebook into two vectors ---*/

                                 /* Training set of vectors */
                                 /* Generated (sequence of) codebook */
                                 /* Coordinates weights in block */
                                 /* Cell indices of tr. set vectors */
                      		 /* Actual number of vector in codebook */
                       	         /* Final dimension of codebook */
                         
					

{
  register float *ptrts, *ptrcb1, *ptrcb2, *ptrcb3;
  register short *ptris, *ptrcl;
  long            t, l, c, n, nn;
  long            ts, ls, lsn;
  int             sizev, sizet;	/* size of block and training set */
  int             nsplit;	/* Number of cells to be split */
  int             nsplitth;	/* Number of cells to be split theoretically */
  int		  nisplit;	/* Current number of splitted cells */
  long           *weight;	/* Number of training set elements  
				 * in a cell */
  long            maxweight;    /* Maximum of weight */
  double          s, d, s1, d1, diff;
  double          mdsp;
  float           minmd;        /* Mean square error with optimal splitting */
  float         **md;		/* Mean square deviation of tr. set
				 * elements in a cell along each 
				 * coordinate */
  float	         *mdsum;	/* Mean square deviation of tr. set
				 * elements in a cell */
  float        ***mds;          /* Mean square deviation with splitted vectors
				 * along a coordinate */
  long           *indvect;       /* Indices of vectors in a cell */
  short	         *indsplit;	/* Index of cells to split */
  short           mode, optmode; /* (optimal) mode of splitting vector */
  short           modec;
  int             usemode;      /* Flag for use of mode */

  sizev = codebook->ncol;
  sizet = trainset->nrow;

  /*--- Computation of number of cells to split ---*/

  if(*sizec == oldsizec)
    nitersplit++;
  else
    nitersplit = 0;

  oldsizec = *sizec;

  if (*sizec * 2 < sizecf)
    nsplit = *sizec;
  else
    nsplit = sizecf - *sizec;
  nsplitth = nsplit;
  
  if((nitersplit > 4) && (nsplit > 1))
    nsplit--;



  /*--- Memory allocation for mean square deviation buffers ---*/

  md = (float **) malloc(sizeof(float *) * *sizec);
  if (md == NULL)
    mwerror(FATAL, 1, "Not enough memory for md buffer.\n");
  for (l = 0; l < *sizec; l++) {
    md[l] = (float *) malloc(sizeof(float) * sizev);
    if (md[l] == NULL)
      mwerror(FATAL, 1, "Not enough memory for md buffer.\n");
  }
  mdsum = (float *) malloc(sizeof(float) * *sizec);
  if (mdsum == NULL)
    mwerror(FATAL, 1, "Not enough memory for mdsum buffer.\n");

  for (l = 0; l < *sizec; l++) {
    mdsum[l] = 0.0;
    for (c = 0; c < sizev; c++)
      md[l][c] = 0.0;
  }

  weight = (long *) malloc(sizeof(long) * *sizec);
  for (l = 0; l < *sizec; l++)
    weight[l] = 0;

  indsplit = (short *) malloc(sizeof(short) * nsplit);
  for (l = 0; l < nsplit; l++)
    indsplit[l] = 0;

  /*--- Computation of mean square deviation per cell & coordinate ---*/

  ptrts = trainset->gray;
  ptrcl = cell;
  for (t = 0; t < sizet; t++, ptrcl++) {
    ls = sizev * *ptrcl;
    ptrcb1 = codebook->gray + ls;
    for (c = 0; c < sizev; c++, ptrts++, ptrcb1++) {
      d = *ptrcb1 - *ptrts;
      md[*ptrcl][c] += d * d;
    }
    weight[*ptrcl]++;
  }

  /*--- Computation of mean square deviation per cell ---*/

  if (bweight)
    for (l = 0; l < *sizec; l++)
      for (c = 0; c < sizev; c++)
	mdsum[l] += bweight->values[c] * md[l][c];
  else
    for (l = 0; l < *sizec; l++)
      for (c = 0; c < sizev; c++)
	mdsum[l] += md[l][c];


  /*--- Computation of cell indices to be split ---*/

  if (nsplit < *sizec) {

    /*--- Cells with greater mean deviation are splitted ---*/

    /*--- Order the first nsplit cells ---*/

    nisplit = 1;
    indsplit[0] = 0;
    for (l = 1; l < nsplit; l++, nisplit++) {
      n = 0;
      while ((n < nisplit) && (mdsum[l] < mdsum[indsplit[n]]))
	n++;
      for(nn=nisplit;nn>n;nn--)
	indsplit[nn] = indsplit[nn-1];
      indsplit[n] = l;
    }

    /*--- Include the other cells ---*/

    for (l = nsplit; l < *sizec; l++) {
      n = 0;
      while ((n < nsplit) && (mdsum[l] < mdsum[indsplit[n]]))
	n++;
      for(nn=nsplit-1;nn>n;nn--)
	indsplit[nn] = indsplit[nn-1];
      if (n < nsplit)
	indsplit[n] = l;
    }

    if (nsplit == 1) {
      indsplit[0] = 0;
      for (l = 1; l < *sizec; l++) 
	if (mdsum[l] > mdsum[indsplit[0]])
	  indsplit[0] = l;
      if (nitersplit > 4) {
	indsplit[0] = 0;
	for (n = 1; n < *sizec; n++) 
	  if ((mdsum[n] > mdsum[indsplit[0]]) && (n != l))
	    indsplit[0] = n;
      }
    }

    l = nsplit - 1;
    while (weight[indsplit[l--]] <= 1)
      nsplit--;

  } else
    {
      /*--- All cells are splitted ---*/

      n = l = 0;
      while (l<nsplit) 
	if (weight[n] > 1) 
	  indsplit[l++] = n++;
	else
	  {
	    n++;
	    nsplit--;
	  }
    }

  if ((nsplit != nsplitth) && !printsnr)
    printf("nsplit th.: %d,  nsplit eff.: %d\n", nsplitth, nsplit);

  /*--- Splitting ---*/

  for(l=0;l<nsplit;l++)
    for (c = 0; c < sizev; c++) {
      md[indsplit[l]][c] /= (float) weight[indsplit[l]];
      s = md[indsplit[l]][c];
      md[indsplit[l]][c] = sqrt((double) s);
    }

    /*--- Compute the size of the biggest cell ---*/

  maxweight = weight[0];
  for(l=1;l<nsplit;l++)
    if (maxweight < weight[l])
      maxweight = weight[l];

    /*--- Memory allocation for optimal splitting buffers ---*/

  indvect = (long *) malloc(sizeof(long) * maxweight);
  if (indvect == NULL)
    mwerror(FATAL, 1, "Not enough memory for indvect buffer.\n");

  if ((maxweight + sizet) * sizev <= MAXSIZE_SPLITBUF) {
    usemode = 1;
    mds = (float ***) malloc((int) 2 * sizeof(float **));
    for (l = 0; l <= 1; l++) {
      mds[l] = (float **) malloc((int) maxweight * sizeof(float *));
      if (mds[l] == NULL)
	mwerror(FATAL, 1, "Not enough memory for m.s.d. buffer.\n");
      for (t = 0; t < maxweight; t++) {
	mds[l][t] = (float *) malloc(sizeof(float) * sizev);
	if (mds[l][t] == NULL)
	  mwerror(FATAL, 1, "Not enough memory for m.s.d. buffer.\n");
      }
    }
  } else
    usemode = 0;

  lsn = *sizec * sizev;
  ptrcb3 = codebook->gray + lsn;
  ptris = indsplit;
  for (l = 0; l < nsplit; l++, ptris++) {

    /*--- Splitting of cell number l ---*/

    ls = *ptris * sizev;
    ptrcb1 = codebook->gray + ls;

      /*--- Search the index of vectors belonging to the cell ---*/

    n = 0;
    ptrcl = cell;
    for (t = 0; t < sizet; t++, ptrcl++)
      if (*ptrcl == l) 
	indvect[n++] = t;
     
      /*--- Search optimal splitting ---*/

    if (usemode == 1) {

      /*--- With buffers ---*/

      for (t = 0; t < weight[l]; t++) {
	ts = sizev * indvect[t];
	ptrts = trainset->gray + ts;
	ptrcb2 = ptrcb1;
	if (bweight)
	  for (c = 0; c < sizev; c++, ptrts++, ptrcb2++) {
	    s = d = *ptrts - *ptrcb2;
	    s += md[*ptris][c];
	    mds[0][t][c] = bweight->values[c] * s * s;
	    d -= md[*ptris][c];
	    mds[1][t][c] = bweight->values[c] * d * d;
	  }
	else
	  for (c = 0; c < sizev; c++, ptrts++, ptrcb2++) {
	    s = d = *ptrts - *ptrcb2;
	    s += md[*ptris][c];
	    mds[0][t][c] = s * s;
	    d -= md[*ptris][c];
	    mds[1][t][c] = d * d;
	  }      
      }

      optmode = 0;
      minmd = 1e30;
      for (mode = 1; mode < sizev; mode++) {
	mdsp = 0.0;
	for (t = 0; t < weight[l]; t++) {
	  s = d = 0.0;
	  for (c = 0; c < sizev; c++) {
	    modec = ((mode * c) / sizev) % 2;
	    s += mds[modec][t][c];
	    d += mds[1 - modec][t][c];
	  }
	  mdsp += Min(s,d);
	}

	if (mdsp < minmd) {
	  minmd = mdsp;
	  optmode = mode;
	}
      }
    } else
      {

	/*--- Without buffer ---*/

	optmode = 0;
	minmd = 1e30;
	for (mode = 1; mode < sizev; mode++) {
	  mdsp = 0.0;
	  for (t = 0; t < weight[l]; t++) {
	    ptrcb2 = ptrcb1;
	    ts = sizev * indvect[t];
	    ptrts = trainset->gray + ts;
	    s = d = 0.0;
	    if (bweight)
	      for (c = 0; c < sizev; c++, ptrts++, ptrcb2++) {
	        s1 = d1 = *ptrts - *ptrcb2;
		modec = (((mode * c) / sizev) % 2) * 2 - 1;
		diff = (float) modec * md[*ptris][c];
		s1 += diff;
		d1 -= diff;
		s += bweight->values[c] * s1 * s1;
		d += bweight->values[c] * d1 * d1;
	      }
	    else
	      for (c = 0; c < sizev; c++, ptrts++, ptrcb2++) {
		s1 = d1 = *ptrts - *ptrcb2;
		modec = (((mode * c) / sizev) % 2) * 2 - 1;
		diff = (float) modec * md[*ptris][c];
		s1 += diff;
		d1 -= diff;
		s += s1 * s1;
		d += d1 * d1;
	      }
	    mdsp += Min(s,d);
	  }
	  if (mdsp < minmd) {
	    minmd = mdsp;
	    optmode = mode;
	  }
	}
      }

      /*--- Splitting ---*/

    ptrcb2 = ptrcb1;
    for (c = 0; c < sizev; c++, ptrcb2++, ptrcb3++) {
      modec = ((optmode * c) / sizev) % 2;
      if (modec == 1) {
	*ptrcb3 = *ptrcb2 + md[*ptris][c];
	*ptrcb2 -= md[*ptris][c];
      } else
	{
	  *ptrcb3 = *ptrcb2 - md[*ptris][c];
	  *ptrcb2 += md[*ptris][c];
	}
    }

    lsn += sizev;
  }

  *sizec += nsplit;

  if (usemode == 1) {
    for (l = 1; l >= 0; l--) {
      for (t = maxweight - 1; t >= 0 ; t--) 
	free(mds[l][t]);
      free(mds[l]);
    }
    free(mds);
  }
  free(indvect);
  free(indsplit);
  free(weight);
  free(mdsum);
  for (l = oldsizec - 1; l >= 0 ; l--) 
    free(md[l]);
  free(md);
}





static void
optivect(Fimage trainset, Fimage codebook, short int *cell, int *sizec)

        /*--- Optimize the coordinates of vectors in codebook ---*/
  /*--- Each vector in codebook is replaced by the center of gravity ---*/
                   /*--- of the corresponding cell ---*/

                                 /* Training set of vectors */
                                 /* Generated (sequence of) codebook */
                                 /* Cell indices of tr. set vectors */
                                 /* Current size of codebook */

{
  register float *ptrts, *ptrcb;
  register short *ptrcl;
  int             t, l, c, ld;
  int             ls, lds;
  int             sizev, sizecloc, sizet, size;

  sizecloc = *sizec;
  sizev = codebook->ncol;
  sizet = trainset->nrow;
  for (l = 0; l < sizecloc; l++)
    weightcell[l] = 0;

  /*--- Clear codebook ---*/

  size = sizecloc * sizev;
  for (c = 0, ptrcb = codebook->gray; c < size; c++, ptrcb++)
    *ptrcb = 0.0;

  /*--- Compute center of gravity ---*/

  ptrts = trainset->gray;
  ptrcl = cell;
  for (t = 0; t < sizet; t++, ptrcl++) {
    weightcell[*ptrcl]++;
    ls = sizev * *ptrcl;
    ptrcb = codebook->gray + ls;
    for (c = 0; c < sizev; c++, ptrts++, ptrcb++) 
      *ptrcb += *ptrts;
  }

  ls = 0;
  ptrcb = codebook->gray;
  for (l = 0; l < *sizec; l++) {
    if (weightcell[l] > 0) {
      for (c = 0; c < sizev; c++, ptrcb++)
	*ptrcb /= (float) weightcell[l];
      ls += sizev;
    } else
      {
	/*--- If the cell is empty, the corresponding vector is removed ---*/ 
	              /*--- from codebook ---*/

	lds = ls;
	for (ld = l + 1; ld < *sizec; ld++) {
	  weightcell[ld - 1] = weightcell[ld];
	  for (c = 0; c < sizev; c++)
	    codebook->gray[lds + c] = codebook->gray[lds + sizev + c];
	  lds += sizev;
	}
	for (t = 0, ptrcl = cell; t < sizet; t++, ptrcl++)
	  if (*ptrcl > l)
	    (*ptrcl)--;
	l--;
	(*sizec)--;
      }
  }

}



static void
opticell(Fimage trainset, Fimage codebook, Fsignal bweight, short int *cell, short int sizec)

  /*--- Optimize the cells of training vectors. Each vector ---*/
     /*--- in training set is put in the cell associated ---*/
           /*--- to the closest vector in codebook ---*/

                                 /* Training set of vectors */
                                 /* Generated (sequence of) codebook */
                                 /* Coordinates weights in block */
                                 /* Cell indices of tr. set vectors */
                                 /* Current size of codebook */

{
  register float *ptrts1, *ptrts2, *ptrcb1, *ptrcb2;
  register short *ptrcl;
  int             t, l, c;
  int             sizev, sizet;
  float           error, minerr, e;

  dist = 0.0;
  sizev = trainset->ncol;
  sizet = trainset->nrow;

  ptrts1 = trainset->gray;
  ptrcl = cell;
  for (t = 0; t < sizet; t++, ptrcl++) {
    minerr = 1e30;
    ptrcb1 = codebook->gray;

    /*--- Search in codebook the closest vector to training vector ---*/
                      /*--- at row t ---*/

    for (l = 0; l < sizec; l++) {
      ptrts2 = ptrts1;
      ptrcb2 = ptrcb1;
      error = 0.0;
      c = 0;
      if (bweight)
	while ((c < sizev) && (error < minerr)) {
	  e = *ptrcb2 - *ptrts2;
	  error += bweight->values[c] * e * e;
	  c++;
	  ptrts2++;
	  ptrcb2++;
	}
      else
	while ((c < sizev) && (error < minerr)) {
	  e = *ptrcb2 - *ptrts2;
	  error += e * e;
	  c++;
	  ptrts2++;
	  ptrcb2++;
	}
      if (error < minerr) {
	minerr = error;
	*ptrcl = l;
      }
      ptrcb1 += sizev; 
    }
    dist += minerr;
    ptrts1 += sizev; 
  }

  dist /= (double) sizet * sizev;

}





static void
optimize(Fimage trainset, Fimage codebook, Fsignal bweight, short int *cell, int *sizec, int sizecf, int *printsnr, int *multicb)

                                 /* Training set of vectors */
                                 /* Generated (sequence of) codebook */
                                 /* Coordinates weights in block */
                                 /* Cell indices of tr. set vectors */
                                 /* Current size of codebook */
                                 /* Final size of codebook */
                         
                                 /* Generate a sequence of codebooks */ 

{
  double    olddist;
  short	    niter;
  double    rate;
  int       l;
  long      totaltest;

  dist = sqdist(trainset, codebook, bweight, cell);
  niter = 0;

  do {
    olddist = dist;
    niter++;
    opticell(trainset, codebook, bweight, cell, *sizec);
    optivect(trainset, codebook, cell, sizec);
  } while (((olddist - dist) / dist > STOP_THRESHOLD) && (niter<10));

  if (printsnr) {
    rate = (double) trainset->nrow * log((double) trainset->nrow);
    totaltest = 0;
    for (l=0; l<*sizec; l++) {
      if (weightcell[l] > 0) 
	rate -= (double) weightcell[l] * log((double) weightcell[l]);
      totaltest += weightcell[l];
    }
    if (totaltest != trainset->nrow) {
      mwerror(WARNING, 0, "Something wrong with weightcell buffer!\n");
      printf("Number of vector in tr.set : %d,  sum of weightcell : %ld\n",
	     trainset->nrow, totaltest);
    }
    rate /= (double) trainset->nrow * trainset->ncol * log((double) 2.0);
    rate += entropyrec;
    if (!multicb || (*sizec == sizecf))
      printf("%.3f  %.3f  %.3f\n", raterec + log((double) *sizec) / ((double) trainset->ncol * log((double) 2.0)), rate, 10.0 * log10(varts / dist));
  } else
    printf("Size = %d,  Niter = %d\n", *sizec, niter);

}




void
flbg_train(int *Size, Fsignal Weight, int *MultiCB, Fimage InitCodeBook, int *NResCB, int *NResResCB, Fimage ResCodeBook, Fimage ResResCodeBook, int *PrintSNR, Fimage TrainSet, float *MSE, Fimage CodeBook)

                            /* Size of codebook */
                            /* Coordinates weights in block */
                            /* Generates all codebooks of size equal 
			     * to a power of 2 and less than *Size */
                            /* Initial codebook */
                                /* Size for residual codebooks */
                                           /* Codebooks to be used for 
			     * quantization of training set */
                            /* Print SNR and rate */
                            /* Input training set of vectors, one vector 
			     * per row */
     	                    /* Mean square error for training set */
                            /* Generated (sequence of) codebook, 
			     * one vector per row */


{
  int             sizev;               /* Size of vectors */
  int             sizet;               /* Size of training set */
  int             sizemcb, sizec;      /* Size of codebook */
  short          *cell;                /* Cell indices of tr. set vectors */
  long            t;                   /* Index of vector in training set */
  long            r;                   /* Index of vector in codebook */
  int             c;                   /* Index of component in vector */ 
  Fsignal         BWeight;	       /* Weights of coordinates in block */
  int             TestWeight;          /* Check if weighting of vector 
					* components */

  sizev = TrainSet->ncol;
  sizet = TrainSet->nrow;
  sizec = *Size;


  /*--- Check sizes of codebooks ---*/

  if (InitCodeBook) {
    if (InitCodeBook->nrow - 4 > *Size) {
      mwerror(WARNING, 0, "InitCodeBook too big!\n");
      InitCodeBook = NULL;
    }
    if (InitCodeBook->ncol != sizev) {
      mwerror(WARNING, 0, "Bad size of vectors in InitCodeBook!\n");
      InitCodeBook = NULL;
    }
  }

  if (ResCodeBook)
    if (ResCodeBook->ncol != sizev) {
      mwerror(WARNING, 0, "Bad size of vectors in ResCodeBook!\n");
      ResCodeBook = NULL;
    }

  if (ResResCodeBook)
    if (ResResCodeBook->ncol != sizev) {
      mwerror(WARNING, 0, "Bad size of vectors in ResResCodeBook!\n");
      ResResCodeBook = NULL;
    }

  /*--- Decide if block coordinates are weighted ---*/

  if (Weight)
    BWeight = Weight;
  else
    if (ResCodeBook) {
      TestWeight = 0;
      for (c=0; c<sizev; c++)
	if (ResCodeBook->gray[(ResCodeBook->nrow - 1) * sizev + c] != 1.0)
	  TestWeight++;

      if (TestWeight != 0) {
	BWeight = mw_new_fsignal();
	if (mw_alloc_fsignal(BWeight, sizev) == NULL)
	  mwerror(FATAL, 1, " Not enough memory for bweight buffer!\n");
	for (c = 0; c < sizev; c++) {
	  BWeight->values[c] = ResCodeBook->gray[(ResCodeBook->nrow - 1) * sizev + c];
	  if (BWeight->values[c] <= 0.0)
	    mwerror(FATAL, 2, "Bad value in rescodebook for weighting!\n");
	}
      } else
	BWeight = NULL;
    } else
      BWeight = NULL;

  /*--- Memory allocation for partition buffer and clusters'size buffer ---*/ 

  cell = (short *) malloc(sizeof(short) * sizet);
  if (cell == NULL)
    mwerror(FATAL, 1, "Not enough memory for cell buffer!\n");
  for (t = 0; t < sizet; t++)
    cell[t] = 0;

  weightcell = (long *) malloc(sizeof(long) * sizec);
  if (weightcell == NULL)
    mwerror(FATAL, 1, "Not enough memory for weightcell buffer!\n");
  for (r = 0; r < sizec; r++)
    weightcell[r] = 0;

  /*--- Quantization of codebook if residual quantization ---*/

  raterec = entropyrec = 0.0;

  if (PrintSNR) 
    ENERGY_TS(TrainSet); 
  
  if (ResCodeBook) {
    if (NResCB)
      extract_cb(ResCodeBook, *NResCB);
    trainset_vq(TrainSet, ResCodeBook, BWeight, PrintSNR);

    if (ResResCodeBook) {
      if (NResResCB)
	extract_cb(ResResCodeBook, *NResResCB);
      trainset_vq(TrainSet, ResResCodeBook, BWeight, PrintSNR);
    }
  }  

  if (sizec > sizet)
    mwerror(FATAL, 1, "Size of codebook is bigger than size of training set!\n");

  /*--- Memory allocation for CodeBook codebook buffer ---*/ 

  if (MultiCB) {
    sizemcb = 2;
    while (sizemcb < sizec)
      sizemcb *= 2;
    CodeBook = mw_change_fimage(CodeBook, sizec + sizemcb + 5, sizev);
    if (CodeBook == NULL)
      mwerror(FATAL, 1, "Not enough memory for CodeBook!\n");
    CodeBook->gray[(CodeBook->nrow - 6) * sizev] = *Size;
    for (c = 1; c < sizev; c++)
      CodeBook->gray[(CodeBook->nrow - 6) * sizev + c] = 0.0;
    if (InitCodeBook)
      CodeBook->gray[(CodeBook->nrow - 5) * sizev] = InitCodeBook->nrow - 4;
    else
      CodeBook->gray[(CodeBook->nrow - 5) * sizev] = 1.0;
    for (c = 1; c < sizev; c++)
      CodeBook->gray[(CodeBook->nrow - 5) * sizev + c] = 0.0;
    CodeBook->firstrow = sizec;
  } else
    {
      CodeBook = mw_change_fimage(CodeBook, sizec + 4, sizev);
      if (CodeBook == NULL)
	mwerror(FATAL, 1, "Not enough memory for CodeBook!\n");
    }

  for (c = 0; c < sizev; c++)
    CodeBook->gray[(CodeBook->nrow - 2) * sizev + c] = CodeBook->gray[(CodeBook->nrow - 3) * sizev + c] = CodeBook->gray[(CodeBook->nrow - 4) * sizev + c] = 0.0;

  if (BWeight)
    for (c = 0; c < sizev; c++)
      CodeBook->gray[(CodeBook->nrow - 1) * sizev + c] = BWeight->values[c];
  else
    if (sizev >= 2)
      for (c = 0; c < sizev; c++)
	CodeBook->gray[(CodeBook->nrow - 1) * sizev + c] = 1.0;

      /*--- Init outer loop ---*/

  printf("\n");
  
  if (InitCodeBook) {
    *Size = InitCodeBook->nrow - 4;
    for (r = 0; r < *Size; r++)
      for (c = 0; c < sizev; c++)
	CodeBook->gray[r * sizev + c] = InitCodeBook->gray[r * sizev + c];
    optimize(TrainSet, CodeBook, BWeight, cell, Size, *Size, PrintSNR, MultiCB);
  } else
    {
      *Size = 1;
      optivect(TrainSet, CodeBook, cell, Size);
    }

  if (MultiCB) {
    sizemcb = 2;
    if (InitCodeBook) 
      while (sizemcb <= InitCodeBook->nrow - 4)
	sizemcb *= 2;
  } else
    sizemcb = sizec;

      /*--- Outer loop : split and optimize with LBG algorithm ---*/

  while (sizemcb <= sizec) {

    if (MultiCB)
      copy_multicb(CodeBook, *Size);

    if (InitCodeBook) {
      *Size = InitCodeBook->nrow - 4;
      for (r = 0; r < *Size; r++)
	for (c = 0; c < sizev; c++)
	  CodeBook->gray[r * sizev + c] = InitCodeBook->gray[r * sizev + c];
      optimize(TrainSet, CodeBook, BWeight, cell, Size, sizemcb, PrintSNR, MultiCB);
    } else
      {
	*Size = 1;
	for (t = 0; t < sizet; t++)
	  cell[t] = 0;
	optivect(TrainSet, CodeBook, cell, Size);
      }
    
    nitersplit = 0;
    oldsizec = *Size;
    
    while (*Size < sizemcb) {
      split(TrainSet, CodeBook, BWeight, cell, Size, sizemcb, PrintSNR);
      
      optimize(TrainSet, CodeBook, BWeight, cell, Size, sizemcb, PrintSNR, MultiCB);
    }

    if ((sizemcb <= sizec / 2) || (sizemcb == sizec))
      sizemcb *= 2;
    else
      sizemcb = sizec;
  }

  free(weightcell);
  free(cell);    
  
  if (BWeight)
    dist = sqdist(TrainSet, CodeBook, NULL, cell);
  *MSE = dist;

}

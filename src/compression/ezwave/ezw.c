/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {ezw};
author = {"Jean-Pierre D'Ales"};
function = {"Wavelet transform compression via EZW algorithm"};
version = {"1.30"};
usage = {
'p'->PrintFull
	"Print full set of information", 
'r':NLevel->NumRec
	"Quantize wavelet transform up to level NLevel (Default : number of level in WavTrans)", 
'w':WeightFac->WeightFac
        "Scaling factor for wavelet coefficients", 
't':Threshold->Thres
	"Fix initial threshold", 
'm':[Max_Count_AC=256]->Max_Count_AC 
	"Number of counts for histogram in arithm. coding", 
'd'->DistRate
        "Computes distorsion-rate function", 
'R':TargetRate->Rate 
        "Target bit rate", 
'P':TargetPSNR->PSNR
	"Target PSNR",
's':SelectArea->SelectedArea
        "Polygonal regions to be encoded with a different rate or PSNR (polygons)",
'o':Compress<-Compress
          "Output compressed representation of WavTrans (cimage)",
WavTrans->Wtrans
	"Input wavelet transform (wtrans2d)", 
QWavTrans<-Output
	"Output quantized wavelet transform (wtrans2d)",
notused->PtrDRC
        "Distorsion rate curve"
	};
 */


/*--- Include files UNIX C ---*/

#include <stdio.h>
#include <math.h>
 

/*--- Megawave2 library ---*/

#include  "mw.h"

#define mw_drcurvesize 256

/* Distortion rate curves */

typedef struct drcurve {
  int nrow;                    /* Number of rows (dy) */
  int ncol;                    /* Number of columns (dx) */
  int nbitplanes;              /* Number of bit per pixel */
  int npoints;                 /* Number of points in the curve */
  float rate[mw_drcurvesize];  /* Bit rates */
  float cr[mw_drcurvesize];    /* Compression ratios */
  float mse[mw_drcurvesize];   /* Mean square error for correponding bit rate
                                * and compression ratio */
  
} DRCurve;

/*--- Megawave2 modules definition ---*/

#ifdef __STDC__
void fillpoly(int *, int *, Polygon, Cimage);
#else
void fillpoly();
#endif


/*--- Constants ---*/

#define  NBIT_GREYVAL 8     /* Number of bit per pixels */
#define  MAX_GREYVAL 255.0  /* Maximum of grey levels in input image */
#define  NBIT_NREC 4        /* Number of bits used to encode the number 
			     * of levels in wavelet transform */
#define  NCHANNEL_POLYG 1   /* Nuber of channels in polygons */
#define  NBIT_NPOLYG 8      /* Number of bits used to encode the number 
			     * of polygons */
#define  MAX_NPOLYG (1 << NBIT_NPOLYG)  /* Maximum number of polygons */
#define  NBIT_NPOINTS 8     /* Number of bits used to encode the number 
			     * of points in a polygon */
#define  MAX_NPOINTS (1 << NBIT_NPOINTS)  /* Maximum number of points 
			     * in a polygon */
#define  NBIT_SIZEIM 16     /* Number of bits used to encode the dimensions 
			     * of image */
#define  MAX_SIZEIM ((long) (1 << NBIT_SIZEIM) - 1)  /* Maximum value 
			     * for the dimensions of image */

#define  ZTI_SYMB 0         /* Symbol for coefficient inside a zerotree */
#define  ZTR_SYMB 1         /* Symbol for a zerotree root coefficient */
#define  ISO_SYMB 2         /* Symbol for an isolated zero coefficient */
#define  POS_SYMB 3         /* Symbol for a significant positive coefficient */
#define  NEG_SYMB 4         /* Symbol for a significant negative coefficient */
#define  SIG_SYMB 5         /* Symbol for an already significant coefficient */

#define  BG_SYMB 0          /* Symbol for background in areamap */
#define  WHITE_SYMB 255     /* Symbol for background in bitmap */

#define  ENDED 0            /* Flag for areas whose encoding 
			     * has been stopped */
#define  PROGRESS 1         /* Flag for areas whose encoding is in progress */
#define  END_NEXT 2         /* Flag for areas whose encoding has to be 
			     * stopped for next symbol */

/*--- Constants for arithmetic encoding ---*/

#define  MAX_SIZEO 32383
#define  code_value_bits 16
#define  top_value (((long) 1 << code_value_bits) - 1)
#define  first_qrt (top_value / 4 + 1)
#define  half (2 * first_qrt)
#define  third_qrt (3 * first_qrt)
#define  nbit_thressymb 16
#define  max_thressymb (((long) 1 << nbit_thressymb) - 1)

/*--- Global variables ---*/

static int    max_freq;
static long   low, high;          /* Interval extremities 
                                   * for arithmetic coding */
static long   bits_to_follow;
static int    bits_to_go;         /* Number of free bits in the currently 
		  	 	   * encoded codeword */
static int    buffer;             /* Bits to encode in the currently 
				   * encoded codeword */
static int    sizec;              /* Size of Compress */
static long   ncodewords;

static int    stop_test;          /* test flag for global stopping 
				   * of encoding */
static int    stop_test_area[MAX_NPOLYG]; /* test flag for stopping of encoding
				   * in a given area */

static long   targnbit_dr[20];
static int    count_dr;
static int    max_count_dr;
static DRCurve  *drc;             /* Distorsion rate curve */

static long   area[MAX_NPOLYG];   /* Areas of different regions */
static long   targnbit_sa[MAX_NPOLYG]; /* Target number of bits for each 
			           * selcected area */
static long   effnbit_sa[MAX_NPOLYG]; /* Effective current number of bits used 
				   * for each selcected area */
static float  targmse_sa[MAX_NPOLYG];  /* Target mse for each selcected area */
static double effmse_sa[MAX_NPOLYG];   /* Effective current mse for each 
			           * selcected area */
static float  targnbit;           /* Target number of bits for background */
static float  effnbit;            /* Effective current number of bits 
				   * for background */
static float  targmse;            /* Target mse for background */
static double effmse;             /* Effective current mse for background */

static int    npolyg;             /* Number of selected polygonal areas */
static int    maxpoint_polyg;     /* Maximum number of points in a polygon */

static unsigned char   *ptrc;     /* Pointer to compress->gray for next 
				   * codeword */



static void
SCALE_WAVT(wtrans, nrec, weightfac)

Wtrans2d      wtrans;
int           nrec;
float         weightfac;

{
  long   x, size;
  float  scalefac;
  int    i,j;

  scalefac = 1.0;
  for (j = 2; j <= nrec; j++) {
    scalefac *= weightfac;
    for (i = 1; i <= 3; i++) {
      size = wtrans->images[j][i]->nrow * wtrans->images[j][i]->ncol;
      for (x = 0; x < size; x++)
	wtrans->images[j][i]->gray[x] *= scalefac;
    }
  }
  size = wtrans->images[nrec][0]->nrow * wtrans->images[nrec][0]->ncol;
      for (x = 0; x < size; x++)
	wtrans->images[nrec][0]->gray[x] *= scalefac;

}




static void
INIT_TARGNBIT_DR(wtrans, ptrdrc)

Wtrans2d      wtrans;
char         *ptrdrc;

{
  long   total_nbit;

  total_nbit = (long) NBIT_GREYVAL * wtrans->ncol * wtrans->nrow;

  targnbit_dr[0] = total_nbit / 1000;
  targnbit_dr[1] = total_nbit / 500;
  targnbit_dr[2] = total_nbit / 250;
  targnbit_dr[3] = total_nbit / 125;
  targnbit_dr[4] = total_nbit / 100;
  targnbit_dr[5] = total_nbit / 80;
  targnbit_dr[6] = total_nbit / 60;
  targnbit_dr[7] = total_nbit / 50;
  targnbit_dr[8] = total_nbit / 40;
  targnbit_dr[9] = total_nbit / 30;
  targnbit_dr[10] = total_nbit / 20;
  targnbit_dr[11] = total_nbit / 15;
  targnbit_dr[12] = total_nbit / 10;
  targnbit_dr[13] = total_nbit / 5;

  max_count_dr = 13;
  count_dr = 0;

  if (ptrdrc) {
    drc = (DRCurve *) ptrdrc;
    drc->npoints = max_count_dr + 1;
    drc->nbitplanes = NBIT_GREYVAL;
    drc->nrow = wtrans->nrow;
    drc->ncol = wtrans->ncol;

    drc->cr[0] = 1000.0;
    drc->cr[1] = 500.0;
    drc->cr[2] = 250.0;
    drc->cr[3] = 125.0;
    drc->cr[4] = 100.0;
    drc->cr[5] = 80.0;
    drc->cr[6] = 60.0;
    drc->cr[7] = 50.0;
    drc->cr[8] = 40.0;
    drc->cr[9] = 30.0;
    drc->cr[10] = 20.0;
    drc->cr[11] = 15.0;
    drc->cr[12] = 10.0;
    drc->cr[13] = 5.0;
  } else
  drc = NULL;

}



static void
COMPUTE_EFF_TARGET_RATE_MSE(wtrans, output, nrec, areamap, selectarea, psnr, rate)

Wtrans2d         wtrans, output; /* Input and quantized wavelet transforms */
int              nrec;           /* Number of level to consider in wavelet 
				  * transform */
unsigned char ***areamap;        /* Mask for selected areas */
Polygons         selectarea;     /* Polygnal regions to be encoded with 
				  * a special rate or PSNR */
float           *rate;           /* Target bit rate for background */
float           *psnr;           /* Target psnr for background */

{
  int              i;                /* Index for orientation in wav. trans. */
  int              j;                /* Index for level in wav. trans. */
  register float  *ptrw, *ptro;      /* Pointers to wavelet coeff. in 
				      * wtrans and output */
  register unsigned char  *ptra;     /* Pointer to area mask */
  long             x;                /* Buffer index for current coefficient */
  long             size;             /* Size of subimages */
  int              p;                /* Index for selected area */
  Polygon          ptr_polyg;        /* Pointer to the current polygon */

  /*--- Compute area of various regions ---*/

  for (p = 0; p <= npolyg; p++)
    area[p] = 0;

  size = wtrans->images[1][1]->nrow * wtrans->images[1][1]->ncol;
  ptra = areamap[1][1];
  for (x = 0; x < size; x++, ptra++) 
    area[*ptra]++;

  for (p = 0; p <= npolyg; p++)
    area[p] *= 4;

  size = wtrans->nrow * wtrans->ncol;

  /*--- Compute target MSE for all selected areas ---*/

  if (psnr) {
    targmse = (double) size * MAX_GREYVAL * MAX_GREYVAL / pow((double) 10.0, (double) *psnr / 10.0);
    targmse_sa[0] = (double) area[0] * MAX_GREYVAL * MAX_GREYVAL / pow((double) 10.0, (double) *psnr / 10.0);

    if (selectarea) {
      ptr_polyg = selectarea->first;
      p = 1;
      while (ptr_polyg) {
	targmse_sa[p] = (double) area[p] * MAX_GREYVAL * MAX_GREYVAL / pow((double) 10.0, (double) ptr_polyg->channel[0] / 10.0);    
	p++;
	ptr_polyg = ptr_polyg->next;
      }
    }
  } else
    {
      targmse = -1.0;
      for (p = 0; p <= npolyg; p++)
	targmse_sa[p] = - 1.0;    
    }

  /*--- Compute target number of bits for all selected areas ---*/

  if (rate) {
    targnbit = (long) (*rate * (double) size);
    targnbit_sa[0] = (long) (*rate * (double) area[0]);

    if (selectarea) {
      ptr_polyg = selectarea->first;
      p = 1;
      while (ptr_polyg) {
	targnbit_sa[p] = (long) (ptr_polyg->channel[0] * (double) area[p]);    
	p++;
	ptr_polyg = ptr_polyg->next;
      }
    }
  } else
    {
      targnbit = NBIT_GREYVAL * size;
      for (p = 0; p <= npolyg; p++)
	targnbit_sa[p] = NBIT_GREYVAL * area[p];

    }

  /*--- Compute initial effective MSE and init output ---*/

  for (p = 0; p <= npolyg; p++)
    effnbit_sa[p] = 0.0;

  output->images[nrec][0] = mw_change_fimage(output->images[nrec][0], wtrans->images[nrec][0]->nrow, wtrans->images[nrec][0]->ncol);
  if (!output->images[nrec][0])
    mwerror(FATAL, 1, "Reallocation for output wavelet transform refused!\n");
  size = wtrans->images[nrec][0]->nrow * wtrans->images[nrec][0]->ncol;
  ptro = output->images[nrec][0]->gray;
  ptrw = wtrans->images[nrec][0]->gray;
  ptra = areamap[nrec][0];
  for (x = 0; x < size; x++, ptro++, ptrw++, ptra++) {
    *ptro = 0.0;
    effmse_sa[*ptra] += *ptrw * *ptrw;
  }
  
  for (j = nrec; j >= 1; j--)
    for (i=1; i<=3; i++) {
      output->images[j][i] = mw_change_fimage(output->images[j][i], wtrans->images[j][i]->nrow, wtrans->images[j][i]->ncol);
      if (!output->images[j][i])
	mwerror(FATAL, 1, "Reallocation for output wavelet transform refused!\n");
      size = wtrans->images[j][i]->nrow * wtrans->images[j][i]->ncol;
      ptro = output->images[j][i]->gray;
      ptrw = wtrans->images[j][i]->gray;
      ptra = areamap[j][i];
      for (x = 0; x < size; x++, ptro++, ptrw++, ptra++) {
	*ptro = 0.0;
	effmse_sa[*ptra] += *ptrw * *ptrw;
      }
    } 

  effmse = 0.0;
  for (p = 0; p <= npolyg; p++)
    effmse += effmse_sa[p];
  if (effmse <= targmse)
    stop_test = FALSE;

  /*--- Compute initial effective number of bits ---*/ 

  if (npolyg == 0) 
    effnbit_sa[0] = effnbit;
  else
    effnbit_sa[0] = 0;

  for (p = 1; p <= npolyg; p++)
    effnbit_sa[p] = 0;

}



static void
COMPUTE_TOTAL_MSE()

{
  int              p;                /* Index for selected area */

  effmse = 0.0;
  for (p = 0; p <= npolyg; p++)
    effmse += effmse_sa[p];
}




static void
INIT_SIGMAP(wtrans, nrec, sigmap)

Wtrans2d         wtrans;         /* Input wavelet transforms */
int              nrec;           /* Number of level to consider in wavelet 
				  * transform */
unsigned char ***sigmap;         /* Map of significance information 
				  * and mask for selected areas */

{
  int              i;                /* Index for orientation in wav. trans. */
  int              j;                /* Index for level in wav. trans. */
  register unsigned char  *ptrs;     /* Pointer to significance map */
  long             x;                /* Buffer index for current coefficient */
  long             size;             /* Size of subimages */

  for (j = 1; j <= nrec; j++) {
    sigmap[j] = (unsigned char **) malloc((int) 4 * sizeof(unsigned char *));
    if (sigmap[j] == NULL)
      mwerror(FATAL, 1, "Allocation for significance map refused!\n");
  }

  for (j = 1; j <= nrec; j++) {
    for (i = 1; i <= 3; i++) {
      size = wtrans->images[j][i]->nrow * wtrans->images[j][i]->ncol;
      sigmap[j][i] = (unsigned char *) malloc((int) size * sizeof(unsigned char));
      if (sigmap[j][i] == NULL)
	mwerror(FATAL, 1, "Allocation for significance map refused!\n");
      for (x = 0, ptrs = sigmap[j][i]; x < size; x++, ptrs++) 
	*ptrs = ZTI_SYMB;      
    }
  }

  size = wtrans->images[nrec][0]->nrow * wtrans->images[nrec][0]->ncol;
  sigmap[nrec][0] = (unsigned char *) malloc(size * sizeof(unsigned char));
  if (sigmap[nrec][0] == NULL)
    mwerror(FATAL, 1, "Allocation for significance map refused!\n");
  for (x = 0, ptrs = sigmap[nrec][0]; x < size; x++, ptrs++) 
    *ptrs = ZTR_SYMB;      
}



static void
UPDATE_AREAMAP(wtrans, nrec, areamap, bitmap, p)

Wtrans2d         wtrans;         /* Input wavelet transform */
int              nrec;           /* Number of level to consider in wavelet 
				  * transform */
unsigned char ***areamap;        /* Mask for selected areas */
Cimage           bitmap;         /* bitmap image */
unsigned char    p;              /* Index of polygon */

{
  int              i;                /* Index for orientation in wav. trans. */
  int              j;                /* Index for level in wav. trans. */
  register unsigned char  *ptra1, *ptra2, *ptra3;  /* Pointers to significance 
				      * map for each orientation */
  register unsigned char  *ptrb;     /* Pointer to bitmap */
  int              r, c;             /* Row and column indices for current 
				      * point in bitmap */
  int              ra, ca;           /* Row and column indices for current 
				      * point in areamap */
  int              fac;              /* Factor between sizes of bitmap 
				      * and subimage */
  long             x;                /* Buffer index for current coefficient 
				      * and its parent */
  long             size;             /* Size of subimages */

  fac = 0;
  for (j = 1; j <= nrec; j++) {
    fac++;
    for (i = 1; i <= 3; i++) {
      ptrb = bitmap->gray;
      for (r = 0; r < bitmap->nrow; r++) {
	ra = r >> fac;
	if (ra < wtrans->images[j][i]->nrow)
	  for (c = 0; c < bitmap->ncol; c++, ptrb++) {
	    if (*ptrb != WHITE_SYMB) {
	      ca = c >> fac;
	      x = ra * wtrans->images[j][i]->ncol + ca;
	      if (ca < wtrans->images[j][i]->ncol)
		areamap[j][i][x] = p;
	    }
	  }
      }
    }
  }

  ptrb = bitmap->gray;
  for (r = 0; r < bitmap->nrow; r++) {
    ra = r >> fac;
    if (ra < wtrans->images[nrec][0]->nrow)
      for (c = 0; c < bitmap->ncol; c++, ptrb++) {
	if (*ptrb != WHITE_SYMB) {
	  ca = c >> fac;
	  x = ra * wtrans->images[nrec][0]->ncol + ca;
	  if (ca < wtrans->images[nrec][0]->ncol)
	    areamap[nrec][0][x] = p;
	}
      }
  }
}



static void
INIT_AREAMAP(wtrans, nrec, areamap, selectarea)

Wtrans2d         wtrans;         /* Input wavelet transforms */
int              nrec;           /* Number of level to consider in wavelet 
				  * transform */
unsigned char ***areamap;        /* Mask for selected areas */
Polygons         selectarea;     /* Polygnal regions to be encoded with 
				  * a special rate or PSNR */

{
  int              i;                /* Index for orientation in wav. trans. */
  int              j;                /* Index for level in wav. trans. */
  register unsigned char  *ptra;     /* Pointer to areamap */
  Polygon          ptr_polyg;        /* Pointer to the current polygon */
  unsigned char    p;                /* Index of polygon */
  long             x;                /* Buffer index for current coefficient */
  long             size;             /* Size of subimages */
  Cimage           bitmap;           /* bitmap image */
  int              dx, dy;           /* Size of image */

  /*--- Memory allocation ---*/

  for (j = 1; j <= nrec; j++) {
    areamap[j] = (unsigned char **) malloc((int) 4 * sizeof(unsigned char *));
    if (areamap[j] == NULL)
      mwerror(FATAL, 1, "Allocation for significance map refused!\n");
  }

  for (j = 1; j <= nrec; j++) {
    for (i = 1; i <= 3; i++) {
      size = wtrans->images[j][i]->nrow * wtrans->images[j][i]->ncol;
      areamap[j][i] = (unsigned char *) malloc((int) size * sizeof(unsigned char));
      if (areamap[j][i] == NULL)
	mwerror(FATAL, 1, "Allocation for selected area mask refused!\n");
      for (x = 0, ptra = areamap[j][i]; x < size; x++, ptra++) 
	*ptra = BG_SYMB;      
    }
  }

  /*--- Initialisation ---*/

  size = wtrans->images[nrec][0]->nrow * wtrans->images[nrec][0]->ncol;
  areamap[nrec][0] = (unsigned char *) malloc(size * sizeof(unsigned char));
  if (areamap[nrec][0] == NULL)
    mwerror(FATAL, 1, "Allocation for selected area mask refused!\n");
  for (x = 0, ptra = areamap[nrec][0]; x < size; x++, ptra++) 
    *ptra = BG_SYMB;      

  if (selectarea) {
    bitmap = mw_new_cimage();
    dy = wtrans->nrow;
    dx = wtrans->ncol;

    ptr_polyg = selectarea->first;
    p = 1;
    while (ptr_polyg) {

      fillpoly(&dx, &dy, ptr_polyg, bitmap);

      UPDATE_AREAMAP(wtrans, nrec, areamap, bitmap, p);

      ptr_polyg = ptr_polyg->next;
      p++;
    }

    mw_delete_cimage(bitmap);
  }
}





static float
COMPUTES_THRES_ADAP(wtrans, nrec)

Wtrans2d     wtrans;             /* Input wavelet transforms */
int          nrec;               /* Number of level to consider in wavelet 
				  * transform */

{
  int             i,j;
  float           max;
  register float *ptrd;
  float           avcoeff;
  long            x, size;

  max = 0.0;
  size = wtrans->images[nrec][0]->nrow * wtrans->images[nrec][0]->ncol;
  for (x = 0, ptrd = wtrans->images[nrec][0]->gray; x < size; x++, ptrd++) {
    avcoeff = fabs((double) *ptrd);
    if (avcoeff > max)
      max = avcoeff;
  }

  for (j = nrec; j >= 1; j--)
    for (i=1; i<=3; i++) {
      size = wtrans->images[j][i]->nrow * wtrans->images[j][i]->ncol;
      for (x = 0, ptrd = wtrans->images[j][i]->gray; x < size; x++, ptrd++) {
	avcoeff = fabs((double) *ptrd);
	if (avcoeff > max)
	  max = avcoeff;
      }
    } 

  return(max / 2.0);
}



static void
TEST_END_ENCODING() 

{
  int p;
  
  stop_test = FALSE;
  for (p = 0; p <= npolyg; p++) 
    if (stop_test_area[p] == PROGRESS)
      stop_test = TRUE;

}



static void
REDRAW_AREAMAP(wtrans, output, nrec, areamap, p)

/*--- Modify areamap after encoding is stopped in some selected area ---*/

Wtrans2d         wtrans, output;     /* Input and output wavelet transforms */
int              nrec;
unsigned char ***areamap;            /* Mask for selected areas */
unsigned char    p;                  /* Index of area to be redrawn */

{
  int              i;                /* Index for orientation in wav. trans. */
  int              j, jp;            /* Index for level in wav. trans. */
  register unsigned char  *ptra;     /* Pointer to area mask */
  register float * ptrw, *ptro;      /* Pointers to wavelet coeff. in 
				      * wtrans and output */
  int              rp, cp;           /* Row and column indices for parent */
  long             x, xp;            /* Buffer index for current coefficient 
				      * and its parent */
  long             size;             /* Size of subimages */
  long             ncol;             /* Number of columns in subimages */

  /*--- Modify areamap ---*/

  for (i=1; i<=3; i++) {
    size = wtrans->images[1][i]->nrow * wtrans->images[1][i]->ncol;
    ptra = areamap[1][i];
    for (x = 0; x < size; x++, ptra++) 
      if (stop_test_area[*ptra] == PROGRESS) {


	/*--- Check parents for area index ---*/

	ncol = wtrans->images[1][i]->ncol;	
	xp = x;
	for (jp = 2; jp <= nrec; jp++) {

	  /*--- Compute column and row index of parent ---*/

	  rp = xp / (2 * ncol);
	  if (rp >= wtrans->images[jp][i]->nrow) {
	    if ((wtrans->images[jp - 1][i]->nrow % 2 == 0) || (rp != wtrans->images[jp][i]->nrow))
	      mwerror(WARNING, 0, "rp too large in REDRAW_AREAMAP\njp = %d, i = %d, rp = %d, nrow = %d, xp = %d\n", jp, i, rp, wtrans->images[jp][i]->nrow, xp);
	    rp--;
	  }
	  cp = (xp % ncol) / 2;
	  if (cp >= wtrans->images[jp][i]->ncol) {
	    if ((wtrans->images[jp - 1][i]->ncol % 2 == 0) || (cp != wtrans->images[jp][i]->ncol))
	      mwerror(WARNING, 0, "cp too large in REDRAW_AREAMAP\njp = %d, i = %d, cp = %d, ncol = %d, xp = %d\n", jp, i, cp, wtrans->images[jp][i]->ncol, xp);
	    cp--;
	  }

	  xp = rp * wtrans->images[jp][i]->ncol + cp;
	  if (xp >= wtrans->images[jp][i]->ncol * wtrans->images[jp][i]->nrow)
	    mwerror(WARNING, 0, "xp too large in REDRAW_AREAMAP\njp = %d, i = %d, rp = %d, cp = %d, xp = %d, size = %d\n", jp, i, rp, cp, xp, size);

	  if (areamap[jp][i][xp] == p)
	    areamap[jp][i][xp] = *ptra;
	  ncol = wtrans->images[jp][i]->ncol;
	}

	if (xp >= wtrans->images[nrec][0]->ncol * wtrans->images[nrec][0]->nrow)
	  mwerror(WARNING, 0, "xp too large in REDRAW_AREAMAP\n, jp = %d, i = %d, rp = %d, cp = %d, xp = %d, size = %d\n", jp, i, rp, cp, xp, size);
	  
	if (areamap[nrec][0][xp] == p)
	  areamap[nrec][0][xp] = *ptra;
	 
      } 
  }

  /*--- Recompute effective MSE ---*/

  for (p = 0; p <= npolyg; p++)
    effnbit_sa[p] = 0.0;

  size = wtrans->images[nrec][0]->nrow * wtrans->images[nrec][0]->ncol;
  ptro = output->images[nrec][0]->gray;
  ptrw = wtrans->images[nrec][0]->gray;
  ptra = areamap[nrec][0];
  for (x = 0; x < size; x++, ptro++, ptrw++, ptra++) 
    effmse_sa[*ptra] += (*ptrw - *ptro) * (*ptrw - *ptro);
  
  
  for (j = nrec; j >= 1; j--)
    for (i=1; i<=3; i++) {
      size = wtrans->images[j][i]->nrow * wtrans->images[j][i]->ncol;
      ptro = output->images[j][i]->gray;
      ptrw = wtrans->images[j][i]->gray;
      ptra = areamap[j][i];
      for (x = 0; x < size; x++, ptro++, ptrw++, ptra++) 
	effmse_sa[*ptra] += (*ptrw - *ptro) * (*ptrw - *ptro);
    } 

  effmse = 0.0;
  for (p = 0; p <= npolyg; p++)
    effmse += effmse_sa[p];

}
      
 

static void
RESIZE_COMPRESS(compress)

Cimage           compress;

{
  int              i;
  int              ncolo, nrowo;


  ncolo = 1;
  nrowo = ncodewords;
  while (nrowo > MAX_SIZEO) {
    i = 2;
    while ((nrowo % i != 0) && (i < nrowo / 4))
      i++;
    if (i < nrowo / 4) {
      nrowo /= i;
      ncolo *= i;
    } else
      {
	ncolo = ncodewords / MAX_SIZEO + 1;
	nrowo = ncodewords / ncolo + 1;
	sizec = nrowo * ncolo;
	ptrc++; 
	for (i = ncodewords; i < sizec; i++, ptrc++) 
	  *ptrc = 0;
      }
  }

  compress = mw_change_cimage(compress, nrowo, ncolo);
  compress->firstrow = 8 - bits_to_go;
  sizec = nrowo * ncolo;
 

}



static void
REALLOCATE_COMPRESS(compress)

Cimage           compress;

{
  int              i;
  Cimage           bufcomp;

  printf("Reallocation of Compress.\n");

  bufcomp = mw_new_cimage();
  bufcomp = mw_change_cimage(bufcomp, compress->nrow,  compress->ncol);
  if (bufcomp == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for reallocation buffer!\n");
  for (i = 0; i < sizec; i++)
    bufcomp->gray[i] = compress->gray[i];

  compress = mw_change_cimage(compress, compress->nrow * 2,  compress->ncol);
  if (compress == NULL)
    mwerror(FATAL, 1, "Memory allocation refused for reallocated compress!\n");

  for (i = 0; i < sizec; i++) {
    compress->gray[i] = bufcomp->gray[i];
    compress->gray[i+sizec] = 0;
  }

  ptrc = compress->gray;
  for (i = 1; i < sizec; i++) 
    ptrc++;

  sizec = compress->ncol * compress->nrow;
  mw_delete_cimage(bufcomp);
}


static void
ADD_BIT_TO_OUTPUT(bit, compress)

int              bit;
Cimage           compress;

{
  buffer >>= 1;
  if (bit) 
    buffer += 128;
  bits_to_go -= 1;
  if (bits_to_go == 0) {
    *ptrc = buffer;
    if (ncodewords == sizec)
      REALLOCATE_COMPRESS(compress);
    ptrc++;
    bits_to_go = 8;
    ncodewords += 1;
    buffer = 0;
  }
}



static void
ENCODE_INT(symb, max, compress)

int              symb, max;
Cimage           compress;

{

  while (max > 0) {
    if (symb >= max) {
      ADD_BIT_TO_OUTPUT(1, compress);
      symb = symb % max;
    } else
      ADD_BIT_TO_OUTPUT(0, compress);
    max /= 2;
  }

}


static void
INIT_ENCODING(nrec, nrow, ncol, thressymb, selectarea, compress)

int               nrec;          /* Number of levels in wav. trans. */
int               nrow, ncol;    /* Size of original image */
int               thressymb;     /* Symbol for initial threshold value */
Polygons          selectarea;    /* Polygnal regions to be encoded with 
				  * a special rate or PSNR */
Cimage            compress;      /* Output compressed file */

{
  Polygon       ptr_polyg;       /* Pointer to the current polygon */
  Point_curve   ptr_point;       /* Pointer to the current point in current 
				  * polygon */
  int           p;
  int           npoints;

  bits_to_follow = 0;
  if (compress) {
    ptrc = compress->gray;
    bits_to_go = 8;
    buffer = 0;
    ncodewords = 1;
    ENCODE_INT(nrec, 1 << (NBIT_NREC - 1), compress);
    ENCODE_INT(nrow, 1 << (NBIT_SIZEIM - 1), compress);
    ENCODE_INT(ncol, 1 << (NBIT_SIZEIM - 1), compress);
    ENCODE_INT(thressymb, 1 << (nbit_thressymb - 1), compress);
  } else
    ptrc = NULL;

  effnbit = NBIT_NREC + 2 * NBIT_SIZEIM + nbit_thressymb;

  npolyg = 0;
  if (selectarea) {

    /*--- Count number of polygons ---*/ 

    ptr_polyg = selectarea->first;
    while (ptr_polyg) {
      npolyg++;
      ptr_polyg = ptr_polyg->next;
    }
    if (npolyg > MAX_NPOLYG)
      mwerror(FATAL, 2, "Too many selected areas!\n");
  }

  if (ptrc)
    ENCODE_INT(npolyg, 1 << (NBIT_NPOLYG - 1), compress);
  effnbit += NBIT_NPOLYG;

  if (npolyg > 0) {

    /*--- Encode vertices of polygons ---*/ 

    maxpoint_polyg = 0;
    ptr_polyg = selectarea->first;
    while (ptr_polyg) {

      /*--- Count number of vertices in polygon ---*/

      npoints = 0;
      ptr_point = ptr_polyg->first;
      while (ptr_point) {
	npoints++;
	ptr_point = ptr_point->next;
      }
      if (npoints > MAX_NPOINTS)
	mwerror(FATAL, 2, "Too many points in an area!\n");
      if (ptrc)
	ENCODE_INT(npoints - 1, 1 << (NBIT_NPOINTS - 1), compress);
      effnbit += NBIT_NPOINTS;
      if (npoints > maxpoint_polyg)
	maxpoint_polyg = npoints;
      ptr_polyg = ptr_polyg->next;
    }

    ptr_polyg = selectarea->first;
    while (ptr_polyg) {
      ptr_point = ptr_polyg->first;
      while (ptr_point) {

	if ((ptr_point->x < 0) || (ptr_point->x >= ncol) || (ptr_point->y < 0) || (ptr_point->y >= nrow))
	  mwerror(FATAL, 2, "Vertex of polygon outside of image!\nr = %d, c = %d, nrow = %d, ncol = %d.\n", ptr_point->y, ptr_point->x, nrow, ncol);

	/*--- Encode one vertex of polygon ---*/ 

	if (ptrc) {
	  ENCODE_INT(ptr_point->x, 1 << (NBIT_SIZEIM - 1), compress);
	  ENCODE_INT(ptr_point->y, 1 << (NBIT_SIZEIM - 1), compress);
	}
	effnbit += 2 * NBIT_SIZEIM;	  
	ptr_point = ptr_point->next;
      }
	
      ptr_polyg = ptr_polyg->next;
    }

  } else
    npolyg = 0;

  stop_test = TRUE;
  stop_test_area[BG_SYMB] = PROGRESS;
  for (p = 1; p <= npolyg; p++)
    stop_test_area[p] = PROGRESS;
}


static void
UPDATE_MODEL(symbol, cum_freq, freq, nsymbol)

int        symbol;
long       cum_freq[], freq[];
int        nsymbol;

{
  int      z;
  int      cum;

  if (cum_freq[0] == max_freq) {
    cum = 0;
    for (z = nsymbol + 1; z >= 0; z--) {
      freq[z] = (freq[z] + 1) / 2;
      cum_freq[z] = cum;
      cum += freq[z];
    }
  }
  freq[symbol]++;
  for (z = symbol - 1; z >= 0; z--)
    cum_freq[z]++;

}



static void
OUTPUT_BITS(bit, compress)

int              bit;
Cimage           compress;

{

  if (ptrc)
    ADD_BIT_TO_OUTPUT(bit, compress);
  while (bits_to_follow > 0) {
    if (ptrc)
      ADD_BIT_TO_OUTPUT(!bit, compress);
    bits_to_follow -= 1;
  }

}



static void
ENCODE_SYMBOL(symbol, cum_freq, effnbitar, compress)

unsigned char    symbol;
long             cum_freq[];
long            *effnbitar;
Cimage           compress;

{
  long     range;

  *effnbitar = 0;
  range = (long) high - low + 1;
  high = low + (range * cum_freq[symbol - 1]) / cum_freq[0] - 1;
  low = low + (range * cum_freq[symbol]) / cum_freq[0];

  for (;;) {
    if (high < half) {
      OUTPUT_BITS(0, compress);
      *effnbitar += 1;
    } else
      if (low >= half) {
	OUTPUT_BITS(1, compress);
	*effnbitar += 1;
	low -= half;
	high -= half;
      } else
	if ((low >= first_qrt) && (high < third_qrt)) {
	  bits_to_follow += 1;
	  *effnbitar += 1;
	  low -= first_qrt;
	  high -= first_qrt;
	} else
	  break;

    low = 2 * low;
    high = 2 * high + 1;
  }

}



static void
ENCODE_SIGNIF_MAP(wtrans, output, compress, sigmap, areamap, nsignif, nrec, thres, printfull, distrate)

Wtrans2d         wtrans, output;
Cimage           compress;
unsigned char ***sigmap;             /* Map of significance information */
unsigned char ***areamap;            /* Mask for selected areas */
long            *nsignif;
int              nrec;
float            thres;
int             *printfull;
int             *distrate;

{
  int             i;               /* Direction of sub-image 
				    * in wavelet transform */
  int             j;               /* Level of sub-image 
				    * in wavelet transform */
  register float *ptrw, *ptro;      /* Pointers to wavelet coeff. in 
				     * wtrans and output */
  register unsigned char *ptrs;    /* Pointer to significance map */
  register unsigned char *ptra;    /* Pointer to area mask */
  long            x, size;
  long            ncol;
  double          quant_step;
  long            count_sigpos, count_signeg, count_ztroot, count_isolz;
  long            count_total;
  double          rate;
  long            effnbitar;
  int             z;
  long            freq[7];
  long            cum_freq[7];
  int             nsymbol;
  double          sizei;
  long            EOF_nbit;
  unsigned char   old_area_index;   /* Area index before redrawing */
  unsigned char   EOP_symb;         /* End of pass symbol */
  unsigned char   EOA_symb;         /* End of area symbol */

  sizei = (double) wtrans->ncol * wtrans->nrow;

  /*--- Initialize model for arithmetic coding ---*/

  nsymbol = 5;
  for (z = 0; z <= nsymbol + 1; z++) {
    freq[z] = 1;
    cum_freq[z] = nsymbol + 1 - z;
  }
  freq[0] = 0;
  EOA_symb = nsymbol;
  EOP_symb = nsymbol + 1;

  /*--- Initialize encoding ---*/
  
  low = 0;
  high = top_value;

  count_sigpos = count_signeg = count_ztroot = count_isolz = 0;

  quant_step = 1.5 * thres;
  size = wtrans->images[nrec][0]->nrow * wtrans->images[nrec][0]->ncol;
  ptro = output->images[nrec][0]->gray;
  ptrw = wtrans->images[nrec][0]->gray;
  ptrs = sigmap[nrec][0];
  ptra = areamap[nrec][0];
  for (x = 0; (x < size) && (stop_test == TRUE); x++) 
    if (stop_test_area[*ptra] == PROGRESS) {
      if (*ptrs == POS_SYMB) {

	/*--- Coefficient is significant positive ---*/

	*ptro = quant_step;
	effmse_sa[*ptra] += (*ptrw - quant_step) * (*ptrw - quant_step) - *ptrw * *ptrw;
	(*nsignif)++;
	count_sigpos++;
      } else
	if (*ptrs == NEG_SYMB) {

	  /*--- Coefficient is significant negative ---*/

	  *ptro = - quant_step;
	  effmse_sa[*ptra] += (*ptrw + quant_step) * (*ptrw + quant_step) - *ptrw * *ptrw;
	  (*nsignif)++;
	  count_signeg++;
	}

      if (*ptrs == ZTR_SYMB)
	count_ztroot++;
      else
	if (*ptrs == ISO_SYMB)
	  count_isolz++;

      if ((*ptrs <= NEG_SYMB) && (*ptrs >= ZTR_SYMB)) {

	/*--- Encode symbol and update model ---*/

	ENCODE_SYMBOL(*ptrs, cum_freq, &effnbitar, compress);
	UPDATE_MODEL(*ptrs, cum_freq, freq, nsymbol);

	/*--- Test if target mse or rate is reached ---*/

	if (distrate) {
	  effnbit += effnbitar;
	  if (effnbit + bits_to_follow + 5 >= targnbit_dr[count_dr]) {
	    EOF_nbit = 0;
	    ENCODE_SYMBOL(EOP_symb, cum_freq, &EOF_nbit, NULL);
	    COMPUTE_TOTAL_MSE();
	    if (!drc)
	      printf("%.4f  %.2f\n", ((double) effnbit + EOF_nbit + bits_to_follow + 1.0) / sizei, 10.0 * log10((double) sizei * MAX_GREYVAL * MAX_GREYVAL / effmse));
	    if (drc)
	      drc->mse[count_dr] = effmse / (double) sizei;
	    if (count_dr == max_count_dr)
	      stop_test = FALSE;
	    count_dr++;
	  }
	} else
	  {
	    effnbit_sa[*ptra] += effnbitar;
	    if ((effmse_sa[*ptra] <= targmse_sa[*ptra]) || (effnbit_sa[*ptra] + bits_to_follow >= targnbit_sa[*ptra] - 5)) 
	      stop_test_area[*ptra] = END_NEXT;
	    
	  }
      }
      ptro++;
      ptrw++;
      ptrs++;
      ptra++;

    } else
      if ((stop_test_area[*ptra] == END_NEXT) && (*ptrs <= NEG_SYMB) && (*ptrs >= ZTR_SYMB)) {

	/*--- Encode end of area symbol and update model ---*/
	
	ENCODE_SYMBOL(EOA_symb, cum_freq, &effnbitar, compress);
	UPDATE_MODEL(EOA_symb, cum_freq, freq, nsymbol);
	effnbit_sa[*ptra] += effnbitar;
	stop_test_area[*ptra] = ENDED;
	TEST_END_ENCODING();
	if (stop_test == TRUE) {
	  old_area_index = *ptra;
	  REDRAW_AREAMAP(wtrans, output, nrec, areamap, *ptra);
	  if (old_area_index == *ptra) {
	    ptro++;
	    ptrw++;
	    ptrs++;
	    ptra++;
	  } else
	    x--;
	}
      } else
	{
	  ptro++;
	  ptrw++;
	  ptrs++;
	  ptra++;
	}


  
  for (j = nrec; j >= 1; j--)
    for (i=1; i<=3; i++) {
      size = wtrans->images[j][i]->nrow * wtrans->images[j][i]->ncol;
      ptro = output->images[j][i]->gray;
      ptrw = wtrans->images[j][i]->gray;
      ptrs = sigmap[j][i];
      ptra = areamap[j][i];
      for (x = 0; (x < size) && (stop_test == TRUE); x++) 
	if (stop_test_area[*ptra] == PROGRESS) {
	  if (*ptrs == POS_SYMB) {

	    /*--- Coefficient is significant positive ---*/

	    *ptro = quant_step;
	    effmse_sa[*ptra] += (*ptrw - quant_step) * (*ptrw - quant_step) - *ptrw * *ptrw;
	    (*nsignif)++;
	    count_sigpos++;
	  } else
	    if (*ptrs == NEG_SYMB) {

	      /*--- Coefficient is significant negative ---*/

	      *ptro = - quant_step;
	      effmse_sa[*ptra] += (*ptrw + quant_step) * (*ptrw + quant_step) - *ptrw * *ptrw;
	      (*nsignif)++;
	      count_signeg++;
	    }

	  if (*ptrs == ZTR_SYMB)
	    count_ztroot++;
	  else
	    if (*ptrs == ISO_SYMB)
	      count_isolz++;

	  if ((*ptrs <= NEG_SYMB) && (*ptrs >= ZTR_SYMB)) {
	  
	    /*--- Encode symbol and update model ---*/
	  
	    ENCODE_SYMBOL(*ptrs, cum_freq, &effnbitar, compress);
	    UPDATE_MODEL(*ptrs, cum_freq, freq, nsymbol);

	    /*--- Test if target mse or rate is reached ---*/

	    if (distrate) {
	      effnbit += effnbitar;
	      if (effnbit + bits_to_follow + 5 >= targnbit_dr[count_dr]) {
		EOF_nbit = 0;
		ENCODE_SYMBOL(EOP_symb, cum_freq, &EOF_nbit, NULL);
		COMPUTE_TOTAL_MSE();
		if (!drc)
		  printf("%.4f  %.2f\n", ((double) effnbit + EOF_nbit + bits_to_follow + 1.0) / sizei, 10.0 * log10((double) sizei * MAX_GREYVAL * MAX_GREYVAL / effmse));
		if (drc)
		  drc->mse[count_dr] = effmse / (double) sizei;
		if (count_dr == max_count_dr)
		  stop_test = FALSE;
		count_dr++;
	      }
	    } else
	      {
		effnbit_sa[*ptra] += effnbitar;
		if ((effmse_sa[*ptra] <= targmse_sa[*ptra]) || (effnbit_sa[*ptra] + bits_to_follow >= targnbit_sa[*ptra] - 5)) 
		  stop_test_area[*ptra] = END_NEXT;
	      }
	  }
	  ptro++;
	  ptrw++;
	  ptrs++;
	  ptra++;

	} else
	  if ((stop_test_area[*ptra] == END_NEXT) && (*ptrs <= NEG_SYMB) && (*ptrs >= ZTR_SYMB)) {

	    /*--- Encode end of area symbol and update model ---*/

	    ENCODE_SYMBOL(EOA_symb, cum_freq, &effnbitar, compress);
	    UPDATE_MODEL(EOA_symb, cum_freq, freq, nsymbol);
	    effnbit_sa[*ptra] += effnbitar;
	    stop_test_area[*ptra] = ENDED;
	    TEST_END_ENCODING();
	    if (stop_test == TRUE) {
	      old_area_index = *ptra;
	      REDRAW_AREAMAP(wtrans, output, nrec, areamap, *ptra);
	      if (old_area_index == *ptra) {
		ptro++;
		ptrw++;
		ptrs++;
		ptra++;
	      } else
		x--;
	    }
	  } else
	    {
	      ptro++;
	      ptrw++;
	      ptrs++;
	      ptra++;
	    }
    } 

  ENCODE_SYMBOL(EOP_symb, cum_freq, &effnbitar, compress);
  bits_to_follow += 1;
  if (low < first_qrt)
    OUTPUT_BITS(0, compress);
  else
    OUTPUT_BITS(1, compress);

  if (distrate)
    effnbit += effnbitar + 1;
  else
    effnbit_sa[BG_SYMB] += effnbitar + 1;

  count_total = count_sigpos + count_signeg + count_ztroot + count_isolz;
  if (count_total > 0)
    rate = (double) count_total * log((double) count_total);
  else
    rate = 0.0;
  if (count_sigpos > 0)
    rate -= (double) count_sigpos * log((double) count_sigpos);
  if (count_signeg > 0)
    rate -= (double) count_signeg * log((double) count_signeg);
  if (count_ztroot > 0)
    rate -= (double) count_ztroot * log((double) count_ztroot);
  if (count_isolz > 0)
    rate -= (double) count_isolz * log((double) count_isolz);
  rate /= log((double) 2.0);
  if (count_total > 0)
    rate /= (double) count_total;

  if (printfull && !distrate)
    printf("ZTR =%6d, IZ =%5d, RM = %.2f, ", count_ztroot, count_isolz, rate);
}




static void
ENCODE_QUANT_STEP(wtrans, output, compress, sigmap, areamap, nrec, thres, printfull, distrate)

Wtrans2d         wtrans, output;
Cimage           compress;
unsigned char ***sigmap;             /* Map of significance information */
unsigned char ***areamap;            /* Mask for selected areas */
int              nrec;
float            thres;
int             *printfull;
int             *distrate;

{
  int             i;                /* Index for orientation in wav. trans. */
  int             j;                /* Index for level in wav. trans. */
  register float *ptrw, *ptro;      /* Pointers to wavelet coeff. in 
				     * wtrans and output */
  double          quant_step;
  register unsigned char *ptrs;
  register unsigned char *ptra;     /* Pointer to area mask */
  long            x, size;
  long            count_sigpos, count_signeg;
  long            count_total;
  long            effnbitar;
  double          rate;
  unsigned char   symbol;           /* Binary symbol for quantization 
				     * refinement */
  int             z;
  long            freq[5];
  long            cum_freq[5];
  int             nsymbol;
  double          sizei;
  long            EOF_nbit;
  unsigned char   old_area_index;   /* Area index before redrawing */
  unsigned char   EOP_symb;         /* End of pass symbol */
  unsigned char   EOA_symb;         /* End of area symbol */

  sizei = (double) wtrans->ncol * wtrans->nrow;

  /*--- Initialize model for arithmetic coding ---*/

  nsymbol = 3;
  for (z=0; z<=nsymbol+1; z++) {
    freq[z] = 1;
    cum_freq[z] = nsymbol + 1 - z;
  }
  freq[0] = 0;
  EOA_symb = nsymbol;
  EOP_symb = nsymbol + 1;

  /*--- Initialize encoding ---*/
  
  low = 0;
  high = top_value;

  count_sigpos = count_signeg = 0;

  quant_step = 0.5 * thres;
  size = wtrans->images[nrec][0]->nrow * wtrans->images[nrec][0]->ncol;
  ptro = output->images[nrec][0]->gray;
  ptrw = wtrans->images[nrec][0]->gray;
  ptrs = sigmap[nrec][0];
  ptra = areamap[nrec][0];
  for (x = 0; (x < size) && (stop_test == TRUE); x++) 
    if (stop_test_area[*ptra] == PROGRESS) {
      if (*ptrs == SIG_SYMB) {
	effmse_sa[*ptra] -= (*ptrw - *ptro) * (*ptrw - *ptro);
	if (*ptrw > *ptro) { 
	  *ptro += quant_step;
	  count_sigpos++;
	  symbol = 1;
	} else
	  {
	    *ptro -= quant_step;
	    count_signeg++;
	    symbol = 2;
	  }
	effmse_sa[*ptra] += (*ptrw - *ptro) * (*ptrw - *ptro);

	/*--- Encode symbol and update model ---*/
      
	ENCODE_SYMBOL(symbol, cum_freq, &effnbitar, compress);
	UPDATE_MODEL(symbol, cum_freq, freq, nsymbol);

	/*--- Test if target mse or rate is reached ---*/
      
	if (distrate) {
	  effnbit += effnbitar;
	  if (effnbit + bits_to_follow + 5 >= targnbit_dr[count_dr]) {
	    EOF_nbit = 0;
	    ENCODE_SYMBOL(EOP_symb, cum_freq, &EOF_nbit, NULL);
	    COMPUTE_TOTAL_MSE();
	    if (!drc)
	      printf("%.4f  %.2f\n", ((double) effnbit + EOF_nbit + bits_to_follow + 1.0) / sizei, 10.0 * log10((double) sizei * MAX_GREYVAL * MAX_GREYVAL / effmse));
	    if (drc)
	      drc->mse[count_dr] = effmse / (double) sizei;
	    if (count_dr == max_count_dr)
	      stop_test = FALSE;
	    count_dr++;
	  }
	} else
	  {
	    effnbit_sa[*ptra] += effnbitar;
	    if ((effmse_sa[*ptra] <= targmse_sa[*ptra]) || (effnbit_sa[*ptra] + bits_to_follow >= targnbit_sa[*ptra] - 5)) 
	      stop_test_area[*ptra] = END_NEXT;
	  }
	
      }
      ptro++;
      ptrw++;
      ptrs++;
      ptra++;

    } else
      if ((stop_test_area[*ptra] == END_NEXT) && (*ptrs == SIG_SYMB)) {

	/*--- Encode end of area symbol and update model ---*/

	ENCODE_SYMBOL(EOA_symb, cum_freq, &effnbitar, compress);
	UPDATE_MODEL(EOA_symb, cum_freq, freq, nsymbol);
	effnbit_sa[*ptra] += effnbitar;
	stop_test_area[*ptra] = ENDED;
	TEST_END_ENCODING();
	if (stop_test == TRUE) {
	  old_area_index = *ptra;
	  REDRAW_AREAMAP(wtrans, output, nrec, areamap, *ptra);
	  if (old_area_index == *ptra) {
	    ptro++;
	    ptrw++;
	    ptrs++;
	    ptra++;
	  } else
	    x--;
	}
      } else
	{
	  ptro++;
	  ptrw++;
	  ptrs++;
	  ptra++;
	}


  
  for (j = nrec; j >= 1; j--)
    for (i=1; i<=3; i++) {
      size = wtrans->images[j][i]->nrow * wtrans->images[j][i]->ncol;
      ptro = output->images[j][i]->gray;
      ptrw = wtrans->images[j][i]->gray;
      ptrs = sigmap[j][i];
      ptra = areamap[j][i];
      for (x = 0; (x < size) && (stop_test == TRUE); x++) 
	if (stop_test_area[*ptra] == PROGRESS) {
	  if (*ptrs == SIG_SYMB) {
	    effmse_sa[*ptra] -= (*ptrw - *ptro) * (*ptrw - *ptro);
	    if (*ptrw > *ptro) { 
	      *ptro += quant_step;
	      count_sigpos++;
	      symbol = 1;
	    } else
	      {
		*ptro -= quant_step;
		count_signeg++;
		symbol = 2;
	      }
	    effmse_sa[*ptra] += (*ptrw - *ptro) * (*ptrw - *ptro);

	    /*--- Encode symbol and update model ---*/
	  
	    ENCODE_SYMBOL(symbol, cum_freq, &effnbitar, compress);
	    UPDATE_MODEL(symbol, cum_freq, freq, nsymbol);
	
	    /*--- Test if target mse or rate is reached ---*/

	    if (distrate) {
	      effnbit += effnbitar;
	      if (effnbit + bits_to_follow + 5 >= targnbit_dr[count_dr]) {
		EOF_nbit = 0;
		ENCODE_SYMBOL(EOP_symb, cum_freq, &EOF_nbit, NULL);
		COMPUTE_TOTAL_MSE();
		if (!drc)
		  printf("%.4f  %.2f\n", ((double) effnbit + EOF_nbit + bits_to_follow + 1.0) / sizei, 10.0 * log10((double) sizei * MAX_GREYVAL * MAX_GREYVAL / effmse));
		if (drc)
		  drc->mse[count_dr] = effmse / (double) sizei;
		if (count_dr == max_count_dr)
		  stop_test = FALSE;
		count_dr++;
	      }
	    } else
	      {
		effnbit_sa[*ptra] += effnbitar;
		if ((effmse_sa[*ptra] <= targmse_sa[*ptra]) || (effnbit_sa[*ptra] + bits_to_follow >= targnbit_sa[*ptra] - 5)) 
		  stop_test_area[*ptra] = END_NEXT;
	      }
	  
	  }
	  ptro++;
	  ptrw++;
	  ptrs++;
	  ptra++;

	} else
	  if ((stop_test_area[*ptra] == END_NEXT) && (*ptrs == SIG_SYMB)) {

	    /*--- Encode end of area symbol and update model ---*/

	    ENCODE_SYMBOL(EOA_symb, cum_freq, &effnbitar, compress);
	    UPDATE_MODEL(EOA_symb, cum_freq, freq, nsymbol);
	    effnbit_sa[*ptra] += effnbitar;
	    stop_test_area[*ptra] = ENDED;
	    TEST_END_ENCODING();
	    if (stop_test == TRUE) {
	      old_area_index = *ptra;
	      REDRAW_AREAMAP(wtrans, output, nrec, areamap, *ptra);
	      if (old_area_index == *ptra) {
		ptro++;
		ptrw++;
		ptrs++;
		ptra++;
	      } else
		x--;
	    }
	  } else
	    {
	      ptro++;
	      ptrw++;
	      ptrs++;
	      ptra++;
	    }


    } 

  ENCODE_SYMBOL(EOP_symb, cum_freq, &effnbitar, compress);
  bits_to_follow += 1;
  if (low < first_qrt)
    OUTPUT_BITS(0, compress);
  else
    OUTPUT_BITS(1, compress);

  if (distrate)
    effnbit += effnbitar + 1;
  else
    effnbit_sa[BG_SYMB] += effnbitar + 1;

  count_total = count_sigpos + count_signeg;
  if (count_total > 0)
    rate = (double) count_total * log((double) count_total);
  else
    rate = 0.0;
  if (count_sigpos > 0)
    rate -= (double) count_sigpos * log((double) count_sigpos);
  if (count_signeg > 0)
    rate -= (double) count_signeg * log((double) count_signeg);
  rate /= log((double) 2.0);
  if (count_total > 0)
    rate /= (double) count_total;
  if (!distrate && printfull)
    printf("RQ = %.2f,   ", rate); 
}




static void
ZERO_TREE_ENCODE(wtrans, output, compress, nrec, thres, rate, psnr, selectarea, printfull, distrate)

Wtrans2d     wtrans, output;     /* Input and quantized wavelet transforms */
Cimage       compress;           /* Compressed file */
int          nrec;               /* Number of level to consider in wavelet 
				  * transform */
float        thres;              /* Initial threshold */
float       *rate;               /* Target bit rate for background */
float       *psnr;               /* Target psnr for background */
Polygons     selectarea;         /* Polygnal regions to be encoded with 
				  * a special rate or PSNR */
int         *printfull;          /* Flag for information printing */
int         *distrate;           /* Flag for computation of dist. rate curve */

{
  int              i;                /* Index for orientation in wav. trans. */
  int              j, jp;            /* Index for level in wav. trans. */
  unsigned char ***sigmap;           /* Map of significance information */
  unsigned char ***areamap;          /* Mask for selected areas */
  register float  *ptrw, *ptro;      /* Pointers to wavelet coeff. in 
				      * wtrans and output */
  register unsigned char  *ptrs;     /* Pointer to significance map */
  register unsigned char  *ptra;     /* Pointer to area mask */
  int              rp, cp;           /* Row and column indices for parent */
  long             x, xp;            /* Buffer index for current coefficient 
				      * and its parent */
  long             size;             /* Size of subimages */
  long             ncol;             /* Number of columns in subimages */
  long             nsignif;          /* Number of significant coefficients */
  int              test_par;         /* test flag for parents */
  float            effrate;          /* Effective rate */
  long             effnbitar;        /* Number of bit encoded at each call 
				      * of ENCODE_SYMBOL */
  double           thresprec;        /* Quantization step for initial 
				      * threshold */
  int              thressymb;        /* Threshold symbol for encoding */

  /*--- Computes exact threshold ---*/

  thresprec = pow((double) 2.0, (double) nrec - 4.0);
  thressymb = (int) ((double) thres / thresprec + 1.0);
  if (thressymb > max_thressymb) {
    thressymb = max_thressymb;
    mwerror(WARNING, 0, "Threshold is smaller than half of the max. coeff. amplitude :\n %.2f / %.2f\n", thressymb * thresprec, thres);
  }
   thres = thressymb * thresprec;
  
  if (printfull)
    printf("Thres = %.2f\n", thres);

  /*--- Init encoding ---*/

  INIT_ENCODING(nrec, wtrans->nrow, wtrans->ncol, thressymb, selectarea, compress);

    /*--- Memory allocation and initialisation for significance map ---*/

  sigmap = (unsigned char ***) malloc((int) (nrec + 1) * sizeof(unsigned char **));
  if (sigmap == NULL)
    mwerror(FATAL, 1, "Allocation for significance map refused!\n");

  INIT_SIGMAP(wtrans, nrec, sigmap);

    /*--- Memory allocation and initialisation for selected area mask ---*/

  areamap = (unsigned char ***) malloc((int) (nrec + 1) * sizeof(unsigned char **));
  if (areamap == NULL)
    mwerror(FATAL, 1, "Allocation for significance map refused!\n");

  INIT_AREAMAP(wtrans, nrec, areamap, selectarea);

  /*--- Computes target and initial effective MSE and target number ---*/
  /*--- of bits and initialize quantized wavelet transform ---*/

  COMPUTE_EFF_TARGET_RATE_MSE(wtrans, output, nrec, areamap, selectarea, psnr, rate);

  /*--- Encoding ---*/

  nsignif = 0;
  while (stop_test == TRUE) {
    
    /*--- Dominant pass ---*/

    size = wtrans->images[nrec][0]->nrow * wtrans->images[nrec][0]->ncol;
    ptrw = wtrans->images[nrec][0]->gray;
    ptrs = sigmap[nrec][0];
    ptra = areamap[nrec][0];
    for (x = 0; x < size; x++, ptrw++, ptrs++, ptra++) 
      if (stop_test_area[*ptra] == PROGRESS) {
	if (*ptrs >= POS_SYMB) {

	  /*--- Coefficient is already significant ---*/

	  *ptrs = SIG_SYMB;

	} else
	  {

	    /*--- Coefficient was not already  significant ---*/

	    if (fabs((double) *ptrw) >= thres) {

	      /*--- Coefficient is significant ---*/

	      if (*ptrw >= 0.0) 
		*ptrs = POS_SYMB;
	      else
		*ptrs = NEG_SYMB;
	    } 
	  }      
      }

    for (j = nrec; j >= 1; j--)
      for (i=1; i<=3; i++) {
	size = wtrans->images[j][i]->nrow * wtrans->images[j][i]->ncol;
	ptrw = wtrans->images[j][i]->gray;
	ptrs = sigmap[j][i];
	ptra = areamap[j][i];
	for (x = 0; x < size; x++, ptrs++, ptra++, ptrw++) 
	  if (stop_test_area[*ptra] != ENDED) {
	    if (*ptrs >= POS_SYMB) {

	      /*--- Coefficient is already significant ---*/

	      *ptrs = SIG_SYMB;
	    
	    } else
	      {

		/*--- Coefficient was not already  significant ---*/

		if (fabs((double) *ptrw) >= thres) {

		  /*--- Coefficient is significant ---*/

		  if (*ptrw >= 0.0) 
		    *ptrs = POS_SYMB;
		  else
		    *ptrs = NEG_SYMB;

		  /*--- Check parents for isolated zero ---*/

		  ncol = wtrans->images[j][i]->ncol;
		  jp = j+1;
		  xp = x;
		  test_par = TRUE;
		  while ((jp <= nrec) && (test_par == TRUE)) {
		    rp = xp / (2 * ncol);
		    if (rp >= wtrans->images[jp][i]->nrow) {
		      if ((wtrans->images[jp - 1][i]->nrow % 2 == 0) || (rp != wtrans->images[jp][i]->nrow))
			mwerror(WARNING, 0, "rp too large in ZERO_TREE_ENCODE (checking for isolated zero)\nj = %d, jp = %d, i = %d, rp = %d, nrow = %d, xp = %d\n", j, jp, i, rp, wtrans->images[jp][i]->nrow, xp);
		      rp--;
		    }
		    cp = (xp % ncol) / 2;
		    if (cp >= wtrans->images[jp][i]->ncol) {
		      if ((wtrans->images[jp - 1][i]->ncol % 2 == 0) || (cp != wtrans->images[jp][i]->ncol))
			mwerror(WARNING, 0, "cp too large in ZERO_TREE_ENCODE (checking for isolated zero)\nj = %d, jp = %d, i = %d, cp = %d, ncol = %d, xp = %d\n", j, jp, i, cp, wtrans->images[jp][i]->ncol, xp);
		      cp--;
		    }
		    xp = rp * wtrans->images[jp][i]->ncol + cp;
		    if (xp >= wtrans->images[jp][i]->ncol * wtrans->images[jp][i]->nrow)
		      mwerror(WARNING, 0, "xp too large in ZERO_TREE_ENCODE (checking for isolated zero)\nj = %d, jp = %d, i = %d, rp = %d, cp = %d, xp = %d, size = %d\n", j, jp, i, rp, cp, xp, size);
		    if (sigmap[jp][i][xp] <= ZTR_SYMB)
		      sigmap[jp][i][xp] = ISO_SYMB;
		    else
		      test_par = FALSE;
		    ncol = wtrans->images[jp][i]->ncol;
		    jp++;
		  }
		  if ((jp == nrec + 1) && (test_par == TRUE)) {
		    if (xp >= wtrans->images[nrec][0]->ncol * wtrans->images[nrec][0]->nrow)
		      mwerror(WARNING, 0, "xp too large in ZERO_TREE_ENCODE (checking for isolated zero)\nj = %d, jp = %d, i = %d, rp = %d, cp = %d, xp = %d, size = %d\n", j, jp, i, rp, cp, xp, size);

		    if (sigmap[nrec][0][xp] <= ZTR_SYMB)
		      sigmap[nrec][0][xp] = ISO_SYMB;
		  }  
		} 
	      }
	  }
      }
  
    /*--- Search new zerotree root ---*/

    for (j = nrec; j >= 1; j--)
      for (i=1; i<=3; i++) {
	size = wtrans->images[j][i]->nrow * wtrans->images[j][i]->ncol;
	ptrw = wtrans->images[j][i]->gray;
	ptrs = sigmap[j][i];
	ptra = areamap[j][i];
	for (x = 0; x < size; x++, ptrs++, ptra++, ptrw++) {

	  /*--- Check parents for zerotree roots ---*/

	  if (stop_test_area[*ptra] != ENDED) 
	    if (*ptrs < ZTR_SYMB)
	      if (j < nrec) {
		ncol = wtrans->images[j][i]->ncol;
		jp = j+1;
		rp = x / (2 * ncol);
		if (rp >= wtrans->images[jp][i]->nrow) {
		  if ((wtrans->images[jp - 1][i]->nrow %2 == 0) || (rp != wtrans->images[jp][i]->nrow))
		    mwerror(WARNING, 0, "rp too large in ZERO_TREE_ENCODE\nj = %d, jp = %d, i = %d, rp = %d, nrow = %d, xp = %d\n", j, jp, i, rp, wtrans->images[jp][i]->nrow, xp);
		  rp--;
		}
		cp = (x % ncol) / 2;
		if (cp >= wtrans->images[jp][i]->ncol) {
		  if ((wtrans->images[jp - 1][i]->ncol %2 == 0) || (cp != wtrans->images[jp][i]->ncol))
		    mwerror(WARNING, 0, "cp too large in ZERO_TREE_ENCODE\nj = %d, jp = %d, i = %d, cp = %d, ncol = %d, xp = %d\n", j, jp, i, cp, wtrans->images[jp][i]->ncol, xp);
		  cp--;
		}
		xp = rp * wtrans->images[jp][i]->ncol + cp;
		if (xp >= wtrans->images[jp][i]->ncol * wtrans->images[jp][i]->nrow)
		  mwerror(WARNING, 0, "xp too large in ZERO_TREE_ENCODE\nj = %d, jp = %d, i = %d, rp = %d, cp = %d, xp = %d, size = %d\n", j, jp, i, rp, cp, xp, wtrans->images[jp][i]->ncol * wtrans->images[jp][i]->nrow);
		if (sigmap[jp][i][xp] > ZTR_SYMB)
		  *ptrs = ZTR_SYMB;
	      } else
		{
		  ncol = wtrans->images[j][i]->ncol;
		  rp = x / ncol;
		  cp = x % ncol;
		  xp = rp * wtrans->images[nrec][0]->ncol + cp;
		  if (sigmap[nrec][0][xp] > ZTR_SYMB)
		    *ptrs = ZTR_SYMB; 
		}
	}
	
      }      
  
  

    size = wtrans->nrow * wtrans->ncol;

    ENCODE_SIGNIF_MAP(wtrans, output, compress, sigmap, areamap, &nsignif, nrec, thres, printfull, distrate);

    if (stop_test == TRUE)
      ENCODE_QUANT_STEP(wtrans, output, compress, sigmap, areamap, nrec, thres, printfull, distrate);
     
    thres /= 2.0;

    if (!distrate && printfull) {
      effrate = (double) effnbit / size; 
      printf("Rate = %.4f, Rate AR = %.4f, PSNR = %.2f,  nsignif = %d\n", effrate, (double) effnbit / size, 10.0 * log10((double) size * MAX_GREYVAL * MAX_GREYVAL / effmse), nsignif);
    }
  }
  
  if (compress) {
    if (bits_to_go < 8) {
      buffer >>= bits_to_go;
      *ptrc = buffer;
    }
    RESIZE_COMPRESS(compress);
    sprintf(compress->cmt,"%s","");
  }

  effmse /= (double) size;

    /*--- Desallocation for significance map ---*/

  free(sigmap[nrec][0]);
  for (j = nrec; j >= 1; j--)
    for (i = 3; i >= 1; i--)
      free(sigmap[j][i]);
  for (j = nrec; j >= 1; j--)
    free(sigmap[j]);
  free(sigmap);

    /*--- Desallocation for selected area mask ---*/

  free(areamap[nrec][0]);
  for (j = nrec; j >= 1; j--)
    for (i = 3; i >= 1; i--)
      free(areamap[j][i]);
  for (j = nrec; j >= 1; j--)
    free(areamap[j]);
  free(areamap);

}



void
ezw(PrintFull, NumRec, WeightFac, Thres, Max_Count_AC, DistRate, Rate, PSNR, SelectedArea, Compress, Wtrans, Output, PtrDRC)

int            *PrintFull;       /* Print full set of information */
int            *NumRec;		 /* Consider only *Numrec level */
float          *WeightFac;       /* Weighting factor for wavelet coeff. */
float          *Thres;           /* Initial value for theshold */
int            *Max_Count_AC;    /* Capacity of histogram for ar. coding */
int            *DistRate;        /* Compute distortion-rate function */
float          *Rate;            /* Target Rate */
float          *PSNR;            /* Target PSNR */
Polygons        SelectedArea;    /* Polygnal regions to be encoded with 
				  * a special rate or PSNR */
Cimage          Compress;	 /* Compressed `Image` */
Wtrans2d        Wtrans;          /* Input wavelet transform */
Wtrans2d        Output;          /* Quantized wavelet transform */
char           *PtrDRC;          /* Distorsion rate curve */

{
  int             NRec;
  float           Threshold;

  /*--- Memory allocation for quantized wavelet transform ---*/

  if (mw_alloc_ortho_wtrans2d(Output, Wtrans->nlevel, Wtrans->nrow, Wtrans->ncol) == NULL)
    mwerror(FATAL, 1, "Allocation of buffer for wavelet transform refused!");

  /*--- Memory allocation for compressed image buffer ---*/

  if (Compress) {
    Compress = mw_change_cimage(Compress, Wtrans->nrow,  Wtrans->ncol);
    if (Compress == NULL)
      mwerror(FATAL, 1, "Memory allocation refused for `RecompImage`!\n");
    sizec = Compress->ncol * Compress->nrow;
  }

  /*--- Check number of level in wavelet transform ---*/

  NRec = Wtrans->nlevel;
  if (NumRec)
    if ((*NumRec > 0) && (*NumRec <= Wtrans->nlevel))
      NRec = *NumRec;

  /*--- Weight wavelet coefficients according to sub-image ---*/

  if (WeightFac)
    if (*WeightFac > 0.0)
      SCALE_WAVT(Wtrans, NRec, *WeightFac);

  /*--- Computes initial threshold value for significance ---*/

  if (Thres)
    Threshold = *Thres;
  else 
    Threshold = COMPUTES_THRES_ADAP(Wtrans, NRec);

  max_freq = *Max_Count_AC;
  if (Compress)
    max_freq = 256;

  if (DistRate) {

  /*--- Output distortion rate data ---*/

    INIT_TARGNBIT_DR(Wtrans, PtrDRC);
    Rate = NULL;
    PSNR = NULL;
    SelectedArea = NULL;
    PrintFull = NULL;
  }

  /*--- Quantize and encode wavelet transform Wtrans ---*/

  ZERO_TREE_ENCODE(Wtrans, Output, Compress, NRec, Threshold, Rate, PSNR, SelectedArea, PrintFull, DistRate); 

  if (!DistRate && WeightFac)
    if (*WeightFac > 0.0)
      SCALE_WAVT(Output, NRec, 1.0 / *WeightFac);

}

/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {iezw};
author = {"Jean-Pierre D'Ales"};
function = {"Wavelet transform compression via EZW algorithm"};
version = {"1.30"};
usage = {
'p'->PrintFull
	"Print full set of information", 
'w':WeightFac->WeightFac
        "Scaling factor for wavelet coefficients", 
Compress->Compress
          "Input string of codewords (cimage)",
QWavTrans<-Output
	"Output reconstructed wavelet transform (wtrans2d)"
	};
 */


/*--- Include files UNIX C ---*/
#include <stdio.h>
#include <math.h>

/*--- Megawave2 library ---*/
#include  "mw.h"

/*--- Megawave2 modules definition ---*/

extern void fillpoly();

/*--- Constants ---*/

#define  NBIT_GREYVAL 8     /* Number of bit per pixels */
#define  MAX_GREYVAL 255.0  /* Maximum of grey levels in input image */
#define  NBIT_NREC 4        /* Number of bits used to encode the number 
			     * of levels in wavelet transform */
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

/*--- Constants for arithmetic encoding ---*/

#define  MAX_SIZEO 32383
#define  code_value_bits 16
#define  top_value  (((long) 1 << 16) - 1)
#define  first_qrt (top_value / 4 + 1)
#define  half (2 * first_qrt)
#define  third_qrt (3 * first_qrt)
#define  nbit_thressymb 16
#define  max_thressymb (((long) 1 << nbit_thressymb) - 1)

static int    max_freq;
static long   low, high;         /* Interval extremities 
                                  * for arithmetic coding */
static int    value;             /* Current value of decoded symbol */
static int    bits_to_go;        /* Number of free bits in the currently 
				  * encoded codeword */
static int    garbage_bits;
static int    buffer;
static long   ncwread;           /* Number of output codewords */
static unsigned char  *ptrc, *ptrc2, *ptrc3;
static long   effnbit, effnbitar;
static long   sizebuf;

static int    stop_test;         /* test flag for global stopping 
				  * of encoding */
static int    stop_test_area[MAX_NPOLYG]; /* test flag for stopping of encoding
				  * in a given area */
static Polygons selectarea;      /* Polygnal regions to be encoded with 
				  * a special rate or PSNR */
static int    npolyg;            /* Number of selected polygonal areas */





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

Wtrans2d         wtrans;         /* Input wavelet transforms */
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
INIT_AREAMAP(wtrans, nrec, areamap)

Wtrans2d         wtrans;         /* Input wavelet transforms */
int              nrec;           /* Number of level to consider in wavelet 
				  * transform */
unsigned char ***areamap;        /* Mask for selected areas */

{
  int              i;                /* Index for orientation in wav. trans. */
  int              j;                /* Index for level in wav. trans. */
  register unsigned char  *ptra;     /* Pointer to significance map */
  Polygon          ptr_polyg;        /* Pointer to the current polygon */
  unsigned char    p;                /* Index of polygon */
  long             x;                /* Buffer index for current coefficient */
  long             size;             /* Size of subimages */
  Cimage           bitmap;           /* bitmap image */
  int              dx, dy;           /* Size of image */

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
	mwerror(FATAL, 1, "Allocation for significance map refused!\n");
      for (x = 0, ptra = areamap[j][i]; x < size; x++, ptra++) 
	*ptra = BG_SYMB;      
    }
  }

  size = wtrans->images[nrec][0]->nrow * wtrans->images[nrec][0]->ncol;
  areamap[nrec][0] = (unsigned char *) malloc(size * sizeof(unsigned char));
  if (areamap[nrec][0] == NULL)
    mwerror(FATAL, 1, "Allocation for significance map refused!\n");
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




static void
REALLOCATE_SUBIMAGES(output)

Wtrans2d    output;

{
  int    i, j, nrec;
  int    nrow, ncol;
  int    nrow2, ncol2;

  nrec = output->nlevel;
  nrow = nrow2 = output->nrow;
  ncol = ncol2 = output->ncol;

  for (j = 1; j <= nrec; j++) {
    nrow2 >>= 1;
    ncol2 >>= 1;
    if (nrow % 2 == 1)
      nrow = nrow2 + 1;
    else
      nrow = nrow2;
    if (ncol % 2 == 1)
      ncol = ncol2 + 1;
    else
      ncol = ncol2;

    output->images[j][0] = mw_change_fimage(output->images[j][0], nrow, ncol);
    if (!output->images[j][0])
	mwerror(FATAL, 1, "Reallocation for output wavelet transform refused!\n");
    output->images[j][1] = mw_change_fimage(output->images[j][1], nrow2, ncol);
    if (!output->images[j][1])
	mwerror(FATAL, 1, "Reallocation for output wavelet transform refused!\n");
    output->images[j][2] = mw_change_fimage(output->images[j][2], nrow, ncol2);
    if (!output->images[j][2])
	mwerror(FATAL, 1, "Reallocation for output wavelet transform refused!\n");
    output->images[j][3] = mw_change_fimage(output->images[j][3], nrow2, ncol2);
    if (!output->images[j][3])
	mwerror(FATAL, 1, "Reallocation for output wavelet transform refused!\n");

    nrow2 = nrow;
    ncol2 = ncol;
  }


}




static void
TEST_END_DECODING() 

{
  int p;
  
  stop_test = FALSE;
  for (p = 0; p <= npolyg; p++) 
    if (stop_test_area[p] == TRUE)
      stop_test = TRUE;

}



static void
REDRAW_AREAMAP(wtrans, nrec, areamap, p)

Wtrans2d         wtrans;
int              nrec;
unsigned char ***areamap;            /* Mask for selected areas */
unsigned char    p;                  /* Index of area to be redrawn */

{
  int              i;                /* Index for orientation in wav. trans. */
  int              jp;               /* Index for level in wav. trans. */
  register unsigned char  *ptra;     /* Pointer to area mask */
  int              rp, cp;           /* Row and column indices for parent */
  long             x, xp;            /* Buffer index for current coefficient 
				      * and its parent */
  long             size;             /* Size of subimages */
  long             ncol;             /* Number of columns in subimages */

  for (i=1; i<=3; i++) {
    size = wtrans->images[1][i]->nrow * wtrans->images[1][i]->ncol;
    ptra = areamap[1][i];
    for (x = 0; x < size; x++, ptra++) 
      if (stop_test_area[*ptra] == TRUE) {


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
}
      
 

static int 
READ_BIT()

{
  int           bit;

  if (bits_to_go == 0) {
    if (ncwread == sizebuf) {
      garbage_bits++;
      bits_to_go = 1;
      if (garbage_bits > code_value_bits - 2)
	mwerror(FATAL, 1, "Buffer ended to soon while decoding a symbol!\n");
    } else 
      {
	ptrc++;
	ptrc2++;
	buffer = *ptrc;
	bits_to_go = 8;
	ncwread += 1;
      }
  }

  bit = buffer&1;
  buffer >>= 1;
  bits_to_go -= 1;
  effnbitar++;
  return bit;
}



static void
DECODE_INT(symb, max)

int           *symb, max;

{
  int bit;

  *symb = 0;
  while (max > 0) {
    *symb <<= 1;
    bit = READ_BIT();
    if (bit)
      *symb += 1;
    max /= 2;
  }
   
}


static void
READ_HEADER(compress, nrec, nrow, ncol, thres)

Cimage            compress;
int              *nrec;
int              *nrow, *ncol;
float            *thres;

{
  int           thressymb;
  Polygon       ptr_polyg;       /* Pointer to the current polygon */
  Polygon       alloc_polyg;     /* Table of polygon for memory allocation */
  Point_curve   ptr_point;       /* Pointer to the current point in current 
				  * polygon */
  Point_curve   nextp_point;     /* Pointer to the first point of next 
				  * polygon */
  Point_curve   alloc_point;     /* Table of points for memory allocation */ 
  int           p, v;
  int           c;
  int           npoints[MAX_NPOLYG], npoints_total;

  max_freq = 256;
  sizebuf = compress->nrow * compress->ncol;
  if (sizebuf == 0)
    mwerror(FATAL, 1, "Compressed file empty!\n");

  if (compress->firstcol > 0) {
    ncwread = compress->firstcol;
    ptrc = ptrc2 = compress->gray + ncwread - 1;
  } else
    {
      ncwread = 1;
      ptrc = ptrc2 = compress->gray;
    }
  buffer = *ptrc;
  if (compress->firstrow > 0) 
    bits_to_go = compress->firstrow;
  else
      bits_to_go = 8;
  buffer >>= 8 - bits_to_go;

  garbage_bits = 0;
  effnbitar = 0;
  DECODE_INT(nrec, 1 << (NBIT_NREC - 1), ptrc);
  printf("Number of levels : %d\n", *nrec);
  DECODE_INT(nrow, 1 << (NBIT_SIZEIM - 1), ptrc);
  printf("Number of rows of image : %d\n", *nrow);
  DECODE_INT(ncol, 1 << (NBIT_SIZEIM - 1), ptrc);
  printf("Number of columns of image : %d\n", *ncol);
  DECODE_INT(&thressymb, 1 << (nbit_thressymb - 1), ptrc);
  effnbit = effnbitar;

  /*--- Computes exact threshold ---*/

  *thres = (float) thressymb * pow((double) 2.0, (double) *nrec - 4.0);

  /*--- Decode number of polygons ---*/ 

  DECODE_INT(&npolyg, 1 << (NBIT_NPOLYG - 1), compress);
  printf("Number of selected areas : %d\n", npolyg);

  if (npolyg > 0) {

    /*--- Memory allocation for polygonal selected area ---*/

    selectarea = mw_new_polygons();
    if (selectarea == NULL) 
      mwerror(FATAL, 1, "Not enough memory for polygons!\n");

    alloc_polyg = (Polygon) malloc(npolyg * sizeof(struct polygon));
    if (alloc_polyg == NULL)
      mwerror(FATAL, 1, "Not enough memory for polygon buffer!\n");
    selectarea->first = alloc_polyg;
    ptr_polyg = selectarea->first;
    ptr_polyg->previous = NULL;
    for (p = 1; p < npolyg; p++) {
      ptr_polyg->next = ptr_polyg + 1;
      ptr_polyg++;
      ptr_polyg->previous = ptr_polyg - 1;
    }
    ptr_polyg->next = NULL;

    /*--- Decode polygons ---*/ 

    npoints_total = 0;
    for (p = 1; p <= npolyg; p++) {

      /*--- Decode number of vertices in polygons ---*/

      DECODE_INT(&c, 1 << (NBIT_NPOINTS - 1), compress);
      npoints[p] = c + 1;
      npoints_total += c + 1;
    }

    /*--- Memory allocation for vertices ---*/

    alloc_point = (Point_curve) malloc(npoints_total * sizeof(struct point_curve));
    if (alloc_point == NULL)
      mwerror(FATAL, 1, "Not enough memory for polygon vertices!\n");

    /*--- Decode vertices of polygons ---*/ 
      
    ptr_polyg = selectarea->first;
    ptr_point = alloc_point;
    ptr_polyg->first = ptr_point;
    for (p = 1; p <= npolyg; p++) {
      ptr_point = ptr_polyg->first;
      ptr_point->previous = NULL;
      for (v = 1; v < npoints[p]; v++) {
	ptr_point->next = ptr_point + 1;
	ptr_point++;
	ptr_point->previous = ptr_point - 1;
      }
      ptr_point->next = NULL;
      if (ptr_polyg->next)
	nextp_point = ptr_point + 1;

      /*--- Decode vertices of one polygon ---*/ 
      
      ptr_point = ptr_polyg->first;
      while (ptr_point) {
	DECODE_INT(&c, 1 << (NBIT_SIZEIM - 1), compress);
	ptr_point->x = c;
	DECODE_INT(&c, 1 << (NBIT_SIZEIM - 1), compress);
	ptr_point->y = c;
	ptr_point = ptr_point->next;
      }
	
      ptr_polyg = ptr_polyg->next;
      if (ptr_polyg) 
	ptr_polyg->first = nextp_point;
      
    }
  } else
    selectarea = NULL;
  
  stop_test = TRUE;
  stop_test_area[BG_SYMB] = TRUE;
  for (p = 1; p <= npolyg; p++)
    stop_test_area[p] = TRUE;
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



static int
DECODE_SYMBOL(cum_freq)

long           cum_freq[];

{
  int          symbol;
  int          cum;
  long         range;

  /*--- find symbol ---*/

  range = (long) high - low + 1;
  cum = (((long) (value - low) + 1) * cum_freq[0] - 1) / range;
  for (symbol = 1; cum_freq[symbol] > cum; symbol++);
  high = low + (range * cum_freq[symbol - 1]) / cum_freq[0] - 1;
  low = low + (range * cum_freq[symbol]) / cum_freq[0];

  for (;;) {
    if (high < half) {
    } else
      if (low >= half) {
	value -= half;
	low -= half;
	high -= half;
      } else
	if ((low >= first_qrt) && (high < third_qrt)) {
	  value -= first_qrt;
	  low -= first_qrt;
	  high -= first_qrt;
	} else
	  break;

    low = 2 * low;
    high = 2 * high + 1;
    value = 2 * value + READ_BIT();
  }

  return(symbol);
}




static void
MODIFY_CHILDREN(j, i, x, sigmap, output)

int              i;               /* Direction of sub-image 
				   * in wavelet transform */
int              j;               /* Level of sub-image 
				   * in wavelet transform */
long             x;               /* Buffer index for current coefficient */
Wtrans2d         output;
unsigned char ***sigmap;

{
  long            size;             /* Size of subimages */
  long            xc;               /* Buffer index for children 
				     * current coefficient */
  int             rc, cc;
  long            ncol, nrow;

  nrow = output->images[j][i]->nrow;
  ncol = output->images[j][i]->ncol;
  size = nrow * ncol;
  rc = (x / ncol);
  cc = (x % ncol);
  xc = rc  * 2 * output->images[j - 1][i]->ncol + cc * 2;
  sigmap[j-1][i][xc] = ZTR_SYMB;
  if (rc == nrow - 1) {

    if (nrow << 1 < output->images[j - 1][i]->nrow) {
      sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol] = sigmap[j - 1][i][xc + 2 * output->images[j - 1][i]->ncol] = ZTR_SYMB;
      if (cc == ncol - 1) {
	if (ncol << 1 < output->images[j - 1][i]->ncol) {
	  sigmap[j-1][i][xc+1] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 1] = sigmap[j - 1][i][xc + 2 * output->images[j - 1][i]->ncol + 1] = ZTR_SYMB;
	  sigmap[j-1][i][xc+2] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 2] = sigmap[j - 1][i][xc + 2 * output->images[j - 1][i]->ncol + 2] = ZTR_SYMB;
	} else
	  if (cc << 1 < output->images[j - 1][i]->ncol - 1) 
	    sigmap[j-1][i][xc+1] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 1] = sigmap[j - 1][i][xc + 2 * output->images[j - 1][i]->ncol + 1] = ZTR_SYMB;
      } else
	sigmap[j-1][i][xc+1] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 1] = sigmap[j - 1][i][xc + 2 * output->images[j - 1][i]->ncol + 1] = ZTR_SYMB;
    } else

      if (rc << 1 < output->images[j - 1][i]->nrow - 1) {
	sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol] = ZTR_SYMB;
	if (cc == ncol - 1) {
	  if (ncol << 1 < output->images[j - 1][i]->ncol) {
	    sigmap[j-1][i][xc+1] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 1] = ZTR_SYMB;
	    sigmap[j-1][i][xc+2] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 2] = ZTR_SYMB;
	  } else
	    if (cc << 1 < output->images[j - 1][i]->ncol - 1) 
	      sigmap[j-1][i][xc+1] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 1] = ZTR_SYMB;
	} else
	  sigmap[j-1][i][xc+1] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 1] = ZTR_SYMB;
      } else

	if (cc == ncol - 1) {
	  if (ncol << 1 < output->images[j - 1][i]->ncol) {
	    sigmap[j-1][i][xc+1] = ZTR_SYMB;
	    sigmap[j-1][i][xc+2] = ZTR_SYMB;
	  } else
	    if (cc << 1 < output->images[j - 1][i]->ncol - 1) 
	      sigmap[j-1][i][xc+1] = ZTR_SYMB;
	} else
	    sigmap[j-1][i][xc+1] = ZTR_SYMB;
	
  } else
    {
      sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol] = ZTR_SYMB;
      if (cc == ncol - 1) {
	if (ncol << 1 < output->images[j - 1][i]->ncol) {
	  sigmap[j-1][i][xc+1] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 1] = ZTR_SYMB;
	  sigmap[j-1][i][xc+2] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 2] = ZTR_SYMB;
	} else
	  if (cc << 1 < output->images[j - 1][i]->ncol - 1) 
	    sigmap[j-1][i][xc+1] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 1] = ZTR_SYMB;
      } else
	sigmap[j-1][i][xc+1] = sigmap[j - 1][i][xc + output->images[j - 1][i]->ncol + 1] = ZTR_SYMB;
    }

}




static void
DECODE_SIGNIF_MAP(output, sigmap, areamap, nsignif, thres, printfull)

Wtrans2d       output;              /* Reconstructed wavelet transform */
unsigned char ***sigmap;            /* Map of significance information */
unsigned char ***areamap;           /* Mask for selected areas */
long          *nsignif;             /* Number of sgnificant coefficients */
float          thres;               /* Current threshold */
int           *printfull;           /* Flag for information printing */

{
  int             i;                /* Direction of sub-image 
				     * in wavelet transform */
  int             j;                /* Level of sub-image 
				     * in wavelet transform */
  int             nrec;             /* Number of level in output */
  register float *ptro;             /* Pointer to wavelet coeff. in output */
  register unsigned char *ptrs;     /* Pointer to significance map */
  register unsigned char *ptra;     /* Pointer to area mask */
  long            x, xc;            /* Buffer index for current coefficient 
				     * and its children */
  long            size;             /* Size of subimages */
  int             rc, cc;
  long            ncol, nrow;
  double          quant_step;
  long            count_sigpos, count_signeg, count_ztroot, count_isolz;
  long            count_total;
  double          rate;
  int             z;
  long            freq[7];
  long            cum_freq[7];
  int             nsymbol;
  unsigned char   old_area_index;   /* Area index before redrawing */
  unsigned char   EOP_symb;         /* End of pass symbol */
  unsigned char   EOA_symb;         /* End of area symbol */
  unsigned char   symbol;           /* Binary symbol for quantization 
				     * refinement */
  double          sizei;

  sizei = (double) output->ncol * output->nrow;
  nrec = output->nlevel;
  quant_step = (double) 1.5 * thres;

  /*--- Initialize model for arithmetic decoding ---*/

  nsymbol = 5;
  for (z = 0; z <= nsymbol + 1; z++) {
    freq[z] = 1;
    cum_freq[z] = nsymbol + 1 - z;
  }
  freq[0] = 0;
  EOA_symb = nsymbol;
  EOP_symb = nsymbol + 1;

  /*--- Initialize decoding ---*/
  
  low = 0;
  high = top_value;
  value = 0;
  ptrc = ptrc2;
  ptrc3 = ptrc2;
  for (x = 1; x <= code_value_bits; x++) 
    value = 2 * value + READ_BIT();
  ptrc2 = ptrc3;

  count_sigpos = count_signeg = count_ztroot = count_isolz = 0;

  quant_step = 1.5 * thres;
  nrow = output->images[nrec][0]->nrow;
  ncol = output->images[nrec][0]->ncol;
  size = nrow * ncol;
  ptro = output->images[nrec][0]->gray;
  ptrs = sigmap[nrec][0];
  ptra = areamap[nrec][0];
  for (x = 0; (x < size) && (stop_test == TRUE); x++, ptro++, ptrs++, ptra++) {
    if (stop_test_area[*ptra] == TRUE) {
      if (*ptrs == ZTR_SYMB) {

	/*--- Coefficient was zerotree root ---*/

	old_area_index = *ptra;
	symbol = DECODE_SYMBOL(cum_freq);
	if (symbol == EOA_symb) {

	  /*--- Stop decoding area ---*/

	  stop_test_area[*ptra] = FALSE;
	  UPDATE_MODEL(symbol, cum_freq, freq, nsymbol);
	  TEST_END_DECODING();
	  if (stop_test == TRUE) {
	    REDRAW_AREAMAP(output, nrec, areamap, *ptra);
	    if (old_area_index != *ptra) 
	      symbol = DECODE_SYMBOL(cum_freq);
	  } 
	}
	    
	if ((symbol != EOA_symb) || (old_area_index != *ptra)) {

	  *ptrs = symbol;
	  UPDATE_MODEL(*ptrs, cum_freq, freq, nsymbol);

	  if (*ptrs == POS_SYMB) { 

	    /*--- Coefficient is significant positive ---*/

	    *ptro = quant_step;
	    (*nsignif)++;
	    count_sigpos++;
	    rc = x / ncol;
	    cc = x % ncol;
	    if (rc < output->images[nrec][1]->nrow) {
	      xc = rc * output->images[nrec][1]->ncol + cc;
	      sigmap[nrec][1][xc] = ZTR_SYMB;
	    }
	    if (cc < output->images[nrec][2]->ncol) {
	      xc = rc * output->images[nrec][2]->ncol + cc;
	      sigmap[nrec][2][xc] = ZTR_SYMB;
	    
	      if (rc < output->images[nrec][3]->nrow) 
		sigmap[nrec][3][xc] = ZTR_SYMB;
	    }
	  } else
	    if (*ptrs == NEG_SYMB) {

	      /*--- Coefficient is significant negative ---*/

	      *ptro = - quant_step;
	      (*nsignif)++;
	      count_signeg++;
	      rc = x / ncol;
	      cc = x % ncol;
	      if (rc < output->images[nrec][1]->nrow) {
		xc = rc * output->images[nrec][1]->ncol + cc;
		sigmap[nrec][1][xc] = ZTR_SYMB;
	      }
	      if (cc < output->images[nrec][2]->ncol) {
		xc = rc * output->images[nrec][2]->ncol + cc;
		sigmap[nrec][2][xc] = ZTR_SYMB;
	    
		if (rc < output->images[nrec][3]->nrow) 
		  sigmap[nrec][3][xc] = ZTR_SYMB;
	      }
	    } else
	      if (*ptrs == ISO_SYMB) {

		/*--- Coefficient is isolated zero ---*/

		count_isolz++;
		rc = x / ncol;
		cc = x % ncol;
		if (rc < output->images[nrec][1]->nrow) {
		  xc = rc * output->images[nrec][1]->ncol + cc;
		  sigmap[nrec][1][xc] = ZTR_SYMB;
		}
		if (cc < output->images[nrec][2]->ncol) {
		  xc = rc * output->images[nrec][2]->ncol + cc;
		  sigmap[nrec][2][xc] = ZTR_SYMB;
	    
		  if (rc < output->images[nrec][3]->nrow) 
		    sigmap[nrec][3][xc] = ZTR_SYMB;
		}
	      } else
		count_ztroot++;
	}	  

      } else
	if (*ptrs == ISO_SYMB) {

	  /*--- Coefficient was isolated zero ---*/
	
	  symbol = DECODE_SYMBOL(cum_freq);
	  if (symbol == EOA_symb) {
	    
	    /*--- Stop decoding area ---*/

	    stop_test_area[*ptra] = FALSE;
	    UPDATE_MODEL(symbol, cum_freq, freq, nsymbol);
	    TEST_END_DECODING();	  
	    if (stop_test == TRUE) {
	      old_area_index = *ptra;
	      REDRAW_AREAMAP(output, nrec, areamap, *ptra);
	      if (old_area_index != *ptra) 
		symbol = DECODE_SYMBOL(cum_freq);
	    } 
	  }
	    
	  if ((symbol != EOA_symb) || (old_area_index != *ptra)) {

	    *ptrs = symbol;
	    UPDATE_MODEL(*ptrs, cum_freq, freq, nsymbol);

	    if (*ptrs == POS_SYMB) { 

	      /*--- Coefficient is significant positive ---*/

	      *ptro = quant_step;
	      (*nsignif)++;
	      count_sigpos++;
	    } else
	      if (*ptrs == NEG_SYMB) {

		/*--- Coefficient is significant negative ---*/

		*ptro = - quant_step;
		(*nsignif)++;
		count_signeg++;
	      } else
		if (*ptrs == ISO_SYMB) {

		  /*--- Coefficient is isolated zero ---*/
	      
		  count_isolz++;
		} else
		  mwerror(WARNING, 1, "Something wrong with decoding of significance map :\nValue %d for isolated zero (j = %d, i = 0, x = %d, nrow = %d, ncol = %d)\n", *ptrs, nrec, x, nrow, ncol);

	  }
	}
    }
  }
  
  
  for (j = nrec; j >= 1; j--)
    for (i=1; i<=3; i++) {
      nrow = output->images[j][i]->nrow;
      ncol = output->images[j][i]->ncol;
      size = nrow * ncol;
      ptro = output->images[j][i]->gray;
      ptrs = sigmap[j][i];
      ptra = areamap[j][i];
      for (x = 0; (x < size) && (stop_test == TRUE); x++, ptro++, ptrs++, ptra++) {
	if (stop_test_area[*ptra] == TRUE) {
	  if (*ptrs == ZTR_SYMB) {

	    /*--- Coefficient was zerotree root ---*/

	    old_area_index = *ptra;
	    symbol = DECODE_SYMBOL(cum_freq);
	    if (symbol == EOA_symb) {
	      
	      /*--- Stop decoding area ---*/

	      stop_test_area[*ptra] = FALSE;
	      UPDATE_MODEL(symbol, cum_freq, freq, nsymbol);
	      TEST_END_DECODING();	  
	      if (stop_test == TRUE) {
		REDRAW_AREAMAP(output, nrec, areamap, *ptra);
		if (old_area_index != *ptra) 
		  symbol = DECODE_SYMBOL(cum_freq);
	      } 
	    }
	    
	    if ((symbol != EOA_symb) || (old_area_index != *ptra)) {

	      *ptrs = symbol;
	      UPDATE_MODEL(*ptrs, cum_freq, freq, nsymbol);
	    

	      if (*ptrs == POS_SYMB) { 

		/*--- Coefficient is significant positive ---*/

		*ptro = quant_step;
		(*nsignif)++;
		count_sigpos++;
		if (j > 1) 
		  MODIFY_CHILDREN(j, i, x, sigmap, output);
	      } else
		if (*ptrs == NEG_SYMB) {

		  /*--- Coefficient is significant negative ---*/

		  *ptro = - quant_step;
		  (*nsignif)++;
		  count_signeg++;
		  if (j > 1) 
		    MODIFY_CHILDREN(j, i, x, sigmap, output);
		} else
		  if (*ptrs == ISO_SYMB) {

		    /*--- Coefficient is isolated zero ---*/

		    count_isolz++;
		    if (j > 1) 
		      MODIFY_CHILDREN(j, i, x, sigmap, output);
		  } else
		    count_ztroot++;
	    }
	    
	  } else
	    if (*ptrs == ISO_SYMB) {

	      /*--- Coefficient was isolated zero ---*/
	
	      old_area_index = *ptra;
	      symbol = DECODE_SYMBOL(cum_freq);
	      if (symbol == EOA_symb) {
		
		/*--- Stop decoding area ---*/

		stop_test_area[*ptra] = FALSE;
		UPDATE_MODEL(symbol, cum_freq, freq, nsymbol);
		TEST_END_DECODING();	  
		if (stop_test == TRUE) {
		  REDRAW_AREAMAP(output, nrec, areamap, *ptra);
		  if (old_area_index != *ptra) 
		    symbol = DECODE_SYMBOL(cum_freq);
		} 
	      }
	    
	      if ((symbol != EOA_symb) || (old_area_index != *ptra)) {

		*ptrs = symbol;
		UPDATE_MODEL(*ptrs, cum_freq, freq, nsymbol);

		if (*ptrs == POS_SYMB) { 

		  /*--- Coefficient is significant positive ---*/

		  *ptro = quant_step;
		  (*nsignif)++;
		  count_sigpos++;
		} else
		  if (*ptrs == NEG_SYMB) {

		    /*--- Coefficient is significant negative ---*/

		    *ptro = - quant_step;
		    (*nsignif)++;
		    count_signeg++;
		  } else
		    if (*ptrs == ISO_SYMB) {

		      /*--- Coefficient is isolated zero ---*/
	      
		      count_isolz++;
		    } else
		      mwerror(WARNING, 1, "Something wrong with decoding of significance map :\nValue %d for isolated zero (j = %d, i = %d, x = %d, nrow = %d, ncol = %d)\n", *ptrs, j, i, x, nrow, ncol);
	      }
	    }	
	}
      }
    } 

  

  symbol = DECODE_SYMBOL(cum_freq);
  if (symbol != EOP_symb) 
    mwerror(WARNING, 1, "Something wrong with decoding of significance map : \nexpecting end of pass symbol (%d), getting %d.\n", EOP_symb, symbol);

  if (garbage_bits == 0)
    if (bits_to_go >= 2) {
      ncwread -= 2;
      bits_to_go -= 2;
    } else
      {
	ncwread -= 1;
	ptrc2++;
	bits_to_go += 6;
      }
  else
    if (garbage_bits <= 6) {
      ncwread -= 1;
      bits_to_go = 6 - garbage_bits;
    } else
      bits_to_go = 14 - garbage_bits;

  buffer = *ptrc2 >> (8 - bits_to_go);
  effnbitar -= code_value_bits - 2;

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
  effnbit += (long) rate + 1;
  if (count_total > 0)
    rate /= (double) count_total;

  if (printfull)
    printf("ZTR =%6d, IZ =%5d, RM = %.2f, ", count_ztroot, count_isolz, rate);
}



static void
DECODE_QUANT_STEP(output, sigmap, areamap, thres, printfull)

Wtrans2d       output;              /* Reconstructed wavelet transform */
unsigned char ***sigmap;            /* Map of significance information */
unsigned char ***areamap;           /* Mask for selected areas */
float          thres;               /* Current threshold */
int           *printfull;           /* Flag for information printing */

{
  int             i;                /* Index for orientation in wav. trans. */
  int             j;                /* Index for level in wav. trans. */
  int             nrec;             /* Number of level in output */
  register float *ptro;             /* Pointer to wavelet coeff. in output */
  register unsigned char *ptrs;     /* Pointer to significance map */
  register unsigned char *ptra;     /* Pointer to area mask */
  double          quant_step;
  long            x;                /* Buffer index for current coefficient */
  long            size;             /* Size of subimages */
  long            count_sigpos, count_signeg;
  long            count_total;
  double          rate;
  int             z;
  long            freq[5];
  long            cum_freq[5];
  int             nsymbol;
  unsigned char   old_area_index;   /* Area index before redrawing */
  unsigned char   EOP_symb;         /* End of pass symbol */
  unsigned char   EOA_symb;         /* End of area symbol */
  unsigned char   symbol;           /* Binary symbol for quantization 
				     * refinement */
  double          sizei;

  sizei = (double) output->ncol * output->nrow;
  nrec = output->nlevel;

  /*--- Initialize model for arithmetic coding ---*/

  nsymbol = 3;
  for (z=0; z<=nsymbol+1; z++) {
    freq[z] = 1;
    cum_freq[z] = nsymbol + 1 - z;
  }
  freq[0] = 0;
  EOA_symb = nsymbol;
  EOP_symb = nsymbol + 1;

  /*--- Initialize decoding ---*/
  
  ptrc = ptrc2;
  ptrc3 = ptrc2;
  low = 0;
  high = top_value;
  value = 0;
  for (x = 1; x <= code_value_bits; x++) 
    value = 2 * value + READ_BIT();
  ptrc2 = ptrc3;

  count_sigpos = count_signeg = 0;

  quant_step = 0.5 * thres;
  size = output->images[nrec][0]->nrow * output->images[nrec][0]->ncol;
  ptro = output->images[nrec][0]->gray;
  ptrs = sigmap[nrec][0];
  ptra = areamap[nrec][0];
  for (x = 0; (x < size) && (stop_test == TRUE); x++, ptro++, ptrs++, ptra++) 
    if (stop_test_area[*ptra] == TRUE) {
      if (*ptrs == SIG_SYMB) {
      
	old_area_index = *ptra;
	symbol = DECODE_SYMBOL(cum_freq);
	if (symbol == EOA_symb) {
	  
	  /*--- Stop decoding area ---*/

	  stop_test_area[*ptra] = FALSE;
	  UPDATE_MODEL(symbol, cum_freq, freq, nsymbol);
	  TEST_END_DECODING();	  
	  if (stop_test == TRUE) {
	    REDRAW_AREAMAP(output, nrec, areamap, *ptra);
	    if (old_area_index != *ptra) 
	      symbol = DECODE_SYMBOL(cum_freq);
	  } 
	}
	    
	if ((symbol != EOA_symb) || (old_area_index != *ptra)) {

	  if (symbol == 1) {
	    *ptro += quant_step;
	    count_sigpos++;
	  } else
	    {
	      *ptro -= quant_step;
	      count_signeg++;
	    }
	  UPDATE_MODEL(symbol, cum_freq, freq, nsymbol);
	}
      } else
	if (*ptrs >= POS_SYMB)
	  *ptrs = SIG_SYMB;
    
    }
  
  for (j = nrec; j >= 1; j--)
    for (i=1; i<=3; i++) {
      size = output->images[j][i]->nrow * output->images[j][i]->ncol;
      ptro = output->images[j][i]->gray;
      ptrs = sigmap[j][i];
      ptra = areamap[j][i];
      for (x = 0; (x < size) && (stop_test == TRUE); x++, ptro++, ptrs++, ptra++) 
	if (stop_test_area[*ptra] == TRUE) {
	  if (*ptrs == SIG_SYMB) {
      
	    old_area_index = *ptra;
	    symbol = DECODE_SYMBOL(cum_freq);
	    if (symbol == EOA_symb) {

	      /*--- Stop decoding area ---*/

	      stop_test_area[*ptra] = FALSE;
	      UPDATE_MODEL(symbol, cum_freq, freq, nsymbol);
	      TEST_END_DECODING();	  
	      if (stop_test == TRUE) {
		REDRAW_AREAMAP(output, nrec, areamap, *ptra);
		if (old_area_index != *ptra) 
		  symbol = DECODE_SYMBOL(cum_freq);
	      } 
	    }
	    
	    if ((symbol != EOA_symb) || (old_area_index != *ptra)) {
    
	      if (symbol == 1) {
		*ptro += quant_step;
		count_sigpos++;
	      } else
		{
		  *ptro -= quant_step;
		  count_signeg++;
		}
	      UPDATE_MODEL(symbol, cum_freq, freq, nsymbol);
	    }
	  } else
	    if (*ptrs >= POS_SYMB)
	      *ptrs = SIG_SYMB;
  	
	}
    } 

  symbol = DECODE_SYMBOL(cum_freq);
  if (symbol != EOP_symb) 
    mwerror(WARNING, 1, "Something wrong with decoding of quantization steps :\nexpecting end of pass symbol (%d), getting %d.\n", EOP_symb, symbol);
  if (garbage_bits == 0)
    if (bits_to_go >= 2) {
      ncwread -= 2;
      bits_to_go -= 2;
    } else
      {
	ncwread -= 1;
	ptrc2++;
	bits_to_go += 6;
      }
  else
    if (garbage_bits <= 6) {
      ncwread -= 1;
      bits_to_go = 6 - garbage_bits;
    } else
      bits_to_go = 14 - garbage_bits;

  buffer = *ptrc2 >> (8 - bits_to_go);
  effnbitar -= code_value_bits - 2;

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
  effnbit += (long) rate + 1;
  if (count_total > 0)
    rate /= (double) count_total;
  if (printfull)
    printf("RQ = %.2f,   ", rate); 
}




static void
ZERO_TREE_DECODE(compress, output, thres, printfull)

Cimage       compress;            /* Compressed buffer */
Wtrans2d     output;              /* Reconstructed wavelet transform */
float        thres;               /* Initial threshold */
int         *printfull;           /* Flag for information printing */

{
  int             i;                /* Index for orientation in wav. trans. */
  int             j;                /* Index for level in wav. trans. */
  int             nrec;             /* Number of level to consider in wavelet 
				     * transform */
  unsigned char  ***sigmap;         /* Map of significance information */
  unsigned char  ***areamap;        /* Mask for selected areas */
  register float *ptro;             /* Pointer to wavelet coeff. in output */
  register unsigned char *ptrs;     /* Pointer to significance map */
  long            x;                /* Buffer index for current coefficient */
  long            size;             /* Size of subimages */
  long            nsignif;          /* Number of significant coefficients */
  float           effrate;          /* Effective rate */
  
  if (printfull)
    printf("Thres = %.2f\n", thres);

  nrec = output->nlevel;

    /*--- Init quantized image ---*/

  size = output->images[nrec][0]->nrow * output->images[nrec][0]->ncol;
  ptro = output->images[nrec][0]->gray;
  for (x = 0; x < size; x++, ptro++) 
    *ptro = 0.0;
  
  for (j = nrec; j >= 1; j--)
    for (i=1; i<=3; i++) {
      size = output->images[j][i]->nrow * output->images[j][i]->ncol;
      ptro = output->images[j][i]->gray;
      for (x = 0; x < size; x++, ptro++) 
	*ptro = 0.0;
    } 

    /*--- Memory allocation for significance map ---*/

  sigmap = (unsigned char ***) malloc((int) (nrec + 1) * sizeof(unsigned char **));
  if (sigmap == NULL)
    mwerror(FATAL, 1, "Allocation for significance map refused!\n");

  INIT_SIGMAP(output, nrec, sigmap);

    /*--- Memory allocation and initialisation for selected area mask ---*/

  areamap = (unsigned char ***) malloc((int) (nrec + 1) * sizeof(unsigned char **));
  if (areamap == NULL)
    mwerror(FATAL, 1, "Allocation for significance map refused!\n");

  INIT_AREAMAP(output, nrec, areamap);

  /*--- Decoding ---*/

  nsignif = 0;
  while (stop_test == TRUE) {
    
    DECODE_SIGNIF_MAP(output, sigmap, areamap, &nsignif, thres, printfull);

    if (stop_test == TRUE)
      DECODE_QUANT_STEP(output, sigmap, areamap, thres, printfull);

    thres /= 2.0;

    if (printfull) {
      size = output->nrow * output->ncol;
      effrate = (double) effnbit / size; 
      printf("Rate = %.4f, Rate AR = %.4f,  nsignif = %d\n", effrate, (double) effnbitar / size, nsignif);
    }
  }

  compress->firstrow = bits_to_go;
  compress->firstcol = ncwread;
  if (bits_to_go == 0)
    compress->firstcol += 1;

    /*--- Desallocation for significance map ---*/

  free(sigmap[nrec][0]);
  for (j = nrec; j >= 1; j--)
    for (i = 3; i >= 1; i--)
      free(sigmap[j][i]);
  for (j = nrec; j >= 1; j--)
    free(sigmap[j]);
  free(sigmap);

    /*--- Desallocation for area map ---*/

  free(areamap[nrec][0]);
  for (j = nrec; j >= 1; j--)
    for (i = 3; i >= 1; i--)
      free(areamap[j][i]);
  for (j = nrec; j >= 1; j--)
    free(areamap[j]);
  free(areamap);

}



void
iezw(PrintFull, WeightFac, Compress, Output)

int            *PrintFull;          /* Print full set of information */
float          *WeightFac;          /* Weighting factor for wavelet coeff. */
Cimage          Compress;           /* input compressed file */
Wtrans2d        Output;             /* Reconstructed wavelet transform */

{
  int             NRec;
  int             NRow, NCol;
  float           Threshold;

  /*--- Read Header ---*/

  READ_HEADER(Compress, &NRec, &NRow, &NCol, &Threshold);


  /*--- Memory allocation for quantized wavelet transform ---*/

  if (mw_alloc_ortho_wtrans2d(Output, NRec, NRow, NCol) == NULL)
    mwerror(FATAL, 1, "Allocation of buffer for wavelet transform refused!\n");

  REALLOCATE_SUBIMAGES(Output);


  /*--- Quantize and encode wavelet transform Wtrans ---*/

  ZERO_TREE_DECODE(Compress, Output, Threshold, PrintFull); 

  /*--- Weight wavelet coefficients according to sub-image ---*/

  if (WeightFac)
    if (*WeightFac > 0.0)
      SCALE_WAVT(Output, NRec, 1.0 / *WeightFac);

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   wtrans2d.h
   
   Vers. 1.0
   (C) 1993 Jacques Froment
   Internal Input/Output wtrans2d structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef wtrans2d_flg
#define wtrans2d_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "fimage.h"

/* Maximum Number of levels in a Wavelet Decomposition */
/* # 0 is the original image */

#ifndef mw_max_nlevel
#define mw_max_nlevel 20
#endif

/* Maximum Number of orientations (directions) in a Wavelet Decomposition */
#define mw_max_norient 5

/* Maximum Number of filter file names (prefixs only) */
#define mw_max_nfilter 6

/* Meaning of the orientation # */

#define mw_sample 0  /* Low-pass images */

/* Orthogonal, biorthogonal and dyadic cases */
#define mw_horizontal 1
#define mw_vertical 2

/* Orthogonal/biorthogonal cases only */
#define mw_diagonal 3

/* Dyadic case (polar coordinates representation) */
#define mw_magnitude 3
#define mw_argument 4

/* Types of the wtrans2d performed */

#define mw_orthogonal 1
#define mw_biorthogonal 2
#define mw_dyadic 3

/* Types of the edges statments */

#define mw_edges_zeropad 1  /* image is Zero-padded (no statment) */
#define mw_edges_periodic 2 /* image is made periodic */
#define mw_edges_mirror 3   /* mirror effect */
#define mw_edges_wadapted 4 /* adapted wavelet on edges */

/* 2-Dimensional wavelet representation */

typedef struct wtrans2d {

  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the wtrans2d */

  int type;  /* Type of the wtrans2d performed */
  int edges; /* Type of the edges statments */

  /* Define the Names (prefix) of the filters used */
  /* Orthogonal case: 1 Fsignal "name.ri" */
  /* Biorthogonal case: 2 Fsignal "name1.ri" & "name2.ri" */
     /* if filters for edges, 1 or 2 Fimage with same prefix "name"         */ 
     /* and extension "edge" instead of "ri".                               */
  /* Dyadic case: 6 Fsignal H1,G1,K1,H2,G2,K2 */  

  char filter_name[mw_namesize][mw_max_nfilter];

  int nrow;
  int ncol; /* Size of the original image which is in images[0][0] */
  
  /* images[j][d] is the wavelet decomposition at the level #j */
  /* (i.e. at the scale 2^j in the dyadic case) and at the     */
  /* direction d of the original image images[0][0].           */

  Fimage images[mw_max_nlevel+1][mw_max_norient+1];

  int nlevel;   /* Number of levels for this decomposition */
  int norient;  /* Number of orientations for this decomposition */
  int nfilter;  /* Number of filters used to compute the decomposition */

} *Wtrans2d;


/* Functions definition */

#ifdef __STDC__

Wtrans2d mw_new_wtrans2d(void);
void *mw_alloc_ortho_wtrans2d(Wtrans2d,int,int,int);
void *mw_alloc_biortho_wtrans2d(Wtrans2d,int,int,int);
void *mw_alloc_dyadic_wtrans2d(Wtrans2d,int,int,int);
void mw_delete_wtrans2d(Wtrans2d);

#else

Wtrans2d mw_new_wtrans2d();
void *mw_alloc_ortho_wtrans2d();
void *mw_alloc_biortho_wtrans2d();
void *mw_alloc_dyadic_wtrans2d();
void mw_delete_wtrans2d();


#endif

#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   wtrans1d.h
   
   Vers. 1.1
   (C) 1993-95 Jacques Froment
   Internal Input/Output wtrans1d structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef wtrans1d_flg
#define wtrans1d_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "fsignal.h"

/* Maximum Number of levels (octaves) in a Wavelet Decomposition */
/* # 0 is the original image */

#ifndef mw_max_nlevel
#define mw_max_nlevel 20
#endif

/* Maximum Number of voices per octave in a Wavelet Decomposition */

#define mw_max_nvoice 50

/* Maximum Number of filter file names (prefixs only) */
#define mw_max_nfilter_1d 4

/* Types of the wtrans1d performed */

#ifndef mw_orthogonal
#define mw_orthogonal 1
#endif
#ifndef mw_biorthogonal
#define mw_biorthogonal 2
#endif
#ifndef mw_dyadic
#define mw_dyadic 3
#endif
#ifndef mw_continuous
#define mw_continuous 4
#endif

/* Types of the edges statments */

#ifndef mw_edges_zeropad
#define mw_edges_zeropad 1  /* image is Zero-padded (no statment) */
#endif
#ifndef mw_edges_periodic
#define mw_edges_periodic 2 /* image is made periodic */
#endif
#ifndef mw_edges_mirror
#define mw_edges_mirror 3   /* mirror effect */
#endif
#ifndef mw_edges_wadapted
#define mw_edges_wadapted 4 /* adapted wavelet on edges */
#endif

/* 1-Dimensional wavelet representation */

typedef struct wtrans1d {

  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the wtrans1d */

  int type;  /* Type of the wtrans1d performed */
  int edges; /* Type of the edges statments */

  /* Define the Names (prefix) of the filters used */
  /* Orthogonal case: 1 Fsignal "name.ri" */
  /* Biorthogonal case: 2 Fsignal "name1.ri" & "name2.ri" */
  /* Dyadic case: 4 Fsignal H1,G1,H2,G2 */  

  char filter_name[mw_namesize][mw_max_nfilter_1d];

  int size; /* Size of the original signal which is in average[0][0] */

  /* Average or low-pass signals. A[l][v] is the low-pass       */
  /* signal at the octave #l and at the voice #v, that is the   */
  /* signal at the scale 2^(l+v/nvoice).                        */
  /* If the wavelet is complex, A represents the modulus only.  */

  Fsignal A[mw_max_nlevel+1][mw_max_nvoice];

  /* Phase of the average.                                      */
  /* Used in case of complex wavelet only.                      */

  Fsignal AP[mw_max_nlevel+1][mw_max_nvoice];

  /* Detail or wavelet coefficients signals.                    */
  /* D[l][v] is the difference signal at the octave #l and at   */
  /* the voice #v. If the wavelet is complex, D represents the  */
  /* modulus value only. Use DP to get the phase.               */

  Fsignal D[mw_max_nlevel+1][mw_max_nvoice];

  /* Phase of the Detail or wavelet coefficients signals.       */
  /* Used in case of complex wavelet only.                      */

  Fsignal DP[mw_max_nlevel+1][mw_max_nvoice];

  int nlevel;   /* Number of levels (octave) for this decomposition */
  int nvoice;   /* Number of voices per octave for this decomposition */

  int complex;  /* 1 if the wavelet is complex that is, if P[][] is used */
  int nfilter;  /* Number of filters used to compute the decomposition */

} *Wtrans1d;


/* Functions definition */


#ifdef __STDC__

Wtrans1d mw_new_wtrans1d(void);
void *mw_alloc_ortho_wtrans1d(Wtrans1d,int,int);
void *mw_alloc_biortho_wtrans1d(Wtrans1d,int,int);
void *mw_alloc_dyadic_wtrans1d(Wtrans1d,int,int);
void *mw_alloc_continuous_wtrans1d(Wtrans1d,int,int,int,int);
void mw_delete_wtrans1d(Wtrans1d);

#else

Wtrans1d mw_new_wtrans1d();
void *mw_alloc_ortho_wtrans1d();
void *mw_alloc_biortho_wtrans1d();
void *mw_alloc_dyadic_wtrans1d();
void *mw_alloc_continuous_wtrans1d();
void mw_delete_wtrans1d();

#endif

#endif

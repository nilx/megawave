/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   wmax2d.h
   
   Vers. 1.1
   (C) 1993-2000 Jacques Froment
   Internal Input/Output for the 2D Wavelet Maxima structures

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef wmax2d_flg
#define wmax2d_flg

#ifdef SunOS
#include <sys/types.h>
#endif
#include "wtrans2d.h"

/* Preset value for an unknown argument */
#define mw_not_an_argument 1.0e9

/* Preset value for an unknown magnitude */
#define mw_not_a_magnitude -1.0

 
/* ----- Virtual Maxima points & chains ----- */

/* A Virtual Maxima Point in a virtual chain */

typedef struct vpoint_wmax {
  int x,y;              /* Location of the point in the image */
  float mag[mw_max_nlevel]; /* mag[n] refers to magnitude at scale 2^(n+1) */
  float arg[mw_max_nlevel]; /* arg[n] refers to argument at scale 2^(n+1) */
  float argp;           /* Prediction, if any done */
  struct vpoint_wmax *previous, *next; /* link to others vpoints */
  } *Vpoint_wmax;

/* A Virtual Chain of Maxima points */

typedef struct vchain_wmax {
  int size;             /* number of vpoint in the vchain */
  Vpoint_wmax first;     /* Beginning of the vchain */
  struct vchain_wmax *previous,*next; /* link to others vchains */
} *Vchain_wmax;


/* Set of Virtual Chains */

typedef struct vchains_wmax {

  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the set */

  int size;               /* number of vchain in the vchains */
  int ref_level;          /* Reference level. That is, octave for which the */
                          /* location (x,y) corresponds really to maxima. */
                          /* Others are wavelet coefficients only. */
  int nlevel;             /* Total number of octaves in the wavelet decomp. */
  int nrow, ncol;         /* Size of the picture where (x,y) belong */
  struct vchain_wmax *first; /* First vchain_wmax */
} *Vchains_wmax;

/* ---------- */

/* Functions definition */

#ifdef __STDC__

Vpoint_wmax mw_new_vpoint_wmax(void);
Vpoint_wmax mw_change_vpoint_wmax(Vpoint_wmax);
void mw_delete_vpoint_wmax(Vpoint_wmax);
Vchain_wmax mw_new_vchain_wmax(void);
Vchain_wmax mw_change_vchain_wmax(Vchain_wmax);
void mw_delete_vchain_wmax(Vchain_wmax);
Vchains_wmax mw_new_vchains_wmax(void);
Vchains_wmax mw_change_vchains_wmax(Vchains_wmax);
void mw_delete_vchains_wmax(Vchains_wmax);
Vpoint_wmax mw_copy_vpoint_wmax(Vpoint_wmax,Vpoint_wmax);
Vchain_wmax mw_copy_vchain_wmax(Vchain_wmax, Vchain_wmax);
int mw_give_nlevel_vchain(Vchain_wmax);

#else

Vpoint_wmax mw_new_vpoint_wmax();
Vpoint_wmax mw_change_vpoint_wmax();
void mw_delete_vpoint_wmax();
Vchain_wmax mw_new_vchain_wmax();
Vchain_wmax mw_change_vchain_wmax();
void mw_delete_vchain_wmax();
Vchains_wmax mw_new_vchains_wmax();
Vchains_wmax mw_change_vchains_wmax();
void mw_delete_vchains_wmax();
Vpoint_wmax mw_copy_vpoint_wmax();
Vchain_wmax mw_copy_vchain_wmax();
int mw_give_nlevel_vchain();

#endif

#endif

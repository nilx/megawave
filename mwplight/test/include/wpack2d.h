/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   wpack2d.h
   
   Vers. 0.0
   Authors : Jacques Froment
   wpack2d internal type

   Versions history :
   v0.0 (JF): initial release (from Fpack)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef wpack2d_flg
#define wpack2d_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#define WPACK2D_DEFAULT_NAME "untitled"
#define WPACK2D_DEFAULT_CMT "?"

/* 2-Dimensional wavelet packets */

typedef struct wpack2d {
  
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the wpack2d */

  Fsignal signal1;     /* Impulse response of the filter 'h'*/
  Fsignal signal2;     /* Impulse response of the filter '\tilde h', for biorthogonal wavelet packet*/
  int level;          /* Decomposition level (calculated) */
  Cimage tree;        /* Decomposition tree */
  Fimage *images;     /* Array for output images (containing the wavelet packet coefficients) */
 
  int img_array_size; /* Number of elements in *images */
   
  int img_ncol; /*number of colums in the image before the decomposition*/
  int img_nrow; /*number of rows in the image before the decomposition*/

  struct wpack2d *previous; /* Pointer to the previous wpack2d (may be NULL) */ 
  struct wpack2d *next;     /* Pointer to the previous wpack2d (may be NULL) */
} *Wpack2d;



/* Functions definition */

#ifdef __STDC__

Wpack2d mw_new_wpack2d(void);
int  mw_checktree_wpack2d(Cimage);
Wpack2d mw_alloc_wpack2d(Wpack2d , Cimage, Fsignal, Fsignal, int, int);
void mw_delete_wpack2d(Wpack2d);
Wpack2d mw_change_wpack2d(Wpack2d, Cimage, Fsignal, Fsignal, int, int);
void mw_copy_wpack2d(Wpack2d , Wpack2d, int);
void mw_clear_wpack2d(Wpack2d , float);
void mw_prune_wpack2d(Wpack2d in, Wpack2d out, Cimage tree);


#else

Wpack2d mw_new_wpack2d();
int  mw_checktree_wpack2d();
Wpack2d mw_alloc_wpack2d();
void mw_delete_wpack2d();
Wpack2d mw_change_wpack2d();
void mw_copy_wpack2d();
void mw_clear_wpack2d();
void mw_prune_wpack2d();

#endif

#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   fimage.h
   
   Vers. 1.3
   (C) 1994-2000 Jacques Froment
   Internal Input/Output fimage structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef fimage_flg
#define fimage_flg

#ifdef SunOS
#include <sys/types.h>
#endif

/* Floating Point Gray Level Image */

typedef struct fimage {
  int nrow;        /* Number of rows (dy) */
  int ncol;        /* Number of columns (dx) */
  int allocsize;   /* Size allocated (in bytes) for the gray plane */
  float *gray;     /* The Gray level plane (may be NULL) */

  float scale;     /* Scale of the picture (should be 1 for original pict.) */
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the image */
  
  /* Defines the signifiant part of the picture : */
  int firstcol;    /* index of the first col not affected by left side effect*/
  int lastcol;     /* index of the last col not affected by right side effect*/
  int firstrow;    /* index of the first row not aff. by upper side effect */  
  int lastrow;     /* index of the last row not aff. by lower side effect */  

  /* For use in Movies only */
  struct fimage *previous; /* Pointer to the previous image (may be NULL) */
  struct fimage *next; /* Pointer to the next image (may be NULL) */

} *Fimage;



/* Functions definition */

#ifdef __STDC__

Fimage mw_new_fimage(void);
Fimage mw_alloc_fimage(Fimage, int, int);
void mw_delete_fimage(Fimage);
Fimage mw_change_fimage(Fimage, int, int);
float mw_getdot_fimage(Fimage, int, int);
void mw_plot_fimage(Fimage, int, int, float);
void mw_draw_fimage(Fimage, int, int, int, int, float);
void mw_clear_fimage(Fimage, float);
void mw_copy_fimage(Fimage, Fimage);
float ** mw_newtab_gray_fimage(Fimage);

#else

Fimage mw_new_fimage();
Fimage mw_alloc_fimage();
void mw_delete_fimage();
Fimage mw_change_fimage();
float mw_getdot_fimage();
void mw_plot_fimage();
void mw_draw_fimage();
void mw_clear_fimage();
void mw_copy_fimage();
float ** mw_newtab_gray_fimage();

#endif


#endif

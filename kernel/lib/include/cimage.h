/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   cimage.h
   
   Vers. 1.2
   (C) 1993-99 Jacques Froment
   Internal Input/Output cimage structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef cimage_flg
#define cimage_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "string_size.h"

/* Unsigned Char (byte) Gray Level Image */

typedef struct cimage {
  int nrow;        /* Number of rows (dy) */
  int ncol;        /* Number of columns (dx) */
  int allocsize;   /* Size allocated (in bytes) for the gray plane */
  unsigned char *gray;     /* The Gray level plane (may be NULL) */

  float scale;     /* Scale of the picture (should be 1 for original pict.) */
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the image */
  
  /* Defines the signifiant part of the picture : */
  int firstcol;    /* index of the first col not affected by left side effect*/
  int lastcol;     /* index of the last col not affected by right side effect*/
  int firstrow;    /* index of the first row not aff. by upper side effect */  
  int lastrow;     /* index of the last row not aff. by lower side effect */  

  /* For use in Movies only */
  struct cimage *previous; /* Pointer to the previous image (may be NULL) */
  struct cimage *next; /* Pointer to the next image (may be NULL) */

} *Cimage;

/* Functions definition */

#ifdef __STDC__

Cimage mw_new_cimage(void);
Cimage mw_alloc_cimage(Cimage, int, int);
void mw_delete_cimage(Cimage);
Cimage mw_change_cimage(Cimage, int, int);
unsigned char mw_getdot_cimage(Cimage, int, int);
void mw_plot_cimage(Cimage, int, int, unsigned char);
void mw_draw_cimage(Cimage, int, int, int, int, unsigned char);
void mw_clear_cimage(Cimage, unsigned char);
void mw_copy_cimage(Cimage, Cimage);
unsigned char ** mw_newtab_gray_cimage(Cimage);
unsigned char mw_isitbinary_cimage(Cimage);
#else

Cimage mw_new_cimage();
Cimage mw_alloc_cimage();
void mw_delete_cimage();
Cimage mw_change_cimage();
unsigned char mw_getdot_cimage();
void mw_plot_cimage();
void mw_draw_cimage();
void mw_clear_cimage();
void mw_copy_cimage();
unsigned char ** mw_newtab_gray_cimage();
unsigned char mw_isitbinary_cimage();
#endif

#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ccimage.h
   
   Vers. 1.1
   (C) 1994-97 Jacques Froment
   Internal Input/Output ccimage structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef ccimage_flg
#define ccimage_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "string_size.h"

/* Unsigned Char (byte) Color Image */

typedef struct ccimage {
  int nrow;        /* Number of rows (dy) */
  int ncol;        /* Number of columns (dx) */
  int allocsize;   /* Size allocated (in bytes) for the gray plane */

  unsigned char *red;     /* The red level plane (may be NULL) */
  unsigned char *green;   /* The green level plane (may be NULL) */
  unsigned char *blue;    /* The blue level plane (may be NULL) */

  float scale;     /* Scale of the picture (should be 1 for original pict.) */
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the image */
  
  /* Defines the signifiant part of the picture : */
  int firstcol;    /* index of the first col not affected by left side effect*/
  int lastcol;     /* index of the last col not affected by right side effect*/
  int firstrow;    /* index of the first row not aff. by upper side effect */  
  int lastrow;     /* index of the last row not aff. by lower side effect */  

  /* For use in Movies only */
  struct ccimage *previous; /* Pointer to the previous image (may be NULL) */
  struct ccimage *next; /* Pointer to the next image (may be NULL) */

} *Ccimage;

/* Functions definition */

#ifdef __STDC__

Ccimage mw_new_ccimage(void);
Ccimage mw_alloc_ccimage(Ccimage, int, int);
void mw_delete_ccimage(Ccimage);
Ccimage mw_change_ccimage(Ccimage, int, int);
void mw_getdot_ccimage(Ccimage, int, int, unsigned char *,unsigned char *,
		      unsigned char *);
void mw_plot_ccimage(Ccimage, int, int, unsigned char, unsigned char, 
		    unsigned char);
void mw_draw_ccimage(Ccimage, int, int, int, int, unsigned char, unsigned char,
		    unsigned char);
void mw_clear_ccimage(Ccimage, unsigned char, unsigned char, unsigned char);
void mw_copy_ccimage(Ccimage, Ccimage);
unsigned char ** mw_gettab_red_ccimage(Ccimage);
unsigned char ** mw_gettab_green_ccimage(Ccimage);
unsigned char ** mw_gettab_blue_ccimage(Ccimage);

#else

Ccimage mw_new_ccimage();
Ccimage mw_alloc_ccimage();
void mw_delete_ccimage();
Ccimage mw_change_ccimage();
void mw_getdot_ccimage();
void mw_plot_ccimage();
void mw_draw_ccimage();
void mw_clear_ccimage();
void mw_copy_ccimage();
unsigned char ** mw_newtab_red_ccimage();
unsigned char ** mw_newtab_green_ccimage();
unsigned char ** mw_newtab_blue_ccimage();
     
#endif

#endif

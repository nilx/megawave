/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   cfimage.h
   
   Vers. 1.04
   (C) 1994-2000 Jacques Froment
   Internal Input/Output cfimage structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef cfimage_flg
#define cfimage_flg

#ifdef SunOS
#include <sys/types.h>
#endif

/* Models of the colorimetric system */
#define MODEL_RGB 0
#define MODEL_YUV 1
/* Hue, Saturation, Intensity */
#define MODEL_HSI 2
/* More or less same than above but without trigonometric transform */
#define MODEL_HSV 3


/* Floating point Color Image */

typedef struct cfimage {
  int nrow;        /* Number of rows (dy) */
  int ncol;        /* Number of columns (dx) */
  int allocsize;   /* Size allocated (in bytes) for the gray plane */
  unsigned char model; /* Model of the colorimetric system */

  float *red;      /* The Red plane if model=MODEL_RGB (may be NULL) or U/H   */
  float *green;    /* The Green plane if model=MODEL_RGB (may be NULL) or Y/S */
  float *blue;     /* The Blue plane if model=MODEL_RGB (may be NULL) or V/I  */

  float scale;     /* Scale of the picture (should be 1 for original pict.) */
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the image */
  
  /* Defines the signifiant part of the picture : */
  int firstcol;    /* index of the first col not affected by left side effect*/
  int lastcol;     /* index of the last col not affected by right side effect*/
  int firstrow;    /* index of the first row not aff. by upper side effect */  
  int lastrow;     /* index of the last row not aff. by lower side effect */  

  /* For use in Movies only */
  struct cfimage *previous; /* Pointer to the previous image (may be NULL) */
  struct cfimage *next; /* Pointer to the next image (may be NULL) */

} *Cfimage;



/* Functions definition */

#ifdef __STDC__

Cfimage mw_new_cfimage(void);
Cfimage mw_alloc_cfimage(Cfimage, int, int);
void mw_delete_cfimage(Cfimage);
Cfimage mw_change_cfimage(Cfimage, int, int);
void mw_getdot_cfimage(Cfimage, int, int, float *,float *, float *);
void mw_plot_cfimage(Cfimage, int, int, float, float, float);
void mw_draw_cfimage(Cfimage, int, int, int, int, float, float, float);
void mw_clear_cfimage(Cfimage, float, float, float);
void mw_copy_cfimage(Cfimage, Cfimage);
float ** mw_newtab_red_cfimage(Cfimage);
float ** mw_newtab_green_cfimage(Cfimage);
float ** mw_newtab_blue_cfimage(Cfimage);

#else

Cfimage mw_new_cfimage();
Cfimage mw_alloc_cfimage();
void mw_delete_cfimage();
Cfimage mw_change_cfimage();
void mw_getdot_cfimage();
void mw_plot_cfimage();
void mw_draw_cfimage();
void mw_clear_cfimage();
void mw_copy_cfimage();
float ** mw_newtab_red_cfimage();
float ** mw_newtab_green_cfimage();
float ** mw_newtab_blue_cfimage();

#endif

#endif

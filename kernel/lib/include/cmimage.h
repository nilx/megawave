/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   cmimage.h
   
   Vers. 1.1
   (C) 1999 Jacques Froment

   Internal Input/Output for the following morphological structures
       Cmorpho_line
       Cfmorpho_line
       Cmimage 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef cmimage_flg
#define cmimage_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "string_size.h"

#ifndef mimage_flg
#include "mimage.h"
#endif

typedef struct color {
  unsigned char model;    /* model of the colorimetric system */
  float red;              /* the red value if model=MODEL_RGB */
  float green;            /* the green value if model=MODEL_RGB */
  float blue;             /* the blue value if model=MODEL_RGB */  
} Color;

/* One Cmorpho_line in the discrete grid */
/* This is the border of a connected component of the following morpho set   */
/* {(x,y) / minvalue <= u(x,y) <= maxvalue} for u a color image              */
/* When minvalue=maxvalue this is an iso line, when                          */
/* minvalue=-MORPHO_INFTY or when maxvalue=MORPHO_INFTY this is a level line */

typedef struct cmorpho_line {
  Point_curve first_point;/* Pointer to the first point of the cmorpho_line curve */
  Point_type first_type;  /* Pointer to the first Point_type */
  Color minvalue;         /* Minimum color value of this morpho line */
  Color maxvalue;         /* Maximum color value of this morpho line */
  unsigned char open;     /* 0 if the color morpho line is closed, opened otherwise */
  float data;             /* User-defined data field (saved) */
  void *pdata;            /* User-defined data field : pointer to something (not saved) */
 
  /* For use in Cmimage only */
  struct cmorpho_sets *cmorphosets;/* Pointer to the associated cmorpho sets */
  unsigned int num;               /* Morpho line number (range in the chain) */
  struct cmorpho_line *previous;  /* Pointer to the previous m.l. (may be NULL) */
  struct cmorpho_line *next;      /* Pointer to the next m.l. (may be NULL) */
} *Cmorpho_line;

/* One Cmorpho_line in the continuous plane */

typedef struct cfmorpho_line {
  Point_fcurve first_point;/* Pointer to the first point of the fmorpho_line curve */
  Point_type first_type;   /* Pointer to the first Point_type */
  Color minvalue;          /* Minimum color value of this morpho line */
  Color maxvalue;          /* Maximum color value of this morpho line */
  unsigned char open;      /* 0 if the morpho line is closed, opened otherwise */
  float data;             /* User-defined data field (saved) */
  void *pdata;            /* User-defined data field : pointer to something (not saved) */
 
  /* For use in Cmimage only */
  struct cfmorpho_line *previous; /* Pointer to the previous m.l. (may be NULL) */
  struct cfmorpho_line *next;     /* Pointer to the next m.l. (may be NULL) */
} *Cfmorpho_line;


/* One Cmorpho_set in the discrete grid */
/* This is one connected component of the following set  :                  */
/* {(x,y) / minvalue <= u(x,y) <= maxvalue} for u a color image             */
/* When minvalue=maxvalue this is an iso set (section), when                */
/* minvalue=-MORPHO_INFTY or when maxvalue=MORPHO_INFTY this is a level set */

typedef struct cmorpho_set {
  unsigned int num;      /* Morpho set number (range in the Morpho_sets struct.) */
  Segment first_segment; /* Pointer to the first segment of the morpho set */
  Segment last_segment;  /* Pointer to the last segment of the morpho set */  
  Color minvalue;        /* Minimum color value of this set */
  Color maxvalue;        /* Maximum color level value of this set */
  unsigned char stated;  /* 1 if this m.s. has already been stated, 0 otherwise */
  int area;              /* Area of the set (number of pixels belonging to this set) */
  struct cmorpho_sets *neighbor; /* Pointer to a chain of neighbor morpho sets (may be NULL)*/
} *Cmorpho_set;

/* A set of cmorpho sets. This structure is used to record a chain of cmorpho sets.
*/ 

typedef struct cmorpho_sets {
  Cmorpho_set cmorphoset;       /* Pointer to the current morpho set */
  struct cmorpho_sets *previous;/* Pointer to the previous morpho sets of the chain */
  struct cmorpho_sets *next;    /* Pointer to the next morpho sets of the chain */
  /* For use in Cmimage only */
  struct cmorpho_line *cmorpholine;  /* Pointer to the associated morpho line */
} *Cmorpho_sets;


/* Color Morphological image */

typedef struct cmimage {
  char cmt[mw_cmtsize];   /* Comments */
  char name[mw_namesize]; /* Name of the set */
  int nrow;               /* Number of rows (dy) */
  int ncol;               /* Number of columns (dx) */
  Color minvalue;         /* Minimal color value in the image */
  Color maxvalue;         /* Maximal color value in the image */
  Cmorpho_line first_ml;  /* Pointer to the first cmorpho line in the discrete grid */
  Cfmorpho_line first_fml;/* Pointer to the first morpho line in the continuous plane */
  Cmorpho_sets first_ms;   /* Pointer to the first morpho sets in the discrete grid */
} *Cmimage;

/* Functions definition */

#ifdef __STDC__

Cmorpho_line mw_new_cmorpho_line(void);
Cmorpho_line mw_change_cmorpho_line(Cmorpho_line);
void mw_delete_cmorpho_line(Cmorpho_line);
void mw_copy_cmorpho_line(Cmorpho_line,Cmorpho_line);
unsigned int mw_cmorpho_line_length(Cmorpho_line);
unsigned int mw_cmorpho_line_num(Cmorpho_line);

Cfmorpho_line mw_new_cfmorpho_line(void);
Cfmorpho_line mw_change_cfmorpho_line(Cfmorpho_line);
void mw_delete_cfmorpho_line(Cfmorpho_line);
void mw_copy_cfmorpho_line(Cfmorpho_line,Cfmorpho_line);
unsigned int mw_cfmorpho_line_length(Cfmorpho_line);

Cmorpho_set mw_new_cmorpho_set(void);
Cmorpho_set mw_alloc_cmorpho_set(Cmorpho_set,int);
Cmorpho_set mw_change_cmorpho_set(Cmorpho_set);
void mw_delete_cmorpho_set(Cmorpho_set);
void mw_copy_cmorpho_set(Cmorpho_set, Cmorpho_set);
unsigned int mw_cmorpho_set_length(Cmorpho_set);

Cmorpho_sets mw_new_cmorpho_sets(void);
Cmorpho_sets mw_change_cmorpho_sets(Cmorpho_sets);
void mw_delete_cmorpho_sets(Cmorpho_sets);
void mw_copy_cmorpho_sets(Cmorpho_sets, Cmorpho_sets);
unsigned int mw_cmorpho_sets_length(Cmorpho_sets);
unsigned int mw_cmorpho_sets_num(Cmorpho_sets);
void mw_cmorpho_sets_clear_stated(Cmorpho_sets);

Cmimage mw_new_cmimage(void);
Cmimage mw_change_cmimage(Cmimage);
void mw_delete_cmimage(Cmimage);
void mw_copy_cmimage(Cmimage,Cmimage);
unsigned int mw_cmimage_length_ml(Cmimage);
unsigned int mw_cmimage_length_fml(Cmimage);
unsigned int mw_cmimage_length_ms(Cmimage);

#else

Point_type mw_new_point_type();
Point_type mw_change_point_type();
void mw_delete_point_type();
void mw_copy_point_type();

Cmorpho_line mw_new_cmorpho_line();
Cmorpho_line mw_change_cmorpho_line();
void mw_delete_cmorpho_line();
void mw_copy_cmorpho_line();
unsigned int mw_cmorpho_line_length();
unsigned int mw_cmorpho_line_num();

Cfmorpho_line mw_new_cfmorpho_line();
Cfmorpho_line mw_change_cfmorpho_line();
void mw_delete_cfmorpho_line();
void mw_copy_cfmorpho_line();
unsigned int mw_cfmorpho_line_length();

Segment mw_new_segment();
Segment mw_change_segment();
void mw_delete_segment();

Cmorpho_set mw_new_cmorpho_set();
Cmorpho_set mw_alloc_cmorpho_set();
Cmorpho_set mw_change_cmorpho_set();
void mw_delete_cmorpho_set();
void mw_copy_cmorpho_set();
unsigned int mw_cmorpho_set_length();

Cmorpho_sets mw_new_cmorpho_sets();
Cmorpho_sets mw_change_cmorpho_sets();
void mw_delete_cmorpho_sets();
void mw_copy_cmorpho_sets();
unsigned int mw_cmorpho_sets_length();
unsigned int mw_cmorpho_sets_num();
void mw_cmorpho_sets_clear_stated();

Cmimage mw_new_cmimage();
Cmimage mw_change_cmimage();
void mw_delete_cmimage();
void mw_copy_cmimage();
unsigned int mw_cmimage_length_ml();
unsigned int mw_cmimage_length_fml();
unsigned int mw_cmimage_length_ms();

#endif

#endif

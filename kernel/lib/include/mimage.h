/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   mimage.h
   
   Vers. 1.7
   (C) 1996-99 Coloma Ballester, Jacques Froment, Manolo Gonzalez

   Internal Input/Output for the following morphological structures
       Morpho_line
       Fmorpho_line
       Segment
       Morpho_set
       Morpho_sets
       Mimage 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef mimage_flg
#define mimage_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "string_size.h"

#ifndef curve_flg
#include "curve.h"
#endif
#ifndef fcurve_flg
#include "fcurve.h"
#endif

/* Almost Infinity for floating points */
#ifdef Linux  
/* There is a strange value in FLT_MAX (Linux 2.0.36) */
#define MORPHO_INFTY 1e+37
#else
#ifdef FLT_MAX
#define MORPHO_INFTY FLT_MAX
#else
#define MORPHO_INFTY 3.40282347e+38
#endif
#endif

/* Type of a Point in a curve */

typedef struct point_type {
  unsigned char type; /* Type of the point */
                      /*
			 0 : regular point;
			 1 : point in the image's border;
			 2 : T-junction;
			 3 : Tau-junction;
			 4 : X-junction;
			 5 : Y-junction.
			 */
  struct point_type *previous; /*Pointer to the previous point (may be NULL)*/
  struct point_type *next; /* Pointer to the next point (may be NULL) */
} *Point_type;

/* One Morpho_line in the discrete grid */
/* This is the border of a connected component of the following morpho set   */
/* {(x,y) / minvalue <= u(x,y) <= maxvalue} for u a gray-level image         */
/* When minvalue=maxvalue this is an iso line, when                          */
/* minvalue=-MORPHO_INFTY or when maxvalue=MORPHO_INFTY this is a level line */

typedef struct morpho_line {
  Point_curve first_point;/* Pointer to the first point of the morpho_line curve */
  Point_type first_type;  /* Pointer to the first Point_type */
  float minvalue;         /* Minimum gray level value of this morpho line */
  float maxvalue;         /* Maximum gray level value of this morpho line */
  unsigned char open;     /* 0 if the morpho line is closed, opened otherwise */
  float data;             /* User-defined data field (saved) */
  void *pdata;            /* User-defined data field : pointer to something (not saved) */
   
  /* For use in Mimage only */
  struct morpho_sets *morphosets;/* Pointer to the associated morpho sets */
  unsigned int num;              /* Morpho line number (range in the chain) */
  struct morpho_line *previous;  /* Pointer to the previous m.l. (may be NULL) */
  struct morpho_line *next;      /* Pointer to the next m.l. (may be NULL) */
} *Morpho_line;

/* One Morpho_line in the continuous plane */

typedef struct fmorpho_line {
  Point_fcurve first_point;/* Pointer to the first point of the fmorpho_line curve */
  Point_type first_type;   /* Pointer to the first Point_type */
  float minvalue;          /* Minimum gray level value of this morpho line */
  float maxvalue;          /* Maximum gray level value of this morpho line */
  unsigned char open;      /* 0 if the morpho line is closed, opened otherwise */
  float data;             /* User-defined data field (saved) */
  void *pdata;            /* User-defined data field : pointer to something (not saved) */
 
  /* For use in Mimage only */
  struct fmorpho_line *previous; /* Pointer to the previous m.l. (may be NULL) */
  struct fmorpho_line *next;     /* Pointer to the next m.l. (may be NULL) */
} *Fmorpho_line;


/* An horizontal segment in the discrete grid */

typedef struct segment {
  int xstart; /* Left x-coordinate of the segment */
  int xend;   /* Right x-coordinate of the segment */
  int y;      /* y-coordinate of the segment */
  struct segment *previous; /* Pointer to the previous segment (may be NULL) */
  struct segment *next;     /* Pointer to the next segment (may be NULL) */
} *Segment;


/* One Morpho_set in the discrete grid */
/* This is one connected component of the following set  :                  */
/* {(x,y) / minvalue <= u(x,y) <= maxvalue} for u a gray-level image        */
/* When minvalue=maxvalue this is an iso set, when                          */
/* minvalue=-MORPHO_INFTY or when maxvalue=MORPHO_INFTY this is a level set */

typedef struct morpho_set {
  unsigned int num;      /* Morpho set number (range in the Morpho_sets struct.) */
  Segment first_segment; /* Pointer to the first segment of the morpho set */
  Segment last_segment;  /* Pointer to the last segment of the morpho set */  
  float minvalue;        /* Minimum gray level value of this set */
  float maxvalue;        /* Maximum gray level value of this set */
  unsigned char stated;  /* 1 if this m.s. has already been stated, 0 otherwise */
  int area;              /* Area of the set (number of pixels belonging to this set) */
  struct morpho_sets *neighbor; /* Pointer to a chain of neighbor morpho sets (may be NULL)*/
} *Morpho_set;

/* A set of morpho sets. This structure is used to record a chain of morpho sets.
*/ 

typedef struct morpho_sets {
  Morpho_set morphoset;         /* Pointer to the current morpho set */
  struct morpho_sets *previous;/* Pointer to the previous morpho sets of the chain */
  struct morpho_sets *next;    /* Pointer to the next morpho sets of the chain */
  /* For use in Mimage only */
  struct morpho_line *morpholine;  /* Pointer to the associated morpho line */
} *Morpho_sets;


/* Morphological image */

typedef struct mimage {
  char cmt[mw_cmtsize];   /* Comments */
  char name[mw_namesize]; /* Name of the set */
  int nrow;               /* Number of rows (dy) */
  int ncol;               /* Number of columns (dx) */
  float minvalue;         /* Minimal Gray level value in the image */
  float maxvalue;         /* Maximal Gray level value in the image */
  Morpho_line first_ml;   /* Pointer to the first morpho line in the discrete grid */
  Fmorpho_line first_fml; /* Pointer to the first morpho line in the continuous plane */
  Morpho_sets first_ms;   /* Pointer to the first morpho sets in the discrete grid */
} *Mimage;

/* Functions definition */

#ifdef __STDC__

Point_type mw_new_point_type(void);
Point_type mw_change_point_type(Point_type);
void mw_delete_point_type(Point_type);
void mw_copy_point_type(Point_type, Point_type);

Morpho_line mw_new_morpho_line(void);
Morpho_line mw_change_morpho_line(Morpho_line);
void mw_delete_morpho_line(Morpho_line);
void mw_copy_morpho_line(Morpho_line,Morpho_line);
unsigned int mw_morpho_line_length(Morpho_line);
unsigned int mw_morpho_line_num(Morpho_line);

Fmorpho_line mw_new_fmorpho_line(void);
Fmorpho_line mw_change_fmorpho_line(Fmorpho_line);
void mw_delete_fmorpho_line(Fmorpho_line);
void mw_copy_fmorpho_line(Fmorpho_line,Fmorpho_line);
unsigned int mw_fmorpho_line_length(Fmorpho_line);

Segment mw_new_segment(void);
Segment mw_change_segment(Segment);
void mw_delete_segment(Segment);

Morpho_set mw_new_morpho_set(void);
Morpho_set mw_alloc_morpho_set(Morpho_set,int);
Morpho_set mw_change_morpho_set(Morpho_set);
void mw_delete_morpho_set(Morpho_set);
void mw_copy_morpho_set(Morpho_set, Morpho_set);
unsigned int mw_morpho_set_length(Morpho_set);

Morpho_sets mw_new_morpho_sets(void);
Morpho_sets mw_change_morpho_sets(Morpho_sets);
void mw_delete_morpho_sets(Morpho_sets);
void mw_copy_morpho_sets(Morpho_sets, Morpho_sets);
unsigned int mw_morpho_sets_length(Morpho_sets);
unsigned int mw_morpho_sets_num(Morpho_sets);
void mw_morpho_sets_clear_stated(Morpho_sets);

Mimage mw_new_mimage(void);
Mimage mw_change_mimage(Mimage);
void mw_delete_mimage(Mimage);
void mw_copy_mimage(Mimage,Mimage);
unsigned int mw_mimage_length_ml(Mimage);
unsigned int mw_mimage_length_fml(Mimage);
unsigned int mw_mimage_length_ms(Mimage);

#else

Point_type mw_new_point_type();
Point_type mw_change_point_type();
void mw_delete_point_type();
void mw_copy_point_type();

Morpho_line mw_new_morpho_line();
Morpho_line mw_change_morpho_line();
void mw_delete_morpho_line();
void mw_copy_morpho_line();
unsigned int mw_morpho_line_length();
unsigned int mw_morpho_line_num();

Fmorpho_line mw_new_fmorpho_line();
Fmorpho_line mw_change_fmorpho_line();
void mw_delete_fmorpho_line();
void mw_copy_fmorpho_line();
unsigned int mw_fmorpho_line_length();

Segment mw_new_segment();
Segment mw_change_segment();
void mw_delete_segment();

Morpho_set mw_new_morpho_set();
Morpho_set mw_alloc_morpho_set();
Morpho_set mw_change_morpho_set();
void mw_delete_morpho_set();
void mw_copy_morpho_set();
unsigned int mw_morpho_set_length();

Morpho_sets mw_new_morpho_sets();
Morpho_sets mw_change_morpho_sets();
void mw_delete_morpho_sets();
void mw_copy_morpho_sets();
unsigned int mw_morpho_sets_length();
unsigned int mw_morpho_sets_num();
void mw_morpho_sets_clear_stated();

Mimage mw_new_mimage();
Mimage mw_change_mimage();
void mw_delete_mimage();
void mw_copy_mimage();
unsigned int mw_mimage_length_ml();
unsigned int mw_mimage_length_fml();
unsigned int mw_mimage_length_ms();

#endif

#endif

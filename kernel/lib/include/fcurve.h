/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   fcurve.h
   
   Vers. 1.1
   (C) 1995-98 Jacques Froment
   Internal Input/Output point_fcurve, fcurve & fcurves structures

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef fcurve_flg
#define fcurve_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "string_size.h"

/* Point in a fcurve */

typedef struct point_fcurve {
  float x,y; /* Coordinates of the point */

  /* For use in Fcurve only */
  struct point_fcurve *previous; /*Pointer to the previous point (may be NULL)*/
  struct point_fcurve *next; /* Pointer to the next point (may be NULL) */
} *Point_fcurve;

/* One Fcurve */

typedef struct fcurve {
  Point_fcurve first; /* Pointer to the first point of the fcurve */

  /* For use in Fcurves only */
  struct fcurve *previous; /* Pointer to the previous fcurve (may be NULL) */
  struct fcurve *next; /* Pointer to the next fcurve (may be NULL) */
} *Fcurve;

/* Set of Fcurves */

typedef struct fcurves {
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the set */
  Fcurve first;    /* Pointer to the first fcurve */
} *Fcurves;



/* Functions definition */

#ifdef __STDC__
 
Point_fcurve mw_new_point_fcurve(void);
Point_fcurve mw_change_point_fcurve(Point_fcurve);
void mw_delete_point_fcurve(Point_fcurve);
void mw_copy_point_fcurve(Point_fcurve,Point_fcurve);
Fcurve mw_new_fcurve(void);
Fcurve mw_change_fcurve(Fcurve);
void mw_delete_fcurve(Fcurve);
unsigned int mw_fcurve_length(Fcurve);
Fcurves mw_new_fcurves(void);
Fcurves mw_change_fcurves(Fcurves);
void mw_delete_fcurves(Fcurves);
unsigned int mw_fcurves_length(Fcurves);
unsigned int mw_fcurves_npoints(Fcurves);

#else

Point_fcurve mw_new_point_fcurve();
Point_fcurve mw_change_point_fcurve();
void mw_delete_point_fcurve();
void mw_copy_point_fcurve();
Fcurve mw_new_fcurve();
Fcurve mw_change_fcurve();
void mw_delete_fcurve();
Fcurves mw_new_fcurves();
Fcurves mw_change_fcurves();
void mw_delete_fcurves();
unsigned int mw_fcurve_length();
Fcurves mw_new_fcurves();
Fcurves mw_change_fcurves();
void mw_delete_fcurves();
unsigned int mw_fcurves_length();
unsigned int mw_fcurves_npoints();

#endif

#endif

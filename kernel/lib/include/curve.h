/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   curve.h
   
   Vers. 1.1
   (C) 1993-98 Jacques Froment
   Internal Input/Output point_curve, curve & curves structures

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef curve_flg
#define curve_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "string_size.h"

/* Point in a curve */

typedef struct point_curve {
  int x,y; /* Coordinates of the point */

  /* For use in Curve only */
  struct point_curve *previous; /*Pointer to the previous point (may be NULL)*/
  struct point_curve *next; /* Pointer to the next point (may be NULL) */
} *Point_curve;

/* One Curve */

typedef struct curve {
  Point_curve first; /* Pointer to the first point of the curve */

  /* For use in Curves only */
  struct curve *previous; /* Pointer to the previous curve (may be NULL) */
  struct curve *next; /* Pointer to the next curve (may be NULL) */
} *Curve;

/* Set of Curves */

typedef struct curves {
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the set */
  Curve first;    /* Pointer to the first curve */
} *Curves;



/* Functions definition */

#ifdef __STDC__
 
Point_curve mw_new_point_curve(void);
Point_curve mw_change_point_curve(Point_curve);
void mw_delete_point_curve(Point_curve);
void mw_copy_point_curve(Point_curve,Point_curve);
Curve mw_new_curve(void);
Curve mw_change_curve(Curve);
void mw_delete_curve(Curve);
void mw_copy_curve(Curve, Curve);
unsigned int mw_curve_length(Curve);
Curves mw_new_curves(void);
Curves mw_change_curves(Curves);
void mw_delete_curves(Curves);
unsigned int mw_curves_length(Curves);
unsigned int mw_curves_npoints(Curves);

#else

Point_curve mw_new_point_curve();
Point_curve mw_change_point_curve();
void mw_delete_point_curve();
void mw_copy_point_curve();
Curve mw_new_curve();
Curve mw_change_curve();
void mw_delete_curve();
void mw_copy_curve();
unsigned int mw_curve_length();
Curves mw_new_curves();
Curves mw_change_curves();
void mw_delete_curves();
unsigned int mw_curves_length();
unsigned int mw_curves_npoints();

#endif

#endif

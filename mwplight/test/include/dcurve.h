/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   dcurve.h
   
   Vers. 1.2
   (C) 2000-2002 Jacques Froment
   Internal Input/Output point_dcurve, dcurve & dcurves structures

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef dcurve_flg
#define dcurve_flg

#ifdef SunOS
#include <sys/types.h>
#endif

/* Point in a dcurve */

typedef struct point_dcurve {
  double x,y; /* Coordinates of the point */

  /* For use in Dcurve only */
  struct point_dcurve *previous; /*Pointer to the previous point (may be NULL)*/
  struct point_dcurve *next; /* Pointer to the next point (may be NULL) */
} *Point_dcurve;

/* One Dcurve */

typedef struct dcurve {
  Point_dcurve first; /* Pointer to the first point of the dcurve */

  /* For use in Dcurves only */
  struct dcurve *previous; /* Pointer to the previous dcurve (may be NULL) */
  struct dcurve *next; /* Pointer to the next dcurve (may be NULL) */
} *Dcurve;

/* Set of Dcurves */

typedef struct dcurves {
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the set */
  Dcurve first;    /* Pointer to the first dcurve */
} *Dcurves;



/* Functions definition */

#ifdef __STDC__
 
Point_dcurve mw_new_point_dcurve(void);
Point_dcurve mw_change_point_dcurve(Point_dcurve);
void mw_delete_point_dcurve(Point_dcurve);
Point_dcurve mw_copy_point_dcurve(Point_dcurve,Point_dcurve);
Dcurve mw_new_dcurve(void);
Dcurve mw_change_dcurve(Dcurve);
void mw_delete_dcurve(Dcurve);
Dcurve mw_copy_dcurve(Dcurve, Dcurve);
unsigned int mw_length_dcurve(Dcurve);
Dcurves mw_new_dcurves(void);
Dcurves mw_change_dcurves(Dcurves);
void mw_delete_dcurves(Dcurves);
unsigned int mw_length_dcurves(Dcurves);
unsigned int mw_npoints_dcurves(Dcurves);

#else

Point_dcurve mw_new_point_dcurve();
Point_dcurve mw_change_point_dcurve();
void mw_delete_point_dcurve();
Point_dcurve mw_copy_point_dcurve();
Dcurve mw_new_dcurve();
Dcurve mw_change_dcurve();
void mw_delete_dcurve();
Dcurve mw_copy_dcurve();
unsigned int mw_length_dcurve();
Dcurves mw_new_dcurves();
Dcurves mw_change_dcurves();
void mw_delete_dcurves();
unsigned int mw_length_dcurve();
Dcurves mw_new_dcurves();
Dcurves mw_change_dcurves();
void mw_delete_dcurves();
unsigned int mw_length_dcurves();
unsigned int mw_npoints_dcurves();

#endif

#endif

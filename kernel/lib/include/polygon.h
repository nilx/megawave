/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   polygon.h
   
   Vers. 1.3
   (C) 1993-2002 Jacques Froment
   Internal Input/Output polygon & polygons structures

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef polygon_flg
#define polygon_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#ifndef curve_flg
#include "curve.h"
#endif

/* Need the Point_curve structure defined in curve.h */

/* One Polygon */

typedef struct polygon {
  int nb_channels;  /* Number of channels */
  float *channel;  /* Tab to the channel */
                   /* The number of elements is given by nb_channels */
  Point_curve first; /* Pointer to the first point of the curve */

  /* For use in Polygons only */
  struct polygon *previous; /* Pointer to the previous poly. (may be NULL) */
  struct polygon *next; /* Pointer to the next poly. (may be NULL) */
} *Polygon;

/* Set of Polygons */

typedef struct polygons {
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the set */
  Polygon first;    /* Pointer to the first polygon */
} *Polygons;



/* Functions definition */

#ifdef __STDC__

Polygon mw_new_polygon(void);
Polygon mw_alloc_polygon(Polygon,int);
Polygon mw_change_polygon(Polygon, int);
void mw_delete_polygon(Polygon);
unsigned int mw_length_polygon(Polygon);
Polygons mw_new_polygons(void);
Polygons mw_change_polygons(Polygons);
void mw_delete_polygons(Polygons);
unsigned int mw_length_polygons(Polygons);

#else

Polygon mw_new_polygon();
Polygon mw_alloc_polygon();
Polygon mw_change_polygon();
void mw_delete_polygon();
unsigned int mw_length_polygon();
Polygons mw_new_polygons();
Polygons mw_change_polygons();
void mw_delete_polygons();
unsigned int mw_length_polygons();

#endif

#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   fpolygon.h
   
   Vers. 1.2
   (C) 1995-2002 Jacques Froment
   Internal Input/Output fpolygon & fpolygons structures

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef fpolygon_flg
#define fpolygon_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#ifndef fcurve_flg
#include "fcurve.h"
#endif

/* Need the Point_fcurve structure defined in fcurve.h */

/* One Fpolygon */

typedef struct fpolygon {
  int nb_channels;  /* Number of channels */
  float *channel;  /* Tab to the channel */
                   /* The number of elements is given by nb_channels */
  Point_fcurve first; /* Pointer to the first point of the fcurve */

  /* For use in Fpolygons only */
  struct fpolygon *previous; /* Pointer to the previous poly. (may be NULL) */
  struct fpolygon *next; /* Pointer to the next poly. (may be NULL) */
} *Fpolygon;

/* Set of Fpolygons */

typedef struct fpolygons {
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the set */
  Fpolygon first;    /* Pointer to the first fpolygon */
} *Fpolygons;



/* Functions definition */

#ifdef __STDC__

Fpolygon mw_new_fpolygon(void);
Fpolygon mw_alloc_fpolygon(Fpolygon,int);
Fpolygon mw_change_fpolygon(Fpolygon, int);
void mw_delete_fpolygon(Fpolygon);
unsigned int mw_length_fpolygon(Fpolygon);
Fpolygons mw_new_fpolygons(void);
Fpolygons mw_change_fpolygons(Fpolygons);
void mw_delete_fpolygons(Fpolygons);
unsigned int mw_length_fpolygons(Fpolygons);

#else

Fpolygon mw_new_fpolygon();
Fpolygon mw_alloc_fpolygon();
Fpolygon mw_change_fpolygon();
void mw_delete_fpolygon();
unsigned int mw_length_fpolygon();
Fpolygons mw_new_fpolygons();
Fpolygons mw_change_fpolygons();
void mw_delete_fpolygons();
unsigned int mw_length_fpolygons();

#endif

#endif

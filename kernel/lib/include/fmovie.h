/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   fmovie.h
   
   Vers. 1.0
   (C) 1994 Jacques Froment
   Internal Input/Output fmovie structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef fmovie_flg
#define fmovie_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "fimage.h"

/* Movie of floating point Gray Level Image */

typedef struct fmovie {
  float scale;     /* Scale (time-domain) of the movie (should be 1) */
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the image */
  Fimage first;    /* Pointer to the first image */
} *Fmovie;

/* Functions definition */

#ifdef __STDC__

Fmovie mw_new_fmovie(void);
void mw_delete_fmovie(Fmovie);
Fmovie mw_change_fmovie(Fmovie);

#else

Fmovie mw_new_fmovie();
void mw_delete_fmovie();
Fmovie mw_change_fmovie();

#endif

#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   cfmovie.h
   
   Vers. 1.0
   (C) 1995 Jacques Froment
   Internal Input/Output cfmovie structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef cfmovie_flg
#define cfmovie_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "cfimage.h"

/* Movie of Unsigned Char (byte) Gray Level Image */

typedef struct cfmovie {
  float scale;     /* Scale (time-domain) of the movie (should be 1) */
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the image */
  Cfimage first;    /* Pointer to the first image */
} *Cfmovie;

/* Functions definition */

#ifdef __STDC__

Cfmovie mw_new_cfmovie(void);
void mw_delete_cfmovie(Cfmovie);
Cfmovie mw_change_cfmovie(Cfmovie);

#else

Cfmovie mw_new_cfmovie();
void mw_delete_cfmovie();
Cfmovie mw_change_cfmovie();

#endif

#endif

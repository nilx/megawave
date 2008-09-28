/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ccmovie.h
   
   Vers. 1.1
   (C) 1995-2000 Jacques Froment
   Internal Input/Output ccmovie structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef ccmovie_flg
#define ccmovie_flg

#ifdef SunOS
#include <sys/types.h>
#endif

/* Movie of Unsigned Char (byte) Gray Level Image */

typedef struct ccmovie {
  float scale;     /* Scale (time-domain) of the movie (should be 1) */
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the image */
  Ccimage first;    /* Pointer to the first image */
} *Ccmovie;

/* Functions definition */

#ifdef __STDC__

Ccmovie mw_new_ccmovie(void);
void mw_delete_ccmovie(Ccmovie);
Ccmovie mw_change_ccmovie(Ccmovie);

#else

Ccmovie mw_new_ccmovie();
void mw_delete_ccmovie();
Ccmovie mw_change_ccmovie();

#endif

#endif

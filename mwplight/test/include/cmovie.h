/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   cmovie.h
   
   Vers. 1.0
   (C) 1993 Jacques Froment
   Internal Input/Output cmovie structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef cmovie_flg
#define cmovie_flg

#ifdef SunOS
#include <sys/types.h>
#endif

#include "cimage.h"

/* Movie of Unsigned Char (byte) Gray Level Image */

typedef struct cmovie {
  float scale;     /* Scale (time-domain) of the movie (should be 1) */
  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the image */
  Cimage first;    /* Pointer to the first image */
} *Cmovie;

/* Functions definition */

#ifdef __STDC__

Cmovie mw_new_cmovie(void);
void mw_delete_cmovie(Cmovie);
Cmovie mw_change_cmovie(Cmovie);

#else

Cmovie mw_new_cmovie();
void mw_delete_cmovie();
Cmovie mw_change_cmovie();

#endif

#endif

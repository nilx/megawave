/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   fsignal.h
   
   Vers. 1.1
   (C) 1993-2000 Jacques Froment
   Internal Input/Output fsignal structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef fsignal_flg
#define fsignal_flg

#ifdef SunOS
#include <sys/types.h>
#endif

/* Floating Point Signal */

typedef struct fsignal {
  int size;        /* Number of samples */
  int allocsize;   /* Size allocated (in bytes) for the values plane */
  float *values;   /* The samples */

  float scale;     /* Scale of the signal */
  float shift;     /* shifting of the signal with respect to zero */
  float gain;      /* Gain of the signal given by the digitalization process */
  float sgrate;    /* Sampling rate given by the digitalization process */

  char cmt[mw_cmtsize]; /* Comments */
  char name[mw_namesize]; /* Name of the image */
  
  /* Defines the signifiant part of the signal : */
  int firstp;     /* index of the first point not aff. by left side effect */
  int lastp;      /* index of the last point not aff. by right side effect */
  float param;    /* distance between two succesive uncorrelated points */

} *Fsignal;
   
/* Functions definition */

#ifdef __STDC__

Fsignal mw_new_fsignal(void);
Fsignal mw_alloc_fsignal(Fsignal, int);
void mw_delete_fsignal(Fsignal);
Fsignal mw_change_fsignal(Fsignal, int);
void mw_clear_fsignal(Fsignal, float);
void mw_copy_fsignal(Fsignal,Fsignal);

#else

Fsignal mw_new_fsignal();
Fsignal mw_alloc_fsignal();
void mw_delete_fsignal();
Fsignal mw_change_fsignal();
void mw_clear_fsignal();
void mw_copy_fsignal();

#endif

#endif

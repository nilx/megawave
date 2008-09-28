/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   unix_bsd.h
   
   Vers. 1.1
   (C) 1994-2001 Jacques Froment
   Definition of some Unix BSD functions not implemented on Unix System V

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef unix_bsd_flg
#define unix_bsd_flg

/* Functions definition */

#ifdef __STDC__

double my_rint(double);
double aint(double);
int nint(double);
double log2(double);
double exp2(double);
double my_cbrt(double);
#ifndef sun4_5
#ifndef iris
#ifndef Linux
void usleep(unsigned);
#endif
#endif
#endif

#else

double my_rint();
double aint();
int nint();
double log2();
double exp2();
double my_cbrt();
#ifndef sun4_5
#ifndef iris
#ifndef Linux
void usleep();
#endif
#endif
#endif

#endif

#endif

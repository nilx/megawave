/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   mwi.h
   
   Vers. 1.1
   (C) 1993-2000 Jacques Froment & Sylvain Parrino
   Include file for module's interpretor structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef MWI_H
#define MWI_H

struct Mwiline {
  char *name;
  int (*mwarg)();
  int (*mwuse)();
  char *group;
  char *function;
  char *usage;
  char *fsummary;
};

typedef struct Mwiline Mwiline;

#ifndef MWI_DEC
extern Mwiline mwicmd[];
extern int mwind;

#define mwusage(S)          (mwicmd[mwind].mwuse(S))
#define _mwmain(ARGC, ARGV) (mwicmd[mwind].mwarg((ARGC), (ARGV)))

#endif


#endif




/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef GENMAIN_INC
#define GENMAIN_INC

#ifdef GENMAIN_DEC
/* Modifie par JF 12/11/96 : dimensionnement de usagebuf insuffisant */
/* pour certains headers (provoque un segmentation fault)            */
char usagebuf[2*BUFSIZ];
char groupbuf[BUFSIZ];
#else
extern char usagebuf[];
extern char groupbuf[];
#endif

#ifdef __STDC__
extern void genmain(FILE *);
extern void Lowerline(char *);
#else
extern genmain();
extern void Lowerline();
#endif
#endif

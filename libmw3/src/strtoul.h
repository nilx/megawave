/* strtoul v 1.0
  (c)1995 Jacques Froment

  Attention: ne definir cette fonction que si la librairie standard
             ne l'inclut pas ! (sinon, risque de malfonction).
*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifdef SunOS
#ifndef sun4_5
#define strtoul(S, P, B) (unsigned long)strtol((S), (P), (B))
#endif
#endif

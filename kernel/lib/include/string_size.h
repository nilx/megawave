/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   string_size.h
   
   Vers. 1.1
   (C) 1993-2000 Jacques Froment
   Define the size of some string used in internal MegaWave2 types

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef string_size_flg
#define string_size_flg

/* Maximum Size of the MegaWave2 memory types (such as "Cimage") */
#define mw_mtype_size 20

/* Maximum Size of the MegaWave2 file types (such as "IMG") */
#define mw_ftype_size 20

/* Used in many of internal I/O types */

#define mw_cmtsize 255 /* Size of the comments field */
#define mw_namesize 255 /* Size of the name field */

#endif



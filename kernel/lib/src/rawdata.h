/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   rawdata.h
   
   Vers. 1.1
   (C) 2000 Jacques Froment
   Internal Input/Output rawdata structure.
   This internal type allows to load into memory any kind of file.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef rawdata_flg
#define rawdata_flg

Rawdata mw_new_rawdata(void);
Rawdata mw_alloc_rawdata(Rawdata, int);
void mw_delete_rawdata(Rawdata);
Rawdata mw_change_rawdata(Rawdata, int);
void mw_copy_rawdata(Rawdata, Rawdata);

#endif

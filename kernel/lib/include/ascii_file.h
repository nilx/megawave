/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ascii_file.h
   
   Vers. 1.1
   (C) 1993-2002 Jacques Froment
   Functions for the MW DATA ASCII files

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef ascii_file_flg
#define ascii_file_flg

#include <sys/file.h>

#define _MW_DATA_ASCII_FILE_HEADER "MegaWave2 - DATA ASCII file -\n"

/* Functions definition */

#ifdef __STDC__

int _mw_fascii_search_string(FILE *, char *);
int _mw_fascii_get_field(FILE *, char *, char *, char *, void *);
int _mw_fascii_get_optional_field(FILE *, char *, char *, char *, void *);
FILE *_mw_open_data_ascii_file(char *);
FILE *_mw_create_data_ascii_file(char *);

#else

int _mw_fascii_search_string();
int _mw_fascii_get_field();
int _mw_fascii_get_optinal_field();
FILE *_mw_open_data_ascii_file();
FILE *_mw_create_data_ascii_file();

#endif

#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   file_type.h
   
   Vers. 2.02
   (C) 1993-2001 Jacques Froment
   Functions declaration in file_type.c

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef file_type_flg
#define file_type_flg

#include <stdio.h>
#include "type_conv.h"
#include "native_ftype.h"

/* 
   Size of Header ID for MW2 binary types. 
   Header ID is now defined from the file format version number 
   by a call to _mw_write_header_file().
*/
#define SIZE_OF_MW2_BIN_TYPE_ID 6

/*===== functions declaration in file_type.c =====*/

#ifdef __STDC__

int _mw_get_range_array(char *,char *,char *[]);
int _mw_get_max_range_array(char *,char *[]);
void _mw_put_range_array(char *,char *,int r,char *[]);
int _mw_exist_array(char *,char *,char *[]);
void _mw_lower_type(char []);
int _mw_get_range_type(char [],char *);
void _mw_put_range_type(char [],char *, int);
int _mw_native_ftype_exists(char [], char *);
int _mw_ftype_exists_for_output(char *, char *);
void _mw_make_type(char [], char [], char *);
void _mw_print_available_ftype_for_output(char *);
void _mw_choose_type(char [], char *, char *);
void  _mw_make_comment(char [], char []);
int _mw_get_binary_file_type(char *, char *, char *, int *, float *);
int _mw_get_ascii_file_type(char *, char *, char *, int *, float *);
int _mw_get_file_type(char *, char *, char *, int *, float *);

#else

int _mw_get_range_array();
int _mw_get_max_range_array();
void _mw_put_range_array();
int _mw_exist_array();
void _mw_lower_type();
int _mw_get_range_type();
void _mw_put_range_type();
int _mw_native_ftype_exists();
int _mw_ftype_exists_for_output();
void _mw_make_type();
void _mw_print_available_ftype_for_output();
void _mw_choose_type();
void  _mw_make_comment();
int _mw_get_binary_file_type();
int _mw_get_ascii_file_type();
int _mw_get_file_type();

#endif

#endif








/*
 * ascii_file.h
 */

#ifndef _ASCII_FILE_H
#define _ASCII_FILE_H

#define _MW_DATA_ASCII_FILE_HEADER "MegaWave2 - DATA ASCII file -\n"

int _mw_fascii_search_string(FILE *, char *);
void _mw_remove_first_spaces(char *);
void _mw_basename(char *, char *);
void _mw_dirbasename(char *,char *,char *);
int _mw_fascii_get_field(FILE *, char *, char *, char *, void *);
int _mw_fascii_get_optional_field(FILE *, char *, char *, char *, void *);
FILE * _mw_open_data_ascii_file(char *);
FILE * _mw_create_data_ascii_file(char *);

#endif /* !_ASCII_FILE_H */

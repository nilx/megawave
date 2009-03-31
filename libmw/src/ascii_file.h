/*
 * ascii_file.h
 */

#ifndef _ASCII_FILE_H_
#define _ASCII_FILE_H_

/* src/ascii_file.c */
int _mw_fascii_search_string(FILE * fp, char *str);
void _mw_remove_first_spaces(char *s);
void _mw_basename(char *s, char *bname);
void _mw_dirbasename(char *s, char *dname, char *bname);
int _mw_fascii_get_field(FILE * fp, char *fname, char *field_name,
                         char *str_control, void *ptr);
int _mw_fascii_get_optional_field(FILE * fp, char *fname, char *field_name,
                                  char *str_control, void *ptr);
FILE *_mw_open_data_ascii_file(char *fname);
FILE *_mw_create_data_ascii_file(char *fname);

#endif                          /* !_ASCII_FILE_H_ */

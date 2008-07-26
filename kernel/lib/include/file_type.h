/*
 * file_type.h
 */

#ifndef _FILE_TYPE_H
#define _FILE_TYPE_H

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
char *_mw_get_ftype_opt(char *);
char *_mw_get_ftype_only(char *);
int _mw_is_of_ftype(char *, char *);
void  _mw_make_comment(char [], char []);
int _mw_get_binary_file_type(char *, char *, char *, int *, float *);
int _mw_get_ascii_file_type(char *, char *, char *, int *, float *);
int _mw_get_file_type(char *, char *, char *, int *, float *);

#endif /* !_FILE_TYPE_H */

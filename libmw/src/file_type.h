/*
 * file_type.h
 */

#ifndef _FILE_TYPE_H_
#define _FILE_TYPE_H_

/* src/file_type.c */
void _mw_lower_type(char type[]);
void _mw_put_range_type(char type[], char *mtype, int r);
void _mw_make_type(char type[], char type_in[], char *mtype);
void _mw_choose_type(char type[], char *type_force, char *mtype);
char *_mw_get_ftype_opt(char *ftype);
int _mw_is_of_ftype(char *in, char *type);
void _mw_make_comment(char comment[], char comment_in[]);
int _mw_get_file_type(char *fname, char *ftype, char *mtype, int *hsize,
                      float *version);

#endif                          /* !_FILE_TYPE_H_ */

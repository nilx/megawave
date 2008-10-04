/*
 * file_type.h
 */

#ifndef _FILE_TYPE_H_
#define _FILE_TYPE_H_

/* src/file_type.c */
int _mw_get_range_array(char *a, char *b, char *A[]);
int _mw_get_max_range_array(char *a, char *A[]);
void _mw_put_range_array(char *a, char *b, int r, char *A[]);
int _mw_exist_array(char *a, char *b, char *A[]);
void _mw_lower_type(char type[]);
int _mw_get_range_type(char type[], char *mtype);
void _mw_put_range_type(char type[], char *mtype, int r);
int _mw_native_ftype_exists(char type[], char *mtype);
int _mw_ftype_exists_for_output(char *ftype, char *mtype);
void _mw_make_type(char type[], char type_in[], char *mtype);
void _mw_print_available_ftype_for_output(char *mtype);
void _mw_choose_type(char type[], char *type_force, char *mtype);
char *_mw_get_ftype_opt(char *ftype);
char *_mw_get_ftype_only(char *ftype);
int _mw_is_of_ftype(char *in, char *type);
void _mw_make_comment(char comment[], char comment_in[]);
int _mw_get_binary_file_type(char *fname, char *ftype, char *mtype, int *hsize, float *version);
int _mw_get_ascii_file_type(char *fname, char *ftype, char *mtype, int *hsize, float *version);
int _mw_get_file_type(char *fname, char *ftype, char *mtype, int *hsize, float *version);

#endif /* !_FILE_TYPE_H_ */

/*
 * cmimage.h
 */

#ifndef _CMIMAGE_H
#define _CMIMAGE_H

Cmorpho_line mw_new_cmorpho_line(void);
Cmorpho_line mw_change_cmorpho_line(Cmorpho_line);
void mw_delete_cmorpho_line(Cmorpho_line);
Cmorpho_line mw_copy_cmorpho_line(Cmorpho_line,Cmorpho_line);
unsigned int mw_length_cmorpho_line(Cmorpho_line);
unsigned int mw_num_cmorpho_line(Cmorpho_line);

Cfmorpho_line mw_new_cfmorpho_line(void);
Cfmorpho_line mw_change_cfmorpho_line(Cfmorpho_line);
void mw_delete_cfmorpho_line(Cfmorpho_line);
Cfmorpho_line mw_copy_cfmorpho_line(Cfmorpho_line,Cfmorpho_line);
unsigned int mw_length_cfmorpho_line(Cfmorpho_line);

Cmorpho_set mw_new_cmorpho_set(void);
Cmorpho_set mw_alloc_cmorpho_set(Cmorpho_set,int);
Cmorpho_set mw_change_cmorpho_set(Cmorpho_set);
void mw_delete_cmorpho_set(Cmorpho_set);
Cmorpho_set mw_copy_cmorpho_set(Cmorpho_set, Cmorpho_set);
unsigned int mw_length_cmorpho_set(Cmorpho_set);

Cmorpho_sets mw_new_cmorpho_sets(void);
Cmorpho_sets mw_change_cmorpho_sets(Cmorpho_sets);
void mw_delete_cmorpho_sets(Cmorpho_sets);
Cmorpho_sets mw_copy_cmorpho_sets(Cmorpho_sets, Cmorpho_sets);
unsigned int mw_length_cmorpho_sets(Cmorpho_sets);
unsigned int mw_num_cmorpho_sets(Cmorpho_sets);
void mw_cmorpho_sets_clear_stated(Cmorpho_sets);

Cmimage mw_new_cmimage(void);
Cmimage mw_change_cmimage(Cmimage);
void mw_delete_cmimage(Cmimage);
Cmimage mw_copy_cmimage(Cmimage,Cmimage);
unsigned int mw_length_ml_cmimage(Cmimage);
unsigned int mw_length_fml_cmimage(Cmimage);
unsigned int mw_length_ms_cmimage(Cmimage);

#endif /* !_CMIMAGE_H */

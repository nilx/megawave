/*
 * cmimage.h
 */

#ifndef _CMIMAGE_H_
#define _CMIMAGE_H_

/* src/cmimage.c */
Cmorpho_line mw_new_cmorpho_line(void);
Cmorpho_line mw_change_cmorpho_line(Cmorpho_line ll);
void mw_delete_cmorpho_line(Cmorpho_line cmorpho_line);
Cmorpho_line mw_copy_cmorpho_line(Cmorpho_line in, Cmorpho_line out);
unsigned int mw_length_cmorpho_line(Cmorpho_line cmorpho_line);
unsigned int mw_num_cmorpho_line(Cmorpho_line ml_first);
Cfmorpho_line mw_new_cfmorpho_line(void);
Cfmorpho_line mw_change_cfmorpho_line(Cfmorpho_line ll);
void mw_delete_cfmorpho_line(Cfmorpho_line cfmorpho_line);
Cfmorpho_line mw_copy_cfmorpho_line(Cfmorpho_line in, Cfmorpho_line out);
unsigned int mw_length_cfmorpho_line(Cfmorpho_line cfmorpho_line);
Cmorpho_set mw_new_cmorpho_set(void);
Cmorpho_set mw_change_cmorpho_set(Cmorpho_set is);
void mw_delete_cmorpho_set(Cmorpho_set cmorpho_set);
Cmorpho_set mw_copy_cmorpho_set(Cmorpho_set in, Cmorpho_set out);
unsigned int mw_length_cmorpho_set(Cmorpho_set cmorpho_set);
Cmorpho_sets mw_new_cmorpho_sets(void);
Cmorpho_sets mw_change_cmorpho_sets(Cmorpho_sets is);
void mw_delete_cmorpho_sets(Cmorpho_sets cmorpho_sets);
Cmorpho_sets mw_copy_cmorpho_sets(Cmorpho_sets in, Cmorpho_sets out);
unsigned int mw_length_cmorpho_sets(Cmorpho_sets cmorpho_sets);
unsigned int mw_num_cmorpho_sets(Cmorpho_sets mss_first);
void mw_cmorpho_sets_clear_stated(Cmorpho_sets mss_first);
Cmimage mw_new_cmimage(void);
Cmimage mw_change_cmimage(Cmimage mi);
void mw_delete_cmimage(Cmimage cmimage);
Cmimage mw_copy_cmimage(Cmimage in, Cmimage out);
unsigned int mw_length_ml_cmimage(Cmimage cmimage);
unsigned int mw_length_fml_cmimage(Cmimage cmimage);
unsigned int mw_length_ms_cmimage(Cmimage cmimage);

#endif /* !_CMIMAGE_H_ */

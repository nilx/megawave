/*
 * cmimage_io.h
 */

#ifndef _CMIMAGE_IO_H_
#define _CMIMAGE_IO_H_

/* src/cmimage_io.c */
Cmorpho_line _mw_load_cml_mw2_cml(char *fname);
Cmorpho_line _mw_cmorpho_line_load_native(char *fname, char *type);
Cmorpho_line _mw_load_cmorpho_line(char *fname, char *type);
void _mw_write_cml_mw2_cml(FILE * fp, Cmorpho_line ll, unsigned int nml);
short _mw_create_cml_mw2_cml(char *fname, Cmorpho_line ll);
short _mw_cmorpho_line_create_native(char *fname, Cmorpho_line ll,
                                     char *Type);
short _mw_create_cmorpho_line(char *fname, Cmorpho_line ll, char *Type);
Cfmorpho_line _mw_load_cfml_mw2_cfml(char *fname);
Cfmorpho_line _mw_cfmorpho_line_load_native(char *fname, char *type);
Cfmorpho_line _mw_load_cfmorpho_line(char *fname, char *type);
void _mw_write_cfml_mw2_cfml(FILE * fp, Cfmorpho_line fll);
short _mw_create_cfml_mw2_cfml(char *fname, Cfmorpho_line fll);
short _mw_cfmorpho_line_create_native(char *fname, Cfmorpho_line ll,
                                      char *Type);
short _mw_create_cfmorpho_line(char *fname, Cfmorpho_line ll, char *Type);
Cmorpho_set _mw_read_cms_mw2_cms(char *fname, FILE * fp, int need_flipping);
Cmorpho_set _mw_load_cms_mw2_cms(char *fname);
Cmorpho_set _mw_cmorpho_set_load_native(char *fname, char *type);
Cmorpho_set _mw_load_cmorpho_set(char *fname, char *type);
void _mw_write_cms_mw2_cms(FILE * fp, Cmorpho_set is);
short _mw_create_cms_mw2_cms(char *fname, Cmorpho_set is);
short _mw_cmorpho_set_create_native(char *fname, Cmorpho_set ms, char *Type);
short _mw_create_cmorpho_set(char *fname, Cmorpho_set ms, char *Type);
Cmorpho_sets _mw_read_cmss_mw2_cmss(char *fname, FILE * fp,
                                    int need_flipping);
Cmorpho_sets _mw_load_cmss_mw2_cmss(char *fname);
Cmorpho_sets _mw_cmorpho_sets_load_native(char *fname, char *type);
Cmorpho_sets _mw_load_cmorpho_sets(char *fname, char *type);
void _mw_write_cmss_mw2_cmss(FILE * fp, Cmorpho_sets iss);
short _mw_create_cmss_mw2_cmss(char *fname, Cmorpho_sets iss);
short _mw_cmorpho_sets_create_native(char *fname, Cmorpho_sets mss,
                                     char *Type);
short _mw_create_cmorpho_sets(char *fname, Cmorpho_sets mss, char *Type);
Cmimage _mw_load_cmimage_mw2_cmimage(char *fname);
Cmimage _mw_cmimage_load_native(char *fname, char *type);
Cmimage _mw_load_cmimage(char *fname, char *type);
short _mw_create_cmimage_mw2_cmimage(char *fname, Cmimage cmimage);
short _mw_cmimage_create_native(char *fname, Cmimage cmimage, char *Type);
short _mw_create_cmimage(char *fname, Cmimage cmimage, char *Type);

#endif                          /* !_CMIMAGE_IO_H_ */

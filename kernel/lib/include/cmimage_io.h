/*
 * cmimage_io.h
 */

#ifndef _CMIMAGE_IO_H
#define _CMIMAGE_IO_H

Cmorpho_line _mw_load_cml_mw2_cml(char *);
Cmorpho_line _mw_cmorpho_line_load_native(char *, char *);
Cmorpho_line _mw_load_cmorpho_line(char *, char *);
void _mw_write_cml_mw2_cml(FILE *, Cmorpho_line, unsigned int);
short _mw_create_cml_mw2_cml(char *, Cmorpho_line);
short _mw_cmorpho_line_create_native(char *, Cmorpho_line, char *);
short _mw_create_cmorpho_line(char *, Cmorpho_line, char *);
Cfmorpho_line _mw_load_cfml_mw2_cfml(char *);
Cfmorpho_line _mw_cfmorpho_line_load_native(char *, char *);
Cfmorpho_line _mw_load_cfmorpho_line(char *, char *);
void _mw_write_cfml_mw2_cfml(FILE *, Cfmorpho_line);
short _mw_create_cfml_mw2_cfml(char *, Cfmorpho_line);
short _mw_cfmorpho_line_create_native(char *, Cfmorpho_line, char *);
short _mw_create_cfmorpho_line(char *, Cfmorpho_line, char *);
Cmorpho_set _mw_read_cms_mw2_cms(char *, FILE *, int);
Cmorpho_set _mw_load_cms_mw2_cms(char *);
Cmorpho_set _mw_cmorpho_set_load_native(char *, char *);
Cmorpho_set _mw_load_cmorpho_set(char *, char *);
void _mw_write_cms_mw2_cms(FILE *, Cmorpho_set);
short _mw_create_cms_mw2_cms(char *, Cmorpho_set);
short _mw_cmorpho_set_create_native(char *, Cmorpho_set, char *);
short _mw_create_cmorpho_set(char *, Cmorpho_set, char *);
Cmorpho_sets _mw_read_cmss_mw2_cmss(char *, FILE *, int);
Cmorpho_sets _mw_load_cmss_mw2_cmss(char *);
Cmorpho_sets _mw_cmorpho_sets_load_native(char *, char *);
Cmorpho_sets _mw_load_cmorpho_sets(char *, char *);
void _mw_write_cmss_mw2_cmss(FILE *, Cmorpho_sets);
short _mw_create_cmss_mw2_cmss(char *, Cmorpho_sets);
short _mw_cmorpho_sets_create_native(char *, Cmorpho_sets, char *);
short _mw_create_cmorpho_sets(char *, Cmorpho_sets, char *);
Cmimage _mw_load_cmimage_mw2_cmimage(char *);
Cmimage _mw_cmimage_load_native(char *, char *);
Cmimage _mw_load_cmimage(char *, char *);
short _mw_create_cmimage_mw2_cmimage(char *, Cmimage);
short _mw_cmimage_create_native(char *, Cmimage, char *);
short _mw_create_cmimage(char *, Cmimage, char *);

#endif /* !_CMIMAGE_IO_H */

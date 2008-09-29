/*
 * mimage_io.h
 */

#ifndef _MIMAGE_IO_H
#define _MIMAGE_IO_H

Morpho_line _mw_load_ml_mw2_ml(char *);
Morpho_line _mw_morpho_line_load_native(char *, char *);
Morpho_line _mw_load_morpho_line(char *, char *);
void _mw_write_ml_mw2_ml(FILE *, Morpho_line, unsigned int);
short _mw_create_ml_mw2_ml(char *, Morpho_line);
short _mw_morpho_line_create_native(char *, Morpho_line, char *);
short _mw_create_morpho_line(char *, Morpho_line, char *);
Fmorpho_line _mw_load_fml_mw2_fml(char *);
Fmorpho_line _mw_fmorpho_line_load_native(char *, char *);
Fmorpho_line _mw_load_fmorpho_line(char *, char *);
void _mw_write_fml_mw2_fml(FILE *, Fmorpho_line);
short _mw_create_fml_mw2_fml(char *, Fmorpho_line);
short _mw_fmorpho_line_create_native(char *, Fmorpho_line, char *);
short _mw_create_fmorpho_line(char *, Fmorpho_line, char *);
Morpho_set _mw_read_ms_mw2_ms(char *, FILE *, int);
Morpho_set _mw_load_ms_mw2_ms(char *);
Morpho_set _mw_morpho_set_load_native(char *, char *);
Morpho_set _mw_load_morpho_set(char *, char *);
void _mw_write_ms_mw2_ms(FILE *, Morpho_set);
short _mw_create_ms_mw2_ms(char *, Morpho_set);
short _mw_morpho_set_create_native(char *, Morpho_set, char *);
short _mw_create_morpho_set(char *, Morpho_set, char *);
Morpho_sets _mw_read_mss_mw2_mss(char *, FILE *, int);
Morpho_sets _mw_load_mss_mw2_mss(char *);
Morpho_sets _mw_morpho_sets_load_native(char *, char *);
Morpho_sets _mw_load_morpho_sets(char *, char *);
void _mw_write_mss_mw2_mss(FILE *, Morpho_sets);
short _mw_create_mss_mw2_mss(char *, Morpho_sets);
short _mw_morpho_sets_create_native(char *, Morpho_sets, char *);
short _mw_create_morpho_sets(char *, Morpho_sets, char *);
Mimage _mw_load_mimage_mw2_mimage(char *);
Mimage _mw_mimage_load_native(char *, char *);
Mimage _mw_load_mimage(char *, char *);
short _mw_create_mimage_mw2_mimage(char *, Mimage);
short _mw_mimage_create_native(char *, Mimage, char *);
short _mw_create_mimage(char *, Mimage, char *);

#endif /* !_MIMAGE_IO_H */

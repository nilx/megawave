/*
 * mimage_io.h
 */

#ifndef _MIMAGE_IO_H_
#define _MIMAGE_IO_H_

/* src/mimage_io.c */
Morpho_line _mw_load_ml_mw2_ml(char *fname);
Morpho_line _mw_morpho_line_load_native(char *fname, char *type);
Morpho_line _mw_load_morpho_line(char *fname, char *type);
void _mw_write_ml_mw2_ml(FILE * fp, Morpho_line ll, unsigned int nml);
short _mw_create_ml_mw2_ml(char *fname, Morpho_line ll);
short _mw_morpho_line_create_native(char *fname, Morpho_line ll, char *Type);
short _mw_create_morpho_line(char *fname, Morpho_line ll, char *Type);
Fmorpho_line _mw_load_fml_mw2_fml(char *fname);
Fmorpho_line _mw_fmorpho_line_load_native(char *fname, char *type);
Fmorpho_line _mw_load_fmorpho_line(char *fname, char *type);
void _mw_write_fml_mw2_fml(FILE * fp, Fmorpho_line fll);
short _mw_create_fml_mw2_fml(char *fname, Fmorpho_line fll);
short _mw_fmorpho_line_create_native(char *fname, Fmorpho_line ll,
                                     char *Type);
short _mw_create_fmorpho_line(char *fname, Fmorpho_line ll, char *Type);
Morpho_set _mw_read_ms_mw2_ms(char *fname, FILE * fp, int need_flipping);
Morpho_set _mw_load_ms_mw2_ms(char *fname);
Morpho_set _mw_morpho_set_load_native(char *fname, char *type);
Morpho_set _mw_load_morpho_set(char *fname, char *type);
void _mw_write_ms_mw2_ms(FILE * fp, Morpho_set is);
short _mw_create_ms_mw2_ms(char *fname, Morpho_set is);
short _mw_morpho_set_create_native(char *fname, Morpho_set ms, char *Type);
short _mw_create_morpho_set(char *fname, Morpho_set ms, char *Type);
Morpho_sets _mw_read_mss_mw2_mss(char *fname, FILE * fp, int need_flipping);
Morpho_sets _mw_load_mss_mw2_mss(char *fname);
Morpho_sets _mw_morpho_sets_load_native(char *fname, char *type);
Morpho_sets _mw_load_morpho_sets(char *fname, char *type);
void _mw_write_mss_mw2_mss(FILE * fp, Morpho_sets iss);
short _mw_create_mss_mw2_mss(char *fname, Morpho_sets iss);
short _mw_morpho_sets_create_native(char *fname, Morpho_sets mss, char *Type);
short _mw_create_morpho_sets(char *fname, Morpho_sets mss, char *Type);
Mimage _mw_load_mimage_mw2_mimage(char *fname);
Mimage _mw_mimage_load_native(char *fname, char *type);
Mimage _mw_load_mimage(char *fname, char *type);
short _mw_create_mimage_mw2_mimage(char *fname, Mimage mimage);
short _mw_mimage_create_native(char *fname, Mimage mimage, char *Type);
short _mw_create_mimage(char *fname, Mimage mimage, char *Type);

#endif                          /* !_MIMAGE_IO_H_ */

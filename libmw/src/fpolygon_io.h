/*
 * fpolygon_io.h
 */

#ifndef _FPOLYGON_IO_H_
#define _FPOLYGON_IO_H_

/* src/fpolygon_io.c */
Fpolygon _mw_load_fpolygon_a_fpoly(char *fname);
Fpolygon _mw_fpolygon_load_native(char *fname, char *type);
Fpolygon _mw_load_fpolygon(char *fname, char *type);
short _mw_create_fpolygon_a_fpoly(char *fname, Fpolygon fpoly);
short _mw_fpolygon_create_native(char *fname, Fpolygon fpoly, char *Type);
short _mw_create_fpolygon(char *fname, Fpolygon fpoly, char *Type);
Fpolygons _mw_load_fpolygons_a_fpoly(char *fname);
Fpolygons _mw_fpolygons_load_native(char *fname, char *type);
Fpolygons _mw_load_fpolygons(char *fname, char *type);
short _mw_create_fpolygons_a_fpoly(char *fname, Fpolygons poly);
short _mw_fpolygons_create_native(char *fname, Fpolygons fpoly, char *Type);
short _mw_create_fpolygons(char *fname, Fpolygons fpoly, char *Type);

#endif                          /* !_FPOLYGON_IO_H_ */

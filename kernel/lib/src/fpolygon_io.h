/*
 * fpolygon_io.h
 */

#ifndef _FPOLYGON_IO_H
#define _FPOLYGON_IO_H

Fpolygon _mw_load_fpolygon_a_fpoly(char *);
Fpolygon _mw_fpolygon_load_native(char *, char *);
Fpolygon _mw_load_fpolygon(char *, char *);
short _mw_create_fpolygon_a_fpoly(char *, Fpolygon);
short _mw_fpolygon_create_native(char *, Fpolygon, char *);
short _mw_create_fpolygon(char *, Fpolygon, char *);
Fpolygons _mw_load_fpolygons_a_fpoly(char *);
Fpolygons _mw_fpolygons_load_native(char *, char *);
Fpolygons _mw_load_fpolygons(char *, char *);
short _mw_create_fpolygons_a_fpoly(char *, Fpolygons);
short _mw_fpolygons_create_native(char *, Fpolygons, char *);
short _mw_create_fpolygons(char *, Fpolygons, char *);

#endif /* !_FPOLYGON_IO_H */

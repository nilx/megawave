/*
 * curve_io.h
 */

#ifndef _CURVE_IO_H_
#define _CURVE_IO_H_

/* src/curve_io.c */
Point_curve _mw_point_curve_load_native(char *fname, char *type);
Curve _mw_load_curve_mw2_curve(char *fname);
Curve _mw_curve_load_native(char *fname, char *type);
Curve _mw_load_curve(char *fname, char *type);
short _mw_create_curve_mw2_curve(char *fname, Curve cv);
short _mw_curve_create_native(char *fname, Curve cv, char *Type);
short _mw_create_curve(char *fname, Curve cv, char *Type);
Curves _mw_load_curves_mw2_curves(char *fname);
Curves _mw_curves_load_native(char *fname, char *type);
Curves _mw_load_curves(char *fname, char *type);
short _mw_create_curves_mw2_curves(char *fname, Curves cvs);
short _mw_curves_create_native(char *fname, Curves cvs, char *Type);
short _mw_create_curves(char *fname, Curves cvs, char *Type);

#endif /* !_CURVE_IO_H_ */

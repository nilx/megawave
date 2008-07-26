/*
 * curve_io.h
 */

#ifndef _CURVE_IO_H
#define _CURVE_IO_H

Point_curve _mw_point_curve_load_native(char *, char *);
Curve _mw_load_curve_mw2_curve(char *);
Curve _mw_curve_load_native(char *, char *);
Curve _mw_load_curve(char *, char *);
short _mw_create_curve_mw2_curve(char *, Curve);
short _mw_curve_create_native(char *, Curve, char *);
short _mw_create_curve(char *, Curve, char *);
Curves _mw_load_curves_mw2_curves(char *);
Curves _mw_curves_load_native(char *, char *);
Curves _mw_load_curves(char *, char *);
short _mw_create_curves_mw2_curves(char *, Curves);
short _mw_curves_create_native(char *, Curves, char *);
short _mw_create_curves(char *, Curves, char *);

#endif /* !_CURVE_IO_H */

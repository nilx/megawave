/*
 * fcurve_io.h
 */

#ifndef _FCURVE_IO_H
#define _FCURVE_IO_H
 
Point_fcurve _mw_point_fcurve_load_native(char *, char *);
Fcurve _mw_load_fcurve_mw2_fcurve(char *);
Fcurve _mw_fcurve_load_native(char *, char *);
Fcurve _mw_load_fcurve(char *, char *);
short _mw_create_fcurve_mw2_fcurve(char *, Fcurve);
short _mw_fcurve_create_native(char *, Fcurve, char *);
short _mw_create_fcurve(char *, Fcurve, char *);
Fcurves _mw_load_fcurves_mw2_fcurves_1_00(char *);
Fcurves _mw_load_fcurves_mw2_fcurves(char *);
Fcurves _mw_fcurves_load_native(char *, char *);
Fcurves _mw_load_fcurves(char *, char *);
short _mw_create_fcurves_mw2_fcurves(char *, Fcurves);
short _mw_fcurves_create_native(char *, Fcurves, char *);
short _mw_create_fcurves(char *, Fcurves, char *);

#endif /* !_FCURVE_IO_H */

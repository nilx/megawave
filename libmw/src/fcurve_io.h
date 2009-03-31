/*
 * fcurve_io.h
 */

#ifndef _FCURVE_IO_H_
#define _FCURVE_IO_H_

/* src/fcurve_io.c */
Point_fcurve _mw_point_fcurve_load_native(char *fname, char *type);
Fcurve _mw_load_fcurve_mw2_fcurve(char *fname);
Fcurve _mw_fcurve_load_native(char *fname, char *type);
Fcurve _mw_load_fcurve(char *fname, char *type);
short _mw_create_fcurve_mw2_fcurve(char *fname, Fcurve cv);
short _mw_fcurve_create_native(char *fname, Fcurve cv, char *Type);
short _mw_create_fcurve(char *fname, Fcurve cv, char *Type);
Fcurves _mw_load_fcurves_mw2_fcurves_1_00(char *fname);
Fcurves _mw_load_fcurves_mw2_fcurves(char *fname);
Fcurves _mw_fcurves_load_native(char *fname, char *type);
Fcurves _mw_load_fcurves(char *fname, char *type);
short _mw_create_fcurves_mw2_fcurves(char *fname, Fcurves cvs);
short _mw_fcurves_create_native(char *fname, Fcurves cvs, char *Type);
short _mw_create_fcurves(char *fname, Fcurves cvs, char *Type);

#endif                          /* !_FCURVE_IO_H_ */

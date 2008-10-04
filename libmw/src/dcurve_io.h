/*
 * dcurve_io.h
 */

#ifndef _DCURVE_IO_H_
#define _DCURVE_IO_H_

/* src/dcurve_io.c */
Point_dcurve _mw_point_dcurve_load_native(char *fname, char *type);
Dcurve _mw_load_dcurve_mw2_dcurve(char *fname);
Dcurve _mw_dcurve_load_native(char *fname, char *type);
Dcurve _mw_load_dcurve(char *fname, char *type);
short _mw_create_dcurve_mw2_dcurve(char *fname, Dcurve cv);
short _mw_dcurve_create_native(char *fname, Dcurve cv, char *Type);
short _mw_create_dcurve(char *fname, Dcurve cv, char *Type);
Dcurves _mw_load_dcurves_mw2_dcurves_1_00(char *fname);
Dcurves _mw_load_dcurves_mw2_dcurves(char *fname);
Dcurves _mw_dcurves_load_native(char *fname, char *type);
Dcurves _mw_load_dcurves(char *fname, char *type);
short _mw_create_dcurves_mw2_dcurves(char *fname, Dcurves cvs);
short _mw_dcurves_create_native(char *fname, Dcurves cvs, char *Type);
short _mw_create_dcurves(char *fname, Dcurves cvs, char *Type);

#endif /* !_DCURVE_IO_H_ */

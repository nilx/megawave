/*
 * dcurve_io.h
 */

#ifndef _DCURVE_IO_H
#define _DCURVE_IO_H

Point_dcurve _mw_point_dcurve_load_native(char *, char *);
Dcurve _mw_load_dcurve_mw2_dcurve(char *);
Dcurve _mw_dcurve_load_native(char *, char *);
Dcurve _mw_load_dcurve(char *, char *);
short _mw_create_dcurve_mw2_dcurve(char *, Dcurve);
short _mw_dcurve_create_native(char *, Dcurve, char *);
short _mw_create_dcurve(char *, Dcurve, char *);
Dcurves _mw_load_dcurves_mw2_dcurves_1_00(char *);
Dcurves _mw_load_dcurves_mw2_dcurves(char *);
Dcurves _mw_dcurves_load_native(char *, char *);
Dcurves _mw_load_dcurves(char *, char *);
short _mw_create_dcurves_mw2_dcurves(char *, Dcurves);
short _mw_dcurves_create_native(char *, Dcurves, char *);
short _mw_create_dcurves(char *, Dcurves, char *);

#endif /* !_DCURVE_IO_H */

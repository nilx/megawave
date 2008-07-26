/*
 * dcurve.h
 */

#ifndef _DCURVE_H
#define _DCURVE_H

Point_dcurve mw_new_point_dcurve(void);
Point_dcurve mw_change_point_dcurve(Point_dcurve);
void mw_delete_point_dcurve(Point_dcurve);
Point_dcurve mw_copy_point_dcurve(Point_dcurve, Point_dcurve);
Dcurve mw_new_dcurve(void);
Dcurve mw_change_dcurve(Dcurve);
void mw_delete_dcurve(Dcurve);
Dcurve mw_copy_dcurve(Dcurve, Dcurve);
unsigned int mw_length_dcurve(Dcurve);
Dcurves mw_new_dcurves(void);
Dcurves mw_change_dcurves(Dcurves);
void mw_delete_dcurves(Dcurves);
unsigned int mw_length_dcurves(Dcurves);
unsigned int mw_npoints_dcurves(Dcurves);

#endif /* !_DCURVE_H */

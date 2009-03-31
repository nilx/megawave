/*
 * dcurve.h
 */

#ifndef _DCURVE_H_
#define _DCURVE_H_

/* src/dcurve.c */
Point_dcurve mw_new_point_dcurve(void);
Point_dcurve mw_change_point_dcurve(Point_dcurve point);
void mw_delete_point_dcurve(Point_dcurve point);
Point_dcurve mw_copy_point_dcurve(Point_dcurve in, Point_dcurve out);
Dcurve mw_new_dcurve(void);
Dcurve mw_change_dcurve(Dcurve cv);
void mw_delete_dcurve(Dcurve dcurve);
Dcurve mw_copy_dcurve(Dcurve in, Dcurve out);
unsigned int mw_length_dcurve(Dcurve dcurve);
Dcurves mw_new_dcurves(void);
Dcurves mw_change_dcurves(Dcurves cvs);
void mw_delete_dcurves(Dcurves dcurves);
unsigned int mw_length_dcurves(Dcurves dcurves);
unsigned int mw_npoints_dcurves(Dcurves dcurves);

#endif                          /* !_DCURVE_H_ */

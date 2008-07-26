/*
 * curve.h
 */

#ifndef _CURVE_H
#define _CURVE_H

Point_curve mw_new_point_curve(void);
Point_curve mw_change_point_curve(Point_curve);
void mw_delete_point_curve(Point_curve);
Point_curve mw_copy_point_curve(Point_curve,Point_curve);
Curve mw_new_curve(void);
Curve mw_change_curve(Curve);
void mw_delete_curve(Curve);
Curve mw_copy_curve(Curve, Curve);
unsigned int mw_length_curve(Curve);
Curves mw_new_curves(void);
Curves mw_change_curves(Curves);
void mw_delete_curves(Curves);
unsigned int mw_length_curves(Curves);
unsigned int mw_npoints_curves(Curves);

#endif /* !_CURVE_H */

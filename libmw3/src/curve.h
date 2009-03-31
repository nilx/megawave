/*
 * curve.h
 */

#ifndef _CURVE_H_
#define _CURVE_H_

/* src/curve.c */
Point_curve mw_new_point_curve(void);
Point_curve mw_change_point_curve(Point_curve point);
void mw_delete_point_curve(Point_curve point);
Point_curve mw_copy_point_curve(Point_curve in, Point_curve out);
Curve mw_new_curve(void);
Curve mw_change_curve(Curve cv);
void mw_delete_curve(Curve curve);
Curve mw_copy_curve(Curve in, Curve out);
unsigned int mw_length_curve(Curve curve);
Curves mw_new_curves(void);
Curves mw_change_curves(Curves cvs);
void mw_delete_curves(Curves curves);
unsigned int mw_length_curves(Curves curves);
unsigned int mw_npoints_curves(Curves curves);

#endif                          /* !_CURVE_H_ */

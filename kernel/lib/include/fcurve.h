/*
 * fcurve.h
 */

#ifndef _FCURVE_H
#define _FCURVE_H
 
Point_fcurve mw_new_point_fcurve(void);
Point_fcurve mw_change_point_fcurve(Point_fcurve);
void mw_delete_point_fcurve(Point_fcurve);
Point_fcurve mw_copy_point_fcurve(Point_fcurve, Point_fcurve);
Fcurve mw_new_fcurve(void);
Fcurve mw_change_fcurve(Fcurve);
void mw_delete_fcurve(Fcurve);
Fcurve mw_copy_fcurve(Fcurve, Fcurve);
unsigned int mw_length_fcurve(Fcurve);
Fcurves mw_new_fcurves(void);
Fcurves mw_change_fcurves(Fcurves);
void mw_delete_fcurves(Fcurves);
unsigned int mw_length_fcurves(Fcurves);
unsigned int mw_npoints_fcurves(Fcurves);

#endif /* !_FCURVE_H */

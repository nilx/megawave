/*
 * fcurve.h
 */

#ifndef _FCURVE_H_
#define _FCURVE_H_

/* src/fcurve.c */
Point_fcurve mw_new_point_fcurve(void);
Point_fcurve mw_change_point_fcurve(Point_fcurve point);
void mw_delete_point_fcurve(Point_fcurve point);
Point_fcurve mw_copy_point_fcurve(Point_fcurve in, Point_fcurve out);
Fcurve mw_new_fcurve(void);
Fcurve mw_change_fcurve(Fcurve cv);
void mw_delete_fcurve(Fcurve fcurve);
Fcurve mw_copy_fcurve(Fcurve in, Fcurve out);
unsigned int mw_length_fcurve(Fcurve fcurve);
Fcurves mw_new_fcurves(void);
Fcurves mw_change_fcurves(Fcurves cvs);
void mw_delete_fcurves(Fcurves fcurves);
unsigned int mw_length_fcurves(Fcurves fcurves);
unsigned int mw_npoints_fcurves(Fcurves fcurves);

#endif /* !_FCURVE_H_ */

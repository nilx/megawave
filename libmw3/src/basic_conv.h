/*
 * basic_conv.h
 */

#ifndef _BASIC_CONV_H_
#define _BASIC_CONV_H_

/* src/basic_conv.c */
void _mw_float_to_uchar(register float *ptr_float,
                        register unsigned char *ptr_uchar, int N,
                        char *data_name);
void _mw_uchar_to_float(register unsigned char *ptr_uchar,
                        register float *ptr_float, int N);
void _mw_1x24XV_to_3x8_ucharplanes(register unsigned char *ptr,
                                   register unsigned char *ptr1,
                                   register unsigned char *ptr2,
                                   register unsigned char *ptr3, int N);
void _mw_3x8_to_1x24XV_ucharplanes(register unsigned char *ptr1,
                                   register unsigned char *ptr2,
                                   register unsigned char *ptr3,
                                   register unsigned char *ptr, int N);
Cimage mw_fimage_to_cimage(Fimage image_fimage, Cimage image);
Fimage mw_cimage_to_fimage(Cimage image_cimage, Fimage image);
Ccimage mw_cfimage_to_ccimage(Cfimage image_cfimage, Ccimage image);
Cfimage mw_ccimage_to_cfimage(Ccimage image_ccimage, Cfimage image);
Fimage mw_cfimage_to_fimage(Cfimage image_cfimage, Fimage image);
Cfimage mw_fimage_to_cfimage(Fimage image_fimage, Cfimage image);
Fimage mw_ccimage_to_fimage(Ccimage image_ccimage, Fimage image);
Cimage mw_ccimage_to_cimage(Ccimage image_ccimage, Cimage image);
Ccimage mw_cimage_to_ccimage(Cimage image_cimage, Ccimage image);
Polygons mw_curves_to_polygons(Curves curves, Polygons polys);
Fpolygons mw_fcurves_to_fpolygons(Fcurves fcurves, Fpolygons fpolys);
Fcurves mw_curves_to_fcurves(Curves curves, Fcurves fcurves);
Curves mw_fcurves_to_curves(Fcurves fcurves, Curves curves);
Fcurves mw_dcurves_to_fcurves(Dcurves dcurves, Fcurves fcurves);
Dcurves mw_fcurves_to_dcurves(Fcurves fcurves, Dcurves dcurves);
Fpolygons mw_polygons_to_fpolygons(Polygons polys, Fpolygons fpolys);
Polygons mw_fpolygons_to_polygons(Fpolygons fpolys, Polygons polys);
Fcurves mw_fpolygons_to_fcurves(Fpolygons fpolys, Fcurves fcurves);
Curves mw_polygons_to_curves(Polygons polys, Curves curves);
Polygon mw_curve_to_polygon(Curve curve, Polygon poly);
Fpolygon mw_fcurve_to_fpolygon(Fcurve fcurve, Fpolygon fpoly);
Point_fcurve mw_point_curve_to_point_fcurve(Point_curve pcurve,
                                            Point_fcurve first_point);
Fcurve mw_curve_to_fcurve(Curve curve, Fcurve fcurve);
Point_curve mw_point_fcurve_to_point_curve(Point_fcurve pfcurve,
                                           Point_curve first_point);
Curve mw_fcurve_to_curve(Fcurve fcurve, Curve curve);
Point_fcurve mw_point_dcurve_to_point_fcurve(Point_dcurve pcurve,
                                             Point_fcurve first_point);
Fcurve mw_dcurve_to_fcurve(Dcurve dcurve, Fcurve fcurve);
Point_dcurve mw_point_fcurve_to_point_dcurve(Point_fcurve pfcurve,
                                             Point_dcurve first_point);
Dcurve mw_fcurve_to_dcurve(Fcurve fcurve, Dcurve dcurve);
Fpolygon mw_polygon_to_fpolygon(Polygon poly, Fpolygon fpoly);
Polygon mw_fpolygon_to_polygon(Fpolygon fpoly, Polygon poly);
Fcurve mw_fpolygon_to_fcurve(Fpolygon fpoly, Fcurve fcurve);
Curve mw_polygon_to_curve(Polygon poly, Curve curve);
Curve mw_curves_to_curve(Curves curves, Curve curve);
Curves mw_curve_to_curves(Curve curve, Curves curves);
Fcurve mw_fcurves_to_fcurve(Fcurves fcurves, Fcurve fcurve);
Fcurves mw_fcurve_to_fcurves(Fcurve fcurve, Fcurves fcurves);
Morpho_line mw_curve_to_morpho_line(Curve curve, Morpho_line ll);
Curve mw_morpho_line_to_curve(Morpho_line ll, Curve cv);
Fmorpho_line mw_fcurve_to_fmorpho_line(Fcurve fcurve, Fmorpho_line fll);
Fcurve mw_fmorpho_line_to_fcurve(Fmorpho_line fll, Fcurve cv);
Fmorpho_line mw_morpho_line_to_fmorpho_line(Morpho_line ll, Fmorpho_line fll);
Morpho_line mw_fmorpho_line_to_morpho_line(Fmorpho_line fll, Morpho_line ll);
Mimage mw_morpho_line_to_mimage(Morpho_line ll, Mimage mimage);
Morpho_line mw_mimage_to_morpho_line(Mimage mimage, Morpho_line ll);
Mimage mw_fmorpho_line_to_mimage(Fmorpho_line fll, Mimage mimage);
Fmorpho_line mw_mimage_to_fmorpho_line(Mimage mimage, Fmorpho_line fll);
Curves mw_mimage_to_curves(Mimage mimage, Curves curves);
Mimage mw_curves_to_mimage(Curves curves, Mimage mimage);
Fcurves mw_mimage_to_fcurves(Mimage mimage, Fcurves fcurves);
Mimage mw_fcurves_to_mimage(Fcurves fcurves, Mimage mimage);
Morpho_sets mw_morpho_set_to_morpho_sets(Morpho_set is,
                                         Morpho_sets morpho_sets);
Morpho_set mw_morpho_sets_to_morpho_set(Morpho_sets morpho_sets,
                                        Morpho_set is);
Mimage mw_morpho_sets_to_mimage(Morpho_sets iss, Mimage mimage);
Morpho_sets mw_mimage_to_morpho_sets(Mimage mimage, Morpho_sets iss);
Cmorpho_line mw_morpho_line_to_cmorpho_line(Morpho_line ll, Cmorpho_line cll);
Morpho_line mw_cmorpho_line_to_morpho_line(Cmorpho_line cll, Morpho_line ll);
Cfmorpho_line mw_fmorpho_line_to_cfmorpho_line(Fmorpho_line ll,
                                               Cfmorpho_line cll);
Fmorpho_line mw_cfmorpho_line_to_fmorpho_line(Cfmorpho_line cll,
                                              Fmorpho_line ll);
Cfmorpho_line mw_cmorpho_line_to_cfmorpho_line(Cmorpho_line ll,
                                               Cfmorpho_line fll);
Cmorpho_line mw_cfmorpho_line_to_cmorpho_line(Cfmorpho_line fll,
                                              Cmorpho_line ll);
Cmimage mw_cmorpho_line_to_cmimage(Cmorpho_line ll, Cmimage cmimage);
Cmorpho_line mw_cmimage_to_cmorpho_line(Cmimage cmimage, Cmorpho_line ll);
Cmorpho_line mw_curve_to_cmorpho_line(Curve curve, Cmorpho_line ll);
Curve mw_cmorpho_line_to_curve(Cmorpho_line ll, Curve cv);
Cfmorpho_line mw_fcurve_to_cfmorpho_line(Fcurve fcurve, Cfmorpho_line fll);
Fcurve mw_cfmorpho_line_to_fcurve(Cfmorpho_line fll, Fcurve cv);
Curves mw_cmimage_to_curves(Cmimage cmimage, Curves curves);
Cmimage mw_curves_to_cmimage(Curves curves, Cmimage cmimage);
Fcurves mw_cmimage_to_fcurves(Cmimage cmimage, Fcurves fcurves);
Cmimage mw_fcurves_to_cmimage(Fcurves fcurves, Cmimage cmimage);
Cmimage mw_cmorpho_sets_to_cmimage(Cmorpho_sets iss, Cmimage cmimage);
Cmorpho_sets mw_cmimage_to_cmorpho_sets(Cmimage cmimage, Cmorpho_sets iss);
Cmorpho_sets mw_cmorpho_set_to_cmorpho_sets(Cmorpho_set is,
                                            Cmorpho_sets cmorpho_sets);
Cmorpho_set mw_cmorpho_sets_to_cmorpho_set(Cmorpho_sets cmorpho_sets,
                                           Cmorpho_set is);
Cmimage mw_cfmorpho_line_to_cmimage(Cfmorpho_line fll, Cmimage cmimage);
Cfmorpho_line mw_cmimage_to_cfmorpho_line(Cmimage cmimage, Cfmorpho_line fll);
Cmimage mw_mimage_to_cmimage(Mimage mimage, Cmimage cmimage);
Mimage mw_cmimage_to_mimage(Cmimage cmimage, Mimage mimage);
Flist mw_flists_to_flist(Flists ls, Flist l);
Flists mw_flist_to_flists(Flist l, Flists ls);
Flist mw_fcurve_to_flist(Fcurve c, Flist l);
Fcurve mw_flist_to_fcurve(Flist l, Fcurve c);
Flists mw_fcurves_to_flists(Fcurves cs, Flists ls);
Fcurves mw_flists_to_fcurves(Flists ls, Fcurves cs);
Dlist mw_dlists_to_dlist(Dlists ls, Dlist l);
Dlists mw_dlist_to_dlists(Dlist l, Dlists ls);
Dlist mw_dcurve_to_dlist(Dcurve c, Dlist l);
Dcurve mw_dlist_to_dcurve(Dlist l, Dcurve c);
Dlists mw_dcurves_to_dlists(Dcurves cs, Dlists ls);
Dcurves mw_dlists_to_dcurves(Dlists ls, Dcurves cs);
Dlist mw_flist_to_dlist(Flist in, Dlist out);
Flist mw_dlist_to_flist(Dlist in, Flist out);
Dlists mw_flists_to_dlists(Flists in, Dlists out);
Flists mw_dlists_to_flists(Dlists in, Flists out);

#endif                          /* !_BASIC_CONV_H_ */

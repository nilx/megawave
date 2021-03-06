/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   basic_conv.h
   
   Vers. 1.1
   (C) 2000-2001 Jacques Froment
   Prototypes for conversion functions of internal MegaWave2 formats
   All documented conversion functions in the Guid #2 should be prototyped !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef basic_conv_flg
#define basic_conv_flg

#ifdef __STDC__

void _mw_float_to_uchar(register float *, register unsigned char *, int, char [8192   ]);
void _mw_uchar_to_float(register unsigned char *, register float *, int);
void _mw_1x24XV_to_3x8_ucharplanes(register unsigned char *, register unsigned char *, register unsigned char *, register unsigned char *, int);
void _mw_3x8_to_1x24XV_ucharplanes(register unsigned char *, register unsigned char *, register unsigned char *, register unsigned char *, int);
Cimage mw_fimage_to_cimage(Fimage, Cimage);
Fimage mw_cimage_to_fimage(Cimage, Fimage);
Ccimage mw_cfimage_to_ccimage(Cfimage, Ccimage);
Cfimage mw_ccimage_to_cfimage(Ccimage, Cfimage);
Fimage mw_cfimage_to_fimage(Cfimage, Fimage);
Cfimage mw_fimage_to_cfimage(Fimage, Cfimage);
Cimage mw_ccimage_to_cimage(Ccimage, Cimage);
Ccimage mw_cimage_to_ccimage(Cimage, Ccimage);
Polygons mw_curves_to_polygons(Curves, Polygons);
Fpolygons mw_fcurves_to_fpolygons(Fcurves, Fpolygons);
Fcurves mw_curves_to_fcurves(Curves, Fcurves);
Curves mw_fcurves_to_curves(Fcurves, Curves);
Fcurves mw_dcurves_to_fcurves(Dcurves, Fcurves);
Dcurves mw_fcurves_to_dcurves(Fcurves, Dcurves);
Fpolygons mw_polygons_to_fpolygons(Polygons, Fpolygons);
Polygons mw_fpolygons_to_polygons(Fpolygons, Polygons);
Fcurves mw_fpolygons_to_fcurves(Fpolygons, Fcurves);
Curves mw_polygons_to_curves(Polygons, Curves);
Polygon mw_curve_to_polygon(Curve, Polygon);
Fpolygon mw_fcurve_to_fpolygon(Fcurve, Fpolygon);
Point_fcurve mw_point_curve_to_point_fcurve(Point_curve, Point_fcurve);
Fcurve mw_curve_to_fcurve(Curve, Fcurve);
Point_curve mw_point_fcurve_to_point_curve(Point_fcurve, Point_curve);
Curve mw_fcurve_to_curve(Fcurve, Curve);
Point_fcurve mw_point_dcurve_to_point_fcurve(Point_dcurve, Point_fcurve);
Fcurve mw_dcurve_to_fcurve(Dcurve, Fcurve);
Point_dcurve mw_point_fcurve_to_point_dcurve(Point_fcurve, Point_dcurve);
Dcurve mw_fcurve_to_dcurve(Fcurve, Dcurve);
Fpolygon mw_polygon_to_fpolygon(Polygon, Fpolygon);
Polygon mw_fpolygon_to_polygon(Fpolygon, Polygon);
Fcurve mw_fpolygon_to_fcurve(Fpolygon, Fcurve);
Curve mw_polygon_to_curve(Polygon, Curve);
Curve mw_curves_to_curve(Curves, Curve);
Curves mw_curve_to_curves(Curve, Curves);
Fcurve mw_fcurves_to_fcurve(Fcurves, Fcurve);
Fcurves mw_fcurve_to_fcurves(Fcurve, Fcurves);
Morpho_line mw_curve_to_morpho_line(Curve, Morpho_line);
Curve mw_morpho_line_to_curve(Morpho_line, Curve);
Fmorpho_line mw_fcurve_to_fmorpho_line(Fcurve, Fmorpho_line);
Fcurve mw_fmorpho_line_to_fcurve(Fmorpho_line, Fcurve);
Fmorpho_line mw_morpho_line_to_fmorpho_line(Morpho_line, Fmorpho_line);
Morpho_line mw_fmorpho_line_to_morpho_line(Fmorpho_line, Morpho_line);
Mimage mw_morpho_line_to_mimage(Morpho_line, Mimage);
Morpho_line mw_mimage_to_morpho_line(Mimage, Morpho_line);
Mimage mw_fmorpho_line_to_mimage(Fmorpho_line, Mimage);
Fmorpho_line mw_mimage_to_fmorpho_line(Mimage, Fmorpho_line);
Curves mw_mimage_to_curves(Mimage, Curves);
Mimage mw_curves_to_mimage(Curves, Mimage);
Fcurves mw_mimage_to_fcurves(Mimage, Fcurves);
Mimage mw_fcurves_to_mimage(Fcurves, Mimage);
Morpho_sets mw_morpho_set_to_morpho_sets(Morpho_set, Morpho_sets);
Morpho_set mw_morpho_sets_to_morpho_set(Morpho_sets, Morpho_set);
Mimage mw_morpho_sets_to_mimage(Morpho_sets, Mimage);
Morpho_sets mw_mimage_to_morpho_sets(Mimage, Morpho_sets);
Cmorpho_line mw_morpho_line_to_cmorpho_line(Morpho_line, Cmorpho_line);
Morpho_line mw_cmorpho_line_to_morpho_line(Cmorpho_line, Morpho_line);
Cfmorpho_line mw_fmorpho_line_to_cfmorpho_line(Fmorpho_line, Cfmorpho_line);
Fmorpho_line mw_cfmorpho_line_to_fmorpho_line(Cfmorpho_line, Fmorpho_line);
Cfmorpho_line mw_cmorpho_line_to_cfmorpho_line(Cmorpho_line, Cfmorpho_line);
Cmorpho_line mw_cfmorpho_line_to_cmorpho_line(Cfmorpho_line, Cmorpho_line);
Cmimage mw_cmorpho_line_to_cmimage(Cmorpho_line, Cmimage);
Cmorpho_line mw_cmimage_to_cmorpho_line(Cmimage, Cmorpho_line);
Cmorpho_line mw_curve_to_cmorpho_line(Curve, Cmorpho_line);
Curve mw_cmorpho_line_to_curve(Cmorpho_line, Curve);
Cfmorpho_line mw_fcurve_to_cfmorpho_line(Fcurve, Cfmorpho_line);
Fcurve mw_cfmorpho_line_to_fcurve(Cfmorpho_line, Fcurve);
Curves mw_cmimage_to_curves(Cmimage, Curves);
Cmimage mw_curves_to_cmimage(Curves, Cmimage);
Fcurves mw_cmimage_to_fcurves(Cmimage, Fcurves);
Cmimage mw_fcurves_to_cmimage(Fcurves, Cmimage);
Cmimage mw_cmorpho_sets_to_cmimage(Cmorpho_sets, Cmimage);
Cmorpho_sets mw_cmimage_to_cmorpho_sets(Cmimage, Cmorpho_sets);
Cmorpho_sets mw_cmorpho_set_to_cmorpho_sets(Cmorpho_set, Cmorpho_sets);
Cmorpho_set mw_cmorpho_sets_to_cmorpho_set(Cmorpho_sets, Cmorpho_set);
Cmimage mw_cfmorpho_line_to_cmimage(Cfmorpho_line, Cmimage);
Cfmorpho_line mw_cmimage_to_cfmorpho_line(Cmimage, Cfmorpho_line);
Cmimage mw_mimage_to_cmimage(Mimage, Cmimage);
Mimage mw_cmimage_to_mimage(Cmimage, Mimage);
Flist mw_fcurve_to_flist(Fcurve, Flist);
Fcurve mw_flist_to_fcurve(Flist, Fcurve);
Fimage mw_flist_to_fimage(Flist, Fimage);
Flist mw_fimage_to_flist(Fimage, Flist);
Flists mw_fcurves_to_flists(Fcurves, Flists);
Fcurves mw_flists_to_fcurves(Flists, Fcurves);
Dlist mw_dcurve_to_dlist(Dcurve, Dlist);
Dcurve mw_dlist_to_dcurve(Dlist, Dcurve);
Dlists mw_dcurves_to_dlists(Dcurves, Dlists);
Dcurves mw_dlists_to_dcurves(Dlists, Dcurves);
Dlist mw_flist_to_dlist(Flist, Dlist);
Flist mw_dlist_to_flist(Dlist, Flist);
Dlists mw_flists_to_dlists(Flists, Dlists);
Flists mw_dlists_to_flists(Dlists, Flists);

#else

void _mw_float_to_uchar();
void _mw_uchar_to_float();
void _mw_1x24XV_to_3x8_ucharplanes();
void _mw_3x8_to_1x24XV_ucharplanes();
Cimage mw_fimage_to_cimage();
Fimage mw_cimage_to_fimage();
Ccimage mw_cfimage_to_ccimage();
Cfimage mw_ccimage_to_cfimage();
Fimage mw_cfimage_to_fimage();
Cfimage mw_fimage_to_cfimage();
Cimage mw_ccimage_to_cimage();
Ccimage mw_cimage_to_ccimage();
Polygons mw_curves_to_polygons();
Fpolygons mw_fcurves_to_fpolygons();
Fcurves mw_curves_to_fcurves();
Curves mw_fcurves_to_curves();
Fcurves mw_dcurves_to_fcurves();
Dcurves mw_fcurves_to_dcurves();
Fpolygons mw_polygons_to_fpolygons();
Polygons mw_fpolygons_to_polygons();
Fcurves mw_fpolygons_to_fcurves();
Curves mw_polygons_to_curves();
Polygon mw_curve_to_polygon();
Fpolygon mw_fcurve_to_fpolygon();
Point_fcurve mw_point_curve_to_point_fcurve();
Fcurve mw_curve_to_fcurve();
Point_curve mw_point_fcurve_to_point_curve();
Curve mw_fcurve_to_curve();
Point_fcurve mw_point_dcurve_to_point_fcurve();
Fcurve mw_dcurve_to_fcurve();
Point_dcurve mw_point_fcurve_to_point_dcurve();
Dcurve mw_fcurve_to_dcurve();
Fpolygon mw_polygon_to_fpolygon();
Polygon mw_fpolygon_to_polygon();
Fcurve mw_fpolygon_to_fcurve();
Curve mw_polygon_to_curve();
Curve mw_curves_to_curve();
Curves mw_curve_to_curves();
Fcurve mw_fcurves_to_fcurve();
Fcurves mw_fcurve_to_fcurves();
Morpho_line mw_curve_to_morpho_line();
Curve mw_morpho_line_to_curve();
Fmorpho_line mw_fcurve_to_fmorpho_line();
Fcurve mw_fmorpho_line_to_fcurve();
Fmorpho_line mw_morpho_line_to_fmorpho_line();
Morpho_line mw_fmorpho_line_to_morpho_line();
Mimage mw_morpho_line_to_mimage();
Morpho_line mw_mimage_to_morpho_line();
Mimage mw_fmorpho_line_to_mimage();
Fmorpho_line mw_mimage_to_fmorpho_line();
Curves mw_mimage_to_curves();
Mimage mw_curves_to_mimage();
Fcurves mw_mimage_to_fcurves();
Mimage mw_fcurves_to_mimage();
Morpho_sets mw_morpho_set_to_morpho_sets();
Morpho_set mw_morpho_sets_to_morpho_set();
Mimage mw_morpho_sets_to_mimage();
Morpho_sets mw_mimage_to_morpho_sets();
Cmorpho_line mw_morpho_line_to_cmorpho_line();
Morpho_line mw_cmorpho_line_to_morpho_line();
Cfmorpho_line mw_fmorpho_line_to_cfmorpho_line();
Fmorpho_line mw_cfmorpho_line_to_fmorpho_line();
Cfmorpho_line mw_cmorpho_line_to_cfmorpho_line();
Cmorpho_line mw_cfmorpho_line_to_cmorpho_line();
Cmimage mw_cmorpho_line_to_cmimage();
Cmorpho_line mw_cmimage_to_cmorpho_line();
Cmorpho_line mw_curve_to_cmorpho_line();
Curve mw_cmorpho_line_to_curve();
Cfmorpho_line mw_fcurve_to_cfmorpho_line();
Fcurve mw_cfmorpho_line_to_fcurve();
Curves mw_cmimage_to_curves();
Cmimage mw_curves_to_cmimage();
Fcurves mw_cmimage_to_fcurves();
Cmimage mw_fcurves_to_cmimage();
Cmimage mw_cmorpho_sets_to_cmimage();
Cmorpho_sets mw_cmimage_to_cmorpho_sets();
Cmorpho_sets mw_cmorpho_set_to_cmorpho_sets();
Cmorpho_set mw_cmorpho_sets_to_cmorpho_set();
Cmimage mw_cfmorpho_line_to_cmimage();
Cfmorpho_line mw_cmimage_to_cfmorpho_line();
Cmimage mw_mimage_to_cmimage();
Mimage mw_cmimage_to_mimage();
Flist mw_fcurve_to_flist();
Fcurve mw_flist_to_fcurve();
Fimage mw_flist_to_fimage();
Flist mw_fimage_to_flist();
Flists mw_fcurves_to_flists();
Fcurves mw_flists_to_fcurves();
Dlist mw_dcurve_to_dlist();
Dcurve mw_dlist_to_dcurve();
Dlists mw_dcurves_to_dlists();
Dcurves mw_dlists_to_dcurves();
Dlist mw_flist_to_dlist();
Flist mw_dlist_to_flist();
Dlists mw_flists_to_dlists();
Flists mw_dlists_to_flists();

#endif

#endif

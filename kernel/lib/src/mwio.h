/*
 * mwio.h
 */

#ifndef _MWIO_H
#define _MWIO_H

void _mw_flip_image(register unsigned char *, short, short, short, char);
FILE *_mw_write_header_file(char *, char *, float);
int _search_filename(char *);
long _mw_find_pattern_in_file(FILE *, char *);
int _mw_byte_ordering_is_little_endian(void);
short _mwload_cimage(char *, char [], char [], Cimage *);
short _mwsave_cimage(char *, char [], char [], char [], Cimage);
short _mwload_fimage(char *, char [], char [], Fimage *);
short _mwsave_fimage(char *, char [], char [], char [], Fimage);
short _mwload_cmovie(char *, char [], char [], Cmovie *);
short _mwsave_cmovie(char *, char [], char [], char [], Cmovie);
short _mwload_fmovie(char *, char [], char [], Fmovie *);
short _mwsave_fmovie(char *, char [], char [], char [], Fmovie);
short _mwload_ccmovie(char *, char [], char [], Ccmovie *);
short _mwsave_ccmovie(char *, char [], char [], char [], Ccmovie);
short _mwload_cfmovie(char *, char [], char [], Cfmovie *);
short _mwsave_cfmovie(char *, char [], char [], char [], Cfmovie);
short _mwload_curve(char *, char [], char [], Curve *);
short _mwsave_curve(char *, char [], char [], char [], Curve);
short _mwload_curves(char *, char [], char [], Curves *);
short _mwsave_curves(char *, char [], char [], char [], Curves);
short _mwload_polygon(char *, char [], char [], Polygon *);
short _mwsave_polygon(char *, char [], char [], char [], Polygon);
short _mwload_polygons(char *, char [], char [], Polygons *);
short _mwsave_polygons(char *, char [], char [], char [], Polygons);
short _mwload_fcurve(char *, char [], char [], Fcurve *);
short _mwsave_fcurve(char *, char [], char [], char [], Fcurve);
short _mwload_fcurves(char *, char [], char [], Fcurves *);
short _mwsave_fcurves(char *, char [], char [], char [], Fcurves);
short _mwload_fpolygon(char *, char [], char [], Fpolygon *);
short _mwsave_fpolygon(char *, char [], char [], char [], Fpolygon);
short _mwload_fpolygons(char *, char [], char [], Fpolygons *);
short _mwsave_fpolygons(char *, char [], char [], char [], Fpolygons);
short _mwload_fsignal(char *, char [], char [], Fsignal *);
short _mwsave_fsignal(char *, char [], char [], char [], Fsignal);
short _mwload_wtrans1d(char *, char [], char [], Wtrans1d *);
short _mwsave_wtrans1d(char *, char [], char [], char [], Wtrans1d);
short _mwload_wtrans2d(char *, char [], char [], Wtrans2d *);
short _mwsave_wtrans2d(char *, char [], char [], char [], Wtrans2d);
short _mwload_vchain_wmax(char *, char [], char [], Vchain_wmax *);
short _mwsave_vchain_wmax(char *, char [], char [], char [], Vchain_wmax);
short _mwload_vchains_wmax(char *, char [], char [], Vchains_wmax *);
short _mwsave_vchains_wmax(char *, char [], char [], char [], Vchains_wmax);
short _mwload_ccimage(char *, char [], char [], Ccimage *);
short _mwsave_ccimage(char *, char [], char [], char [], Ccimage);
short _mwload_cfimage(char *, char [], char [], Cfimage *);
short _mwsave_cfimage(char *, char [], char [], char [], Cfimage);
short _mwload_modules(char *, char [], char [], Modules *);
short _mwsave_modules(char *, char [], char [], char [], Modules);
short _mwload_morpho_line(char *, char [], char [], Morpho_line *);
short _mwsave_morpho_line(char *, char [], char [], char [], Morpho_line);
short _mwload_fmorpho_line(char *, char [], char [], Fmorpho_line *);
short _mwsave_fmorpho_line(char *, char [], char [], char [], Fmorpho_line);
short _mwload_morpho_set(char *, char [], char [], Morpho_set *);
short _mwsave_morpho_set(char *, char [], char [], char [], Morpho_set);
short _mwload_morpho_sets(char *, char [], char [], Morpho_sets *);
short _mwsave_morpho_sets(char *, char [], char [], char [], Morpho_sets);
short _mwload_mimage(char *, char [], char [], Mimage *);
short _mwsave_mimage(char *, char [], char [], char [], Mimage);
short _mwload_cmorpho_line(char *, char [], char [], Cmorpho_line *);
short _mwsave_cmorpho_line(char *, char [], char [], char [], Cmorpho_line);
short _mwload_cfmorpho_line(char *, char [], char [], Cfmorpho_line *);
short _mwsave_cfmorpho_line(char *, char [], char [], char [], Cfmorpho_line);
short _mwload_cmorpho_set(char *, char [], char [], Cmorpho_set *);
short _mwsave_cmorpho_set(char *, char [], char [], char [], Cmorpho_set);
short _mwload_cmorpho_sets(char *, char [], char [], Cmorpho_sets *);
short _mwsave_cmorpho_sets(char *, char [], char [], char [], Cmorpho_sets);
short _mwload_cmimage(char *, char [], char [], Cmimage *);
short _mwsave_cmimage(char *, char [], char [], char [], Cmimage);
short _mwload_shape(char *, char [], char [], Shape *);
short _mwsave_shape(char *, char [], char [], char [], Shape);
short _mwload_shapes(char *, char [], char [], Shapes *);
short _mwsave_shapes(char *, char [], char [], char [], Shapes);
short _mwload_dcurve(char *, char [], char [], Dcurve *);
short _mwsave_dcurve(char *, char [], char [], char [], Dcurve);
short _mwload_dcurves(char *, char [], char [], Dcurves *);
short _mwsave_dcurves(char *, char [], char [], char [], Dcurves);
short _mwload_rawdata(char *, char [], char [], Rawdata *);
short _mwsave_rawdata(char *, char [], char [], char [], Rawdata);
short _mwload_flist(char *, char [], char [], Flist *);
short _mwsave_flist(char *, char [], char [], char [], Flist);
short _mwload_flists(char *, char [], char [], Flists *);
short _mwsave_flists(char *, char [], char [], char [], Flists);
short _mwload_dlist(char *, char [], char [], Dlist *);
short _mwsave_dlist(char *, char [], char [], char [], Dlist);
short _mwload_dlists(char *, char [], char [], Dlists *);
short _mwsave_dlists(char *, char [], char [], char [], Dlists);
short _mwload_wpack2d(char *, char [], char [], Wpack2d *);
short _mwsave_wpack2d(char *, char [], char [], char [], Wpack2d);

#endif /* !_MWIO_H */

/*
 * mwio.h
 */

#ifndef _MWIO_H_
#define _MWIO_H_

/* src/mwio.c */
long mw_fsize(FILE *fp);
void _mw_flip_image(register unsigned char *ptr, short size, short dx, short dy, char flip);
FILE *_mw_write_header_file(char *fname, char *type, float IDvers);
int _search_filename(char *fname);
int _check_filename(const char *fname);
long _mw_find_pattern_in_file(FILE *fp, char *label);
int _mw_byte_ordering_is_little_endian(void);
short _mwload_cimage(char *name, char type[], char comment[], Cimage *im);
short _mwsave_cimage(char *name, char type[], char type_force[], char comment[], Cimage im);
short _mwload_fimage(char *name, char type[], char comment[], Fimage *im);
short _mwsave_fimage(char *name, char type[], char type_force[], char comment[], Fimage im);
short _mwload_cmovie(char *name, char type[], char comment[], Cmovie *movie);
short _mwsave_cmovie(char *name, char type[], char type_force[], char comment[], Cmovie movie);
short _mwload_fmovie(char *name, char type[], char comment[], Fmovie *movie);
short _mwsave_fmovie(char *name, char type[], char type_force[], char comment[], Fmovie movie);
short _mwload_ccmovie(char *name, char type[], char comment[], Ccmovie *movie);
short _mwsave_ccmovie(char *name, char type[], char type_force[], char comment[], Ccmovie movie);
short _mwload_cfmovie(char *name, char type[], char comment[], Cfmovie *movie);
short _mwsave_cfmovie(char *name, char type[], char type_force[], char comment[], Cfmovie movie);
short _mwload_curve(char *name, char type[], char comment[], Curve *cv);
short _mwsave_curve(char *name, char type[], char type_force[], char comment[], Curve cv);
short _mwload_curves(char *name, char type[], char comment[], Curves *cv);
short _mwsave_curves(char *name, char type[], char type_force[], char comment[], Curves cv);
short _mwload_polygon(char *name, char type[], char comment[], Polygon *poly);
short _mwsave_polygon(char *name, char type[], char type_force[], char comment[], Polygon poly);
short _mwload_polygons(char *name, char type[], char comment[], Polygons *poly);
short _mwsave_polygons(char *name, char type[], char type_force[], char comment[], Polygons poly);
short _mwload_fcurve(char *name, char type[], char comment[], Fcurve *cv);
short _mwsave_fcurve(char *name, char type[], char type_force[], char comment[], Fcurve cv);
short _mwload_fcurves(char *name, char type[], char comment[], Fcurves *cv);
short _mwsave_fcurves(char *name, char type[], char type_force[], char comment[], Fcurves cv);
short _mwload_fpolygon(char *name, char type[], char comment[], Fpolygon *poly);
short _mwsave_fpolygon(char *name, char type[], char type_force[], char comment[], Fpolygon poly);
short _mwload_fpolygons(char *name, char type[], char comment[], Fpolygons *poly);
short _mwsave_fpolygons(char *name, char type[], char type_force[], char comment[], Fpolygons poly);
short _mwload_fsignal(char *name, char type[], char comment[], Fsignal *signal);
short _mwsave_fsignal(char *name, char type[], char type_force[], char comment[], Fsignal signal);
short _mwload_wtrans1d(char *name, char type[], char comment[], Wtrans1d *wtrans);
short _mwsave_wtrans1d(char *name, char type[], char type_force[], char comment[], Wtrans1d wtrans);
short _mwload_wtrans2d(char *name, char type[], char comment[], Wtrans2d *wtrans);
short _mwsave_wtrans2d(char *name, char type[], char type_force[], char comment[], Wtrans2d wtrans);
short _mwload_vchain_wmax(char *name, char type[], char comment[], Vchain_wmax *vchain);
short _mwsave_vchain_wmax(char *name, char type[], char type_force[], char comment[], Vchain_wmax vchain);
short _mwload_vchains_wmax(char *name, char type[], char comment[], Vchains_wmax *vchains);
short _mwsave_vchains_wmax(char *name, char type[], char type_force[], char comment[], Vchains_wmax vchains);
short _mwload_ccimage(char *name, char type[], char comment[], Ccimage *im);
short _mwsave_ccimage(char *name, char type[], char type_force[], char comment[], Ccimage im);
short _mwload_cfimage(char *name, char type[], char comment[], Cfimage *im);
short _mwsave_cfimage(char *name, char type[], char type_force[], char comment[], Cfimage im);
short _mwload_morpho_line(char *name, char type[], char comment[], Morpho_line *ll);
short _mwsave_morpho_line(char *name, char type[], char type_force[], char comment[], Morpho_line ll);
short _mwload_fmorpho_line(char *name, char type[], char comment[], Fmorpho_line *fll);
short _mwsave_fmorpho_line(char *name, char type[], char type_force[], char comment[], Fmorpho_line fll);
short _mwload_morpho_set(char *name, char type[], char comment[], Morpho_set *morpho_set);
short _mwsave_morpho_set(char *name, char type[], char type_force[], char comment[], Morpho_set morpho_set);
short _mwload_morpho_sets(char *name, char type[], char comment[], Morpho_sets *morpho_sets);
short _mwsave_morpho_sets(char *name, char type[], char type_force[], char comment[], Morpho_sets morpho_sets);
short _mwload_mimage(char *name, char type[], char comment[], Mimage *mimage);
short _mwsave_mimage(char *name, char type[], char type_force[], char comment[], Mimage mimage);
short _mwload_cmorpho_line(char *name, char type[], char comment[], Cmorpho_line *ll);
short _mwsave_cmorpho_line(char *name, char type[], char type_force[], char comment[], Cmorpho_line ll);
short _mwload_cfmorpho_line(char *name, char type[], char comment[], Cfmorpho_line *fll);
short _mwsave_cfmorpho_line(char *name, char type[], char type_force[], char comment[], Cfmorpho_line fll);
short _mwload_cmorpho_set(char *name, char type[], char comment[], Cmorpho_set *cmorpho_set);
short _mwsave_cmorpho_set(char *name, char type[], char type_force[], char comment[], Cmorpho_set cmorpho_set);
short _mwload_cmorpho_sets(char *name, char type[], char comment[], Cmorpho_sets *cmorpho_sets);
short _mwsave_cmorpho_sets(char *name, char type[], char type_force[], char comment[], Cmorpho_sets cmorpho_sets);
short _mwload_cmimage(char *name, char type[], char comment[], Cmimage *cmimage);
short _mwsave_cmimage(char *name, char type[], char type_force[], char comment[], Cmimage cmimage);
short _mwload_shape(char *name, char type[], char comment[], Shape *shape);
short _mwsave_shape(char *name, char type[], char type_force[], char comment[], Shape shape);
short _mwload_shapes(char *name, char type[], char comment[], Shapes *shapes);
short _mwsave_shapes(char *name, char type[], char type_force[], char comment[], Shapes shapes);
short _mwload_dcurve(char *name, char type[], char comment[], Dcurve *cv);
short _mwsave_dcurve(char *name, char type[], char type_force[], char comment[], Dcurve cv);
short _mwload_dcurves(char *name, char type[], char comment[], Dcurves *cv);
short _mwsave_dcurves(char *name, char type[], char type_force[], char comment[], Dcurves cv);
short _mwload_rawdata(char *name, char type[], char comment[], Rawdata *rd);
short _mwsave_rawdata(char *name, char type[], char type_force[], char comment[], Rawdata rd);
short _mwload_flist(char *name, char type[], char comment[], Flist *lst);
short _mwsave_flist(char *name, char type[], char type_force[], char comment[], Flist lst);
short _mwload_flists(char *name, char type[], char comment[], Flists *lsts);
short _mwsave_flists(char *name, char type[], char type_force[], char comment[], Flists lsts);
short _mwload_dlist(char *name, char type[], char comment[], Dlist *lst);
short _mwsave_dlist(char *name, char type[], char type_force[], char comment[], Dlist lst);
short _mwload_dlists(char *name, char type[], char comment[], Dlists *lsts);
short _mwsave_dlists(char *name, char type[], char type_force[], char comment[], Dlists lsts);
short _mwload_wpack2d(char *name, char type[], char comment[], Wpack2d *pack);
short _mwsave_wpack2d(char *name, char type[], char type_force[], char comment[], Wpack2d pack);

#endif /* !_MWIO_H_ */

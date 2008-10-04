/*
 * mw-modules api header
 */

#ifndef _MW_MODULES_
#define _MW_MODULES_


#include <mw.h>

/*
 * cfezw.lib.h
 */

#ifndef _CFEZW_LIB_H_
#define _CFEZW_LIB_H_

/* src/compression/ezwave/cfezw.lib.c */
void cfezw(int *NumRec, Fimage Edge_Ri, Fsignal Ri2, int *FilterNorm, float *WeightFac, int *DistRate, float *Rate, float *GRate, float *BRate, float *PSNR, int *Conv, Polygons SelectedArea, Cimage Output, Cfimage Image, Fsignal Ri, Cfimage QImage);

#endif /* !_CFEZW_LIB_H_ */
/*
 * cfiezw.lib.h
 */

#ifndef _CFIEZW_LIB_H_
#define _CFIEZW_LIB_H_

/* src/compression/ezwave/cfiezw.lib.c */
void cfiezw(Fimage Edge_Ri, Fsignal Ri2, float *WeightFac, int *Conv, Cimage Compress, Fsignal Ri, Cfimage Output);

#endif /* !_CFIEZW_LIB_H_ */
/*
 * ezw.lib.h
 */

#ifndef _EZW_LIB_H_
#define _EZW_LIB_H_

/* src/compression/ezwave/ezw.lib.c */
void ezw(int *PrintFull, int *NumRec, float *WeightFac, float *Thres, int *Max_Count_AC, int *DistRate, float *Rate, float *PSNR, Polygons SelectedArea, Cimage Compress, Wtrans2d Wtrans, Wtrans2d Output, char *PtrDRC);

#endif /* !_EZW_LIB_H_ */
/*
 * fezw.lib.h
 */

#ifndef _FEZW_LIB_H_
#define _FEZW_LIB_H_

/* src/compression/ezwave/fezw.lib.c */
void fezw(int *NumRec, Fimage Edge_Ri, Fsignal Ri2, int *FilterNorm, float *WeightFac, int *DistRate, float *Rate, float *PSNR, Polygons SelectedArea, Cimage Output, Fimage Image, Fsignal Ri, Fimage QImage, char *PtrDRC);

#endif /* !_FEZW_LIB_H_ */
/*
 * fiezw.lib.h
 */

#ifndef _FIEZW_LIB_H_
#define _FIEZW_LIB_H_

/* src/compression/ezwave/fiezw.lib.c */
void fiezw(Fimage Edge_Ri, Fsignal Ri2, float *WeightFac, Cimage Compress, Fsignal Ri, Fimage Output);

#endif /* !_FIEZW_LIB_H_ */
/*
 * iezw.lib.h
 */

#ifndef _IEZW_LIB_H_
#define _IEZW_LIB_H_

/* src/compression/ezwave/iezw.lib.c */
void iezw(int *PrintFull, float *WeightFac, Cimage Compress, Wtrans2d Output);

#endif /* !_IEZW_LIB_H_ */
/*
 * ardecode2.lib.h
 */

#ifndef _ARDECODE2_LIB_H_
#define _ARDECODE2_LIB_H_

/* src/compression/lossless/ardecode2.lib.c */
void ardecode2(int *Print, int *NRow, int *NSymb, long *Cap_Histo, int *Predic, Fsignal Histo, Cimage Input, double *Rate, Fimage Output);

#endif /* !_ARDECODE2_LIB_H_ */
/*
 * arencode2.lib.h
 */

#ifndef _ARENCODE2_LIB_H_
#define _ARENCODE2_LIB_H_

/* src/compression/lossless/arencode2.lib.c */
void arencode2(int *Print, long *Size, int *NSymb, long *Cap_Histo, int *Predic, Fsignal Histo, int *Header, Fimage Input, double *Rate, Cimage Output);

#endif /* !_ARENCODE2_LIB_H_ */
/*
 * cvsencode.lib.h
 */

#ifndef _CVSENCODE_LIB_H_
#define _CVSENCODE_LIB_H_

/* src/compression/lossless/cvsencode.lib.c */
double cvsencode(int *L, Curves O, Curves C, unsigned int *N, double *B);

#endif /* !_CVSENCODE_LIB_H_ */
/*
 * cvsfrecode.lib.h
 */

#ifndef _CVSFRECODE_LIB_H_
#define _CVSFRECODE_LIB_H_

/* src/compression/lossless/cvsfrecode.lib.c */
double cvsfrecode(Curves C, unsigned int *N, double *B);

#endif /* !_CVSFRECODE_LIB_H_ */
/*
 * cvsorgcode.lib.h
 */

#ifndef _CVSORGCODE_LIB_H_
#define _CVSORGCODE_LIB_H_

/* src/compression/lossless/cvsorgcode.lib.c */
double cvsorgcode(Curves C, unsigned int *N, double *B);

#endif /* !_CVSORGCODE_LIB_H_ */
/*
 * fencode.lib.h
 */

#ifndef _FENCODE_LIB_H_
#define _FENCODE_LIB_H_

/* src/compression/lossless/fencode.lib.c */
double fencode(Fimage U);

#endif /* !_FENCODE_LIB_H_ */
/*
 * fiscalq.lib.h
 */

#ifndef _FISCALQ_LIB_H_
#define _FISCALQ_LIB_H_

/* src/compression/scalar/fiscalq.lib.c */
void fiscalq(int *Print, int *NRow, int *NCol, Cimage Compress, Fimage Result, double *Rate);

#endif /* !_FISCALQ_LIB_H_ */
/*
 * fscalq.lib.h
 */

#ifndef _FSCALQ_LIB_H_
#define _FSCALQ_LIB_H_

/* src/compression/scalar/fscalq.lib.c */
void fscalq(int *PrintSNR, int *SmallHeader, int *NStep, float *SStep, int *Center, Cimage Compress, Fimage Image, Fimage Result, double *MSE, double *SNR, double *Ent, double *RateAr);

#endif /* !_FSCALQ_LIB_H_ */
/*
 * fivq.lib.h
 */

#ifndef _FIVQ_LIB_H_
#define _FIVQ_LIB_H_

/* src/compression/vector/fivq.lib.c */
void fivq(int *Print, int *NRow, int *NCol, Fimage CodeBook2, Fimage CodeBook3, Fimage CodeBook4, Fimage ResCodeBook1, Fimage ResCodeBook2, Fimage ResCodeBook3, Fimage ResCodeBook4, Fimage ResResCodeBook1, Fimage ResResCodeBook2, Cimage Compress, Fimage CodeBook1, Fimage Result, double *Rate);

#endif /* !_FIVQ_LIB_H_ */
/*
 * flbg.lib.h
 */

#ifndef _FLBG_LIB_H_
#define _FLBG_LIB_H_

/* src/compression/vector/flbg.lib.c */
void flbg(int *Size, int *Width, int *Height, int *Lap, int *Decim, int *Edge, Fsignal Weight, int *MultiCB, int *PrintSNR, Fimage Image2, Fimage Image3, Fimage Image4, Fimage Image5, Fimage Image6, Fimage Image7, Fimage Image8, int *InitRandCB, int *RepeatRand, int *NResCB, int *NResResCB, Fimage ResCodeBook, Fimage ResResCodeBook, Fimage Image1, Fimage CodeBook, float *MSE);

#endif /* !_FLBG_LIB_H_ */
/*
 * flbg_adap.lib.h
 */

#ifndef _FLBG_ADAP_LIB_H_
#define _FLBG_ADAP_LIB_H_

/* src/compression/vector/flbg_adap.lib.c */
void flbg_adap(int *Size, int *Width, int *Height, int *Lap, int *Decim, int *Edge, float *ThresVal1, float *ThresVal2, float *ThresVal3, Fsignal Weight, int *MultiCB, int *PrintSNR, int *Size2, int *Size3, int *Size4, Fimage Image2, Fimage Image3, Fimage Image4, Fimage Image5, Fimage Image6, Fimage Image7, Fimage Image8, Fimage Output2, Fimage Output3, Fimage Output4, Fimage Image1, Fimage Output);

#endif /* !_FLBG_ADAP_LIB_H_ */
/*
 * flbg_train.lib.h
 */

#ifndef _FLBG_TRAIN_LIB_H_
#define _FLBG_TRAIN_LIB_H_

/* src/compression/vector/flbg_train.lib.c */
void flbg_train(int *Size, Fsignal Weight, int *MultiCB, Fimage InitCodeBook, int *NResCB, int *NResResCB, Fimage ResCodeBook, Fimage ResResCodeBook, int *PrintSNR, Fimage TrainSet, float *MSE, Fimage CodeBook);

#endif /* !_FLBG_TRAIN_LIB_H_ */
/*
 * fvq.lib.h
 */

#ifndef _FVQ_LIB_H_
#define _FVQ_LIB_H_

/* src/compression/vector/fvq.lib.c */
void fvq(int *PrintSNR, int *SmallHeader, int *BitMapCode, int *RateDist, int *NCB1, int *NCB2, int *NCB3, int *NCB4, Fimage CodeBook2, Fimage CodeBook3, Fimage CodeBook4, int *NResCB1, int *NResCB2, int *NResCB3, int *NResCB4, Fimage ResCodeBook1, Fimage ResCodeBook2, Fimage ResCodeBook3, Fimage ResCodeBook4, int *NResResCB1, int *NResResCB2, Fimage ResResCodeBook1, Fimage ResResCodeBook2, Cimage Compress, Fimage Image, Fimage CodeBook1, Fimage Result, double *MSE, double *SNR, double *Entropy, double *RateAr);

#endif /* !_FVQ_LIB_H_ */
/*
 * mk_codebook.lib.h
 */

#ifndef _MK_CODEBOOK_LIB_H_
#define _MK_CODEBOOK_LIB_H_

/* src/compression/vector/mk_codebook.lib.c */
void mk_codebook(int *Normal, double *Mean, double *Variance, int *Size, int *BlockSize, Fimage CodeBook);

#endif /* !_MK_CODEBOOK_LIB_H_ */
/*
 * mk_trainset.lib.h
 */

#ifndef _MK_TRAINSET_LIB_H_
#define _MK_TRAINSET_LIB_H_

/* src/compression/vector/mk_trainset.lib.c */
void mk_trainset(int *Width, int *Height, int *Lap, int *Decim, int *Edge, float *ThresVal1, float *ThresVal2, float *ThresVal3, int *SizeCB, Fimage Image2, Fimage Image3, Fimage Image4, Fimage Image5, Fimage Image6, Fimage Image7, Fimage Image8, Fimage Result2, Fimage Result3, Fimage Result4, Fimage Image, Fimage Result);

#endif /* !_MK_TRAINSET_LIB_H_ */
/*
 * fwivq.lib.h
 */

#ifndef _FWIVQ_LIB_H_
#define _FWIVQ_LIB_H_

/* src/compression/vqwave/fwivq.lib.c */
void fwivq(Fimage Edge_Ri, Fsignal Ri2, int *FilterNorm, float *WeightFac, Fimage CodeBook2, Fimage CodeBook3, Fimage ResCodeBook1, Fimage ResCodeBook2, Fimage ResCodeBook3, Fimage ResResCodeBook1, Fimage ResResCodeBook2, Cimage Compress, Fimage CodeBook1, Fsignal Ri, Fimage Output);

#endif /* !_FWIVQ_LIB_H_ */
/*
 * fwlbg_adap.lib.h
 */

#ifndef _FWLBG_ADAP_LIB_H_
#define _FWLBG_ADAP_LIB_H_

/* src/compression/vqwave/fwlbg_adap.lib.c */
void fwlbg_adap(int *NumRecMax, int *Level, int *Orient, Fimage Edge_Ri, Fsignal Ri2, int *FilterNorm, int *StopDecim, int *Width, int *Height, int *MultiCB, int *Lap, int *Sizec1, int *Sizec2, int *Sizec3, float *ThresVal1, float *ThresVal2, float *ThresVal3, Fimage OldCodeBook, Fimage OldAdapCodeBook2, Fimage OldAdapCodeBook3, Fimage Output2, Fimage Output3, Fimage Image2, Fimage Image3, Fimage Image4, Fimage ResCodeBook, Fimage ResResCodeBook, Fimage Image1, Fsignal Ri, Fimage Output1);

#endif /* !_FWLBG_ADAP_LIB_H_ */
/*
 * fwvq.lib.h
 */

#ifndef _FWVQ_LIB_H_
#define _FWVQ_LIB_H_

/* src/compression/vqwave/fwvq.lib.c */
void fwvq(int *NumRec, Fimage Edge_Ri, Fsignal Ri2, int *FilterNorm, float *WeightFac, int *NumRecScal, int *NStep, int *MultiCB, Fimage CodeBook2, Fimage CodeBook3, Fimage ResCodeBook1, Fimage ResCodeBook2, Fimage ResCodeBook3, Fimage ResResCodeBook1, Fimage ResResCodeBook2, int *DistRate, float *TargRate, Cimage Output, Fimage Image, Fimage CodeBook1, Fsignal Ri, Fimage QImage);

#endif /* !_FWVQ_LIB_H_ */
/*
 * wlbg_adap.lib.h
 */

#ifndef _WLBG_ADAP_LIB_H_
#define _WLBG_ADAP_LIB_H_

/* src/compression/vqwave/wlbg_adap.lib.c */
void wlbg_adap(int *NumRecMax, int *Level, int *Orient, int *Dyadic, int *Lap, int *Edge, int *Width, int *Height, int *MultiCB, int *Sizec1, int *Sizec2, int *Sizec3, float *ThresVal1, float *ThresVal2, float *ThresVal3, Wtrans2d OldCodeBook, Wtrans2d OldAdapCodeBook2, Wtrans2d OldAdapCodeBook3, Wtrans2d *Output2, Wtrans2d *Output3, Wtrans2d TrainWtrans2, Wtrans2d TrainWtrans3, Wtrans2d TrainWtrans4, Wtrans2d ResCodeBook, Wtrans2d ResResCodeBook, Wtrans2d TrainWtrans1, Wtrans2d *Output1);

#endif /* !_WLBG_ADAP_LIB_H_ */
/*
 * area.lib.h
 */

#ifndef _AREA_LIB_H_
#define _AREA_LIB_H_

/* src/curve/area.lib.c */
double area(Dlist in);

#endif /* !_AREA_LIB_H_ */
/*
 * circle.lib.h
 */

#ifndef _CIRCLE_LIB_H_
#define _CIRCLE_LIB_H_

/* src/curve/circle.lib.c */
Dlist circle(Dlist out, double *r, int *n);

#endif /* !_CIRCLE_LIB_H_ */
/*
 * disc.lib.h
 */

#ifndef _DISC_LIB_H_
#define _DISC_LIB_H_

/* src/curve/disc.lib.c */
Curve disc(double r, float *i);

#endif /* !_DISC_LIB_H_ */
/*
 * dsplit_convex.lib.h
 */

#ifndef _DSPLIT_CONVEX_LIB_H_
#define _DSPLIT_CONVEX_LIB_H_

/* src/curve/dsplit_convex.lib.c */
Dlists dsplit_convex(Dlist in, Dlists out, int *ncc, double *eps);

#endif /* !_DSPLIT_CONVEX_LIB_H_ */
/*
 * extract_connex.lib.h
 */

#ifndef _EXTRACT_CONNEX_LIB_H_
#define _EXTRACT_CONNEX_LIB_H_

/* src/curve/extract_connex.lib.c */
Fcurves extract_connex(Cimage in, Fcurves curves, int *g);

#endif /* !_EXTRACT_CONNEX_LIB_H_ */
/*
 * fillpoly.lib.h
 */

#ifndef _FILLPOLY_LIB_H_
#define _FILLPOLY_LIB_H_

/* src/curve/fillpoly.lib.c */
void fillpoly(int *dx, int *dy, Polygon poly, Cimage bitmap);

#endif /* !_FILLPOLY_LIB_H_ */
/*
 * fillpolys.lib.h
 */

#ifndef _FILLPOLYS_LIB_H_
#define _FILLPOLYS_LIB_H_

/* src/curve/fillpolys.lib.c */
void fillpolys(int *dx, int *dy, Polygons polys, Cimage bitmap);

#endif /* !_FILLPOLYS_LIB_H_ */
/*
 * fkbox.lib.h
 */

#ifndef _FKBOX_LIB_H_
#define _FKBOX_LIB_H_

/* src/curve/fkbox.lib.c */
void fkbox(Fcurves cs, float *xmin, float *ymin, float *xmax, float *ymax, float *z, Fcurve box);

#endif /* !_FKBOX_LIB_H_ */
/*
 * fkcenter.lib.h
 */

#ifndef _FKCENTER_LIB_H_
#define _FKCENTER_LIB_H_

/* src/curve/fkcenter.lib.c */
void fkcenter(Fcurves cs, float *xg, float *yg);

#endif /* !_FKCENTER_LIB_H_ */
/*
 * fkcrop.lib.h
 */

#ifndef _FKCROP_LIB_H_
#define _FKCROP_LIB_H_

/* src/curve/fkcrop.lib.c */
Fcurves fkcrop(double X1, double Y1, double X2, double Y2, Fcurves cs, Fcurve box);

#endif /* !_FKCROP_LIB_H_ */
/*
 * fkzrt.lib.h
 */

#ifndef _FKZRT_LIB_H_
#define _FKZRT_LIB_H_

/* src/curve/fkzrt.lib.c */
Fcurves fkzrt(Fcurves cs, double zoom, double angle, double x, double y, char *sym);

#endif /* !_FKZRT_LIB_H_ */
/*
 * flconcat.lib.h
 */

#ifndef _FLCONCAT_LIB_H_
#define _FLCONCAT_LIB_H_

/* src/curve/flconcat.lib.c */
Flists flconcat(Flists l1, Flists l2, Flists out);

#endif /* !_FLCONCAT_LIB_H_ */
/*
 * flscale.lib.h
 */

#ifndef _FLSCALE_LIB_H_
#define _FLSCALE_LIB_H_

/* src/curve/flscale.lib.c */
Flists flscale(Flists in, Fsignal s);

#endif /* !_FLSCALE_LIB_H_ */
/*
 * fsplit_convex.lib.h
 */

#ifndef _FSPLIT_CONVEX_LIB_H_
#define _FSPLIT_CONVEX_LIB_H_

/* src/curve/fsplit_convex.lib.c */
Flists fsplit_convex(Flist in, Flists out, int *ncc, double *eps);

#endif /* !_FSPLIT_CONVEX_LIB_H_ */
/*
 * dkinfo.lib.h
 */

#ifndef _DKINFO_LIB_H_
#define _DKINFO_LIB_H_

/* src/curve/io/dkinfo.lib.c */
void dkinfo(Dlists in);

#endif /* !_DKINFO_LIB_H_ */
/*
 * fkplot.lib.h
 */

#ifndef _FKPLOT_LIB_H_
#define _FKPLOT_LIB_H_

/* src/curve/io/fkplot.lib.c */
Cimage fkplot(Fcurves cs, Cimage out, char *s_flag);

#endif /* !_FKPLOT_LIB_H_ */
/*
 * fkprintasc.lib.h
 */

#ifndef _FKPRINTASC_LIB_H_
#define _FKPRINTASC_LIB_H_

/* src/curve/io/fkprintasc.lib.c */
void fkprintasc(Fcurves fcvs);

#endif /* !_FKPRINTASC_LIB_H_ */
/*
 * fkprintfig.lib.h
 */

#ifndef _FKPRINTFIG_LIB_H_
#define _FKPRINTFIG_LIB_H_

/* src/curve/io/fkprintfig.lib.c */
void fkprintfig(Fcurves in, int *d, char *e_flag, char *s_flag, float *m, float *r);

#endif /* !_FKPRINTFIG_LIB_H_ */
/*
 * fkreadasc.lib.h
 */

#ifndef _FKREADASC_LIB_H_
#define _FKREADASC_LIB_H_

/* src/curve/io/fkreadasc.lib.c */
Fcurves fkreadasc(void);

#endif /* !_FKREADASC_LIB_H_ */
/*
 * fkview.lib.h
 */

#ifndef _FKVIEW_LIB_H_
#define _FKVIEW_LIB_H_

/* src/curve/io/fkview.lib.c */
void fkview(Flists in, Ccimage *out, int *sx, int *sy, Flists ref, Fimage bg, int *i, char *a, char *s, char *e, int *d, int *g, int *c, int *C, char *n, char *window, int *x_0, int *y_0, Flist *curve);

#endif /* !_FKVIEW_LIB_H_ */
/*
 * flprintasc.lib.h
 */

#ifndef _FLPRINTASC_LIB_H_
#define _FLPRINTASC_LIB_H_

/* src/curve/io/flprintasc.lib.c */
void flprintasc(Flists in, char *v, int *n);

#endif /* !_FLPRINTASC_LIB_H_ */
/*
 * flreadasc.lib.h
 */

#ifndef _FLREADASC_LIB_H_
#define _FLREADASC_LIB_H_

/* src/curve/io/flreadasc.lib.c */
Flists flreadasc(int dim);

#endif /* !_FLREADASC_LIB_H_ */
/*
 * kplot.lib.h
 */

#ifndef _KPLOT_LIB_H_
#define _KPLOT_LIB_H_

/* src/curve/io/kplot.lib.c */
void kplot(Cimage A, char *line, Curves curves, Cimage B, int *d);

#endif /* !_KPLOT_LIB_H_ */
/*
 * kplotasc.lib.h
 */

#ifndef _KPLOTASC_LIB_H_
#define _KPLOTASC_LIB_H_

/* src/curve/io/kplotasc.lib.c */
void kplotasc(Curve cv);

#endif /* !_KPLOTASC_LIB_H_ */
/*
 * kreadfig.lib.h
 */

#ifndef _KREADFIG_LIB_H_
#define _KREADFIG_LIB_H_

/* src/curve/io/kreadfig.lib.c */
Curves kreadfig(void);

#endif /* !_KREADFIG_LIB_H_ */
/*
 * readpoly.lib.h
 */

#ifndef _READPOLY_LIB_H_
#define _READPOLY_LIB_H_

/* src/curve/io/readpoly.lib.c */
Polygons readpoly(Cimage image, char *window);

#endif /* !_READPOLY_LIB_H_ */
/*
 * kspline.lib.h
 */

#ifndef _KSPLINE_LIB_H_
#define _KSPLINE_LIB_H_

/* src/curve/kspline.lib.c */
void kspline(int *C, float *Step, Curve P, Curve spline);

#endif /* !_KSPLINE_LIB_H_ */
/*
 * ksplines.lib.h
 */

#ifndef _KSPLINES_LIB_H_
#define _KSPLINES_LIB_H_

/* src/curve/ksplines.lib.c */
void ksplines(int *C, float *Step, Curves ctrl_pts, Curves splines);

#endif /* !_KSPLINES_LIB_H_ */
/*
 * km_bitangents.lib.h
 */

#ifndef _KM_BITANGENTS_LIB_H_
#define _KM_BITANGENTS_LIB_H_

/* src/curve/matching/km_bitangents.lib.c */
Flist km_bitangents(Flist curve, Flist curve_IP, Flist curve_BP);

#endif /* !_KM_BITANGENTS_LIB_H_ */
/*
 * km_codecurve_ai.lib.h
 */

#ifndef _KM_CODECURVE_AI_LIB_H_
#define _KM_CODECURVE_AI_LIB_H_

/* src/curve/matching/km_codecurve_ai.lib.c */
Flists km_codecurve_ai(Flist curve, Flist curve_IP, Flist curve_FP, Flist curve_BP, Flists dict, int NC, int NN, double FN);

#endif /* !_KM_CODECURVE_AI_LIB_H_ */
/*
 * km_codecurve_si.lib.h
 */

#ifndef _KM_CODECURVE_SI_LIB_H_
#define _KM_CODECURVE_SI_LIB_H_

/* src/curve/matching/km_codecurve_si.lib.c */
Flists km_codecurve_si(Flist curve, Flist curve_IP, Flist curve_FP, Flist curve_BP, Flists dict, int NC, int NN, double FN);

#endif /* !_KM_CODECURVE_SI_LIB_H_ */
/*
 * km_createdict_ai.lib.h
 */

#ifndef _KM_CREATEDICT_AI_LIB_H_
#define _KM_CREATEDICT_AI_LIB_H_

/* src/curve/matching/km_createdict_ai.lib.c */
void km_createdict_ai(float *FNorm, int *NNorm, Flists list_curves, Flists dict);

#endif /* !_KM_CREATEDICT_AI_LIB_H_ */
/*
 * km_createdict_si.lib.h
 */

#ifndef _KM_CREATEDICT_SI_LIB_H_
#define _KM_CREATEDICT_SI_LIB_H_

/* src/curve/matching/km_createdict_si.lib.c */
void km_createdict_si(float *FNorm, int *NNorm, Flists list_curves, Flists dict);

#endif /* !_KM_CREATEDICT_SI_LIB_H_ */
/*
 * km_flatpoints.lib.h
 */

#ifndef _KM_FLATPOINTS_LIB_H_
#define _KM_FLATPOINTS_LIB_H_

/* src/curve/matching/km_flatpoints.lib.c */
Flist km_flatpoints(Flist curve, Flist curve_IP, Flist curve_FP, double angle, double dist);

#endif /* !_KM_FLATPOINTS_LIB_H_ */
/*
 * km_inflexionpoints.lib.h
 */

#ifndef _KM_INFLEXIONPOINTS_LIB_H_
#define _KM_INFLEXIONPOINTS_LIB_H_

/* src/curve/matching/km_inflexionpoints.lib.c */
Flist km_inflexionpoints(Flist curve, Flist curve_IP);

#endif /* !_KM_INFLEXIONPOINTS_LIB_H_ */
/*
 * km_match_ai.lib.h
 */

#ifndef _KM_MATCH_AI_LIB_H_
#define _KM_MATCH_AI_LIB_H_

/* src/curve/matching/km_match_ai.lib.c */
void km_match_ai(double maxError1, double maxError2, double minLength, double minComplex, Flists levlines1, Flists levlines2, Flists dict1, Flists dict2, Flist matchings, Flist matching_pieces);

#endif /* !_KM_MATCH_AI_LIB_H_ */
/*
 * km_match_si.lib.h
 */

#ifndef _KM_MATCH_SI_LIB_H_
#define _KM_MATCH_SI_LIB_H_

/* src/curve/matching/km_match_si.lib.c */
void km_match_si(double maxError1, double maxError2, double minLength, double minComplex, Flists levlines1, Flists levlines2, Flists dict1, Flists dict2, Flist matchings, Flist matching_pieces);

#endif /* !_KM_MATCH_SI_LIB_H_ */
/*
 * km_prematchings.lib.h
 */

#ifndef _KM_PREMATCHINGS_LIB_H_
#define _KM_PREMATCHINGS_LIB_H_

/* src/curve/matching/km_prematchings.lib.c */
Flists km_prematchings(double maxError, Flists dict1, Flists dict2, Flists matchings);

#endif /* !_KM_PREMATCHINGS_LIB_H_ */
/*
 * km_savematchings.lib.h
 */

#ifndef _KM_SAVEMATCHINGS_LIB_H_
#define _KM_SAVEMATCHINGS_LIB_H_

/* src/curve/matching/km_savematchings.lib.c */
void km_savematchings(Flist matching_pieces, Flists levlines1, Flists levlines2, Flists aux1, Flists aux2, Flists pieceaux1, Flists pieceaux2);

#endif /* !_KM_SAVEMATCHINGS_LIB_H_ */
/*
 * perimeter.lib.h
 */

#ifndef _PERIMETER_LIB_H_
#define _PERIMETER_LIB_H_

/* src/curve/perimeter.lib.c */
double perimeter(Dlist in, double *min, double *max);

#endif /* !_PERIMETER_LIB_H_ */
/*
 * fksmooth.lib.h
 */

#ifndef _FKSMOOTH_LIB_H_
#define _FKSMOOTH_LIB_H_

/* src/curve/smooth/fksmooth.lib.c */
Flist fksmooth(Flist in, int *n, float *std, float *t, char *P);

#endif /* !_FKSMOOTH_LIB_H_ */
/*
 * gass.lib.h
 */

#ifndef _GASS_LIB_H_
#define _GASS_LIB_H_

/* src/curve/smooth/gass.lib.c */
Dlists gass(Dlists in, Dlists out, double *first, double *last, double *eps, double *step, int *n, double *r, char *v);

#endif /* !_GASS_LIB_H_ */
/*
 * gcsf.lib.h
 */

#ifndef _GCSF_LIB_H_
#define _GCSF_LIB_H_

/* src/curve/smooth/gcsf.lib.c */
Dlists gcsf(Dlists in, Dlists out, double *gam, double *first, double *last, double *eps, double *area, int *n, double *r, char *v, int *iter, char *conv);

#endif /* !_GCSF_LIB_H_ */
/*
 * iter_fksmooth.lib.h
 */

#ifndef _ITER_FKSMOOTH_LIB_H_
#define _ITER_FKSMOOTH_LIB_H_

/* src/curve/smooth/iter_fksmooth.lib.c */
Flists iter_fksmooth(Flist in, Flists out, int *niter, int *n, float *std, float *t, char *P);

#endif /* !_ITER_FKSMOOTH_LIB_H_ */
/*
 * iter_gass.lib.h
 */

#ifndef _ITER_GASS_LIB_H_
#define _ITER_GASS_LIB_H_

/* src/curve/smooth/iter_gass.lib.c */
Dlists iter_gass(Dlist in, Dlists out, double *scale, int *niter, double *e, double *s, int *n);

#endif /* !_ITER_GASS_LIB_H_ */
/*
 * iter_gcsf.lib.h
 */

#ifndef _ITER_GCSF_LIB_H_
#define _ITER_GCSF_LIB_H_

/* src/curve/smooth/iter_gcsf.lib.c */
Dlists iter_gcsf(Dlist in, Dlists out, double *gam, double *last, double *area, double *eps, int *n, int *N, char *v);

#endif /* !_ITER_GCSF_LIB_H_ */
/*
 * canny.lib.h
 */

#ifndef _CANNY_LIB_H_
#define _CANNY_LIB_H_

/* src/image/detection/canny.lib.c */
Fimage canny(float *alpha, Fimage IN, Fimage OUT);

#endif /* !_CANNY_LIB_H_ */
/*
 * falign.lib.h
 */

#ifndef _FALIGN_LIB_H_
#define _FALIGN_LIB_H_

/* src/image/detection/falign.lib.c */
Flist falign(Fimage u, int *d, int *nl, double *eps, float *g, char *all, Flists crv);

#endif /* !_FALIGN_LIB_H_ */
/*
 * falign_mdl.lib.h
 */

#ifndef _FALIGN_MDL_LIB_H_
#define _FALIGN_MDL_LIB_H_

/* src/image/detection/falign_mdl.lib.c */
Fimage falign_mdl(Fimage u, int *d, int *nd, int *no_mdl, int *nl, double *eps, float *g, Flists crv);

#endif /* !_FALIGN_MDL_LIB_H_ */
/*
 * harris.lib.h
 */

#ifndef _HARRIS_LIB_H_
#define _HARRIS_LIB_H_

/* src/image/detection/harris.lib.c */
Flist harris(Fimage in, float *k, float *g, int *size, double *t);

#endif /* !_HARRIS_LIB_H_ */
/*
 * ll_boundaries.lib.h
 */

#ifndef _LL_BOUNDARIES_LIB_H_
#define _LL_BOUNDARIES_LIB_H_

/* src/image/detection/ll_boundaries.lib.c */
Flists ll_boundaries(Fimage in, Shapes tree, float *eps, char *all, float *step, int *precision, char *z, char *weak);

#endif /* !_LL_BOUNDARIES_LIB_H_ */
/*
 * ll_boundaries2.lib.h
 */

#ifndef _LL_BOUNDARIES2_LIB_H_
#define _LL_BOUNDARIES2_LIB_H_

/* src/image/detection/ll_boundaries2.lib.c */
Flists ll_boundaries2(Fimage in, float *eps, Shapes tree, float *step, int *prec, float *std, float *hstep, char *all, int *visit, char *loc, Fimage image_out, Shapes keep_tree);

#endif /* !_LL_BOUNDARIES2_LIB_H_ */
/*
 * ll_edges.lib.h
 */

#ifndef _LL_EDGES_LIB_H_
#define _LL_EDGES_LIB_H_

/* src/image/detection/ll_edges.lib.c */
Flists ll_edges(Fimage in, Shapes tree, float *eps, float *step, int *precision, char *z);

#endif /* !_LL_EDGES_LIB_H_ */
/*
 * vpoint.lib.h
 */

#ifndef _VPOINT_LIB_H_
#define _VPOINT_LIB_H_

/* src/image/detection/vpoint.lib.c */
int vpoint(Fimage imagein, Fimage allsegs, Flist output, Flists segs, double *eps, char *all, char *masked, char *verbose, int *maskedVPs);

#endif /* !_VPOINT_LIB_H_ */
/*
 * vpsegplot.lib.h
 */

#ifndef _VPSEGPLOT_LIB_H_
#define _VPSEGPLOT_LIB_H_

/* src/image/detection/vpsegplot.lib.c */
void vpsegplot(Fimage image, Fimage allsegs, Flist vpoints, Flists vsegs, int n, Flists crv, char *lines);

#endif /* !_VPSEGPLOT_LIB_H_ */
/*
 * cccrop.lib.h
 */

#ifndef _CCCROP_LIB_H_
#define _CCCROP_LIB_H_

/* src/image/domain/cccrop.lib.c */
Ccimage cccrop(Ccimage in, Ccimage out, float *sx, float *sy, float *z, unsigned char *bg, int *o, float *p, double X1, double Y1, double X2, double Y2);

#endif /* !_CCCROP_LIB_H_ */
/*
 * ccextract.lib.h
 */

#ifndef _CCEXTRACT_LIB_H_
#define _CCEXTRACT_LIB_H_

/* src/image/domain/ccextract.lib.c */
Ccimage ccextract(int *b, Ccimage in, Ccimage bg, Ccimage out, int X1, int Y1, int X2, int Y2, int *Xc, int *Yc, char *r);

#endif /* !_CCEXTRACT_LIB_H_ */
/*
 * cclocal_zoom.lib.h
 */

#ifndef _CCLOCAL_ZOOM_LIB_H_
#define _CCLOCAL_ZOOM_LIB_H_

/* src/image/domain/cclocal_zoom.lib.c */
Ccimage cclocal_zoom(Ccimage Input, int *X, int *Y, int *W, int *factor);

#endif /* !_CCLOCAL_ZOOM_LIB_H_ */
/*
 * ccmcollect.lib.h
 */

#ifndef _CCMCOLLECT_LIB_H_
#define _CCMCOLLECT_LIB_H_

/* src/image/domain/ccmcollect.lib.c */
Ccimage ccmcollect(Ccmovie in, Ccimage out, int *n, int *i, int *o, int *c, int *x, int *y);

#endif /* !_CCMCOLLECT_LIB_H_ */
/*
 * ccmzoom.lib.h
 */

#ifndef _CCMZOOM_LIB_H_
#define _CCMZOOM_LIB_H_

/* src/image/domain/ccmzoom.lib.c */
void ccmzoom(Ccmovie Input, Ccmovie Output, char *x_flg, char *y_flg, float *factor, int *o, char *i_flg);

#endif /* !_CCMZOOM_LIB_H_ */
/*
 * cczoom.lib.h
 */

#ifndef _CCZOOM_LIB_H_
#define _CCZOOM_LIB_H_

/* src/image/domain/cczoom.lib.c */
Ccimage cczoom(Ccimage in, Ccimage out, char *x_flg, char *y_flg, float *zoom, int *o, char *i_flg);

#endif /* !_CCZOOM_LIB_H_ */
/*
 * cextcenter.lib.h
 */

#ifndef _CEXTCENTER_LIB_H_
#define _CEXTCENTER_LIB_H_

/* src/image/domain/cextcenter.lib.c */
Cimage cextcenter(int *Fact, Cimage Image);

#endif /* !_CEXTCENTER_LIB_H_ */
/*
 * cextract.lib.h
 */

#ifndef _CEXTRACT_LIB_H_
#define _CEXTRACT_LIB_H_

/* src/image/domain/cextract.lib.c */
Cimage cextract(int *b, Cimage in, Cimage bg, Cimage out, int X1, int Y1, int X2, int Y2, int *Xc, int *Yc, char *r);

#endif /* !_CEXTRACT_LIB_H_ */
/*
 * cfextcenter.lib.h
 */

#ifndef _CFEXTCENTER_LIB_H_
#define _CFEXTCENTER_LIB_H_

/* src/image/domain/cfextcenter.lib.c */
Cfimage cfextcenter(int *Fact, Cfimage Image);

#endif /* !_CFEXTCENTER_LIB_H_ */
/*
 * cfunzoom.lib.h
 */

#ifndef _CFUNZOOM_LIB_H_
#define _CFUNZOOM_LIB_H_

/* src/image/domain/cfunzoom.lib.c */
Cfimage cfunzoom(Cfimage in, Cfimage out, float *z, int *o, float *tx, float *ty);

#endif /* !_CFUNZOOM_LIB_H_ */
/*
 * clocal_zoom.lib.h
 */

#ifndef _CLOCAL_ZOOM_LIB_H_
#define _CLOCAL_ZOOM_LIB_H_

/* src/image/domain/clocal_zoom.lib.c */
Cimage clocal_zoom(Cimage Input, int *X, int *Y, int *W, int *factor);

#endif /* !_CLOCAL_ZOOM_LIB_H_ */
/*
 * cmcollect.lib.h
 */

#ifndef _CMCOLLECT_LIB_H_
#define _CMCOLLECT_LIB_H_

/* src/image/domain/cmcollect.lib.c */
Cimage cmcollect(Cmovie in, Cimage out, int *n, int *i, int *o, int *c, int *x, int *y);

#endif /* !_CMCOLLECT_LIB_H_ */
/*
 * cmextract.lib.h
 */

#ifndef _CMEXTRACT_LIB_H_
#define _CMEXTRACT_LIB_H_

/* src/image/domain/cmextract.lib.c */
Cmovie cmextract(int *b, Cmovie in, Cmovie bg, int X1, int Y1, int T1, int X2, int Y2, int T2, int *Xc, int *Yc, int *Tc, char *r);

#endif /* !_CMEXTRACT_LIB_H_ */
/*
 * cmparitysep.lib.h
 */

#ifndef _CMPARITYSEP_LIB_H_
#define _CMPARITYSEP_LIB_H_

/* src/image/domain/cmparitysep.lib.c */
Cmovie cmparitysep(Cmovie u, char *e, char *l);

#endif /* !_CMPARITYSEP_LIB_H_ */
/*
 * cmzoom.lib.h
 */

#ifndef _CMZOOM_LIB_H_
#define _CMZOOM_LIB_H_

/* src/image/domain/cmzoom.lib.c */
void cmzoom(Cmovie Input, Cmovie Output, char *x_flg, char *y_flg, float *factor, int *o, char *i_flg);

#endif /* !_CMZOOM_LIB_H_ */
/*
 * csample.lib.h
 */

#ifndef _CSAMPLE_LIB_H_
#define _CSAMPLE_LIB_H_

/* src/image/domain/csample.lib.c */
Cimage csample(Cimage in, Cimage out, double step);

#endif /* !_CSAMPLE_LIB_H_ */
/*
 * czoom.lib.h
 */

#ifndef _CZOOM_LIB_H_
#define _CZOOM_LIB_H_

/* src/image/domain/czoom.lib.c */
Cimage czoom(Cimage in, Cimage out, char *x_flg, char *y_flg, float *zoom, int *o, char *i_flg);

#endif /* !_CZOOM_LIB_H_ */
/*
 * fcrop.lib.h
 */

#ifndef _FCROP_LIB_H_
#define _FCROP_LIB_H_

/* src/image/domain/fcrop.lib.c */
Fimage fcrop(Fimage in, Fimage out, float *sx, float *sy, float *z, float *bg, int *o, float *p, double X1, double Y1, double X2, double Y2);

#endif /* !_FCROP_LIB_H_ */
/*
 * fdirspline.lib.h
 */

#ifndef _FDIRSPLINE_LIB_H_
#define _FDIRSPLINE_LIB_H_

/* src/image/domain/fdirspline.lib.c */
Fimage fdirspline(Fimage in, int n, Fimage out);

#endif /* !_FDIRSPLINE_LIB_H_ */
/*
 * fextract.lib.h
 */

#ifndef _FEXTRACT_LIB_H_
#define _FEXTRACT_LIB_H_

/* src/image/domain/fextract.lib.c */
Fimage fextract(float *b, Fimage in, Fimage bg, Fimage out, int X1, int Y1, int X2, int Y2, int *Xc, int *Yc, char *r);

#endif /* !_FEXTRACT_LIB_H_ */
/*
 * finvspline.lib.h
 */

#ifndef _FINVSPLINE_LIB_H_
#define _FINVSPLINE_LIB_H_

/* src/image/domain/finvspline.lib.c */
void finvspline(Fimage in, int order, Fimage out);

#endif /* !_FINVSPLINE_LIB_H_ */
/*
 * flocal_zoom.lib.h
 */

#ifndef _FLOCAL_ZOOM_LIB_H_
#define _FLOCAL_ZOOM_LIB_H_

/* src/image/domain/flocal_zoom.lib.c */
Fimage flocal_zoom(Fimage Input, int *X, int *Y, int *W, int *factor);

#endif /* !_FLOCAL_ZOOM_LIB_H_ */
/*
 * fmaskrot.lib.h
 */

#ifndef _FMASKROT_LIB_H_
#define _FMASKROT_LIB_H_

/* src/image/domain/fmaskrot.lib.c */
Fimage fmaskrot(Fimage in, Fimage out, float *bg, float *s);

#endif /* !_FMASKROT_LIB_H_ */
/*
 * fproj.lib.h
 */

#ifndef _FPROJ_LIB_H_
#define _FPROJ_LIB_H_

/* src/image/domain/fproj.lib.c */
void fproj(Fimage in, Fimage out, int *sx, int *sy, float *bg, int *o, float *p, char *i, double X1, double Y1, double X2, double Y2, double X3, double Y3, float *x4, float *y4);

#endif /* !_FPROJ_LIB_H_ */
/*
 * frot.lib.h
 */

#ifndef _FROT_LIB_H_
#define _FROT_LIB_H_

/* src/image/domain/frot.lib.c */
void frot(Fimage in, Fimage out, float *a, float *b, char *k_flag);

#endif /* !_FROT_LIB_H_ */
/*
 * fsample.lib.h
 */

#ifndef _FSAMPLE_LIB_H_
#define _FSAMPLE_LIB_H_

/* src/image/domain/fsample.lib.c */
Fimage fsample(Fimage in, Fimage out, double step, double *delta, int *norm);

#endif /* !_FSAMPLE_LIB_H_ */
/*
 * fshift.lib.h
 */

#ifndef _FSHIFT_LIB_H_
#define _FSHIFT_LIB_H_

/* src/image/domain/fshift.lib.c */
Fimage fshift(Fimage in, Fimage out, int *x, int *y, int *h, int *i);

#endif /* !_FSHIFT_LIB_H_ */
/*
 * funzoom.lib.h
 */

#ifndef _FUNZOOM_LIB_H_
#define _FUNZOOM_LIB_H_

/* src/image/domain/funzoom.lib.c */
Fimage funzoom(Fimage in, Fimage out, float *z, int *o, float *tx, float *ty);

#endif /* !_FUNZOOM_LIB_H_ */
/*
 * fzoom.lib.h
 */

#ifndef _FZOOM_LIB_H_
#define _FZOOM_LIB_H_

/* src/image/domain/fzoom.lib.c */
Fimage fzoom(Fimage in, Fimage out, char *x_flg, char *y_flg, float *zoom, int *o, float *p, char *i_flg);

#endif /* !_FZOOM_LIB_H_ */
/*
 * fzrt.lib.h
 */

#ifndef _FZRT_LIB_H_
#define _FZRT_LIB_H_

/* src/image/domain/fzrt.lib.c */
void fzrt(Fimage in, Fimage out, double zoom, double angle, double x, double y, int *o, float *p, float *b);

#endif /* !_FZRT_LIB_H_ */
/*
 * amss.lib.h
 */

#ifndef _AMSS_LIB_H_
#define _AMSS_LIB_H_

/* src/image/filter/amss.lib.c */
void amss(char *isotrop, char *power, float *Step, float *MinGrad, float *outputStep, float *firstScale, float *lastScale, Fimage image, Fimage *imageD, Fimage *imageG, Fimage *imageC, Cmovie cmovieD, Cmovie cmovieG, Cmovie cmovieC, char *no_norm);

#endif /* !_AMSS_LIB_H_ */
/*
 * cfdiffuse.lib.h
 */

#ifndef _CFDIFFUSE_LIB_H_
#define _CFDIFFUSE_LIB_H_

/* src/image/filter/cfdiffuse.lib.c */
void cfdiffuse(float *deltat, float *epsilon, Cfimage in, Cfimage out, Fsignal MDiag0, Fsignal MDiag1, Fsignal U0, Cfimage Yimage, Cfimage Vimage, Fimage L2h, Fimage L2v);

#endif /* !_CFDIFFUSE_LIB_H_ */
/*
 * cfmdiffuse.lib.h
 */

#ifndef _CFMDIFFUSE_LIB_H_
#define _CFMDIFFUSE_LIB_H_

/* src/image/filter/cfmdiffuse.lib.c */
void cfmdiffuse(float *deltat, int *N, float *epsilon, Cfimage in, Cfmovie out);

#endif /* !_CFMDIFFUSE_LIB_H_ */
/*
 * cfsharpen.lib.h
 */

#ifndef _CFSHARPEN_LIB_H_
#define _CFSHARPEN_LIB_H_

/* src/image/filter/cfsharpen.lib.c */
Cfimage cfsharpen(Cfimage A, Cfimage B, float *p, char *LumOnly);

#endif /* !_CFSHARPEN_LIB_H_ */
/*
 * erosion.lib.h
 */

#ifndef _EROSION_LIB_H_
#define _EROSION_LIB_H_

/* src/image/filter/erosion.lib.c */
Cimage erosion(Cimage u, Cimage v, float *r, Curve s, int *n, char *i);

#endif /* !_EROSION_LIB_H_ */
/*
 * fconvol.lib.h
 */

#ifndef _FCONVOL_LIB_H_
#define _FCONVOL_LIB_H_

/* src/image/filter/fconvol.lib.c */
void fconvol(Fimage in, Fimage filtre, Fimage out);

#endif /* !_FCONVOL_LIB_H_ */
/*
 * fgrain.lib.h
 */

#ifndef _FGRAIN_LIB_H_
#define _FGRAIN_LIB_H_

/* src/image/filter/fgrain.lib.c */
void fgrain(int *pMinArea, Fimage pFloatImageInput, Fimage pFloatImageOutput);

#endif /* !_FGRAIN_LIB_H_ */
/*
 * flipschitz.lib.h
 */

#ifndef _FLIPSCHITZ_LIB_H_
#define _FLIPSCHITZ_LIB_H_

/* src/image/filter/flipschitz.lib.c */
Fimage flipschitz(Fimage in, double lip, Fimage out, float *r, Curve s, int *n, char *i);

#endif /* !_FLIPSCHITZ_LIB_H_ */
/*
 * forder.lib.h
 */

#ifndef _FORDER_LIB_H_
#define _FORDER_LIB_H_

/* src/image/filter/forder.lib.c */
void forder(int *e, int *N, Fimage in, Fimage out);

#endif /* !_FORDER_LIB_H_ */
/*
 * fsepconvol.lib.h
 */

#ifndef _FSEPCONVOL_LIB_H_
#define _FSEPCONVOL_LIB_H_

/* src/image/filter/fsepconvol.lib.c */
Fimage fsepconvol(Fimage in, Fimage out, Fsignal xker, Fsignal yker, int *width, float *std, int *b);

#endif /* !_FSEPCONVOL_LIB_H_ */
/*
 * fsharpen.lib.h
 */

#ifndef _FSHARPEN_LIB_H_
#define _FSHARPEN_LIB_H_

/* src/image/filter/fsharpen.lib.c */
Fimage fsharpen(Fimage A, Fimage B, float *p);

#endif /* !_FSHARPEN_LIB_H_ */
/*
 * fsmooth.lib.h
 */

#ifndef _FSMOOTH_LIB_H_
#define _FSMOOTH_LIB_H_

/* src/image/filter/fsmooth.lib.c */
void fsmooth(int *S, int *W, Fimage in, Fimage out);

#endif /* !_FSMOOTH_LIB_H_ */
/*
 * heat.lib.h
 */

#ifndef _HEAT_LIB_H_
#define _HEAT_LIB_H_

/* src/image/filter/heat.lib.c */
Fimage heat(Fimage in, Fimage out, int *n, float *s);

#endif /* !_HEAT_LIB_H_ */
/*
 * infsup.lib.h
 */

#ifndef _INFSUP_LIB_H_
#define _INFSUP_LIB_H_

/* src/image/filter/infsup.lib.c */
Cimage infsup(int *Niter, float *deginf, float *degsup, char *average, Cimage image, Fmovie fmovie, Cimage output);

#endif /* !_INFSUP_LIB_H_ */
/*
 * ll_sharp.lib.h
 */

#ifndef _LL_SHARP_LIB_H_
#define _LL_SHARP_LIB_H_

/* src/image/filter/ll_sharp.lib.c */
void ll_sharp(float *pPercentIncreaseArea, Fimage pFloatImageInput, Fimage pFloatImageOutput);

#endif /* !_LL_SHARP_LIB_H_ */
/*
 * mam.lib.h
 */

#ifndef _MAM_LIB_H_
#define _MAM_LIB_H_

/* src/image/filter/mam.lib.c */
void mam(Cmovie in, Cmovie out, float *ptime, float *ppower, int *n_iter, short *pMAXvit, short *pMINvit, short *pfmxa);

#endif /* !_MAM_LIB_H_ */
/*
 * median.lib.h
 */

#ifndef _MEDIAN_LIB_H_
#define _MEDIAN_LIB_H_

/* src/image/filter/median.lib.c */
Cimage median(Cimage u, Cimage v, float *r, Curve s, int *n);

#endif /* !_MEDIAN_LIB_H_ */
/*
 * nlmeans.lib.h
 */

#ifndef _NLMEANS_LIB_H_
#define _NLMEANS_LIB_H_

/* src/image/filter/nlmeans.lib.c */
Fimage nlmeans(Fimage in, Fimage out, double *h, double *a, int *s, int *d, double *c);

#endif /* !_NLMEANS_LIB_H_ */
/*
 * opening.lib.h
 */

#ifndef _OPENING_LIB_H_
#define _OPENING_LIB_H_

/* src/image/filter/opening.lib.c */
Cimage opening(Cimage u, Cimage v, float *r, Curve s, int *n, char *i);

#endif /* !_OPENING_LIB_H_ */
/*
 * osamss.lib.h
 */

#ifndef _OSAMSS_LIB_H_
#define _OSAMSS_LIB_H_

/* src/image/filter/osamss.lib.c */
void osamss(char *isotrop, char *power, float *Step, float *MinGrad, float *firstScale, float *lastScale, Fimage input, Fimage output);

#endif /* !_OSAMSS_LIB_H_ */
/*
 * prolate.lib.h
 */

#ifndef _PROLATE_LIB_H_
#define _PROLATE_LIB_H_

/* src/image/filter/prolate.lib.c */
float prolate(int s, double d, int *m, Fimage ker);

#endif /* !_PROLATE_LIB_H_ */
/*
 * prolatef.lib.h
 */

#ifndef _PROLATEF_LIB_H_
#define _PROLATEF_LIB_H_

/* src/image/filter/prolatef.lib.c */
float prolatef(int s, double d, int *n, Fimage ker, int *p, int *c);

#endif /* !_PROLATEF_LIB_H_ */
/*
 * resthline.lib.h
 */

#ifndef _RESTHLINE_LIB_H_
#define _RESTHLINE_LIB_H_

/* src/image/filter/resthline.lib.c */
void resthline(Cimage u, Cimage v);

#endif /* !_RESTHLINE_LIB_H_ */
/*
 * rotaffin.lib.h
 */

#ifndef _ROTAFFIN_LIB_H_
#define _ROTAFFIN_LIB_H_

/* src/image/filter/rotaffin.lib.c */
void rotaffin(int *Nrota, int *Naffi, int *Size, int *Type, float *Area, int *Definition, double *OptSym, float *Factor, Cimage cimage, Cmovie cmovie);

#endif /* !_ROTAFFIN_LIB_H_ */
/*
 * shock.lib.h
 */

#ifndef _SHOCK_LIB_H_
#define _SHOCK_LIB_H_

/* src/image/filter/shock.lib.c */
Fimage shock(Fimage in, int *n, float *s);

#endif /* !_SHOCK_LIB_H_ */
/*
 * tvdeblur.lib.h
 */

#ifndef _TVDEBLUR_LIB_H_
#define _TVDEBLUR_LIB_H_

/* src/image/filter/tvdeblur.lib.c */
Fimage tvdeblur(Fimage in, Fimage out, Fimage ker, double *s, char *c, char *v, double *e, int *n, double *W, Fimage ref, double *eps, double *p);

#endif /* !_TVDEBLUR_LIB_H_ */
/*
 * tvdenoise.lib.h
 */

#ifndef _TVDENOISE_LIB_H_
#define _TVDENOISE_LIB_H_

/* src/image/filter/tvdenoise.lib.c */
Fimage tvdenoise(Fimage in, Fimage out, double *s, char *c, char *v, double *e, int *n, double *W, Fimage ref, double *eps, double *p);

#endif /* !_TVDENOISE_LIB_H_ */
/*
 * tvdenoise2.lib.h
 */

#ifndef _TVDENOISE2_LIB_H_
#define _TVDENOISE2_LIB_H_

/* src/image/filter/tvdenoise2.lib.c */
Fimage tvdenoise2(Fimage in, Fimage out, double *s, int *v, int *n, double *r, double *W, int *V);

#endif /* !_TVDENOISE2_LIB_H_ */
/*
 * fft2d.lib.h
 */

#ifndef _FFT2D_LIB_H_
#define _FFT2D_LIB_H_

/* src/image/fourier/fft2d.lib.c */
void fft2d(Fimage in_re, Fimage in_im, Fimage out_re, Fimage out_im, char *i_flag);

#endif /* !_FFT2D_LIB_H_ */
/*
 * fft2dpol.lib.h
 */

#ifndef _FFT2DPOL_LIB_H_
#define _FFT2DPOL_LIB_H_

/* src/image/fourier/fft2dpol.lib.c */
void fft2dpol(Fimage in_re, Fimage in_im, Fimage out1, Fimage out2, char *i_flag);

#endif /* !_FFT2DPOL_LIB_H_ */
/*
 * fft2drad.lib.h
 */

#ifndef _FFT2DRAD_LIB_H_
#define _FFT2DRAD_LIB_H_

/* src/image/fourier/fft2drad.lib.c */
Fsignal fft2drad(Fimage in_re, Fimage in_im, Fsignal out, char *l_flag, int *size);

#endif /* !_FFT2DRAD_LIB_H_ */
/*
 * fft2dshrink.lib.h
 */

#ifndef _FFT2DSHRINK_LIB_H_
#define _FFT2DSHRINK_LIB_H_

/* src/image/fourier/fft2dshrink.lib.c */
Fimage fft2dshrink(Fimage in, Fimage out, float *p, char *v);

#endif /* !_FFT2DSHRINK_LIB_H_ */
/*
 * fft2dview.lib.h
 */

#ifndef _FFT2DVIEW_LIB_H_
#define _FFT2DVIEW_LIB_H_

/* src/image/fourier/fft2dview.lib.c */
void fft2dview(int *type, char *h_flag, Fimage in, Fimage out, char *i_flag, float *d);

#endif /* !_FFT2DVIEW_LIB_H_ */
/*
 * fftconvol.lib.h
 */

#ifndef _FFTCONVOL_LIB_H_
#define _FFTCONVOL_LIB_H_

/* src/image/fourier/fftconvol.lib.c */
Fimage fftconvol(Fimage in, Fimage filter, Fimage out);

#endif /* !_FFTCONVOL_LIB_H_ */
/*
 * fftgrad.lib.h
 */

#ifndef _FFTGRAD_LIB_H_
#define _FFTGRAD_LIB_H_

/* src/image/fourier/fftgrad.lib.c */
void fftgrad(Fimage in, Fimage gradx, Fimage grady, Fimage gradn, Fimage gradp);

#endif /* !_FFTGRAD_LIB_H_ */
/*
 * fftrot.lib.h
 */

#ifndef _FFTROT_LIB_H_
#define _FFTROT_LIB_H_

/* src/image/fourier/fftrot.lib.c */
void fftrot(Fimage in, Fimage out, float *a, float *x, float *y, char *o_flag);

#endif /* !_FFTROT_LIB_H_ */
/*
 * fftzoom.lib.h
 */

#ifndef _FFTZOOM_LIB_H_
#define _FFTZOOM_LIB_H_

/* src/image/fourier/fftzoom.lib.c */
void fftzoom(Fimage in, Fimage out, float *z, char *i_flag);

#endif /* !_FFTZOOM_LIB_H_ */
/*
 * fhamming.lib.h
 */

#ifndef _FHAMMING_LIB_H_
#define _FHAMMING_LIB_H_

/* src/image/fourier/fhamming.lib.c */
void fhamming(Fimage in, Fimage out);

#endif /* !_FHAMMING_LIB_H_ */
/*
 * fkeepphase.lib.h
 */

#ifndef _FKEEPPHASE_LIB_H_
#define _FKEEPPHASE_LIB_H_

/* src/image/fourier/fkeepphase.lib.c */
void fkeepphase(Fimage in, Fimage mod, Fimage out);

#endif /* !_FKEEPPHASE_LIB_H_ */
/*
 * frandphase.lib.h
 */

#ifndef _FRANDPHASE_LIB_H_
#define _FRANDPHASE_LIB_H_

/* src/image/fourier/frandphase.lib.c */
void frandphase(Fimage in, Fimage out, char *i_flag);

#endif /* !_FRANDPHASE_LIB_H_ */
/*
 * fshrink2.lib.h
 */

#ifndef _FSHRINK2_LIB_H_
#define _FSHRINK2_LIB_H_

/* src/image/fourier/fshrink2.lib.c */
void fshrink2(Fimage in, Fimage out);

#endif /* !_FSHRINK2_LIB_H_ */
/*
 * fsym2.lib.h
 */

#ifndef _FSYM2_LIB_H_
#define _FSYM2_LIB_H_

/* src/image/fourier/fsym2.lib.c */
void fsym2(Fimage in, Fimage out, char *i);

#endif /* !_FSYM2_LIB_H_ */
/*
 * wiener.lib.h
 */

#ifndef _WIENER_LIB_H_
#define _WIENER_LIB_H_

/* src/image/fourier/wiener.lib.c */
Fimage wiener(Fimage in, Fimage out, Fimage kernel, Fsignal rad, float *g, float *lambda);

#endif /* !_WIENER_LIB_H_ */
/*
 * cccopy.lib.h
 */

#ifndef _CCCOPY_LIB_H_
#define _CCCOPY_LIB_H_

/* src/image/io/cccopy.lib.c */
Ccimage cccopy(Ccimage Input, Ccimage *Output);

#endif /* !_CCCOPY_LIB_H_ */
/*
 * ccmview.lib.h
 */

#ifndef _CCMVIEW_LIB_H_
#define _CCMVIEW_LIB_H_

/* src/image/io/ccmview.lib.c */
void ccmview(Ccmovie input, int *x0, int *y0, float *zoom, int *order, int *loop, int *pause, char *window);

#endif /* !_CCMVIEW_LIB_H_ */
/*
 * ccopy.lib.h
 */

#ifndef _CCOPY_LIB_H_
#define _CCOPY_LIB_H_

/* src/image/io/ccopy.lib.c */
void ccopy(Cimage Input, Cimage *Output);

#endif /* !_CCOPY_LIB_H_ */
/*
 * ccputstring.lib.h
 */

#ifndef _CCPUTSTRING_LIB_H_
#define _CCPUTSTRING_LIB_H_

/* src/image/io/ccputstring.lib.c */
Ccimage ccputstring(Ccimage in, int x, int y, int *c, int *C, float *r, char *str);

#endif /* !_CCPUTSTRING_LIB_H_ */
/*
 * ccview.lib.h
 */

#ifndef _CCVIEW_LIB_H_
#define _CCVIEW_LIB_H_

/* src/image/io/ccview.lib.c */
void ccview(Ccimage image, int *x0, int *y0, float *zoom, int *order, int *no_refresh, char *window);

#endif /* !_CCVIEW_LIB_H_ */
/*
 * cfchgchannels.lib.h
 */

#ifndef _CFCHGCHANNELS_LIB_H_
#define _CFCHGCHANNELS_LIB_H_

/* src/image/io/cfchgchannels.lib.c */
void cfchgchannels(int *Conv, int *Inverse, int *Norm, Cfimage Image, Cfimage TImage);

#endif /* !_CFCHGCHANNELS_LIB_H_ */
/*
 * cfgetchannels.lib.h
 */

#ifndef _CFGETCHANNELS_LIB_H_
#define _CFGETCHANNELS_LIB_H_

/* src/image/io/cfgetchannels.lib.c */
void cfgetchannels(Cfimage Image, Fimage RImage, Fimage GImage, Fimage BImage);

#endif /* !_CFGETCHANNELS_LIB_H_ */
/*
 * cfputchannels.lib.h
 */

#ifndef _CFPUTCHANNELS_LIB_H_
#define _CFPUTCHANNELS_LIB_H_

/* src/image/io/cfputchannels.lib.c */
void cfputchannels(Fimage RImage, Fimage GImage, Fimage BImage, Cfimage Image);

#endif /* !_CFPUTCHANNELS_LIB_H_ */
/*
 * cline_extract.lib.h
 */

#ifndef _CLINE_EXTRACT_LIB_H_
#define _CLINE_EXTRACT_LIB_H_

/* src/image/io/cline_extract.lib.c */
void cline_extract(char *cflag, Cimage Image, Fsignal Signal, long Index, Cimage OutImage);

#endif /* !_CLINE_EXTRACT_LIB_H_ */
/*
 * cmview.lib.h
 */

#ifndef _CMVIEW_LIB_H_
#define _CMVIEW_LIB_H_

/* src/image/io/cmview.lib.c */
void cmview(Cmovie input, int *x0, int *y0, float *zoom, int *order, int *loop, int *pause, char *window);

#endif /* !_CMVIEW_LIB_H_ */
/*
 * cprintasc.lib.h
 */

#ifndef _CPRINTASC_LIB_H_
#define _CPRINTASC_LIB_H_

/* src/image/io/cprintasc.lib.c */
void cprintasc(Cimage u, int *x1, int *y1, int *x2, int *y2, char *verbose);

#endif /* !_CPRINTASC_LIB_H_ */
/*
 * creadasc.lib.h
 */

#ifndef _CREADASC_LIB_H_
#define _CREADASC_LIB_H_

/* src/image/io/creadasc.lib.c */
void creadasc(Cimage u, int dx, int dy);

#endif /* !_CREADASC_LIB_H_ */
/*
 * cview.lib.h
 */

#ifndef _CVIEW_LIB_H_
#define _CVIEW_LIB_H_

/* src/image/io/cview.lib.c */
void cview(Cimage input, int *x0, int *y0, float *zoom, int *order, int *no_refresh, char *window);

#endif /* !_CVIEW_LIB_H_ */
/*
 * fcopy.lib.h
 */

#ifndef _FCOPY_LIB_H_
#define _FCOPY_LIB_H_

/* src/image/io/fcopy.lib.c */
void fcopy(Fimage Input, Fimage *Output);

#endif /* !_FCOPY_LIB_H_ */
/*
 * fline_extract.lib.h
 */

#ifndef _FLINE_EXTRACT_LIB_H_
#define _FLINE_EXTRACT_LIB_H_

/* src/image/io/fline_extract.lib.c */
void fline_extract(char *cflag, Fimage Image, Fsignal Signal, long Index);

#endif /* !_FLINE_EXTRACT_LIB_H_ */
/*
 * flip.lib.h
 */

#ifndef _FLIP_LIB_H_
#define _FLIP_LIB_H_

/* src/image/io/flip.lib.c */
void flip(Fimage in1, Fimage in2, float *z, int *o);

#endif /* !_FLIP_LIB_H_ */
/*
 * fprintasc.lib.h
 */

#ifndef _FPRINTASC_LIB_H_
#define _FPRINTASC_LIB_H_

/* src/image/io/fprintasc.lib.c */
void fprintasc(Fimage u, int *x1, int *y1, int *x2, int *y2, char *verbose);

#endif /* !_FPRINTASC_LIB_H_ */
/*
 * freadasc.lib.h
 */

#ifndef _FREADASC_LIB_H_
#define _FREADASC_LIB_H_

/* src/image/io/freadasc.lib.c */
void freadasc(Fimage u, int *Dx, int *Dy);

#endif /* !_FREADASC_LIB_H_ */
/*
 * fview.lib.h
 */

#ifndef _FVIEW_LIB_H_
#define _FVIEW_LIB_H_

/* src/image/io/fview.lib.c */
void fview(Fimage input, int *x0, int *y0, float *zoom, int *order, int *no_refresh, char *window, char *linear, float *m, float *M, float *d);

#endif /* !_FVIEW_LIB_H_ */
/*
 * cll_remove.lib.h
 */

#ifndef _CLL_REMOVE_LIB_H_
#define _CLL_REMOVE_LIB_H_

/* src/image/level_lines/cll_remove.lib.c */
Cmimage cll_remove(Cmimage cmimage, int *L);

#endif /* !_CLL_REMOVE_LIB_H_ */
/*
 * cml_decompose.lib.h
 */

#ifndef _CML_DECOMPOSE_LIB_H_
#define _CML_DECOMPOSE_LIB_H_

/* src/image/level_lines/cml_decompose.lib.c */
Cmimage cml_decompose(Cmimage cmimage_in, int *ml_opt, int *L, Cfimage image_in);

#endif /* !_CML_DECOMPOSE_LIB_H_ */
/*
 * cml_draw.lib.h
 */

#ifndef _CML_DRAW_LIB_H_
#define _CML_DRAW_LIB_H_

/* src/image/level_lines/cml_draw.lib.c */
Ccimage cml_draw(Cmimage cmimage, Ccimage bimage, char *border, Ccmovie movie);

#endif /* !_CML_DRAW_LIB_H_ */
/*
 * cml_reconstruct.lib.h
 */

#ifndef _CML_RECONSTRUCT_LIB_H_
#define _CML_RECONSTRUCT_LIB_H_

/* src/image/level_lines/cml_reconstruct.lib.c */
Cfimage cml_reconstruct(char *v_flag, Cmimage m_image);

#endif /* !_CML_RECONSTRUCT_LIB_H_ */
/*
 * flst.lib.h
 */

#ifndef _FLST_LIB_H_
#define _FLST_LIB_H_

/* src/image/level_lines/flst.lib.c */
void flst(int *pMinArea, Fimage pImageInput, Shapes pTree);

#endif /* !_FLST_LIB_H_ */
/*
 * flst_bilinear.lib.h
 */

#ifndef _FLST_BILINEAR_LIB_H_
#define _FLST_BILINEAR_LIB_H_

/* src/image/level_lines/flst_bilinear.lib.c */
void flst_bilinear(int *pMinArea, Fimage pImageInput, Shapes pTree);

#endif /* !_FLST_BILINEAR_LIB_H_ */
/*
 * flst_boundary.lib.h
 */

#ifndef _FLST_BOUNDARY_LIB_H_
#define _FLST_BOUNDARY_LIB_H_

/* src/image/level_lines/flst_boundary.lib.c */
Flist flst_boundary(Shapes pTree, Shape pShape, Flist pBoundary);

#endif /* !_FLST_BOUNDARY_LIB_H_ */
/*
 * flst_pixels.lib.h
 */

#ifndef _FLST_PIXELS_LIB_H_
#define _FLST_PIXELS_LIB_H_

/* src/image/level_lines/flst_pixels.lib.c */
void flst_pixels(Shapes pTree);

#endif /* !_FLST_PIXELS_LIB_H_ */
/*
 * flst_reconstruct.lib.h
 */

#ifndef _FLST_RECONSTRUCT_LIB_H_
#define _FLST_RECONSTRUCT_LIB_H_

/* src/image/level_lines/flst_reconstruct.lib.c */
void flst_reconstruct(Shapes pTree, Fimage pFloatImageOutput);

#endif /* !_FLST_RECONSTRUCT_LIB_H_ */
/*
 * flstb_boundary.lib.h
 */

#ifndef _FLSTB_BOUNDARY_LIB_H_
#define _FLSTB_BOUNDARY_LIB_H_

/* src/image/level_lines/flstb_boundary.lib.c */
Flist flstb_boundary(int *pPrecision, Fimage pImage, Shapes pTree, Shape pShape, Flist pDualchain, Flist pBoundary, char *ctabtabSaddleValues);

#endif /* !_FLSTB_BOUNDARY_LIB_H_ */
/*
 * flstb_dual.lib.h
 */

#ifndef _FLSTB_DUAL_LIB_H_
#define _FLSTB_DUAL_LIB_H_

/* src/image/level_lines/flstb_dual.lib.c */
void flstb_dual(Shapes pTree, Shapes pDualTree);

#endif /* !_FLSTB_DUAL_LIB_H_ */
/*
 * flstb_dualchain.lib.h
 */

#ifndef _FLSTB_DUALCHAIN_LIB_H_
#define _FLSTB_DUALCHAIN_LIB_H_

/* src/image/level_lines/flstb_dualchain.lib.c */
void flstb_dualchain(Shapes pTree, Shape pShape, Flist pBoundary, char *ctabtabSaddleValues);

#endif /* !_FLSTB_DUALCHAIN_LIB_H_ */
/*
 * flstb_quantize.lib.h
 */

#ifndef _FLSTB_QUANTIZE_LIB_H_
#define _FLSTB_QUANTIZE_LIB_H_

/* src/image/level_lines/flstb_quantize.lib.c */
void flstb_quantize(Fsignal pLevels, float *pOffset, float *pStep, Shapes pTree, Shapes pQuantizedTree);

#endif /* !_FLSTB_QUANTIZE_LIB_H_ */
/*
 * flstb_tv.lib.h
 */

#ifndef _FLSTB_TV_LIB_H_
#define _FLSTB_TV_LIB_H_

/* src/image/level_lines/flstb_tv.lib.c */
void flstb_tv(float *pScale, float *pQuantizationLevel, int *pQuantizationCurve, Shapes pTree, Fimage pImage);

#endif /* !_FLSTB_TV_LIB_H_ */
/*
 * fml_ml.lib.h
 */

#ifndef _FML_ML_LIB_H_
#define _FML_ML_LIB_H_

/* src/image/level_lines/fml_ml.lib.c */
Morpho_line fml_ml(Fmorpho_line flline);

#endif /* !_FML_ML_LIB_H_ */
/*
 * fsaddles.lib.h
 */

#ifndef _FSADDLES_LIB_H_
#define _FSADDLES_LIB_H_

/* src/image/level_lines/fsaddles.lib.c */
int fsaddles(Fimage pImage, Fimage pSaddlesImage);

#endif /* !_FSADDLES_LIB_H_ */
/*
 * ll_distance.lib.h
 */

#ifndef _LL_DISTANCE_LIB_H_
#define _LL_DISTANCE_LIB_H_

/* src/image/level_lines/ll_distance.lib.c */
void ll_distance(Fimage in, Fimage out, float *level, int *MaxDist, Fimage nearest);

#endif /* !_LL_DISTANCE_LIB_H_ */
/*
 * ll_extract.lib.h
 */

#ifndef _LL_EXTRACT_LIB_H_
#define _LL_EXTRACT_LIB_H_

/* src/image/level_lines/ll_extract.lib.c */
Flists ll_extract(Fimage in, Fsignal levels, float *offset, float *step, int *prec, int *area, Shapes tree, char *z);

#endif /* !_LL_EXTRACT_LIB_H_ */
/*
 * ll_remove.lib.h
 */

#ifndef _LL_REMOVE_LIB_H_
#define _LL_REMOVE_LIB_H_

/* src/image/level_lines/ll_remove.lib.c */
Mimage ll_remove(Mimage mimage, int *L);

#endif /* !_LL_REMOVE_LIB_H_ */
/*
 * llmap.lib.h
 */

#ifndef _LLMAP_LIB_H_
#define _LLMAP_LIB_H_

/* src/image/level_lines/llmap.lib.c */
Cimage llmap(short *ls, char *tmap, Cimage input, Cimage output);

#endif /* !_LLMAP_LIB_H_ */
/*
 * llremove.lib.h
 */

#ifndef _LLREMOVE_LIB_H_
#define _LLREMOVE_LIB_H_

/* src/image/level_lines/llremove.lib.c */
Fimage llremove(Fimage Input, int *Lmin, int *Lmax, char *invert);

#endif /* !_LLREMOVE_LIB_H_ */
/*
 * llview.lib.h
 */

#ifndef _LLVIEW_LIB_H_
#define _LLVIEW_LIB_H_

/* src/image/level_lines/llview.lib.c */
void llview(Fimage in, float *z, int *s, int *l, int *d, int *i, int *b, Ccimage *out, char *n);

#endif /* !_LLVIEW_LIB_H_ */
/*
 * ml_decompose.lib.h
 */

#ifndef _ML_DECOMPOSE_LIB_H_
#define _ML_DECOMPOSE_LIB_H_

/* src/image/level_lines/ml_decompose.lib.c */
Mimage ml_decompose(Mimage m_image_in, int *ml_opt, char *m_flag, Fimage image_in);

#endif /* !_ML_DECOMPOSE_LIB_H_ */
/*
 * ml_draw.lib.h
 */

#ifndef _ML_DRAW_LIB_H_
#define _ML_DRAW_LIB_H_

/* src/image/level_lines/ml_draw.lib.c */
Cimage ml_draw(Mimage m_image, Cimage bimage, char *border, Cmovie movie);

#endif /* !_ML_DRAW_LIB_H_ */
/*
 * ml_extract.lib.h
 */

#ifndef _ML_EXTRACT_LIB_H_
#define _ML_EXTRACT_LIB_H_

/* src/image/level_lines/ml_extract.lib.c */
void ml_extract(float *level, Fsignal levels, int *opt, Cimage c_out, char *m_flag, Fimage image_org, Mimage m_image);

#endif /* !_ML_EXTRACT_LIB_H_ */
/*
 * ml_fml.lib.h
 */

#ifndef _ML_FML_LIB_H_
#define _ML_FML_LIB_H_

/* src/image/level_lines/ml_fml.lib.c */
Fmorpho_line ml_fml(Morpho_line lline);

#endif /* !_ML_FML_LIB_H_ */
/*
 * ml_reconstruct.lib.h
 */

#ifndef _ML_RECONSTRUCT_LIB_H_
#define _ML_RECONSTRUCT_LIB_H_

/* src/image/level_lines/ml_reconstruct.lib.c */
Fimage ml_reconstruct(char *v_flag, Mimage m_image);

#endif /* !_ML_RECONSTRUCT_LIB_H_ */
/*
 * mscarea.lib.h
 */

#ifndef _MSCAREA_LIB_H_
#define _MSCAREA_LIB_H_

/* src/image/level_lines/mscarea.lib.c */
int mscarea(char *connex8, Cimage U, Cimage O, int *stoparea, int a, int b, int x0, int y0);

#endif /* !_MSCAREA_LIB_H_ */
/*
 * tjmap.lib.h
 */

#ifndef _TJMAP_LIB_H_
#define _TJMAP_LIB_H_

/* src/image/level_lines/tjmap.lib.c */
int tjmap(char *connex8, char *values, int *tarea, int *tquant, Cimage U, Cimage J);

#endif /* !_TJMAP_LIB_H_ */
/*
 * tjpoint.lib.h
 */

#ifndef _TJPOINT_LIB_H_
#define _TJPOINT_LIB_H_

/* src/image/level_lines/tjpoint.lib.c */
int tjpoint(char *connex8, int *tarea, int *tquant, Cimage U, int x0, int y0, int *lambda, int *mu, int *xlambda, int *ylambda, int *xmu, int *ymu, unsigned char *M, int *P);

#endif /* !_TJPOINT_LIB_H_ */
/*
 * ccdisocclusion.lib.h
 */

#ifndef _CCDISOCCLUSION_LIB_H_
#define _CCDISOCCLUSION_LIB_H_

/* src/image/misc/ccdisocclusion.lib.c */
void ccdisocclusion(Ccimage Input, Ccimage Output, Fimage Holes, int *energy_type, char *angle);

#endif /* !_CCDISOCCLUSION_LIB_H_ */
/*
 * cdisc.lib.h
 */

#ifndef _CDISC_LIB_H_
#define _CDISC_LIB_H_

/* src/image/misc/cdisc.lib.c */
Cimage cdisc(Cimage out, int nx, int ny, float *x, float *y, float *r);

#endif /* !_CDISC_LIB_H_ */
/*
 * disocclusion.lib.h
 */

#ifndef _DISOCCLUSION_LIB_H_
#define _DISOCCLUSION_LIB_H_

/* src/image/misc/disocclusion.lib.c */
void disocclusion(Cimage Input, Cimage Output, Fimage Holes, int *energy_type, char *angle);

#endif /* !_DISOCCLUSION_LIB_H_ */
/*
 * drawocclusion.lib.h
 */

#ifndef _DRAWOCCLUSION_LIB_H_
#define _DRAWOCCLUSION_LIB_H_

/* src/image/misc/drawocclusion.lib.c */
void drawocclusion(Cimage image, Fimage labelimage, Cimage *hole_image, char *window, int *zoom, int *gray);

#endif /* !_DRAWOCCLUSION_LIB_H_ */
/*
 * emptypoly.lib.h
 */

#ifndef _EMPTYPOLY_LIB_H_
#define _EMPTYPOLY_LIB_H_

/* src/image/misc/emptypoly.lib.c */
Cimage emptypoly(Cimage A, Cimage B);

#endif /* !_EMPTYPOLY_LIB_H_ */
/*
 * lsnakes.lib.h
 */

#ifndef _LSNAKES_LIB_H_
#define _LSNAKES_LIB_H_

/* src/image/misc/lsnakes.lib.c */
Fimage lsnakes(Fimage in, Fimage ref, int *Niter, float *thre, float *force);

#endif /* !_LSNAKES_LIB_H_ */
/*
 * lsnakes_demo.lib.h
 */

#ifndef _LSNAKES_DEMO_LIB_H_
#define _LSNAKES_DEMO_LIB_H_

/* src/image/misc/lsnakes_demo.lib.c */
void lsnakes_demo(Cimage u, Cmovie out, int *Niter, int *Nframes, float *thre, float *force);

#endif /* !_LSNAKES_DEMO_LIB_H_ */
/*
 * mac_snakes.lib.h
 */

#ifndef _MAC_SNAKES_LIB_H_
#define _MAC_SNAKES_LIB_H_

/* src/image/misc/mac_snakes.lib.c */
Dlists mac_snakes(Fimage u, Dlists in, int *niter, double *step, double *power, char *v, float *V);

#endif /* !_MAC_SNAKES_LIB_H_ */
/*
 * skeleton.lib.h
 */

#ifndef _SKELETON_LIB_H_
#define _SKELETON_LIB_H_

/* src/image/misc/skeleton.lib.c */
Cmovie skeleton(int *iteration, int *infsup_iter, int *extremite, char *average, Fmovie cmovie, Cimage image, Cmovie output);

#endif /* !_SKELETON_LIB_H_ */
/*
 * thinning.lib.h
 */

#ifndef _THINNING_LIB_H_
#define _THINNING_LIB_H_

/* src/image/misc/thinning.lib.c */
Cimage thinning(int *niter, char *inv, Cmovie M, Cimage imageI, Cimage imageO);

#endif /* !_THINNING_LIB_H_ */
/*
 * hs_flow.lib.h
 */

#ifndef _HS_FLOW_LIB_H_
#define _HS_FLOW_LIB_H_

/* src/image/motion/hs_flow.lib.c */
void hs_flow(int *niter, float *alpha, Fmovie in, Fmovie xflow, Fmovie yflow);

#endif /* !_HS_FLOW_LIB_H_ */
/*
 * motionseg.lib.h
 */

#ifndef _MOTIONSEG_LIB_H_
#define _MOTIONSEG_LIB_H_

/* src/image/motion/motionseg.lib.c */
void motionseg(int *n, float *prec, float *e, float *alphac, float *alphabr, float *alphacr, float *seu, Fmovie N, Fmovie C, Fimage B);

#endif /* !_MOTIONSEG_LIB_H_ */
/*
 * ofdraw.lib.h
 */

#ifndef _OFDRAW_LIB_H_
#define _OFDRAW_LIB_H_

/* src/image/motion/ofdraw.lib.c */
Cmovie ofdraw(Fmovie U, Fmovie V, int *a, float *m, int *p, int *z, float *h);

#endif /* !_OFDRAW_LIB_H_ */
/*
 * ws_flow.lib.h
 */

#ifndef _WS_FLOW_LIB_H_
#define _WS_FLOW_LIB_H_

/* src/image/motion/ws_flow.lib.c */
void ws_flow(float *percent, int *n, float *tau, float *lambda, float *eps, float *alpha, Fmovie norm, Fmovie movie, Fmovie wsU, Fmovie wsV);

#endif /* !_WS_FLOW_LIB_H_ */
/*
 * cfdiff.lib.h
 */

#ifndef _CFDIFF_LIB_H_
#define _CFDIFF_LIB_H_

/* src/image/operations/cfdiff.lib.c */
void cfdiff(char *absd, Cfimage A, Cfimage B, Cfimage O);

#endif /* !_CFDIFF_LIB_H_ */
/*
 * fabso.lib.h
 */

#ifndef _FABSO_LIB_H_
#define _FABSO_LIB_H_

/* src/image/operations/fabso.lib.c */
void fabso(Fimage A, Fimage O);

#endif /* !_FABSO_LIB_H_ */
/*
 * fadd.lib.h
 */

#ifndef _FADD_LIB_H_
#define _FADD_LIB_H_

/* src/image/operations/fadd.lib.c */
Fimage fadd(Fimage A, Fimage B, Fimage C, float *min, float *max, char *a);

#endif /* !_FADD_LIB_H_ */
/*
 * faxpb.lib.h
 */

#ifndef _FAXPB_LIB_H_
#define _FAXPB_LIB_H_

/* src/image/operations/faxpb.lib.c */
void faxpb(Fimage in, Fimage out, float *a, float *s, float *b, float *m, char *k, float *M, float *N);

#endif /* !_FAXPB_LIB_H_ */
/*
 * fconst.lib.h
 */

#ifndef _FCONST_LIB_H_
#define _FCONST_LIB_H_

/* src/image/operations/fconst.lib.c */
Fimage fconst(double g, int x, int y);

#endif /* !_FCONST_LIB_H_ */
/*
 * fderiv.lib.h
 */

#ifndef _FDERIV_LIB_H_
#define _FDERIV_LIB_H_

/* src/image/operations/fderiv.lib.c */
void fderiv(Fimage in, Fimage curv, Fimage anti, Fimage canny, Fimage laplacian, Fimage gradx, Fimage grady, Fimage gradn, Fimage gradp, float *MinGrad, int *nsize);

#endif /* !_FDERIV_LIB_H_ */
/*
 * fdiff.lib.h
 */

#ifndef _FDIFF_LIB_H_
#define _FDIFF_LIB_H_

/* src/image/operations/fdiff.lib.c */
void fdiff(char *absd, Fimage A, Fimage B, Fimage O);

#endif /* !_FDIFF_LIB_H_ */
/*
 * fentropy.lib.h
 */

#ifndef _FENTROPY_LIB_H_
#define _FENTROPY_LIB_H_

/* src/image/operations/fentropy.lib.c */
float fentropy(Fimage A);

#endif /* !_FENTROPY_LIB_H_ */
/*
 * finfo.lib.h
 */

#ifndef _FINFO_LIB_H_
#define _FINFO_LIB_H_

/* src/image/operations/finfo.lib.c */
void finfo(Fimage u);

#endif /* !_FINFO_LIB_H_ */
/*
 * fmask.lib.h
 */

#ifndef _FMASK_LIB_H_
#define _FMASK_LIB_H_

/* src/image/operations/fmask.lib.c */
void fmask(Fimage mask, Fimage A, Fimage B, Fimage out, char *i_flag, int *v, float *c);

#endif /* !_FMASK_LIB_H_ */
/*
 * fmean.lib.h
 */

#ifndef _FMEAN_LIB_H_
#define _FMEAN_LIB_H_

/* src/image/operations/fmean.lib.c */
float fmean(Fimage A);

#endif /* !_FMEAN_LIB_H_ */
/*
 * fmse.lib.h
 */

#ifndef _FMSE_LIB_H_
#define _FMSE_LIB_H_

/* src/image/operations/fmse.lib.c */
void fmse(Fimage Img1, Fimage Img2, int *Norm, char *PsnrFlg, double *SNR, double *PSNR, double *MSE, double *MRD);

#endif /* !_FMSE_LIB_H_ */
/*
 * fnorm.lib.h
 */

#ifndef _FNORM_LIB_H_
#define _FNORM_LIB_H_

/* src/image/operations/fnorm.lib.c */
float fnorm(Fimage in, Fimage ref, float *p, char *s, char *v, int *b, char *n, float *t);

#endif /* !_FNORM_LIB_H_ */
/*
 * fop.lib.h
 */

#ifndef _FOP_LIB_H_
#define _FOP_LIB_H_

/* src/image/operations/fop.lib.c */
void fop(Fimage B, Fimage out, Fimage A, float *a, char *plus, char *minus, char *times, char *divide, char *dist, char *norm, char *inf, char *sup, char *less, char *greater, char *equal);

#endif /* !_FOP_LIB_H_ */
/*
 * fpset.lib.h
 */

#ifndef _FPSET_LIB_H_
#define _FPSET_LIB_H_

/* src/image/operations/fpset.lib.c */
Fimage fpset(Fimage in, int x, int y, double g);

#endif /* !_FPSET_LIB_H_ */
/*
 * fpsnr255.lib.h
 */

#ifndef _FPSNR255_LIB_H_
#define _FPSNR255_LIB_H_

/* src/image/operations/fpsnr255.lib.c */
double fpsnr255(int *Norm, Fimage image);

#endif /* !_FPSNR255_LIB_H_ */
/*
 * frthre.lib.h
 */

#ifndef _FRTHRE_LIB_H_
#define _FRTHRE_LIB_H_

/* src/image/operations/frthre.lib.c */
void frthre(Fimage A, Fimage B, float *noise);

#endif /* !_FRTHRE_LIB_H_ */
/*
 * fsize.lib.h
 */

#ifndef _FSIZE_LIB_H_
#define _FSIZE_LIB_H_

/* src/image/operations/fsize.lib.c */
void fsize(Fimage in);

#endif /* !_FSIZE_LIB_H_ */
/*
 * fvar.lib.h
 */

#ifndef _FVAR_LIB_H_
#define _FVAR_LIB_H_

/* src/image/operations/fvar.lib.c */
float fvar(Fimage A, int *e, int *s);

#endif /* !_FVAR_LIB_H_ */
/*
 * mschannel.lib.h
 */

#ifndef _MSCHANNEL_LIB_H_
#define _MSCHANNEL_LIB_H_

/* src/image/seg/mschannel.lib.c */
void mschannel(int *N, int *S, int *W, int *p, Fimage in, Fmovie mov);

#endif /* !_MSCHANNEL_LIB_H_ */
/*
 * msegct.lib.h
 */

#ifndef _MSEGCT_LIB_H_
#define _MSEGCT_LIB_H_

/* src/image/seg/msegct.lib.c */
Cimage msegct(Fsignal weight, int *sgrid, int *nb_of_regions, float *lambda, Curves curves, Fmovie u, int *f_nb_of_regions, float *f_lambda, Fmovie orig_data);

#endif /* !_MSEGCT_LIB_H_ */
/*
 * one_levelset.lib.h
 */

#ifndef _ONE_LEVELSET_LIB_H_
#define _ONE_LEVELSET_LIB_H_

/* src/image/seg/one_levelset.lib.c */
void one_levelset(float *level, Cimage cb, Fpolygons pb, Fimage fu, Cimage cu, Fimage image_org);

#endif /* !_ONE_LEVELSET_LIB_H_ */
/*
 * segct.lib.h
 */

#ifndef _SEGCT_LIB_H_
#define _SEGCT_LIB_H_

/* src/image/seg/segct.lib.c */
Cimage segct(int *sgrid, int *nb_of_regions, float *lambda, Curves curves, Fimage u, int *f_nb_of_regions, float *f_lambda, Fimage image_org);

#endif /* !_SEGCT_LIB_H_ */
/*
 * segtxt.lib.h
 */

#ifndef _SEGTXT_LIB_H_
#define _SEGTXT_LIB_H_

/* src/image/seg/segtxt.lib.c */
Cimage segtxt(int *N, int *S, int *W, int *p, int *nr, Fimage in, Fmovie mov);

#endif /* !_SEGTXT_LIB_H_ */
/*
 * sr_distance.lib.h
 */

#ifndef _SR_DISTANCE_LIB_H_
#define _SR_DISTANCE_LIB_H_

/* src/image/shape_recognition/sr_distance.lib.c */
float sr_distance(Fcurves Shape1, Fcurves Shape2);

#endif /* !_SR_DISTANCE_LIB_H_ */
/*
 * sr_genhypo.lib.h
 */

#ifndef _SR_GENHYPO_LIB_H_
#define _SR_GENHYPO_LIB_H_

/* src/image/shape_recognition/sr_genhypo.lib.c */
Fcurves sr_genhypo(Fimage sg, Cimage img, Fimage hypo_list);

#endif /* !_SR_GENHYPO_LIB_H_ */
/*
 * sr_normalize.lib.h
 */

#ifndef _SR_NORMALIZE_LIB_H_
#define _SR_NORMALIZE_LIB_H_

/* src/image/shape_recognition/sr_normalize.lib.c */
Fcurves sr_normalize(Fcurves in);

#endif /* !_SR_NORMALIZE_LIB_H_ */
/*
 * sr_signature.lib.h
 */

#ifndef _SR_SIGNATURE_LIB_H_
#define _SR_SIGNATURE_LIB_H_

/* src/image/shape_recognition/sr_signature.lib.c */
Fimage sr_signature(Fcurves in, int *n, Fimage base);

#endif /* !_SR_SIGNATURE_LIB_H_ */
/*
 * amle.lib.h
 */

#ifndef _AMLE_LIB_H_
#define _AMLE_LIB_H_

/* src/image/values/amle.lib.c */
void amle(Cimage Init, Cimage Input, Fimage Output, float *omega, int *n, float *ht, float *mse);

#endif /* !_AMLE_LIB_H_ */
/*
 * amle3d.lib.h
 */

#ifndef _AMLE3D_LIB_H_
#define _AMLE3D_LIB_H_

/* src/image/values/amle3d.lib.c */
void amle3d(int *num, Fmovie init, Fmovie in, Fmovie out);

#endif /* !_AMLE3D_LIB_H_ */
/*
 * amle3d_init.lib.h
 */

#ifndef _AMLE3D_INIT_LIB_H_
#define _AMLE3D_INIT_LIB_H_

/* src/image/values/amle3d_init.lib.c */
void amle3d_init(Fmovie in, double delta, Fmovie out);

#endif /* !_AMLE3D_INIT_LIB_H_ */
/*
 * amle_init.lib.h
 */

#ifndef _AMLE_INIT_LIB_H_
#define _AMLE_INIT_LIB_H_

/* src/image/values/amle_init.lib.c */
void amle_init(Fimage in, double delta, Fimage out);

#endif /* !_AMLE_INIT_LIB_H_ */
/*
 * bicontrast.lib.h
 */

#ifndef _BICONTRAST_LIB_H_
#define _BICONTRAST_LIB_H_

/* src/image/values/bicontrast.lib.c */
void bicontrast(Fimage u, Fimage v, char *verb, Flist r, Flist g, Fimage out);

#endif /* !_BICONTRAST_LIB_H_ */
/*
 * binarize.lib.h
 */

#ifndef _BINARIZE_LIB_H_
#define _BINARIZE_LIB_H_

/* src/image/values/binarize.lib.c */
void binarize(Fimage in, Cimage out, float *t, char *i);

#endif /* !_BINARIZE_LIB_H_ */
/*
 * ccontrast.lib.h
 */

#ifndef _CCONTRAST_LIB_H_
#define _CCONTRAST_LIB_H_

/* src/image/values/ccontrast.lib.c */
void ccontrast(Cimage in, Cimage out, float *g);

#endif /* !_CCONTRAST_LIB_H_ */
/*
 * ccontrast_local.lib.h
 */

#ifndef _CCONTRAST_LOCAL_LIB_H_
#define _CCONTRAST_LOCAL_LIB_H_

/* src/image/values/ccontrast_local.lib.c */
void ccontrast_local(Cimage in, Cimage out, int *d, char *n_flag);

#endif /* !_CCONTRAST_LOCAL_LIB_H_ */
/*
 * cfquant.lib.h
 */

#ifndef _CFQUANT_LIB_H_
#define _CFQUANT_LIB_H_

/* src/image/values/cfquant.lib.c */
void cfquant(Cfimage A, Cfimage Q, int Mr, int Mg, int Mb, char *left);

#endif /* !_CFQUANT_LIB_H_ */
/*
 * chisto.lib.h
 */

#ifndef _CHISTO_LIB_H_
#define _CHISTO_LIB_H_

/* src/image/values/chisto.lib.c */
Fsignal chisto(Cimage A, Fsignal S, char *i_flag);

#endif /* !_CHISTO_LIB_H_ */
/*
 * cmnoise.lib.h
 */

#ifndef _CMNOISE_LIB_H_
#define _CMNOISE_LIB_H_

/* src/image/values/cmnoise.lib.c */
Cmovie cmnoise(Cmovie in, float *std, float *p, float *q);

#endif /* !_CMNOISE_LIB_H_ */
/*
 * cnoise.lib.h
 */

#ifndef _CNOISE_LIB_H_
#define _CNOISE_LIB_H_

/* src/image/values/cnoise.lib.c */
void cnoise(Cimage u, Cimage v, float *std, float *p, float *q, char *n_flag);

#endif /* !_CNOISE_LIB_H_ */
/*
 * fcontrast.lib.h
 */

#ifndef _FCONTRAST_LIB_H_
#define _FCONTRAST_LIB_H_

/* src/image/values/fcontrast.lib.c */
Fimage fcontrast(Fimage in, Flist g);

#endif /* !_FCONTRAST_LIB_H_ */
/*
 * fhisto.lib.h
 */

#ifndef _FHISTO_LIB_H_
#define _FHISTO_LIB_H_

/* src/image/values/fhisto.lib.c */
Fsignal fhisto(Fimage in, Fsignal out, float *l, float *r, int *n, float *s, char *t, Fimage w);

#endif /* !_FHISTO_LIB_H_ */
/*
 * flgamma.lib.h
 */

#ifndef _FLGAMMA_LIB_H_
#define _FLGAMMA_LIB_H_

/* src/image/values/flgamma.lib.c */
Flist flgamma(Flist out, float *g, float *s, int *n, float *f);

#endif /* !_FLGAMMA_LIB_H_ */
/*
 * fnoise.lib.h
 */

#ifndef _FNOISE_LIB_H_
#define _FNOISE_LIB_H_

/* src/image/values/fnoise.lib.c */
void fnoise(Fimage u, Fimage v, float *std, float *p, float *q, char *n_flag);

#endif /* !_FNOISE_LIB_H_ */
/*
 * fquant.lib.h
 */

#ifndef _FQUANT_LIB_H_
#define _FQUANT_LIB_H_

/* src/image/values/fquant.lib.c */
float fquant(Fimage A, Fimage Q, int M, char *left, float *min, float *max);

#endif /* !_FQUANT_LIB_H_ */
/*
 * frank.lib.h
 */

#ifndef _FRANK_LIB_H_
#define _FRANK_LIB_H_

/* src/image/values/frank.lib.c */
void frank(Fimage u, Fimage rank, Flist g, float *w, int *c);

#endif /* !_FRANK_LIB_H_ */
/*
 * fthre.lib.h
 */

#ifndef _FTHRE_LIB_H_
#define _FTHRE_LIB_H_

/* src/image/values/fthre.lib.c */
Fimage fthre(Fimage in, Fimage out, char *norm, char *N, float *min, float *max, float *p, float *q, float *d, char *aff, char *linear);

#endif /* !_FTHRE_LIB_H_ */
/*
 * fvalues.lib.h
 */

#ifndef _FVALUES_LIB_H_
#define _FVALUES_LIB_H_

/* src/image/values/fvalues.lib.c */
Fsignal fvalues(char *i_flag, Fsignal multiplicity, Fimage image_rank, Fimage image_in);

#endif /* !_FVALUES_LIB_H_ */
/*
 * entropy.lib.h
 */

#ifndef _ENTROPY_LIB_H_
#define _ENTROPY_LIB_H_

/* src/signal/entropy.lib.c */
void entropy(Fsignal Histo, double *Entropy);

#endif /* !_ENTROPY_LIB_H_ */
/*
 * fct1d.lib.h
 */

#ifndef _FCT1D_LIB_H_
#define _FCT1D_LIB_H_

/* src/signal/fct1d.lib.c */
void fct1d(Fsignal X, Fsignal Y, char *inverse);

#endif /* !_FCT1D_LIB_H_ */
/*
 * fft1d.lib.h
 */

#ifndef _FFT1D_LIB_H_
#define _FFT1D_LIB_H_

/* src/signal/fft1d.lib.c */
void fft1d(Fsignal Xr, Fsignal Xi, Fsignal Yr, Fsignal Yi, char *inverse);

#endif /* !_FFT1D_LIB_H_ */
/*
 * fft1dshrink.lib.h
 */

#ifndef _FFT1DSHRINK_LIB_H_
#define _FFT1DSHRINK_LIB_H_

/* src/signal/fft1dshrink.lib.c */
Fsignal fft1dshrink(Fsignal in, Fsignal out, float *p, char *v);

#endif /* !_FFT1DSHRINK_LIB_H_ */
/*
 * saxpb.lib.h
 */

#ifndef _SAXPB_LIB_H_
#define _SAXPB_LIB_H_

/* src/signal/saxpb.lib.c */
void saxpb(Fsignal in, Fsignal out, float *a, float *s, float *b, float *m, char *k, float *M);

#endif /* !_SAXPB_LIB_H_ */
/*
 * sconst.lib.h
 */

#ifndef _SCONST_LIB_H_
#define _SCONST_LIB_H_

/* src/signal/sconst.lib.c */
void sconst(int *size, float *A, Fsignal signal);

#endif /* !_SCONST_LIB_H_ */
/*
 * sderiv.lib.h
 */

#ifndef _SDERIV_LIB_H_
#define _SDERIV_LIB_H_

/* src/signal/sderiv.lib.c */
void sderiv(Fsignal A, Fsignal B);

#endif /* !_SDERIV_LIB_H_ */
/*
 * sdirac.lib.h
 */

#ifndef _SDIRAC_LIB_H_
#define _SDIRAC_LIB_H_

/* src/signal/sdirac.lib.c */
void sdirac(int *size, float *A, Fsignal signal);

#endif /* !_SDIRAC_LIB_H_ */
/*
 * sgauss.lib.h
 */

#ifndef _SGAUSS_LIB_H_
#define _SGAUSS_LIB_H_

/* src/signal/sgauss.lib.c */
Fsignal sgauss(double std, Fsignal out, int *size, float *prec);

#endif /* !_SGAUSS_LIB_H_ */
/*
 * sinfo.lib.h
 */

#ifndef _SINFO_LIB_H_
#define _SINFO_LIB_H_

/* src/signal/sinfo.lib.c */
void sinfo(Fsignal s);

#endif /* !_SINFO_LIB_H_ */
/*
 * sintegral.lib.h
 */

#ifndef _SINTEGRAL_LIB_H_
#define _SINTEGRAL_LIB_H_

/* src/signal/sintegral.lib.c */
Fsignal sintegral(Fsignal in, char *n, char *r);

#endif /* !_SINTEGRAL_LIB_H_ */
/*
 * smse.lib.h
 */

#ifndef _SMSE_LIB_H_
#define _SMSE_LIB_H_

/* src/signal/smse.lib.c */
void smse(Fsignal Sig1, Fsignal Sig2, int *Norm, double *SNR, double *PSNR, double *MSE, double *MRD);

#endif /* !_SMSE_LIB_H_ */
/*
 * snoise.lib.h
 */

#ifndef _SNOISE_LIB_H_
#define _SNOISE_LIB_H_

/* src/signal/snoise.lib.c */
Fsignal snoise(Fsignal in, Fsignal out, float *std, float *t, char *n_flag);

#endif /* !_SNOISE_LIB_H_ */
/*
 * snorm.lib.h
 */

#ifndef _SNORM_LIB_H_
#define _SNORM_LIB_H_

/* src/signal/snorm.lib.c */
float snorm(Fsignal in, Fsignal ref, float *p, char *s, char *v, int *b, char *n, float *t);

#endif /* !_SNORM_LIB_H_ */
/*
 * sop.lib.h
 */

#ifndef _SOP_LIB_H_
#define _SOP_LIB_H_

/* src/signal/sop.lib.c */
void sop(Fsignal B, Fsignal out, Fsignal A, float *a, char *plus, char *minus, char *times, char *divide, char *dist, char *norm, char *inf, char *sup, char *less, char *greater, char *equal);

#endif /* !_SOP_LIB_H_ */
/*
 * splot.lib.h
 */

#ifndef _SPLOT_LIB_H_
#define _SPLOT_LIB_H_

/* src/signal/splot.lib.c */
void splot(Fsignal in, int *x_0, int *y_0, int *sx, int *sy, int *no_refresh, char *window, Ccimage *out, char *n);

#endif /* !_SPLOT_LIB_H_ */
/*
 * sprintasc.lib.h
 */

#ifndef _SPRINTASC_LIB_H_
#define _SPRINTASC_LIB_H_

/* src/signal/sprintasc.lib.c */
void sprintasc(Fsignal s, int *i1, int *i2, char *verbose);

#endif /* !_SPRINTASC_LIB_H_ */
/*
 * sreadasc.lib.h
 */

#ifndef _SREADASC_LIB_H_
#define _SREADASC_LIB_H_

/* src/signal/sreadasc.lib.c */
Fsignal sreadasc(Fsignal out, int *n, float *s, float *t, float *g);

#endif /* !_SREADASC_LIB_H_ */
/*
 * sshrink2.lib.h
 */

#ifndef _SSHRINK2_LIB_H_
#define _SSHRINK2_LIB_H_

/* src/signal/sshrink2.lib.c */
Fsignal sshrink2(Fsignal in, Fsignal out);

#endif /* !_SSHRINK2_LIB_H_ */
/*
 * ssinus.lib.h
 */

#ifndef _SSINUS_LIB_H_
#define _SSINUS_LIB_H_

/* src/signal/ssinus.lib.c */
void ssinus(int *size, float *A, float *D, Fsignal signal);

#endif /* !_SSINUS_LIB_H_ */
/*
 * stvrestore.lib.h
 */

#ifndef _STVRESTORE_LIB_H_
#define _STVRESTORE_LIB_H_

/* src/signal/stvrestore.lib.c */
Fsignal stvrestore(char *relax, int *Niter, double *alpha, Fsignal O, Fimage I, Fsignal V, Fsignal M, Fsignal u);

#endif /* !_STVRESTORE_LIB_H_ */
/*
 * w1threshold.lib.h
 */

#ifndef _W1THRESHOLD_LIB_H_
#define _W1THRESHOLD_LIB_H_

/* src/signal/w1threshold.lib.c */
void w1threshold(float *P, float *T, float *D, char *s, char *o, Fsignal M, Wtrans1d in, Wtrans1d *out);

#endif /* !_W1THRESHOLD_LIB_H_ */
/*
 * biowave1.lib.h
 */

#ifndef _BIOWAVE1_LIB_H_
#define _BIOWAVE1_LIB_H_

/* src/wave/biowave1.lib.c */
void biowave1(int *NumRec, int *Edge, int *FilterNorm, Fsignal Signal, Wtrans1d Output, Fsignal Ri1, Fsignal Ri2);

#endif /* !_BIOWAVE1_LIB_H_ */
/*
 * biowave2.lib.h
 */

#ifndef _BIOWAVE2_LIB_H_
#define _BIOWAVE2_LIB_H_

/* src/wave/biowave2.lib.c */
void biowave2(int *NumRec, int *Haar, int *Edge, int *FilterNorm, Fimage Image, Wtrans2d Output, Fsignal Ri1, Fsignal Ri2);

#endif /* !_BIOWAVE2_LIB_H_ */
/*
 * dybiowave2.lib.h
 */

#ifndef _DYBIOWAVE2_LIB_H_
#define _DYBIOWAVE2_LIB_H_

/* src/wave/dybiowave2.lib.c */
void dybiowave2(int *NumRec, int *StopDecim, int *Ortho, int *Edge, int *FilterNorm, Fimage Image, Wtrans2d Output, Fsignal Ri1, Fsignal Ri2);

#endif /* !_DYBIOWAVE2_LIB_H_ */
/*
 * dyowave2.lib.h
 */

#ifndef _DYOWAVE2_LIB_H_
#define _DYOWAVE2_LIB_H_

/* src/wave/dyowave2.lib.c */
void dyowave2(int *NumRec, int *StopDecim, int *Ortho, int *Edge, int *Precond, int *FilterNorm, Fimage Image, Wtrans2d Output, Fsignal Ri, Fimage Edge_Ri);

#endif /* !_DYOWAVE2_LIB_H_ */
/*
 * ibiowave1.lib.h
 */

#ifndef _IBIOWAVE1_LIB_H_
#define _IBIOWAVE1_LIB_H_

/* src/wave/ibiowave1.lib.c */
void ibiowave1(int *NumRec, int *Edge, int *FilterNorm, Wtrans1d Wtrans, Fsignal Output, Fsignal Ri1, Fsignal Ri2);

#endif /* !_IBIOWAVE1_LIB_H_ */
/*
 * ibiowave2.lib.h
 */

#ifndef _IBIOWAVE2_LIB_H_
#define _IBIOWAVE2_LIB_H_

/* src/wave/ibiowave2.lib.c */
void ibiowave2(int *NumRec, int *Haar, int *Edge, int *FilterNorm, Wtrans2d Wtrans, Fimage Output, Fsignal Ri1, Fsignal Ri2);

#endif /* !_IBIOWAVE2_LIB_H_ */
/*
 * iowave1.lib.h
 */

#ifndef _IOWAVE1_LIB_H_
#define _IOWAVE1_LIB_H_

/* src/wave/iowave1.lib.c */
void iowave1(int *NumRec, int *Haar, int *Edge, int *Precond, int *Inverse, int *FilterNorm, Wtrans1d Wtrans, Fsignal Output, Fsignal Ri, Fimage Edge_Ri);

#endif /* !_IOWAVE1_LIB_H_ */
/*
 * iowave2.lib.h
 */

#ifndef _IOWAVE2_LIB_H_
#define _IOWAVE2_LIB_H_

/* src/wave/iowave2.lib.c */
void iowave2(int *NumRec, int *Haar, int *Edge, int *Precond, int *Inverse, int *FilterNorm, Wtrans2d Wtrans, Fimage Output, Fsignal Ri, Fimage Edge_Ri);

#endif /* !_IOWAVE2_LIB_H_ */
/*
 * owave1.lib.h
 */

#ifndef _OWAVE1_LIB_H_
#define _OWAVE1_LIB_H_

/* src/wave/owave1.lib.c */
void owave1(int *NumRec, int *Haar, int *Edge, int *Precond, int *Inverse, int *FilterNorm, Fsignal Signal, Wtrans1d Output, Fsignal Ri, Fimage Edge_Ri);

#endif /* !_OWAVE1_LIB_H_ */
/*
 * owave2.lib.h
 */

#ifndef _OWAVE2_LIB_H_
#define _OWAVE2_LIB_H_

/* src/wave/owave2.lib.c */
void owave2(int *NumRec, int *Haar, int *Edge, int *Precond, int *Inverse, int *FilterNorm, Fimage Image, Wtrans2d Output, Fsignal Ri, Fimage Edge_Ri);

#endif /* !_OWAVE2_LIB_H_ */
/*
 * owtrans_fimage.lib.h
 */

#ifndef _OWTRANS_FIMAGE_LIB_H_
#define _OWTRANS_FIMAGE_LIB_H_

/* src/wave/owtrans_fimage.lib.c */
void owtrans_fimage(Wtrans2d Wtrans, Fimage Output, int *NumRec, double *Contrast);

#endif /* !_OWTRANS_FIMAGE_LIB_H_ */
/*
 * wp2dchangepack.lib.h
 */

#ifndef _WP2DCHANGEPACK_LIB_H_
#define _WP2DCHANGEPACK_LIB_H_

/* src/wave/packets/wp2dchangepack.lib.c */
void wp2dchangepack(Wpack2d pack_in, Wpack2d pack_out, Cimage tree_out);

#endif /* !_WP2DCHANGEPACK_LIB_H_ */
/*
 * wp2dchangetree.lib.h
 */

#ifndef _WP2DCHANGETREE_LIB_H_
#define _WP2DCHANGETREE_LIB_H_

/* src/wave/packets/wp2dchangetree.lib.c */
void wp2dchangetree(Cimage treeIn, Cimage treeOut, char *up_tree, char *down_tree, int *new_tree_size, char *prune_tree, Cimage tree_for_max, Cimage tree_for_min);

#endif /* !_WP2DCHANGETREE_LIB_H_ */
/*
 * wp2dchecktree.lib.h
 */

#ifndef _WP2DCHECKTREE_LIB_H_
#define _WP2DCHECKTREE_LIB_H_

/* src/wave/packets/wp2dchecktree.lib.c */
int wp2dchecktree(Cimage tree, char *display_on_flag);

#endif /* !_WP2DCHECKTREE_LIB_H_ */
/*
 * wp2ddecomp.lib.h
 */

#ifndef _WP2DDECOMP_LIB_H_
#define _WP2DDECOMP_LIB_H_

/* src/wave/packets/wp2ddecomp.lib.c */
void wp2ddecomp(Fimage A, Fsignal Ri, Fsignal Ri_biortho, Wpack2d pack, Cimage tree);

#endif /* !_WP2DDECOMP_LIB_H_ */
/*
 * wp2deigenval.lib.h
 */

#ifndef _WP2DEIGENVAL_LIB_H_
#define _WP2DEIGENVAL_LIB_H_

/* src/wave/packets/wp2deigenval.lib.c */
void wp2deigenval(Fimage filter, Cimage tree, Fimage pfilter, float *limit);

#endif /* !_WP2DEIGENVAL_LIB_H_ */
/*
 * wp2dfreqorder.lib.h
 */

#ifndef _WP2DFREQORDER_LIB_H_
#define _WP2DFREQORDER_LIB_H_

/* src/wave/packets/wp2dfreqorder.lib.c */
void wp2dfreqorder(int level, Fsignal order, char *inverse_flag);

#endif /* !_WP2DFREQORDER_LIB_H_ */
/*
 * wp2dmktree.lib.h
 */

#ifndef _WP2DMKTREE_LIB_H_
#define _WP2DMKTREE_LIB_H_

/* src/wave/packets/wp2dmktree.lib.c */
void wp2dmktree(int level, Cimage tree, char *sinc_flag, char *quinc_flag, char *wave_flag, char *full_flag, char *mirror_flag);

#endif /* !_WP2DMKTREE_LIB_H_ */
/*
 * wp2doperate.lib.h
 */

#ifndef _WP2DOPERATE_LIB_H_
#define _WP2DOPERATE_LIB_H_

/* src/wave/packets/wp2doperate.lib.c */
void wp2doperate(Fimage input, Fsignal Ri, Fsignal Ri_biortho, Cimage tree, Fimage output, float *threshold_hard, float *threshold_soft, int *translation, float *track_noise_hard, float *track_noise_soft, int *track_noise_level, Fimage pfilter, int *convolution_level);

#endif /* !_WP2DOPERATE_LIB_H_ */
/*
 * wp2drecomp.lib.h
 */

#ifndef _WP2DRECOMP_LIB_H_
#define _WP2DRECOMP_LIB_H_

/* src/wave/packets/wp2drecomp.lib.c */
void wp2drecomp(Wpack2d pack, Fimage F);

#endif /* !_WP2DRECOMP_LIB_H_ */
/*
 * wp2dview.lib.h
 */

#ifndef _WP2DVIEW_LIB_H_
#define _WP2DVIEW_LIB_H_

/* src/wave/packets/wp2dview.lib.c */
void wp2dview(Fimage A, Fsignal Ri, Fsignal Ri_biortho, Cimage tree, char *do_not_reorder_flag, char *do_not_rescale_flag, Fimage toDisplay, Wpack2d input_pack);

#endif /* !_WP2DVIEW_LIB_H_ */
/*
 * wpsconvolve.lib.h
 */

#ifndef _WPSCONVOLVE_LIB_H_
#define _WPSCONVOLVE_LIB_H_

/* src/wave/packets/wpsconvolve.lib.c */
void wpsconvolve(Fsignal Signal, Fsignal Output, Fsignal Ri, char *upSample, char *band, char *oddSize);

#endif /* !_WPSCONVOLVE_LIB_H_ */
/*
 * precond1d.lib.h
 */

#ifndef _PRECOND1D_LIB_H_
#define _PRECOND1D_LIB_H_

/* src/wave/precond1d.lib.c */
void precond1d(int *Inverse, Fsignal Signal, Fsignal Output, Fimage Edge_Ri);

#endif /* !_PRECOND1D_LIB_H_ */
/*
 * precond2d.lib.h
 */

#ifndef _PRECOND2D_LIB_H_
#define _PRECOND2D_LIB_H_

/* src/wave/precond2d.lib.c */
void precond2d(int *Inverse, Fimage Image, Fimage Output, Fimage Edge_Ri);

#endif /* !_PRECOND2D_LIB_H_ */
/*
 * iridgelet.lib.h
 */

#ifndef _IRIDGELET_LIB_H_
#define _IRIDGELET_LIB_H_

/* src/wave/ridgelet/iridgelet.lib.c */
void iridgelet(Fimage in_re, Fimage in_im, int np, Fimage out_re, Fimage out_im);

#endif /* !_IRIDGELET_LIB_H_ */
/*
 * istkwave1.lib.h
 */

#ifndef _ISTKWAVE1_LIB_H_
#define _ISTKWAVE1_LIB_H_

/* src/wave/ridgelet/istkwave1.lib.c */
void istkwave1(int np, Fsignal in, Fsignal out);

#endif /* !_ISTKWAVE1_LIB_H_ */
/*
 * ridgelet.lib.h
 */

#ifndef _RIDGELET_LIB_H_
#define _RIDGELET_LIB_H_

/* src/wave/ridgelet/ridgelet.lib.c */
void ridgelet(Fimage in_re, Fimage in_im, int np, Fimage out_re, Fimage out_im);

#endif /* !_RIDGELET_LIB_H_ */
/*
 * ridgpolrec.lib.h
 */

#ifndef _RIDGPOLREC_LIB_H_
#define _RIDGPOLREC_LIB_H_

/* src/wave/ridgelet/ridgpolrec.lib.c */
void ridgpolrec(Fimage out_re, Fimage out_im, Fimage in_re, Fimage in_im);

#endif /* !_RIDGPOLREC_LIB_H_ */
/*
 * ridgrecpol.lib.h
 */

#ifndef _RIDGRECPOL_LIB_H_
#define _RIDGRECPOL_LIB_H_

/* src/wave/ridgelet/ridgrecpol.lib.c */
void ridgrecpol(Fimage in_re, Fimage in_im, Fimage out_re, Fimage out_im);

#endif /* !_RIDGRECPOL_LIB_H_ */
/*
 * ridgthres.lib.h
 */

#ifndef _RIDGTHRES_LIB_H_
#define _RIDGTHRES_LIB_H_

/* src/wave/ridgelet/ridgthres.lib.c */
void ridgthres(Fimage in_re, Fimage in_im, int np, Fimage out_re, Fimage out_im, int pourcent);

#endif /* !_RIDGTHRES_LIB_H_ */
/*
 * stkwave1.lib.h
 */

#ifndef _STKWAVE1_LIB_H_
#define _STKWAVE1_LIB_H_

/* src/wave/ridgelet/stkwave1.lib.c */
void stkwave1(int np, Fsignal in, Fsignal out);

#endif /* !_STKWAVE1_LIB_H_ */
/*
 * sconvolve.lib.h
 */

#ifndef _SCONVOLVE_LIB_H_
#define _SCONVOLVE_LIB_H_

/* src/wave/sconvolve.lib.c */
void sconvolve(Fsignal Signal, Fsignal Output, int *DownRate, int *UpRate, int *ReflIR, int *Band, int *Edge, int *Prolong, Fsignal Ri, Fimage Edge_Ri);

#endif /* !_SCONVOLVE_LIB_H_ */

#endif /* !_MW_MODULES_ */

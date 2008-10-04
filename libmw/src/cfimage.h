/*
 * cfimage.h
 */

#ifndef _CFIMAGE_H_
#define _CFIMAGE_H_

/* src/cfimage.c */
Cfimage mw_new_cfimage(void);
Cfimage mw_alloc_cfimage(Cfimage image, int nrow, int ncol);
void mw_delete_cfimage(Cfimage image);
Cfimage mw_change_cfimage(Cfimage image, int nrow, int ncol);
void mw_getdot_cfimage(Cfimage image, int x, int y, float *r, float *g, float *b);
void mw_plot_cfimage(Cfimage image, int x, int y, float r, float g, float b);
void mw_draw_cfimage(Cfimage image, int a0, int b0, int a1, int b1, float r, float g, float b);
void mw_clear_cfimage(Cfimage image, float r, float g, float b);
void mw_copy_cfimage(Cfimage in, Cfimage out);
float **mw_newtab_red_cfimage(Cfimage image);
float **mw_newtab_green_cfimage(Cfimage image);
float **mw_newtab_blue_cfimage(Cfimage image);

#endif /* !_CFIMAGE_H_ */

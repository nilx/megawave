/*
 * cimage.h
 */

#ifndef _CIMAGE_H_
#define _CIMAGE_H_

/* src/cimage.c */
Cimage mw_new_cimage(void);
Cimage mw_alloc_cimage(Cimage image, int nrow, int ncol);
void mw_delete_cimage(Cimage image);
Cimage mw_change_cimage(Cimage image, int nrow, int ncol);
unsigned char mw_getdot_cimage(Cimage image, int x, int y);
void mw_plot_cimage(Cimage image, int x, int y, unsigned char v);
void mw_draw_cimage(Cimage image, int a0, int b0, int a1, int b1, unsigned char c);
void mw_clear_cimage(Cimage image, unsigned char v);
void mw_copy_cimage(Cimage in, Cimage out);
unsigned char **mw_newtab_gray_cimage(Cimage image);
unsigned char mw_isitbinary_cimage(Cimage image);

#endif /* !_CIMAGE_H_ */

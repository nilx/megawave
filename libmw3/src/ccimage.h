/*
 * ccimage.h
 */

#ifndef _CCIMAGE_H_
#define _CCIMAGE_H_

/* src/ccimage.c */
Ccimage mw_new_ccimage(void);
Ccimage mw_alloc_ccimage(Ccimage image, int nrow, int ncol);
void mw_delete_ccimage(Ccimage image);
Ccimage mw_change_ccimage(Ccimage image, int nrow, int ncol);
void mw_getdot_ccimage(Ccimage image, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b);
void mw_plot_ccimage(Ccimage image, int x, int y, unsigned char r, unsigned char g, unsigned char b);
void mw_draw_ccimage(Ccimage image, int a0, int b0, int a1, int b1, unsigned char r, unsigned char g, unsigned char b);
void mw_clear_ccimage(Ccimage image, unsigned char r, unsigned char g, unsigned char b);
void mw_copy_ccimage(Ccimage in, Ccimage out);
unsigned char **mw_newtab_red_ccimage(Ccimage image);
unsigned char **mw_newtab_green_ccimage(Ccimage image);
unsigned char **mw_newtab_blue_ccimage(Ccimage image);

#endif /* !_CCIMAGE_H_ */

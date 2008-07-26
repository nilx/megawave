/*
 * cimage.h
 */

#ifndef _CIMAGE_H
#define _CIMAGE_H

Cimage mw_new_cimage(void);
Cimage mw_alloc_cimage(Cimage, int, int);
void mw_delete_cimage(Cimage);
Cimage mw_change_cimage(Cimage, int, int);
unsigned char mw_getdot_cimage(Cimage, int, int);
void mw_plot_cimage(Cimage, int, int, unsigned char);
void mw_draw_cimage(Cimage, int, int, int, int, unsigned char);
void mw_clear_cimage(Cimage, unsigned char);
void mw_copy_cimage(Cimage, Cimage);
unsigned char ** mw_newtab_gray_cimage(Cimage);
unsigned char mw_isitbinary_cimage(Cimage);

#endif /* !_CIMAGE_H */

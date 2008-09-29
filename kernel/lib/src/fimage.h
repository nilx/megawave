/*
 * fimage.h
 */

#ifndef _FIMAGE_H
#define _FIMAGE_H

Fimage mw_new_fimage(void);
Fimage mw_alloc_fimage(Fimage, int, int);
void mw_delete_fimage(Fimage);
Fimage mw_change_fimage(Fimage, int, int);
float mw_getdot_fimage(Fimage, int, int);
void mw_plot_fimage(Fimage, int, int, float);
void mw_draw_fimage(Fimage, int, int, int, int, float);
void mw_clear_fimage(Fimage, float);
void mw_copy_fimage(Fimage, Fimage);
float ** mw_newtab_gray_fimage(Fimage);

#endif /* !_FIMAGE_H */

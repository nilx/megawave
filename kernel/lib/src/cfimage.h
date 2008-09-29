/*
 * cfimage.h
 */

#ifndef _CFIMAGE_H
#define _CFIMAGE_H

Cfimage mw_new_cfimage(void);
Cfimage mw_alloc_cfimage(Cfimage, int, int);
void mw_delete_cfimage(Cfimage);
Cfimage mw_change_cfimage(Cfimage, int, int);
void mw_getdot_cfimage(Cfimage, int, int, float *, float *, float *);
void mw_plot_cfimage(Cfimage, int, int, float, float, float);
void mw_draw_cfimage(Cfimage, int, int, int, int, float, float, float);
void mw_clear_cfimage(Cfimage, float, float, float);
void mw_copy_cfimage(Cfimage, Cfimage);
float **mw_newtab_red_cfimage(Cfimage);
float **mw_newtab_green_cfimage(Cfimage);
float **mw_newtab_blue_cfimage(Cfimage);

#endif /* !_CFIMAGE_H */

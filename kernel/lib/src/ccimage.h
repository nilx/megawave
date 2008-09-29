/*
 * ccimage.h
 */

#ifndef _CCIMAGE_H
#define _CCIMAGE_H

Ccimage mw_new_ccimage(void);
Ccimage mw_alloc_ccimage(Ccimage, int, int);
void mw_delete_ccimage(Ccimage);
Ccimage mw_change_ccimage(Ccimage, int, int);
void mw_getdot_ccimage(Ccimage, int, int, unsigned char *,unsigned char *,
		      unsigned char *);
void mw_plot_ccimage(Ccimage, int, int, unsigned char, unsigned char, 
		    unsigned char);
void mw_draw_ccimage(Ccimage, int, int, int, int, unsigned char, unsigned char,
		    unsigned char);
void mw_clear_ccimage(Ccimage, unsigned char, unsigned char, unsigned char);
void mw_copy_ccimage(Ccimage, Ccimage);
unsigned char ** mw_gettab_red_ccimage(Ccimage);
unsigned char ** mw_gettab_green_ccimage(Ccimage);
unsigned char ** mw_gettab_blue_ccimage(Ccimage);
unsigned char ** mw_newtab_red_ccimage(Ccimage);
unsigned char ** mw_newtab_green_ccimage(Ccimage);
unsigned char ** mw_newtab_blue_ccimage(Ccimage);

#endif /* !_CCIMAGE_H */

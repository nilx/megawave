/*
 * wtrans2d.h
 */

#ifndef _WTRANS2D_H
#define _WTRANS2D_H

Wtrans2d mw_new_wtrans2d(void);
void *_mw_alloc_wtrans2d_norient(Wtrans2d, int, int, int, int, int);
void *mw_alloc_ortho_wtrans2d(Wtrans2d, int, int, int);
void *mw_alloc_biortho_wtrans2d(Wtrans2d, int, int, int);
void *mw_alloc_dyadic_wtrans2d(Wtrans2d, int, int, int);
void mw_delete_wtrans2d(Wtrans2d);

#endif /* !_WTRANS2D_H */

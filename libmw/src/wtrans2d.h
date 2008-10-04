/*
 * wtrans2d.h
 */

#ifndef _WTRANS2D_H_
#define _WTRANS2D_H_

/* src/wtrans2d.c */
Wtrans2d mw_new_wtrans2d(void);
void *_mw_alloc_wtrans2d_norient(Wtrans2d wtrans, int level, int nrow, int ncol, int Norient, int sampling);
void *mw_alloc_ortho_wtrans2d(Wtrans2d wtrans, int level, int nrow, int ncol);
void *mw_alloc_biortho_wtrans2d(Wtrans2d wtrans, int level, int nrow, int ncol);
void *mw_alloc_dyadic_wtrans2d(Wtrans2d wtrans, int level, int nrow, int ncol);
void mw_delete_wtrans2d(Wtrans2d wtrans);

#endif /* !_WTRANS2D_H_ */

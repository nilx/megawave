/*
 * wtrans1d.h
 */

#ifndef _WTRANS1D_H
#define _WTRANS1D_H

Wtrans1d mw_new_wtrans1d(void);
void *_mw_alloc_wtrans1d(Wtrans1d, int, int, int, int, int, int);
void *mw_alloc_ortho_wtrans1d(Wtrans1d, int, int);
void *mw_alloc_biortho_wtrans1d(Wtrans1d, int, int);
void *mw_alloc_dyadic_wtrans1d(Wtrans1d, int, int);
void *mw_alloc_continuous_wtrans1d(Wtrans1d, int, int, int, int);
void mw_delete_wtrans1d(Wtrans1d);

#endif /* !_WTRANS1D_H */

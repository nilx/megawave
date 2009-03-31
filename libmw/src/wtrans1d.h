/*
 * wtrans1d.h
 */

#ifndef _WTRANS1D_H_
#define _WTRANS1D_H_

/* src/wtrans1d.c */
Wtrans1d mw_new_wtrans1d(void);
void *_mw_alloc_wtrans1d(Wtrans1d wtrans, int level, int voice, int size,
                         int complex, int sampling, int use_average);
void *mw_alloc_ortho_wtrans1d(Wtrans1d wtrans, int level, int size);
void *mw_alloc_biortho_wtrans1d(Wtrans1d wtrans, int level, int size);
void *mw_alloc_dyadic_wtrans1d(Wtrans1d wtrans, int level, int size);
void *mw_alloc_continuous_wtrans1d(Wtrans1d wtrans, int level, int voice,
                                   int size, int complex);
void mw_delete_wtrans1d(Wtrans1d wtrans);

#endif                          /* !_WTRANS1D_H_ */

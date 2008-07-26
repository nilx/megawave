/*
 * wpack2d.h
 */

#ifndef _WPACK2D_H
#define _WPACK2D_H

int mw_bandsize_wpack2d(int, int);
Wpack2d mw_new_wpack2d(void);
int mw_checktree_wpack2d(Cimage);
Wpack2d mw_alloc_wpack2d(Wpack2d, Cimage, Fsignal, Fsignal, int, int);
void mw_delete_wpack2d(Wpack2d);
Wpack2d mw_change_wpack2d(Wpack2d, Cimage, Fsignal, Fsignal, int, int);
void mw_copy_wpack2d(Wpack2d, Wpack2d, int);
void mw_clear_wpack2d(Wpack2d, float);
void mw_prune_wpack2d(Wpack2d, Wpack2d, Cimage);

#endif /* !_WPACK2D_H */

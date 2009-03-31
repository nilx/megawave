/*
 * wpack2d.h
 */

#ifndef _WPACK2D_H_
#define _WPACK2D_H_

/* src/wpack2d.c */
int mw_bandsize_wpack2d(int initial_size, int current_level);
Wpack2d mw_new_wpack2d(void);
int mw_checktree_wpack2d(Cimage tree);
Wpack2d mw_alloc_wpack2d(Wpack2d pack, Cimage tree, Fsignal signal1, Fsignal signal2, int start_nrow, int start_ncol);
void mw_delete_wpack2d(Wpack2d pack);
Wpack2d mw_change_wpack2d(Wpack2d pack, Cimage tree, Fsignal signal1, Fsignal signal2, int start_nrow, int start_ncol);
void mw_copy_wpack2d(Wpack2d in, Wpack2d out, int new_tree_size);
void mw_clear_wpack2d(Wpack2d pack, float v);
void mw_prune_wpack2d(Wpack2d in, Wpack2d out, Cimage tree);

#endif /* !_WPACK2D_H_ */

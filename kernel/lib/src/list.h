/*
 * list.h
 */

#ifndef _LIST_H
#define _LIST_H

Flist mw_new_flist(void);
Flist mw_realloc_flist(Flist,int);
Flist mw_enlarge_flist(Flist);
Flist mw_change_flist(Flist,int,int,int);
void mw_delete_flist(Flist);
void mw_clear_flist(Flist,float);
Flist mw_copy_flist(Flist,Flist);

Flists mw_new_flists(void);
Flists mw_realloc_flists(Flists,int);
Flists mw_enlarge_flists(Flists);
Flists mw_change_flists(Flists,int,int);
void mw_delete_flists(Flists);
Flists mw_copy_flists(Flists,Flists);

Dlist mw_new_dlist(void);
Dlist mw_realloc_dlist(Dlist,int);
Dlist mw_enlarge_dlist(Dlist);
Dlist mw_change_dlist(Dlist,int,int,int);
void mw_delete_dlist(Dlist);
void mw_clear_dlist(Dlist,double);
Dlist mw_copy_dlist(Dlist,Dlist);

Dlists mw_new_dlists(void);
Dlists mw_realloc_dlists(Dlists,int);
Dlists mw_enlarge_dlists(Dlists);
Dlists mw_change_dlists(Dlists,int,int);
void mw_delete_dlists(Dlists);
Dlists mw_copy_dlists(Dlists,Dlists);

#endif /* !_LIST_H */

/*
 * list_io.h
 */

#ifndef _LIST_IO_H_
#define _LIST_IO_H_

/* src/list_io.c */
Flist _mw_read_mw2_flist(char *fname, FILE *fp, int need_flipping);
Flist _mw_load_mw2_flist(char *fname);
Flist _mw_flist_load_native(char *fname, char *type);
Flist _mw_load_flist(char *fname, char *type);
int _mw_write_mw2_flist(FILE *fp, Flist lst);
short _mw_create_mw2_flist(char *fname, Flist lst);
short _mw_flist_create_native(char *fname, Flist lst, char *type);
short _mw_create_flist(char *fname, Flist lst, char *type);
Flists _mw_load_mw2_flists(char *fname);
Flists _mw_flists_load_native(char *fname, char *type);
Flists _mw_load_flists(char *fname, char *type);
short _mw_create_mw2_flists(char *fname, Flists lsts);
short _mw_flists_create_native(char *fname, Flists lsts, char *type);
short _mw_create_flists(char *fname, Flists lsts, char *type);
Dlist _mw_load_mw2_dlist(char *fname);
Dlist _mw_dlist_load_native(char *fname, char *type);
Dlist _mw_load_dlist(char *fname, char *type);
int _mw_write_mw2_dlist(FILE *fp, Dlist lst);
short _mw_create_mw2_dlist(char *fname, Dlist lst);
short _mw_dlist_create_native(char *fname, Dlist lst, char *type);
short _mw_create_dlist(char *fname, Dlist lst, char *type);
Dlists _mw_load_mw2_dlists(char *fname);
Dlists _mw_dlists_load_native(char *fname, char *type);
Dlists _mw_load_dlists(char *fname, char *type);
short _mw_create_mw2_dlists(char *fname, Dlists lsts);
short _mw_dlists_create_native(char *fname, Dlists lsts, char *type);
short _mw_create_dlists(char *fname, Dlists lsts, char *type);

#endif /* !_LIST_IO_H_ */

/*
 * list_io.h
 */

#ifndef _LIST_IO_H
#define _LIST_IO_H

Flist _mw_read_mw2_flist(char *, FILE *, int);
Flist _mw_load_mw2_flist(char *);
Flist _mw_flist_load_native(char *, char *);
Flist _mw_load_flist(char *, char *);
int _mw_write_mw2_flist(FILE *, Flist);
short _mw_create_mw2_flist(char *, Flist);
short _mw_flist_create_native(char *, Flist, char *);
short _mw_create_flist(char *, Flist, char *);
Flists _mw_load_mw2_flists(char *);
Flists _mw_flists_load_native(char *, char *);
Flists _mw_load_flists(char *, char *);
short _mw_create_mw2_flists(char *, Flists);
short _mw_flists_create_native(char *, Flists, char *);
short _mw_create_flists(char *, Flists, char *);
Dlist _mw_load_mw2_dlist(char *);
Dlist _mw_dlist_load_native(char *, char *);
Dlist _mw_load_dlist(char *, char *);
int _mw_write_mw2_dlist(FILE *, Dlist);
short _mw_create_mw2_dlist(char *, Dlist);
short _mw_dlist_create_native(char *, Dlist, char *);
short _mw_create_dlist(char *, Dlist, char *);
Dlists _mw_load_mw2_dlists(char *);
Dlists _mw_dlists_load_native(char *, char *);
Dlists _mw_load_dlists(char *, char *);
short _mw_create_mw2_dlists(char *, Dlists);
short _mw_dlists_create_native(char *, Dlists, char *);
short _mw_create_dlists(char *, Dlists, char *);

#endif /* !_LIST_IO_H */

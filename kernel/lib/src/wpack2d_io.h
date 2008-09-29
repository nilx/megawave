/*
 * wpack2d_io.h
 */

#ifndef _WPACK2D_IO_H
#define _WPACK2D_IO_H

Wpack2d _mw_load_wpack2d_ascii(char *);
Wpack2d _mw_wpack2d_load_native(char *, char *);
Wpack2d _mw_load_wpack2d(char *, char *);
short _mw_create_wpack2d_ascii(char *, Wpack2d);
short _mw_wpack2d_create_native(char *, Wpack2d, char *);
short _mw_create_wpack2d(char *, Wpack2d, char *);

#endif /* !_WPACK2D_IO_H */

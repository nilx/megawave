/*
 * wpack2d_io.h
 */

#ifndef _WPACK2D_IO_H_
#define _WPACK2D_IO_H_

/* src/wpack2d_io.c */
Wpack2d _mw_load_wpack2d_ascii(char *fname);
Wpack2d _mw_wpack2d_load_native(char *fname, char *type);
Wpack2d _mw_load_wpack2d(char *fname, char *type);
short _mw_create_wpack2d_ascii(char *fname, Wpack2d pack);
short _mw_wpack2d_create_native(char *fname, Wpack2d pack, char *Type);
short _mw_create_wpack2d(char *fname, Wpack2d pack, char *Type);

#endif                          /* !_WPACK2D_IO_H_ */

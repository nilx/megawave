/*
 * wtrans2d_io.h
 */

#ifndef _WTRANS2D_IO_H_
#define _WTRANS2D_IO_H_

/* src/wtrans2d_io.c */
Wtrans2d _mw_load_wtrans2d_header(char *fname);
short _mw_create_wtrans2d_header(char *fname, Wtrans2d wtrans);
Wtrans2d _mw_wtrans2d_load_wtrans(char *fname, char *type);
short _mw_wtrans2d_create_wtrans(char *fname, Wtrans2d wtrans, char *type);

#endif                          /* !_WTRANS2D_IO_H_ */

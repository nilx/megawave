/*
 * wtrans2d_io.h
 */

#ifndef _WTRANS2D_IO_H
#define _WTRANS2D_IO_H

Wtrans2d _mw_load_wtrans2d_header(char *);
short _mw_create_wtrans2d_header(char *, Wtrans2d);
Wtrans2d _mw_wtrans2d_load_wtrans(char *, char *);
short _mw_wtrans2d_create_wtrans(char *, Wtrans2d, char *);

#endif /* !_WTRANS2D_IO_H */

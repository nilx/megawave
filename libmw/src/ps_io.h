/*
 * ps_io.h
 */

#ifndef _PS_IO_H_
#define _PS_IO_H_

/* src/ps_io.c */
Cimage _mw_cimage_load_ps(char *fname);
short _mw_cimage_create_ps(char *fname, Cimage image);

#endif /* !_PS_IO_H_ */

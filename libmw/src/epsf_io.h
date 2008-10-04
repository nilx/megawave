/*
 * epsf_io.h
 */

#ifndef _EPSF_IO_H_
#define _EPSF_IO_H_

/* src/epsf_io.c */
Cimage _mw_cimage_load_epsf(char *fname);
short _mw_cimage_create_epsf(char *fname, Cimage image);

#endif /* !_EPSF_IO_H_ */

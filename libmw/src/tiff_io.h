/*
 * tiff_io.h
 */

#ifndef _TIFF_IO_H_
#define _TIFF_IO_H_

/* src/tiff_io.c */
Ccimage _mw_ccimage_load_tiff(const char *fname);
Cimage _mw_cimage_load_tiff(const char *fname);
short _mw_ccimage_create_tiff(char *const fname, const Ccimage image);
short _mw_cimage_create_tiff(const char *fname, const Cimage image);

#endif                          /* !_TIFF_IO_H_ */

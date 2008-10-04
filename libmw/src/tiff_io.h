/*
 * tiff_io.h
 */

#ifndef _TIFF_IO_H_
#define _TIFF_IO_H_

/* src/tiff_io.c */
Ccimage _mw_ccimage_load_tiff(char *fname);
Cimage _mw_cimage_load_tiff(char *fname);
short _mw_ccimage_create_tiff(char *fname, Ccimage image);
short _mw_cimage_create_tiff(char *fname, Cimage image);

#endif /* !_TIFF_IO_H_ */

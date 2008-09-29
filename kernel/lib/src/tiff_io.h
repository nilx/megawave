/*
 * tiff_io.h
 */

#ifndef _TIFF_IO_H
#define _TIFF_IO_H

Cimage _mw_cimage_load_tiff(char *);
short _mw_cimage_create_tiff(char *, Cimage);
Ccimage _mw_ccimage_load_tiff(char *);
short _mw_ccimage_create_tiff(char *, Ccimage);

#endif /* !_TIFF_IO_H */

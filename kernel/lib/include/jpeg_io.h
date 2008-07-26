/*
 * jpeg_io.h
 */

#ifndef _JPEG_IO_H
#define _JPEG_IO_H

Cimage _mw_cimage_load_jpeg(char *);
Ccimage _mw_ccimage_load_jpeg(char *);
short _mw_cimage_create_jpeg(char *, Cimage, char *);
short _mw_ccimage_create_jpeg(char *, Ccimage, char *);

#endif /* !_JPEG_IO_H */

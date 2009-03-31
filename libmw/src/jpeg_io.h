/*
 * jpeg_io.h
 */

#ifndef _JPEG_IO_H_
#define _JPEG_IO_H_

/* src/jpeg_io.c */
Cimage _mw_cimage_load_jpeg(const char *fname);
Ccimage _mw_ccimage_load_jpeg(const char *fname);
short _mw_cimage_create_jpeg(const char *fname, const Cimage image,
                             const char *quality);
short _mw_ccimage_create_jpeg(const char *fname, const Ccimage image,
                              const char *quality);

#endif                          /* !_JPEG_IO_H_ */

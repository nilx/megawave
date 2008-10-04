/*
 * jpeg_io.h
 */

#ifndef _JPEG_IO_H_
#define _JPEG_IO_H_

/* src/jpeg_io.c */
Cimage _mw_cimage_load_jpeg(char *fname);
Ccimage _mw_ccimage_load_jpeg(char *fname);
short _mw_cimage_create_jpeg(char *fname, Cimage image, char *Quality);
short _mw_ccimage_create_jpeg(char *fname, Ccimage image, char *Quality);

#endif /* !_JPEG_IO_H_ */

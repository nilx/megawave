/*
 * bmp_io.h
 */

#ifndef _BMP_IO_H_
#define _BMP_IO_H_

/* src/bmp_io.c */
FILE *_mw_read_bmp_header(char *fname, unsigned int *nx, unsigned int *ny, unsigned int *offset, unsigned int *size, unsigned int *planes, unsigned int *bitcount, unsigned int *compression);
Cimage _mw_cimage_load_bmp(char *file);
short _mw_cimage_create_bmp(char *file, Cimage image);
Ccimage _mw_ccimage_load_bmp(char *file);
short _mw_ccimage_create_bmp(char *file, Ccimage image);

#endif /* !_BMP_IO_H_ */

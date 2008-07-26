/*
 * bmp_io.h
 */

#ifndef _BMP_IO_H
#define _BMP_IO_H

FILE * _mw_read_bmp_header(char *, 
			   unsigned int *, unsigned int *,
			   unsigned int *, unsigned int *,
			   unsigned int *,
			   unsigned int *,
			   unsigned int *);
Cimage _mw_cimage_load_bmp(char *);
short _mw_cimage_create_bmp(char *, Cimage);
Ccimage _mw_ccimage_load_bmp(char *);
short _mw_ccimage_create_bmp(char *, Ccimage);

#endif /* !_BMP_IO_H */



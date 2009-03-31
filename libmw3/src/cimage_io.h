/*
 * cimage_io.h
 */

#ifndef _CIMAGE_IO_H_
#define _CIMAGE_IO_H_

/* src/cimage_io.c */
Cimage _mw_cimage_load_megawave1(char *NomFic, char *Type);
short _mw_cimage_create_megawave1(char *NomFic, Cimage image, char *Type);
Cimage _mw_cimage_load_native(char *NomFic, char *Type);
short _mw_cimage_create_native(char *NomFic, Cimage image, char *Type);
Cimage _mw_cimage_load_image(char *NomFic, char *Type);
short _mw_cimage_create_image(char *NomFic, Cimage image, char *Type);

#endif /* !_CIMAGE_IO_H_ */

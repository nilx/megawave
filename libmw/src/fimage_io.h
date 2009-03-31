/*
 * fimage_io.h
 */

#ifndef _FIMAGE_IO_H_
#define _FIMAGE_IO_H_

/* src/fimage_io.c */
Fimage _mw_fimage_load_megawave1(char *NomFic, char *Type);
short _mw_fimage_create_megawave1(char *NomFic, Fimage image, char *Type);
Fimage _mw_fimage_load_native(char *NomFic, char *Type);
short _mw_fimage_create_native(char *NomFic, Fimage image, char *Type);
Fimage _mw_fimage_load_image(char *NomFic, char *Type);
short _mw_fimage_create_image(char *NomFic, Fimage image, char *Type);

#endif                          /* !_FIMAGE_IO_H_ */

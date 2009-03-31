/*
 * ccimage_io.h
 */

#ifndef _CCIMAGE_IO_H_
#define _CCIMAGE_IO_H_

/* src/ccimage_io.c */
Ccimage _mw_ccimage_load_native(char *NomFic, char *Type);
short _mw_ccimage_create_native(char *NomFic, Ccimage image, char *Type);
Ccimage _mw_ccimage_load_image(char *NomFic, char *Type);
short _mw_ccimage_create_image(char *NomFic, Ccimage image, char *Type);

#endif                          /* !_CCIMAGE_IO_H_ */

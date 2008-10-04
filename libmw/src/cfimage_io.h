/*
 * cfimage_io.h
 */

#ifndef _CFIMAGE_IO_H_
#define _CFIMAGE_IO_H_

/* src/cfimage_io.c */
Cfimage _mw_cfimage_load_native(char *NomFic, char *Type);
short _mw_cfimage_create_native(char *NomFic, Cfimage image, char *Type);
Cfimage _mw_cfimage_load_image(char *NomFic, char *Type);
short _mw_cfimage_create_image(char *NomFic, Cfimage image, char *Type);

#endif /* !_CFIMAGE_IO_H_ */

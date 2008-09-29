/*
 * cfimage_io.h
 */

#ifndef _CFIMAGE_IO_H
#define _CFIMAGE_IO_H

Cfimage _mw_cfimage_load_native(char *, char *);
short _mw_cfimage_create_native(char *, Cfimage, char *);
Cfimage _mw_cfimage_load_image(char *, char *);
short _mw_cfimage_create_image(char *, Cfimage, char *);

#endif /* !_CFIMAGE_IO_H */

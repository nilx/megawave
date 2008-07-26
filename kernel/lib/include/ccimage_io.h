/*
 * ccimage_io.h
 */

#ifndef _CCIMAGE_IO_H
#define _CCIMAGE_IO_H

Ccimage _mw_ccimage_load_native(char *, char *);
short _mw_ccimage_create_native(char *, Ccimage, char *);
Ccimage _mw_ccimage_load_image(char *, char *);
short _mw_ccimage_create_image(char *, Ccimage, char *);

#endif /* !_CCIMAGE_IO_H */

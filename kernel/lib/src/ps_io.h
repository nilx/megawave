/*
 * ps_io.h
 */

#ifndef _PS_IO_H
#define _PS_IO_H

Cimage _mw_cimage_load_ps(char *);
short _mw_cimage_create_ps(char *, Cimage);
Ccimage _mw_ccimage_load_ps(char *);
short _mw_ccimage_create_ps(char *, Ccimage);

#endif /* !_PS_IO_H */

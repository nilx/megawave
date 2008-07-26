/*
 * epsf_io.h
 */

#ifndef _EPSF_IO_H
#define _EPSF_IO_H

Cimage _mw_cimage_load_epsf(char *);
short _mw_cimage_create_epsf(char *, Cimage);
Ccimage _mw_ccimage_load_epsf(char *);
short _mw_ccimage_create_epsf(char *, Ccimage);

#endif /* !_EPSF_IO_H */

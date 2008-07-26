/*
 * cimage_io.h
 */

#ifndef _CIMAGE_IO_H
#define _CIMAGE_IO_H

long fsize(int);
Cimage _mw_cimage_load_megawave1(char *, char *);
short _mw_cimage_create_megawave1(char *, Cimage, char *);
Cimage _mw_cimage_load_native(char *, char *);
short _mw_cimage_create_native(char *, Cimage, char *);
Cimage _mw_cimage_load_image(char *, char *);
short _mw_cimage_create_image(char *, Cimage, char *);

#endif /* !_CIMAGE_IO_H */

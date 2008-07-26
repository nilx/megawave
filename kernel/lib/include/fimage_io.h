/*
 * fimage_io.h
 */

#ifndef _FIMAGE_IO_H
#define _FIMAGE_IO_H

Fimage _mw_fimage_load_megawave1(char *, char *);
short _mw_fimage_create_megawave1(char *, Fimage, char *);
Fimage _mw_fimage_load_native(char *, char *);
short _mw_fimage_create_native(char *, Fimage, char *);
Fimage _mw_fimage_load_image(char *, char *);
short _mw_fimage_create_image(char *, Fimage, char *);

#endif /* !_FIMAGE_IO_H */

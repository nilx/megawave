/*
 * ppm_io.h
 */

#ifndef _PPM_IO_H_
#define _PPM_IO_H_

/* src/ppm_io.c */
Ccimage _mw_ccimage_load_ppmr(char *file);
short _mw_ccimage_create_ppmr(char *file, Ccimage image);

#endif                          /* !_PPM_IO_H_ */

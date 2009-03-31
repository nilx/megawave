/*
 * pgm_io.h
 */

#ifndef _PGM_IO_H_
#define _PGM_IO_H_

/* src/pgm_io.c */
int _mw_pgm_get_next_item(FILE * fp, char *comment);
Cimage _mw_cimage_load_pgma(char *file);
short _mw_cimage_create_pgma(char *file, Cimage image);
Cimage _mw_cimage_load_pgmr(char *file);
short _mw_cimage_create_pgmr(char *file, Cimage image);

#endif                          /* !_PGM_IO_H_ */

/*
 * pgm_io.h
 */

#ifndef _PGM_IO_H
#define _PGM_IO_H

int _mw_pgm_get_next_item(FILE *, char *);
Cimage _mw_cimage_load_pgma(char *);
short _mw_cimage_create_pgma(char *, Cimage);
Cimage _mw_cimage_load_pgmr(char *);
short _mw_cimage_create_pgmr(char *, Cimage);

#endif /* !_PGM_IO_H */

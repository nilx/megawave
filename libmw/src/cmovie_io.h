/*
 * cmovie_io.h
 */

#ifndef _CMOVIE_IO_H_
#define _CMOVIE_IO_H_

/* src/cmovie_io.c */
Cmovie _mw_cmovie_load_movie_old_format(char *NomFic, char *Type);
Cmovie _mw_cmovie_load_native(char *fname, char *Type);
Cmovie _mw_cmovie_load_movie(char *NomFic, char *Type);
short _mw_cmovie_create_movie(char *NomFic, Cmovie movie, char *Type);

#endif                          /* !_CMOVIE_IO_H_ */

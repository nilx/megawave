/*
 * fmovie_io.h
 */

#ifndef _FMOVIE_IO_H_
#define _FMOVIE_IO_H_

/* src/fmovie_io.c */
Fmovie _mw_fmovie_load_movie_old_format(char *NomFic, char *Type);
Fmovie _mw_fmovie_load_movie(char *fname, char *Type);
short _mw_fmovie_create_movie(char *NomFic, Fmovie movie, char *Type);

#endif                          /* !_FMOVIE_IO_H_ */

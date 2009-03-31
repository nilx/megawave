/*
 * cfmovie_io.h
 */

#ifndef _CFMOVIE_IO_H_
#define _CFMOVIE_IO_H_

/* src/cfmovie_io.c */
Cfmovie _mw_cfmovie_load_movie_old_format(char *NomFic, char *Type);
Cfmovie _mw_cfmovie_load_native(char *fname, char *Type);
Cfmovie _mw_cfmovie_load_movie(char *NomFic, char *Type);
short _mw_cfmovie_create_movie(char *NomFic, Cfmovie movie, char *Type);

#endif                          /* !_CFMOVIE_IO_H_ */

/*
 * ccmovie_io.h
 */

#ifndef _CCMOVIE_IO_H_
#define _CCMOVIE_IO_H_

/* src/ccmovie_io.c */
Ccmovie _mw_ccmovie_load_movie_old_format(char *NomFic, char *Type);
Ccmovie _mw_ccmovie_load_native(char *fname, char *Type);
Ccmovie _mw_ccmovie_load_movie(char *NomFic, char *Type);
short _mw_ccmovie_create_movie(char *NomFic, Ccmovie movie, char *Type);

#endif                          /* !_CCMOVIE_IO_H_ */

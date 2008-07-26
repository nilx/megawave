/*
 * fmovie_io.h
 */

#ifndef _FMOVIE_IO_H
#define _FMOVIE_IO_H

Fmovie _mw_fmovie_load_movie_old_format(char *, char *);
Fmovie _mw_fmovie_load_movie(char *, char *);
short _mw_fmovie_create_movie(char *, Fmovie, char *);

#endif /* !_FMOVIE_IO_H */

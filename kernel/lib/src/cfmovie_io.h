/*
 * cfmovie_io.h
 */

#ifndef _CFMOVIE_IO_H
#define _CFMOVIE_IO_H

Cfmovie _mw_cfmovie_load_movie_old_format(char *, char *);
Cfmovie _mw_cfmovie_load_native(char *, char *);
Cfmovie _mw_cfmovie_load_movie(char *, char *);
short _mw_cfmovie_create_movie(char *, Cfmovie, char *);

#endif /* !_CFMOVIE_IO_H */

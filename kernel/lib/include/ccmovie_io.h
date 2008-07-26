/*
 * ccmovie_io.h
 */

#ifndef _CCMOVIE_IO_H
#define _CCMOVIE_IO_H

Ccmovie _mw_ccmovie_load_movie_old_format(char *, char *);
Ccmovie _mw_ccmovie_load_movie(char *, char *);
Ccmovie _mw_ccmovie_load_native(char *, char *);
short _mw_ccmovie_create_movie(char *, Ccmovie, char *);

#endif /* !_CCMOVIE_IO_H */

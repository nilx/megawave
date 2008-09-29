/*
 * cmovie_io.h
 */

#ifndef _CMOVIE_IO_H
#define _CMOVIE_IO_H

Cmovie _mw_cmovie_load_movie_old_format(char *, char *);
Cmovie _mw_cmovie_load_native(char *, char *);
Cmovie _mw_cmovie_load_movie(char *, char *);
short _mw_cmovie_create_movie(char *, Cmovie, char *);

#endif /* !_CMOVIE_IO_H */

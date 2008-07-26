/*
 * wtrans1d_io.h
 */

#ifndef _WTRANS1D_IO_H
#define _WTRANS1D_IO_H

Wtrans1d _mw_load_wtrans1d_header(char *);
short _mw_create_wtrans1d_header(char *, Wtrans1d);
void *_mw_wtrans1d_load_signal_wtrans(char *, char *, Wtrans1d, Fsignal (*)[50], char *);
Wtrans1d _mw_wtrans1d_load_wtrans(char *, char *);
void _mw_wtrans1d_create_signal_wtrans(char *, char *, Wtrans1d, Fsignal (*)[50], char *);
short _mw_wtrans1d_create_wtrans(char *, Wtrans1d, char *);

#endif /* !_WTRANS1D_IO_H */

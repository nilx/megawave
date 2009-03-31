/*
 * wtrans1d_io.h
 */

#ifndef _WTRANS1D_IO_H_
#define _WTRANS1D_IO_H_

/* src/wtrans1d_io.c */
Wtrans1d _mw_load_wtrans1d_header(char *fname);
short _mw_create_wtrans1d_header(char *fname, Wtrans1d wtrans);
void *_mw_wtrans1d_load_signal_wtrans(char *fname, char *type,
                                      Wtrans1d wtrans, Fsignal(*S)[50],
                                      char *Sname);
Wtrans1d _mw_wtrans1d_load_wtrans(char *fname, char *type);
void _mw_wtrans1d_create_signal_wtrans(char *fname, char *type,
                                       Wtrans1d wtrans, Fsignal(*S)[50],
                                       char *Sname);
short _mw_wtrans1d_create_wtrans(char *fname, Wtrans1d wtrans, char *type);

#endif                          /* !_WTRANS1D_IO_H_ */

/*
 * fsignal_io.h
 */

#ifndef _FSIGNAL_IO_H_
#define _FSIGNAL_IO_H_

/* src/fsignal_io.c */
Fsignal _mw_load_fsignal_ascii(char *fname, Fsignal signal);
short _mw_create_fsignal_ascii(char *fname, Fsignal signal);
Fsignal _mw_load_fsignal(char *fname, char *type, Fsignal signal);
short _mw_create_fsignal(char *fname, Fsignal signal, char *type);

#endif /* !_FSIGNAL_IO_H_ */

/*
 * fsignal_io.h
 */

#ifndef _FSIGNAL_IO_H
#define _FSIGNAL_IO_H

Fsignal _mw_load_fsignal_ascii(char *, Fsignal);
short _mw_create_fsignal_ascii(char *, Fsignal);
Fsignal _mw_load_fsignal(char *, char *, Fsignal);
short _mw_create_fsignal(char *, Fsignal, char *);

#endif /* !_FSIGNAL_IO_H */

/*
 * rawdata_io.h
 */

#ifndef _RAWDATA_IO_H_
#define _RAWDATA_IO_H_

/* src/rawdata_io.c */
Rawdata _mw_load_rawdata(char *fname);
short _mw_create_rawdata(char *fname, Rawdata rd);

#endif                          /* !_RAWDATA_IO_H_ */

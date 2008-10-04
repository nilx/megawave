/*
 * rawdata.h
 */

#ifndef _RAWDATA_H_
#define _RAWDATA_H_

/* src/rawdata.c */
Rawdata mw_new_rawdata(void);
Rawdata mw_alloc_rawdata(Rawdata rd, int newsize);
void mw_delete_rawdata(Rawdata rd);
Rawdata mw_change_rawdata(Rawdata rd, int newsize);
void mw_copy_rawdata(Rawdata in, Rawdata out);

#endif /* !_RAWDATA_H_ */

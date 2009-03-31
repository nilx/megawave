/*
 * cmovie.h
 */

#ifndef _CMOVIE_H_
#define _CMOVIE_H_

/* src/cmovie.c */
Cmovie mw_new_cmovie(void);
void mw_delete_cmovie(Cmovie movie);
Cmovie mw_change_cmovie(Cmovie movie);

#endif                          /* !_CMOVIE_H_ */

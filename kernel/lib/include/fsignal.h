/*
 * fsignal.h
 */

#ifndef _FSIGNAL_H
#define _FSIGNAL_H

Fsignal mw_new_fsignal(void);
Fsignal mw_alloc_fsignal(Fsignal, int);
void mw_delete_fsignal(Fsignal);
Fsignal mw_change_fsignal(Fsignal, int);
void mw_clear_fsignal(Fsignal, float);
void mw_copy_fsignal_values(Fsignal,Fsignal);
void mw_copy_fsignal_header(Fsignal,Fsignal);
void mw_copy_fsignal(Fsignal,Fsignal);

#endif /* !_FSIGNAL_H */

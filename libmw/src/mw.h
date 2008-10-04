/*
 * mw.h
 */

#ifndef _MW_H_
#define _MW_H_

/* src/mw.c */
void mwexit(int n);
void mw_exit(int n);
void *mwmalloc(size_t size);
void mwfree(void *ptr);
void *mwrealloc(void *ptr, size_t size);
void *mwcalloc(size_t nelem, size_t elsize);
void mwcfree(void *ptr);
int _mw_main(int argc, char *argv[], char *envp[]);
void MegaWaveDefOpt(char *vers);
int mw_opt_used(char c);
void mwdebug(char *fmt, ...);
void mwerror(int code, int exit_code, char *fmt, ...);
char *_mw_ctoa_(char c);
char *_mw_uctoa_(unsigned char uc);
char *_mw_stoa_(short s);
char *_mw_ustoa_(unsigned short us);
char *_mw_itoa_(int i);
char *_mw_uitoa_(unsigned int ui);
char *_mw_ltoa_(long l);
char *_mw_ultoa_(unsigned long ul);
char *_mw_ftoa_(float f);
char *_mw_dtoa_(double d);
int _mwis_open(char *s, char *rw);
int _mwgetopt(int argc, char **argv, char *optstring);

#endif /* !_MW_H_ */

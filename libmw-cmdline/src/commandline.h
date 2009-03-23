/*
 * commandline.h
 */

#ifndef _COMMANDLINE_H_
#define _COMMANDLINE_H_

/* src/commandline.c */
int _mw_main(int argc, char *argv[], char *envp[], Mwiline mwicmd[], int mwind);
void MegaWaveDefOpt(char *vers, Mwiline mwicmd[], int mwind);
int mw_opt_used(char c);
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

#endif /* !_COMMANDLINE_H_ */

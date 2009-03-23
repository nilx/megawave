/*
 * commandline.h
 */

#ifndef _COMMANDLINE_H_
#define _COMMANDLINE_H_

/* src/commandline.c */
int _mw_main(int argc, char *argv[], char *envp[], Mwiline mwicmd[], int mwind);
void mwdefopt(char *vers, Mwiline mwicmd[], int mwind);
int mw_opt_used(char c);
int _mwis_open(char *s, char *rw);
int _mwgetopt(int argc, char **argv, char *optstring);

#endif /* !_COMMANDLINE_H_ */

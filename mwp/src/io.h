/*
 * io.h
 */

#ifndef _IO_H_
#define _IO_H_

/* src/io.c */
void debug(char *fmt, ...);
void info(char *fmt, ...);
void warning(char *fmt, ...);
void error(char *fmt, ...);
int lowerstring(char *in);
char *getprintfstring(char *s);
void removespaces(char *in);
int getline(FILE * sfile, char *line);
int getsentence(FILE * sfile, char *s);
int getinstruction(FILE * sfile, char *s, long *lbeg, long *lend);
int removebraces(char *in, char *out);
void RemoveTerminatingSpace(char *in);
int getenclosedstring(char *in, char *out);
int getword(char *s, char *w);
int getCid(char *s, char *cid);
int getInterval(char *s, char *min, char *max, int *ai);
int IsStringCid(char *s);
void fprinttex(FILE * fd, char *fmt, ...);

#endif                          /* !_IO_H_ */

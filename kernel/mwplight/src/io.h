/*
 * io.h for megawave, section mwplight
 *
 * header for io.c
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2007)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

#ifndef _MWPL_IO_H
#define _MWPL_IO_H

void debug (char * fmt, ...);
void info (char * fmt, ...);
void warning (char * fmt, ...);
void /*@noreturn@*/ error (char * fmt, ...);
int lowerstring (char * in);
char * getprintfstring (char * s);
void removespaces (char * in);
int getline (FILE * sfile, char * line);
int getsentence (FILE * sfile, char * s);
int getinstruction (FILE * sfile, char * s, long int * lbeg, long int * lend);
int removebraces (char * in, char * out);
void RemoveTerminatingSpace (char * in);
int getenclosedstring (char * in, /*@out@*/ char * out);
int getword (char * s, char * w);
int getCid (char * s, char * cid);
int getInterval (char * s, \
		 /*@out@*/ char * min, /*@out@*/ char * max, \
		 /*@out@*/ int * ai);
int IsStringCid (char * s);
void WriteFuncPrototype (FILE * fd, t_varfunc * f, int ansi);
void fprinttex (FILE * fd, char * fmt, ...);

#endif /* !_MWPL_IO_H */

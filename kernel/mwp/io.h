/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#ifndef ERROR_INC
#define ERROR_INC 1

#ifdef __STDC__
void yyerror(char *);
#else
void yyerror();
#endif

#ifdef __STDC__
void yyerrmsg(char *, ...);
void prline(int);
void restoreprline(void);
void int_error(char *, int, char *);
void error(char *, ...);
void error2(char*, int, char *, ...);
void warning(char *, ...);
void warning2(char*, int, char *, ...);
void fatal_error(char *, ...);
void fprinttex(FILE *, char *, ...);
void mwcexit(int);
#else
void warning();
void prline();
void restoreprline();
void int_error();
void error();
void error2();
void warning();
void fatal_error();
void fprinttex();
void mwcexit();
#endif

#ifdef DEBUG
#ifdef __STDC__
void PRDBG(char *, ...);
#else
void PRDBG();
#endif
#endif

#define INT_ERROR(F) int_error(__FILE__, __LINE__, (F))
#define ERROR(M)     error(M)

#define ENABLE  1
#define DISABLE 0

#ifdef ERROR_DEC
int errorcnt = 0;
#else
extern int errorcnt;
#endif

#endif

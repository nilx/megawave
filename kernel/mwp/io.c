/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define sparc 1
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#define ERROR_DEC
#include "io.h"
#include "bintree.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef DEBUG
char yyfilein[BUFSIZ];
#endif

static int prlineflg = ENABLE, oldprlineflg;

int nowarning = FALSE;
extern FILE * mwerr;


#ifdef __STDC__
void prline(int i)
#else
void prline(i)
int i;
#endif
{
  oldprlineflg = prlineflg;
  switch(i) {
    case ENABLE :
      prlineflg = ENABLE;
      break;
    default :
      prlineflg = DISABLE;
      break;
  }
}


#ifdef __STDC__
void restoreprline(void)
#else
void restoreprline()
#endif
{
  prlineflg = oldprlineflg;
}


#ifdef __STDC__
void yyerror(char *s)
#else
void yyerror(s)
char *s;
#endif
{
  extern char filein[];
  extern int lineno;
#ifdef DEBUG
  extern int yydebug;
#endif

#ifdef DEBUG
  if (yydebug)
    fprintf(mwerr, "%s : %s line %d\n", yyfilein, s, lineno);
  else
#endif
  fprintf(mwerr, "%s : %s line %d\n", filein, s, lineno);
  errorcnt++;
}


#ifdef __STDC__
void yyerrmsg(char *fmt, ...)
#else
void yyerrmsg(va_alist)
va_dcl
#endif
{
  if (nowarning == FALSE) {
    extern char filein[];
    extern int lineno;
#ifdef DEBUG
    extern int yydebug;
#endif
    char msg[BUFSIZ];
#ifndef __STDC__
    char *fmt;
#endif
    va_list marker;

#ifdef __STDC__
    va_start(marker, fmt);
#else
    va_start(marker);
    fmt = va_arg(marker, char *);
#endif
    vsprintf(msg, fmt, marker);
#ifdef DEBUG
    if (yydebug) {
      int i;
      for (i = strlen(filein) + 2; i>=0; i--)
        putc(' ', mwerr);
      fprintf(mwerr, "%s", msg);
    }
    else
#endif
    {
      int i;
      for (i = strlen(filein) + 2; i>=0; i--)
        putc(' ', mwerr);
      fprintf(mwerr, "%s", msg);
    }
    va_end(marker);
  }
}


#ifdef __STDC__
void int_error(char *file, int line, char *func)
#else
void int_error(file, line, func)
char *file;
int line;
char *func;
#endif
{
#ifdef SunOS 
  /* Necessaire en tout cas pour le cc SUNWspro du SunOS 5.6 */
  extern char *sys_errlist[]; 
#endif
#ifdef HPUX
  extern char *sys_errlist[]; 
#endif
  int i;
  fprintf(mwerr, "%s : INTERNAL ERROR line %d\n", file, line);
  for (i = strlen(file) + 2; i>=0; i--)
    putc(' ', mwerr);
  fprintf(mwerr, "function or macro %s.\n", func);
  fprintf(mwerr, "%s\n", sys_errlist[errno]);
  for (i = strlen(file) + 2; i>=0; i--)
    putc(' ', mwerr);
  fprintf(mwerr, "Exit.\n");
  mwcexit(1);
}


#ifdef __STDC__
void warning(char *fmt, ...)
#else
void warning(va_alist)
va_dcl
#endif
{
  if (nowarning == FALSE) {
    extern char filein[];
    extern int lineno;

    char msg[BUFSIZ];
#ifndef __STDC__
    char *fmt;
#endif
    va_list marker;
    int i;

#ifdef __STDC__
    va_start(marker, fmt);
#else
    va_start(marker);
    fmt = va_arg(marker, char *);
#endif
    vsprintf(msg, fmt, marker);
    va_end(marker);

    if (prlineflg == ENABLE) {
      fprintf(mwerr, "%s : warning line %d\n", filein, lineno);
      for (i = strlen(filein) + 2; i>=0; i--)
        putc(' ', mwerr);
    }
    else
       fprintf(mwerr, "warning : ");
    fprintf(mwerr, "%s", msg);
  }
}


#ifdef __STDC__
void warning2(char *filein, int lineno, char *fmt, ...)
#else
void warning2(va_alist)
va_dcl
#endif
{
  char msg[BUFSIZ];
#ifndef __STDC__
  char *filein;
  int lineno;
  char *fmt;
#endif
  va_list marker;

#ifdef __STDC__
  va_start(marker, fmt);
#else
  va_start(marker);
  filein = va_arg(marker, char*);
  lineno = va_arg(marker, int);
  fmt    = va_arg(marker, char *);
#endif
  vsprintf(msg, fmt, marker);
  fprintf(mwerr, "warning : ");
  if (prlineflg == ENABLE)
    fprintf(mwerr, "\"%s\", line %d : ", filein, lineno);
  fprintf(mwerr, "%s", msg);
  va_end(marker);
}



#ifdef __STDC__
void error(char *fmt, ...)
#else
void error(va_alist)
va_dcl
#endif
{
  extern char filein[];
  extern int lineno;

  int i;
  char msg[BUFSIZ];
#ifndef __STDC__
  char *fmt;
#endif
  va_list marker;

#ifdef __STDC__
  va_start(marker, fmt);
#else
  va_start(marker);
  fmt = va_arg(marker, char *);
#endif
  vsprintf(msg, fmt, marker);
  va_end(marker);

  if (prlineflg == ENABLE) {
    fprintf(mwerr, "%s : error line %d\n", filein, lineno);
    for (i = strlen(filein) + 2; i>=0; i--)
      putc(' ', mwerr);
  }
  else
    fprintf(mwerr, "error : ");

  fprintf(mwerr, "%s", msg);
  errorcnt++;
}


#ifdef __STDC__
void error2(char *filein, int lineno, char *fmt, ...)
#else
void error2(va_alist)
va_dcl
#endif
{
  char msg[BUFSIZ];
#ifndef __STDC__
  char *filein;
  int lineno;
  char *fmt;
#endif
  va_list marker;

#ifdef __STDC__
  va_start(marker, fmt);
#else
  va_start(marker);
  filein = va_arg(marker, char*);
  lineno = va_arg(marker, int);
  fmt    = va_arg(marker, char *);
#endif
  vsprintf(msg, fmt, marker);
  fprintf(mwerr, "error : ");
  if (prlineflg == ENABLE)
    fprintf(mwerr, "\"%s\", line %d : ", filein, lineno);
  fprintf(mwerr, "%s", msg);
  va_end(marker);
  errorcnt++;
}


#ifdef __STDC__
void fatal_error(char *fmt, ...)
#else
void fatal_error(va_alist)
va_dcl
#endif
{
  extern char filein[];
  extern int lineno;
  extern int headerflg;

  int i;
  char msg[BUFSIZ];
#ifndef __STDC__
  char *fmt;
#endif
  va_list marker;

#ifdef __STDC__
  va_start(marker, fmt);
#else
  va_start(marker);
  fmt = va_arg(marker, char *);
#endif
  vsprintf(msg, fmt, marker);
  va_end(marker);

  if (prlineflg == ENABLE) {
    fprintf(mwerr, "%s : fatal error line %d\n", filein, lineno);
    for (i = strlen(filein) + 2; i>=0; i--)
      putc(' ', mwerr);
  }
  else
    fprintf(mwerr, "fatal error : ");

  fprintf(mwerr, "%s", msg);
  mwcexit(headerflg ? 1 : 2);
}



#ifdef __STDC__
void fprinttex(FILE *fd, char *fmt, ...)
#else
void fprinttex(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
  FILE *fd;
  char *fmt;
#endif
  va_list marker;
  int longflg;
  int fmtflg;
  Node *n;
  longflg = fmtflg = FALSE;
#ifdef __STDC__
  va_start(marker, fmt);
#else
  va_start(marker);
  fd = va_arg(marker, FILE *);
  fmt = va_arg(marker, char *);
#endif
  while(*fmt != 0) {  
    if (*fmt != '%' && fmtflg == FALSE)
      fputc(*fmt++, fd);
    else {
      char c;
      int i;
      int t_val;
      char *s;
      fmtflg = TRUE;
      
      switch(*++fmt) {
        case '%':
          fprintf(fd, "\\%", fd);
          longflg = fmtflg = FALSE;
          break;
        case 'c':
          c = va_arg(marker, char);
          fprintf(fd, "%c", c);
          longflg = fmtflg = FALSE;
          break;
        case 's' :
          fprintf(fd, "%s", va_arg(marker, char *));
          longflg = fmtflg = FALSE;
          break;
        case 'T' :
          for (s = va_arg(marker, char *); *s != '\0'; s++) {
            int escflg = FALSE;
            switch(*s) {
              case '$' :
              case '%' :
              case '{' :
              case '}' :
              case '_' :
              case '&' :
              case '#' :
                if (escflg) {
                  fprintf(fd, "\\verb!\\!");
                  escflg = FALSE;
                }
                fprintf(fd, "\\%c", *s);
                break;
              case '\\' :
                if (escflg) {
                  fprintf(fd, "\\verb!\\!");
                  escflg = FALSE;
                }
                else
                  escflg = TRUE;
                break;
              case '^' :
		/* \verb!^! does not work inside a LaTeX 2e macro ! */
		if (escflg) {
		  fprintf(fd, "\\verb!\\!");
                  escflg = FALSE;
                }
		fprintf(fd, "$\\mathbf{\\hat{}}\\;$");
                break;
              case '~' :
              case '|' :
                if (escflg) {
                  fprintf(fd, "\\verb!\\!");
                  escflg = FALSE;
                }
                fprintf(fd, "\\verb!%c!", *s);
                break;
              case 'a' :
              case 'b' :
              case 'f' :
              case 'n' :
              case 'r' :
              case 't' :
              case 'v' :
              case '?' :
              case '\'' :
              case '"' :
                if (escflg) {
                  switch (*s) {
                    case 'a' :
                    case 'b' :
                    case 'f' :
                    case 't' :
                    case 'v' :
                      break;
                    case 'n' :
                    case 'r' :
                      fprintf(fd, "\\newline\n");
                      break;
                    case '?' :
                    case '\'' :
                    case '"' :
                      putc(*s, fd);
                      break;
                  }
                  escflg = FALSE;
                }
                else
                  putc(*s, fd);
                break;
              default :
                if (escflg) {
                  fprintf(fd, "\\verb!\\!");
                  escflg = FALSE;
                }
                putc(*s, fd);
                break;
            }
          }
          longflg = fmtflg = FALSE;
          break;
        case 'x' :
          if (longflg)
            fprintf(fd, "%lx", va_arg(marker, unsigned long));
          else
            fprintf(fd, "%x", va_arg(marker, unsigned int));
          longflg = fmtflg = FALSE;
          break;
        case 'X' :
          if (longflg)
            fprintf(fd, "%lX", va_arg(marker, unsigned long));
          else
            fprintf(fd, "%X", va_arg(marker, unsigned int));
          longflg = fmtflg = FALSE;
          break;
        case 'u' :
          if (longflg)
            fprintf(fd, "%lu", va_arg(marker, unsigned long));
          else
            fprintf(fd, "%u", va_arg(marker, unsigned int));
          longflg = fmtflg = FALSE;
          break;
        case 'd' :
          if (longflg)
            fprintf(fd, "%ld", va_arg(marker, long));
          else
            fprintf(fd, "%d", va_arg(marker, int));
          longflg = fmtflg = FALSE;
          break;
        case 'f' :
          if (longflg)
            fprintf(fd, "%g", va_arg(marker, double));
          else
            fprintf(fd, "%g", (double)va_arg(marker, float));
          longflg = fmtflg = FALSE;
          break;
        case 'l' :
          longflg = TRUE;
          break;
        default :
          fprintf(fd, "%%%c", *fmt);
          longflg = fmtflg = FALSE;
          break;
      }
      if(longflg == FALSE)
        fmt++;
    }
  }
  va_end(marker);
}

#ifdef DEBUG



#ifdef __STDC__
void PRDBG(char *fmt, ...)
#else
void PRDBG(va_alist)
va_dcl
#endif
{
  extern int yydebug;
  FILE *fd_dbg;

#ifdef FLEX
  fd_dbg = stderr;
#else
  fd_dbg = stdout;
#endif

  if (yydebug) {
    extern int lineno;
#ifdef __GNUC__
/* extern char *yyname[]; */
#else
    typedef struct {
      char *t_name;
      int t_val;
    } yytoktype;
    extern yytoktype yytoks[];
#endif
#ifndef __STDC__
    char *fmt;
#endif
    va_list marker;
    int longflg;
    int fmtflg;
    Node *n;
    longflg = fmtflg = FALSE;
#ifdef __STDC__
    va_start(marker, fmt);
#else
    va_start(marker);
    fmt = va_arg(marker, char *);
#endif
    fprintf(fd_dbg, "<<DEBUG>> %5.5d : ", lineno);
    /* Correction JF 22/9/98 */
     while(*fmt != (char) NULL) {
      if (*fmt != '%' && fmtflg == FALSE)
        fputc(*fmt++, fd_dbg);
      else {
        char c;
        int i;
        int t_val;
        fmtflg = TRUE;
      
        switch(*++fmt) {
          case '%':
            putc('%', fd_dbg);
            longflg = fmtflg = FALSE;
            break;
          case 'c':
            c = va_arg(marker, char);
            fprintf(fd_dbg, "%c", c);
            longflg = fmtflg = FALSE;
            break;
          case 's' :
            fprintf(fd_dbg, "%s", va_arg(marker, char *));
            longflg = fmtflg = FALSE;
            break;
          case 'x' :
            if (longflg)
              fprintf(fd_dbg, "%lx", va_arg(marker, unsigned long));
            else
              fprintf(fd_dbg, "%x", va_arg(marker, unsigned int));
            longflg = fmtflg = FALSE;
            break;
          case 'X' :
            if (longflg)
              fprintf(fd_dbg, "%lX", va_arg(marker, unsigned long));
            else
              fprintf(fd_dbg, "%X", va_arg(marker, unsigned int));
            longflg = fmtflg = FALSE;
            break;
          case 'u' :
            if (longflg)
              fprintf(fd_dbg, "%lu", va_arg(marker, unsigned long));
            else
              fprintf(fd_dbg, "%u", va_arg(marker, unsigned int));
            longflg = fmtflg = FALSE;
            break;
          case 'd' :
            if (longflg)
              fprintf(fd_dbg, "%ld", va_arg(marker, long));
            else
              fprintf(fd_dbg, "%d", va_arg(marker, int));
            longflg = fmtflg = FALSE;
            break;
          case 'g' :
          case 'f' :
            if (longflg)
              fprintf(fd_dbg, "%g", va_arg(marker, double));
            else
              fprintf(fd_dbg, "%g", (double)va_arg(marker, float));
            longflg = fmtflg = FALSE;
            break;
          case 'l' :
            longflg = TRUE;
            break;
          case 'M' :
            t_val = va_arg(marker, int);
            if (t_val >= 0) {
#ifdef __GNUC__
/*
              if (yyname[t_val] != 0)
                fprintf(fd_dbg, "%s", yyname[t_val]);
              else
                fprintf(fd_dbg, "-unknown- (%d)", t_val);
*/
              fprintf(fd_dbg, "<%d>", t_val);
#else
              for (i=0; yytoks[i].t_val >= 0 && yytoks[i].t_val!=t_val; i++);
              if (yytoks[i].t_val < 0)
                fprintf(fd_dbg, "-unknown- (%d)", t_val);
              else
                fprintf(fd_dbg, "%s", yytoks[i].t_name);
#endif
            }
            else
              fprintf(fd_dbg, "-out of range- (%d)", t_val);
            longflg = fmtflg = FALSE;
            break;
          case 'N' :
            n = va_arg(marker, Node *);
            if (n == NULL)
              fprintf(fd_dbg, "null");
            else {
              yydebug = FALSE;
	      /* Modif JF 22/9/98 : */
	      /* L'appel a printnode plante parfois sur Linux */
              printnode(fd_dbg, n);
	      /*fprintf(fd_dbg,"\n  [Node= 0x%lx : Warning : call to printnode removed !]\n",(unsigned long)n);*/
              yydebug = TRUE;
            }
            longflg = fmtflg = FALSE;
            break;
          default :
            fprintf(fd_dbg, "%%%c", *fmt);
            longflg = fmtflg = FALSE;
            break;
        }
        if(longflg == FALSE)
          fmt++;
      }
    }
    va_end(marker);
    /*(void) fflush(NULL);*/
    (void) fflush(fd_dbg);
  }
}


#ifdef __STDC__
void _PRDBG(char *fmt, ...)
#else
void _PRDBG(va_alist)
va_dcl
#endif
{
#ifdef DEBUG
  extern int yydebug;
#endif

  if (yydebug) {
#ifndef __STDC__
    char *fmt;
#endif
    va_list marker;
    fprintf(mwerr, "<<DEBUG>> : ");
#ifdef __STDC__
    va_start(marker, fmt);
#else
    va_start(marker);
    fmt = va_arg(marker, char *);
#endif
    vfprintf(mwerr, fmt, marker);
    va_end(marker);
  }
}
#endif


#ifdef __STDC__
void mwcexit(int no)
#else
void mwcexit(no)
int no;
#endif
{
  if (no != 0) {
    /* Delete all temporary file */
    ;
    fprintf(mwerr, "Exit.\n");
  }
  exit(no);
}


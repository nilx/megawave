/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

%{
static char *sccsid = "%Z%MegaWave2 %R%, %M% %I%, %G%";

/* Fichiers d'include */
#include <stdio.h>
#include <string.h>
#ifdef __STDC__
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include "bintree.h"
#include "symbol.h"
#include "token.h"
#include "io.h"
#include "y.tab.h"
#include "strtoul.h"


#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif


static char lexerrbuf[BUFSIZ] = "";
static char *perr = lexerrbuf;

#ifdef DEBUG
extern int yydebug;
#endif

extern int headerflg;

#ifdef __STDC__
void lexerr(void);
unsigned long *toktoio(char *);
unsigned long *toktoid(char *);
unsigned long *toktoih(char *);
double *toktodr(char *);
double *toktoer(char *);
char *toktoce(char *);
char *toktoco(char *);
char *toktoch(char *);
char *toktoc(char *);
char *toktos(char *);
#else
extern lexerr();
extern unsigned long *toktoio();
extern unsigned long *toktoid();
extern unsigned long *toktoih();
extern double *toktodr();
extern double *toktoer();
extern char *toktoce();
extern char *toktoco();
extern char *toktoch();
extern char *toktoc();
extern char *toktos();
#endif

extern int lineno;
extern char filein[];

#define EAT_LINE while(input() != '\n'); unput('\n')
#define RET(T)   lexerr(); return(T)
/*#define RET(T)   lexerr(); ret(T)*/

#define DBX_SYNC if ((yylval.sync=(Sync *)malloc(sizeof(Sync))) != NULL) {\
		   yylval.sync->lineno=lineno;\
		   yylval.sync->filein=toktos(filein);\
		 }\
		 else\
		   fatal_error("Cannot alloc memory")

%}
decimal			[0-9]
octal			[0-7]
hexa			[0-9a-fA-F]
integer_dec		{decimal}+[uU]?[lL]?
integer_oct		0{octal}+U?L?
integer_hex		0[xX]{hexa}+U?L?

/* Changed by jf 14/12/99 : fF added */
dreal			{decimal}*"."{decimal}+[lLfF]?
dreal2			{decimal}+"."[lLfF]?
ereal			{decimal}*"."{decimal}+[eE][+-]?{decimal}+[lLfF]?
ereal2			{decimal}+"."?[eE][+-]?{decimal}+[lLfF]?

set_char_esc		\\[abfnrtv\\\?'\"]
set_char_oct		\\{octal}{octal}?{octal}?
set_char_hex		\\x{hexa}{hexa}?
ws			[ \t]+
id			[_a-zA-Z][_a-zA-Z0-9]*

/* Changed by jf 02/06/98 : ^L added */
nl			[\n]

%%
{ws}			;			/* Eat white space */

"author"		{					/* MWP */
			  if (headerflg) {
			    RET(AUTHOR) ;
			  }
			  else
			    REJECT ;	/* Enable the use of typic MWP keyword
			                   in C program */
			}
"auto"			{ RET(AUTO) ;}				/*  C  */
"break"			{ RET(BREAK) ;}				/*  C  */
"case"			{ RET(CASE) ;}				/*  C  */
"char"			{ RET(CHAR) ;}				/*  C  */
"const"			{					/*  C  */
#			ifdef ACCEPT_ANSI
			  RET(CONST) ;
#			else
			  REJECT;
#			endif
			}
"continue"		{ RET(CONTINUE) ;}			/*  C  */
"default"		{ RET(DEFAULT) ;}			/*  C  */
"do"			{ RET(DO) ;}				/*  C  */
"double"		{ RET(DOUBLE) ;}			/*  C  */
"else"			{ RET(ELSE) ;}				/*  C  */
"enum"			{ RET(ENUM) ;}				/*  C  */
"extern"		{ RET(EXTERN) ;}			/*  C  */
"float"			{ RET(FLOAT) ;}				/*  C  */
"for"			{ RET(FOR) ;}				/*  C  */
"function"		{					/* MWP */
			  if (headerflg) {
			    RET(FUNCTION) ;
			  }
			  else
			    REJECT ;	/* Enable the use of typic MWP keyword
			                   in C program */
			}
"goto"			{ RET(GOTO) ;}				/*  C  */
"group"			{					/* MWP */
			  if (headerflg) {
			    RET(GROUP) ;
			  }
			  else
			    REJECT ;	/* Enable the use of typic MWP keyword
			                   in C program */
			}
"if"			{ RET(IF) ;}				/*  C  */
"int"			{ RET(INT) ;}				/*  C  */
"labo"			{					/* MWP */
			  if (headerflg) {
			    RET(LABO) ;
			  }
			  else
			    REJECT ;	/* Enable the use of typic MWP keyword
			                   in C program */
			}
"long"			{ RET(LONG) ;}				/*  C  */
"mwcommand"		{					/* MWP */
			  EAT_LINE;
			  RET(MWCOMMAND) ;
			}
"name"			{					/* MWP */
			  if (headerflg) {
			    RET(NAME) ;
			  }
			  else
			    REJECT ;	/* Enable the use of typic MWP keyword
			                   in C program */
			}
"notused"		{					/* MWP */
			  if (headerflg) {
			    RET(NOTUSED) ;
			  }
			  else
			    REJECT ;	/* Enable the use of typic MWP keyword
			                   in C program */
			}
"register"		{ RET(REGISTER) ;}			/*  C  */
"return"		{ RET(RETURN) ;}			/*  C  */
"short"			{ RET(SHORT) ;}				/*  C  */
"signed"		{ RET(SIGNED) ;}			/*  C  */
"sizeof"		{ RET(SIZEOF) ;}			/*  C  */
"static"		{ RET(STATIC) ;}			/*  C  */
"struct"		{ RET(STRUCT) ;}			/*  C  */
"switch"		{ RET(SWITCH) ;}			/*  C  */
"typedef"		{ RET(TYPEDEF) ;}			/*  C  */
"union"			{ RET(UNION) ;}				/*  C  */
"unsigned"		{ RET(UNSIGNED) ;}			/*  C  */
"usage"			{ 					/* MWP */
			  if (headerflg) {
			    RET(USAGE) ;
			  }
			  else
			    REJECT ;	/* Enable the use of typic MWP keyword
			                   in C program */
			}
"version"		{					/* MWP */
			  if (headerflg) {
			    RET(VERSION) ;
			  }
			  else
			    REJECT ;	/* Enable the use of typic MWP keyword
			                   in C program */
			}
"void"			{ RET(VOID) ;}				/*  C  */
"volatile"		{					/*  C  */
#			ifdef ACCEPT_ANSI
			  RET(VOLATILE) ;
#			else
			  REJECT;
#			endif
			}
"while"			{ RET(WHILE) ;}				/*  C
			  */

^"#"			{					/*  C  */
			  static char prev_filenam[BUFSIZ] = "";
			  unsigned int size_string;
                          int linenotest;
			  /* Intercept the CPP line "#{ws}line {integer} [{qstring}]"
			     and set 'lineno' and 'filein' variables with
			     corresponding fields if they exist; if {qstring}
			     does not exist, filename is assumed to be in
			     static variable prev_filename. */
			  char c, buf[BUFSIZ], *pbuf;

			  lexerr();

			  /* Line is supposed to be correct ie : */
			  /*   #{ws}line {integer} [{qstring}]             */
			  while ((c=input()) == '\t' || c == ' ');
			  /* Line number field */
			  buf[0] = c;
			  for (pbuf = buf+1, size_string = 1;
			       (c=input())!='\t' && c!=' ' && c!='\n' &&
			       size_string < BUFSIZ-1;
			       pbuf++, size_string++)
			    *pbuf = c;
			  *pbuf= '\0';
                          if (strncmp(buf,"line",5) == 0) {
			    while ((c=input()) == '\t' || c == ' ');
			    /* Line number field */
			    buf[0] = c;
			    for (pbuf = buf+1, size_string = 1;
			         (c=input())!='\t' && c!=' ' && c!='\n' &&
				 size_string < BUFSIZ-1;		 
			         pbuf++, size_string++)
			      *pbuf = c;
			    *pbuf= '\0';
			    if (c == '\n')
			      unput(c);
			    linenotest = (int)(*toktoid(buf));
                          }
                          else
                            linenotest = (int)(*toktoid(buf));
                          if (linenotest != 0) {
                            lineno = linenotest - 1;
			    while ((c=input()) == '\t' || c == ' ');
			    if (c == '\n') {
			      unput('\n');
			      if (strlen(prev_filenam) == 0) {
#			        ifdef DEBUG
                                PRDBG("No available filename\n");
#			        endif
			        INT_ERROR("yylex");
			      }
			      else {
			        strcpy(filein, prev_filenam);
#			        ifdef DEBUG
			        PRDBG("Prev filename : \"%s\"\n", prev_filenam);
#			        endif
			      }
			    }
			    else {
			      /* Source file name field */
			      for (pbuf = filein, size_string = 0;
			           (c=input())!='"' &&
				   size_string < BUFSIZ-1;
				   pbuf++, size_string++)
			        *pbuf = c;
			      *pbuf = '\0';
			      strncpy(prev_filenam, filein, BUFSIZ-1);
                            }
#			    ifdef DEBUG
			      PRDBG("File \"%s\", line %d\n", filein, lineno+1);
#			    endif
			  }
			  /* Destroy line up to just before {nl} */
			  EAT_LINE;
			  }

"="			{ RET('='); }				/*  C  */
"{"			{ RET('{'); }				/*  C  */
"}"			{ DBX_SYNC; RET('}'); }			/*  C  */
"("			{ RET('('); }				/*  C  */
")"			{ DBX_SYNC; RET(')'); }			/*  C  */
"["			{ RET('['); }				/*  C  */
"]"			{ RET(']'); }				/*  C  */
"+"			{ RET('+'); }				/*  C  */
"-"			{ RET('-'); }				/*  C  */
"*"			{ RET('*'); }				/*  C  */
"/"			{ RET('/'); }				/*  C  */
"%"			{ RET('%'); }				/*  C  */
"&"			{ RET('&'); }				/*  C  */
"|"			{ RET('|'); }				/*  C  */
"^"			{ RET('^'); }				/*  C  */
"!"			{ RET('!'); }				/*  C  */
"?"			{ RET('?'); }				/*  C  */
":"			{ RET(':'); }				/*  C  */
";"			{ DBX_SYNC; RET(';'); }			/*  C  */
","			{ RET(','); }				/*  C  */
"."			{ RET('.'); }				/*  C  */
">"			{ RET(GT) ;}				/*  C  */
"<"			{ RET(LT) ;}				/*  C  */

"*="			{ RET(MULEQ) ;}				/*  C  */
"/="			{ RET(DIVEQ) ;}				/*  C  */
"%="			{ RET(MODEQ) ;}				/*  C  */
"+="			{ RET(PLUSEQ) ;}			/*  C  */
"-="			{ RET(MINUSEQ) ;}			/*  C  */
"&="			{ RET(ANDEQ) ;}				/*  C  */
"^="			{ RET(EREQ) ;}				/*  C  */
"|="			{ RET(OREQ) ;}				/*  C  */
"||"			{ RET(OROR) ;}				/*  C  */
"&&"			{ RET(ANDAND) ;}			/*  C  */
"=="			{ RET(EQ) ;}				/*  C  */
"!="			{ RET(NE) ;}				/*  C  */
"<="			{ RET(LE) ;}				/*  C  */
">="			{ RET(GE) ;}				/*  C  */
"<<"			{ RET(LS) ;}				/*  C  */
">>"			{ RET(RS) ;}				/*  C  */
"++"			{ RET(INCR) ;}				/*  C  */
"--"			{ RET(DECR) ;}				/*  C  */
"<-"			{ RET(LARROW) ;}			/* MWP */
"->"			{					/*  C  */
			  if (headerflg) {
			    RET(RARROW) ;
			  }
			  else {
			    RET(STREF);
			  }
			}

"<<="			{ RET(LSEQ) ;}				/*  C  */
">>="			{ RET(RSEQ) ;}				/*  C  */
"..."			{ RET(ELLIPSIS) ;}			/*  C  */

"/*"{ws}*/mwcommand	{ RET(BCOMMENT) ;}			/* MWP */
"*/"			{ RET(ECOMMENT) ;}			/* MWP */

"/*"			{					/*  C  */
			  int cont_flg, c;
			  extern int lineno;
			  lexerr();
			  cont_flg = TRUE;
			  while (cont_flg) {
			    while ((c=input()) != '*')
			      if (c == '\n')
			        lineno++;
			    switch (input()) {
			      case '/': cont_flg = FALSE; break;
			      case '*': unput('*');
			      default: break;
			    }
			  }
			}

"\""			{
			  char buffer[BUFSIZ];
			  char c, *buf;
			  int cont_flg, i;
			  lexerr();
			  cont_flg = TRUE;
			  buffer[0] = '"';
			  for (i=1; cont_flg;) {
			    switch (c=input()) {
			      case '\\': if ((c = input()) != '\n') {
			                   buffer[i++] = '\\';
			                   buffer[i++] = c;
			                 }
			                 break;
			      case '"':  buffer[i++] = '"';
			                 buffer[i] = '\0';
			                 cont_flg = FALSE;
			                 break;
			      case '\n': error("missing \" in string constant\n");
			                 buffer[i] = '\0';
			                 cont_flg = FALSE;
			                 break;
			      default:   buffer[i++] = c;
			                 break;
			    }
			  }

			  yylval.qstring = toktos(buffer);
#ifdef DEBUG
			  PRDBG("buffer = >%s<\n", buffer);
#endif
			  RET(QSTRING) ;
			}

{integer_oct}		{
			  yylval.integer = toktoio(yytext);
			  RET(INTEGER) ;
			}
{integer_dec}		{
			  yylval.integer = toktoid(yytext);
			  RET(INTEGER) ;
			}
{integer_hex}		{
			  yylval.integer = toktoih(yytext);
			  RET(INTEGER) ;
			}

{dreal}			{
			  yylval.real = toktodr(yytext);
			  RET(REAL) ;
			}
{dreal2}		{
			  yylval.real = toktodr(yytext);
			  RET(REAL) ;
			}
{ereal}			{
			  yylval.real = toktoer(yytext);
			  RET(REAL) ;
			}
{ereal2}		{
			  yylval.real = toktoer(yytext);
			  RET(REAL) ;
			}

"'"{set_char_esc}"'"	{
			  yylval.character = toktoce(yytext);
			  RET(CHARACTER) ;
			}
"'"{set_char_oct}"'"	{
			  yylval.character= toktoco(yytext);
			  RET(CHARACTER) ;
			}
"'"{set_char_hex}"'"	{
			  yylval.character= toktoch(yytext);
			  RET(CHARACTER) ;
			}
"'"."'"			{
			  yylval.character= toktoc(yytext);
			  RET(CHARACTER) ;
			}

{id}			{
			  Symbol *s;
			  s = LOOKUP(yytext);
#ifdef DEBUG
			  if (s != NULL)
			    PRDBG("yylex : obj = %M\n", GET_OBJ(s));
#endif
			  if (s == NULL) {
/*			    char *p, *renameid();		*/
/*			    if ((p = renameid(yytext)) != NULL)	*/
/*			      yylval.text = toktos(p);		*/
/*			    else				*/
			      yylval.text = toktos(yytext);
			    RET(ID) ;
			  } else if (GET_OBJ(s) == TYPEDEF) {
			    yylval.text = toktos(yytext);
			    RET(USRTYPID) ;
			  } else if (GET_OBJ(s) == ENUMERATION) {
			    yylval.text = toktos(yytext);
			    RET(ENUMERATION) ;
			  } else {
			    yylval.text = toktos(yytext);
			    RET(ID) ;
                          }
			}

{nl}			{
			  extern int lineno;
			  lexerr();
			  lineno++;

			}

.			{		/* LEXICAL ERROR */
			  *perr++ = yytext[0];
			}
%%
#ifdef __STDC__
void lexerr(void)
#else
lexerr()
#endif
{
  *perr = '\0';
  if (strlen(lexerrbuf) != 0) {
    error("Lexical error : '%s' is a bad item\n", lexerrbuf);
    lexerrbuf[0] = '\0';
    perr = lexerrbuf;
  }
}

/* Transform string represent octal number in octal number */
#ifdef __STDC__
unsigned long *toktoio(char *s)
#else
unsigned long *toktoio(s)
char *s;
#endif
{
  unsigned long *ret;
  ret = MALLOC(unsigned long);
  *ret = (unsigned long)strtoul(s, NULL, 8);
  return ret;
}


/* Transform string represent decimal number in decimal number */
#ifdef __STDC__
unsigned long *toktoid(char *s)
#else
unsigned long *toktoid(s)
char *s;
#endif
{
  unsigned long *ret;
  ret = MALLOC(unsigned long);
  *ret = (unsigned long)strtoul(s, NULL, 10);
  return ret;
}

/* Transform string represent hexadecimal number in hexadecimal number */
#ifdef __STDC__
unsigned long *toktoih(char *s)
#else
unsigned long *toktoih(s)
char *s;
#endif
{
  unsigned long *ret;
  ret = MALLOC(unsigned long);
  *ret = (unsigned long)strtoul(s, NULL, 16);
  return ret;
}

/* Transform string represent fixed real number in real number */
#ifdef __STDC__
double *toktodr(char *s)
#else
double *toktodr(s)
char *s;
#endif
{
  double *ret;
  float f;
  (void) sscanf(s, "%g", &f);
  ret = MALLOC(double);
  *ret = (double)f;
#ifdef DEBUG
  PRDBG("toktodr : %s -> %lf\n", s, *ret);
#endif
  return ret;
}


/* Transform string represent exponent real number in real number */
#ifdef __STDC__
double *toktoer(char *s)
#else
double *toktoer(s)
char *s;
#endif
{
  double *ret;
  float f;
  (void) sscanf(s, "%g", &f);
  ret = MALLOC(double);
  *ret = (double)f;
#ifdef DEBUG
  PRDBG("toktoer : %s -> %lf\n", s, *ret);
#endif
  return ret;
}


/* Transform string represent escaped sequence character in character */
#ifdef __STDC__
char *toktoce(char *s)
#else
char *toktoce(s)
char *s;
#endif
{
  char *c;
  c = MALLOC(char);
  switch(s[2]) {	/* Look for the character after the "\" */
    case 'a':  *c = '\a'; break;
    case 'b':  *c = '\b'; break;
    case 'f':  *c = '\f'; break;
    case 'n':  *c = '\n'; break;
    case 'r':  *c = '\r'; break;
    case 't':  *c = '\t'; break;
    case 'v':  *c = '\v'; break;
    case '\\': *c = '\\'; break;
    case '?':  *c = '?'; break;
    case '\'': *c = '\''; break;
    case '"':  *c = '"'; break;
    default :  error("illegal character"); break;
  }
  return c;
}


/* Transform string represent octal escaped sequence character in character */
#ifdef __STDC__
char *toktoco(char *s)
#else
char *toktoco(s)
char *s;
#endif
{
  char buf[BUFSIZ], *c;
  c = MALLOC(char);
  strcpy(buf, s);
  /* Look for the octal number between the "'\" and "'"*/
  buf[strlen(s)-1] = '\0';
  *c = (char)strtoul(buf + 2, NULL, 8);
  return c;
}


/* Transform string represent hexa escaped sequence character in character */
#ifdef __STDC__
char *toktoch(char *s)
#else
char *toktoch(s)
char *s;
#endif
{
  char buf[BUFSIZ], *c;
  c = MALLOC(char);
  strcpy(buf, yytext);
  /* Look for the hexadecimal number between the "'\x" and "'"*/
  buf[strlen(yytext)-1] = '\0';
  *c = (char)strtol(buf + 3, NULL, 16);
  return c;
}


/* Transform string represent character in character */
#ifdef __STDC__
char *toktoc(char *s)
#else
char *toktoc(s)
char *s;
#endif
{
  char *c;
  c = MALLOC(char);
  /* Look for the character between the "'" and "'"*/
  *c = s[1];
  return c;
}


/*
 * CREATION
 * AUTEUR
 *
 * NOM		
 * FONCTION	
 * ENTREES	
 * SORTIES	
 *
 * EVOLUTIONS
 * DATE		PAR		MODIFICATIONS
 */
/* Transform string represent quoted string in string */
#ifdef __STDC__
char *toktos(char *s)
#else
char *toktos(s)
char *s;
#endif
{
  char *ret;
  if ((ret = malloc(strlen(s)+1)) != NULL)
    (void) strcpy(ret, s);
  else
    fatal_error("Cannot alloc memory");
  return ret;
}

static struct forbidenfunc {
  char *name;
  char *newname;
} forbidenfunc[] = {
  {"exit",    "mwexit"},
  {"_exit",   "_mwexit"},
  {"malloc",  "mwmalloc"},
  {"free",    "mwfree"},
  {"realloc", "mwrealloc"},
  {"calloc",  "mwcalloc"},
  {"cfree",   "mwcfree"},
  {NULL}
};


/*
 * CREATION
 * AUTEUR
 *
 * NOM		
 * FONCTION	
 * ENTREES	
 * SORTIES	
 *
 * EVOLUTIONS
 * DATE		PAR		MODIFICATIONS
 */
#ifdef __STDC__
char *renameid(char *s)
#else
char *renameid(s)
char *s;
#endif
{
  struct forbidenfunc *p;

  for (p = forbidenfunc; p->name != NULL; p++) {
    if (!strcmp(s, p->newname)) {
      error("'%s' : reserved MegaWave2 id\n", s);
      return s;
    }
    else if (!strcmp(s, p->name))
      return p->newname;
  }
  return NULL;
}

/**
 * @file io.c
 *
 * input/output functions for the megawave preprocessor
 *
 * @author Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2007), \
 *         Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/* TODO: drop */
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "definitions.h"

#include "io.h"

#define MSG_ERROR_NULL_FILE \
     "NULL file pointer"
#define MSG_ERROR_EOF_BODY \
     "Unexpected end of file while parsing comment in C body"
#define MSG_ERROR_EOF_HEADER \
     "Unexpected end of file while parsing comment in header"
#define MSG_ERROR_GETSENTENCE_LONG_SENTENCE \
     "[getsentence] sentence exceeds the maximum number of character = %d"
#define MSG_ERROR_GETINSTRUCTION_LONG_SENTENCE \
     "[getinstruction] sentence exceeds the maximum number of character = %d"
#define MSG_ERROR_GETWORD_LONG_WORD \
     "Word in \"%s\" exceeds maximum size length of %d char"
#define MSG_ERROR_GETCID_LONG_CID \
     "C_id in \"%s\" exceeds maximum size length of %d char"
#define MSG_ERROR_LEFT_BOUND \
     "Left bound of interval \"%s\" exceeds maximum size length of %d char"
#define MSG_ERROR_RIGHT_BOUND \
     "Right bound of interval \"%s\" exceeds maximum size length of %d char"
#define MSG_DEBUG_GETINSTRUCTION \
     "[getinstruction] s='%s'"

#define LINE_LENGTH 80

/* test for a C id character (= valid char for C variable name) */
#define IS_CID(x) ((((x) >= 'A') && ((x) <= 'Z')) || \
                   (((x) >= 'a') && ((x) <= 'z')) ||    \
                   (((x) >= '0') && ((x) <= '9')) ||    \
                   ((x)=='_'))

/*
 * compute the line nb and char nb of the current <posfile> file
 * position
 * write a report in the <out> string, which must be longer than
 * LINE_LENGTH * 3
 */
static char * str_fileposition(char * out, FILE * posfile)
{
     long position;
     int nblines = 0;
     int nbchars = 0;
     int linestart = 0;
     char line[LINE_LENGTH + 1];
     int c;

     if ((position = ftell(posfile)) < 0)
          /* cannot optain the current position */
          return out;

     rewind(posfile);

     /* count the number of lines */
     while (((long) nbchars < position) && ((c = getc(posfile)) != EOF))
     {
          nbchars ++;
          if ((char) c == '\n')
          {
               nblines++;
               linestart = nbchars;
          }
     }

     /*
      * get the position in the line,
      * then get back to the beginning of the line
      * and get the line
      */
     nbchars -= linestart;
     fseek(posfile, linestart, SEEK_SET);
     (void) fgets(line, LINE_LENGTH - 3, posfile);

     if (nbchars > LINE_LENGTH)
     {
          strcat(line, "...");
          nbchars = LINE_LENGTH;
     }
     /* go back to the previous position in the file */
     fseek(posfile, position, SEEK_SET);

     sprintf(out,                                                       \
              "%i,%i:\n%s%*c\n",                                     \
              nblines, nbchars, line, nbchars, '^');

     return out;
}

#define LOG_ERROR 0
#define LOG_WARN  1
#define LOG_INFO  2
#define LOG_DEBUG 3

/*
 * output a log for debug, infos, warnings and errors
 */
/* TODO : add function, file and line info */
static void logger(int log_level, FILE * outfile, char * msg)
{
     switch(log_level)
     {
     case LOG_DEBUG:
          fprintf(outfile, "mwp.debug   : %s\n", msg);
          break;
     case LOG_INFO:
          fprintf(outfile, "mwp.info    : %s\n", msg);
          break;
     case LOG_WARN:
          fprintf(outfile, "mwp.warning : %s\n", msg);
          break;
     case LOG_ERROR:
          fprintf(outfile, "mwp.error   : %s\n", msg);
          break;
     default:
          fprintf(outfile, "mwp         : %s\n", msg);
          break;
     }
}

void debug(char * fmt, ...)
{
     va_list marker;
     char msg[STRSIZE];

     va_start(marker, fmt);
     (void) vsprintf(msg, fmt, marker);
     va_end(marker);

     logger(LOG_DEBUG, stdout, msg);
}

void info(char * fmt, ...)
{
     va_list marker;
     char msg[STRSIZE];

     va_start(marker, fmt);
     vsprintf(msg, fmt, marker);
     va_end(marker);

     logger(LOG_INFO, stdout, msg);
}

void warning(char * fmt, ...)
{
     va_list marker;
     char msg[STRSIZE];

     va_start(marker, fmt);
     vsprintf(msg, fmt, marker);
     va_end(marker);

     logger(LOG_WARN, stderr, msg);
}

void error(char * fmt, ...)
{
     va_list marker;
     char msg[STRSIZE];
     char position_info[LINE_LENGTH * 3];

     va_start(marker, fmt);
     vsprintf(msg, fmt, marker);
     va_end(marker);

     logger(LOG_ERROR, stderr, msg);
     fprintf(stderr, "\n");
     fprintf(stderr, "%s", str_fileposition(position_info, source_file_global));
     abort();
}



/*
 * convert string <in> to lowercase.
 * return the number of words in <in>, given as the number of spaces + 1.
 */
int lowerstring(char * in)
{
     int ns = 0;

     for ( ; * in != '\0'; in++)
     {
          if (isupper(* in))
               * in = tolower(* in);
          else
               if (* in == ' ')
                    ns++;
     }
     return (ns + 1);
}

/*
 * return the input string s corrected to be printed using printf()
 */

char * getprintfstring(char * s)
{
     static char o[TREESTRSIZE];
     int i, j;

     for (i =0, j = 0; s[i] != '\0'; i++, j++)
          switch(s[i])
          {
          case '"':
               j--;
               break;
          case '%':
               o[j++] = '%';
          default :
               o[j] = s[i];
          }
     o[j] = '\0';
     return o;
}


/*
 * remove unnecessary spaces in <in>.
 * example : " toto  titi " -> "toto titi"
 * change also \n \t \f to a space.
 */
void removespaces(char * in)
{
     int i, j;

     if (in[0] == '\0')
          return;
     /* change blank char to space */
     i = 0;
     while (in[i] != '\0')
     {
          if ((in[i] == '\n') || (in[i] == '\t') || (in[i] == '\f'))
               in[i]=' ';
          i++;
     }

     i=0;
     while (in[i] != '\0')
     {
          if ((in[i] == ' ') && ((i == 0) || (in[i - 1] == ' ') || \
                                 (in[i + 1] == '\0')))
          {
               /* shift 1 char on the left */
               j = i;
               do
               {
                    in[j] = in[j + 1];
               } while (in[j++] != '\0');
               i--;
          }
          i++;
     }
}


/*
 * skip comment sequence in <fs>, assuming being already inside the comment.
 * comments are delimited by '/' '*' .... '*' '/' in C body
 * and by '/' '/' in header until EOL (as in C++)
 * generate an error if EOF encountered
 */
static void skipcomment(FILE * sfile)
{
     int l, l0 = 0;

     if (sfile == NULL)
          error(MSG_ERROR_NULL_FILE);
     if (inside_header == 0)
     {
          /* seek for end of comment sequence in C body */
          while (((l = getc(sfile)) != EOF) && \
                 ((l != '/') || (l0 != '*')))
               l0=l;
          if (l == EOF)
               error(MSG_ERROR_EOF_BODY);
     }
     else
     {
          /* seek for end of comment sequence in header */
          while (((l = getc(sfile)) != EOF) && (l != '\n'));
          if (l == EOF)
               error(MSG_ERROR_EOF_HEADER);
     }
}


/*
 * skip the current line in <sfile> : go to the beginning of the next line.
 * '\' is  recognized as the symbol to avoid line breaking.
 * if a comment is encountered in the line, skip lines until end of comment.
 * return EOF if end of file, '\n' elsewhere.
 */
static int skipline(FILE * sfile)
{
     int l, l0;

cont:
     l0 = 0;
     while (((l = getc(sfile)) != EOF) && (l != '\n'))
     {
          if ((l0 == '/') && (l == '*'))
               skipcomment(sfile);
          l0 = l;
     }
     if (l0 == '\\')
          goto cont;
     return(l);
}

/*
 * get in <line> next line in file <sfile>.
 * '\' is not recognized as the symbol to avoid line breaking.
 * return EOF if end of file, 0 if the line is empty, 1 elsewhere.
 */
int getline(FILE * sfile, char * line)
{
     int l;

     if (sfile == NULL)
          error(MSG_ERROR_NULL_FILE);
     l = fscanf(sfile, "%[^\n]", line);
     /* read the \n */
     getc(sfile);
     if ((l == 0) || (l == EOF))
          line[0]='\0';
     else
          removespaces(line);
     return l;
}

/*
 * get in <s> next sentence in file <sfile>.
 * a "sentence" is anything until next ';' not inside a string "..."
 * does not read comments (see skipcomment() for comments syntax)
 * return last char read (EOF if end of file, EOH if end of header)
 * should work inside or outside the header.
 */
int getsentence(FILE * sfile, char * s)
{
     char * p;
     int n, l, l0 = 0, l00 = 0;
     int intext = 0;

     if (sfile == NULL)
          error(MSG_ERROR_NULL_FILE);
     p = s;
     * p = '\0';
     n = 0;
     while (((l = getc(sfile)) != EOF) &&                  \
            ((intext == 1) || (l != ';'))               \
            /* EOF or ';' but maybe inside a text */
            && ((inside_header == 0) || (l != '/') || (l0 != '*')))
            /* seek for EOH */
     {
          if (((inside_header == 0) && (l0 == '/') && (l == '*')) || \
               ((inside_header > 0) && (l0 == '/') && (l == '/')))
          {
               /* we enter comment sequence */
               /* remove '/' from sentence */
               p--;
               n--;
               skipcomment(sfile);
               l0 = l00;
          }
          else
          {
               if (n > STRSIZE)
                    error(MSG_ERROR_GETSENTENCE_LONG_SENTENCE, STRSIZE);
               *(p++) = (char)l;
               n++;
               l00 = l0;
               l0 = l;
               if (l == '"')
                    /* a "text" is everything inside "..." */
                    intext = 1 - intext;
          }
     }

     if ((inside_header != 0) && (l == '/') && (l0 == '*'))
     {
          /* EOH read : '*' followed by '/' */
          * (p--) = '\0';
          removespaces(s);
          /* printf("EOH detected s='%s'\n",s); */
          return(EOH);
     }

     * p = '\0';
     removespaces(s);
     /* printf("s='%s'\n",s); */
     return l;
}


/*
 * skip block {...} in file <sfile> :
 * continue until next } is reached (or EOF).
 * work recursively on sub-blocks.
 * work outside the header only.
 */
static void skipblock(FILE * sfile)
{
     int l, l0 = 0, in_quote, in_dquote;

     if (sfile == NULL)
          error(MSG_ERROR_NULL_FILE);

     in_quote  = 0;
     in_dquote = 0;

     while ((l = getc(sfile)) != EOF)
     {
          switch(l)
          {
          case '"' :
               if (l0 != '\\')
               {
                    if (in_quote == 0)
                         in_dquote = 1 - in_dquote;
               }
               break;
          case '\'' :
               if (l0 != '\\')
               {
                    if (in_dquote == 0)
                         in_quote = 1 - in_quote;
               }
               break;
          case '*' :
               if ((in_quote == 0) && (in_dquote == 0))
               {
                    if (l0 == '/')
                         /* we enter comment sequence */
                         skipcomment(sfile);
               }
               break;
          case '#' :
               if ((in_quote == 0) && (in_dquote == 0))
                    /* we enter a #something sequence */
                    skipline(sfile);
               break;
          case '{' :
               if ((in_quote == 0) && (in_dquote == 0))
                    /* we enter a new block {...} */
                    skipblock(sfile);
               break;
          case '}' :
               if ((in_quote == 0) && (in_dquote == 0))
                    /* we enter a block {...} */
                    /* end of block reached */
                    return;
               break;
          }
          l0 = l;
     }
}


/*
 * get in <s> next instruction in file <sfile>.
 * an "instruction" is anything until next ';', but
 * - lines beginning by #,
 * - block instructions {...}
 * does not read comments (see skipcomment() for comments syntax)
 * return last char read (EOF if end of file).
 * set <lbeg>,<lend> that try to record location of the beginning
 * and of the end of the "right" instruction in the file.
 * work outside the header only.
 */
int getinstruction(FILE * sfile, char * s,long * lbeg, long * lend)
{
     char * p, * q;
     int n, l, l0 = 0, l00 = 0, in_quote, in_dquote, nl;

     if (sfile == NULL)
          error(MSG_ERROR_NULL_FILE);
     p = s;
     * p = '\0';
     n = 0;
     /* FIXME: what should be the correct initial value? */
     nl = 0;
     in_quote  = 0;
     in_dquote = 0;
     * lbeg = -1;
     * lend = -1;

     while ((l = getc(sfile)) != EOF)
     {
          switch(l)
          {
          case '"':
               if (l0 != '\\')
                    if (in_quote == 0)
                         in_dquote = 1 - in_dquote;
               goto def;
          case '\'':
               if (l0 != '\\')
               {
                    if (in_dquote == 0)
                         in_quote = 1 - in_quote;
               }
               goto def;
          case '*':
               if ((in_quote == 0) && (in_dquote == 0))
               {
                    if (l0 == '/')
                    {
                         /* we enter comment sequence */
                         /* remove '/' from sentence */
                         p--;
                         if (nl == n)
                              *lbeg = -1;
                         n--;
                         skipcomment(sfile);
                         l0 = l00;
                         break;
                    }
                    goto def;
               }
               break;
          case '#' :
               if ((in_quote == 0) && (in_dquote == 0))
               {
                    /* we enter a #something sequence */
                    skipline(sfile);
                    break;
               }
               goto def;
          case '{' :
               if ((in_quote == 0) && (in_dquote == 0))
               {
                    /* we enter a block {...} */
                    skipblock(sfile);

                    /*
                     * record the {} so that we will be able
                     * to keep trace of it,
                     * so the block is replaced by an empty one.
                     */
                    if ( n + 1 > STRSIZE)
                         error(MSG_ERROR_GETINSTRUCTION_LONG_SENTENCE, \
                               STRSIZE);
                    q = p - 1;
                    * (p++) = '{';
                    n++;
                    l00 = l0;
                    l0 = l;
                    *(p++) = '}';
                    n++;
                    l00 = l0;
                    l0 = l;

                    /*
                     * analyse if char preceding '{' was ')'
                     * so that we guess a function
                     * declaration and we end the instruction after '}'.
                     */
                    while ((q != s) && ((* q == ' ')  || (* q == '\n') || \
                                        (* q == '\t') || (* q == '\f')))
                         q--;
                    if (* q == ')')
                         /* end of the instruction */
                         goto EndofInstruction;

                    break;
               }
               goto def;

          default:
          def:
               /* record the character */
               if (n > STRSIZE)
                         error(MSG_ERROR_GETINSTRUCTION_LONG_SENTENCE, \
                               STRSIZE);
               * (p++) = (char)l;
               n++;
               l00 = l0;
               l0 = l;

               /*
                * check if the character is meaningful
                * so that we can say here the "right"
                * instruction begins.
                */
               if ((* lbeg == -1) && (l != ' ') && (l != '\n') && \
                   (l != '\t') && (l != '\f') && (l != '\r') && (l != '\v'))
               {
                    * lbeg = ftell(sfile);
                    nl = n;
               }

               if (l == ';')
                    /* end of the instruction */
                    goto EndofInstruction;
               break;
          }
     }
EndofInstruction:
     * p = '\0';
     removespaces(s);
     /* can compute better estimate (as with lbeg) if needed */
     * lend = ftell(sfile);

     if (debug_flag)
          debug(MSG_DEBUG_GETINSTRUCTION, s);

     return l;
}


/*
 * remove first surrounding braces in <in>, removing outside spaces.
 * output is in <out>.
 * ex: in=" {toto ti{t}i } " -> out="toto ti{t}i"
 * return 1 if braces removed, 0 elsewhere.
 */
int removebraces(char * in, char * out)
{
     int i, i0, i1;

     out[0] = '\0';
     if (in[0] == '\0')
          return 0;

     i = 0;
     while (in[i] == ' ')
          i++;
     if (in[i] != '{')
          return 0;
     i0 = i;

     i = (int) strlen(in) - 1;
     while ((i >= 0) && (in[i] == ' '))
          i--;
     if ((i <= i0) || (in[i] != '}'))
          return 0;
     i1 = i;

     for (i = i0 + 1; i < i1; i++)
          out[i - i0 - 1] = in[i];
     out[i - i0 - 1] = '\0';

     return 1;
}

/*
 * remove the terminating space in string <in>, if any.
 * ex: in="char * " -> in="char *"
 */
void RemoveTerminatingSpace(char * in)
{
     size_t l;

     if ((!in) || (( l = strlen(in)) == 0))
          return;
     if (in[l - 1] == ' ')
          in[l - 1] = '\0';
}


/*
 * get the enclosed string by removing surrounding quotation marks (").
 * everything before those marks are removed, if any.
 * ex: '..."toto "titi""...' -> 'toto "titi"'
 * return 1 if quotation marks found and removed, 0 if nothing done
 * (in that case <out> is undefined).
 */
int getenclosedstring(char * in, char * out)
{
     int l, i, i0, i1;

     l = (int) strlen(in) - 1;
     /* find fist quotation mark */
     for (i0 = 0; (i0 <= l) && (in[i0] != '"'); i0++);
     if (i0 == l)
          return 0;

     /* find last quotation mark */
     for (i1 = l; (i1 >= 0) && (in[i1] != '"'); i1--);
     if (i1 <= i0)
          return 0;

     /* set <out> */
     for (i = i0 + 1; i < i1; i++)
          out[i - i0 - 1] = in[i];
     out[i - i0 - 1] = '\0';
     return 1;
}


/*
 * get the first word in <s> and put it in <w>
 * (a word may be a non-blank separator).
 * Return the non-blank position just after the last char of word,
 * or 0 if word not found.
 * ex: " toto, titi" -> <w>="toto" pos=5 (index. of ',')
 */
int getword(char * s, char * w)
{
     int i, i0;

     i = 0;
     while ((s[i] != '\0') && (s[i] == ' '))
          i++;
     if (s[i] == '\0')
     {
          w[0] = '\0';
          return 0;
     }
     if (!IS_CID(s[i]))
     {
          /* separator */
          w[0] = s[i];
          w[1] = '\0';
          return (i + 1);
     }
     i0 = i;
     while ((s[i] != '\0') && (IS_CID(s[i])))
     {
          if (i - i0 >= TREESTRSIZE)
               error(MSG_ERROR_GETWORD_LONG_WORD, s, TREESTRSIZE - 1);
          w[i - i0] = s[i];
          i++;
     }
     w[i - i0] = '\0';
     if (i > 0)
          while ((s[i] != '\0') && (s[i] == ' '))
               i++;
     return i;
}

/*
 * get the first C_id (C variable name) in <s> and put it in <cid>.
 * return the non-blank position just after the last char of C_id,
 * or 0 if C_id not found.
 * ex: " toto [0,100]" -> <C_id>="toto" pos=6 (index. of '[')
 */
int getCid(char * s, char * cid)
{
     int i, i0;

     i = 0;
     while ((s[i] != '\0') && (s[i] == ' '))
          i++;
     i0 = i;
     while ((s[i] != '\0') && (IS_CID(s[i])))
     {
          if (i - i0 >= TREESTRSIZE)
               error(MSG_ERROR_GETCID_LONG_CID, s, TREESTRSIZE - 1);
          cid[i - i0] = s[i];
          i++;
     }
     cid[i - i0] = '\0';
     if (i > 0)
          while ((s[i] != '\0') && (s[i] == ' '))
               i++;
     return i;
}

/*
 * get the first interval [Min,Max] in <s> and put bounds in <min> and <max>.
 * return IC type (NONE if no interval found).
 * if an interval was found, return in <ai> the non-blanck position after
 * the interval in <s>.
 * note : it does not perform interval checking. This should be done after the
 * C type has been determined.
 */
int getInterval(char * s, char * min, char * max, int * ai)
{
     int i, a0, a1, b0, b1;
     char a, b;

     min[0] = '\0';
     max[0] = '\0';
     * ai = 0;

     i = 0;
     while ((s[i] != '\0') && (s[i]==' '))
          i++;
     if (s[i] == '\0')
          return NONE;

     /* seek left bracket */
     while ((s[i] != '\0') && (s[i] != '[') && (s[i] != ']'))
          i++;
     a0 = i + 1;
     a = s[i];
     if ((a == '\0') || (s[a0] == '\0'))
          return NONE;

     /* seek comma */
     while ((s[i] != '\0') && (s[i] != ','))
          i++;
     a1 = i - 1;
     b0 = i + 1;
     if ((s[i] == '\0') || (a1 < a0) || (s[b0] == '\0'))
          return NONE;

     /* seek right bracket */
     while ((s[i] != '\0') && (s[i] != ']') && (s[i] != '['))
          i++;
     b1 = i + 1;
     b = s[i];

     if (b == '\0')
          return NONE;

     if (a1 - a0 + 1 >= TREESTRSIZE)
          error(MSG_ERROR_LEFT_BOUND, s, TREESTRSIZE - 1);
     for (i = a0; i <= a1; i++)
          min[i - a0] = s[i];
     min[i - a0] = '\0';
     removespaces(min);

     if (b1 - 1 - b0 >= TREESTRSIZE)
          error(MSG_ERROR_RIGHT_BOUND, s, TREESTRSIZE - 1);
     for (i = b0; i <= b1 - 2; i++)
          max[i - b0] = s[i];
     max[i - b0] = '\0';
     removespaces(max);

     * ai = b1;
     while (s[*ai] == ' ')
          (* ai)++;

     if (a == '[')
     {
          if (b == ']')
               return CLOSED;
          else
               return MAX_EXCLUDED;
     }
     else
     {
          if (b == ']')
               return MIN_EXCLUDED;
          else
               return OPEN;
     }
}

/*
 * return 1 if string <s> is a valid C identifier, 0 elsewhere
 */
int IsStringCid(char * s)
{
     char c;
     int i;

     /* first char must be a letter */
     c = s[0];
     if (((c < 'A') || (c > 'Z')) && ((c < 'a') || (c > 'z')) && (c != '_'))
          return 0;

     /* check other char */
     i = 1;
     while ((c = s[i++]) != '\0')
          if (!IS_CID(c))
               return(0);
     return 1;
}

/*
 * write in file <fd> the text given, following the syntax of fprintf,
 * and do needed conversion for inclusion in a LaTeX source (T-file : .doc).
 */
/* TODO: rely on external LaTeX generators (pandoc?) */
void fprinttex(FILE * fd, char * fmt, ...)
{
     va_list marker;
     int longflg;
     int fmtflg;
     longflg = 0;
     fmtflg  = 0;
     va_start(marker, fmt);
     while(* fmt != 0)
     {
          if ((* fmt != '%') && (fmtflg == 0))
               fputc(* fmt++, fd);
          else
          {
               char c;
               char * s;
               fmtflg = 1;

               switch(*++fmt) {
               case '%':
                    /* FIXME: what shall we do? */
                    /* fprintf(fd, "\\%", fd); */
                    fprintf(fd, "\\%%");
                    longflg = 0;
                    fmtflg  = 0;
                    break;
               case 'c':
                    /*
                     * need a cast here since va_arg only
                     * takes fully promoted types
                     */
                    c = (char) va_arg(marker, int);
                    fprintf(fd, "%c", c);
                    longflg = 0;
                    fmtflg  = 0;
                    break;
               case 's':
                    fprintf(fd, "%s", va_arg(marker, char *));
                    longflg = 0;
                    fmtflg = 0;
                    break;
               case 'T':
                    for (s = va_arg(marker, char *); * s != '\0'; s++)
                    {
                         int escflg = 0;
                         switch(* s)
                         {
                         case '$':
                         case '%':
                         case '{':
                         case '}':
                         case '_':
                         case '&':
                         case '#':
                              if (escflg)
                              {
                                   fprintf(fd, "\\verb!\\!");
                                   escflg = 0;
                              }
                              fprintf(fd, "\\%c", * s);
                              break;
                         case '\\' :
                              if (escflg)
                              {
                                   fprintf(fd, "\\verb!\\!");
                                   escflg = 0;
                              }
                              else
                                   escflg = 1;
                              break;
                         case '^':
                              /*
                               * \verb!^! does not work
                               * inside a LaTeX 2e macro !
                               */
                              if (escflg)
                              {
                                   fprintf(fd, "\\verb!\\!");
                                   escflg = 0;
                              }
                              fprintf(fd, "$\\mathbf{\\hat{}}\\;$");
                              break;
                         case '~':
                         case '|':
                         case '<':
                         case '>':
                              if (escflg)
                              {
                                   fprintf(fd, "\\verb!\\!");
                                   escflg = 0;
                              }
                              fprintf(fd, "$\\mathbf{%c}\\;$", * s);
                              break;
                         case 'a':
                         case 'b':
                         case 'f':
                         case 'n':
                         case 'r':
                         case 't':
                         case 'v':
                         case '?':
                         case '\'':
                         case '"':
                              if (escflg) {
                                   switch (* s) {
                                   case 'a':
                                   case 'b':
                                   case 'f':
                                   case 't':
                                   case 'v':
                                        break;
                                   case 'n':
                                   case 'r':
                                        fprintf(fd, "\\newline\n");
                                        break;
                                   case '?':
                                   case '\'':
                                   case '"':
                                        putc(* s, fd);
                                        break;
                                   }
                                   escflg = 0;
                              }
                              else
                                   putc(* s, fd);
                              break;
                         default:
                              if (escflg) {
                                   fprintf(fd, "\\verb!\\!");
                                   escflg = 0;
                              }
                              putc(* s, fd);
                              break;
                         }
                    }
                    longflg = 0;
                    fmtflg  = 0;
                    break;
               case 'x':
                    if (longflg)
                         fprintf(fd, "%lx", va_arg(marker, unsigned long));
                    else
                         fprintf(fd, "%x", va_arg(marker, unsigned int));
                    longflg = 0;
                    fmtflg  = 0;
                    break;
               case 'X':
                    if (longflg)
                         fprintf(fd, "%lX", va_arg(marker, unsigned long));
                    else
                         fprintf(fd, "%X", va_arg(marker, unsigned int));
                    longflg = 0;
                    fmtflg  = 0;
                    break;
               case 'u':
                    if (longflg)
                         fprintf(fd, "%lu", va_arg(marker, unsigned long));
                    else
                         fprintf(fd, "%u", va_arg(marker, unsigned int));
                    longflg = 0;
                    fmtflg  = 0;
                    break;
               case 'd':
                    if (longflg)
                         fprintf(fd, "%ld", va_arg(marker, long));
                    else
                         fprintf(fd, "%d", va_arg(marker, int));
                    longflg = 0;
                    fmtflg  = 0;
                    break;
               case 'f':
                    if (longflg)
                         fprintf(fd, "%g", va_arg(marker, double));
                    else
                         /*
                          * need a cast here since va_arg only
                          * takes fully promoted types
                          */
                         fprintf(fd, "%g", (double)va_arg(marker, double));
                    longflg = 0;
                    fmtflg  = 0;
                    break;
               case 'l':
                    longflg = 1;
                    break;
               default:
                    fprintf(fd, "%%%c", * fmt);
                    longflg = 0;
                    fmtflg = 0;
                    break;
               }
               if(longflg == 0)
                    fmt++;
          }
     }
     va_end(marker);
}

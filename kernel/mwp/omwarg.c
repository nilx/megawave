/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include "bintree.h"
#include "symbol.h"
#include "token.h"
#include "mwarg.h"
#include "data_io.h"
#include "y.tab.h"
#include "io.h"
#define OMWARG_DEC
#include "omwarg.h"


#ifdef __STDC__
extern void printaccess_optvar(FILE *, Valtype, Io, Node *, char *);
extern void printaccess_optptr(FILE *, Valtype, Io, Node *, char *);
#else
extern printaccess_optvar();
extern printaccess_optptr();
#endif

#ifdef __STDC__
static short lookup_string(Header *h, char *s)
#else
static short lookup_string(h, s)
Header *h;
char *s;
#endif
{
  if (GET_NUMBER(h) !=  0) {
    Cell *c;
    for (c=GET_FIRST(h); c!=NULL; c=GET_NEXT(c)) {
      if (!strcmp((char *)GET_ELT(c), s))
        return TRUE;
    }
  }
  return FALSE;
}

#ifdef __STDC__
static void make_include_list(Header *h, Cell *c)
#else
static make_include_list(h, c)
Header *h;
Cell *c;
#endif
{
  Mwarg *opt;

  opt = (Mwarg *)GET_ELT(c);
  if (opt->iodesc != NULL && opt->iodesc->include != NULL) {
    char *include;
    include = opt->iodesc->include;
    if (!lookup_string(h, include)) {
      if (GET_NUMBER(h) == 0)
        (void) addcell(h, NULL, newcell(include));
      else
        (void) addcell(h, GET_LAST(h), newcell(include));
    }
  }
}

#ifdef __STDC__
void print_io_include(FILE *fd)
#else
print_io_include(fd)
FILE *fd;
#endif
{
  Header h;
  Cell *c;

  SET_FIRST(&h, NULL);
  SET_NUMBER(&h, 0);

  for (c=GET_FIRST(optionlist); c!=NULL; c=GET_NEXT(c))
    make_include_list(&h, c);

  for (c=GET_FIRST(neededarglist); c!=NULL; c=GET_NEXT(c))
    make_include_list(&h, c);

  for (c=GET_FIRST(vararglist); c!=NULL; c=GET_NEXT(c))
    make_include_list(&h, c);

  for (c=GET_FIRST(optarglist); c!=NULL; c=GET_NEXT(c))
    make_include_list(&h, c);

  for (c=GET_FIRST(&h); c!=NULL; c=GET_NEXT(c))
    fprintf(fd, "#include \"%s\"\n", (char *)GET_ELT(c));
}

#ifdef __STDC__
static void make_function_list(Header *h, Cell *c)
#else
static make_function_list(h, c)
Header *h;
Cell *c;
#endif
{
  Mwarg *opt;

  opt = (Mwarg *)GET_ELT(c);
  if (opt->iodesc != NULL && opt->iodesc->include != NULL) {
    char *function;
    function = opt->iodesc->function;
    if (!lookup_string(h, function) && opt->iodesc->t == MW2_T) {
      if (GET_NUMBER(h) == 0)
        (void) addcell(h, NULL, newcell(function));
      else
        (void) addcell(h, GET_LAST(h), newcell(function));
    }
  }
}

#ifdef __STDC__
void print_io_function(FILE *fd)
#else
print_io_function(fd)
FILE *fd;
#endif
{
  Header h;
  Cell *c;

  SET_FIRST(&h, NULL);
  SET_NUMBER(&h, 0);

  for (c=GET_FIRST(optionlist); c!=NULL; c=GET_NEXT(c))
    make_function_list(&h, c);

  for (c=GET_FIRST(neededarglist); c!=NULL; c=GET_NEXT(c))
    make_function_list(&h, c);

  for (c=GET_FIRST(vararglist); c!=NULL; c=GET_NEXT(c))
    make_function_list(&h, c);

  for (c=GET_FIRST(optarglist); c!=NULL; c=GET_NEXT(c))
    make_function_list(&h, c);

  for (c=GET_FIRST(&h); c!=NULL; c=GET_NEXT(c))
    fprintf(fd, "  extern short %s() ;\n", (char *)GET_ELT(c));
}

#ifdef __STDC__
print_init_io_var(FILE *fd, Mwarg *opt)
#else
print_init_io_var(fd, opt)
FILE *fd;
Mwarg *opt;
#endif
{
  Desc *d;

  switch(opt->t) {
    case OPTION :
      d = &(opt->d.o.d);
      break;
    case NEEDEDARG :
    case VARARG :
      d = NULL;
      break;
    case OPTIONARG :
      d = &(opt->d.a);
      break;
    case NOTUSEDARG :
      d = NULL;
      break;
    default :
      INT_ERROR("print_init_io_var");
      break;
  }

  if (d != NULL) {
    switch(d->t) {
      case FILEARG :
        fprintf(fd, " = (");
        printnode(fd, opt->type);
        if (d->rw == WRITE && opt->access != NULL)
          printnode(fd, opt->access->left);
        else
          printnode(fd, opt->access);
        fprintf(fd, ")NULL ;\n");
        break;
      case SCALARARG :
        fprintf(fd, " = (");
        printnode(fd, opt->type); printnode(fd, opt->access->left); 
        fprintf(fd, ")");
        switch(d->v.p.t) {
          case QSTRING_T :
            if (d->v.p.d != NULL)
              /* fprintf(fd, "%s ;\n", d->v.p.d->q);
		 Modified by JF 14/04/2000. Try to manage QSTRING_T as CHAR_T, 
		 since strings are not allowed (confusion with char *).
	       */
              fprintf(fd, "'%s' ;\n", d->v.p.d->q);
            else
              fprintf(fd, "NULL ;\n");
            break;
          case CHAR_T :
            if (d->v.p.d != NULL)
              switch (d->v.p.d->c) {
                case '\0' :
                  fprintf(fd, "'\\0' ;\n");
                  break;
                case '\a' :
                  fprintf(fd, "'\\a' ;\n");
                  break;
                case '\b' :
                  fprintf(fd, "'\\b' ;\n");
                  break;
                case '\f' :
                  fprintf(fd, "'\\f' ;\n");
                  break;
                case '\n' :
                  fprintf(fd, "'\\n' ;\n");
                  break;
                case '\r' :
                  fprintf(fd, "'\\r' ;\n");
                  break;
                case '\v' :
                  fprintf(fd, "'\\v' ;\n");
                  break;
                default :
                  if (isprint(d->v.p.d->c))
                    fprintf(fd, "'%c' ;\n", d->v.p.d->c);
                  else
                    fprintf(fd, "'\\x%x' ;\n", d->v.p.d->c);
                  break;
              }
            else
              fprintf(fd, "'\\0' ;\n");
            break;
          case UCHAR_T:
            if (d->v.p.d != NULL)
              switch (d->v.p.d->c) {
                case '\0' :
                  fprintf(fd, "'\\0' ;\n");
                  break;
                case '\a' :
                  fprintf(fd, "'\\a' ;\n");
                  break;
                case '\b' :
                  fprintf(fd, "'\\b' ;\n");
                  break;
                case '\f' :
                  fprintf(fd, "'\\f' ;\n");
                  break;
                case '\n' :
                  fprintf(fd, "'\\n' ;\n");
                  break;
                case '\r' :
                  fprintf(fd, "'\\r' ;\n");
                  break;
                case '\v' :
                  fprintf(fd, "'\\v' ;\n");
                  break;
                default :
                  if (!(d->v.p.d->uc & 0x80) && isprint((char)d->v.p.d->uc))
                    fprintf(fd, "'%c' ;\n", (char)d->v.p.d->uc);
                  else
                    fprintf(fd, "0x%x' ;\n", d->v.p.d->uc);
                  break;
              }
            else
              fprintf(fd, "0x0 ;\n");
            break;
          case SHORT_T :
            if (d->v.p.d != NULL)
              fprintf(fd, "%hd ;\n", d->v.p.d->s);
            else
              fprintf(fd, "0 ;\n");
            break;
          case USHORT_T :
            if (d->v.p.d != NULL)
              fprintf(fd, "0x%hx ;\n", d->v.p.d->us);
            else
              fprintf(fd, "0x0 ;\n");
            break;
          case INT_T :
            if (d->v.p.d != NULL)
              fprintf(fd, "%d ;\n", d->v.p.d->i);
            else
              fprintf(fd, "0 ;\n");
            break;
          case UINT_T :
            if (d->v.p.d != NULL)
              fprintf(fd, "0x%x ;\n", d->v.p.d->ui);
            else
              fprintf(fd, "0x0 ;\n");
            break;
          case LONG_T :
            if (d->v.p.d != NULL)
              fprintf(fd, "%ld ;\n", d->v.p.d->l);
            else
              fprintf(fd, "0 ;\n");
            break;
          case ULONG_T :
            if (d->v.p.d != NULL)
              fprintf(fd, "0x%lx ;\n", d->v.p.d->ul);
            else
              fprintf(fd, "0x0 ;\n");
            break;
          case FLOAT_T :
            if (d->v.p.d != NULL)
              fprintf(fd, "%g ;\n", d->v.p.d->f);
            else
              fprintf(fd, "0.0 ;\n");
            break;
          case DOUBLE_T :
            if (d->v.p.d != NULL)
              fprintf(fd, "%g ;\n", d->v.p.d->d);
            else
              fprintf(fd, "0.0 ;\n");
            break;
          default :
#ifdef DEBUG
            PRDBG("print_init_io_var error : d->t = %M\n", d->t);
#endif
            INT_ERROR("print_init_io_var");
            break;
        }
        break;
      case FLAGARG :
#ifdef DEBUG
        PRDBG("print_init_io_var : FLAGARG\n");
#endif
        fprintf(fd, " = FALSE ;\n");
        break;
      default :
        INT_ERROR("print_init_io_var");
        break;
    }
  }
  else
    fprintf(fd, " = NULL ;\n");
}

#ifdef __STDC__
void print_is_range(FILE *fd, Io rw, Paramtype *p, DataIo *io, char *s)
#else
print_is_range(fd, rw, p, io, s)
FILE *fd;
short rw;
Paramtype *p;
DataIo *io;
char *s;
#endif
{
  char *t_m, *t_M;
  switch (p->i->t) {
    case CLOSED :
      t_m = "<"; t_M = "<";
      break;
    case MAX_EXCLUDED :
      t_m = "<"; t_M = "<=";
      break;
    case MIN_EXCLUDED :
      t_m = "<="; t_M = "<";
      break;
    case OPEN :
      t_m = "<="; t_M = "<=";
      break;
    default :
      INT_ERROR("print_is_range");
      break;
  }

  if (rw!=READ && rw!=WRITE)
    INT_ERROR("print_is_range");

  switch (p->t) {
    case QSTRING_T :
      /*
      INT_ERROR("print_is_range");
      Modified by JF 15/04/2000. Try to manage QSTRING_T as CHAR_T, 
      since strings are not allowed (confusion with char *).
      */
      fprintf(fd, "        if (%s(%s) %s '%c' || '%c' %s %s(%s)) {\n",
                          io->function, s, t_m, *p->i->min.q,
                          *p->i->max.q, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"'%%c' is out of range\", %s(%s)) ;\n",
                          io->function, s);
      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }
      fprintf(fd, "        }\n");
      break;

    case CHAR_T :
      fprintf(fd, "        if (%s(%s) %s '%c' || '%c' %s %s(%s)) {\n",
                          io->function, s, t_m, p->i->min.c,
                          p->i->max.c, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"'%%c' is out of range\", %s(%s)) ;\n",
                          io->function, s);
      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }
      fprintf(fd, "        }\n");
      break;


    case UCHAR_T :
      /* Changed JF 03/10/2000 : added cast to corresponding type */
      fprintf(fd, "        if (%s(%s) %s (unsigned char) 0x%2.2X || (unsigned char) 0x%2.2X %s %s(%s)) {\n",
                          io->function, s, t_m, p->i->min.uc,
                          p->i->max.uc, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"0x%%2.2X is out of range\", %s(%s)) ;\n",
                          io->function, s);
      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }
      fprintf(fd, "        }\n");
      break;
    case SHORT_T :
      /* Changed JF 03/10/2000 : added cast to corresponding type */
      fprintf(fd, "        if (%s(%s) %s (short) %hd || (short) %hd %s %s(%s)) {\n",
                          io->function, s, t_m, p->i->min.s,
                          p->i->max.s, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"%%hd is out of range\", %s(%s)) ;\n",
                          io->function, s);
      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }
      fprintf(fd, "        }\n");
      break;
    case USHORT_T :
      /* Changed JF 03/10/2000 : added cast to corresponding type */
      fprintf(fd, "        if (0x%hX %s %s(%s) %s (unsigned short) 0x%hX || (unsigned short) 0x%hX %s %s(%s)) {\n",
                          io->function, s, t_m, p->i->min.us,
                          p->i->max.us, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"0x%%hX is out of range\", %s(%s)) ;\n",
                          io->function, s);
      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }
      fprintf(fd, "        }\n");
      break;
    case INT_T :
      /* Changed JF 03/10/2000 : added cast to corresponding type */
      fprintf(fd, "        if (%s(%s) %s (int) %d || (int) %d %s %s(%s)) {\n",
                          io->function, s, t_m, p->i->min.i,
                          p->i->max.i, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"%%d is out of range\", %s(%s)) ;\n",
                          io->function, s);
      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }
      fprintf(fd, "        }\n");
      break;
    case UINT_T :
      /* Changed JF 03/10/2000 : added cast to corresponding type */
      fprintf(fd, "        if (%s(%s) %s (unsigned int) 0x%X || (unsigned int) 0x%X %s %s(%s)) {\n",
                          io->function, s, t_m, p->i->min.ui,
                          p->i->max.ui, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"0x%%X is out of range\", %s(%s)) ;\n",
                          io->function, s);
      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }
      fprintf(fd, "        }\n");
      break;
    case LONG_T :
      /* Changed JF 03/10/2000 : added cast to corresponding type */
      fprintf(fd, "        if (%s(%s) %s (long) %ld || (long) %ld %s %s(%s)) {\n",
                          io->function, s, t_m, p->i->min.l,
                          p->i->max.l, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"%%ld is out of range\", %s(%s)) ;\n",
                          io->function, s);
      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }
      fprintf(fd, "        }\n");
      break;
    case ULONG_T :
      /* Changed JF 03/10/2000 : added cast to corresponding type */
      fprintf(fd, "        if (%s(%s) %s (unsigned long) 0x%lX || (unsigned long) 0x%lX %s %s(%s)) {\n",
                          io->function, s, t_m, p->i->min.ul,
                          p->i->max.ul, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"0x%%lX is out of range\", %s(%s)) ;\n",
                          io->function, s);
      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }
      fprintf(fd, "        }\n");
      break;
    case FLOAT_T :
      /* Changed JF 03/10/2000 : added cast to corresponding type */
      fprintf(fd, "        if (%s(%s) %s (float) %g || (float) %g %s %s(%s)) {\n",
                          io->function, s, t_m, (double)p->i->min.f,
                          (double)p->i->max.f, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"%%g is out of range\", (double)%s(%s)) ;\n",
                          io->function, s);

      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }

      fprintf(fd, "        }\n");
      break;
    case DOUBLE_T :
      /* Changed JF 03/10/2000 : added cast to corresponding type */
      fprintf(fd, "        if (%s(%s) %s (double) %g || (double) %g %s %s(%s)) {\n",
                          io->function, s, t_m, p->i->min.d,
                          p->i->max.d, t_M, io->function, s);
      switch(rw) {
        case READ :
      fprintf(fd, "          char buffer[BUFSIZ] ;\n");
      fprintf(fd, "          sprintf(buffer, \"%%g is out of range\", %s(%s)) ;\n",
                          io->function, s);
      fprintf(fd, "          mwusage(buffer) ;\n");
          break;
        case WRITE :
      fprintf(fd, "          printf(\"'%%s' is out of range\", %s(%s)) ;\n",
                                                                       io->function, s);
          break;
      }
      fprintf(fd, "        }\n");
      break;
    default :
      INT_ERROR("print_is_range");
      break;
  }
}

#ifdef __STDC__
void print_verify_io_arg(FILE *fd)
#else
print_verify_io_arg(fd)
FILE *fd;
#endif
{
  Cell *c;
  int n;
#ifdef __STDC__
  void print_is_range(FILE *, Io, Paramtype *, DataIo *, char *);
#else
  int print_is_range();
#endif

  if (GET_NUMBER(optionlist) != 0) {
    fprintf(fd, "  /* Verify io option list */\n");
    fprintf(fd, "  _mwoptind = 1 ;\n");
    fprintf(fd, "  while ((%sc=_mwgetopt(argc, argv, \"%s\")) != -1) {\n", MW_PFX2, optb);
    fprintf(fd, "    switch (%sc) {\n", MW_PFX2);
    for (c=GET_FIRST(optionlist); c!=NULL; c=GET_NEXT(c)) {
      Mwarg *opt;
      opt = GET_ELT(c);
      fprintf(fd, "      case '%c' :\n", opt->d.o.o);
      switch(opt->d.o.d.t) {
        case FILEARG :
          if (opt->d.o.d.rw == READ) {
            fprintf(fd, "        if (!_mwis_readable(_mwoptarg)) {\n");
            fprintf(fd, "          char buffer[BUFSIZ] ;\n");
            fprintf(fd, "          sprintf(buffer, \"cannot find '%%s' in default path\", _mwoptarg) ;\n");
            fprintf(fd, "          mwusage(buffer) ;\n");
            fprintf(fd, "        }\n");
          }
          else if (opt->d.o.d.rw == WRITE) {
            fprintf(fd, "        if (!_mwis_writable(_mwoptarg)) {\n");
            fprintf(fd, "          char buffer[BUFSIZ] ;\n");
            fprintf(fd, "          sprintf(buffer, \"cannot write '%%s'\", _mwoptarg) ;\n");
            fprintf(fd, "          mwusage(buffer) ;\n");
            fprintf(fd, "        }\n");
          }
          break;
        case SCALARARG :
          if (opt->d.o.d.rw == READ || opt->d.o.d.rw == WRITE) {
            if (opt->d.o.d.v.p.i != NULL)
              print_is_range(fd, opt->d.o.d.rw, &(opt->d.o.d.v.p), opt->iodesc, "_mwoptarg");
          }
          break;
        case FLAGARG :
#ifdef DEBUG
          PRDBG("print_verify_io_arg : FLAGARG\n");
#endif
          break;
        default :
          INT_ERROR("print_verify_io_arg");
          break;
      }
      fprintf(fd, "        break ;\n");
    }
    /* Default : unknown option call usage function */
    fprintf(fd, "      case '?' :\n");
    fprintf(fd, "        mwusage(NULL);\n");
    fprintf(fd, "        break;\n");
    fprintf(fd, "      default :\n");
    fprintf(fd, "        break;\n");
    fprintf(fd, "    }\n");
    fprintf(fd, "  }\n\n");
  }

  n = 0;
  fprintf(fd, "  /* Verify io needed arg list */\n");
  for (c=GET_FIRST(neededarglist); c!=NULL; c=GET_NEXT(c)) {
    Mwarg *opt;
    opt = GET_ELT(c);
    switch(opt->d.a.t) {
      case FILEARG :
        if (opt->d.a.rw == READ) {
          fprintf(fd, "  if (_mwoptind+%d<argc) {\n", n);
          fprintf(fd, "    if (!_mwis_readable(argv[_mwoptind+%d])) {\n", n);
          fprintf(fd, "      char buffer[BUFSIZ] ;\n");
          fprintf(fd, "      sprintf(buffer, \"cannot find '%%s' in default path\",  argv[_mwoptind+%d]) ;\n", n);
          fprintf(fd, "      mwusage(buffer) ;\n");
          fprintf(fd, "    }\n");
          fprintf(fd, "  }\n");
          fprintf(fd, "  else\n");
          fprintf(fd, "    mwusage(\"missing '%s'\") ;\n", opt->texname);
          n++;
        }
        else if (opt->d.a.rw == WRITE) {
          fprintf(fd, "  if (_mwoptind+%d<argc) {\n", n);
          fprintf(fd, "    if (!_mwis_writable(argv[_mwoptind+%d])) {\n", n);
          fprintf(fd, "      char buffer[BUFSIZ] ;\n");
          fprintf(fd, "      sprintf(buffer, \"cannot write '%%s'\", argv[_mwoptind+%d]) ;\n", n);
          fprintf(fd, "      mwusage(buffer) ;\n");
          fprintf(fd, "    }\n");
          fprintf(fd, "  }\n");
          fprintf(fd, "  else\n");
          fprintf(fd, "    mwusage(\"missing '%s'\") ;\n", opt->texname);
          n++;
        }
        break;
      case SCALARARG :
        if (opt->d.a.rw == READ) {
          if (opt->d.a.v.p.i != NULL) {
            char buffer[BUFSIZ];
            sprintf(buffer, "argv[_mwoptind+%d]", n);
            fprintf(fd, "  if (_mwoptind+%d<argc) {\n", n);
            print_is_range(fd, opt->d.a.rw, &(opt->d.a.v.p), opt->iodesc, 
                                                                       buffer);
            fprintf(fd, "  }\n");
            fprintf(fd, "  else\n");
            fprintf(fd, "    mwusage(\"missing '%s'\") ;\n", opt->texname);
          }
          else {
            fprintf(fd, "  if (_mwoptind+%d>=argc) {\n", n);
            fprintf(fd, "    char buffer[BUFSIZ] ;\n");
            fprintf(fd, "    sprintf(buffer, \"missing '%s'\") ;\n", 
                                                                 opt->texname);
            fprintf(fd, "    mwusage(buffer) ;\n");
            fprintf(fd, "  }\n");  

          }
          n++;
        }
        break;
      case FLAGARG :
#ifdef DEBUG
        PRDBG("print_verify_io_arg : FLAGARG\n");
        PRDBG("Flag '-%s' is not allowed in needed argument list\n", opt->d.o.o);
#endif
        fatal_error("Flag '-%s' is not allowed in needed argument list\n", opt->d.o.o);
        break;
      default :
        INT_ERROR("print_verify_io_arg");
        break;
    }
  }

  if (GET_NUMBER(optarglist) != 0) {
    fprintf(fd, "  /* Verify io option arg list */\n");
    fprintf(fd, "  if (_mwoptind+%d < argc) {\n", n);
    for (c=GET_FIRST(optarglist); c!=NULL; c=GET_NEXT(c)) {
      Mwarg *opt;
      opt = GET_ELT(c);
      switch(opt->d.a.t) {
        case FILEARG :
          fprintf(fd, "    if (_mwoptind+%d < argc) {\n", n);
          if (opt->d.a.rw == READ) {
            fprintf(fd, "    if (!_mwis_readable(argv[_mwoptind+%d])) {\n", n);
            fprintf(fd, "      char buffer[BUFSIZ] ;\n");
            fprintf(fd, "      sprintf(buffer, \"cannot find '%%s' in default path\", argv[_mwoptind+%d]) ;\n", n);
            fprintf(fd, "      mwusage(buffer) ;\n");
            fprintf(fd, "    }\n");
            n++;
          }
          else if (opt->d.a.rw == WRITE) {
            fprintf(fd, "    if (!_mwis_writable(argv[_mwoptind+%d])) {\n", n);
            fprintf(fd, "      char buffer[BUFSIZ] ;\n");
            fprintf(fd, "      sprintf(buffer, \"cannot write '%%s'\", argv[_mwoptind+%d]) ;\n", n);
            fprintf(fd, "      mwusage(buffer) ;\n");
            fprintf(fd, "    }\n");
            n++;
          }
          else
            fprintf(fd, "    ;\n");
          fprintf(fd, "    }\n");
          fprintf(fd, "    else\n");
          fprintf(fd, "      mwusage(\"missing '%s'\") ;\n", opt->texname);
          break;
        case SCALARARG :
          if (opt->d.a.rw == READ) {
            fprintf(fd, "    if (_mwoptind+%d < argc) {\n", n);
            if (opt->d.a.v.p.i != NULL) {
              char buffer[BUFSIZ];
              sprintf(buffer, "argv[_mwoptind+%d]", n);
              print_is_range(fd, opt->d.a.rw, &(opt->d.a.v.p), opt->iodesc, buffer);
            }
            else
              fprintf(fd, "      ;\n");
            fprintf(fd, "    }\n");
            fprintf(fd, "    else\n");
            fprintf(fd, "      mwusage(\"missing '%s'\") ;\n", opt->texname);
            n++;
          }
          else
            fprintf(fd, "    ;\n");
          break;
        case FLAGARG :
#ifdef DEBUG
          PRDBG("print_verify_io_arg : FLAGARG\n");
          PRDBG("Flag '-%s' is not allowed in optional argument list\n", opt->d.o.o);
#endif
          fatal_error("Flag '-%s' is not allowed in optional argument list\n", opt->d.o.o);
          break;
        default :
          INT_ERROR("print_verify_io_arg");
          break;
      }
    }
    fprintf(fd, "  }\n");
  }

  if (GET_NUMBER(vararglist) != 0) {
    Mwarg *opt;
    fprintf(fd, "  /* Verify io variable arg list */\n");
    fprintf(fd, "  if (_mwoptind+%d < argc) {\n", n);
    fprintf(fd, "    for (%si = _mwoptind+%d; %si<argc; %si++) {\n", MW_PFX2, n, MW_PFX2, MW_PFX2);
    c=GET_FIRST(vararglist);
    opt = GET_ELT(c);
    switch(opt->d.a.t) {
      case FILEARG :
        if (opt->d.a.rw == READ) {
          fprintf(fd, "      if (!_mwis_readable(argv[%si])) {\n", MW_PFX2);
          fprintf(fd, "        char buffer[BUFSIZ] ;\n");
          fprintf(fd, "        sprintf(buffer, \"cannot find '%%s' in default path\", argv[%si]) ;\n", MW_PFX2);
          fprintf(fd, "        mwusage(buffer) ;\n");
          fprintf(fd, "      }\n");
        }
        else if (opt->d.a.rw == WRITE) {
          fprintf(fd, "      if (!_mwis_writable(argv[%si])) {\n", MW_PFX2);
          fprintf(fd, "        char buffer[BUFSIZ] ;\n");
          fprintf(fd, "        sprintf(buffer, \"cannot write '%%s'\", argv[%si]) ;\n", MW_PFX2);
          fprintf(fd, "        mwusage(buffer) ;\n");
          fprintf(fd, "      }\n");
        }
        else
          fprintf(fd, "      ;\n");
        break;
      case SCALARARG :
        if (opt->d.a.rw == READ) {
          if (opt->d.a.v.p.i != NULL) {
            char buffer[BUFSIZ];
            sprintf(buffer, "argv[%si]", MW_PFX2);
            print_is_range(fd, opt->d.a.rw, &(opt->d.a.v.p), opt->iodesc, buffer);
          }
          else
            fprintf(fd, "      ;\n");
        }
        else
          fprintf(fd, "      ;\n");
        break;
      case FLAGARG :
#ifdef DEBUG
        PRDBG("print_verify_io_arg : FLAGARG\n");
        PRDBG("Flag '-%s' is not allowed in variable argument\n", opt->d.o.o);
#endif
        fatal_error("Flag '-%s' is not allowed in variable argument\n", opt->d.o.o);
        break;
      default :
        INT_ERROR("print_verify_io_arg");
        break;
    }
    fprintf(fd, "    }\n");
    fprintf(fd, "  }\n");
  }
}

#ifdef __STDC__
void print_optdecl(FILE *fd, Mwarg *opt)
#else
void print_optdecl(fd, opt)
Mwarg *opt;
FILE *fd;
#endif
{
  Desc *d;

  switch(opt->t) {
    case OPTION :
      d = &opt->d.o.d;
      break;
    case VARARG:
    case OPTIONARG:
    case NEEDEDARG:
      d = &opt->d.a;
      break;
    default :
      INT_ERROR("print_optdecl");
      break;
  }
  switch(d->t) {
    case FILEARG :
      fprintf(fd, "  "); printnode(fd, opt->type);
      printaccess_optvar(fd, d->t, d->rw, opt->access, opt->name);
      print_init_io_var(fd, opt);
      break;
    case SCALARARG :
      fprintf(fd, "  "); printnode(fd, opt->type);
      printaccess_optvar(fd, d->t, d->rw, opt->access, opt->name);
      print_init_io_var(fd, opt);
      fprintf(fd, "  "); printnode(fd, opt->type);
      printaccess_optptr(fd, d->t, d->rw, opt->access, opt->name);
      if (d->v.p.d != NULL)
        fprintf(fd, " = &%s%s ;\n", MW_PFX, opt->name);
      else
        fprintf(fd, " = NULL ;\n");
      break;
    case FLAGARG :
      fprintf(fd, "  "); printnode(fd, opt->type);
      printaccess_optvar(fd, d->t, d->rw, opt->access, opt->name);
      print_init_io_var(fd, opt);
      fprintf(fd, "  "); printnode(fd, opt->type);
      printaccess_optptr(fd, d->t, d->rw, opt->access, opt->name);
      fprintf(fd, " = NULL ;\n");
      break;
    default :
      INT_ERROR("printdecl");
      break;
  }
}

/* Written by jf 29/9/98 
   Used in genmain.c and genxmain.c to write variable declarations 
   (needed or not used arguments) instead of the former " = NULL "
*/

#ifdef __STDC__
void print_ndecl(FILE *fd, Mwarg *opt)
#else
void print_ndecl(fd, opt)
Mwarg *opt;
FILE *fd;
#endif
{
  fprintf(fd, " = (");
  printnode(fd, opt->type); /* print type of variable */
  fprintf(fd, ") ");  
  switch (opt->type->name) 
    {
    case FLOAT : case DOUBLE : case REAL :
      fprintf(fd,"0.0;\n");
      break;
    case INTEGER : case LONG : case SHORT :
      fprintf(fd,"0;\n");
      break;
    default :
      fprintf(fd,"NULL;\n");
      break;      
    }
}

#ifdef __STDC__
void printaccess_optvar(FILE *fd, Valtype t, Io rw, Node *n, char *id)
#else
printaccess_optvar(fd, t, rw, n, id)
FILE *fd;
short t;
short rw;
Node *n;
char *id;
#endif
{
#ifdef DEBUG
  char buffer1[BUFSIZ], buffer2[BUFSIZ];
  sprintf(buffer1, "printaccess_optvar(0x%lx, ", (unsigned long)fd);
  switch(t) {
    case FILEARG :
      strcat(buffer1, "FILEARG");
      break;
    case SCALARARG :
      strcat(buffer1, "SCALARARG");
      break;
    case FLAGARG :
      strcat(buffer1, "FLAGARG");
      break;
    default :
      strcat(buffer1, "<UNKNOWN>");
      break;
  }
  strcat(buffer1, ", ");
  switch(rw) {
    case READ :
      strcat(buffer1, "READ");
      break;
    case WRITE :
      strcat(buffer1, "WRITE");
      break;
    default :
      strcat(buffer1, "<UNKNOWN>");
      break;
  }
  sprintf(buffer2, ", 0x%lx, \"%s\")\n", (unsigned long)n, id);
  strcat(buffer1, buffer2);
  PRDBG(buffer1);
#endif
  if (n != NULL) {
    Node *i;
    for (i = n; i->left != NULL; i = i->left)
      ;
    switch (t) {
      case FILEARG :
        switch(rw) {
          case READ :
            {
              char buffer[BUFSIZ];
              Node *mw_nid;
              sprintf(buffer, "%s", id);
              mw_nid = mkleaf(NAME, NULL, 0, buffer);
              i->left = mw_nid;
              printnode(fd, n->left);
              free(mw_nid);
            }
            break;
          case WRITE :
            {
              char buffer[BUFSIZ];
              Node *mw_nid;
              sprintf(buffer, "%s", id);
              mw_nid = mkleaf(NAME, NULL, 0, buffer);
              i->left = mw_nid;
              printnode(fd, n->left);
              free(mw_nid);
            }
            break;
          default :
#ifdef DEBUG
            PRDBG("printaccess_optvar : %d unknown Io\n", (int)rw);
#endif
            INT_ERROR("printaccess_optvar");
            break;
        }
        break;
      case SCALARARG :
        switch(rw) {
          case READ :
            if (IS_PTR(n)) {
              char buffer[BUFSIZ];
              Node *mw_nid;
              sprintf(buffer, "%s%s", MW_PFX, id);
              mw_nid = mkleaf(NAME, NULL, 0, buffer);
              i->left = mw_nid;
              printnode(fd, n->left);
              free(mw_nid);
            }
            else
              fatal_error("%s : must be a pointer\n", id);
            break;
          case WRITE :
            if (IS_PTR(n)) {
              char buffer[BUFSIZ];
              Node *mw_nid;
              sprintf(buffer, "%s%s", MW_PFX, id);
              mw_nid = mkleaf(NAME, NULL, 0, buffer);
              i->left = mw_nid;
              printnode(fd, n->left);
              free(mw_nid);
            }
            else
              fatal_error("%s : must be a pointer\n", id);
            break;
          default :
#ifdef DEBUG
            PRDBG("printaccess_optvar : %d unknown Io\n", (int)rw);
#endif
            INT_ERROR("printaccess_optvar");
            break;
        }
        break;
      case FLAGARG :
        if (IS_PTR(n)) {
          char buffer[BUFSIZ];
          Node *mw_nid;
          sprintf(buffer, "%s%s", MW_PFX, id);
          mw_nid = mkleaf(NAME, NULL, 0, buffer);
          i->left = mw_nid;
          printnode(fd, n->left);
          free(mw_nid);
        }
        else
          fatal_error("%s : must be a pointer\n", id);
        break;
      default :
        INT_ERROR("printaccess_optvar");
        break;
    }
    i->left = (Node *) NULL;
  }
  else if (t != FILEARG) {
/*
    fprintf(fd ,"%s%s", MW_PFX, id);
*/
    fatal_error("%s : must be a pointer\n", id);
  }
  else
    fprintf(fd ,"%s", id);
}

#ifdef __STDC__
void printaccess_optptr(FILE *fd, Valtype t, Io rw, Node *n, char *id)
#else
printaccess_optptr(fd, t, rw, n, id)
FILE *fd;
short t;
short rw;
Node *n;
char *id;
#endif
{
#ifdef DEBUG
  char buffer1[BUFSIZ], buffer2[BUFSIZ];
  sprintf(buffer1, "printaccess_optptr(0x%lx, ", (unsigned long)fd);
  switch(t) {
    case FILEARG :
      strcat(buffer1, "FILEARG");
      break;
    case SCALARARG :
      strcat(buffer1, "SCALARARG");
      break;
    case FLAGARG :
      strcat(buffer1, "FLAGARG");
      break;
    default :
      strcat(buffer1, "<UNKNOWN>");
      break;
  }
  strcat(buffer1, ", ");
  switch(rw) {
    case READ :
      strcat(buffer1, "READ");
      break;
    case WRITE :
      strcat(buffer1, "WRITE");
      break;
    default :
      strcat(buffer1, "<UNKNOWN>");
      break;
  }
  sprintf(buffer2, ", 0x%lx, \"%s\")\n", (unsigned long)n, id);
  strcat(buffer1, buffer2);
  PRDBG(buffer1);
#endif
  if (n != NULL) {
    Node *i, *nid;
    for (i = n; i->left != NULL; i = i->left)
      ;
    nid = mkleaf(NAME, NULL, 0, id);
    switch (t) {
      case FILEARG :
        switch(rw) {
          case READ :
            {
              i->left = nid;
              printnode(fd, n);
            }
            break;
          case WRITE :
            {
              i->left = nid;
              printnode(fd, n);
            }
            break;
          default :
#ifdef DEBUG
            PRDBG("printaccess_optptr : %d unknown Io\n", (int)rw);
#endif
            INT_ERROR("printaccess_optptr");
            break;
        }
        break;
      case SCALARARG :
        switch(rw) {
          case READ :
            if (IS_PTR(n)) {
              i->left = nid;
              printnode(fd, n);
            }
            else
              error("%s : must be a pointer\n", id);
            break;
          case WRITE :
            if (IS_PTR(n)) {
              i->left = nid;
              printnode(fd, n);
            }
            else
              error("%s : must be a pointer\n", id);
            break;
          default :
#ifdef DEBUG
            PRDBG("printaccess_optptr : %d unknown Io\n", (int)rw);
#endif
            INT_ERROR("printaccess_optptr");
            break;
        }
        break;
      case FLAGARG :
        if (IS_PTR(n)) {
          i->left = nid;
          printnode(fd, n);
        }
        else
          error("%s : must be a pointer\n", id);
        break;
      default :
        INT_ERROR("printaccess_optptr");
        break;
    }
    i->left = (Node *) NULL;
    free(nid);
  }
  else
    fprintf(fd ,"%s", id);
}

#ifdef __STDC__
void printaccess_opt(FILE *fd, Valtype t, Io rw, Node *n, char *id)
#else
printaccess_opt(fd, t, rw, n, id)
FILE *fd;
short t;
short rw;
Node *n;
char *id;
#endif
{
#ifdef DEBUG
  char buffer1[BUFSIZ], buffer2[BUFSIZ];
  sprintf(buffer1, "printaccess_opt(0x%lx, ", (unsigned long)fd);
  switch(t) {
    case FILEARG :
      strcat(buffer1, "FILEARG");
      break;
    case SCALARARG :
      strcat(buffer1, "SCALARARG");
      break;
    case FLAGARG :
      strcat(buffer1, "FLAGARG");
      break;
    default :
      strcat(buffer1, "<UNKNOWN>");
      break;
  }
  strcat(buffer1, ", ");
  switch(rw) {
    case READ :
      strcat(buffer1, "READ");
      break;
    case WRITE :
      strcat(buffer1, "WRITE");
      break;
    default :
      strcat(buffer1, "<UNKNOWN>");
      break;
  }
  sprintf(buffer2, ", 0x%lx, \"%s\")\n", (unsigned long)n, id);
  strcat(buffer1, buffer2);
  PRDBG(buffer1);
#endif
  if (n != NULL) {
    Node *i, *nid;
    for (i = n; i->left != NULL; i = i->left)
      ;
    nid = mkleaf(NAME, NULL, 0, id);
    switch (t) {
      case FILEARG :
        switch(rw) {
          case READ :
            {
              char buffer[BUFSIZ];
              Node *mw_nid;
              i->left = nid;
              printnode(fd, n); fprintf(fd, " = NULL, ");
              sprintf(buffer, "%s%s", MW_PFX, id);
              mw_nid = mkleaf(NAME, NULL, 0, buffer);
              i->left = mw_nid;
              printnode(fd, n->left);
              free(mw_nid);
            }
            break;
          case WRITE :
            {
              char buffer[BUFSIZ];
              Node *mw_nid;
              i->left = nid;
              printnode(fd, n); fprintf(fd, " = NULL, ");
              sprintf(buffer, "%s%s", MW_PFX, id);
              mw_nid = mkleaf(NAME, NULL, 0, buffer);
              i->left = mw_nid;
              printnode(fd, n->left);
              free(mw_nid);
            }
            break;
          default :
#ifdef DEBUG
            PRDBG("printaccess_opt : %d unknown Io\n", (int)rw);
#endif
            INT_ERROR("printaccess_opt");
            break;
        }
        break;
      case SCALARARG :
        switch(rw) {
          case READ :
            if (IS_PTR(n)) {
              char buffer[BUFSIZ];
              Node *mw_nid;
              i->left = nid;
              printnode(fd, n); fprintf(fd, " = NULL, ");
              sprintf(buffer, "%s%s", MW_PFX, id);
              mw_nid = mkleaf(NAME, NULL, 0, buffer);
              i->left = mw_nid;
              printnode(fd, n->left);
              free(mw_nid);
            }
            else
              error("%s : must be a pointer\n", id);
            break;
          case WRITE :
            if (IS_PTR(n)) {
              char buffer[BUFSIZ];
              Node *mw_nid;
              i->left = nid;
              printnode(fd, n); fprintf(fd, " = NULL, ");
              sprintf(buffer, "%s%s", MW_PFX, id);
              mw_nid = mkleaf(NAME, NULL, 0, buffer);
              i->left = mw_nid;
              printnode(fd, n->left);
              free(mw_nid);
            }
            else
              error("%s : must be a pointer\n", id);
            break;
          default :
#ifdef DEBUG
            PRDBG("printaccess_opt : %d unknown Io\n", (int)rw);
#endif
            INT_ERROR("printaccess_opt");
            break;
        }
        break;
      case FLAGARG :
        if (IS_PTR(n)) {
          char buffer[BUFSIZ];
          Node *mw_nid;
          i->left = nid;
          printnode(fd, n); fprintf(fd, " = NULL, ");
          sprintf(buffer, "%s%s", MW_PFX, id);
          mw_nid = mkleaf(NAME, NULL, 0, buffer);
          i->left = mw_nid;
          printnode(fd, n->left);
          free(mw_nid);
        }
        else
          error("%s : must be a pointer\n", id);
        break;
      default :
        INT_ERROR("printaccess_opt");
        break;
    }
    i->left = (Node *) NULL;
    free(nid);
  }
  else
    fprintf(fd ,"%s", id);
}


#ifdef __STDC__
void printaccess(FILE *fd, Io rw, Node *n, char *id)
#else
printaccess(fd, rw, n, id)
FILE *fd;
short rw;
Node *n;
char *id;
#endif
{
#ifdef DEBUG
  char buffer1[BUFSIZ], buffer2[BUFSIZ];
  sprintf(buffer1, "printaccess(0x%lx, ", (unsigned long)fd);
  switch(rw) {
    case READ :
      strcat(buffer1, "READ");
      break;
    case WRITE :
      strcat(buffer1, "WRITE");
      break;
    default :
      strcat(buffer1, "<UNKNOWN>");
      break;
  }
  sprintf(buffer2, ", 0x%lx, \"%s\")\n", (unsigned long)n, id);
  strcat(buffer1, buffer2);
  PRDBG(buffer1);
#endif
  if (n != NULL) {
    Node *i, *nid;
    for (i = n; i->left != NULL; i = i->left);
    nid = mkleaf(NAME, NULL, 0, id);
    if (rw == WRITE && IS_PTR(n)) {
      i->left = nid;
      printnode(fd, n->left);
    }
    else {
      i->left = nid;
      printnode(fd, n->left);
    }
    i->left = (Node *) NULL;
    free(nid);
  }
  else
    fprintf(fd ,"%s", id);
}

#ifdef __STDC__
void printaccess_io(FILE *fd, Node *n, char *id)
#else
printaccess_io(fd, n, id)
FILE *fd;
Node *n;
char *id;
#endif
{
#ifdef DEBUG
  PRDBG("printaccess_io(0x%lx, 0x%lx, \"%s\")\n", (unsigned long)fd,
                                               (unsigned long)n, id);
#endif
  if (n != NULL) {
    Node *i, *nid;
    for (i = n; i->left != NULL && i->left->left != NULL; i = i->left)
      ;
    nid = mkleaf(NAME, NULL, 0, id);
    if (i->left != NULL) {
      int *l;
      Node *nsz;
      switch(i->left->name) {
        case DEREF :
          i->left = nid;
          printnode(fd, n);
          i->left = (Node *) NULL;
          break;
        case ARRAY :
          i->left->left = nid;
          if ((l = MALLOC(int)) != NULL) {
            *l = BUFSIZ;
            nsz = mkleaf(INTEGER, NULL, 0, (void *)l);
            i->left->right = nsz;
            printnode(fd, n);
            i->left->left = i->left->right = (Node *)NULL;
            free(l); free(nsz);
          }
          else
            INT_ERROR("printaccess_io");
          break;
        default:
          i->left->left = nid;
          printnode(fd, n);
          i->left->left = (Node *) NULL;
          break;  
      }
    }
    else {
      int *l;
      Node *nsz;
      switch(i->name) {
        case DEREF :
          printnode(fd, nid);
          break;
        case ARRAY :
          i->left = nid;
          if ((l = MALLOC(int)) != NULL) {
            *l = BUFSIZ;
            nsz = mkleaf(INTEGER, NULL, 0, (void *)l);
            i->right = nsz;
            printnode(fd, n);
            i->left = i->right = (Node *)NULL;
            free(l); free(nsz);
          }
          else
            INT_ERROR("printaccess_io");
          break;
        default:
          i->left = nid;
          printnode(fd, n);
          i->left = (Node *) NULL;
          break;
      }
    }
    free(nid);
  }
  else
    fprintf(fd ,"%s", id);
}

/*
  Remove quote " found in the first and last positions of 
  the opt->desc field.
*/

#ifdef __STDC__
char *noquote(char* s)
#else
char *noquote(s)
char *s;
#endif
{
  static char buffer[BUFSIZ];
  int l;

  l = strlen(s)-2;
  strncpy(buffer, s+1, l);
  buffer[l] = '\0';
  return buffer;
}

/*
  Return the input string s corrected to be printed using printf()
*/

#ifdef __STDC__
char *make_printable(char* s)
#else
char *make_printable(s)
char *s;
#endif
{
  static char o[BUFSIZ];
  int i,j;

  for (i=j=0;s[i]!='\0';i++,j++)
    switch(s[i])
      {
      case '"' :
	j--;
	break;
	
      case '%':
	o[j++]='%';

      default :
	o[j]=s[i];
      }
  o[j] = '\0';
  return(o);
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mw.c

  Vers. 1.13
  (C) 1995-2003 Jacques Froment & Sylvain Parrino

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Standart UNIX include files */
#include <sys/file.h>
#include <string.h>

/*  For execle() */
#include <unistd.h>

#ifdef __STDC__
#include <stdlib.h>
#endif
#include <stdio.h>
#include <fcntl.h>

#ifndef __STDC__
#include <malloc.h>
#endif

#ifdef __STDC__
#include <stdarg.h>

#else
#include <varargs.h>
#endif

#include <setjmp.h>

/* MegaWave 2 include file */
#define MW_LIB
#include "mw.h"
#include "mwi.h"

#ifdef __STDC__
extern void mwerror(int, int, char *, ...);
#else
extern mwerror();
#endif

#define FNULL "/dev/null"

/* Global Variables */

char *mwname=NULL;         /* Name of the module */
char *mwgroup=NULL;        /* Group of the module */
char *mwerrormessage=NULL; /* Error message (from a module to XMegaWave2) */

/* Mode in which the library is running */
/* 1 :  Unix command */
/* 2 :  XMegaWave2  */
int mwrunmode=0;

/* For MegaWave interpretor (XMegaWave2) */
jmp_buf *_mwienv = NULL;

/* For redirection of stdout and stderr*/
static int out_sav = -1, fd_out, err_sav = -1, fd_err;
int verbose_flg = FALSE;
/* Change, if wanted, stdout and/or stderr */
#ifdef __STDC__
static void setnewout(void)
#else
static setnewout()
#endif
{
  char *s, *getenv();

  if ((s = getenv("MW_STDOUT")) != NULL && verbose_flg == FALSE) {
    char buffer[BUFSIZ];
    int oflag, mode;
    out_sav = dup(1);
    close(1);
    if (!strcmp(s, "NULL") || !strcmp(s, "")) {
      strcpy(buffer, FNULL);
      oflag = O_WRONLY;
      mode = 0666;
    }
    else {
      strcpy(buffer, s);
      oflag = O_RDWR | O_CREAT;
      mode = 0644;
    }
    if ((fd_out = open(buffer, oflag, mode)) < 0) {
      fd_out = dup(out_sav);
      close(out_sav);
      out_sav = -1;
      fprintf(stderr, "Cannot redirect standard output to \"%s\"\n", buffer);
    }
  }

  if ((s = getenv("MW_STDERR")) != NULL && verbose_flg == FALSE) {
    char buffer[BUFSIZ];
    int oflag, mode;
    err_sav = dup(2);
    close(2);
    if (!strcmp(s, "NULL") || !strcmp(s, "")) {
      strcpy(buffer, FNULL);
      oflag = O_WRONLY;
      mode = 0666;
    }
    else {
      strcpy(buffer, s);
      oflag = O_RDWR | O_CREAT;
      mode = 0644;
    }
    if ((fd_err = open(buffer, oflag, mode)) < 0) {
      fd_err = dup(err_sav);
      close(err_sav);
      err_sav = -1;
      fprintf(stderr, "Cannot redirect standard error to \"%s\"\n", buffer);
    }
  }
}

/* Restore, if needed, stdout and/or stderr */
#ifdef __STDC__
static void restoreout(void)
#else
static restoreout()
#endif
{
  if (out_sav >= 0) {
    fflush(stdout);
    close(fd_out);
    fd_out = dup(out_sav);
    out_sav = -1;
    close(out_sav);
  }

  if (err_sav >= 0) {
    fflush(stderr);
    close(fd_err);
    fd_err = dup(err_sav);
    err_sav = -1;
    close(err_sav);
  }
}

/* MegaWave exit */
#ifdef __STDC__
void mwexit(int n)
#else
mwexit(n)
int n;
#endif
{
  restoreout();
  if (_mwienv == NULL)
    exit(n);
  else
    longjmp(*_mwienv, n);
}

/* MegaWave _exit */
#ifdef __STDC__
void mw_exit(int n)
#else
mw_exit(n)
int n;
#endif
{
  restoreout();
  if (_mwienv == NULL) 
    _exit(n);
  else
    longjmp(*_mwienv, n);
}

#ifdef __STDC__
void *mwmalloc(size_t size)
#else
char *mwmalloc(size)
unsigned size;
#endif
{
  return (void *)malloc(size);
}

#ifdef __STDC__
void mwfree(void *ptr)
{
  free(ptr);
}
#else

/* Supprime car sur HP-UX free retourne un void 
int mwfree(ptr)
char *ptr;
{
  return(free(ptr));
}
*/
void mwfree(ptr)
char *ptr;
{
  free(ptr);
}

#endif

#ifdef __STDC__
void *mwrealloc(void *ptr, size_t size)
#else
char *mwrealloc(ptr, size)
char *ptr;
unsigned size;
#endif
{
  return (void *)realloc(ptr, size);
}

#ifdef __STDC__
void *mwcalloc(size_t nelem, size_t elsize)
#else
char *mwcalloc(nelem, elsize)
unsigned nelem, elsize;
#endif
{
  return (void *)calloc(nelem, elsize);
}

#ifdef __STDC__
void mwcfree(void *ptr)
{
  free(ptr);
}
#else
int mwcfree(ptr)
char *ptr;
{
  return cfree(ptr);
}
#endif
/****************************/
/* Default MegaWave options */
/****************************/

/* Data structure for default megawave options */
struct mwargs {
  char *name;
  char *arg;
  int   argsiz;
  char *argtexname;
  void (*action)();
};

/* Default option buffer */
char _mwdefoptbuf[BUFSIZ];

static void call_help(), call_debug(), call_verbose(), call_ftype(), call_vers(), call_fsum(), call_proto(), call_ftypelist();
char type_force[mw_ftype_size+1] = {'?', '\0'};


/* System options */
struct mwargs mwargs[] = { 
  {"-debug",   NULL,       0,                  NULL,             call_debug}, 
  {"-help",    NULL,       0,                  NULL,             call_help}, 
  {"-verbose", NULL,       0,                  NULL,             call_verbose}, 
  {"-ftype",   type_force, sizeof(type_force), "<image type>",   call_ftype}, 
  {"-vers",   NULL,        0,                  NULL,             call_vers}, 
  {"-fsum",   NULL,        0,                 NULL,             call_fsum}, 
  {"-proto",   NULL,        0,                 NULL,             call_proto},
  {"-ftypelist",   NULL,        0,                 NULL,             call_ftypelist},  
  {NULL}
};

#ifdef __STDC__
static struct mwargs *lookup(char *s)
#else
static struct mwargs *lookup(s)
char *s;
#endif
{
  struct mwargs *p;
  for (p=mwargs; p->name != NULL; p++)
    if (!strcmp(p->name, s))
      return p;
  return NULL;
}

/* Actions for default MegaWave options */

int mwdbg = FALSE;
#ifdef __STDC__
static void call_debug(char *s)
#else
static void call_debug(s)
char *s;
#endif
{
  mwdbg = TRUE;
}

int help_flg = FALSE;
#ifdef __STDC__
static void call_help(char *s)
#else
static void call_help(s)
char *s;
#endif
{
  help_flg = TRUE;
}

#ifdef __STDC__
static void call_verbose(char *s)
#else
static void call_verbose(s)
char *s;
#endif
{
  verbose_flg = TRUE;
}


#ifdef __STDC__
static void call_ftype(char *s)
#else
static void call_ftype(s)
char *s;
#endif
{
  ;
}

int vers_flg = FALSE;
#ifdef __STDC__
static void call_vers(void)
#else
static void call_vers()
#endif
{
  vers_flg = TRUE;
}

/* Write function summary */
#ifdef __STDC__
static void call_fsum(void)
#else
static void call_fsum()
#endif
{
  printf("%s",mwicmd[mwind].fsummary);
  mwexit(0);
}

/* For call_proto() : return in <type> the type of the variable <var>
   from the function declaration <fdecl> (fsum without first line).
*/

static void find_type(fdecl,var,type)

char *fdecl,*var,*type;

{
  char vtype[BUFSIZ]; /* current variable type */
  char v[BUFSIZ];     /* current variable name */
  char pvar[BUFSIZ];  /* *<var> */
  int i,j;

  sprintf(pvar,"*%s",var);
  i=0;
  while (1)
    {
      j=i;
      for (; (fdecl[i]!='\0')&&(fdecl[i]!=' '); i++) vtype[i-j]=fdecl[i];
      if (fdecl[i]=='\0')
	mwerror(INTERNAL,1,"[find_type] cannot find <space> !\n");  
      vtype[i-j]='\0';
      /*printf("vtype=<%s>\n",vtype);*/

      do
	{
	  i++;
	  j=i;
	  for (; (fdecl[i]!='\0')&&(fdecl[i]!=' ')&&(fdecl[i]!=';'); i++) 
	    v[i-j]=fdecl[i];
	  if (fdecl[i]=='\0')
	    mwerror(INTERNAL,1,"[find_type] cannot find <space> !\n");  
	  v[i-j]='\0';
	  if ((i>j)&&(v[0]!=',')) 
	    {
	      /*printf("v=<%s>\n",v);*/
	      if (strcmp(var,v)==0) 
		{
		  strcpy(type,vtype);
		  return;
		}
	      if (strcmp(pvar,v)==0) 
		{
		  sprintf(type,"%s *",vtype);
		  return;
		}
	    }
	}
      while (i>j);

      for (; (fdecl[i]!='\0')&&(fdecl[i]!='\n'); i++);
      if (fdecl[i]=='\0')
	mwerror(INTERNAL,1,"[find_type] cannot find var='%s' !\n",var);
      i++;
    }
}

/* Write function prototype */
#ifdef __STDC__
static void call_proto(void)
#else
static void call_proto()
#endif
{
  char type[BUFSIZ];  /* type of the function */
  char name[BUFSIZ];  /* name of the function */
  char var[BUFSIZ];   /* list of all variables */
  char v[BUFSIZ];     /* current variable name */
  char vtype[BUFSIZ]; /* current variable type */
  char fdecl[BUFSIZ]; /* declaration of variables */
  int i,j;

  if (sscanf(mwicmd[mwind].fsummary,"%s%s%[^)]",type,name,var)!=3)
    mwerror(INTERNAL,1,"[call_proto] cannot extract var field (3) !\n");
  if ((var[0]!=' ')||(var[1]!='('))
    /* Maybe function type forgotten */
    {
      if (sscanf(mwicmd[mwind].fsummary,"%s%[^)]",name,var)!=2)
	mwerror(INTERNAL,1,"[call_proto] cannot extract var field (2) !\n");
      if ((var[0]!=' ')||(var[1]!='('))      
	mwerror(INTERNAL,1,"[call_proto] invalid extracted var field '%s'\n",var);
      strcpy(type,"int");
      mwerror(WARNING,1,"No type definition for function '%s'; assuming <int> but the author probably meant <void>.\n",name);
    }
  
  for (i=0; (mwicmd[mwind].fsummary[i]!='\n')&&
	 (mwicmd[mwind].fsummary[i]!='\0'); i++);
  if (mwicmd[mwind].fsummary[i]=='\0')
    mwerror(INTERNAL,1,"[call_proto] cannot find \\n !\n");    

  /* get fdecl */
  for (j=i+1; mwicmd[mwind].fsummary[j]!='\0'; j++)
    fdecl[j-i-1]= mwicmd[mwind].fsummary[j];
  fdecl[j-i-3]='\0';
  /*printf("fdecl=<%s>\n",fdecl);*/

  printf("#ifdef __STDC__\n");
  printf("%s %s(",type,name);
  for (i=2; var[i]!='\0'; )
    {
      for (j=0;(var[i+j]!='\0')&&(var[i+j]!=' '); j++) v[j]=var[i+j];
      if (var[i+j]=='\0')
	mwerror(INTERNAL,1,"[call_proto] cannot find <space> !\n");    	
      v[j]='\0';
      if (*v!=',')
	{
	  find_type(fdecl,v,vtype);
	  printf("%s",vtype);
	}
      else
	printf(",");
      i+=j+1;
    }
  printf(");\n");
  printf("#else\n");
  printf("%s %s();\n",type,name);
  printf("#endif\n");


  mwexit(0);
}

/* Write function prototype */
#ifdef __STDC__
static void call_ftypelist(void)
#else
static void call_ftypelist()
#endif
{
  char **A;
  int i;
 
  A=(char **)mw_ftypes_description;

  for (i=0; (A[i]!=NULL)&&(A[i+1]!=NULL); i+=2)
    {
      printf("%s \t\t %s\n",A[i],A[i+1]);
    }
  mwexit(0);
}


/* MegaWave2 main function */
#ifdef __STDC__
_mw_main(int argc, char *argv[], char *envp[])
#else
_mw_main(argc, argv, envp)
int argc;
char *argv[], *envp[];
#endif
{ 
  char *userargv[BUFSIZ], *strrchr();
  int i, userargc, flg;
  struct mwargs *p;
  char command[BUFSIZ],*chm;
  int retcommand;
#ifdef XMWP
  char *mw_xmw;
#endif
  char *getenv();  

  /* Name of module */
  if ((mwname = strrchr(argv[0], '/')) != NULL)
    mwname = mwname + 1;
  else
    mwname = argv[0];

  /* Group */
  mwgroup = mwicmd[mwind].group;

  /* This current main function is executed in run-time mode only */
  mwrunmode = 1;

  /* If MW_CHECK_HIDDEN set, check for hidden module */
  chm=getenv("MW_CHECK_HIDDEN");
  if ((chm) && (chm[0]=='1'))
    {
      sprintf(command,"mwwhere -bin %s > /dev/null",mwname);
      retcommand=system(command) >> 8;
      if (retcommand == 2)
	mwerror(WARNING,1,"Module of same name hidden by this one !\n");
    }
#ifdef XMWP
  /* If module is executed without any argument and if shell variable MW_XMW is
     set then the module tries to exec the XMegaWave2 main program. */
  if (argc == 1  && (mw_xmw = getenv("MW_XMW")) != NULL) {
    char xmw2_path[BUFSIZ], xmw2_name[BUFSIZ], module_path[BUFSIZ], *ind;
  
    if ((ind = strrchr(mw_xmw, '/')) != NULL) {
      *ind = '\0';
      strcpy(xmw2_path, mw_xmw);
      strcpy(xmw2_name, ind + 1);
      *ind = '/';
    }
    else {
      sprintf(xmw2_path, "./%s", mw_xmw);
      strcpy(xmw2_name, mw_xmw);
    }
    
    sprintf(module_path, "%s/%s", mwgroup, mwname);
    execle(mw_xmw, xmw2_name, "-x", module_path, NULL, envp);
    /* The following instruction are executed if the module cannot execute
       the XMegaWave2 main program. */
    mwerror(WARNING, 0, "Cannot find or exec XMegaWave2 main program \"%s/%s\".\n",
                        xmw2_path, xmw2_name);
    mwerror(WARNING, 0, "Continue without the window-oriented environment.\n");
  }
#endif
  
  /* Make default option buf for usage */
  for (p=mwargs; p->name != NULL; p++) {
    strcat(_mwdefoptbuf, " [");
    strcat(_mwdefoptbuf, p->name);
    if (p->arg != NULL) {
      strcat(_mwdefoptbuf, " ");
      strcat(_mwdefoptbuf, p->argtexname);
    }
    strcat(_mwdefoptbuf, "]");
  }

  /* Sort user options and MegaWave default options ; for the lasts, do
     corresponding action */
  for (i=1, userargc=1, userargv[0]=argv[0], flg = FALSE; i<argc; i++) {
    if (flg == TRUE) {
      if (strlen(argv[i]) > (p->argsiz-1))
        *(argv[i] + p->argsiz - 1) = '\0';
      strcpy(p->arg, argv[i]);
      (*p->action)(p->arg);
      flg = FALSE;
    }
    else if ((p=lookup(argv[i])) != NULL) {
      if (p->arg != NULL)
        flg = TRUE;
      else
        (*p->action)(NULL);
    }
    else
      userargv[userargc++]=argv[i];
  }

  setnewout();
  _mwmain(userargc, userargv);
  mwexit(0);
}

/* MegaWave default options actions */
#ifdef __STDC__
void MegaWaveDefOpt(char *vers)
#else
MegaWaveDefOpt(vers)
char *vers;
#endif
{
  /* Version flag */
  if (vers_flg == TRUE)
    {
      printf("%s\n",vers);
      mwexit(0);
    }

  /* Help flag */
  if (help_flg == TRUE)
    mwusage(NULL);
}

char _mwoptlist[BUFSIZ] = {'\0'};

#ifdef __STDC__
int mw_opt_used(char c)
#else
mw_opt_used(c)
char c;
#endif
{
  return (strchr(_mwoptlist, c) != NULL) ? TRUE : FALSE;
}

#ifdef __STDC__
void mwdebug(char *fmt, ...)
#else
mwdebug(va_alist)
va_dcl
#endif
{
  if (mwdbg) {
    va_list marker;
#ifndef __STDC__
    char *fmt;
#endif

#ifdef __STDC__
    va_start(marker, fmt);
#else
    va_start(marker);
    fmt = va_arg(marker, char *);
#endif
    fprintf(stderr, "<dbg> ");
    vfprintf(stderr, fmt, marker);
    va_end(marker);
  }
}

int mwerrcnt = 0;
#ifdef __STDC__
void mwerror(int code, int exit_code, char *fmt, ...)
#else
mwerror(va_alist)
va_dcl
#endif
{
  va_list marker;
#ifndef __STDC__
  int code, exit_code;
  char *fmt;
#endif
  char message[BUFSIZ];

#ifdef __STDC__
  va_start(marker, fmt);
#else
  va_start(marker);
  code = va_arg(marker, int);
  exit_code = va_arg(marker, int);
  fmt = va_arg(marker, char *);
#endif

  switch(code) {
    case WARNING:
      fprintf(stderr, "MegaWave2 warning (%s) : ", mwname);
      vfprintf(stderr, fmt, marker);
      break;
    case ERROR:
      fprintf(stderr, "MegaWave2 error (%s) : ", mwname);
      vfprintf(stderr, fmt, marker);
      mwerrcnt++;
      break;
    case FATAL:
      fprintf(stderr, "MegaWave2 fatal (%s) : ", mwname);
      vfprintf(stderr, fmt, marker);
      fprintf(stderr, "Exit.\n");
      if (mwrunmode == 2)
	{ /* Send error message to XMegaWave2 */
	  (void) vsprintf(message, fmt, marker);
	  if (mwerrormessage) free(mwerrormessage);
	  mwerrormessage = (char *) malloc(strlen(message)+1);
	  strcpy(mwerrormessage,message);
	}
      mwexit(exit_code);
      break;
    case INTERNAL:
      fprintf(stderr, "MegaWave2 internal (%s) : ", mwname);
      vfprintf(stderr, fmt, marker);
      fprintf(stderr, "Exit.\n");
      if (mwrunmode == 2)
	{ /* Send error message to XMegaWave2 */
	  (void) vsprintf(message, fmt, marker);
	  if (mwerrormessage) free(mwerrormessage);
	  mwerrormessage = (char *) malloc(strlen(message)+1);
	  strcpy(mwerrormessage,message);
	}
      mwexit(exit_code);
      break;
    case USAGE:
      (void) vsprintf(message, fmt, marker);
      if (mwrunmode == 1)
	mwusage(message);
      else
	{ /* Send error message to XMegaWave2 */
	  if (mwerrormessage) free(mwerrormessage);
	  mwerrormessage = (char *) malloc(strlen(message)+1);
	  strcpy(mwerrormessage,message);
	  longjmp(*_mwienv,1);	
	}
	  break;
    default:
      mwerror(FATAL, 1, "Bad usage of mwerror : code %d is unknown\n", code);
      break;
  }
  va_end(marker);
}

/* I/O conversion functions called by mwp (data_io.c) */

#ifdef __STDC__
char *_mw_ctoa_(char c)
#else
char *_mw_ctoa_(c)
char c;
#endif
{
  char *ret;
  char buffer[BUFSIZ];
  sprintf(buffer, "%c", c);
  ret = (char *)malloc(strlen(buffer)+1);
  strcpy(ret, buffer);
  return ret;
}

#ifdef __STDC__
char *_mw_uctoa_(unsigned char uc)
#else
char *_mw_uctoa_(uc)
unsigned char uc;
#endif
{
  char *ret;
  char buffer[BUFSIZ];
  sprintf(buffer, "%c", (char)uc);
  ret = (char *)malloc(strlen(buffer)+1);
  strcpy(ret, buffer);
  return ret;
}

#ifdef __STDC__
char *_mw_stoa_(short s)
#else
char *_mw_stoa_(s)
short s;
#endif
{
  char *ret;
  char buffer[BUFSIZ];
  sprintf(buffer, "%hd", s);
  ret = (char *)malloc(strlen(buffer)+1);
  strcpy(ret, buffer);
  return ret;
}

#ifdef __STDC__
char *_mw_ustoa_(unsigned short us)
#else
char *_mw_ustoa_(us)
unsigned short us;
#endif
{
  char *ret;
  char buffer[BUFSIZ];
  sprintf(buffer, "%hu", us);
  ret = (char *)malloc(strlen(buffer)+1);
  strcpy(ret, buffer);
  return ret;
}

#ifdef __STDC__
char *_mw_itoa_(int i)
#else
char *_mw_itoa_(i)
int i;
#endif
{
  char *ret;
  char buffer[BUFSIZ];
  sprintf(buffer, "%d", i);
  ret = (char *)malloc(strlen(buffer)+1);
  strcpy(ret, buffer);
  return ret;
}

#ifdef __STDC__
char *_mw_uitoa_(unsigned int ui)
#else
char *_mw_uitoa_(ui)
unsigned int ui;
#endif
{
  char *ret;
  char buffer[BUFSIZ];
  sprintf(buffer, "%u", ui);
  ret = (char *)malloc(strlen(buffer)+1);
  strcpy(ret, buffer);
  return ret;
}

#ifdef __STDC__
char *_mw_ltoa_(long l)
#else
char *_mw_ltoa_(l)
long l;
#endif
{
  char *ret;
  char buffer[BUFSIZ];
  sprintf(buffer, "%ld", l);
  ret = (char *)malloc(strlen(buffer)+1);
  strcpy(ret, buffer);
  return ret;
}

#ifdef __STDC__
char *_mw_ultoa_(unsigned long ul)
#else
char *_mw_ultoa_(ul)
unsigned long ul;
#endif
{
  char *ret;
  char buffer[BUFSIZ];
  sprintf(buffer, "%lu", ul);
  ret = (char *)malloc(strlen(buffer)+1);
  strcpy(ret, buffer);
  return ret;
}

/* 17/3/95: Erreur inexpliquee sur Sun Solaris 5.3 (Grib1): */
/*          la variable f n'est pas mise a la bonne valeur  */
/*          lors de l'appel de _mw_ftoa_(xx) si cette       */
/*          fonction n'est pas declaree suivant la norme ANSI !! */
/*  A enlever dans une future version (tester par exemple   */
/*  la valeur retournee par le module fentropy)             */

#if defined (__STDC__) || defined(sun4_5)
char *_mw_ftoa_(float f)
#else
char *_mw_ftoa_(f)
float f;
#endif
{
  char *ret;
  char buffer[BUFSIZ];
  sprintf(buffer, "%g", (double)f);
  ret = (char *)malloc(strlen(buffer)+1);
  strcpy(ret, buffer);
  return ret;
}

#ifdef __STDC__
char *_mw_dtoa_(double d)
#else
char *_mw_dtoa_(d)
double d;
#endif
{
  char *ret;
  char buffer[BUFSIZ];
  sprintf(buffer, "%g", d);
  ret = (char *)malloc(strlen(buffer)+1);
  strcpy(ret, buffer);
  return ret;
}

#ifdef __STDC__
_mwis_open(char *s, char *rw)
#else
_mwis_open(s, rw)
char *s;
char *rw;
#endif
{
  FILE *fd, *fopen();
  char fname[BUFSIZ];

  if (*rw == 'r') /* read */
    {
      strcpy(fname,s);          /* Do Not Change the value of s */
      return(_search_filename(fname));
    }
  return(TRUE); /* Checking diseable for writing */
}


int   _mwoptind = 1;
char *_mwoptarg = NULL;

#ifdef __STDC__
int _mwgetopt(int argc, char **argv, char *optstring)
#else
int _mwgetopt(argc, argv, optstring)
int argc;
char **argv;
char *optstring;
#endif
{
  if (_mwoptind < argc) {
    if (*argv[_mwoptind] == '-') {
      if (*(argv[_mwoptind] + 1) != '.' && *(argv[_mwoptind] + 1) != '-' &&
          !isdigit(*(argv[_mwoptind] + 1))) {
        char *opt;
        opt = strchr(optstring, *(argv[_mwoptind] + 1));
        if (opt == NULL)
          return '?';
        else {
          if (*(opt+1) == ':') {
            if (++_mwoptind < argc)
              _mwoptarg = argv[_mwoptind];
            else {
              _mwoptarg = NULL;
              return '?';
            }
          }
          _mwoptind++;
          return *opt;
        }
      }
      else
        return -1;
    }
    else
      return -1;
  }
  else
    return -1;
}

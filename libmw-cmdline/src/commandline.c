/**
 * @mainpage libmw-cmdline
 *
 * @section Introduction
 *
 * This library provides all the tools used for command-line parsing
 * and automated structures allocation/deallocation and input/output
 * in the executable versions of the megawave modules.
 */

/**
 * @file commandline.c
 *
 * @version 1.14
 * @author Jacques Froment (1995 - 2005)
 * @author Sylvain Parrino (1995 - 2005)
 * @author Nicolas Limare (2009)
 */

/* TODO: include _mwsave_xx form libmw */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <setjmp.h>

#include "mw.h"

/*
 * structures
 */

/**
 *  data structure for the default megawave options
 */
struct mwargs {
     char *name;
     char *arg;
     int   argsiz;
     char *argtexname;
     void (*action)(void);
};

struct Mwiline {
  char *name;
  int (*mwarg)(int, char**);
  void (*mwuse)(char *);
  char *group;
  char *function;
  char *usage;
};
typedef struct Mwiline Mwiline;

#include "commandline.h"

/*
 * global variables
 */

static int verbose_flg = FALSE;
static int vers_flg = FALSE;

/* TODO : make static, passa as param from the command-line source */
int help_flg = FALSE;
char _mwdefoptbuf[BUFSIZ]="";
char type_force[mw_ftype_size+1] = "?";
char _mwoptlist[BUFSIZ] = "";
int   _mwoptind = 1;
char *_mwoptarg = NULL;

/**
 * system options
 */
/* TODO: explanations */

static void call_debug(void);
static void call_help(void);
static void call_verbose(void);
static void call_vers(void);
static void call_ftypelist(void);
static void call_ftype(void);

struct mwargs mwargs[] = { 
    {"-debug",     NULL, 0, NULL, call_debug}, 
    {"-help",      NULL, 0, NULL, call_help}, 
    {"-verbose",   NULL, 0, NULL, call_verbose}, 
    {"-vers",      NULL, 0, NULL, call_vers}, 
    {"-ftypelist", NULL, 0, NULL, call_ftypelist},
    {"-ftype",     type_force, sizeof(type_force), "<image type>", call_ftype}, 
    {NULL,         NULL, 0, NULL, NULL}
};

/*
 * local functions
 */

/**
 * emulate a simple isdigit() without handling localization
 * to avoid glibc dependencies
 */
static int simple_isdigit(int c)
{
     return ((c >= '0') && (c <= '9'));
}

/**
 * change, if wanted, stdout and/or stderr
 */
/* TODO: drop, use shell redirection instead? */
static void setnewout(void)
{
     char *fname;

     if (NULL != (fname = getenv("MW_STDOUT")) 
	 && verbose_flg == FALSE)
     {
	 if (NULL == freopen(fname, "w", stdout))
	     fprintf(stderr,						\
		     "Cannot redirect standard output to \"%s\"\n", fname);
     }

     if (NULL != (fname = getenv("MW_STDERR"))
	 && verbose_flg == FALSE)
     {
	 if (NULL == freopen(fname, "w", stderr))
	     fprintf(stderr,						\
		     "Cannot redirect standard error to \"%s\"\n", fname);
     }
}

/*
 * actions for default megawave options
 */

static void call_debug(void)
{
     mwdbg = TRUE;
     return;
}

static void call_help(void)
{
     help_flg = TRUE;
     return;
}

static void call_verbose(void)
{
     verbose_flg = TRUE;
     return;
}


static void call_ftype(void)
{
     return;
}

static void call_vers(void)
{
     vers_flg = TRUE;
     return;
}

static void call_ftypelist(void)
{
     char **A;
     int i;
 
     A = (char **)mw_ftypes_description;

     for (i = 0; (A[i] != NULL) && (A[i+1] != NULL); i += 2)
     {
	 printf("%s \t\t %s\n", A[i], A[i+1]);
     }
     exit(0);
}

static struct mwargs *lookup(char *s)
{
     struct mwargs *p;
     for (p=mwargs; p->name != NULL; p++)
	  if (!strcmp(p->name, s))
	       return p;
     return NULL;
}

/**
 * main run-time function
 */
int _mw_main(int argc, char *argv[], char *envp[],
	     Mwiline mwicmd[], int mwind)
{ 
    char *userargv[BUFSIZ];
    int i, userargc, flg;
    struct mwargs *p;
    char command[BUFSIZ],*chm;
    int retcommand;
    
    /* FIXME: unused parameter */
    envp = envp;
    
    /* Name of module */
    if ((mwname = strrchr(argv[0], '/')) != NULL)
	mwname = mwname + 1;
    else
	mwname = argv[0];
    
    /* Group */
    mwgroup = mwicmd[mwind].group;
    
    /* If MW_CHECK_HIDDEN set, check for hidden module */
    chm=getenv("MW_CHECK_HIDDEN");
    if ((chm) && (chm[0]=='1'))
    {
	sprintf(command,"mwwhere -bin %s > /dev/null",mwname);
	retcommand=system(command) >> 8;
	if (retcommand == 2)
	    mwerror(WARNING,1,"Module of same name hidden by this one !\n");
    }
  
     /* make default option buf for usage */
     for (p=mwargs; p->name != NULL; p++) {
	  strcat(_mwdefoptbuf, " [");
	  strcat(_mwdefoptbuf, p->name);
	  if (p->arg != NULL) {
	       strcat(_mwdefoptbuf, " ");
	       strcat(_mwdefoptbuf, p->argtexname);
	  }
	  strcat(_mwdefoptbuf, "]");
     }

     /*
      * sort user options and MegaWave default options;
      * for the lasts, do corresponding action
      */
     for (i=1, userargc=1, userargv[0]=argv[0], flg = FALSE; i<argc; i++) {
	  if (flg == TRUE) {
	       if (strlen(argv[i]) > (size_t) (p->argsiz -1))
		    *(argv[i] + p->argsiz - 1) = '\0';
	       strcpy(p->arg, argv[i]);
	       (*p->action)();
	       flg = FALSE;
	  }
	  else if ((p=lookup(argv[i])) != NULL) {
	       if (p->arg != NULL)
		    flg = TRUE;
	       else
		    (*p->action)();
	  }
	  else
	       userargv[userargc++]=argv[i];
     }

     setnewout();
     mwicmd[mwind].mwarg(userargc, userargv);
     exit(0);
     return 0;
}

/**
 * megawave default options actions
 */
void mwdefopt(char *vers, Mwiline mwicmd[], int mwind)
{
     /* Version flag */
     if (vers_flg == TRUE)
     {
	  printf("%s\n",vers);
	  exit(0);
     }

     /* Help flag */
     if (help_flg == TRUE)
	  mwicmd[mwind].mwuse(NULL);

}

int mw_opt_used(char c)
{
     return (strchr(_mwoptlist, c) != NULL) ? TRUE : FALSE;
}

int _mwgetopt(int argc, char **argv, char *optstring)
{
     if (_mwoptind < argc) {
	  if (*argv[_mwoptind] == '-') {
	       if (*(argv[_mwoptind] + 1) != '.' 
		   && *(argv[_mwoptind] + 1) != '-' &&
		   !simple_isdigit(*(argv[_mwoptind] + 1))) {
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

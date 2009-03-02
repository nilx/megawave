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
 * @author Jacques Froment (1995-2005)
 * @author Sylvain Parrino (1995-2005)
 */

/* TODO: cleanup #includes */

/* TODO: remove, unix-centric */
#include <sys/file.h>
#include <unistd.h> /* for execle() */
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <setjmp.h>

/* #include "definitions.h" */

#include "mw.h"
#include "commandline.h"

/*
 * global variables
 */

/* TODO : move to config.h */

/* TODO: drop, unix-centric */
#define FNULL "/dev/null"

/* for redirection of stdout and stderr*/
static int out_sav = -1, fd_out, err_sav = -1, fd_err;
int verbose_flg = FALSE;

/**
 * default option buffer
 */
char _mwdefoptbuf[BUFSIZ];

char type_force[mw_ftype_size+1] = {'?', '\0'};

int mwdbg = FALSE;
int help_flg = FALSE;
int vers_flg = FALSE;

extern int help_flg;
extern int vers_flg;

char _mwoptlist[BUFSIZ] = {'\0'};

int   _mwoptind = 1;
char *_mwoptarg = NULL;

/*
 * structures
 */

/* TODO : move to definitions.h */

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

/**
 * system options
 */
/* TODO: explanations */

static void call_debug(void);
static void call_help(void);
static void call_verbose(void);
static void call_vers(void);
static void call_fsum(void);
static void call_proto(void);
static void call_ftypelist(void);
static void call_ftype(void);

struct mwargs mwargs[] = { 
    {"-debug",     NULL, 0, NULL, call_debug}, 
    {"-help",      NULL, 0, NULL, call_help}, 
    {"-verbose",   NULL, 0, NULL, call_verbose}, 
    {"-vers",      NULL, 0, NULL, call_vers}, 
    {"-fsum",      NULL, 0, NULL, call_fsum}, 
    {"-proto",     NULL, 0, NULL, call_proto},
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
/* TODO: use freopen() */
static void setnewout(void)
{
     char *s;

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
	       fprintf(stderr, \
		       "Cannot redirect standard output to \"%s\"\n", buffer);
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
	       fprintf(stderr, \
		       "Cannot redirect standard error to \"%s\"\n", buffer);
	  }
     }
}

/* TODO: drop, unused? */
void *mwmalloc(size_t size)
{ return (void *)malloc(size); }
void mwfree(void *ptr)
{ free(ptr); }
void *mwrealloc(void *ptr, size_t size)
{ return (void *)realloc(ptr, size); }
void *mwcalloc(size_t nelem, size_t elsize) 
{ return (void *)calloc(nelem, elsize); }
void mwcfree(void *ptr)
{ free(ptr); }

static void find_type(char * fdecl,char * var, char * type)
{
     char vtype[BUFSIZ]; /* current variable type */
     char v[BUFSIZ];     /* current variable name */
     char pvar[BUFSIZ];  /* *<var> */
     char pvars[BUFSIZ]; /* * <var> */
     int i,j;

     if (var[0] == '\0') mwerror(INTERNAL, 1,
			       "[find_type] no variable specified !\n");  
  
     sprintf(pvar, "*%s", var);
     sprintf(pvars, "* %s", var);
     i=0;
     while (1)
     {
	  j=i;
	  /* TODO: change to human-readable code */
	  for (; (fdecl[i] != '\0') && (fdecl[i] != ' '); i++)
	      vtype[i-j] = fdecl[i];
	  if (fdecl[i] == '\0')
	       mwerror(INTERNAL, 1,
		       "[find_type] cannot find <space> ! (1)\n");  
	  vtype[i-j] = '\0';
	  do
	  {
	       i++;
	       j = i;
	       /* TODO: change to human-readable code */
	       for (; (fdecl[i] != '\0')
			&&((fdecl[i] != ' ')
			   ||(fdecl[i-1] == '*'))
			&&(fdecl[i] != ';'); i++) 
		    v[i-j] = fdecl[i];
	       if (fdecl[i] == '\0')
		    mwerror(INTERNAL, 1, 
			    "[find_type] cannot find <space> ! (2)\n");  
	       v[i-j] = '\0';
	       if ((i > j) && (v[0] != ',')) 
	       {
		    if (strcmp(var, v) == 0) 
		    {
			 strcpy(type, vtype);
			 return;
		    }
		    if ((strcmp(pvar,v) == 0) || (strcmp(pvars, v) == 0))
		    {
			 sprintf(type,"%s *", vtype);
			 return;
		    }
	       }
	  }
	  while (i > j);

	  for (; (fdecl[i] != '\0') && (fdecl[i] != '\n'); i++);
	  if (fdecl[i] == '\0')
	       mwerror(INTERNAL, 1,
		       "[find_type] cannot find var='%s' !\n",var);
	  i++;
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

/**
 * write function summary
 */
static void call_fsum(void)
{
     printf("%s", mwicmd[mwind].fsummary);
     exit(0);
}

/*
 * write function prototype
 */
static void call_proto(void)
{
     char type[BUFSIZ];  /* type of the function */
     char name[BUFSIZ];  /* name of the function */
     char var[BUFSIZ];   /* list of all variables */
     char v[BUFSIZ];     /* current variable name */
     char vtype[BUFSIZ]; /* current variable type */
     char fdecl[BUFSIZ]; /* declaration of variables */
     int i,j;

     if (sscanf(mwicmd[mwind].fsummary, "%s%s%[^)]", type, name, var) != 3)
	  mwerror(INTERNAL, 1,
		  "[call_proto] cannot extract var field (3) !\n");
     if ((var[0] != ' ') || (var[1] != '('))
     {
	 /* maybe function type forgotten */
	 if (sscanf(mwicmd[mwind].fsummary, "%s%[^)]", name, var) != 2)
	     mwerror(INTERNAL, 1,
		     "[call_proto] cannot extract var field (2) !\n");
	 if ((var[0] != ' ') || (var[1] != '('))      
	     mwerror(INTERNAL, 1,
		     "[call_proto] invalid extracted var field '%s'\n", var);
	 strcpy(type, "int");
	 mwerror(WARNING, 1, 
		 "No type definition for function '%s';"
		 "assuming <int> but the author probably meant <void>.\n",
		 name);
     }
  
     for (i = 0; (mwicmd[mwind].fsummary[i] != '\n') &&
	      (mwicmd[mwind].fsummary[i] != '\0'); i++);
     if (mwicmd[mwind].fsummary[i] == '\0')
	 mwerror(INTERNAL, 1, "[call_proto] cannot find \\n !\n");    

     /* get fdecl */
     for (j = i + 1; mwicmd[mwind].fsummary[j] != '\0'; j++)
	 fdecl[j-i-1] = mwicmd[mwind].fsummary[j];
     fdecl[j-i-3]='\0';

     printf("%s %s(", type, name);
     for (i=2; var[i] != '\0'; )
     {
	  for (j = 0; (var[i+j] != '\0') 
		   && (var[i+j] != ' '); j++) 
	      v[j] = var[i+j];
	  if (j == 0) 
	      goto cont;
	  if (var[i+j] == '\0')
	       mwerror(INTERNAL, 1,
		       "[call_proto] cannot find <space> !\n");    	
	  v[j] = '\0';
	  if (*v != ',')
	  {
	      find_type(fdecl, v, vtype);
	      printf("%s", vtype);
	  }
	  else
	       printf(",");
     cont:
	  i += j + 1;
     }
     printf(");\n");

     exit(0);
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
int _mw_main(int argc, char *argv[], char *envp[])
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
void MegaWaveDefOpt(char *vers)
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

/*
 * i/o conversion functions called by mwp (data_io.c)
 */

char *_mw_ctoa_(char c)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%c", c);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_uctoa_(unsigned char uc)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%c", (char)uc);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_stoa_(short s)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%hd", s);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_ustoa_(unsigned short us)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%hu", us);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_itoa_(int i)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%d", i);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_uitoa_(unsigned int ui)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%u", ui);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_ltoa_(long l)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%ld", l);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_ultoa_(unsigned long ul)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%lu", ul);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

/*
 * 17/3/95: Erreur inexpliquee sur Sun Solaris 5.3 (Grib1):
 *          la variable f n'est pas mise a la bonne valeur
 *          lors de l'appel de _mw_ftoa_(xx) si cette
 *          fonction n'est pas declaree suivant la norme ANSI !!
 *  A enlever dans une future version (tester par exemple
 *  la valeur retournee par le module fentropy)
 */

char *_mw_ftoa_(float f)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%g", (double)f);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

char *_mw_dtoa_(double d)
{
     char *ret;
     char buffer[BUFSIZ];
     sprintf(buffer, "%g", d);
     ret = (char *)malloc(strlen(buffer)+1);
     strcpy(ret, buffer);
     return ret;
}

int _mwis_open(char *s, char *rw)
{
     char fname[BUFSIZ];

     if (*rw == 'r') /* read */
     {
	 strcpy(fname,s);          /* do not change the value of s */
	 return(_search_filename(fname));
     }
     return(TRUE); /* checking diseable for writing */
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

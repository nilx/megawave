/**
 ** main of mwp
 ** (c)1993-2001 J.Froment - S.Parrino
 ** Version 1.4
 **/
/*~~~~~~~~~~~  This file is part of the MegaWave2 preprocessor ~~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*-- PORTABILITY NOTICE --
The call to the preprocessor may change
regarding the system. See CPPCMD below
and the Makefile notice.
-----------------------*/

/* Fichiers d'include */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ajout Jacques 22/3/93 */
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "io.h"
#include "bintree.h"
#include "symbol.h"
#include "mwarg.h"
#include "genmain.h"
#include "genifile.h"
#include "data_io.h"

/* Defines */
#define USERMWCNAME     "umwp"
#define ADMMWCNAME      "mwp"
#define MWC_FILE_PREFIX "mwp"
#define NAMEMODFILE     "MODNAM"

#define TMPDIR1          "/usr/tmp"  /* First choice */
#define TMPDIR2          "/tmp"      /* Second choice */
#define SRCDIR          "src"
#define FILEIONAME      "megawave2.io"
#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif

/* Functions */
#ifdef __STDC__
void usage(char *);
void init(int, char **);
#else
int usage();
int init();
#endif
/* Buffers only used in this file */
static char texfile[BUFSIZ];
static char i_intfile[BUFSIZ];
static char argfile[BUFSIZ];
#ifdef XMWP
static char xargfile[BUFSIZ];
static int xmwp_flg = FALSE;
#endif
static char modfile[BUFSIZ];
static char nmmodfile[BUFSIZ];
static char tmpdir[BUFSIZ];
static char bufcpp[BUFSIZ];
static char homedir[BUFSIZ];
char mwdir[BUFSIZ];
static char mwsysdir[BUFSIZ];
static char mwbufname[BUFSIZ];

/* Ajout Jacques 22/3/93 */
static struct stat fstat_buf;

/* Global variables */
char *progname;
char filein[BUFSIZ];
int lineno = 1;
FILE *mwerr = NULL, *fopen();
int ansifunc = FALSE;

extern char groupbuf[];

#ifdef XMWP
# ifdef DEBUG
#  define OPTS "awXdD:U:I:_:"
# else
#  define OPTS "awXD:U:I:_:"
# endif
#else
# ifdef DEBUG
#  define OPTS "awdD:U:I:_:"
# else
#  define OPTS "awD:U:I:_:"
# endif
#endif


main(argc, argv)
int argc;
char *argv[];
{
  extern FILE *yyin, *fopen();
  extern char *optarg;
  extern int optind;
  extern int errorcnt;
  extern int nowarning;
  int errflg, c;
#ifdef DEBUG
  extern int yydebug;
  extern char yyfilein[];
  yydebug = FALSE;
#endif

  prline(DISABLE);

  init(argc, argv);

  errflg = FALSE;
  nowarning = FALSE;
  ansifunc = FALSE;

  /* Analysis of MegaWave command line options */
  while ((c = getopt(argc, argv, OPTS)) != -1)
    switch (c) 
      {
#ifdef DEBUG
	/* Enable print debug */
      case 'd':
        yydebug = TRUE;
        break;
#endif
	/* ANSI main function declaration */
      case 'a':
	ansifunc = TRUE;
	break;
	/* Disable warning print */
      case 'w':
        nowarning = TRUE;
        break;
	/* Pass to CPP all define flags */
      case 'D':
        (void)strcat(bufcpp, " -D");
        (void)strcat(bufcpp, optarg);
        break;
	/* Pass to CPP all undefine flags */
      case 'U':
        (void)strcat(bufcpp, " -U");
        (void)strcat(bufcpp, optarg);
        break;
	/* Pass to CPP alternative include path */
      case 'I':
        (void)strcat(bufcpp, " -I");
        (void)strcat(bufcpp, optarg);
        break;
      /* Administrator flags */
      case '_' :
        switch (*optarg) {
          case 'T' :
            (void)strcpy(texfile, optarg+1);
            break;
          case 'I' :
            (void)strcpy(i_intfile, optarg+1);
            break;
          case 'A' :
            (void)strcpy(argfile, optarg+1);
            break;
#ifdef XMWP
          case 'X' :
            (void)strcpy(xargfile, optarg+1);
            xmwp_flg = TRUE;
            break;
#endif
          case 'M' :
            (void)strcpy(modfile, optarg+1);
            break;
          case 'N' :
            (void)strcpy(nmmodfile, optarg+1);
            break;
          default:
            error("'_%c %s' : Unknown MegaWave2 administrator option\n", *optarg,
                                                             optarg+1);
            mwcexit(1);
            break;
        }
        break;
#ifdef XMWP
        case 'X' :
          xmwp_flg = TRUE;
          break;
#endif
      /* Unknown MegaWave flag */
      case '?':
        errflg++;
        break;
    }

  /* Errors in option flags cause usage print and exit */
  if (errflg) {
    usage(progname);
    mwcexit (1);
  }

  /* Process files */
  if (optind == argc) {
    /* If no file name is present it's an error. */
    usage(progname);
    mwcexit (1);
  }
  else
    for (; optind < argc; optind++) {
      if ((yyin = fopen(argv[optind], "r")) != NULL) {
        char buffer[BUFSIZ];
        unsigned int status;
        strcpy(filein, argv[optind]);

/* Ajout Jacques 22/3/93 : pour generer dans la doc la date de modif. */
    fstat(fileno(yyin),&fstat_buf);
/*  */

        fclose(yyin);
        /* Create temporary path name for preprocessed MegaWave file */
        sprintf(buffer, "%s/mw%d.i", tmpdir, getpid());

/* Ajout Jacques 22/12/94 : si TMPDIR1 indisponible, essaye TMPDIR2
*/
	if ((yyin = fopen(buffer, "w")) == NULL)
	  {
	    strcpy(tmpdir,TMPDIR2);
	    sprintf(buffer, "%s/mw%d.i", tmpdir, getpid());
	    if ((yyin = fopen(buffer, "w")) == NULL)
	      fatal_error("%s : cannot open temporary input file %s\n", 
			  progname, buffer);
	  }
	fclose(yyin);

	/* Preprocess MegaWave file */
#ifdef CPPCMD
#ifdef Linux
	/* Call cpp with -undef to remove definitions as __GNUC__ 

	sprintf(buffer, "%s -undef -C %s %s %s/mw%d.i", CPPCMD, bufcpp, argv[optind], 
                                                              tmpdir, getpid());
	*/

	/* Note JF 23/02/01 : -undef cannot be used on Linux kernel 2.2.16-9
	   and it seems useless on kernel 2.0.36 : assuming that -undef was needed
	   by very old kernels only and remove this flag.
	*/
	sprintf(buffer, "%s -C %s %s %s/mw%d.i", CPPCMD, bufcpp, argv[optind], 
                                                              tmpdir, getpid());
#else
	sprintf(buffer, "%s -C %s %s %s/mw%d.i", CPPCMD, bufcpp, argv[optind], 
                                                              tmpdir, getpid());
#endif
#else
#ifdef IRIX
	sprintf(buffer, "cc -E -Wp,-showdefines -Wp,-C %s %s > %s/mw%d.i", bufcpp, argv[optind], 
			tmpdir, getpid());
#else
ERROR -- "Don't know how to process file on this architecture !" --
#endif
#endif
  /*fprintf(stderr,"exec %s\n", buffer); */

#ifdef DEBUG
	PRDBG("exec %s\n", buffer);
#endif
	status = system(buffer);
#ifdef DEBUG
	PRDBG("status = 0x%x\n", status);
#endif
	if (status != 0) {
	  sprintf(buffer, "%s/mw%d.i", tmpdir, getpid());
	  unlink(buffer);
	  fatal_error("Some problems in preprocessing ...\n");
	}
            
	/* Create internal representation of MegaWave program */
	printf("%s :\n", argv[optind]);
	sprintf(buffer, "%s/mw%d.i", tmpdir, getpid());
	if ((yyin = fopen(buffer, "r")) != NULL) {
#ifdef DEBUG
	  if (yydebug)
	    (void) strcpy(yyfilein, buffer);
#endif
	  prline(ENABLE);
	  yyparse();
	  prline(DISABLE);
	  fclose(yyin);
	  unlink(buffer);
          }
	else
	  fatal_error("%s : cannot open temporary input file %s\n",
		      progname, buffer);

      }
      else
	fatal_error("%s : cannot open file %s\n", progname, argv[optind]);
    }

  if (errorcnt == 0) {
    FILE *fd;
    char buffer[BUFSIZ];
    extern char modulename[];
  
    /* Change default (sub)group name if needed*/
    changegroup();

    /* Generate file where there is the module name */
    if ((fd = fopen(nmmodfile, "w")) != NULL) {
      fprintf(fd, "%s/%s\n", groupbuf, modulename);
      fclose(fd);
    }
    else
      fatal_error("%s : cannot create %s\n", progname, nmmodfile);
    
    /* Fill the internal MegaWave2 structure which describe usage field */
    fillmwarg();
    
    /* Generate arguments analysis file for MegaWave command */
    if ((fd = fopen(argfile, "w")) != NULL) {
      genmain(fd);
      fclose(fd);
    }
    else
      fatal_error("%s : cannot create %s\n", progname, argfile);

#ifdef XMWP
    if (xmwp_flg) {
      /* Generate XMegWave2 file for MegaWave command */
      if ((fd = fopen(xargfile, "w")) != NULL) {
        genxmain(fd);
        fclose(fd);
      }
      else
        fatal_error("%s : cannot create %s\n", progname, xargfile);
    }
#endif

    /* Generate interface file for MegaWave interpretor */
    if ((fd = fopen(i_intfile, "w")) != NULL) {
      genifile(fd);
      fclose(fd);
    }
    else
      fatal_error("%s : cannot create %s\n", progname, i_intfile);

    /* Generate TeX documentation squeleton */
    if (strlen(texfile) == 0)
      sprintf(buffer, "%s.tex", modulename);
    else
      strcpy(buffer, texfile);
    /* printf("texfile = %s\n", texfile);  */
    if ((fd = fopen(buffer, "w")) != NULL) 
      {
	/* Ajout Jacques 22/3/93 : pour generer dans la doc la date de modif. */
	gentex(fd,ctime(&(fstat_buf.st_mtime)));
	fclose(fd);
      }
    else
      fatal_error("%s : cannot create %s\n", progname, buffer);
  } else
    fatal_error("%d error detected. Stop.\n", errorcnt);

  if (errorcnt == 0) {
    /* Generate module file */
    FILE *fd;
    if ((fd = fopen(modfile, "w")) != NULL) {
      printpgm(fd);
      fclose(fd);
    }
    else
      fatal_error("%s : cannot create %s\n", progname, modfile);
      
  }
  else
    fatal_error("%d error detected. Stop.\n", errorcnt);

  if (errorcnt == 0) {
    printf("done.\n");
    mwcexit(0);
  }
  else
    fatal_error("%d error detected. Stop.\n", errorcnt);
}


#ifdef __STDC__
void usage(char *pgm)
#else
usage(pgm)
char *pgm;
#endif
{
  static char *ustr;

#ifdef XMWP
# ifdef DEBUG
  ustr = "-a -d -X -w -D<define> -U<undef> -I<includedir> file [...]";
# else
  ustr = "-a -w -X -D<define> -U<undef> -I<includedir> file [...]";
# endif
#else
# ifdef DEBUG
  ustr = "-a -d -w -D<define> -U<undef> -I<includedir> file [...]";
# else
  ustr = "-a -w -D<define> -U<undef> -I<includedir> file [...]";
# endif
#endif

  fprintf(stderr, "Usage : %s %s\n", pgm, ustr);
}

static struct var_env {
  char *name;
  char *buf;
  char *def;
} var_env[] = {
               {"HOME", homedir, NULL},
               {mwbufname, mwdir, NULL},
               {"TMPDIR", tmpdir, TMPDIR1},
               {"MEGAWAVE2", mwsysdir, NULL},
               {NULL}
              };


#ifdef __STDC__
void init(int argc, char *argv[])
#else
init(argc, argv)
int argc;
char *argv[];
#endif
{
  int i;
  FILE *fp;
  char wd[BUFSIZ], mwsrcdir[BUFSIZ], *pb, iobufname[BUFSIZ], *getcwd();
#ifdef DEBUG
  extern int yydebug;
#endif

  /* MegaWave output error */
#ifdef DEBUG
  mwerr = stdout;
#else
  mwerr = stderr;
#endif

  /* Command name */
  if ((progname = strrchr(argv[0], '/')) == NULL)
    progname = argv[0];
  else
    progname++;

  /* User or adm command ? */
  if (!strcmp(progname, USERMWCNAME))
    strcpy(mwbufname, "MY_MEGAWAVE2");
  else if (!strcmp(progname, ADMMWCNAME))
    strcpy(mwbufname, "MEGAWAVE2");
  else
    fatal_error("'%s' is not a valid name for MegaWave2 preprocessor\n",
                                                                      progname);

  /* Set shell environment variables */
  for (i = 0; var_env[i].name != NULL; i++) {
    char *pb;
    if ((pb = getenv(var_env[i].name)) != NULL)
      (void) strcpy(var_env[i].buf, pb);
    else if (var_env[i].def != NULL)
      (void) strcpy(var_env[i].buf, var_env[i].def);
    else
      error("'%s' must be set\n", var_env[i].name);
  }
#ifdef DEBUG
  if (yydebug)
    (void) strcpy(tmpdir, ".");
#endif

  /* Set defaults */
  texfile[0] = '\0';
  sprintf(argfile,   "%sA.c", MWC_FILE_PREFIX);
#ifdef XMWP
  sprintf(xargfile,  "%sX.c", MWC_FILE_PREFIX);
#endif
  sprintf(i_intfile, "%sI.c", MWC_FILE_PREFIX);
  sprintf(modfile,   "%sM.i", MWC_FILE_PREFIX);
  strcpy(nmmodfile, NAMEMODFILE);

  bufcpp[0] = '\0';

  /* Read IO file */
  sprintf(iobufname, "%s/%s", mwsysdir, FILEIONAME);
  if ((fp = fopen(iobufname, "r")) != NULL) {
    prline(ENABLE);
#ifdef DEBUG
    PRDBG("main : READ_DATA_IO\n");
#endif
    READ_DATA_IO(fp, iobufname);
    prline(DISABLE);
    fclose(fp);
  }
  else
    fatal_error("Cannot open '%s'\n", iobufname);
 
#ifndef TEST
  /* Find default group of command */
  sprintf(mwsrcdir, "%s/%s", mwdir, SRCDIR);
  if (getcwd(wd, sizeof(wd)) == NULL)
    fatal_error("Cannot get current working directory !\n");
  if ((pb = strstr(wd, mwsrcdir)) != NULL) 
    {
      int l;
      l = strlen(mwsrcdir);
      strcpy(groupbuf, (*(pb+l)=='\0') ? "." : pb+l+1);
    }
  else
    fatal_error("Your current directory \"%s\" is not valid.\nYou must develop under MegaWave2 directory \"%s\" or a subdir !\n", wd,mwsrcdir);
#else
  strcpy(groupbuf, ".");
#endif
}

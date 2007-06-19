/*~~~~~~~~~~~~~~ mwplight - The MegaWave2 Light Preprocessor ~~~~~~~~~~~~~~~~~~~

 Main of mwplight

 Author : Jacques Froment
 Date : 2007
 Version : 1.0
 Versions history :
   0.1 (August 2005, JF) initial internal release
   0.2 (February 2006, JF) added include <string.h> (Linux 2.6.12 & gcc 4.0.2) 
   1.0 (April, 2007) final revision, ready for external release
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~  This file is part of the MegaWave2 light preprocessor ~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>

#include "mwpl_main.h"


/*~~~~~ Global variables ~~~~~*/

/* Useful environment variables */
char MEGAWAVE2[BUFSIZ];
char MY_MEGAWAVE2[BUFSIZ];
char CWD[BUFSIZ]; /* Current working Directory as returned by getcwd() or /bin/pwd */
char PWD[BUFSIZ]; /* Current working Directory as returned by shell built-in pwd */

int adm_mode=0; /* Administrator mode. If set, module are compiled in
		   $MEGAWAVE2. If not, in $MY_MEGAWAVE2.
		*/
int inside_comment; /* 1 if parsing inside a comment, 0 elsewhere */
int inside_header;  /* 0 if inside C body; > 0 if inside header, the number
		       being the ID number of the header.
		    */
int inside_optionarg; /* 1 if parsing optional arguments in the usage but the last
			 one; 2 if parsing the last one; 0 if not yet inside optional
			 arguments; -1 if no more inside but one list was encountered. 
		      */

char module_file[BUFSIZ]; /* name of the input module file (with extension) */
char module_name[BUFSIZ]; /* name of the input module file (without extension) */
char group_name[BUFSIZ];  /* name of the group (from the current working dir.) */
char usagebuf[STRSIZE];   /* buffer storing the module's usage */
char protobuf[STRSIZE];   /* buffer storing the module's main function prototype (in K&R format) */

char Afile[BUFSIZ]=""; /* name of the output A-file (argfile: main of run-time command)*/
char Mfile[BUFSIZ]=""; /* name of the output M-file (modfile : source of modified module) */
char Tfile[BUFSIZ]=""; /* name of the output T-file (texfile : source of documented module) */
char Ifile[BUFSIZ]=""; /* name of the output I-file (intfile : interface file for an interpretor) */

FILE *fa=NULL; /* file pointer on the argument analysis file 
		  (A-file = source of run-time command ) */
FILE *fm=NULL; /* file pointer on the source of modified module to be put in library
		  (M-file) */
FILE *ft=NULL; /* file pointer on the texfile (T-file) */
FILE *fi=NULL; /* file pointer on the intfile (I-file) */


/*~~~~~ Local variables (used only in this file) ~~~~~ */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

main(argc, argv)
int argc;
char *argv[];
{
  int c;
  char Nfile[BUFSIZ]=""; /* name of the output N-file (nmodfile : group and name of the module) */
  FILE *fn=NULL;         /* file pointer on the N-file */

  /* Analysis of command line options */
  while ((c = getopt(argc, argv, "a_:")) != -1)
    switch (c) 
      {
      case 'a':
	adm_mode=1;
	break;

	/* output files specification */
      case '_' :
        switch (*optarg) 
	  {
          case 'A' :
            strcpy(Afile, optarg+1);
            break;
          case 'M' :
            strcpy(Mfile, optarg+1);
            break;
	  case 'N' :
            strcpy(Nfile, optarg+1);
            break;
	  case 'T' :
            strcpy(Tfile, optarg+1);
            break;
	  case 'I' :
            strcpy(Ifile, optarg+1);
            break;
	  default:
            Error("'_%c %s' : Unknown argument of -_ option\n", *optarg, optarg+1);
            exit(1);
            break;
	  }
        break;
	
	/* Unknown flag */
      case '?':
	Error("Invalid flag");
	break;
      }

  if (optind >= argc) /* No file name found */
    Error("Usage : Missing file name of source module");

  if (Afile[0]=='\0') Error("Usage : Missing A-file (-_A is needed)");
  if (Mfile[0]=='\0') Error("Usage : Missing M-file (-_M is needed)");

  /* Some initializations */
  strcpy(module_file,argv[optind]);
  rmpathextfilename(module_file,module_name);
  getenvironmentvar();
  getcurrentworkingdir();
  getgroupname();

  /* Parse the input module file and build the tree H */
  parse();

  /* Set the <protobuf> variable which prototypes the main function 
     using K&R convention.
     TO DO : generate directly both K&R and ANSI declarations
             and replace the main function declaration in the M-file
	     by this one (see comments of setprotobuf() for more info.).
  */
  setprotobuf();

  /* Generate the M-file */
  genMfile();

 /* Generate the T-file */
  genTfile();

  /* Generate the A-file */
  genAfile();

  /* Generate the I-file */
  genIfile();

  /* Generate the N-file */
  if (Nfile[0]!='\0')
    {
      if ((fn = fopen(Nfile, "w")) == NULL) 
	Error("Cannot open N-file '%s' for writing",Nfile);    
      fprintf(fn, "%s/%s\n", H->Group, H->Name);
      fclose(fn);
    }

  exit(0); /* Exit with error to avoid postprocessing */
}



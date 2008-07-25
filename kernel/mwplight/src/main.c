/*
 * main.c for megawave, section mwplight
 *
 * main function of mwplight
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2007)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "mwpl.h"
#include "io.h"
#include "parse.h"
#include "mfile.h"
#include "afile.h"
#include "ifile.h"
#include "tfile.h"
#include "main.h"

#define MSG_ERROR_OPEN_FILE \
     "Cannot open file '%s'"
#define MSG_DEBUG_PARAMS "\
sfile_name   : %s\n\
afile_name   : %s\n\
mfile_name   : %s\n\
nfile_name   : %s\n\
tfile_name   : %s\n\
ifile_name   : %s\n\
module_name  : %s\n\
module_group : %s"


#define VERSION "megawave 4 development version\n"
/* split the string, ISO C90 limits */
#define USAGE_1 "\
Usage: mwplight [options] filename\n\
Options:\n\
  -a, --afile file    name of the A-file (main function of the run-time)\n\
  -m, --mfile file    name of the M-file (source of the modified module)\n\
  -t, --tfile file    name of the T-file (source of the module documentation)\n\
  -i, --ifile file    name of the I-file (interface file for an interpretor)\n"
#define USAGE_2 "\
  -n, --nfile file    name of the N-file (group and name of the module)\n\
  -g, --group group   group the module belongs to (defaults to \"unknown\")\n\
  -v, --version       print version information\n\
  -d, --debug         debug flag\n\
  -h, -?, --help      print usage help\n"
#define USAGE_3 "\
\n\
  \"-\" can be used for standard input/output.\n"

static void usage(void)
{
     printf(VERSION);
     printf(USAGE_1);
     printf(USAGE_2);
     printf(USAGE_3);
}

static void version(void)
{
     printf(VERSION);
     printf("build %s\n", __DATE__);
}

int main(int argc, char ** argv)
{
     int c;
     char * short_options = "a:m:n:t:i:g:dvh?";
     struct option long_options[] =
          {
               {"afile",   required_argument, 0, 'a'},
               {"mfile",   required_argument, 0, 'm'},
               {"nfile",   required_argument, 0, 'n'},
               {"tfile",   required_argument, 0, 't'},
               {"ifile",   required_argument, 0, 'i'},
               {"group",   required_argument, 0, 'g'},
               {"ifile",   required_argument, 0, 'i'},
               {"version", no_argument,       0, 'v'},
               {"help",    no_argument,       0, 'h'},
               {"debug",   no_argument,       0, 'd'},
          };
     int option_index = 0;

     /*
      * file pointers on
       * - the argument analysis file (A-file = source of run-time)
      * - the source of modified module to be put in library (M-file)
      * - the texfile (T-file)
      * - the intfile (I-file)
      */
     FILE * sfile = NULL;
     FILE * afile = NULL;
     FILE * mfile = NULL;
     FILE * tfile = NULL;
     FILE * ifile = NULL;
     FILE * nfile = NULL;
     char * start;

     /*
      * names of the outputs
      * - A-file (argfile: main of run-time command)
      * - M-file (modfile : source of modified module)
      * - T-file (texfile : source of documented module)
      * - I-file (intfile : interface file for an interpretor)
      */
     char afile_name[BUFSIZ] = "";
     char mfile_name[BUFSIZ] = "";
     char tfile_name[BUFSIZ] = "";
     char ifile_name[BUFSIZ] = "";
     char nfile_name[BUFSIZ] = "";

     sfile_global = sfile;
     debug_flag = 0;
     strcpy(group_name, "unknown");

     /* no command-line options */
     if (argc < 2)
     {
          usage();
          exit(EXIT_FAILURE);
     }


     /* pick the command line options */
     while ((c = getopt_long (argc, argv, short_options,                \
                              long_options, &option_index)) != -1)
          switch (c)
          {
          case 'a':
               strcpy(afile_name, optarg);
               break;
          case 'm':
               strcpy(mfile_name, optarg);
               break;
          case 'n':
               strcpy(nfile_name, optarg);
               break;
          case 't':
               strcpy(tfile_name, optarg);
               break;
          case 'i':
               strcpy(ifile_name, optarg);
               break;
          case 'g':
               strcpy(group_name, optarg);
               break;
          case 'd':
               debug_flag = 1;
               break;
          case 'v':
               version();
               exit(0);
          case '?':
          case 'h':
               usage();
               exit(0);
          default:
               abort();
          }

     /* no module filename in the command-line */
     if (optind >= argc)
     {
          usage();
          exit(EXIT_FAILURE);
     }
     strcpy(sfile_name, argv[optind]);

     /* open the input file handlers */
     if (0 == strcmp("-", sfile_name))
     {
	  sfile = tmpfile();
	  while (EOF != (c = getc(stdin)))
	       putc(c, sfile);
	  rewind(sfile);
     }
     else
	  if (NULL == (sfile = fopen(sfile_name, "r")))
	       error(MSG_ERROR_OPEN_FILE, sfile_name);

     /* open the output file handlers */
     if (0 != strcmp("", afile_name))
     {
	  if (0 == strcmp("-", afile_name))
	       afile = stdout;
	  else
	       if (NULL == (afile = fopen(afile_name, "w")))
		    error(MSG_ERROR_OPEN_FILE, afile_name);
     }
     if (0 != strcmp("", mfile_name))
     {
	  if (0 == strcmp("-", mfile_name))
	       mfile = stdout;
	  else
	       if (NULL == (mfile = fopen(mfile_name, "w")))
		    error(MSG_ERROR_OPEN_FILE, mfile_name);
     }
     if (0 != strcmp("", tfile_name))
     {
	  if (0 == strcmp("-", tfile_name))
	       tfile = stdout;
	  else
	       if (NULL == (tfile = fopen(tfile_name, "w")))
		    error(MSG_ERROR_OPEN_FILE, tfile_name);
     }
     if (0 != strcmp("", ifile_name))
     {
	  if (0 == strcmp("-", ifile_name))
	       ifile = stdout;
	  else
	       if (NULL == (ifile = fopen(ifile_name, "w")))
		    error(MSG_ERROR_OPEN_FILE, ifile_name);
     }
     if (0 != strcmp("", nfile_name))
     {
	  if (0 == strcmp("-", nfile_name))
	       nfile = stdout;
	  else
	       if (NULL == (nfile = fopen(nfile_name, "w")))
		    error(MSG_ERROR_OPEN_FILE, nfile_name);
     }


     /*
      * extract the module name from its filename
      * - find the last '/' in sfile_name
      * - go to the next position if it exists
      * - stay at the beginning if not
      * - measure the length of the prefix without '.'
      * - copy to module_name
      */
     start = strrchr(sfile_name, '/');
     if (start == NULL)
          start = sfile_name;
     else
          start++;
     strncpy(module_name, start, strcspn(start, "."));

     /* case of a filename beginning with a '.' */
     if (module_name[0] == '\0')
          strcpy(module_name, "unknown");

     /* parse the input module file and build the tree H */
     sfile_global = sfile;
     parse(sfile);
     rewind(sfile);

     /* Set the <protobuf> variable which prototypes the main function
        using K&R convention.
        TO DO : generate directly both K&R and ANSI declarations
        and replace the main function declaration in the M-file
        by this one (see comments of setprotobuf() for more info.).
     */
     /* TODO : use only ANSI */
     setprotobuf();

     /*
      * generate
      * - the M-file
      * - the T-file
      * - the A-file
      * - the I-file
      * - the N-file
      */
     if (NULL != mfile)
     {
	  genMfile(mfile, sfile);
	  if (stdout != mfile)
	       fclose(mfile);
     }

     if (NULL != tfile)
     {
	  genTfile(tfile);
	  if (stdout != tfile)
	       fclose(tfile);
     }

     if (NULL != afile)
     {
	  genAfile(afile);
	  if (stdout != afile)
	       fclose(afile);
     }

     if (NULL != ifile)
     {
	  genIfile(ifile);
	  if (stdout != ifile)
	       fclose(ifile);
     }

     if (NULL != nfile)
     {
	  fprintf(nfile, "%s/%s\n", group_name, module_name);
	  if (stdout != nfile)
	       fclose(nfile);
     }

     if (stdin != sfile)
	  fclose(sfile);

     exit(0);
}

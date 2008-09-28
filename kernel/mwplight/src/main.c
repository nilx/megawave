/*
 * main.c
 *
 * main function of mwplight
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2007)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

/**
 * GENGETOPT BEGIN
 *
 * package "mwplight"
 * version "1.01"
 * purpose "Parse a megawave module"
 * usage   "mwplight [-s module-source] [output-options]"
 * description "This parser reads the megawave header of a module. It \
 * produces:
 * * a library file, with a modified version of the module C code for \
 *   inclusion in the megawave modules library
 * * a main file, with a main() function in C, for the stand-alone \
 *   module
 * * a documentation file, documenting the module syntax (interface) \
 *   in TeX
 * * an interface file, describing the module io, to design the \
 *   interface for some external code
 * * a name file, containing the name and group of the module
 * 
 * Author: Jacques Froment <jacques.froment@univ-ubs.fr>
 * "
 *
 * section "Input"
 * option "source"         s "module source file name"
 *         string             typestr="filename" default="-"
 *
 * section "Outputs (optional)"
 * option "library"        l  "library code file name"
 *         string              typestr="filename" optional
 * option "main"           m  "main executable file name"
 *         string              typestr="filename" optional
 * option "documentation"  d  "documentation file name"
 *         string              typestr="filename" optional
 * option "interface"      i  "interface file"
 *         string              typestr="filename" optional
 * option "name"           n  "name file name"
 *         string              typestr="filename" optional
 *
 * section "Misc"
 * option "module-name"    N  "module name"
 *         string              typestr="name" default="unknown" optional
 * option "group-name"     G  "module group name"
 *         string              typestr="name" default="unknown" optional
 * text ""
 * option "help"           h  "print help and exit" flag off
 * option "version"        V  "print version and exit" flag off
 * option "debug"          D  "debug flag" flag off
 *
 * text "
 * Use '-' for standard input/output.
 *  
 * This program is part of the megawave framework.
 * See http://megawave.cmla.ens-cachan.fr/ for details.
 * (C) 2008 CMLA, ENS Cachan, 94235 Cachan cedex, France."
 * GENGETOPT END
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mwpl.h"
#include "io.h"
#include "parse.h"
#include "mfile.h"
#include "afile.h"
#include "ifile.h"
#include "tfile.h"
#include "main.h"

/**
 * @brief Open a file for read or write, with a default file
 *
 * @param filename file name 
 * @param mode opening mode
 * @param default_file default file
 *
 * @param 
 */ 
static FILE * open_file(const char * filename, const char * mode, 
			FILE * default_file)
{
     FILE * file;

     if (0 == strcmp("-", filename))
	  return default_file;
     else
	  if (NULL == (file = fopen(filename, mode)))
	  {
	       fprintf(stderr, "mwplight: cannot open file '%s'\n", filename);
	       exit(1);
	  }
     return file;
}

#include "main_cmdline.h"
int main( int argc, char **argv)
{
     struct mw_args_info args_info;

     /* io file pointers */
     FILE * sfile = NULL; /* source    */
     FILE * afile = NULL; /* lib       */
     FILE * mfile = NULL; /* main      */
     FILE * tfile = NULL; /* doc       */
     FILE * ifile = NULL; /* interface */
     FILE * nfile = NULL; /* name      */

     char c;
     char * start;
     int length;
     char * module_name;
     char * group_name;
     int cmdline_err;

     /* parse the command-line */
     cmdline_err = mw_cmdline(argc, argv, &args_info);
     if (args_info.help_given)
	  mw_cmdline_print_help();
     if (args_info.version_given)
	  mw_cmdline_print_version();
     if (cmdline_err)
     {
	  printf("Command-line error. Use '--help' for details.\n");
	  exit(1);
     }

     debug_flag = args_info.debug_flag;

     if (debug_flag)
	  mw_cmdline_dump(stdout, &args_info);

     /*
      * if the module name was not given from the command-line (thus
      * having its default value) and the source file is not stdin, 
      * extract the module name from its filename
      * - find the last '/' in sfile_name
      * - go to the next position if it exists
      * - stay at the beginning if not
      * - measure the length of the prefix without '.'
      * - copy to module_name
      */
     if  ((!args_info.module_name_given)
	  && (0 != strcmp("-", args_info.source_arg)))
     {
	  start = strrchr(args_info.source_arg, '/');
	  if (start == NULL)
	       start = args_info.source_arg;
	  else
	       start++;
	  length = strcspn(start, ".");
	  module_name = (char *) malloc(sizeof(char) * (length + 1));
	  strncpy(module_name, start, length);
	  module_name[length] = '\0';
     }
     else
     {
	  length = strlen(args_info.module_name_arg);
	  module_name = (char *) malloc(sizeof(char) * (length + 1));
	  strcpy(module_name, args_info.module_name_arg);
     }

     length = strlen(args_info.group_name_arg);
     group_name = (char *) malloc(sizeof(char) * (length + 1));
     strcpy(group_name, args_info.group_name_arg);
  
     if (debug_flag)
	  printf("module name : '%s'\ngroup  name : '%s'\n", 
		 module_name, group_name);
     /* open the input file */
     sfile = open_file(args_info.source_arg, "r", stdin);

     /*
      * input is stdin : we store the data in a temporary file,
      * because we need to rewind on the source file
      */
     if (stdin == sfile)
     {
	  sfile = tmpfile();
	  while (EOF != (c = getc(stdin)))
	       putc(c, sfile);
	  rewind(sfile);
     }

     /* 
      * parse the source file and build the tree H (global variable)
      */
     sfile_global = sfile;
     parse(sfile);
     rewind(sfile);

     /*
      * set the <protobuf> variable (global) which prototypes the main
      * function using K&R convention.  
      */
     /* TODO: use only ANSI */
     setprotobuf();

     /*
      * generate the library code
      */
     afile = open_file(args_info.library_arg, "w", stdout);
     genAfile(afile);
     fclose(afile);

     /*
      * generate the main code
      */
     mfile = open_file(args_info.main_arg, "w", stdout);
     genMfile(mfile, sfile);
     fclose(mfile);

     /*
      * generate the documentation
      */
     tfile = open_file(args_info.documentation_arg, "w", stdout);
     genTfile(tfile);
     fclose(tfile);

     /*
      * generate the interface
      */
     ifile = open_file(args_info.interface_arg, "w", stdout);
     genIfile(ifile);
     fclose(ifile);

     /*
      * generate the name
      */
     nfile = open_file(args_info.name_arg, "w", stdout);
     fprintf(nfile, "%s/%s\n", group_name, module_name);
     fclose(nfile);

     fclose(sfile);
     free(module_name);
     free(group_name);
     exit(0);
}


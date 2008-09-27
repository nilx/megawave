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
 *   inclusion in the megawave modules library;
 * * a main file, with a main() function in C, for the stand-alone \
 *   module;
 * * a documentation file, documenting the module syntax (interface) \
 *   in TeX;
 * * an interface file, describing the module io, to design the \
 *   interface for some external code;
 * * a name file, containing the name and group of the module.
 * 
 * Author: Jacques Froment <jacques.froment@univ-ubs.fr>
 * "
 *
 * section "Input"
 * option "source"         s "module source file name"    optional
 *         string             typestr="filename" default="-"
 *
 * section "Outputs (optional)"
 * option "library"        l  "library code file name"    optional
 *         string              typestr="filename"
 *
 * option "exec"           e  "executable code file name" optional
 *         string              typestr="filename"
 *
 * option "documentation"  d  "documentation file name"   optional
 *         string              typestr="filename"
 *
 * option "interface"      i  "interface file"            optional
 *         string              typestr="filename"
 *
 * option "name"           n  "name file name"            optional
 *         string              typestr="filename"
 *
 * section "Misc"
 * option "module-name"    m  "module name"
 *         string              typestr="name" default="unknown" optional
 *
 * option "group-name"     g  "module group name"
 *         string              typestr="name" default="unknown" optional
 *
 * text ""
 * option "help"           h  "print help and exit" flag off
 * option "version"        V  "print version and exit" flag off
 * option "debug"          D  "debug flag" flag off
 *
 * text "
 * Use '-' for standard input/output. Default mode is to use stdin for
 * the source file, and no output activated. The output order doesn't
 * follow the options order (in case of multiple output to a single
 * file/pipe).
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
 * @return the file pointer
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

/**
 * @brief Strip directory and suffix part of a fule name
 *
 * - find the last '/' in sfile_name
 * - go to the next position if it exists
 * - stay at the beginning if not
 * - measure the length of the prefix without '.'
 * - copy
 *
 * @param filename file name 
 *
 * @return the base name, as an allocated string pointer
 */ 
static char * basename(const char * filename)
{
     char * base;
     const char * start;
     int length;

     start = strrchr(filename, '/');
     if (start == NULL)
	  start = filename;
     else
	  start++;
     length = strcspn(start, ".");
     base = (char *) malloc(sizeof(char) * (length + 1));
     strncpy(base, start, length);
     base[length] = '\0';
     
     return base;
}

/**
 * @brief Clone a string
 *
 * @param str string
 *
 * @return the cloned string, as an allocated string pointer
 */ 
static char * strclone(const char * str)
{
     char * clone;
     int length;

     length = strlen(str);
     clone = (char *) malloc(sizeof(char) * (length + 1));
     strcpy(clone, str);
     
     return clone;
}


/* TODO: cleanup */
extern char * module_name;
extern char * group_name;

#include "main_cmdline.h"
int main( int argc, char **argv)
{
     struct mw_args_info args_info;
     struct mw_cmdline_params args_params;

     /* io file pointers */
     FILE * sfile = NULL; /* source    */
     FILE * afile = NULL; /* exec      */
     FILE * mfile = NULL; /* lib       */
     FILE * tfile = NULL; /* doc       */
     FILE * ifile = NULL; /* interface */
     FILE * nfile = NULL; /* name      */

     char c;

     /* 1rst parsing of the command-line, searching -h or -v */
     args_params.check_required = 0;
     args_params.print_errors = 0;
     mw_cmdline_ext(argc, argv, &args_info, &args_params);
     if (args_info.help_given)
     {
	  mw_cmdline_print_help();
	  exit(0);
     }
     if (args_info.version_given)
     {
	  mw_cmdline_print_version();
	  exit(0);
     }

     /* 2nd parsing of the command-line, checking the options */
     args_params.check_required = 1;
     args_params.print_errors = 1;
     if (0 != mw_cmdline_ext(argc, argv, &args_info, &args_params))
     {
	  printf("Use '--help' for details.\n");
	  exit(1);
     }

     /* OK, use the params now */
     debug_flag = args_info.debug_flag;
     if (debug_flag)
     {
	  debug("command-line parameters:");
	  mw_cmdline_dump(stdout, &args_info);
     }

     /*
      * if the module name was not given from the command-line (thus
      * having its default value) and the source file is not stdin, 
      * extract the module name from its filename
      */
     if  ((!args_info.module_name_given)
	  && (0 != strcmp("-", args_info.source_arg)))
	  module_name = basename(args_info.source_arg);
     else
	  module_name = strclone(args_info.module_name_arg);

     group_name = strclone(args_info.group_name_arg);
  
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
     if (args_info.library_given)
     {
	  mfile = open_file(args_info.library_arg, "w", stdout);
	  genMfile(mfile, sfile);
	  fclose(mfile);
     }

     /*
      * generate the main code
      */
     if (args_info.exec_given)
     {
	  afile = open_file(args_info.exec_arg, "w", stdout);
	  genAfile(afile);
	  fclose(afile);
     }

     /*
      * generate the documentation
      */
     if (args_info.documentation_given)
     {
	  tfile = open_file(args_info.documentation_arg, "w", stdout);
	  genTfile(tfile);
	  fclose(tfile);
     }

     /*
      * generate the interface
      */
     if (args_info.interface_given)
     {
	  ifile = open_file(args_info.interface_arg, "w", stdout);
	  genIfile(ifile);
	  fclose(ifile);
     }

     /*
      * generate the name
      */
     if (args_info.name_given)
     {
	  nfile = open_file(args_info.name_arg, "w", stdout);
	  fprintf(nfile, "%s/%s\n", group_name, module_name);
	  fclose(nfile);
     }

     fclose(sfile);
     free(module_name);
     free(group_name);
     exit(0);
}


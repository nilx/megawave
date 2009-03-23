/**
 * @file mw-cmdline.h
 *
 * @author Nicolas Limare (2009)
 *
 * mw-cmdline API header
 */

#ifndef _MW_CMDLINE_H_
#define _MW_CMDLINE_H_

/* src/definitions.h */
struct Mwiline {
  char *name;
  int (*mwarg)(int, char**);
  void (*mwuse)(char *);
  char *group;
  char *function;
  char *usage;
};
typedef struct Mwiline Mwiline;

/* src/commandline.c */
int _mw_main(int argc, char *argv[], char *envp[], Mwiline mwicmd[], int mwind);
void mwdefopt(char *vers, Mwiline mwicmd[], int mwind);
int mw_opt_used(char c);
int _mwgetopt(int argc, char **argv, char *optstring);

#endif /* !_MW_CMDLINE_H_ */

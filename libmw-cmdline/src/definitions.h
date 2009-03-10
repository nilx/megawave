/**
 * @file definitions.h
 */

#ifndef _LIBMW_CMDLINE_DEFS_H_
#define _LIBMW_CMDLINE_DEFS_H_

struct Mwiline {
  char *name;
  int (*mwarg)(int, char**);
  int (*mwuse)(char *);
  char *group;
  char *function;
  char *usage;
};

typedef struct Mwiline Mwiline;

extern Mwiline mwicmd[];

#endif /* !_LIBMW_CMDLINE_DEFS_H_ */

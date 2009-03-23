/**
 * @file definitions.h
 */

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

struct Mwiline {
  char *name;
  int (*mwarg)(int, char**);
  void (*mwuse)(char *);
  char *group;
  char *function;
  char *usage;
};

typedef struct Mwiline Mwiline;

#endif /* !_DEFINITIONS_ */

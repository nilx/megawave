/*
 * module_io.h
 */

#ifndef _MODULE_IO_H_
#define _MODULE_IO_H_

/* src/module_io.c */
short _mw_load_submodules(FILE *fp, Module upm, char *line);
Modules _mw_load_modules(char *fname);
void _mw_write_submodules(FILE *fp, Module levelm, char *groupid);
short _mw_create_modules(char *fname, Modules modules);

#endif /* !_MODULE_IO_H_ */

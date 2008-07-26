/*
 * module_io.h
 */

#ifndef _MODULE_IO_H
#define _MODULE_IO_H

short _mw_load_submodules(FILE *, Module, char *);
Modules _mw_load_modules(char *);
void _mw_write_submodules(FILE *, Module, char *);
short _mw_create_modules(char *, Modules);

#endif /* !_MODULE_IO_H */

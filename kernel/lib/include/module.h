/*
 * module.h
 */

#ifndef _MODULE_H
#define _MODULE_H

Module mw_new_module(void);
Module mw_change_module(Module);
void mw_delete_module(Module);
Modules mw_new_modules(void);
Modules mw_change_modules(Modules);
void mw_delete_modules(Modules);

#endif /* !_MODULE_H */

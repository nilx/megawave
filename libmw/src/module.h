/*
 * module.h
 */

#ifndef _MODULE_H_
#define _MODULE_H_

/* src/module.c */
Module mw_new_module(void);
Module mw_change_module(Module module);
void mw_delete_module(Module module);
Modules mw_new_modules(void);
Modules mw_change_modules(Modules modules);
void mw_delete_modules(Modules modules);

#endif /* !_MODULE_H_ */

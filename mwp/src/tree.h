/*
 * tree.h
 */

#ifndef _TREE_H_
#define _TREE_H_

/* src/tree.c */
t_argument *new_arg(void);
t_header *new_header(void);
void CheckConsistencyH(void);
t_variable *new_variable(void);
t_varfunc *new_varfunc(void);
t_body *new_cbody(void);
void strdump_arg(char *str, t_argument *a);
void strdump_varfunc(char *str, t_varfunc *f);
t_token *new_cword(void);
t_statement *new_cinstruction(void);
void delete_cinstruction(t_statement *c);
void merge_cinstruction(t_statement *c);

#endif /* !_TREE_H_ */

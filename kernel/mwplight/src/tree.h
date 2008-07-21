/*
 * tree.h for megawave, section mwplight
 *
 * header for tree.c
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2005 - 2007)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

#ifndef _MWPL_TREE_H
#define _MWPL_TREE_H

t_argument * new_arg (void);
t_header * new_header (void);
void CheckConsistencyH (void);
t_variable * new_variable (void);
t_varfunc * new_varfunc (void);
t_body * new_cbody (void);
t_token * new_cword (void);
t_statement * new_cinstruction (void);
void delete_cinstruction (t_statement * c);
void merge_cinstruction (t_statement * c);
void strdump_arg (char * str, t_argument * a);
void strdump_varfunc (char * str, t_varfunc * f);

#endif /* !_MWPL_TREE_H */

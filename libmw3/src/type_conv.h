/*
 * type_conv.h
 */

#ifndef _TYPE_CONV_H_
#define _TYPE_CONV_H_

/* src/type_conv.c */
void *mw_conv_internal_type(void *mwstruct, char *typein, char *typeout);
void *_mw_load_etype_to_itype(char *fname, char *typein, char *typeout, char *Type);
short _mw_create_etype_from_itype(char *fname, void *mwstruct, char *typein, char *ftype);

#endif /* !_TYPE_CONV_H_ */

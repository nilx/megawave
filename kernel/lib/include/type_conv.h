/*
 * type_conv.h
 */

#ifndef _TYPE_CONV_H
#define _TYPE_CONV_H

void *mw_conv_internal_type(void *, char *, char *);
void *_mw_load_etype_to_itype(char *, char *, char *, char *);
short _mw_create_etype_from_itype(char *, void *, char *, char *);

#endif /* !_TYPE_CONV_H */

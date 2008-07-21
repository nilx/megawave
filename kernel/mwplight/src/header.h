/*
 * header.h for megawave, section mwplight
 *
 * header for header.c
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2007)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

#ifndef _MWPL_HEADER_H
#define _MWPL_HEADER_H

void GetHeaderStatement (char * s, char * name, char * value);
void AnalyseHeaderStatement (char * argclass, char * value);

#endif /* !_MWPL_HEADER_H */

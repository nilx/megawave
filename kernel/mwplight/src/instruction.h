/*
 * instruction.h for megawave, section mwplight
 *
 * header for instruction.c
 *
 * author : Jacques Froment <jacques.froment@univ-ubs.fr> (2007)
 * author : Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

#ifndef _MWPL_INSTRUCTION_H
#define _MWPL_INSTRUCTION_H

void Init_Cuserdatatype (void);
void Free_Cuserdatatype (void);
int GetNextInstruction (FILE * sfile);

#endif /* !_MWPL_INSTRUCTION_H */

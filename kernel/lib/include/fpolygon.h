/*
 * fpolygon.h
 */

#ifndef _FPOLYGON_H
#define _FPOLYGON_H

Fpolygon mw_new_fpolygon(void);
Fpolygon mw_alloc_fpolygon(Fpolygon,int);
Fpolygon mw_change_fpolygon(Fpolygon, int);
void mw_delete_fpolygon(Fpolygon);
unsigned int mw_length_fpolygon(Fpolygon);
Fpolygons mw_new_fpolygons(void);
Fpolygons mw_change_fpolygons(Fpolygons);
void mw_delete_fpolygons(Fpolygons);
unsigned int mw_length_fpolygons(Fpolygons);

#endif /* !_FPOLYGON_H */

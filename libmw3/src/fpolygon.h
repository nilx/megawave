/*
 * fpolygon.h
 */

#ifndef _FPOLYGON_H_
#define _FPOLYGON_H_

/* src/fpolygon.c */
Fpolygon mw_new_fpolygon(void);
Fpolygon mw_alloc_fpolygon(Fpolygon fpolygon, int nc);
Fpolygon mw_change_fpolygon(Fpolygon poly, int nc);
void mw_delete_fpolygon(Fpolygon fpolygon);
unsigned int mw_length_fpolygon(Fpolygon fpoly);
Fpolygons mw_new_fpolygons(void);
Fpolygons mw_change_fpolygons(Fpolygons poly);
void mw_delete_fpolygons(Fpolygons fpolygons);
unsigned int mw_length_fpolygons(Fpolygons fpolys);

#endif /* !_FPOLYGON_H_ */

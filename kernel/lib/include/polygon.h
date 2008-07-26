/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   polygon.h
   
   Vers. 1.3
   (C) 1993-2002 Jacques Froment
   Internal Input/Output polygon & polygons structures

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef polygon_flg
#define polygon_flg

Polygon mw_new_polygon(void);
Polygon mw_alloc_polygon(Polygon,int);
Polygon mw_change_polygon(Polygon, int);
void mw_delete_polygon(Polygon);
unsigned int mw_length_polygon(Polygon);
Polygons mw_new_polygons(void);
Polygons mw_change_polygons(Polygons);
void mw_delete_polygons(Polygons);
unsigned int mw_length_polygons(Polygons);

#endif

/*
 * polygon.h
 */

#ifndef _POLYGON_H_
#define _POLYGON_H_

/* src/polygon.c */
Polygon mw_new_polygon(void);
Polygon mw_alloc_polygon(Polygon polygon, int nc);
Polygon mw_change_polygon(Polygon poly, int nc);
void mw_delete_polygon(Polygon polygon);
unsigned int mw_length_polygon(Polygon poly);
Polygons mw_new_polygons(void);
Polygons mw_change_polygons(Polygons poly);
void mw_delete_polygons(Polygons polygons);
unsigned int mw_length_polygons(Polygons polys);

#endif /* !_POLYGON_H_ */

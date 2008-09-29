/*
 * polygon_io.h
 */

#ifndef _POLYGON_IO_H
#define _POLYGON_IO_H

Polygon _mw_load_polygon_a_poly(char *);
Polygon _mw_polygon_load_native(char *, char *);
Polygon _mw_load_polygon(char *, char *);
short _mw_create_polygon_a_poly(char *, Polygon);
short _mw_polygon_create_native(char *, Polygon, char *);
short _mw_create_polygon(char *, Polygon, char *);
Polygons _mw_load_polygons_a_poly(char *);
Polygons _mw_polygons_load_native(char *, char *);
Polygons _mw_load_polygons(char *, char *);
short _mw_create_polygons_a_poly(char *, Polygons);
short _mw_polygons_create_native(char *, Polygons, char *);
short _mw_create_polygons(char *, Polygons, char *);

#endif /* !_POLYGON_IO_H */

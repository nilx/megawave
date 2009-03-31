/*
 * polygon_io.h
 */

#ifndef _POLYGON_IO_H_
#define _POLYGON_IO_H_

/* src/polygon_io.c */
Polygon _mw_load_polygon_a_poly(char *fname);
Polygon _mw_polygon_load_native(char *fname, char *type);
Polygon _mw_load_polygon(char *fname, char *type);
short _mw_create_polygon_a_poly(char *fname, Polygon poly);
short _mw_polygon_create_native(char *fname, Polygon poly, char *Type);
short _mw_create_polygon(char *fname, Polygon poly, char *Type);
Polygons _mw_load_polygons_a_poly(char *fname);
Polygons _mw_polygons_load_native(char *fname, char *type);
Polygons _mw_load_polygons(char *fname, char *type);
short _mw_create_polygons_a_poly(char *fname, Polygons poly);
short _mw_polygons_create_native(char *fname, Polygons poly, char *Type);
short _mw_create_polygons(char *fname, Polygons poly, char *Type);

#endif                          /* !_POLYGON_IO_H_ */

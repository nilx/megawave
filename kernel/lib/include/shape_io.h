/*
 * shape_io.h
 */

#ifndef _SHAPE_IO_H
#define _SHAPE_IO_H

Shape _mw_load_mw2_shape(char *);
Shape _mw_load_shape(char *, char *);
void _mw_write_mw2_shape(FILE *, Shape, int);
short _mw_create_mw2_shape(char *, Shape);
short _mw_create_shape(char *, Shape, char *);
Shapes _mw_load_mw2_shapes_1_00(char *);
Shapes _mw_load_mw2_shapes(char *);
Shapes _mw_load_shapes(char *, char *);
short _mw_create_mw2_shapes(char *, Shapes);
short _mw_create_shapes(char *, Shapes, char *);

#endif /* !_SHAPE_IO_H */

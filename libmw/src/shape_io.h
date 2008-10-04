/*
 * shape_io.h
 */

#ifndef _SHAPE_IO_H_
#define _SHAPE_IO_H_

/* src/shape_io.c */
Shape _mw_load_mw2_shape(char *fname);
Shape _mw_load_shape(char *fname, char *Type);
void _mw_write_mw2_shape(FILE *fp, Shape sh, int iparent);
short _mw_create_mw2_shape(char *fname, Shape sh);
short _mw_create_shape(char *fname, Shape sh, char *Type);
Shapes _mw_load_mw2_shapes_1_00(char *fname);
Shapes _mw_load_mw2_shapes(char *fname);
Shapes _mw_load_shapes(char *fname, char *Type);
short _mw_create_mw2_shapes(char *fname, Shapes shs);
short _mw_create_shapes(char *fname, Shapes shs, char *Type);

#endif /* !_SHAPE_IO_H_ */

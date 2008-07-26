/*
 * mimage.h
 */

#ifndef _MIMAGE_H
#define _MIMAGE_H

Point_type mw_new_point_type(void);
Point_type mw_change_point_type(Point_type);
void mw_delete_point_type(Point_type);
Point_type mw_copy_point_type(Point_type, Point_type);

Morpho_line mw_new_morpho_line(void);
Morpho_line mw_change_morpho_line(Morpho_line);
void mw_delete_morpho_line(Morpho_line);
Morpho_line mw_copy_morpho_line(Morpho_line,Morpho_line);
unsigned int mw_length_morpho_line(Morpho_line);
unsigned int mw_num_morpho_line(Morpho_line);

Fmorpho_line mw_new_fmorpho_line(void);
Fmorpho_line mw_change_fmorpho_line(Fmorpho_line);
void mw_delete_fmorpho_line(Fmorpho_line);
Fmorpho_line mw_copy_fmorpho_line(Fmorpho_line,Fmorpho_line);
unsigned int mw_length_fmorpho_line(Fmorpho_line);

Hsegment mw_new_hsegment(void);
Hsegment mw_change_hsegment(Hsegment);
void mw_delete_hsegment(Hsegment);

Morpho_set mw_new_morpho_set(void);
Morpho_set mw_alloc_morpho_set(Morpho_set,int);
Morpho_set mw_change_morpho_set(Morpho_set);
void mw_delete_morpho_set(Morpho_set);
Morpho_set mw_copy_morpho_set(Morpho_set, Morpho_set);
unsigned int mw_length_morpho_set(Morpho_set);

Morpho_sets mw_new_morpho_sets(void);
Morpho_sets mw_change_morpho_sets(Morpho_sets);
void mw_delete_morpho_sets(Morpho_sets);
Morpho_sets mw_copy_morpho_sets(Morpho_sets, Morpho_sets);
unsigned int mw_length_morpho_sets(Morpho_sets);
unsigned int mw_num_morpho_sets(Morpho_sets);
void mw_morpho_sets_clear_stated(Morpho_sets);

Mimage mw_new_mimage(void);
Mimage mw_change_mimage(Mimage);
void mw_delete_mimage(Mimage);
Mimage mw_copy_mimage(Mimage,Mimage);
unsigned int mw_length_ml_mimage(Mimage);
unsigned int mw_length_fml_mimage(Mimage);
unsigned int mw_length_ms_mimage(Mimage);


#endif /* !_MIMAGE_H */

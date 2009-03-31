/*
 * mimage.h
 */

#ifndef _MIMAGE_H_
#define _MIMAGE_H_

/* src/mimage.c */
Point_type mw_new_point_type(void);
Point_type mw_change_point_type(Point_type point);
void mw_delete_point_type(Point_type point);
Point_type mw_copy_point_type(Point_type in, Point_type out);
Morpho_line mw_new_morpho_line(void);
Morpho_line mw_change_morpho_line(Morpho_line ll);
void mw_delete_morpho_line(Morpho_line morpho_line);
Morpho_line mw_copy_morpho_line(Morpho_line in, Morpho_line out);
unsigned int mw_length_morpho_line(Morpho_line morpho_line);
unsigned int mw_num_morpho_line(Morpho_line ml_first);
Fmorpho_line mw_new_fmorpho_line(void);
Fmorpho_line mw_change_fmorpho_line(Fmorpho_line ll);
void mw_delete_fmorpho_line(Fmorpho_line fmorpho_line);
Fmorpho_line mw_copy_fmorpho_line(Fmorpho_line in, Fmorpho_line out);
unsigned int mw_length_fmorpho_line(Fmorpho_line fmorpho_line);
Hsegment mw_new_hsegment(void);
Hsegment mw_change_hsegment(Hsegment segment);
void mw_delete_hsegment(Hsegment segment);
Morpho_set mw_new_morpho_set(void);
Morpho_set mw_change_morpho_set(Morpho_set is);
void mw_delete_morpho_set(Morpho_set morpho_set);
Morpho_set mw_copy_morpho_set(Morpho_set in, Morpho_set out);
unsigned int mw_length_morpho_set(Morpho_set morpho_set);
Morpho_sets mw_new_morpho_sets(void);
Morpho_sets mw_change_morpho_sets(Morpho_sets is);
void mw_delete_morpho_sets(Morpho_sets morpho_sets);
Morpho_sets mw_copy_morpho_sets(Morpho_sets in, Morpho_sets out);
unsigned int mw_length_morpho_sets(Morpho_sets morpho_sets);
unsigned int mw_num_morpho_sets(Morpho_sets mss_first);
void mw_morpho_sets_clear_stated(Morpho_sets mss_first);
Mimage mw_new_mimage(void);
Mimage mw_change_mimage(Mimage mi);
void mw_delete_mimage(Mimage mimage);
Mimage mw_copy_mimage(Mimage in, Mimage out);
unsigned int mw_length_ml_mimage(Mimage mimage);
unsigned int mw_length_fml_mimage(Mimage mimage);
unsigned int mw_length_ms_mimage(Mimage mimage);

#endif                          /* !_MIMAGE_H_ */

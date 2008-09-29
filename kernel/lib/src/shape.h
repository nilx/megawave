/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   shapes.h
   
   Vers. 1.2
   (C) 1999-2001 Pascal Monasse, Frederic Guichard, Jacques Froment.

   Internal Input/Output for the following FLST (Fast Level Set Transform)
   structures
     Point_plane
     Shape
     Shapes

Changes :
0.0: First version
0.1: minor modifications = some variable names (L.Moisan)
1.0: include "string_size.h" removed.
1.1: added data_size field in Shape and Shapes (L.Moisan), interpolation added.
1.2: no more modes for flst (L.Moisan)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef shape_flg
#define shape_flg

Point_plane mw_new_point_plane(void);
Point_plane mw_change_point_plane(Point_plane);
Shape mw_new_shape(void);
Shape mw_change_shape(Shape);
void mw_delete_shape(Shape);
Shape mw_get_not_removed_shape(Shape);
Shape mw_get_parent_shape(Shape);
Shape mw_get_first_child_shape(Shape);
Shape mw_get_next_sibling_shape(Shape);
Shape mw_get_smallest_shape(Shapes, int, int);
Shapes mw_new_shapes(void);
Shapes mw_alloc_shapes(Shapes,int,int,float);
Shapes mw_change_shapes(Shapes,int,int,float);     
void mw_delete_shapes(Shapes);

#endif


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   shapes.h
   
   Vers. 0.1
   (C) 1999 Pascal Monasse, Frederic Guichard, Jacques Froment.

   Internal Input/Output for the following FLST (Fast Level Set Transform)
   structures
     Point_plane
     Shape
     Shapes

Changes :
0.0: First version
0.1: minor modifications = some variable names (L.Moisan)

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

#ifdef SunOS
#include <sys/types.h>
#endif

#include "string_size.h"

#ifndef curve_flg
#include "curve.h"
#endif


/* Point in the plane */

typedef struct point_plane {
  short x,y; /* Coordinates of the point */
} *Point_plane;

/* A shape  : a connected component of a level set, with filled holes */
typedef struct shape
{
  char inferior_type; /* Indicates if it is extracted from a superior or inferior level set */
  float value; /* Limiting gray-level of the level set */
  char open; /* Indicates if the shape meets the border of the image */
  int area; /* Area of the shape = area of the cc of level set + areas of the holes */
  char removed; /* Indicates whether the shape exists or not */

  Point_plane pixels; /* The array of pixels contained in the shape */
  Curve boundary; /* The boundary curve defining the shape */
  /* Data to include it in a tree. It has a parent (the smallest containing shape), children
     (the largest contained shapes, whose first is pChild and the others are its siblings), and
     siblings (the other children of its parent) */
  struct shape *parent, *next_sibling, *child;
  /* User defined field. A pointer to something */
  void* data;
} *Shape;


/* A set of shapes (complete representation of an image) */
typedef struct shapes
{
  char cmt[mw_cmtsize];   /* Comments */
  char name[mw_namesize]; /* Name of the set */
  int nrow;               /* Number of rows (dy) of the image */
  int ncol;               /* Number of columns (dx) of the image */
  Shape the_shapes; /* Array of the shapes. The root of the tree is at index 0 */
  int nb_shapes;    /* The number of shapes (the size of the array the_shapes) */
  /* Link between pixels and shapes */
  Shape *smallest_shape; /* An image giving for each pixel the smallest shape containing it */
  /* User defined field. A pointer to something */
  void* data;
} *Shapes;


/* Modes of the flst: the flst is available in different modes. If you do not care about these
   subtilities, choose 0 as mode.
    1) We have to choose the connectedness for inf and sup level sets: they must have different notions
   of connectedness. If one is in 8-connectedness, the other is in 4-connectedness.
   The other options are meaningful only if the min area is > 1. In this case, that means that an extrema
   killer (also known as connected filter, or as Luc Vincent filter) is to be applied to the image before
   extracting the shapes. The advantage is that it reduces drastically the number of shapes and saves time
   and memory, even for small min areas (a few pixels). The drawback is that there are multiple versions of
   the extrema killer, which are slightly different.
   SUP INF (SI) is a sup-inf operator over all connected sets of the given min area. It kills some minima.
   INF SUP (IS) is an inf-sup operator over all connected sets of the given min area. It kills some maxima.
   SI and IS do not commute, so that we can apply SIoIS or ISoSI before extracting shapes, or just IS or SI.
   2) We have to choose if the last extrema killer before extraction of shapes is IS or SI. This is applied
   with the connectedness chosen in choice 1. For example, if we have chosen 4-connectedness for inf
   level sets, the minima killer (if chosen here) is applied with 4-connectedness.
   3) We have to choose if we apply a composed filter (ISoSI or SIoIS) or a simple filter (SI or IS).
   4) Notion of connectedness for the first filter in a composed filter. This parameter is meaningless
   if in 3) a simple filter is chosen.
   For a mode, make a combination of these different constants with a |, for example the mode 0 corresonds
   to MW_LS_4 | MW_LS_INF | MW_LS_COMPOSED | MW_LS_4_FIRST meaning:
   apply a composed Luc Vincent filter (MW_LS_COMPOSED), which is ISoSI (maxima killer before minima killer),
   with 4-connectedness for IS (MW_LS_4) and also 4-connected for SI (MW_LS_4_FIRST).
*/
#define MW_LS_4 0
#define MW_LS_8 1
#define MW_LS_INF 0
#define MW_LS_SUP 2
#define MW_LS_COMPOSED 0
#define MW_LS_UNCOMPOSED 4
#define MW_LS_4_FIRST 0
#define MW_LS_8_FIRST 8

/* Functions definition */

#ifdef __STDC__

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

#else

Point_plane mw_new_point_plane();
Point_plane mw_change_point_plane();
Shape mw_new_shape();
Shape mw_change_shape();
void mw_delete_shape();
Shape mw_get_not_removed_shape();
Shape mw_get_parent_shape();
Shape mw_get_first_child_shape();
Shape mw_get_next_sibling_shape();
Shape mw_get_smallest_shape();
Shapes mw_new_shapes();
Shapes mw_alloc_shapes();
Shapes mw_change_shapes();     
void mw_delete_shapes();

#endif

#endif


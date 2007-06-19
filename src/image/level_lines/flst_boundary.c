/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
 name = {flst_boundary}; 
 version = {"1.0"}; 
 author = {"Pascal Monasse"}; 
 function = {"Find boundary of shape"}; 
 usage = { 
    tree->pTree          "The tree of shapes",
    shape->pShape        "The shape whose boundary is to be computed",
    boundary<-pBoundary  "boundary computed (output Flist)"
}; 
*/ 

#include "mw.h"

extern void flst_pixels();


#define EAST  0
#define NORTH 1
#define WEST  2
#define SOUTH 3
typedef struct {
  short int x, y; /* Point coordinates */
  unsigned char cDirection; /* Direction followed when accessing the point */
} DualPoint;

#ifndef POINT_T
#define POINT_T
typedef struct {
  float x, y;
} point_t;
#endif

/* Is the point in the shape? */
char point_in_shape(x, y, pShape, pTree)
short int x, y;
Shape pShape;
Shapes pTree;
{
  Shape pShapePoint = pTree->smallest_shape[y*pTree->ncol+x];
  return (pShape->pixels <= pShapePoint->pixels &&
	  pShapePoint->pixels < pShape->pixels+pShape->area);
}

#define TURN_LEFT(dir) \
dir = (dir==NORTH ? WEST :\
      (dir==WEST ? SOUTH :\
      (dir==SOUTH ? EAST : NORTH)))
#define TURN_RIGHT(dir) \
dir = (dir==NORTH ? EAST :\
      (dir==EAST ? SOUTH :\
      (dir==SOUTH ? WEST : NORTH)))

/* Find the dual point following pDualPoint as we follow the shape boundary */
void find_next_dual_point(pDualPoint, pShape, pTree)
DualPoint* pDualPoint;
Shape pShape;
Shapes pTree;
{
  char bLeftIn, bRightIn;

  switch(pDualPoint->cDirection) {
  case NORTH:
    bLeftIn  = point_in_shape(pDualPoint->x-1, pDualPoint->y-1, pShape, pTree);
    bRightIn = point_in_shape(pDualPoint->x,   pDualPoint->y-1, pShape, pTree);
    if(bLeftIn && ! bRightIn)
      -- pDualPoint->y;
    else if(! bLeftIn && (! bRightIn || pShape->inferior_type)) {
      -- pDualPoint->x;
      TURN_LEFT(pDualPoint->cDirection);
    } else {
      ++ pDualPoint->x;
      TURN_RIGHT(pDualPoint->cDirection);
    }
    break;
  case WEST:
    bLeftIn  = point_in_shape(pDualPoint->x-1, pDualPoint->y,   pShape, pTree);
    bRightIn = point_in_shape(pDualPoint->x-1, pDualPoint->y-1, pShape, pTree);
    if(bLeftIn && ! bRightIn)
      -- pDualPoint->x;
    else if(! bLeftIn && (! bRightIn || pShape->inferior_type)) {
      ++ pDualPoint->y;
      TURN_LEFT(pDualPoint->cDirection);
    } else {
      -- pDualPoint->y;
      TURN_RIGHT(pDualPoint->cDirection);
    }
    break;
  case SOUTH:
    bLeftIn  = point_in_shape(pDualPoint->x,   pDualPoint->y, pShape, pTree);
    bRightIn = point_in_shape(pDualPoint->x-1, pDualPoint->y, pShape, pTree);
    if(bLeftIn && ! bRightIn)
      ++ pDualPoint->y;
    else if(! bLeftIn && (! bRightIn || pShape->inferior_type)) {
      ++ pDualPoint->x;
      TURN_LEFT(pDualPoint->cDirection);
    } else {
      -- pDualPoint->x;
      TURN_RIGHT(pDualPoint->cDirection);
    }
    break;
  case EAST:
    bLeftIn  = point_in_shape(pDualPoint->x, pDualPoint->y-1, pShape, pTree);
    bRightIn = point_in_shape(pDualPoint->x, pDualPoint->y,   pShape, pTree);
    if(bLeftIn && ! bRightIn)
      ++ pDualPoint->x;
    else if(! bLeftIn && (! bRightIn || pShape->inferior_type)) {
      -- pDualPoint->y;
      TURN_LEFT(pDualPoint->cDirection);
    } else {
      ++ pDualPoint->y;
      TURN_RIGHT(pDualPoint->cDirection);
    }
    break;
  }
}

/* Find the boundary of the shape, which is closed */
int find_closed_boundary(pTree, pShape, pBoundary)
Shapes pTree;
Shape pShape;
Flist pBoundary;
{
  short int x0, y0;
  DualPoint dualPoint;
  point_t* pPoint = (point_t*) pBoundary->values;
  short int iWidth = (short int)pTree->ncol, iHeight = (short int)pTree->nrow;

  /* 1) Find initial point, with NORTH direction */
  dualPoint.x = pShape->pixels[0].x; dualPoint.y = pShape->pixels[0].y;
  dualPoint.cDirection = NORTH;
  do ++ dualPoint.x;
  while(point_in_shape(dualPoint.x, dualPoint.y, pShape, pTree));
  
  /* 2) Follow the boundary */
  x0 = dualPoint.x; y0 = dualPoint.y;
  do {
    pPoint[pBoundary->size  ].x = dualPoint.x;
    pPoint[pBoundary->size++].y = dualPoint.y;
    find_next_dual_point(&dualPoint, pShape, pTree);
  } while(dualPoint.x != x0 || dualPoint.y != y0 ||
	  dualPoint.cDirection != NORTH);
  /* Close the boundary */
  pPoint[pBoundary->size  ].x = dualPoint.x;
  pPoint[pBoundary->size++].y = dualPoint.y;  
}

/* Find an initial point (to follow the boundary) at the border of the image */
void initial_point_border(pDualPoint, pShape, pTree)
DualPoint* pDualPoint;
Shape pShape;
Shapes pTree;
{
  short int iWidth = (short int)pTree->ncol, iHeight = (short int)pTree->nrow;
  short int x, y;

  /* Right border */
  pDualPoint->cDirection = WEST;
  x = iWidth-1; y = 0;
  if(point_in_shape(x, y++, pShape, pTree))
    while(y < iHeight && point_in_shape(x, y++, pShape, pTree));
  while(y < iHeight && ! point_in_shape(x, y, pShape, pTree))
    ++ y;
  if(y < iHeight) {
    pDualPoint->x = iWidth;
    pDualPoint->y = y;
    return;
  }
  /* Top border */
  pDualPoint->cDirection = SOUTH;
  x = 0; y = 0;
  if(point_in_shape(x++, y, pShape, pTree))
    while(x < iWidth && point_in_shape(x++, y, pShape, pTree));
  while(x < iWidth && ! point_in_shape(x, y, pShape, pTree))
    ++ x;
  if(x < iWidth) {
    pDualPoint->x = x;
    pDualPoint->y = 0;
    return;
  }
  /* Left border */
  pDualPoint->cDirection = EAST;
  x = 0; y = iHeight-1;
  if(point_in_shape(x, y--, pShape, pTree))
    while(y >= 0 && point_in_shape(x, y--, pShape, pTree));
  while(y >= 0 && ! point_in_shape(x, y, pShape, pTree))
    -- y;
  if(y >= 0) {
    pDualPoint->x = 0;
    pDualPoint->y = y+1;
    return;
  }
  /* Bottom border */
  pDualPoint->cDirection = NORTH;
  x = iWidth-1; y = iHeight-1;
  if(point_in_shape(x--, y, pShape, pTree))
    while(x >= 0 && point_in_shape(x--, y, pShape, pTree));
  while(x >= 0 && ! point_in_shape(x, y, pShape, pTree))
    -- x;
  if(x >= 0) {
    pDualPoint->x = x+1;
    pDualPoint->y = iHeight;
    return;
  }
  mwerror(FATAL, 1, "initial_point_border --> Coherency?");
}

/* Find an open boundary */
void find_open_boundary(pTree, pShape, pBoundary)
Shapes pTree;
Shape pShape;
Flist pBoundary;
{
  DualPoint dualPoint;
  point_t* pPoint = (point_t*) pBoundary->values;
  short int iWidth = (short int)pTree->ncol, iHeight = (short int)pTree->nrow;

  initial_point_border(&dualPoint, pShape, pTree);
  do {
    pPoint[pBoundary->size  ].x = dualPoint.x;
    pPoint[pBoundary->size++].y = dualPoint.y;
    find_next_dual_point(&dualPoint, pShape, pTree);
  } while(0 < dualPoint.x && dualPoint.x < iWidth &&
	  0 < dualPoint.y && dualPoint.y < iHeight);
  pPoint[pBoundary->size  ].x = dualPoint.x; /* We store the exit */
  pPoint[pBoundary->size++].y = dualPoint.y;
}

/*------------------------------ MAIN MODULE ------------------------------*/

/* Find boundary of the shape */
Flist flst_boundary(pTree, pShape, pBoundary)
Shapes pTree;
Shape pShape;
Flist pBoundary;
{
  int i;
  char bBuildBoundary;
  
  bBuildBoundary = (pBoundary==NULL);

  if(pTree->the_shapes[0].pixels == NULL)
    flst_pixels(pTree);

  pBoundary = mw_change_flist(pBoundary, 4*pShape->area+1, 0, 2);
  if(pBoundary == NULL)
    mwerror(FATAL, 1, "Boundary allocation error");

  if(pShape->open)
    find_open_boundary(pTree, pShape, pBoundary);
  else
    find_closed_boundary(pTree, pShape, pBoundary);

  /* free over-allocated bytes if pBoundary was created in this module) */
  if (bBuildBoundary) mw_realloc_flist(pBoundary, pBoundary->size);

  return(pBoundary);
}

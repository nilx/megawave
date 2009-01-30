/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
 name = {flst_bilinear}; 
 version = {"1.2"}; 
 author = {"Pascal Monasse"}; 
 function = {"Fast Level Sets Transform of a bilinear interpolated image"}; 
 usage = { 
    'a':min_area->pMinArea   "argument of the grain filter",
    image->pImageInput       "Input fimage",
    tree<-pTree              "The tree of shapes"
}; 
*/ 
/*----------------------------------------------------------------------
 v1.2: removed IMAGE_T macro (JF)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <float.h>
#include <assert.h>
#include "mw.h"
#include "mw-modules.h" /* for fsaddles() */


/* Optimization parameters. Probably should depend on image size, but these
values seem good for most images. */
#define INIT_MAX_AREA 10
#define STEP_MAX_AREA 8

#define NOT_A_SADDLE FLT_MAX /* Value must be coherent with fsaddles.c */
#define is_a_saddle(x) ((x) != NOT_A_SADDLE)

/* To deal with another type of image, just change these macros, and the call
to fsaddles */
#define PIXEL_T float
#define IMAGE_ALLOC_FUNC mw_change_fimage

/* A 'local configuration' of the pixel is the logical OR of these values,
stored in one byte. The bit is set if the neighbor is in the region */
#define EAST         1
#define NORTH_EAST   2
#define NORTH        4
#define NORTH_WEST   8
#define WEST        16
#define SOUTH_WEST  32
#define SOUTH       64
#define SOUTH_EAST 128

/* Gives for each configuration of the neighborhood around the pixel the number
of new connected components of the complement created (sometimes negative,
since cc's can be deleted), when the pixel is appended to the region. This
is the change in the number of 4-connected components of the complement.
Configurations are stored on one byte. */
static char tabPatterns[256] =
{ 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0,
  0, 1, 1, 1, 1, 2, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0,
  0, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 1, 2, 1, 1,
  0, 1, 1, 1, 1, 2, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0,
  0, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 1, 2, 1, 1,
  1, 2, 2, 2, 2, 3, 2, 2, 1, 2, 2, 2, 1, 2, 1, 1,
  0, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 1, 2, 1, 1,
  0, 1, 1, 1, 1, 2, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0,
  0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 2, 1, 1, 1, 1, 0,
  1, 1, 2, 1, 2, 2, 2, 1, 1, 1, 2, 1, 1, 1, 1, 0,
  1, 1, 2, 1, 2, 2, 2, 1, 2, 2, 3, 2, 2, 2, 2, 1,
  1, 1, 2, 1, 2, 2, 2, 1, 1, 1, 2, 1, 1, 1, 1, 0,
  0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 2, 1, 1, 1, 1, 0,
  1, 1, 2, 1, 2, 2, 2, 1, 1, 1, 2, 1, 1, 1, 1, 0,
  0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 2, 1, 1, 1, 1, 0,
  0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, -1};

/* A pixel or a saddle point */
typedef struct {
  short int x, y; /* For saddle point, north-east corner of dual pixel */
  char bSaddle; /* Pixel (0) or saddle point (1)? */
} PixelOrSaddle;

/* The structure representing the neighborhood of a region. It is a priority
queue organized as a heap (a binary tree, where each node has a key larger,
resp. smaller, than its two children), stored in an array.
Index 1 is the root, its children are at index 2 and 3.
Therefore, the parent of node at k is at k/2 and its children at 2*k and 2*k+1.
Index 0 is only a temporary buffer to swap two nodes. */
enum TypeOfTree {AMBIGUOUS, MIN, MAX, INVALID};
typedef struct {
  PixelOrSaddle point; /* Neighbor pixel or saddle point */
  PIXEL_T value; /* Its gray level */
} Neighbor;
typedef struct
{
  Neighbor* tabPoints; /* The array of neighbors, organized as a binary tree */
  int iNbPoints; /* The size of the previous arrays */
  enum TypeOfTree type; /* max- or min- oriented heap? */
  PIXEL_T otherBound; /* Min gray level if max-oriented, max if min-oriented */
} Neighborhood;

/* Structure to find connections between shapes. This is used when a monotone
section is extracted. The goal is to find the parent of its largest shape. The
gray level of the parent is known, so as a point of the parent, since we use
in fact an image of connections */
typedef struct {
  Shape shape; /* Largest shape of the monotone section */
  float level; /* Connection level */
} Connection;

/* Global variables. Ugly, but avoids weighing the code */
static int iWidth, iHeight;
static int iMinArea, iMaxArea, iAreaImage, iHalfAreaImage, iPerimeterImage;
static int iExploration; /* Used to avoid reinitializing images */
static PixelOrSaddle* tabPointsInShape;
static int** tabtabVisitedNeighbors; /* Exterior boundary */
static int** tabtabVisitedNeighborSaddles; /* Adjacent saddle points */
static Shapes pGlobalTree;
static int iAtBorder; /* #points meeting at border of image */

#define ARRAY_2D_ALLOC(array, iWidth, iHeight, type) \
(array) = (type**) malloc((iHeight)*sizeof(type*)); \
if((array) == NULL) mwerror(FATAL, 1, "ARRAY_2D_ALLOC --> memory"); \
(array)[0] = (type*) calloc((iWidth)*(iHeight), sizeof(type)); \
if((array)[0] == NULL) mwerror(FATAL, 1, "ARRAY_2D_ALLOC --> memory"); \
for(i = 1; i < (iHeight); i++) \
  (array)[i] = (array)[i-1]+(iWidth);

/* ------------------------------------------------------------------------
   --------- Functions to manage the neighborhood structure ---------------
   ------------------------------------------------------------------------ */

/* Reinitialise the neighborhood, so that it will be used for a new region */
static void reinit_neighborhood(Neighborhood *pNeighborhood, enum TypeOfTree type)
{
  pNeighborhood->iNbPoints = 0;
  pNeighborhood->type = type;
}

/* To allocate the structure representing the neighborhood of a region */
static void init_neighborhood(Neighborhood *pNeighborhood, int iMaxArea)
{
  iMaxArea = 4*(iMaxArea+1);
  if(iMaxArea > iWidth*iHeight)
    iMaxArea = iWidth*iHeight;

  pNeighborhood->tabPoints = (Neighbor*) malloc((iMaxArea+1)*sizeof(Neighbor));
  if(pNeighborhood->tabPoints == NULL)
    mwerror(FATAL, 1, "init_neighborhood --> neighbors allocation error\n");
  reinit_neighborhood(pNeighborhood, AMBIGUOUS);
}

/* Free the structure representing the neighborhood of a region */
static void free_neighborhood(Neighborhood *pNeighborhood)
{
  free(pNeighborhood->tabPoints);
}

#define ORDER_MAX(k,l) (tabNeighbors[k].value > tabNeighbors[l].value)
#define ORDER_MIN(k,l) (tabNeighbors[k].value < tabNeighbors[l].value)
#define SWAP(k,l) tabNeighbors[0] = tabNeighbors[k]; \
                  tabNeighbors[k] = tabNeighbors[l]; \
                  tabNeighbors[l] = tabNeighbors[0];
/* Put the last neighbor at a position so that we fix the heap */
static void fix_up(Neighborhood *pNeighborhood)
{
  Neighbor* tabNeighbors = pNeighborhood->tabPoints;
  int k = pNeighborhood->iNbPoints, l;
  
  if(pNeighborhood->type == MAX)
    while(k > 1 && ORDER_MAX(k, l=k>>1)) {
      SWAP(k, l);
      k = l;
    }
  else while(k > 1 && ORDER_MIN(k, l=k>>1)) {
    SWAP(k, l);
    k = l;
  }
}

/* Put the first neighbor at a position so that we fix the heap */
static void fix_down(Neighborhood *pNeighborhood)
{
  Neighbor* tabNeighbors = pNeighborhood->tabPoints;
  int N = pNeighborhood->iNbPoints, k = 1, l;

  if(pNeighborhood->type == MAX)
    while((l = k << 1) <= N) {
      if(l < N && ORDER_MAX(l+1,l)) ++l;
      if(ORDER_MAX(k,l))
	break;
      SWAP(k, l);
      k = l;
    }
  else while((l = k << 1) <= N) {
    if(l < N && ORDER_MIN(l+1,l)) ++l;
    if(ORDER_MIN(k,l))
      break;
    SWAP(k, l);
    k = l;
  }
}

/* Add the pixel (x,y), of gray-level VALUE, to the neighbor pixels */
static void add_neighbor(Neighborhood *pNeighborhood, short int x, short int y, float value, char bSaddle)
{
  Neighbor* pNewNeighbor;

  /* 0) Tag the pixel as 'visited neighbor' */
  if(bSaddle)
    tabtabVisitedNeighborSaddles[y][x] = iExploration;
  else
    tabtabVisitedNeighbors[y][x] = iExploration;
  /* 1) Update the fields TYPE and OTHERBOUND of PNEIGHBORHOOD */
  if(pNeighborhood->iNbPoints == 0)
    pNeighborhood->otherBound = value;
  else switch(pNeighborhood->type) {
  case MIN:
    if(value > pNeighborhood->otherBound)
      pNeighborhood->otherBound = value;
    else if(value < pNeighborhood->tabPoints[1].value)
      pNeighborhood->type = INVALID;
    break;
  case MAX:
    if(value < pNeighborhood->otherBound)
      pNeighborhood->otherBound = value;
    else if(value > pNeighborhood->tabPoints[1].value)
      pNeighborhood->type = INVALID;
    break;
  case AMBIGUOUS:
    if(value != pNeighborhood->tabPoints[1].value) {
      pNeighborhood->type =
	(value < pNeighborhood->tabPoints[1].value)? MAX: MIN;
      pNeighborhood->otherBound = value;
    }
    break;
  case INVALID:
    return;
  }
  if(pNeighborhood->type == INVALID)
    return;
  /* 2) Add the point to the heap and update it */
  pNewNeighbor = &pNeighborhood->tabPoints[++pNeighborhood->iNbPoints];
  pNewNeighbor->point.x = x; /* Initialise the new neighbor point */
  pNewNeighbor->point.y = y;
  pNewNeighbor->point.bSaddle = bSaddle;
  pNewNeighbor->value = value;
  fix_up(pNeighborhood); /* Update the heap of neighbors */
}

/* Remove  neighbor at the top of the heap, i.e., the root */
static void remove_neighbor(Neighborhood *pNeighborhood)
{
  Neighbor* pTop = &pNeighborhood->tabPoints[1];
  PIXEL_T value = pTop->value;

  if(pNeighborhood->type == INVALID)
    return;
  *pTop = pNeighborhood->tabPoints[pNeighborhood->iNbPoints--];
  if(pNeighborhood->iNbPoints == 0)
    return;
  fix_down(pNeighborhood);
  if(value != pTop->value && pTop->value == pNeighborhood->otherBound)
    pNeighborhood->type = AMBIGUOUS;
}

/* ------------------------------------------------------------------------
   --------- Allocations of structures used in the algorithm --------------
   ------------------------------------------------------------------------ */

/* Allocate image of the tags for visited pixels and the visited neighbors.
Do not be afraid about the parameters: pointers to 2-D arrays. */
static void init_image_of_visited_pixels(int ***ptabtabVisitedPixels, int ***ptabtabVisitedSaddles)
{
  int i;

  ARRAY_2D_ALLOC(*ptabtabVisitedPixels, iWidth, iHeight, int);
  ARRAY_2D_ALLOC(tabtabVisitedNeighbors, iWidth, iHeight, int);
  ARRAY_2D_ALLOC(*ptabtabVisitedSaddles, iWidth-1, iHeight-1, int);
  ARRAY_2D_ALLOC(tabtabVisitedNeighborSaddles, iWidth-1, iHeight-1, int);
}

static void free_image_of_visited_pixels(int **tabtabVisitedPixels, int **tabtabVisitedSaddles)
{
  free(tabtabVisitedPixels[0]); /* Actually a 1-D array */
  free(tabtabVisitedPixels);

  free(tabtabVisitedSaddles[0]);
  free(tabtabVisitedSaddles);

  free(tabtabVisitedNeighbors[0]);
  free(tabtabVisitedNeighbors);

  free(tabtabVisitedNeighborSaddles[0]);
  free(tabtabVisitedNeighborSaddles);
}

/* Initialize the output image */
static void init_output_image(float *tabPixelsIn, float ***ptabtabPixelsOutput)
{
  int i;

  *ptabtabPixelsOutput = (PIXEL_T**) malloc(iHeight * sizeof(PIXEL_T*));
  if(*ptabtabPixelsOutput == NULL)
    mwerror(FATAL, 1, "init_output_image --> allocation error\n");
  for(i = 0; i < iHeight; i++)
    (*ptabtabPixelsOutput)[i] = tabPixelsIn + i * iWidth;
}

static void free_output_image(float **tabtabPixelsOutput)
{
  free(tabtabPixelsOutput);
}

static void init_region(int iMaxArea)
{
  tabPointsInShape = (PixelOrSaddle*)malloc(iMaxArea * sizeof(PixelOrSaddle));
  if(tabPointsInShape == NULL)
    mwerror(FATAL, 1, "init_region --> impossible to allocate the array\n");
}

static void free_region(void)
{
  free(tabPointsInShape);
}

/* ------------------------------------------------------------------------
   --------- The core extraction algorithm --------------------------------
   ------------------------------------------------------------------------ */

/* Is pixel (x, y) a local minimum? */
static char is_local_min(float **ou, short int x, short int y)
              /* A 2-D array of the image */
               
{
  PIXEL_T v;
  char n = 0;

  v = ou[y][x];
  return (x==iWidth-1 || (ou[y][x+1]>v && ++n) || ou[y][x+1]==v) &&
    (x==0 || (ou[y][x-1]>v && ++n) || ou[y][x-1]==v) &&
    (y==iHeight-1 || (ou[y+1][x]>v && ++n) || ou[y+1][x]==v) &&
    (y==0 || (ou[y-1][x]>v && ++n) || ou[y-1][x]==v) &&
    n != 0;
}

/* Is pixel (x,y) a local maximum? */
static char is_local_max(float **ou, short int x, short int y)
              /* A 2-D array of the image */
               
{
  PIXEL_T v;
  char n = 0;

  v = ou[y][x];
  return (x==iWidth-1 || (ou[y][x+1]<v && ++n) || ou[y][x+1]==v) &&
    (x==0 || (ou[y][x-1]<v && ++n) || ou[y][x-1]==v) &&
    (y==iHeight-1 || (ou[y+1][x]<v && ++n) || ou[y+1][x]==v) &&
    (y==0 || (ou[y-1][x]<v && ++n) || ou[y-1][x]==v) && 
    n != 0;
}

/* Set pixels and saddle points in `tabPoints' at level newGray */
static void levelize(float **tabtabPixelsOutput, float **tabtabSaddleValues, PixelOrSaddle *tabPoints, int iNbPoints, float newGray)
{
  int i;
  for(i = iNbPoints - 1; i >= 0; i--)
    if(tabPoints[i].bSaddle)
      tabtabSaddleValues[tabPoints[i].y][tabPoints[i].x] = newGray;
    else
      tabtabPixelsOutput[tabPoints[i].y][tabPoints[i].x] = newGray;
}

/* Return, coded in one byte, the local configuration around the pixel (x,y) */
static unsigned char configuration(int **tabtabVisitedPixels, int **tabtabVisitedSaddles, short int x, short int y)
{
  short int iMaxX = iWidth-1, iMaxY = iHeight-1;
  unsigned char cPattern = 0;

  if(x != 0) {
    if(tabtabVisitedPixels[y][x-1] == iExploration)
      cPattern = WEST;
  } else if(iAtBorder)
    cPattern = SOUTH_WEST | WEST | NORTH_WEST;
  
  if(x != iMaxX) {
    if(tabtabVisitedPixels[y][x+1] == iExploration)
      cPattern |= EAST;
  } else if(iAtBorder)
    cPattern |= SOUTH_EAST | EAST | NORTH_EAST;
  
  if(y != 0) {
     if(tabtabVisitedPixels[y-1][x] == iExploration)
       cPattern |= NORTH;
  } else if(iAtBorder)
    cPattern |= NORTH_EAST | NORTH | NORTH_WEST;

  if(y != iMaxY) {
     if(tabtabVisitedPixels[y+1][x] == iExploration)
       cPattern |= SOUTH;
  } else if(iAtBorder)
    cPattern |= SOUTH_EAST | SOUTH | SOUTH_WEST;

#define DIAGONAL_IN_REGION(pixel, x, y, diag) ( \
  (pixel == iExploration && \
        ((cPattern&(diag)) || tabtabVisitedSaddles[y][x] == iExploration)) || \
  ((cPattern&(diag))==(diag) && tabtabVisitedSaddles[y][x] == iExploration) \
 )
  if(x != 0 && y != 0 &&
     DIAGONAL_IN_REGION(tabtabVisitedPixels[y-1][x-1], x-1, y-1, NORTH|WEST))
    cPattern |= NORTH_WEST;
  if(x != 0 && y != iMaxY &&
     DIAGONAL_IN_REGION(tabtabVisitedPixels[y+1][x-1], x-1, y, SOUTH|WEST))
    cPattern |= SOUTH_WEST;
  if(x != iMaxX && y != 0 &&
     DIAGONAL_IN_REGION(tabtabVisitedPixels[y-1][x+1], x, y-1, NORTH|EAST))
    cPattern |= NORTH_EAST;
  if(x != iMaxX && y != iMaxY &&
     DIAGONAL_IN_REGION(tabtabVisitedPixels[y+1][x+1], x, y, SOUTH|EAST))
    cPattern |= SOUTH_EAST;

  return cPattern;
}

/* Insert a new shape and its siblings in the tree, with parent pParent */
static void insert_children(Shape pParent, Shape pNewChildToInsert)
{
  Shape pSibling = pNewChildToInsert;
  while(pSibling->next_sibling != NULL) {
    pSibling->parent = pParent;
    pSibling = pSibling->next_sibling;
  }
  pSibling->parent = pParent;
  pSibling->next_sibling = pParent->child;
  pParent->child = pNewChildToInsert;
}

static Shape new_shape(int iCurrentArea, float currentGrayLevel, char bOfInferiorType, Shape pChild)
                 
                         
                     
              /* Supposed to have no sibling. Can be NULL */
{
  Shape pNewShape = &pGlobalTree->the_shapes[pGlobalTree->nb_shapes++];

  pNewShape->inferior_type = bOfInferiorType;
  pNewShape->value = currentGrayLevel;
  pNewShape->open = (char)(iAtBorder != 0);
  pNewShape->area = iCurrentArea;
  pNewShape->removed = 0;
  pNewShape->pixels = NULL;
  pNewShape->boundary = pNewShape->data = NULL;
  pNewShape->data_size = 0;
  /* Make links */
  pNewShape->parent = NULL;
  pNewShape->next_sibling = NULL;
  pNewShape->child = pChild;
  if(pChild != NULL)
    pChild->parent = pNewShape;
  return pNewShape;
}

/* Knowing that the last extracted shape contains the points, update,
for each one, the smallest shape containing it */
static void update_smallest_shapes(PixelOrSaddle *tabPoints, int iNbPoints)
{
  int i, iIndex;
  Shape pNewShape, pRoot = &pGlobalTree->the_shapes[0];

  pNewShape = &pGlobalTree->the_shapes[pGlobalTree->nb_shapes-1];
  for(i = iNbPoints - 1; i >= 0; i--)
    if(! tabPoints[i].bSaddle) {
      iIndex = tabPoints[i].y * iWidth + tabPoints[i].x;
      if(pGlobalTree->smallest_shape[iIndex] == pRoot)
	pGlobalTree->smallest_shape[iIndex] = pNewShape;
    }
}

/* Find children of the last constructed monotone section, which is composed
of the interval between pSmallestShape and the last extracted shape. That is,
find shapes in other monotone sections whose parent is inside this interval */
static void connect(PixelOrSaddle *tabPoints, int iNbPoints, Connection *tabConnections, Shape pSmallestShape)
{
  int i, iIndex;
  Shape pShape, pParent;
  float level;

  for(i = iNbPoints-1; i >= 0; i--) {
    iIndex = (tabPoints[i].y * iWidth + tabPoints[i].x) << 1;
    if(tabPoints[i].bSaddle) ++ iIndex;
    pShape = tabConnections[iIndex].shape;
    if(pShape != NULL) {
      level = tabConnections[iIndex].level;
      pParent = pSmallestShape;
      while(pParent->value != level)
	pParent = pParent->parent;
      insert_children(pParent, pShape);
      tabConnections[iIndex].shape = NULL;
    }
  }
}

/* Make a new connection structure at the given point */
static void new_connection(PixelOrSaddle *pPoint, float level, Connection *tabConnections)
{
  int iIndex;
  Shape pSibling, pShape = &pGlobalTree->the_shapes[pGlobalTree->nb_shapes-1];

  iIndex = (pPoint->y*iWidth + pPoint->x) << 1;
  if(pPoint->bSaddle) ++ iIndex;
  if(tabConnections[iIndex].shape == NULL) {
    tabConnections[iIndex].shape = pShape;
    tabConnections[iIndex].level = level;
  } else {
    pSibling = tabConnections[iIndex].shape;
    while(pSibling->next_sibling != NULL)
      pSibling = pSibling->next_sibling;
    pSibling->next_sibling = pShape;
  }
}

/* Is the neighbor pixel already stored for this exploration? */
#define NEIGHBOR_NOT_STORED(x,y) (tabtabVisitedNeighbors[y][x] < iExploration)

/* Store the 4-neighbors of pixel (x,y) */
static void store_4neighbors(float **ou, short int x, short int y, Neighborhood *pNeighborhood)
{
  if(x > 0         && NEIGHBOR_NOT_STORED(x-1,y))
    add_neighbor(pNeighborhood, x-1, y, ou[y][x-1], (char)0);
  if(x < iWidth-1  && NEIGHBOR_NOT_STORED(x+1,y))
    add_neighbor(pNeighborhood, x+1, y, ou[y][x+1], (char)0);
  if(y > 0         && NEIGHBOR_NOT_STORED(x,y-1))
    add_neighbor(pNeighborhood, x, y-1, ou[y-1][x], (char)0);
  if(y < iHeight-1 && NEIGHBOR_NOT_STORED(x,y+1))
    add_neighbor(pNeighborhood, x, y+1, ou[y+1][x], (char)0);
}

/* Store the neighbors of the saddle point (x,y) */
static void store_neighbors_to_saddle(float **ou, short int x, short int y, Neighborhood *pNeighborhood)
{
  if(NEIGHBOR_NOT_STORED(x,y))
    add_neighbor(pNeighborhood, x, y, ou[y][x], (char)0);
  if(NEIGHBOR_NOT_STORED(x+1,y))
    add_neighbor(pNeighborhood, x+1, y, ou[y][x+1], (char)0);
  if(NEIGHBOR_NOT_STORED(x,y+1))
    add_neighbor(pNeighborhood, x, y+1, ou[y+1][x], (char)0);
  if(NEIGHBOR_NOT_STORED(x+1,y+1))
    add_neighbor(pNeighborhood, x+1, y+1, ou[y+1][x+1], (char)0);  
}

/* Is the neighbor saddle point already stored? */
#define SADDLE_NOT_STORED(x,y) \
  (is_a_saddle(ou[y][x]) && tabtabVisitedNeighborSaddles[y][x] < iExploration)

/* Store the saddle points being neighbors of pixel (x,y) */
static void store_saddle_neighbors(float **ou, short int x, short int y, Neighborhood *pNeighborhood)
{
  if(x > 0) {
    if(y > 0         && SADDLE_NOT_STORED(x-1,y-1))
      add_neighbor(pNeighborhood, x-1, y-1, ou[y-1][x-1], (char)1);
    if(y < iHeight-1 && SADDLE_NOT_STORED(x-1,y))
      add_neighbor(pNeighborhood, x-1, y, ou[y][x-1], (char)1);
  }
  if(x < iWidth-1) {
    if(y > 0         && SADDLE_NOT_STORED(x,y-1))
      add_neighbor(pNeighborhood, x, y-1, ou[y-1][x], (char)1);
    if(y < iHeight-1 && SADDLE_NOT_STORED(x,y))
      add_neighbor(pNeighborhood, x, y, ou[y][x], (char)1);
  }
}

/* Does the addition of saddle point (x,y) change the Euler number? */
static char saddle_change(int **tabtabVisitedPixels, short int x, short int y)
{
  char cDiag1, cDiag2, cDiag3;

  cDiag1 = (tabtabVisitedPixels[y][x] == iExploration);
  if(cDiag1)
    cDiag2 = (tabtabVisitedPixels[y+1][x+1] == iExploration);
  else
    cDiag2 = (tabtabVisitedPixels[y+1][x+1] != iExploration);

  if(cDiag2) { /* Pixels of 1st diag inside (cDiag1) or outside (!cDiag1)? */
    cDiag2 = (tabtabVisitedPixels[y+1][x] == iExploration);
    if(cDiag2)
      cDiag3 = (tabtabVisitedPixels[y][x+1] == iExploration);
    else
      cDiag3 = (tabtabVisitedPixels[y][x+1] != iExploration);
    /* cDiag3: Pixels of 2nd diag inside (cDiag2) or outside (!cDiag2)? */
    if(cDiag3 && cDiag1 != cDiag2)
      return (char)1;
  }
  return 0;
}

/* Add the points in the neighborhood of gray level `currentGrayLevel' to the
region `tabPointsInShape' and return 1 if a new shape is detected. New points
are added from position `pCurrentArea'. This value is changed at exit in case
of success. */
static char add_iso_level(PixelOrSaddle *tabPointsInShape, int *pCurrentArea, int *pNbPixels, float currentGrayLevel, Neighborhood *pNeighborhood, float **ou, int **tabtabVisitedPixels, int **tabtabVisitedSaddles, float **tabtabSaddleValues)
{
  short int x, y;
  Neighbor* pNeighbor;
  int iCurrentArea, iNbHoles;
  unsigned char cPattern;

  iNbHoles = 0;
  iCurrentArea = *pCurrentArea;
  pNeighbor = &pNeighborhood->tabPoints[1];
  do { /* 1) Neighbor is added to the region */
    tabPointsInShape[iCurrentArea].x = x = pNeighbor->point.x;
    tabPointsInShape[iCurrentArea].y = y = pNeighbor->point.y;
    tabPointsInShape[iCurrentArea++].bSaddle = pNeighbor->point.bSaddle;
    if(! pNeighbor->point.bSaddle) {
      ++ *pNbPixels;
      cPattern = configuration(tabtabVisitedPixels, tabtabVisitedSaddles, x,y);
      iNbHoles += tabPatterns[cPattern];
      if(x == 0 || x == iWidth-1 || y == 0 || y == iHeight-1)
	++ iAtBorder;
      tabtabVisitedPixels[y][x] = iExploration;
    } else {
      if(saddle_change(tabtabVisitedPixels, x, y))
	++ iNbHoles;
      tabtabVisitedSaddles[y][x] = iExploration;
    }
    /* 2) Store new neighbors */
    if(! pNeighbor->point.bSaddle) {
      store_4neighbors(ou, x, y, pNeighborhood);
      store_saddle_neighbors(tabtabSaddleValues, x, y, pNeighborhood);
    } else
      store_neighbors_to_saddle(ou, x, y, pNeighborhood);
    remove_neighbor(pNeighborhood);
  } while(*pNbPixels <= iMaxArea &&
	  pNeighbor->value == currentGrayLevel &&
	  pNeighborhood->type != INVALID);

  if(*pNbPixels <= iMaxArea &&
     iAtBorder != iPerimeterImage &&
     (! iAtBorder || *pNbPixels <= iHalfAreaImage) &&
     pNeighborhood->type != INVALID &&
     iNbHoles == 0) {
    *pCurrentArea = iCurrentArea;
    return (char)1;
  }
  return 0;
}

/* Extract the terminal branch containing the point (x,y) */
static void find_terminal_branch(float **ou, int **tabtabVisitedPixels, int **tabtabVisitedSaddles, float **tabtabSaddleValues, short int x, short int y, Neighborhood *pNeighborhood, enum TypeOfTree type, Connection *tabConnections)
{
  PIXEL_T level;
  int iArea=0, iLastArea=0, iNbPixels=0;
  Neighbor* pNext = &pNeighborhood->tabPoints[1];
  Shape pSmallestShape=NULL, pLastShape=NULL;

  ++ iExploration;
  iAtBorder = 0; /* Shape does not meet the image border yet */
  level = ou[y][x];
  reinit_neighborhood(pNeighborhood, type);
  add_neighbor(pNeighborhood, x, y, level, (char)0);
  while(add_iso_level(tabPointsInShape, &iArea, &iNbPixels,
		      level, pNeighborhood, ou, tabtabVisitedPixels,
		      tabtabVisitedSaddles, tabtabSaddleValues) != 0) {
    if((type == MAX && level < pNext->value) ||
       (type == MIN && level > pNext->value)) { /* Type change */
      type = (level < pNext->value) ? MIN : MAX;
      if(pLastShape != NULL)
	connect(tabPointsInShape, iLastArea, tabConnections, pSmallestShape);
      pSmallestShape = NULL;
    }
    if(iMinArea <= iNbPixels) { /* Store new shape? */
      pLastShape = new_shape(iNbPixels, level, (char)(type==MIN), pLastShape);
      if(pSmallestShape == NULL) pSmallestShape = pLastShape;
      update_smallest_shapes(tabPointsInShape+iLastArea, iArea-iLastArea);
      iLastArea = iArea;
    }
    if(iAtBorder && iNbPixels == iHalfAreaImage)
      break;
    level = pNext->value;
  }
  if(pLastShape != NULL) {
    connect(tabPointsInShape, iArea, tabConnections, pSmallestShape);
    if(iAtBorder && iNbPixels == iHalfAreaImage)
      insert_children(pGlobalTree->the_shapes, pLastShape);
    else if(iArea != 0)
      new_connection(&tabPointsInShape[iArea], level, tabConnections);
  }
  levelize(ou, tabtabSaddleValues, tabPointsInShape, iArea, level);
}

/* Scan the image, calling a procedure to extract terminal branch at each
(not yet visited) local extremum */
static void scan(float **tabtabPixelsOutput, int **tabtabVisitedPixels, int **tabtabVisitedSaddles, float **tabtabSaddleValues, Neighborhood *pNeighborhood, Connection *tabConnections)
{
  short int i, j;
  enum TypeOfTree type;
  int iExplorationInit;

  iExplorationInit = iExploration;
  type = MIN;
  for(i = 0; i < iHeight; i++)
    for(j = 0; j < iWidth; j++)
      if(tabtabVisitedPixels[i][j] <= iExplorationInit &&
	 (is_local_min(tabtabPixelsOutput, j, i) ||
	  (is_local_max(tabtabPixelsOutput, j, i) && (type=MAX)==MAX))) {
	find_terminal_branch(tabtabPixelsOutput,
			     tabtabVisitedPixels, tabtabVisitedSaddles,
			     tabtabSaddleValues, j, i,
			     pNeighborhood, type, tabConnections);
	type = MIN;
      }
}

/* ------------------------------------------------------------------------
   --------- The main function --------------------------------------------
   ------------------------------------------------------------------------ */

/* The "Fast Level Set Transform" gives the tree of interiors of level lines
(named 'shapes') representing the image.
Only shapes of area >= *pMinArea are in the tree. pMinArea==NULL means 1.
Output: *pTree is filled (pTree must point to an allocated tree). */
void flst_bilinear(int *pMinArea, Fimage pImageInput, Shapes pTree)
{
  PIXEL_T** tabtabPixelsOutput; /* Array accessing pixels of output image */
  struct fimage imageSaddles;
  float** tabtabSaddleValues;
  Neighborhood neighborhood; /* Neighborhood of current region */
  int** tabtabVisitedPixels; /* Image of last visit for each pixel */
  int** tabtabVisitedSaddles;
  int iNbSaddles;
  Connection* tabConnections;
  int i;

  iWidth = pImageInput->ncol;
  iHeight = pImageInput->nrow;
  iAreaImage = iWidth * iHeight; iHalfAreaImage = (iAreaImage+1) / 2;
  if(iWidth == 1) iPerimeterImage = iHeight;
  else if(iHeight == 1) iPerimeterImage = iWidth;
  else iPerimeterImage = (iWidth+iHeight-2)*2;
  iMinArea = (pMinArea != NULL) ? *pMinArea : 1;
  if(iMinArea > iAreaImage) mwerror(USAGE, 1, "min area > image");

  pGlobalTree = mw_change_shapes(pTree, iHeight, iWidth, pImageInput->gray[0]);
  if(pGlobalTree == NULL)
    mwerror(FATAL, 1, "shapes allocation error");
  pTree->interpolation = 1;
  /* tabConnection[2*k] <-> pixel(k), tabConnection[2*k+1] <-> saddle(k) */
  tabConnections = (Connection*) malloc(2*iAreaImage * sizeof(Connection));
  if(tabConnections == NULL)
    mwerror(FATAL, 1, "allocation of image of connections error\n");
  for(i = 2*iAreaImage-1; i >= 0; i--)
    tabConnections[i].shape = NULL;

  init_output_image(pImageInput->gray, &tabtabPixelsOutput);
  imageSaddles.nrow = imageSaddles.ncol = 0; imageSaddles.gray = NULL;
  imageSaddles.allocsize = 0;
  iNbSaddles = fsaddles(pImageInput, &imageSaddles);
  if((tabtabSaddleValues = mw_newtab_gray_fimage(&imageSaddles)) == NULL)
    mwerror(FATAL, 1, "Lines of saddle values: allocation error");
  init_image_of_visited_pixels(&tabtabVisitedPixels, &tabtabVisitedSaddles);
  init_neighborhood(&neighborhood, iAreaImage + iNbSaddles);
  init_region(iAreaImage + iNbSaddles);
  /* Reallocation, the kernel allocating for a 0-order interpolation */
  pTree->the_shapes = (Shape)
    realloc(pTree->the_shapes, (iNbSaddles+iAreaImage+1)*sizeof(struct shape));
  for(i = iAreaImage-1; i >= 0; i--)
    pGlobalTree->smallest_shape[i] = pGlobalTree->the_shapes;

  iExploration = 0;
  /* Loop with increasing iMaxArea: speed optimization. Result correct
  with only one call to `scan' and iMaxArea = iAreaImage-1 */
  iMaxArea = 0;
  do {
    if(iMaxArea == 0)
      iMaxArea = INIT_MAX_AREA;
    else
      iMaxArea <<= STEP_MAX_AREA;
    if(iMaxArea == 0 || iMaxArea >= iHalfAreaImage) /* iMaxArea==0: overflow */
      iMaxArea = iAreaImage-1;
    scan(tabtabPixelsOutput, tabtabVisitedPixels, tabtabVisitedSaddles,
	 tabtabSaddleValues, &neighborhood, tabConnections);
  } while(iMaxArea+1 < iAreaImage);

  /* Make connections with root */
  pTree->the_shapes[0].value = tabtabPixelsOutput[0][0];
  for(i = 2*iAreaImage-1; i >= 0; i--)
    if(tabConnections[i].shape != NULL) {
      assert(tabConnections[i].level == pTree->the_shapes[0].value);
      insert_children(pGlobalTree->the_shapes, tabConnections[i].shape);
    }
  free(tabConnections);

  free_image_of_visited_pixels(tabtabVisitedPixels, tabtabVisitedSaddles);
  free_region();
  free_neighborhood(&neighborhood);
  free_output_image(tabtabPixelsOutput);
  free(tabtabSaddleValues[0]); free(tabtabSaddleValues);
}

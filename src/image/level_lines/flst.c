/*----------------------------- MegaWave Module -----------------------------*/ 
/* mwcommand 
  name = {flst}; 
  version = {"1.0"}; 
  author = {"Pascal Monasse, Frederic Guichard"}; 
  function = {"Fast Level Sets Transform of an image"}; 
  usage = { 
    'a': min_area-> pMinArea "argument of the extrema killer", 
    'm':[mode=0]->pMode "possible modes (default 0). See doc for details", 
    'l'-> pStoreCurves "flag to store also the associated level lines", 
    image->pFloatImageInput  "Input fimage", 
    tree<- pTree "The tree of shapes" 
    }; 
*/ 
/*--- MegaWave2 - Copyright (C)1994 Jacques Froment. All Rights Reserved. ---*/ 
 
#include <math.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
 
/* Include always the MegaWave2 Library */ 
#include "mw.h" 
 
/* This is because the parser of mw2 does not understand 3.40282347e+38F of math.h in RedHat6.0 */ 
#define FLT_MAX ((float)3.40282347e+38) 
 
/* This is the maximum total number of holes in the shapes that we expect */ 
#define MAXHOLES 3000000 
 
/* The directions for the frontier of a connected component of level set. 
   These are the values of the pixels of the oriented frontier. Each connected component 
   of the frontier is oriented such that the set is at the right when we pass in the indicated 
   direction. Notice that each pixel can take a combination of these values (a logical OR), but 
   horizontal (resp. vertical) directions are mutually exclusive */ 
#define UP 1 
#define LEFT 4 
#define DOWN 16 
#define RIGHT 64 
 
/* The structure to encode the configuration of the frontier of a shape. The member cDirections represents 
   the directions of separation between this pixel and up and left neighbors. The orientations indicate 
   in which side of the direction is the interior of the shape. The configurations: (pixels in: *, out: o) 
   -------------------------------------------------------------------------------------------- 
   |      o             *             *                       o                               | 
   |    * o -> DOWN   o o -> LEFT   * o  -> LEFT | DOWN     o o  -> 0 (not a frontier pixel)  | 
   |                                                                                          | 
   |      *             o             o                       *                               | 
   |    o * -> UP     * * -> RIGHT  o *  -> UP | RIGHT      * *  -> 0 (not a frontier pixel)  | 
   -------------------------------------------------------------------------------------------- 
    Notice that this way, we need to have an image of frontiers of 1 line and 1 column larger than 
    the original image. 
    We keep an index iExploration in memory, which is used to know when the pixel was initialised for the 
    last time (so that we do not need to clear the image of frontiers at each new shape). 
    In the same spirit, iExplorationHole is used to dicriminate the last time the pixel was explored (shapes 
    starting from the same local extremum have the same index iExploration, so this iExplorationHole is 
    used when we need to follow the frontier, for shapes with hole). cCurrentDirections is the current 
    configuration of the frontier, that is cDirections minus the directions already explored (we erase the 
    directions already explored). */ 
struct FlstFrontierPixel 
{ 
  int iExploration; /* To know the last time this frontier pixel was initialised (for which local extremum)*/ 
  unsigned char cDirections; /* The coded local configuration of the frontier of the level set. */ 
  /* Data used to follow the frontier */ 
  int iExplorationHole; /* To discriminate the shapes starting from the same local extremum in the image */ 
  unsigned char cCurrentDirections; /* Idem to cDirections but updated to erase already visited directions */ 
}; 
 
/* These (diagonal) directions are not directions of frontiers (invalid values for the pixels of frontiers), 
   but are used to discriminate the local configurations of the frontier (to count the number of cc of 
   the frontier), indicating the state of the diagonal pixels (in or out the shape). */ 
#define UPLEFT 2 
#define LEFTDOWN 8 
#define DOWNRIGHT 32 
#define RIGHTUP 128 
 
/* This is just half a direction: it is used when starting to follow a 
 * cc of the frontier. 
 */ 
#define HALFUP 2 
 
/* To know if a neighbor pixel was already visited for this local extremum */ 
#define NOTVISITED(x,y) (visitefront[y][x] < iIndexOfExploration) 
 
#define MIN(x,y) ( ((x) <= (y))? (x):(y) ) 
#define MAX(x,y) ( ((x) >= (y))? (x):(y) ) 
 
/* The structure representing the neighborhood of a region. It is made of all the neighbors and of a 
   smart indexing structure to access easily the ones with largest, resp. smallest, gray-value. It is a priority 
   queue organized as a heap (a binary tree, where each node has a key larger, resp. smaller, than its two children), 
   stored in an array. Index 0 of the array is not used, index 1 is the root, its children are at index 2 and 3. 
   Therefore, the parent of node at index k is at index k/2 and its children at indices 2*k and 2*k+1. Index 0 is 
   used only as a temporary buffer to swap two nodes. */ 
struct FlstNeighborhood 
{ 
  struct FlstNeighbor 
    { 
      struct point_plane point; /* The neighbor pixel */ 
      float value; /* Its gray level */ 
    } *tabNeighbors; /* The array of neighbors, organized as a binary tree */ 
  int iNbPoints; /* The size of the previous arrays */ 
  char bMaximumTree; /* Indicates if it is a maximum- or minimum- oriented heap */ 
}; 
 
/* This is the structure for a hole. It is a point in the hole, a pointer to the smallest shape containing 
   this hole, and a pointer to the shape of the hole. The link between container and contained shapes */ 
struct LsPointHole 
{ 
  struct point_plane point; /* A point in the hole */ 
  Shape pContainingShape; /* The shape containing this hole */ 
  Shape pShapeOfHole; /* The shape corresponding to the hole */ 
}; 
 
/* Well, here are global variables. It is not very beautiful, but it avoids to weigh the code too much, 
   since they are used almost everywhere */ 
static int iWidth, iHeight, iMinArea, iMaxArea, iAreaImage, iHalfAreaImage; 
static int iIndexOfExploration; 
static float** tabtabPixelsOriginalImage; /* To access easily to the pixels of the original image */ 
static struct point_plane* tabPointsInCurrentRegion; 
static int **visitefront; /* An image indicating the points belonging to the exterior frontier */ 
static struct FlstFrontierPixel** tabtabFrontierPixels; 
/* Gives for each configuration of the frontier around the pixel the number of new connected components 
   of the complementary created (sometimes negative, since cc can be deleted) */ 
static int tabPattern4[256], tabPattern8[256]; 
/* Gives for each configuration of the frontier the direction to follow to remain on the same connected 
   component of the frontier */ 
static unsigned char tabChoiceDirection4[256], tabChoiceDirection8[256]; 
 
static Shapes pGlobalTree; 
 
static struct LsPointHole* tabHoles; 
static int iNbHoles; 
 
static char bDoesShapeMeetBorder; 
static char bStoreExternalLine; 
static struct point_plane* tabNodesOfCurve; 
 
/* -------------------------------------------------------------------------------------- 
   --------- Functions to manage the neighborhood structure ----------------------------- 
   -------------------------------------------------------------------------------------- */ 
 
/* Reinitialise the neighborhood, so that it will be used for a new region */ 
static void flst_reinit_neighborhood(pNeighborhood) 
struct FlstNeighborhood* pNeighborhood; 
{ 
  pNeighborhood->iNbPoints = 0; 
} 
 
/* To allocate the structure representing the neighborhood of a region */ 
static void init_neighborhood(pNeighborhood) 
struct FlstNeighborhood* pNeighborhood; 
{ 
  pNeighborhood->tabNeighbors = (struct FlstNeighbor*) malloc((iMaxArea+1) * sizeof(struct FlstNeighbor)); 
  if(pNeighborhood->tabNeighbors == NULL) 
    mwerror(FATAL, 1, "init_neighborhood --> impossible to allocate the array of neighbors\n"); 
  flst_reinit_neighborhood(pNeighborhood); 
} 
 
/* Free the structure reprensenting the neighborhood of a region */ 
static void free_neighborhood(pNeighborhood) 
struct FlstNeighborhood* pNeighborhood; 
{ 
  free(pNeighborhood->tabNeighbors); 
} 
 
/* Put the last neighbor at a position so that we fix the heap */ 
static void flst_fix_up(pNeighborhood) 
struct FlstNeighborhood* pNeighborhood; 
{ 
  struct FlstNeighbor *tabNeighbors = pNeighborhood->tabNeighbors; 
  int k = pNeighborhood->iNbPoints, l; 
   
  if(pNeighborhood->bMaximumTree) 
    while(k > 1 && tabNeighbors[k].value > tabNeighbors[l = k>>1].value) 
      { 
	tabNeighbors[0] = tabNeighbors[k]; 
	tabNeighbors[k] = tabNeighbors[l]; 
	tabNeighbors[l] = tabNeighbors[0]; 
	k = l; 
      } 
  else 
    while(k > 1 && tabNeighbors[k].value < tabNeighbors[l = k>>1].value) 
      { 
	tabNeighbors[0] = tabNeighbors[k]; 
	tabNeighbors[k] = tabNeighbors[l]; 
	tabNeighbors[l] = tabNeighbors[0]; 
	k = l; 
      }     
} 
 
/* Put the first neighbor at a position so that we fix the heap */ 
static void flst_fix_down(pNeighborhood) 
struct FlstNeighborhood* pNeighborhood; 
{ 
  struct FlstNeighbor *tabNeighbors = pNeighborhood->tabNeighbors; 
  int N = pNeighborhood->iNbPoints, k = 1, l; 
 
  if(pNeighborhood->bMaximumTree) 
    while((l = k << 1) <= N) 
      { 
	if(l < N && tabNeighbors[l].value < tabNeighbors[l+1].value) ++l; 
	if(tabNeighbors[k].value >= tabNeighbors[l].value) 
	  break; 
	tabNeighbors[0] = tabNeighbors[k]; 
	tabNeighbors[k] = tabNeighbors[l]; 
	tabNeighbors[l] = tabNeighbors[0]; 
	k = l; 
      } 
  else 
    while((l = k << 1) <= N) 
      { 
	if(l < N && tabNeighbors[l].value > tabNeighbors[l+1].value) ++l; 
	if(tabNeighbors[k].value <= tabNeighbors[l].value) 
	  break; 
	tabNeighbors[0] = tabNeighbors[k]; 
	tabNeighbors[k] = tabNeighbors[l]; 
	tabNeighbors[l] = tabNeighbors[0]; 
	k = l; 
      } 
} 
 
/* Add the pixel (x,y), of gray-level value, to the neighbor pixels */ 
static void flst_add_neighbor(pNeighborhood, x, y, value) 
struct FlstNeighborhood* pNeighborhood; 
short int x, y; 
float value; 
{ 
  struct FlstNeighbor* pNewNeighbor; 
 
  visitefront[y][x] = iIndexOfExploration; 
  pNewNeighbor = &pNeighborhood->tabNeighbors[++pNeighborhood->iNbPoints]; 
  pNewNeighbor->point.x = x; /* Initialise the new neighbor point */ 
  pNewNeighbor->point.y = y; 
  pNewNeighbor->value = value; 
  flst_fix_up(pNeighborhood); 
} 
 
/* Remove the neighbor at the top of the heap, that is the root of the tree. */ 
static void flst_remove_neighbor(pNeighborhood) 
struct FlstNeighborhood* pNeighborhood; 
{ 
  pNeighborhood->tabNeighbors[1] = pNeighborhood->tabNeighbors[pNeighborhood->iNbPoints--]; 
  flst_fix_down(pNeighborhood); 
} 
 
/* -------------------------------------------------------------------------------------- 
   --------- Allocations of structures used in the algorithm ---------------------------- 
   -------------------------------------------------------------------------------------- */ 
 
/* Allocate the image of the tags for the visited pixels and the visited neighbor pixels. 
   Do not be afraid about the parameter: it is simply a pointer to a 2-D array representing 
   an image (because this image is allocated here) */ 
static void init_image_of_visited_pixels(ptabtabImageOfVisitedPixels) 
int*** ptabtabImageOfVisitedPixels; 
{ 
  int i; 
 
  *ptabtabImageOfVisitedPixels = (int**) malloc(iHeight * sizeof(int*)); 
  visitefront = (int**) malloc(iHeight * sizeof(int*)); 
  if(*ptabtabImageOfVisitedPixels == NULL || visitefront == NULL) 
    mwerror(FATAL, 1, "init_image_of_visited_pixels --> impossible to allocate one array\n"); 
  (*ptabtabImageOfVisitedPixels)[0] = (int*) calloc((size_t)iAreaImage, sizeof(int)); 
  if((*ptabtabImageOfVisitedPixels)[0] == NULL) 
    mwerror(FATAL, 1, "init_image_of_visited_pixels --> impossible to allocate: image of visited pixels\n"); 
  for(i = 1; i < iHeight; i++) 
    (*ptabtabImageOfVisitedPixels)[i] = (*ptabtabImageOfVisitedPixels)[i-1] + iWidth; 
 
  visitefront[0] = (int*) calloc((size_t)iAreaImage, sizeof(int)); 
  if(visitefront[0] == NULL) 
    mwerror(FATAL, 1, "init_image_of_visited_pixels -->impossible to allocate image of visited neighbors\n"); 
  for(i = 1; i < iHeight; i++) 
    visitefront[i] = visitefront[i-1] + iWidth; 
} 
 
static void free_image_of_visited_pixels(tabtabImageOfVisitedPixels) 
int** tabtabImageOfVisitedPixels; 
{ 
  free(tabtabImageOfVisitedPixels[0]); /* Remember that the memory for the image is in fact a 1-D array */ 
  free(tabtabImageOfVisitedPixels); /* This was simply an array of pointers to a location in the 1-D array */ 
 
  free(visitefront[0]); 
  free(visitefront); 
} 
 
static void init_output_image(tabPixelsIn, tabPixelsOut, pTabtabPixelsOutput) 
float *tabPixelsIn, *tabPixelsOut, ***pTabtabPixelsOutput; 
{ 
  int i; 
 
  if(tabPixelsOut != tabPixelsIn) 
    memcpy(tabPixelsOut, tabPixelsIn, iWidth * iHeight * sizeof(float)); 
  if(*pTabtabPixelsOutput == NULL) 
    { 
      *pTabtabPixelsOutput = (float**) malloc(iHeight * sizeof(float*)); 
      if(*pTabtabPixelsOutput == NULL) 
	mwerror(FATAL, 1, "init_output_image --> impossible to allocate the array\n"); 
    } 
  for (i = 0; i < iHeight; i++) 
    (*pTabtabPixelsOutput)[i] = tabPixelsOut + i * iWidth; 
} 
 
static void free_output_image(tabtabPixelsOutput) 
float** tabtabPixelsOutput; 
{ 
  free(tabtabPixelsOutput); 
} 
 
static void init_region() 
{ 
  tabPointsInCurrentRegion = (struct point_plane*) malloc(iMaxArea * sizeof(struct point_plane)); 
  if(tabPointsInCurrentRegion == NULL) 
    mwerror(FATAL, 1, "init_region --> impossible to allocate the array\n"); 
} 
 
static void free_region() 
{ 
  free(tabPointsInCurrentRegion); 
} 
 
static void init_nodes_of_curve() 
{ 
  if(! bStoreExternalLine) { 
    tabNodesOfCurve = NULL; 
    return; 
  } 
  /* The number of nodes in a curve is limited by the fact that we pass at most two times at each 
     "inter-pixel node", that is a point at the corner of a pixel */ 
  tabNodesOfCurve = (struct point_plane*) malloc(2*(iWidth+1)*(iHeight+1)); 
  if(tabNodesOfCurve == NULL) 
    mwerror(FATAL, 1, "init_nodes_of_curve --> impossible to allocate the array\n"); 
} 
 
static void free_nodes_of_curve() 
{ 
  free(tabNodesOfCurve); 
} 
 
static void init_frontier_pixels() 
{ 
  int i; 
  struct FlstFrontierPixel* tabBuffer; 
 
  tabBuffer = (struct FlstFrontierPixel*) calloc((size_t)iAreaImage, sizeof(struct FlstFrontierPixel)); 
  if(tabBuffer == NULL) 
    mwerror(FATAL, 1, "init_frontier_pixels --> impossible to allocate the image of frontier pixels\n"); 
  tabtabFrontierPixels = (struct FlstFrontierPixel**)malloc(iHeight * sizeof(struct FlstFrontierPixel*)); 
  if(tabtabFrontierPixels == NULL) 
    mwerror(FATAL, 1, "init_frontier_pixels --> impossible to allocate the access array of frontier pixels\n"); 
  for(i = 0; i < iHeight; i++) 
    tabtabFrontierPixels[i] = tabBuffer + i * iWidth; 
} 
 
static void free_frontier_pixels() 
{ 
  free(tabtabFrontierPixels[0]); 
  free(tabtabFrontierPixels); 
} 
 
static void init_patterns() 
{ 
  int i; 
  unsigned char c4Neighbors; 
 
  /* This is for the shape in 4-connectedness (and the complementary in 8-connectedness) */ 
  memset(tabPattern4, 0, 256 * sizeof(int)); 
  for(i = 0; i <= 255; i++) 
    { 
      c4Neighbors = i & (UP | LEFT | DOWN | RIGHT); 
      if(c4Neighbors == (UP | LEFT | DOWN | RIGHT)) 
	{ 
	  tabPattern4[i] = -1; 
	  if(i & UPLEFT) 
	    ++ tabPattern4[i]; 
	  if(i & LEFTDOWN) 
	    ++ tabPattern4[i]; 
	  if(i & DOWNRIGHT) 
	    ++ tabPattern4[i]; 
	  if(i & RIGHTUP) 
	    ++ tabPattern4[i]; 
	} 
      else if(c4Neighbors == (UP | LEFT | DOWN)) 
	{ 
	  if(i & UPLEFT) 
	    tabPattern4[i] = 1; 
	  if(i & LEFTDOWN) 
	    ++ tabPattern4[i]; 
	} 
      else if(c4Neighbors == (LEFT | DOWN | RIGHT)) 
	{ 
	  if(i & LEFTDOWN) 
	    tabPattern4[i] = 1; 
	  if(i & DOWNRIGHT) 
	    ++ tabPattern4[i]; 
	} 
      else if(c4Neighbors == (DOWN | RIGHT | UP)) 
	{ 
	  if(i & DOWNRIGHT) 
	    tabPattern4[i] = 1; 
	  if(i & RIGHTUP) 
	    ++ tabPattern4[i]; 
	} 
      else if(c4Neighbors == (RIGHT | UP | LEFT)) 
	{ 
	  if(i & RIGHTUP) 
	    tabPattern4[i] = 1; 
	  if(i & UPLEFT) 
	    ++ tabPattern4[i]; 
	} 
      else if(c4Neighbors == (UP | DOWN)) 
	tabPattern4[i] = 1; 
      else if(c4Neighbors == (RIGHT | LEFT)) 
	tabPattern4[i] = 1; 
      else if(c4Neighbors == (UP | LEFT) && (i & UPLEFT)) 
	tabPattern4[i] = 1; 
      else if(c4Neighbors == (LEFT | DOWN) && (i & LEFTDOWN)) 
	tabPattern4[i] = 1; 
      else if(c4Neighbors == (DOWN | RIGHT) && (i & DOWNRIGHT)) 
	tabPattern4[i] = 1; 
      else if(c4Neighbors == (RIGHT | UP) && (i & RIGHTUP)) 
	tabPattern4[i] = 1; 
    } 
 
  /* This is for the shape in 8-connectedness (and the complementary in 4-connectedness) */ 
  memset(tabPattern8, 0, 256 * sizeof(int)); 
  for(i = 0; i <= 255; i++) 
    { 
      c4Neighbors = i & (UP | LEFT | DOWN | RIGHT); 
      if(c4Neighbors == (UP | LEFT | DOWN | RIGHT)) 
	tabPattern8[i] = -1; 
      else if(c4Neighbors == (UP | DOWN)) 
	tabPattern8[i] = 1; 
      else if(c4Neighbors == (RIGHT | LEFT)) 
	tabPattern8[i] = 1; 
      else if(c4Neighbors == LEFT) 
	{ 
	  if(i & DOWNRIGHT) 
	    tabPattern8[i] = 1; 
	  if(i & RIGHTUP) 
	    ++ tabPattern8[i]; 
	} 
      else if(c4Neighbors == DOWN) 
	{ 
	  if(i & RIGHTUP) 
	    tabPattern8[i] = 1; 
	  if(i & UPLEFT) 
	    ++ tabPattern8[i]; 
	} 
      else if(c4Neighbors == RIGHT) 
	{ 
	  if(i & UPLEFT) 
	    tabPattern8[i] = 1; 
	  if(i & LEFTDOWN) 
	    ++ tabPattern8[i]; 
	} 
      else if(c4Neighbors == UP) 
	{ 
	  if(i & LEFTDOWN) 
	    tabPattern8[i] = 1; 
	  if(i & DOWNRIGHT) 
	    ++ tabPattern8[i]; 
	} 
      else if(c4Neighbors == (UP | LEFT) && (i & DOWNRIGHT)) 
	tabPattern8[i] = 1; 
      else if(c4Neighbors == (LEFT | DOWN) && (i & RIGHTUP)) 
	tabPattern8[i] = 1; 
      else if(c4Neighbors == (DOWN | RIGHT) && (i & UPLEFT)) 
	tabPattern8[i] = 1; 
      else if(c4Neighbors == (RIGHT | UP) && (i & LEFTDOWN)) 
	tabPattern8[i] = 1; 
      else if(c4Neighbors == 0) 
	{ /* There are pixels in the shape only in diagonal directions */ 
	  tabPattern8[i] = -1; 
	  if(i & UPLEFT) 
	    ++ tabPattern8[i]; 
	  if(i & LEFTDOWN) 
	    ++ tabPattern8[i]; 
	  if(i & DOWNRIGHT) 
	    ++ tabPattern8[i]; 
	  if(i & RIGHTUP) 
	    ++ tabPattern8[i]; 
	  if(tabPattern8[i] == -1) /* This happens only when it is the first pixel in the shape */ 
	    tabPattern8[i] = 0; 
	} 
    } 
 
  /* Now initialise the arrays used to follow one component of the frontier. The value 0 corresponds 
     to either an impossible configuration or an end-point of the frontier */ 
  for(i = 0; i < 256; i++) 
    tabChoiceDirection4[i] = tabChoiceDirection8[i] = 0; 
  /* Only one direction possible, so no problem to choose */ 
  tabChoiceDirection4[UP] = tabChoiceDirection8[UP] = UP; 
  tabChoiceDirection4[LEFT] = tabChoiceDirection8[LEFT] = LEFT; 
  tabChoiceDirection4[RIGHT] = tabChoiceDirection8[RIGHT] = RIGHT; 
  tabChoiceDirection4[DOWN] = tabChoiceDirection8[DOWN] = DOWN; 
  tabChoiceDirection4[HALFUP] = tabChoiceDirection8[HALFUP] = HALFUP; 
  /* 3 directions, only two are valid. Choose the good one according to connectedness */ 
  /*      | 
	 \|/ 
     -<-  *  ->-   Arriving from below */ 
  tabChoiceDirection4[LEFT|DOWN|RIGHT] = RIGHT; 
  tabChoiceDirection8[LEFT|DOWN|RIGHT] = LEFT; 
  /*     /|\ 
	  | 
	  *  -<-   Arriving from the left 
	  | 
	 \|/                                */ 
  tabChoiceDirection4[DOWN|RIGHT|UP] = UP; 
  tabChoiceDirection4[DOWN|RIGHT|HALFUP] = HALFUP; 
  tabChoiceDirection8[DOWN|RIGHT|UP] = DOWN; 
  tabChoiceDirection8[DOWN|RIGHT|HALFUP] = DOWN; 
  /* -<-  *  ->-   Arriving from above 
         /|\ 
	  |                                 */ 
  tabChoiceDirection4[RIGHT|UP|LEFT] = LEFT; 
  tabChoiceDirection8[RIGHT|UP|LEFT] = RIGHT; 
  /*     /|\ 
	  | 
     ->-  *        Arriving from the right 
	  | 
	 \|/                                */ 
  tabChoiceDirection4[UP|LEFT|DOWN] = DOWN; 
  tabChoiceDirection4[HALFUP|LEFT|DOWN] = DOWN; 
  tabChoiceDirection8[UP|LEFT|DOWN] = UP; 
  tabChoiceDirection8[HALFUP|LEFT|DOWN] = HALFUP; 
} 
 
/* -------------------------------------------------------------------------------------- 
   --------- The algorithm itself, based on growing regions containing an extremum ------ 
   -------------------------------------------------------------------------------------- */ 
 
/* Indicates if the pixel at position (x, y) is a local minimum in the image */ 
static char is_local_min(ou, x, y, bFlagHeightConnectedness) 
float** ou; /* A 2-D array of the image */ 
short int x, y; 
char bFlagHeightConnectedness; 
{ 
  float v; 
  char n = 0; 
 
  v = ou[y][x]; 
  return (x==iWidth-1 || (ou[y][x+1]>v && ++n) || ou[y][x+1]==v) && 
    (x==0 || (ou[y][x-1]>v && ++n) || ou[y][x-1]==v) && 
      (y==iHeight-1 || (ou[y+1][x]>v && ++n) || ou[y+1][x]==v) && 
	(y==0 || (ou[y-1][x]>v && ++n) || ou[y-1][x]==v) && 
	  (bFlagHeightConnectedness == 0 || 
	   ((x==iWidth-1 || y==0 || (ou[y-1][x+1]>v  && ++n) || ou[y-1][x+1]==v) && 
	    (x==iWidth-1 || y==iHeight-1 || (ou[y+1][x+1]>v && ++n) || ou[y+1][x+1]==v) && 
	    (x==0 || y==iHeight-1 || (ou[y+1][x-1]>v && ++n) || ou[y+1][x-1]==v) && 
	    (x==0 || y==0 || (ou[y-1][x-1]>v && ++n) || ou[y-1][x-1]==v)))	&& 
	      n != 0; 
} 
 
/* Indicates whether the pixel at position (x,y) in the image is a local maximum */ 
static char is_local_max(ou, x, y, bFlagHeightConnectedness) 
float** ou; /* A 2-D array of the image */ 
short int x, y; 
char bFlagHeightConnectedness; 
{ 
  float v; 
  char n = 0; 
 
  v = ou[y][x]; 
  return (x==iWidth-1 || (ou[y][x+1]<v && ++n) || ou[y][x+1]==v) && 
    (x==0 || (ou[y][x-1]<v && ++n) || ou[y][x-1]==v) && 
      (y==iHeight-1 || (ou[y+1][x]<v && ++n) || ou[y+1][x]==v) && 
	(y==0 || (ou[y-1][x]<v && ++n) || ou[y-1][x]==v) &&  
	  (bFlagHeightConnectedness == 0 || 
	   ((x==iWidth-1 || y==0 || (ou[y-1][x+1]<v  && ++n) || ou[y-1][x+1]==v) && 
	    (x==iWidth-1 || y==iHeight-1 || (ou[y+1][x+1]<v && ++n) || ou[y+1][x+1]==v) && 
	    (x==0 || y==iHeight-1 || (ou[y+1][x-1]<v && ++n) || ou[y+1][x-1]==v) && 
	    (x==0 || y==0 || (ou[y-1][x-1]<v && ++n) || ou[y-1][x-1]==v))) && 
	      n != 0; 
} 
 
/* Put all the pixels of tabPoints at level fNewGrayLevel in the image tabtabPixelsOutput */ 
static void flst_set_at_level(tabtabPixelsOutput, tabPoints, iNbPoints, fNewGrayLevel) 
float** tabtabPixelsOutput; 
struct point_plane* tabPoints; 
int iNbPoints; 
float fNewGrayLevel; 
{ 
  int i; 
  for(i = iNbPoints - 1; i >= 0; i--) 
    tabtabPixelsOutput[tabPoints[i].y][tabPoints[i].x] = fNewGrayLevel; 
} 
 
/* Add the point of coordinates (i, j) in the current connected component of level set. Modify the 
   configuration of the frontier suitably and update the number of connected components of the  
   complementary (useful to know the number of holes in the shape). 
   Note that the treatment is different according to the connectedness: if the shape is in 
   4-connectedness, we must consider the complementary as in 8-connectedness, so that the configuration 
   of the frontier along the diagonal neighbors is important. */ 
static void flst_add_point4(i, j, pNbConnectedComponentsOfFrontier) 
short int i, j; 
int* pNbConnectedComponentsOfFrontier; 
{ 
  struct FlstFrontierPixel* pFrontierPixel = tabtabFrontierPixels[i] + j; 
  unsigned char cPattern = 0; 
 
  if(bDoesShapeMeetBorder) 
    { 
      if(i == 0) cPattern |= LEFT; 
      if(j == 0) cPattern |= DOWN; 
    } 
  if(pFrontierPixel->iExploration < iIndexOfExploration) 
    { 
      pFrontierPixel->iExploration = iIndexOfExploration; 
      pFrontierPixel->cDirections = 0; 
      if(j != 0) pFrontierPixel->cDirections |= UP; 
      if(i != 0) pFrontierPixel->cDirections |= RIGHT; 
      pFrontierPixel->iExplorationHole = 0; 
    } 
  else 
    { 
      /* If all is OK, there could not be a right or up direction here */ 
      if(pFrontierPixel->cDirections & RIGHT) 
	mwerror(FATAL, 1, "flst_add_point4 --> right ?\n"); 
      if(pFrontierPixel->cDirections & UP) 
	mwerror(FATAL, 1, "flst_add_point4 --> up ?\n"); 
 
      cPattern |= pFrontierPixel->cDirections & (LEFT | DOWN); 
      if(pFrontierPixel->cDirections & LEFT) 
	pFrontierPixel->cDirections -= LEFT; 
      else if(i != 0) 
	pFrontierPixel->cDirections |= RIGHT; 
      if(pFrontierPixel->cDirections & DOWN) 
	pFrontierPixel->cDirections -= DOWN; 
      else if(j != 0) 
	pFrontierPixel->cDirections |= UP; 
    } 
 
  if(j == iWidth-1) 
    { 
      if(bDoesShapeMeetBorder) cPattern |= UP; 
    } 
  else 
    if(pFrontierPixel[1].iExploration < iIndexOfExploration) 
      { 
	pFrontierPixel[1].iExploration = iIndexOfExploration; 
	pFrontierPixel[1].cDirections = DOWN; 
	pFrontierPixel[1].iExplorationHole = 0; 
      } 
    else 
      { 
	if(pFrontierPixel[1].cDirections & DOWN) 
	  mwerror(FATAL, 1, "flst_add_point4 --> down ?\n"); 
	 
	cPattern |= pFrontierPixel[1].cDirections & UP; 
	if(pFrontierPixel[1].cDirections & RIGHT) 
	  cPattern |= UPLEFT; 
	if(pFrontierPixel[1].cDirections & UP) 
	  pFrontierPixel[1].cDirections -= UP; 
	else 
	  pFrontierPixel[1].cDirections |= DOWN; 
      } 
   
  if(i == iHeight-1) 
    { 
      if(bDoesShapeMeetBorder) cPattern |= RIGHT; 
    } 
  else 
    if(pFrontierPixel[iWidth].iExploration < iIndexOfExploration) 
      { 
	pFrontierPixel[iWidth].iExploration = iIndexOfExploration; 
	pFrontierPixel[iWidth].cDirections = LEFT; 
	pFrontierPixel[iWidth].iExplorationHole = 0; 
      } 
    else 
      { 
	if(pFrontierPixel[iWidth].cDirections & LEFT) 
	  mwerror(FATAL, 1, "flst_add_point4 --> left ?\n"); 
 
	cPattern |= pFrontierPixel[iWidth].cDirections & RIGHT; 
	if(pFrontierPixel[iWidth].cDirections & UP) 
	  cPattern |= DOWNRIGHT; 
	if(pFrontierPixel[iWidth].cDirections & RIGHT) 
	  pFrontierPixel[iWidth].cDirections -= RIGHT; 
	else 
	  pFrontierPixel[iWidth].cDirections |= LEFT; 
      } 
 
  /* Look if the number of connected components of the frontier is changing */ 
  if(j > 0 && pFrontierPixel[-1].iExploration == iIndexOfExploration &&  
     (pFrontierPixel[-1].cDirections & RIGHT)) 
    cPattern |= LEFTDOWN; 
  if(i < iHeight-1 && j < iWidth-1 && pFrontierPixel[iWidth+1].iExploration == iIndexOfExploration &&  
     (pFrontierPixel[iWidth+1].cDirections & DOWN)) 
    cPattern |= RIGHTUP; 
  *pNbConnectedComponentsOfFrontier += tabPattern4[cPattern]; 
 
  if(j == 0 || j == iWidth-1 || i == 0 || i == iHeight-1) 
    bDoesShapeMeetBorder = (char)1; 
} 
 
/* Add the point of coordinates (i, j) in the current connected component of level set. Modify the 
   configuration of the frontier suitably and update the number of connected components of the  
   complementary (useful to know the number of holes in the shape). 
   Note that the treatment is different according to the connectedness: if the shape is in 
   4-connectedness, we must consider the complementary as in 8-connectedness, so that the configuration 
   of the frontier along the diagonal neighbors is important. */ 
static void flst_add_point8(i, j, pNbConnectedComponentsOfFrontier) 
short int i, j; 
int* pNbConnectedComponentsOfFrontier; 
{ 
  struct FlstFrontierPixel* pFrontierPixel = tabtabFrontierPixels[i] + j; 
  unsigned char cPattern = 0; 
 
  if(bDoesShapeMeetBorder) 
    { 
      if(i == 0) cPattern |= LEFT; 
      if(j == 0) cPattern |= DOWN; 
    } 
  if(pFrontierPixel->iExploration < iIndexOfExploration) 
    { 
      pFrontierPixel->iExploration = iIndexOfExploration; 
      pFrontierPixel->cDirections = 0; 
      if(j != 0) pFrontierPixel->cDirections |= UP; 
      if(i != 0) pFrontierPixel->cDirections |= RIGHT; 
      pFrontierPixel->iExplorationHole = 0; 
    } 
  else 
    { 
      /* If all is OK, there could not be a right or up direction here */ 
      if(pFrontierPixel->cDirections & RIGHT) 
	mwerror(FATAL, 1, "flst_add_point8 --> right ?\n"); 
      if(pFrontierPixel->cDirections & UP) 
	mwerror(FATAL, 1, "flst_add_point8 --> up ?\n"); 
 
      cPattern |= pFrontierPixel->cDirections & (LEFT | DOWN); 
      if(pFrontierPixel->cDirections & LEFT) 
	pFrontierPixel->cDirections -= LEFT; 
      else if(i != 0) 
	pFrontierPixel->cDirections |= RIGHT; 
      if(pFrontierPixel->cDirections & DOWN) 
	pFrontierPixel->cDirections -= DOWN; 
      else if(j != 0) 
	pFrontierPixel->cDirections |= UP; 
    } 
 
  if(j == iWidth-1) 
    { 
      if(bDoesShapeMeetBorder) cPattern |= UP; 
    } 
  else 
    if(pFrontierPixel[1].iExploration < iIndexOfExploration) 
      { 
	pFrontierPixel[1].iExploration = iIndexOfExploration; 
	pFrontierPixel[1].cDirections = DOWN; 
	pFrontierPixel[1].iExplorationHole = 0; 
      } 
    else 
      { 
	if(pFrontierPixel[1].cDirections & DOWN) 
	  mwerror(FATAL, 1, "flst_add_point8 --> down ?\n"); 
	 
	cPattern |= pFrontierPixel[1].cDirections & UP; 
	if(pFrontierPixel[1].cDirections & LEFT) 
	  cPattern |= UPLEFT; 
	if(pFrontierPixel[1].cDirections & UP) 
	  pFrontierPixel[1].cDirections -= UP; 
	else 
	  pFrontierPixel[1].cDirections |= DOWN; 
      } 
 
  if(i == iHeight-1) 
    { 
      if(bDoesShapeMeetBorder) cPattern |= RIGHT; 
    } 
  else 
    if(pFrontierPixel[iWidth].iExploration < iIndexOfExploration) 
      { 
	pFrontierPixel[iWidth].iExploration = iIndexOfExploration; 
	pFrontierPixel[iWidth].cDirections = LEFT; 
	pFrontierPixel[iWidth].iExplorationHole = 0; 
      } 
    else 
      { 
	if(pFrontierPixel[iWidth].cDirections & LEFT) 
	  mwerror(FATAL, 1, "flst_add_point8 --> left ?\n"); 
	 
	cPattern |= pFrontierPixel[iWidth].cDirections & RIGHT; 
	if(pFrontierPixel[iWidth].cDirections & DOWN) 
	  cPattern |= DOWNRIGHT; 
	if(pFrontierPixel[iWidth].cDirections & RIGHT) 
	  pFrontierPixel[iWidth].cDirections -= RIGHT; 
	else 
	  pFrontierPixel[iWidth].cDirections |= LEFT; 
      } 
   
  /* Treat the modification of the number of connected components of the frontier */ 
  if(j > 0 && pFrontierPixel[-1].iExploration == iIndexOfExploration &&  
     (pFrontierPixel[-1].cDirections & LEFT)) 
    cPattern |=  LEFTDOWN; 
  if(i < iHeight-1 && j < iWidth-1 && pFrontierPixel[iWidth+1].iExploration == iIndexOfExploration &&  
     (pFrontierPixel[iWidth+1].cDirections & UP)) 
    cPattern |= RIGHTUP; 
  *pNbConnectedComponentsOfFrontier += tabPattern8[cPattern]; 
 
  if(j == 0 || j == iWidth-1 || i == 0 || i == iHeight-1) 
    bDoesShapeMeetBorder = (char)1; 
} 
 
 
#define UPDATE_FRONTIER(a) \
   if((a).iExplorationHole < iCurrentExplorationHole) \
     { \
	 (a).iExplorationHole = iCurrentExplorationHole; \
	 (a).cCurrentDirections = (a).cDirections; \
     } 
 
/* Follow the component of the frontier of a shape starting with the direction cDirection at point 
   (iX, iY) and return the included area in it */ 
static int flst_follow_one_border(iY, iX, iCurrentExplorationHole, cDirection, bFlagHeightConnectedness, pNbNodesOfCurve) 
short int iY, iX; 
int iCurrentExplorationHole; 
char bFlagHeightConnectedness; 
unsigned char cDirection; 
int* pNbNodesOfCurve; 
{ 
  int iAreaInsideComponentOfFrontier = 0; 
  unsigned char cConfiguration; 
  char bStoreBorder = bStoreExternalLine && *pNbNodesOfCurve == 0; 
 
  if(iX == iWidth) 
    iAreaInsideComponentOfFrontier = iWidth * (iY - iHeight); 
  while(cDirection != 0) 
    { 
      if(bStoreBorder) { 
	tabNodesOfCurve[*pNbNodesOfCurve].x = iX; 
	tabNodesOfCurve[(*pNbNodesOfCurve)++].y = iY; 
      } 
      cConfiguration = 0; 
      if(iX < iWidth && iY < iHeight && tabtabFrontierPixels[iY][iX].iExploration == iIndexOfExploration) 
	{ 
	  UPDATE_FRONTIER(tabtabFrontierPixels[iY][iX]); 
	  cConfiguration |= tabtabFrontierPixels[iY][iX].cCurrentDirections & (UP | LEFT | RIGHT | DOWN); 
	} 
      if(iX > 0 && iY < iHeight && tabtabFrontierPixels[iY][iX-1].iExploration == iIndexOfExploration) 
	{ 
	  UPDATE_FRONTIER(tabtabFrontierPixels[iY][iX-1]); 
	  cConfiguration |= tabtabFrontierPixels[iY][iX-1].cCurrentDirections & (LEFT | RIGHT); 
	} 
      if(iX < iWidth && iY > 0 && tabtabFrontierPixels[iY-1][iX].iExploration == iIndexOfExploration) 
	{ 
	  UPDATE_FRONTIER(tabtabFrontierPixels[iY-1][iX]); 
	  cConfiguration |= tabtabFrontierPixels[iY-1][iX].cCurrentDirections & (UP | DOWN | HALFUP); 
	} 
      if(bFlagHeightConnectedness) 
	cDirection = tabChoiceDirection8[cConfiguration]; 
      else 
	cDirection = tabChoiceDirection4[cConfiguration]; 
      switch(cDirection) { 
      case RIGHT: 
	tabtabFrontierPixels[iY][iX].cCurrentDirections -= RIGHT; 
	++ iX; 
	break; 
      case DOWN: 
	iAreaInsideComponentOfFrontier += iX; 
	tabtabFrontierPixels[iY][iX].cCurrentDirections -= DOWN; 
	++ iY; 
	break; 
      case LEFT: 
	-- iX; 
      tabtabFrontierPixels[iY][iX].cCurrentDirections -= LEFT; 
      break; 
      case UP: 
	iAreaInsideComponentOfFrontier -= iX; 
	-- iY; 
	tabtabFrontierPixels[iY][iX].cCurrentDirections -= UP; 
	break; 
      case HALFUP: 
	iAreaInsideComponentOfFrontier -= iX; 
	-- iY; 
	tabtabFrontierPixels[iY][iX].cCurrentDirections -= HALFUP; 
	if(bStoreBorder) { /* A closed curve, last point = first point */ 
	  tabNodesOfCurve[*pNbNodesOfCurve].x = iX; 
	  tabNodesOfCurve[(*pNbNodesOfCurve)++].y = iY; 
	} 
	cDirection = 0; 
	break; 
      default:  
	cDirection = 0; 
	break; 
      } 
    } 
  if(iX == iWidth) 
    iAreaInsideComponentOfFrontier += iWidth * (iHeight - iY); 
 
  return iAreaInsideComponentOfFrontier; 
} 
 
/* Follow the frontier of a shape, isolating the holes in it. Be careful at crossings not to take a forbidden 
   direction: it depends on the chosen notion of connectedness */ 
static int flst_follow_holes(iNbConnectedComponentsOfFrontier, iNbPoints, iCurrentExplorationHole, 
			     pContainingShape, bFlagHeightConnectedness, pNbNodesOfCurve) 
int iNbConnectedComponentsOfFrontier; 
int iNbPoints;  /* The number of points in the shape */ 
int iCurrentExplorationHole; /* An index to know where we are */ 
Shape pContainingShape; 
char bFlagHeightConnectedness; 
int* pNbNodesOfCurve; 
{ 
  int iHole = 0, i, iArea = iAreaImage; 
  struct FlstFrontierPixel* pFrontierPixel; 
  int iAreaInsideComponentOfFrontier = 0; 
  short int iX, iY; 
  unsigned char cDirection; 
  char bExternalLineFound = 0; 
   
  *pNbNodesOfCurve = 0; 
 
  /* If the shape meets the border of the image, find first the components of the frontier that meet it */ 
  if(bDoesShapeMeetBorder) 
    { 
      for(i = 0; i < iNbPoints; i++) 
	{ 
	  iY = tabPointsInCurrentRegion[i].y; 
	  iX = tabPointsInCurrentRegion[i].x; 
	  if((iY == 0 && iX < iWidth-1 && 
	      (tabtabFrontierPixels[iY][iX+1].cDirections & (cDirection = DOWN))) || 
	     (iX == 0 && (tabtabFrontierPixels[iY][iX].cDirections & (cDirection = RIGHT))) || 
	     (iY == iHeight-1 && (tabtabFrontierPixels[iY][iX].cDirections & (cDirection = UP))) || 
	     (iX == iWidth-1 && iY < iHeight-1 && 
	      (tabtabFrontierPixels[iY+1][iX].cDirections & (cDirection = LEFT)))) 
	    { 
	      ++ iHole; 
	      tabHoles[iNbHoles].point.x = iX; 
	      tabHoles[iNbHoles].point.y = iY; 
	      tabHoles[iNbHoles].pContainingShape = pContainingShape; 
	      if(cDirection == DOWN) { ++ iX; ++ tabHoles[iNbHoles].point.x; } 
	      else if(cDirection == UP) { ++ iY; -- tabHoles[iNbHoles].point.x; } 
	      else if(cDirection == LEFT) { ++ iX; ++ iY; ++ tabHoles[iNbHoles].point.y; } 
	      else -- tabHoles[iNbHoles].point.y; 
	      if(++iNbHoles == MAXHOLES) 
		mwerror(FATAL, 1, "flst_follow_holes --> too many holes for the allocated array\n"); 
	      iAreaInsideComponentOfFrontier = flst_follow_one_border(iY, iX, iCurrentExplorationHole, 
								      cDirection, bFlagHeightConnectedness, 
								      pNbNodesOfCurve); 
	      if(iAreaInsideComponentOfFrontier < 0) 
		iAreaInsideComponentOfFrontier += iAreaImage; 
	      if(iAreaInsideComponentOfFrontier <= iHalfAreaImage) 
		{ /* External level line found. It is not a hole */ 
		  bExternalLineFound = (char)1; 
		  -- iNbHoles; 
		  if(iArea != iAreaImage) 
		    mwerror(FATAL, 1, "flst_follow_holes --> 2 exterior borders (case 1)\n"); 
		  iArea = iAreaInsideComponentOfFrontier; 
		} 
	      else if(! bExternalLineFound) *pNbNodesOfCurve = 0; 
	      if(iHole == iNbConnectedComponentsOfFrontier) 
		break; 
	    } 
	} 
      if(iArea == iAreaImage) 
	pGlobalTree->the_shapes[0].value = pContainingShape->value; 
      if(iHole == iNbConnectedComponentsOfFrontier) 
	return iArea; 
    } 
 
  /* Now find the cc of the frontier not meeting the frontier */ 
  for(i = 0; i < iNbPoints; i++) 
    { 
      pFrontierPixel = tabtabFrontierPixels[tabPointsInCurrentRegion[i].y] + tabPointsInCurrentRegion[i].x; 
      if((pFrontierPixel->cDirections & UP) && 
	 (pFrontierPixel->iExplorationHole<iCurrentExplorationHole || 
	  (pFrontierPixel->cCurrentDirections&UP))) 
	{ /* Follow this component of the frontier */ 
	  ++ iHole; 
	  iX = tabPointsInCurrentRegion[i].x; 
	  tabHoles[iNbHoles].point.x =  iX - 1; 
	  tabHoles[iNbHoles].point.y = iY = tabPointsInCurrentRegion[i].y; 
	  tabHoles[iNbHoles].pContainingShape = pContainingShape; 
	  UPDATE_FRONTIER(tabtabFrontierPixels[iY][iX]); 
	  tabtabFrontierPixels[iY][iX].cCurrentDirections -= UP; 
	  tabtabFrontierPixels[iY][iX].cCurrentDirections |= HALFUP; 
	  if(++iNbHoles == MAXHOLES) 
	    mwerror(FATAL, 1, "flst_follow_holes --> too many holes for the allocated array\n"); 
	  iAreaInsideComponentOfFrontier = flst_follow_one_border(iY, iX, iCurrentExplorationHole, 
								  UP, bFlagHeightConnectedness, pNbNodesOfCurve); 
	  if(iAreaInsideComponentOfFrontier > 0) 
	    {  /* This is the exterior frontier, so it is not a hole */ 
	      bExternalLineFound = (char)1; 
	      -- iNbHoles; 
	      if(iArea != iAreaImage) 
		mwerror(FATAL, 1, "flst_follow_holes --> 2 exterior borders (case 2)\n"); 
	      iArea = iAreaInsideComponentOfFrontier; 
	    } 
	  else if(! bExternalLineFound) *pNbNodesOfCurve = 0; 
	  if(iHole == iNbConnectedComponentsOfFrontier) 
	    break; 
	} 
    } 
   
  return iArea; 
} 
 
/* Insert a new shape in the tree, whose parent is pParent */ 
static void flst_insert_child_in_tree(pParent, pNewChildToInsert) 
Shape pParent, pNewChildToInsert; 
{ 
  pNewChildToInsert->parent = pParent; 
  pNewChildToInsert->next_sibling = pParent->child; 
  pParent->child = pNewChildToInsert; 
} 
 
/* Add the points in the neighborhood of gray level fCurrentGrayLevel to the region tabPointsInCurrentRegion 
   and return 1 if we are below the maximum area of a region */ 
static char flst_add_iso_level(tabPointsInCurrentRegion, pCurrentArea, fCurrentGrayLevel, pNeighborhood, ou, 
			       tabtabImageOfVisitedPixels, 
			       pNbConnectedComponentsOfFrontier, bFlagHeightConnectedness, bIsInferiorLevel) 
struct point_plane* tabPointsInCurrentRegion; 
int* pCurrentArea; 
float fCurrentGrayLevel; 
struct FlstNeighborhood* pNeighborhood; 
float** ou;  
int** tabtabImageOfVisitedPixels; 
int* pNbConnectedComponentsOfFrontier; 
char bFlagHeightConnectedness; 
char bIsInferiorLevel; 
{ 
  short int x, y; 
  struct FlstNeighbor* pNeighbor; 
  int iCurrentArea; 
 
  iCurrentArea = *pCurrentArea; 
  pNeighbor = &pNeighborhood->tabNeighbors[1]; 
  while(pNeighborhood->iNbPoints > 0 && pNeighborhood->tabNeighbors[1].value == fCurrentGrayLevel && 
	iCurrentArea < iMaxArea) 
    { 
      x = pNeighbor->point.x; 
      y = pNeighbor->point.y; 
      tabPointsInCurrentRegion[iCurrentArea].x = x; 
      tabPointsInCurrentRegion[iCurrentArea++].y = y; 
      flst_remove_neighbor(pNeighborhood); 
      if(iMinArea <= iMaxArea) /* Otherwise, we are only destroying, so no need to know the frontier */ 
	if(bFlagHeightConnectedness == 0) 
	  flst_add_point4(y, x, pNbConnectedComponentsOfFrontier); 
	else 
	  flst_add_point8(y, x, pNbConnectedComponentsOfFrontier); 
      tabtabImageOfVisitedPixels[y][x] = iIndexOfExploration; 
      if(x > 0 && NOTVISITED(x-1,y)) 
	flst_add_neighbor(pNeighborhood, x-1, y, ou[y][x-1]); 
      if(x < iWidth-1 && NOTVISITED(x+1,y)) 
	flst_add_neighbor(pNeighborhood, x+1, y, ou[y][x+1]); 
      if(y > 0 && NOTVISITED(x,y-1)) 
	flst_add_neighbor(pNeighborhood, x, y-1, ou[y-1][x]); 
      if(y < iHeight-1 && NOTVISITED(x,y+1)) 
	flst_add_neighbor(pNeighborhood, x, y+1, ou[y+1][x]); 
      if(bFlagHeightConnectedness != 0) 
	{       
	  if(x > 0 && y > 0 && NOTVISITED(x-1,y-1)) 
	    flst_add_neighbor(pNeighborhood, x-1, y-1, ou[y-1][x-1]); 
	  if(x < iWidth-1 && y > 0 && NOTVISITED(x+1,y-1)) 
	    flst_add_neighbor(pNeighborhood, x+1, y-1, ou[y-1][x+1]); 
	  if(x < iWidth-1 && y < iHeight-1 && NOTVISITED(x+1,y+1)) 
	    flst_add_neighbor(pNeighborhood, x+1, y+1, ou[y+1][x+1]); 
	  if(x > 0 && y < iHeight-1 && NOTVISITED(x-1,y+1)) 
	    flst_add_neighbor(pNeighborhood, x-1, y+1, ou[y+1][x-1]); 
	} 
    } 
  *pCurrentArea = iCurrentArea; 
  return (char)(iCurrentArea < iMaxArea); 
} 
 
/* Store in a new curve the points in tabNodesOfCurve */ 
static Curve flst_build_external_line(tabNodes, iNbNodes) 
struct point_plane* tabNodes; 
int iNbNodes; 
{ 
  int i = 1; 
  Curve pCurve; 
  Point_curve pPreviousPoint, pCurrentPoint; 
 
  if((pCurve = mw_new_curve()) == NULL) 
    mwerror(FATAL, 1, "flst_build_external_line --> allocation of new curve failed\n"); 
  pCurve->next = pCurve->previous = NULL; 
  if((pCurrentPoint = pCurve->first = mw_new_point_curve()) == NULL) 
    mwerror(FATAL, 1, "flst_build_external_line --> allocation of first point of curve failed\n"); 
  pCurrentPoint->x = (int)tabNodes[0].x; 
  pCurrentPoint->y = (int)tabNodes[0].y; 
  pCurrentPoint->previous = NULL; 
  do { 
    pPreviousPoint = pCurrentPoint; 
    if((pPreviousPoint->next = pCurrentPoint = mw_new_point_curve()) == NULL) 
      mwerror(FATAL, 1, "flst_build_external_line --> allocation of new point of curve failed\n"); 
    pCurrentPoint->previous = pPreviousPoint; 
    pCurrentPoint->x = (int)tabNodes[i].x; 
    pCurrentPoint->y = (int)tabNodes[i].y; 
  } while(++i < iNbNodes); 
  pCurrentPoint->next = NULL; 
  return pCurve; 
} 
 
/* Create a new shape and insert it in the tree, just under the 'universal ancestor' in case it is really 
   a new shape. This is not the case if the shape has same area than the preceding shape starting from the 
   same point or if the shape meets the border and after computation, it results in an interior area greater 
   than half the image */ 
static void flst_create_new_shape(pNewShape, pUniversalAncestor, iArea, iPreviousArea, value, iNbCcOfFrontier, 
				  bFlagHeightConnectedness, pCurrentExplorationHole, type) 
Shape pNewShape, pUniversalAncestor; 
int iArea, iPreviousArea, iNbCcOfFrontier, *pCurrentExplorationHole; 
float value; /* Its gray level */ 
char bFlagHeightConnectedness; /* Indicates if the shape is in 8- or 4- connectedness */ 
char type; 
{ 
  int iNbNodesOfExternalLine = 0; 
 
  pNewShape->inferior_type = type; 
  pNewShape->value = value; 
  pNewShape->removed = 0; 
  pNewShape->open = bDoesShapeMeetBorder; 
  pNewShape->boundary = NULL; 
  pNewShape->data = NULL; 
  pNewShape->area = flst_follow_holes(iNbCcOfFrontier, iArea, ++(*pCurrentExplorationHole), pNewShape, 
				       bFlagHeightConnectedness, &iNbNodesOfExternalLine); 
  if(pNewShape->area > iMaxArea || pNewShape->area == iPreviousArea) 
    return; 
  /* Store the external level line of the shape if asked for */ 
  if(bStoreExternalLine) 
    pNewShape->boundary = flst_build_external_line(tabNodesOfCurve, iNbNodesOfExternalLine); 
  /* Insert the new shape in the tree */ 
  pNewShape->child = NULL; 
  flst_insert_child_in_tree(pUniversalAncestor, pNewShape); 
} 
 
/* Return the previous sibling of pShape. It is supposed that there is one, no verification is made */ 
static Shape ls_previous_sibling(pShape) 
Shape pShape; 
{ 
  Shape pSibling = pShape->parent->child; 
  if(pSibling == pShape) return 0; 
  while(pSibling->next_sibling != pShape) 
    pSibling = pSibling->next_sibling; 
  return pSibling; 
} 
 
/* Update the smallest shape and the largest shape containing each pixel of tabPoints */ 
static void flst_update_image_of_indexes(tabPoints, iNbPoints, 
					 tabImageOfSmallestShape, tabImageOfLargestShape, iIndex, tabShapes) 
struct point_plane* tabPoints; 
int iNbPoints, iIndex; 
Shape *tabImageOfSmallestShape, *tabImageOfLargestShape; 
Shape tabShapes; 
{ 
  int i, iAbsoluteCoordinate; 
  Shape pNewShape, pIncludedShape; 
 
  pNewShape = &tabShapes[iIndex]; 
  for(i = iNbPoints - 1; i >= 0; i--) 
    { 
      iAbsoluteCoordinate = tabPoints[i].y * iWidth + tabPoints[i].x; 
      if(tabImageOfLargestShape[iAbsoluteCoordinate] == tabShapes) 
	tabImageOfSmallestShape[iAbsoluteCoordinate] = pNewShape; 
      else 
	{ 
	  pIncludedShape = tabImageOfLargestShape[iAbsoluteCoordinate]; 
	  if(pIncludedShape->parent != pNewShape) 
	    { 
	      /* The previous sibling of pIncludedShape cannot be 0 since the current shape is inserted */ 
	      ls_previous_sibling(pIncludedShape)->next_sibling = pIncludedShape->next_sibling; 
	      /* Insert the shape in the list of children of the current shape */ 
	      flst_insert_child_in_tree(pNewShape, pIncludedShape); 
	    } 
	} 
      tabImageOfLargestShape[iAbsoluteCoordinate] = pNewShape; 
    } 
} 
 
/* (x,y) being a local minimum in the image, find the cc of the inferior level sets containing it */ 
static void flst_find_inferior_levels(ou, tabtabImageOfVisitedPixels, x, y, tabShapes, pNbShapes, 
				      bFlagHeightConnectedness, tabImageOfSmallestShape, 
				      tabImageOfLargestShape, pNeighborhood) 
float**ou; 
int** tabtabImageOfVisitedPixels; 
short int x, y; 
struct shape* tabShapes; 
int* pNbShapes; 
char bFlagHeightConnectedness; 
Shape* tabImageOfSmallestShape; /* Indicates for each pixel the smallest shape containing it */ 
Shape* tabImageOfLargestShape; 
struct FlstNeighborhood* pNeighborhood; 
{ 
  float fCurrentGrayLevel, fGrayLevelSmall, fSmallestGrayLevelOfNeighbors; 
  int iAreaSmall=0, iCurrentArea = 0, iCurrentExplorationHole = 0; 
  int iIndexCurrentShape; /* The index in the array tabShapes of the current shape */ 
  /* The number of connected components of the frontier of the current region (= 1 + number of holes) */ 
  int iNbConnectedComponentsOfFrontier = 1; 
  int iPreviousArea = 0; /* The area of the shape last found */ 
  int iNbHolesBefore; /* The number of holes before a new shape is added */ 
 
  bDoesShapeMeetBorder = 0; /* The shape does not meet a priori the border of the image */ 
  fSmallestGrayLevelOfNeighbors = fCurrentGrayLevel = ou[y][x]; 
  flst_reinit_neighborhood(pNeighborhood); /* No neighbor yet */ 
  flst_add_neighbor(pNeighborhood, x, y, fCurrentGrayLevel); 
  while(iCurrentArea < iMaxArea && fSmallestGrayLevelOfNeighbors >= fCurrentGrayLevel) 
    { 
      fCurrentGrayLevel = fSmallestGrayLevelOfNeighbors; 
      if(iCurrentArea < iMinArea) fGrayLevelSmall = fCurrentGrayLevel; 
      if(flst_add_iso_level(tabPointsInCurrentRegion, &iCurrentArea, fCurrentGrayLevel, pNeighborhood, ou, 
			    tabtabImageOfVisitedPixels, 
			    &iNbConnectedComponentsOfFrontier, bFlagHeightConnectedness, (char)1) == 0) 
	break; /* Exit if adding the neighbors at current gray level would make iCurrentArea > iMaxArea */ 
      fSmallestGrayLevelOfNeighbors = pNeighborhood->tabNeighbors[1].value; 
      if(iCurrentArea < iMinArea && fCurrentGrayLevel < fSmallestGrayLevelOfNeighbors) 
	iAreaSmall = iCurrentArea; 
 
      /* Look if it is a new shape */ 
      if(iCurrentArea >= iMinArea && iCurrentArea <= iMaxArea && 
	 fSmallestGrayLevelOfNeighbors > fCurrentGrayLevel) 
	{ 
	  iIndexCurrentShape = (*pNbShapes)++; 
	  iNbHolesBefore = iNbHoles; 
	  flst_create_new_shape(&tabShapes[iIndexCurrentShape], &tabShapes[0],iCurrentArea, iPreviousArea, 
				fCurrentGrayLevel, iNbConnectedComponentsOfFrontier, 
				bFlagHeightConnectedness, &iCurrentExplorationHole, (char)1); 
	  if(tabShapes[iIndexCurrentShape].area == iPreviousArea) 
	    { /* The cc of level set grows by the interior (new pixels are in holes only), no new shape */ 
	      -- (*pNbShapes); 
	      iNbHoles = iNbHolesBefore; 
	      continue; 
	    } 
	  else 
	    if((iPreviousArea = tabShapes[iIndexCurrentShape].area) > iMaxArea) 
	      { /* The shape, together with its holes, is too large */ 
		-- (*pNbShapes); 
		iNbHoles = iNbHolesBefore; 
		break; 
	      } 
	  flst_update_image_of_indexes(tabPointsInCurrentRegion, iCurrentArea, 
				       tabImageOfSmallestShape, tabImageOfLargestShape, 
				       iIndexCurrentShape, tabShapes); 
	} 
    } 
  /* Notice the following commands are safe because tabtabPixelsOriginalImage==0 implies iMinArea==1 */ 
  if(iMinArea > iMaxArea) /* In this case, iMaxArea==iMinArea-1 */ 
    flst_set_at_level(tabtabPixelsOriginalImage, tabPointsInCurrentRegion, iAreaSmall, 
		      MAX(fGrayLevelSmall,fSmallestGrayLevelOfNeighbors)); 
  else 
    flst_set_at_level(tabtabPixelsOriginalImage, tabPointsInCurrentRegion, iAreaSmall, fGrayLevelSmall); 
  if(ou != tabtabPixelsOriginalImage) 
    flst_set_at_level(ou, tabPointsInCurrentRegion, iCurrentArea, fCurrentGrayLevel); 
} 
 
/* For each local minimum in the image, find the cc of level sets containing it */ 
static void flst_scan_for_inferior_levels(tabtabPixelsOutput, tabtabImageOfVisitedPixels, tabShapes, pNbShapes, 
					  bFlagHeightConnectedness, tabImageOfSmallestShape,  
					  tabImageOfLargestShape, pNeighborhood) 
float** tabtabPixelsOutput; 
int** tabtabImageOfVisitedPixels; 
struct shape* tabShapes; 
int* pNbShapes; 
char bFlagHeightConnectedness; 
Shape* tabImageOfSmallestShape; /* Indicates for each pixel the smallest shape containing it */ 
Shape* tabImageOfLargestShape; 
struct FlstNeighborhood* pNeighborhood; 
{ 
  short int i, j; 
  int iLimit = iIndexOfExploration; 
 
  pNeighborhood->bMaximumTree = 0; 
  for(i = 0; i < iHeight; i++) 
    for(j = 0; j < iWidth; j++) 
      if(tabtabImageOfVisitedPixels[i][j] < iLimit && 
	 is_local_min(tabtabPixelsOutput, j, i, bFlagHeightConnectedness)) 
	{ 
	  flst_find_inferior_levels(tabtabPixelsOutput, tabtabImageOfVisitedPixels, j, i, tabShapes, 
				    pNbShapes, bFlagHeightConnectedness, 
				    tabImageOfSmallestShape, tabImageOfLargestShape, 
				    pNeighborhood); 
	  ++ iIndexOfExploration; 
	} 
} 
 
static void flst_find_superior_levels(ou, tabtabImageOfVisitedPixels, x, y, tabShapes, pNbShapes, 
				      bFlagHeightConnectedness, tabImageOfSmallestShape, 
				      tabImageOfLargestShape, pNeighborhood) 
float **ou; 
int** tabtabImageOfVisitedPixels; 
short int x, y; 
struct shape* tabShapes; 
int* pNbShapes; 
char bFlagHeightConnectedness; 
Shape* tabImageOfSmallestShape; /* Indicates for each pixel the smallest shape containing it */ 
Shape* tabImageOfLargestShape; 
struct FlstNeighborhood* pNeighborhood; 
{ 
  float fCurrentGrayLevel, fGrayLevelSmall, fLargestGrayLevelOfNeighbors; 
  int iAreaSmall=0, iCurrentArea = 0, iCurrentExplorationHole = 0; 
  int iIndexCurrentShape; 
  /* The number of connected components of the frontier of the current region (= 1 + number of holes) */ 
  int iNbConnectedComponentsOfFrontier = 1; 
  int iPreviousArea = 0; /* The area of the shape last found */ 
  int iNbHolesBefore; /* The number of holes before a new shape is added */ 
 
  bDoesShapeMeetBorder = 0; /* The shape does not meet a priori the border of the image */ 
  fLargestGrayLevelOfNeighbors = fCurrentGrayLevel = ou[y][x]; 
  flst_reinit_neighborhood(pNeighborhood); /* No neighbor yet */ 
  flst_add_neighbor(pNeighborhood, x, y, fCurrentGrayLevel); 
  while(iCurrentArea < iMaxArea && fLargestGrayLevelOfNeighbors <= fCurrentGrayLevel) 
    { 
      fCurrentGrayLevel = fLargestGrayLevelOfNeighbors; 
      if(iCurrentArea < iMinArea) fGrayLevelSmall = fCurrentGrayLevel; 
      if(flst_add_iso_level(tabPointsInCurrentRegion, &iCurrentArea, fCurrentGrayLevel, pNeighborhood, ou, 
			    tabtabImageOfVisitedPixels, 
			    &iNbConnectedComponentsOfFrontier, bFlagHeightConnectedness, (char)0) == 0) 
	break; 
      fLargestGrayLevelOfNeighbors = pNeighborhood->tabNeighbors[1].value; 
      if(iCurrentArea < iMinArea && fCurrentGrayLevel > fLargestGrayLevelOfNeighbors) 
	iAreaSmall = iCurrentArea; 
 
      /* Look if it is a new shape */ 
      if(iCurrentArea >= iMinArea && iCurrentArea <= iMaxArea && 
	 fLargestGrayLevelOfNeighbors < fCurrentGrayLevel) 
	{ 
	  iIndexCurrentShape = (*pNbShapes)++; 
	  iNbHolesBefore = iNbHoles; 
	  flst_create_new_shape(&tabShapes[iIndexCurrentShape], &tabShapes[0],iCurrentArea,iPreviousArea, 
				fCurrentGrayLevel, iNbConnectedComponentsOfFrontier, 
				bFlagHeightConnectedness, &iCurrentExplorationHole, (char)0); 
	  if(tabShapes[iIndexCurrentShape].area == iPreviousArea) 
	    { /* The cc of level set grows by the interior (new pixels are in holes only), no new shape */ 
	      -- (*pNbShapes); 
	      iNbHoles = iNbHolesBefore; 
	      continue; 
	    } 
	  else 
	    if((iPreviousArea = tabShapes[iIndexCurrentShape].area) > iMaxArea) 
	      { /* The shape, together with its holes, is too large */ 
		-- (*pNbShapes); 
		iNbHoles = iNbHolesBefore; 
		break; 
	      } 
	  flst_update_image_of_indexes(tabPointsInCurrentRegion, iCurrentArea, 
				       tabImageOfSmallestShape, tabImageOfLargestShape, 
				       iIndexCurrentShape, tabShapes); 
	} 
    } 
  /* Notice the following commands are safe because tabtabPixelsOriginalImage==0 implies iMinArea==1 */ 
  if(iMinArea > iMaxArea) /* In this case, iMaxArea==iMinArea-1 */ 
    flst_set_at_level(tabtabPixelsOriginalImage, tabPointsInCurrentRegion, iAreaSmall, 
		      MIN(fGrayLevelSmall,fLargestGrayLevelOfNeighbors)); 
  else 
    flst_set_at_level(tabtabPixelsOriginalImage, tabPointsInCurrentRegion, iAreaSmall, fGrayLevelSmall); 
  if(ou != tabtabPixelsOriginalImage) 
    flst_set_at_level(ou, tabPointsInCurrentRegion, iCurrentArea, fCurrentGrayLevel); 
} 
 
static void flst_scan_for_superior_levels(tabtabPixelsOutput, tabtabImageOfVisitedPixels, tabShapes, pNbShapes, 
					  bFlagHeightConnectedness, tabImageOfSmallestShape, 
					  tabImageOfLargestShape,  pNeighborhood) 
float **tabtabPixelsOutput; int** tabtabImageOfVisitedPixels; 
struct shape* tabShapes; 
int* pNbShapes; 
char bFlagHeightConnectedness; 
Shape* tabImageOfSmallestShape; /* Indicates for each pixel the smallest shape containing it */ 
Shape* tabImageOfLargestShape; 
struct FlstNeighborhood* pNeighborhood; 
{ 
  short int i, j; 
  int iLimit = iIndexOfExploration; 
 
  pNeighborhood->bMaximumTree = (char)1; 
  for(i = 0; i < iHeight; i++) 
    for(j = 0; j < iWidth; j++) 
      if(tabtabImageOfVisitedPixels[i][j] < iLimit && 
	 is_local_max(tabtabPixelsOutput, j, i, bFlagHeightConnectedness)) 
	{ 
	  flst_find_superior_levels(tabtabPixelsOutput, tabtabImageOfVisitedPixels, j, i, tabShapes, 
				    pNbShapes, bFlagHeightConnectedness, 
				    tabImageOfSmallestShape, tabImageOfLargestShape, 
				    pNeighborhood); 
	  ++ iIndexOfExploration; 
	} 
} 
 
/* Correct the tree (merge the trees of cc of level sets using the holes). What we call 'hole' here 
   is a pair (containing shape, contained shape) where the contained shape is a topological hole in 
   the containing shape (struct LsPointHole). An active hole is a hole for which no topological hole of 
   a child of the 'containing shape' contains the 'contained shape'. Only active holes are useful for 
   merging the trees. The steps of the algorithm are: 
   1) Build the holes; actually, the holes are incomplete because we know only a point belonging to the 
   'contained shape'. The goal of this step is to determine the 'contained shape'. 
   2) For each shape, give the hole with the smallest 'containing shape' whose it is the 'contained shape' 
   (or null pointer if it is the 'contained shape' of no hole); active holes belong to this set. 
   3) Find the active holes; they are the holes whose parent of the 'contained shape' is larger than the 
   'containing shape'. 
   4) Use the active holes to merge the trees. For each hole, put the subtree of root the 'contained shape' 
   under the 'containing shape' */ 
static void flst_correct_tree(tabShapes, tabImageOfSmallestShapeInf, tabImageOfSmallestShapeSup) 
struct shape* tabShapes; 
Shape *tabImageOfSmallestShapeInf, *tabImageOfSmallestShapeSup; 
{ 
  int i, iIndexShape, iHole; 
  Shape pContainer, pHole, pCurrentShape, pPreviousSibling; 
  float value, valueOfBackground = tabShapes[0].value; 
  struct LsPointHole** tabpActiveHoles; /* Active holes are the holes that are used for merging the trees */ 
  struct LsPointHole** ppActiveHole; 
 
  /* 1) For each hole, find the contained shape */ 
  for(i = iNbHoles-1; i >= 0; i--) 
    { 
      pContainer = tabHoles[i].pContainingShape; 
      value = pContainer->value; 
      if(pContainer->inferior_type) { 
	pHole = tabImageOfSmallestShapeSup[tabHoles[i].point.y * iWidth + tabHoles[i].point.x]; 
	tabShapes[0].value = - FLT_MAX; 
	while(pHole->parent != NULL && pHole->parent->value > value) 
	  pHole = pHole->parent; 
	if(pHole->value <= value) 
	  mwerror(FATAL, 1, "flst_correct_tree --> A hole is too large\n"); 
	tabHoles[i].pShapeOfHole = pHole; 
      } else { 
	pHole = tabImageOfSmallestShapeInf[tabHoles[i].point.y * iWidth + tabHoles[i].point.x]; 
	tabShapes[0].value = FLT_MAX; 
	while(pHole->parent != NULL && pHole->parent->value < value) 
	  pHole = pHole->parent; 
	if(pHole->value >= value) 
	  mwerror(FATAL, 1, "flst_correct_tree --> A hole is too large\n"); 
	tabHoles[i].pShapeOfHole = pHole; 
      } 
    } 
  tabShapes[0].value = valueOfBackground; /* Restore the gray level of the background */ 
 
  tabpActiveHoles = (struct LsPointHole**) calloc((size_t)pGlobalTree->nb_shapes, sizeof(struct LsPointHole*)); 
  if(tabpActiveHoles == NULL) 
    mwerror(FATAL, 1, "flst_correct_tree --> not enough memory to allocate the array of active holes"); 
 
  /* 2) If a shape represents the same 'contained shape' for two holes, the bigger one cannot be active. */ 
  for(i = iNbHoles-1; i >= 0; i--) 
    { 
      iIndexShape = tabHoles[i].pShapeOfHole - tabShapes; /* Index of the shape in the array */ 
      ppActiveHole = &tabpActiveHoles[iIndexShape]; 
      if(*ppActiveHole == NULL || 
	 (*ppActiveHole)->pContainingShape->area > tabHoles[i].pContainingShape->area) 
	*ppActiveHole = &tabHoles[i]; /* Replace with the hole of smaller 'containing shape' */ 
    } 
 
  /* 3) Determine really the active holes */ 
  for(i = pGlobalTree->nb_shapes-1; i > 0; i--) 
    if(tabpActiveHoles[i] != NULL && 
       tabpActiveHoles[i]->pShapeOfHole->parent->area < tabpActiveHoles[i]->pContainingShape->area) 
      tabpActiveHoles[i] = NULL; 
 
  /* 4) Merge the trees, using the active holes */ 
  for(i = pGlobalTree->nb_shapes-1; i > 0; i--) 
    if(tabpActiveHoles[i] != NULL) 
      { 
	pHole = tabpActiveHoles[i]->pShapeOfHole; 
	/* Extract the shape from the tree */ 
	pPreviousSibling = ls_previous_sibling(pHole); 
	if(pPreviousSibling != NULL) 
	  pPreviousSibling->next_sibling = pHole->next_sibling; 
	else 
	  pHole->parent->child = pHole->next_sibling; 
	/* Insert the shape in the tree at its right place */ 
	flst_insert_child_in_tree(tabpActiveHoles[i]->pContainingShape, pHole); 
      } 
 
  free(tabpActiveHoles); 
} 
 
/* Now, update the image giving for each pixel the smallest shape containing it */ 
static void flst_update_image_of_smallest_shape(pTree, tabImageOfSmallestShapeSup) 
Shapes pTree; 
Shape* tabImageOfSmallestShapeSup; 
{ 
  Shape *ppShape1, *ppShape2; 
  int i; 
   
  ppShape1 = pTree->smallest_shape; 
  ppShape2 = tabImageOfSmallestShapeSup; 
  for(i = pTree->nrow*pTree->ncol-1; i >= 0; i--, ppShape1++, ppShape2++) 
    if((*ppShape1)->area > (*ppShape2)->area) 
      *ppShape1 = *ppShape2; 
} 
 
/* Associate to each shape its array of pixels, meaning that we initialize the field 
   pixels of each shape. The tree structure is used to avoid redundancy in allocated 
   memory: each field is in fact a pointer to a subarray of the pixels of the root, which 
   are organized in a smart way. The size of the array pixels is of course the area of 
   the shape */ 
static void flst_find_pixels_of_shapes(pTree) 
Shapes pTree; 
{ 
  Shape *tabpShapesOfStack, pShape, *ppShape; 
  int *tabNbOfProperPixels, *pNbOfProperPixels; /* Indicates for each shape its number of proper pixels */ 
  int i, j, iSizeOfStack, iIndex; 
  struct point_plane *tabPixelsOfRoot, *pCurrentPoint; 
 
  /* 1) Find for each shape its number of proper pixels, that is pixels contained in the shape but not 
     in one of its children. */ 
  if((tabNbOfProperPixels = (int*) malloc(pTree->nb_shapes * sizeof(int))) == NULL) 
    mwerror(FATAL, 1, "flst_find_pixels_of_shapes --> Allocation of the array of proper pixels failed\n"); 
  /* Initialize by the area */ 
  pShape = pTree->the_shapes + pTree->nb_shapes-1; 
  pNbOfProperPixels = tabNbOfProperPixels + pTree->nb_shapes-1; 
  for(i = pTree->nb_shapes-1; i >= 0; i--) 
    *pNbOfProperPixels-- = (pShape--)->area; 
  /* For each shape, substract its area to its parent */ 
  pShape = pTree->the_shapes + pTree->nb_shapes-1; 
  for(i = pTree->nb_shapes-1; i > 0; i--) 
    tabNbOfProperPixels[pShape->parent - pTree->the_shapes] -= (pShape--)->area; 
 
  /* 2) Enumerate the shapes in preorder. What follows is equivalent (but more efficient) to 
     the following call: unwrap_tree(pTree->the_shapes[0]) where the recursive function unwrap_tree would 
     be: unwrap_tree(pShape) { add_proper_pixels(pShape); for each child of pShape, unwrap_tree(child); } 
     This nonrecursive version is similar to [Aho,Hopcroft,Ullman, Data Structures and Algorithms, 
     Addison-Wesley, 1983, p. 85]. It uses a temporary stack of shapes (tabpShapesOfStack) giving 
     the path from the root to the current shape. Thus its max size is the depth of the tree (not computed), 
     therefore always <= to the number of shapes */ 
  tabpShapesOfStack = (Shape*)malloc(pTree->nb_shapes * sizeof(Shape)); 
  if(tabpShapesOfStack == NULL) 
    mwerror(FATAL, 1, "flst_find_pixels_of_shapes --> Allocation of the stack of shapes failed\n"); 
  pShape = &pTree->the_shapes[0]; 
  tabPixelsOfRoot = pShape->pixels = (struct point_plane*) 
    malloc(pTree->nrow*pTree->ncol * sizeof(struct point_plane)); 
  if(tabPixelsOfRoot == NULL) 
    mwerror(FATAL, 1, "flst_find_pixels_of_shapes --> impossible to allocate the pixels of the root\n"); 
  iSizeOfStack = 0; i = 0; 
  while(1) 
    if(pShape != NULL) { 
      /* Write pixels of pShape */ 
      pShape->pixels = &tabPixelsOfRoot[i]; 
      iIndex = pShape - pTree->the_shapes; 
      i += tabNbOfProperPixels[iIndex]; 
      tabpShapesOfStack[iSizeOfStack++] = pShape; /* Push the shape in the stack */ 
      pShape = pShape->child; 
    } else { 
      if(iSizeOfStack == 0) 
	break; 
      pShape = tabpShapesOfStack[--iSizeOfStack]->next_sibling; /* Pop the shape in the stack */ 
    } 
  free(tabpShapesOfStack); 
 
  /* 3) Write the pixels */ 
  ppShape = pTree->smallest_shape + pTree->ncol*pTree->nrow-1; 
  for(i = pTree->nrow-1; i >= 0; i--) 
    for(j = pTree->ncol-1; j >= 0; j--) 
      { 
	iIndex = (*ppShape) - pTree->the_shapes; 
	pCurrentPoint = &(*ppShape--)->pixels[--tabNbOfProperPixels[iIndex]]; 
	pCurrentPoint->x = j; pCurrentPoint->y = i; 
      } 
  free(tabNbOfProperPixels); 
} 
 
/* Reconstruct an image from the tree */ 
void mw_ls_tree_to_image(pTree, pFloatImageOutput) 
Shapes pTree; 
Fimage pFloatImageOutput; 
{ 
  int i; 
  float* pOutputPixel; 
  Shape* ppShapeOfPixel; 
  Shape pShape; 
 
  if(! mw_change_fimage(pFloatImageOutput, pTree->nrow, pTree->ncol)) 
    mwerror(FATAL, 1, "mw_ls_tree_to_image --> impossible to allocate the output image\n"); 
   
  pOutputPixel = pFloatImageOutput->gray; 
  ppShapeOfPixel = pTree->smallest_shape; 
  for(i = pTree->nrow*pTree->ncol-1; i >= 0; i--) 
    { 
      pShape = *ppShapeOfPixel++; 
      while(pShape->removed) 
	pShape = pShape->parent; 
      *pOutputPixel++ = pShape->value; 
    } 
} 
 
/* -------------------------------------------------------------------------------------- 
   --------- The main function ---------------------------------------------------------- 
   -------------------------------------------------------------------------------------- */ 
 
/* Fill the structure of pTree with the representation of the image pFloatImageInput as a tree of 
   interiors of level lines. If pMinArea!=NULL, a Luc Vincent filter of area *pMinArea is applied to 
   pFloatImageInput before the decomposition */ 
void flst(pMinArea, pFloatImageInput, pTree, pStoreCurves, pMode) 
int *pMinArea; /* Shapes of area less than *pMinArea are suppressed from the tree */ 
Fimage pFloatImageInput; 
Shapes pTree; 
char* pStoreCurves; /* Indicates if we have to store the external line of each shape */ 
int* pMode; /* The mode */ 
{ 
  char bFlagHeightConnectedness; /* Indicates whether the region is in 4- (=0) or 8- (!=0) connectedness */ 
  char bFindInfBeforeSup; /* Indicates if we should extract inf level sets before sup */ 
  /* Images giving for each pixel the index in the array tabShapes of the smallest shape containing it */ 
  Shape *tabImageOfSmallestShapeInf, *tabImageOfSmallestShapeSup; 
  Shape* tabImageOfLargestShape; 
  float **tabtabPixelsOutput=NULL; /* A 2-D array accessing the pixels of the output image */ 
  struct FlstNeighborhood neighborhood; /* The neighborhood of the current region */ 
  int** tabtabImageOfVisitedPixels; /* The image saying for each pixel when it has been last visited */ 
  Fimage pTemporaryFloatImage; 
  int i, iMaxAreaSave; /* To keep the true max area */ 
  char cMode; 
 
  pTree = mw_change_shapes(pTree, pFloatImageInput->nrow, pFloatImageInput->ncol, FLT_MAX); 
  if(pTree == NULL) 
    mwerror(FATAL, 1, "flst --> Not enough memory to allocate the tree of shapes"); 
 
  if(pMode == NULL) 
    cMode = 0; 
  else 
    cMode = (char)(*pMode); 
 
  bStoreExternalLine = (char)(pStoreCurves != 0); 
  pGlobalTree = pTree; 
  iWidth = pFloatImageInput->ncol; 
  iHeight = pFloatImageInput->nrow; 
  iAreaImage = iWidth * iHeight; iHalfAreaImage = iAreaImage / 2; 
  tabtabPixelsOriginalImage = 0; 
 
  iMinArea = (pMinArea != NULL && *pMinArea > 0) ? (*pMinArea) : 1; 
  iMaxArea = pFloatImageInput->nrow * pFloatImageInput->ncol - 1; 
  init_image_of_visited_pixels(&tabtabImageOfVisitedPixels); 
  init_neighborhood(&neighborhood); 
  init_region(); 
  init_nodes_of_curve(); 
 
  /* --------------------- Remove too small cc of upper or lower level sets ----------- */ 
  iIndexOfExploration = 1; 
  if(iMinArea > 1 && (cMode & MW_LS_UNCOMPOSED) != MW_LS_UNCOMPOSED) 
    { 
      init_output_image(pFloatImageInput->gray, pFloatImageInput->gray, &tabtabPixelsOriginalImage); 
      iMaxAreaSave = iMaxArea; iMaxArea = iMinArea-1; 
      if((cMode & MW_LS_SUP) == MW_LS_SUP) 
	flst_scan_for_inferior_levels(tabtabPixelsOriginalImage, tabtabImageOfVisitedPixels, 
				      0, 0, 
				      (char)((cMode&MW_LS_8_FIRST) == MW_LS_8_FIRST), 0, 0, &neighborhood); 
      else 
	flst_scan_for_superior_levels(tabtabPixelsOriginalImage, tabtabImageOfVisitedPixels, 
				      0, 0, 
				      (char)((cMode&MW_LS_8_FIRST) == MW_LS_8_FIRST), 0, 0, &neighborhood); 
      iMaxArea = iMaxAreaSave; 
    } 
 
  tabHoles = (struct LsPointHole*) malloc(MAXHOLES * sizeof(struct LsPointHole)); 
  if(tabHoles == NULL) 
    mwerror(FATAL, 1, "mw_flst --> impossible to allocate the array of holes"); 
  iNbHoles = 0; 
 
  pTemporaryFloatImage = mw_change_fimage(NULL, iHeight, iWidth); 
  if(pTemporaryFloatImage == NULL) 
    mwerror(FATAL, 1, "mw_flst --> impossible to allocate the output image\n"); 
  init_output_image(pFloatImageInput->gray, pTemporaryFloatImage->gray, &tabtabPixelsOutput); 
  init_frontier_pixels(); 
 
  init_patterns(); 
 
  /* Initialize the image of the indexes of smallest shapes, one for each tree (inf and sup) */ 
  tabImageOfSmallestShapeInf = pTree->smallest_shape; 
  tabImageOfSmallestShapeSup = (Shape*) malloc(iAreaImage * sizeof(Shape)); 
  if(tabImageOfSmallestShapeSup == NULL) 
    mwerror(FATAL, 1, "mw_flst --> impossible to allocate the image of indexes of smallest shape sup\n"); 
  for(i = iAreaImage-1; i >= 0; i--) 
    tabImageOfSmallestShapeSup[i] = pTree->the_shapes; 
  tabImageOfLargestShape = (Shape*) malloc(iAreaImage * sizeof(Shape)); 
  if(tabImageOfLargestShape == NULL) 
    mwerror(FATAL, 1, "mw_flst --> impossible to allocate the image of indexes of largest shape\n"); 
  for(i = iAreaImage-1; i >= 0; i--) 
    tabImageOfLargestShape[i] = pTree->the_shapes; 
 
  /* --------------------- Find level sets of one type -------------------------- */ 
  bFindInfBeforeSup = (char)((cMode & MW_LS_SUP) != MW_LS_SUP); 
  bFlagHeightConnectedness = (char)((cMode & MW_LS_8) == MW_LS_8); 
  if(bFindInfBeforeSup) 
    flst_scan_for_inferior_levels(tabtabPixelsOutput, tabtabImageOfVisitedPixels, 
				  pTree->the_shapes, &pTree->nb_shapes, bFlagHeightConnectedness, 
				  tabImageOfSmallestShapeInf, tabImageOfLargestShape, &neighborhood); 
  else 
    flst_scan_for_superior_levels(tabtabPixelsOutput, tabtabImageOfVisitedPixels, 
				  pTree->the_shapes, &pTree->nb_shapes, bFlagHeightConnectedness, 
				  tabImageOfSmallestShapeSup, tabImageOfLargestShape, &neighborhood);     
  if(tabtabPixelsOriginalImage != NULL) 
    free(tabtabPixelsOriginalImage); 
 
  /* --------------------- Find level sets of the other type ------------------- */ 
  memcpy(pTemporaryFloatImage->gray, pFloatImageInput->gray, iAreaImage * sizeof(float)); 
  for(i = iAreaImage-1; i >= 0; i--) 
    tabImageOfLargestShape[i] = pTree->the_shapes; 
  bFlagHeightConnectedness = (char)(bFlagHeightConnectedness == 0); /* Change the notion of connectedness */ 
  iMinArea = 1; /* This is not necessary, but can save time */ 
  if(bFindInfBeforeSup) 
    flst_scan_for_superior_levels(tabtabPixelsOutput, tabtabImageOfVisitedPixels, 
				  pTree->the_shapes, &pTree->nb_shapes, bFlagHeightConnectedness, 
				  tabImageOfSmallestShapeSup, tabImageOfLargestShape, &neighborhood);     
  else 
    flst_scan_for_inferior_levels(tabtabPixelsOutput, tabtabImageOfVisitedPixels, 
				  pTree->the_shapes, &pTree->nb_shapes, bFlagHeightConnectedness, 
				  tabImageOfSmallestShapeInf, tabImageOfLargestShape, &neighborhood); 
 
  /* --------------------- Merge the trees ----------------------------------- */ 
  /* Now, we have to correct the tree: inclusions must be updated between inferior and superior level sets */ 
  flst_correct_tree(pTree->the_shapes, tabImageOfSmallestShapeInf, tabImageOfSmallestShapeSup); 
 
  /* --------------------- Update the image of smallest shape ---------------- */ 
  /* Now, update the image giving for each pixel the smallest shape containing it */ 
  flst_update_image_of_smallest_shape(pTree, tabImageOfSmallestShapeSup); 
   
  /* --------------------- Attribute to each shape its pixels ---------------- */ 
  flst_find_pixels_of_shapes(pTree); 
 
  free(tabHoles); 
  free(tabImageOfSmallestShapeSup); 
  free(tabImageOfLargestShape); 
  free_frontier_pixels(); 
  free_image_of_visited_pixels(tabtabImageOfVisitedPixels); 
  free_region(); 
  free_nodes_of_curve(); 
  free_neighborhood(&neighborhood); 
  free_output_image(tabtabPixelsOutput); 
  mw_delete_fimage(pTemporaryFloatImage); 
} 

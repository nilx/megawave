/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
 name = {flstb_quantize}; 
 version = {"1.2"}; 
 author = {"Pascal Monasse"}; 
 function = {"Quantize the Fast Level Sets Transform of a bilinear interpolated image"};
 usage = { 
   'l':levels->pLevels             "Levels of quantization (Fsignal)",
   'o':offset->pOffset             "Offset of the quantization (default 0.5)",
   's':step->pStep                 "Step of the quantization (default 1.0)",
   bilinear_tree->pTree            "The tree of the interpolated image",
   quantized_tree<-pQuantizedTree  "The tree of the quantized image"
}; 
*/ 
/*-------------------------------------------------------------------------
 v1.1: fixed pBound memory allocation bug (P.Monasse)
 v1.2: changed default offset to 0.5 (L.Moisan)
-------------------------------------------------------------------------*/

#include <math.h>
#include "mw.h"

extern void flst_pixels();


int compare_floats(p1, p2)
     void *p1, *p2;
{
  float f1= *(float*)p1, f2 = *(float*)p2;
  if(f1 == f2) return 0;
  if(f1 > f2) return 1;
  return -1;
}

/* Return min and max values of image represented by pTree */
void find_bounds(pTree, pMin, pMax)
     Shapes pTree;
     float *pMin, *pMax;
{
  int i;
  *pMin = *pMax = pTree->the_shapes[0].value;
  for(i = pTree->nb_shapes-1; i > 0; i--)
    if(pTree->the_shapes[i].value < *pMin)
      *pMin = pTree->the_shapes[i].value;
    else if(pTree->the_shapes[i].value > *pMax)
      *pMax = pTree->the_shapes[i].value;
}

/* Return the family of levels associated to the offset and the step */
Fsignal build_signal(pTree, pOffset, pStep)
     Shapes pTree;
     float *pOffset, *pStep;
{
  Fsignal pLevels;
  float fMin, fMax, fOffset, fStep, *pValue;
  int i, iMin, iMax;
  
  find_bounds(pTree, &fMin, &fMax);
  fOffset = (pOffset == NULL) ? 0.5    : *pOffset;
  fStep =   (pStep == NULL)   ? (float)1.0 : *pStep;
  iMin = (int) ceil ((double)((fMin-fOffset)/fStep));
  iMax = (int) floor((double)((fMax-fOffset)/fStep));

  pLevels = mw_change_fsignal(NULL, iMax-iMin+1);
  if(pLevels == NULL)
    mwerror(FATAL, 1, "Not enough memory to allocate the signal");
  pValue = pLevels->values;
  for(i = iMin; i <= iMax; i++)
    *pValue++ = fOffset + i*fStep;

  return pLevels;
}

/* Return min{i: pLevels->values[i] >= fValue} if inf==1 or
          max{i: pLevels->values[i] <= fValue} otherwise */
int lookup_index(pLevels, fValue, inf)
     Fsignal pLevels;
     float fValue;
     char inf;
{
  int iMin = 0, iMax = pLevels->size-1, iMed;
  if(inf) {
    if(fValue > pLevels->values[iMax]) return pLevels->size;
    while(iMin < iMax) {
      iMed = (iMin+iMax) >> 1;
      if(pLevels->values[iMed] < fValue)
	iMin = iMed+1;
      else
	iMax = iMed;
    } 
  } else {
    if(fValue < pLevels->values[0]) return -1;
    while(iMin < iMax) {
      iMed = (iMin+iMax+1) >> 1;
      if(pLevels->values[iMed] > fValue)
	iMax = iMed-1;
      else
	iMin = iMed;
    }
  }
  return iMin;
}

/* Count the number of quantized shapes in the tree */
int count_shapes(pTree, tabIndices, pLevels)
     Shapes pTree;
     int* tabIndices;
     Fsignal pLevels;
{
  int i, iDiff, iNbShapes = 1; /* For the root */
  Shape pRoot = pTree->the_shapes;

  for(i = pTree->nb_shapes-1; i > 0; i--) {
    if(pRoot[i].inferior_type == pRoot[i].parent->inferior_type)
      iDiff = tabIndices[i] - tabIndices[pRoot[i].parent-pRoot];
    else
      iDiff = tabIndices[i] - lookup_index(pLevels, pRoot[i].parent->value,
					   pRoot[i].inferior_type);
    if(iDiff < 0) iDiff = -iDiff;
    iNbShapes += iDiff;
  }
  return iNbShapes;
}

/* Build a new shape, of parent pParent, variation of pShapeTemplate */
void build_shape(pShapeTemplate, pShape, fValue, pParent)
Shape pShapeTemplate, pShape, pParent;
float fValue;
{
  *pShape = *pShapeTemplate;
  pShape->value = fValue;
  pShape->boundary = 0;
  pShape->parent = pParent;
  pShape->next_sibling = pParent->child;
  pShape->child = NULL;
  pShape->data = NULL;
  pShape->data_size = 0;
  pParent->child = pShape;
}

/* Main algorithm: enumerate the shapes of original tree in preorder, where
for each shape, construct the shapes of the quantized tree comprised between
it and the parent */
void fill_quantized_tree(pTree, pQuantizedTree, pLevels, tabIndices)
     Shapes pTree, pQuantizedTree;
     Fsignal pLevels;
     int* tabIndices;
{
  Shape *pStack, pShape; /* In the original tree */
  Shape *pStackNewShapes, pNewShape, pParent; /* In quantized tree */
  Point_plane pPixel, pBound, pPixelsIn, pPixelsOut;
  int i, i1, i2, iSizeStack, iWidth = pTree->ncol;
  
  pPixelsIn = pTree->the_shapes[0].pixels;
  pPixelsOut = pQuantizedTree->the_shapes[0].pixels;
  
  /* Allocation of size=depth of original tree would be sufficient */
  pStack = (Shape*) malloc(pTree->nb_shapes * sizeof(Shape));
  pStackNewShapes = (Shape*) malloc(pTree->nb_shapes * sizeof(Shape));
  if(pStack == NULL || pStackNewShapes == NULL)
    mwerror(FATAL, 1, "Memory error in construction of an array of shapes");
  
  pStack[0] = &pTree->the_shapes[0];
  pShape = pStack[0]->child;
  pStackNewShapes[0] = pNewShape = pQuantizedTree->the_shapes;
  if(pShape == NULL)
    pBound = pStack[0]->pixels + pStack[0]->area;
  else
    pBound = pShape->pixels;
  for(pPixel = pStack[0]->pixels; pPixel != pBound; pPixel++)
    pQuantizedTree->smallest_shape[pPixel->y*iWidth+pPixel->x] = pNewShape;
  iSizeStack = 1;
  while(1)
    if(pShape == NULL) {
      if(iSizeStack == 1)
	break;
      pShape = pStack[--iSizeStack]->next_sibling; /* Pop */
    } else {
      if(pShape->inferior_type == pStack[iSizeStack-1]->inferior_type)
	i1 = tabIndices[pStack[iSizeStack-1]-pTree->the_shapes];
      else 
	i1 = lookup_index(pLevels, pStack[iSizeStack-1]->value,
			  pShape->inferior_type);
      i2 = tabIndices[pShape-pTree->the_shapes];
      pParent = pStackNewShapes[iSizeStack-1];
      if(i1 < i2)
	for(i = i1+1; i <= i2; i++, pParent=pNewShape) {
	  build_shape(pShape, ++pNewShape, pLevels->values[i], pParent);
	  pNewShape->pixels = pPixelsOut + (pShape->pixels-pPixelsIn);
	}
      else
	for(i = i1-1; i >= i2; i--, pParent=pNewShape) {
	  build_shape(pShape, ++pNewShape, pLevels->values[i], pParent);
	  pNewShape->pixels = pPixelsOut + (pShape->pixels-pPixelsIn);
	}
      if(pShape->child == NULL)
	pBound = pShape->pixels + pShape->area;
      else
	pBound = pShape->child->pixels;
      for(pPixel = pShape->pixels; pPixel != pBound; pPixel++)
	pQuantizedTree->smallest_shape[pPixel->y*iWidth+pPixel->x] = pParent;
      pStackNewShapes[iSizeStack] = pParent;
      pStack[iSizeStack++] = pShape; /* Push */
      pShape = pShape->child;
    }
  free(pStack);
  free(pStackNewShapes);
}

void quantize_tree(pLevels, pTree, pQuantizedTree)
     Fsignal pLevels;
     Shapes pTree, pQuantizedTree;
{
  int i;
  int *tabIndices; /* Position of gray level of each shape in pLevels */

  tabIndices = (int*)malloc(pTree->nb_shapes * sizeof(int));
  if(tabIndices == NULL)
    mwerror(FATAL, 1, "Not enough memory for the array of indices");
  for(i = pTree->nb_shapes-1; i >= 0; i--)
    tabIndices[i] = lookup_index(pLevels, pTree->the_shapes[i].value,
				 pTree->the_shapes[i].inferior_type);

  pQuantizedTree->nrow = pTree->nrow; pQuantizedTree->ncol = pTree->ncol;
  pQuantizedTree->interpolation = 1;
  pQuantizedTree->nb_shapes = count_shapes(pTree, tabIndices, pLevels);
  pQuantizedTree->the_shapes = (Shape)
    malloc(pQuantizedTree->nb_shapes*sizeof(struct shape));
  if(pQuantizedTree->the_shapes == NULL)
    mwerror(FATAL, 1, "Error allocating %d shapes", pQuantizedTree->nb_shapes);
  pQuantizedTree->the_shapes[0] = pTree->the_shapes[0];
  pQuantizedTree->the_shapes[0].child = NULL;
  pQuantizedTree->the_shapes[0].boundary = NULL;
  pQuantizedTree->smallest_shape = (Shape*)
    malloc(pTree->nrow*pTree->ncol*sizeof(Shape));
  if(pQuantizedTree->smallest_shape == NULL)
    mwerror(FATAL, 1, "Error allocating array");

  if(pTree->the_shapes[0].pixels == NULL)
    flst_pixels(pTree);
  pQuantizedTree->the_shapes[0].pixels = (Point_plane)
    malloc(pTree->nrow*pTree->ncol*sizeof(struct point_plane));
  if(pQuantizedTree->the_shapes[0].pixels == NULL)
    mwerror(FATAL, 1, "Unable to allocate array of points");
  memcpy(pQuantizedTree->the_shapes[0].pixels, pTree->the_shapes[0].pixels,
	 pTree->nrow*pTree->ncol*sizeof(struct point_plane));

  fill_quantized_tree(pTree, pQuantizedTree, pLevels, tabIndices);
  free(tabIndices);
}

void flstb_quantize(pLevels, pOffset, pStep, pTree, pQuantizedTree)
     Fsignal pLevels;
     float *pOffset, *pStep;
     Shapes pTree, pQuantizedTree;
{
  char bNewLevels = (pLevels == NULL);

  if(pLevels != NULL && (pOffset != NULL || pStep != NULL))
    mwerror(USAGE, 1, "Options levels and (offset+step) are exclusive");
  if(pStep != NULL && *pStep <= 0)
    mwerror(USAGE, 1, "Step must be positive");
  if(pTree->interpolation != 1)
    mwerror(USAGE, 1, "Please apply this module to a *bilinear* tree");
  
  if(pLevels == NULL)
    pLevels = build_signal(pTree, pOffset, pStep);
  else
    qsort(pLevels->values, pLevels->size, sizeof(float), compare_floats);

  quantize_tree(pLevels, pTree, pQuantizedTree);

  if(bNewLevels)
    mw_delete_fsignal(pLevels);
}

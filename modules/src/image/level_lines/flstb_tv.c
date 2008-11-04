/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
 name = {flstb_tv}; 
 version = {"1.1"}; 
 author = {"Pascal Monasse"}; 
 function = {"TV minimization based on bilinear FLST"};
 usage = { 
   't':[scale=5]->pScale                     "Scale",
   'q':[quantization=1.]->pQuantizationLevel "gray level quantization",
   'p': [precision=4]->pQuantizationCurve    "Curve quantization precision",
   bilinear_tree -> pTree       "The tree of the interpolated image",
   image <- pImage              "The output image"
}; 
*/ 
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "mw.h"
#include "mw-modules.h" /* for fsaddles(), flst_reconstruct(),
			   flstb_dualchain(), flstb_boundary() */

#ifndef ABS
#define ABS(x) ((x) >= 0 ? (x) : -(x))
#endif

#ifndef POINT_T
#define POINT_T
typedef struct {
  float x, y;
} point_t;
#endif

static float curve_perimeter(pCurve, iHeight, iWidth)
     Flist pCurve;
     int iHeight, iWidth;
{
  point_t *pPoint, point, pointNext;
  int i;
  double dPerimeter = 0;
  double dx, dy;

  pPoint = (point_t*) pCurve->values;
  for(i = 0; i+1 < pCurve->size; i++, pPoint++)
  {
    dx = pPoint[1].x - pPoint->x;
    dy = pPoint[1].y - pPoint->y;
    dPerimeter += sqrt(dx * dx + dy * dy);
  }
  /* For open curves, close it by the boundary of the image */
  pointNext = *pPoint;
  while(pointNext.x != pCurve->values[0] || pointNext.y != pCurve->values[1]) {
    point = pointNext;
    if(point.x == 0 && point.y != iHeight) {
      if(pCurve->values[0] == 0 && pCurve->values[1] > point.y)
	pointNext.y = pCurve->values[1];
      else
	pointNext.y = iHeight;
      dPerimeter += pointNext.y - point.y;
    } else if(point.y == iHeight && point.x != iWidth) {
      if(pCurve->values[1] == iHeight && pCurve->values[0] > point.x)
	pointNext.x = pCurve->values[0];
      else
	pointNext.x = iWidth;
      dPerimeter += pointNext.x - point.x;
    } else if(point.x == iWidth && point.y != 0) {
      if(pCurve->values[0] == iWidth && pCurve->values[1] < point.y)
	pointNext.y = pCurve->values[1];
      else
	pointNext.y = 0;
      dPerimeter += point.y - pointNext.y;
    } else {
      if(pCurve->values[1] == 0 && pCurve->values[0] < point.x)
	pointNext.x = pCurve->values[0];
      else
	pointNext.x = 0;
      dPerimeter += point.x - pointNext.x;
    }
  }
  return (float)dPerimeter;
}

static float curve_area(pCurve, iHeight, iWidth)
     Flist pCurve;
     int iHeight, iWidth;
{
  point_t *pPoint, point, pointNext;
  int i;
  double dArea = 0;

  pPoint = (point_t*) pCurve->values;
  for(i = 0; i+1 < pCurve->size; i++, pPoint++)
    dArea += (pPoint->x+pPoint[1].x) * (pPoint->y-(double)pPoint[1].y);

  /* For open curves, close it by the boundary of the image */
  pointNext = *pPoint;
  while(pointNext.x != pCurve->values[0] || pointNext.y != pCurve->values[1]) {
    point = pointNext;
    if(point.x == 0 && point.y != iHeight)
      if(pCurve->values[0] == 0 && pCurve->values[1] > point.y)
	pointNext.y = pCurve->values[1];
      else
	pointNext.y = iHeight;
    else if(point.y == iHeight && point.x != iWidth)
      if(pCurve->values[1] == iHeight && pCurve->values[0] > point.x)
	pointNext.x = pCurve->values[0];
      else
	pointNext.x = iWidth;
    else if(point.x == iWidth && point.y != 0) {
      if(pCurve->values[0] == iWidth && pCurve->values[1] < point.y)
	pointNext.y = pCurve->values[1];
      else
	pointNext.y = 0;
      dArea += 2 * iWidth * (point.y - (double)pointNext.y);
    } else /* if(point.y == 0) */
      if(pCurve->values[1] == 0 && pCurve->values[0] < point.x)
	pointNext.x = pCurve->values[0];
      else
	pointNext.x = 0;
  }
  return (float)(dArea * .5);
}

static void decrease_tv(tabScales, fQuantizationLevel, iQuantizationCurve, pImage,
		 pTree, tabtabSaddleValues)
     float *tabScales, fQuantizationLevel;
     int iQuantizationCurve;
     Fimage pImage;
     Shapes pTree;
     float **tabtabSaddleValues;
{
  Shape pShape;
  Flists pListBoundaries;
  Flist pBoundary;
  float fPerimeter, fArea, fCurvature, fDeltaLevel;
  char bDecrease, bNoChange;
  int i;

  pListBoundaries = mw_change_flists(NULL, pTree->nb_shapes, pTree->nb_shapes);
  if(pListBoundaries == NULL)
    mwerror(FATAL, 1, "List of boundaries allocation error");
  for(i = pListBoundaries->size-1; i >= 0; i--)
    pListBoundaries->list[i] = NULL;

  /* Compute global curvatures of leaves */
  do {
    bDecrease = 0;
    pShape = &pTree->the_shapes[pTree->nb_shapes-1];
    for(i = pTree->nb_shapes-1; i > 0; i--, pShape--) {
      if(pShape->removed || mw_get_first_child_shape(pShape) != NULL)
	continue;
      while(tabScales[i] < tabScales[0]) {
	if(pListBoundaries->list[i] == NULL) {
	  if((pListBoundaries->list[i] = mw_new_flist()) == NULL)
	    mwerror(FATAL, 1, "List allocation error");
	  /* FIXME: cast */
	  flstb_dualchain(pTree, pShape, pListBoundaries->list[i],
			  (char *) tabtabSaddleValues);
	}
	  /* FIXME: cast */
	pBoundary = flstb_boundary(&iQuantizationCurve, pImage, pTree, pShape,
				   pListBoundaries->list[i], NULL, 
				   (char *) tabtabSaddleValues);
	fPerimeter = curve_perimeter(pBoundary, pTree->nrow, pTree->ncol);
	fArea = curve_area(pBoundary, pTree->nrow, pTree->ncol);
	assert((fArea>0 && fPerimeter>0) || (fArea==0 && fPerimeter>=0));
	bNoChange = (char)(fArea == 0);
	if(bNoChange)
	  fDeltaLevel = fQuantizationLevel;
	else {
	  fCurvature = fPerimeter / fArea;
	  fDeltaLevel = fCurvature * (tabScales[0] - tabScales[i]);
	  if(fDeltaLevel > fQuantizationLevel)
	    fDeltaLevel = fQuantizationLevel;
	}
	if(fDeltaLevel >= ABS(pShape->parent->value - pShape->value)) {
	  fDeltaLevel = ABS(pShape->parent->value - pShape->value);
	  pShape->removed = (char)1;
	}
	if(! bNoChange)
	  tabScales[i] += fDeltaLevel / fCurvature;
	if(pShape->removed) {
	  if(pShape->parent != pTree->the_shapes)
	    tabScales[pShape->parent - pTree->the_shapes] += tabScales[i];
	  break;
	} else
	  pShape->value += pShape->inferior_type ? fDeltaLevel : -fDeltaLevel;
      }
      if(pShape->removed)
	bDecrease = (char)1;
    }
  } while(bDecrease);
  /* - Bug mw_delete_flists (?): no element in array list must be NULL - */
  for(i = pListBoundaries->size-1; i >= 0; i--)
    if(pListBoundaries->list[i] != NULL)
      mw_delete_flist(pListBoundaries->list[i]);
  pListBoundaries->size = 0;
  /* - End of bug mw_delete_flists - */
  mw_delete_flists(pListBoundaries);
}

void flstb_tv(pScale, pQuantizationLevel, pQuantizationCurve, pTree, pImage)
     float *pScale, *pQuantizationLevel;
     int* pQuantizationCurve;
     Shapes pTree;
     Fimage pImage;
{
  float* tabScales;
  struct fimage originalImage, imageSaddles;
  float** tabtabSaddleValues;

  if(pTree->interpolation != 1)
    mwerror(USAGE, 1, "Please apply to a *bilinear* tree");
  originalImage.nrow = originalImage.ncol = 0;
  imageSaddles.nrow = imageSaddles.ncol = 0;
  originalImage.gray = imageSaddles.gray = NULL;
  originalImage.allocsize = imageSaddles.allocsize = 0;
  flst_reconstruct(pTree, &originalImage);
  fsaddles(&originalImage, &imageSaddles);
  tabtabSaddleValues = mw_newtab_gray_fimage(&imageSaddles);
  if(tabtabSaddleValues == NULL)
    mwerror(FATAL, 1, "Image of saddle values allocation error");

  tabScales = (float*) calloc(pTree->nb_shapes, sizeof(float));
  if(tabScales == NULL)
    mwerror(FATAL, 1, "Allocation of array of scales error");
  tabScales[0] = *pScale;

  decrease_tv(tabScales, *pQuantizationLevel, *pQuantizationCurve,
	      &originalImage, pTree, tabtabSaddleValues);
  
  free(tabScales);
  free(originalImage.gray);
  free(tabtabSaddleValues);
  free(imageSaddles.gray);

  flst_reconstruct(pTree, pImage);
}

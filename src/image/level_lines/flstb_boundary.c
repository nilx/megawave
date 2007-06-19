/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
 name = {flstb_boundary}; 
 version = {"1.0"}; 
 author = {"Pascal Monasse"}; 
 function = {"Discretized level line in bilinear interpolated image"}; 
 usage = { 
  'p': [precision=5] -> pPrecision [1,10000] "Approximate max number of points per dual pixel",
  image->pImage                "The original image",
  tree->pTree                  "A tree of shapes of the image",
  shape->pShape                "The shape whose boundary is to be computed",
  dualchain->pDualchain        "List of dual-pixels (dualchain)",
  boundary<-pBoundary          "boundary computed (output Flist)",
  notused->ctabtabSaddleValues "Saddle values"
}; 
*/ 

#include <assert.h>
#include <float.h>
#include <math.h>
#include "mw.h"

#define EAST 0
#define NORTH 1
#define WEST 2
#define SOUTH 3

#define EPSILON ((float)1.0E-4) /* Constant value related to precision */

#define ABS(x) ((x)>=0 ? (x) : -(x))
/* Value of pixel at absolute coordinate i */
#define VALUE(i) (pImage->gray[(int)(i)])

#ifndef POINT_T
#define POINT_T
typedef struct {
  float x, y;
} point_t;
#endif

/* Return x, (x,value) being on segment joining (0,value1) and (1,value2) */
float offset(value1, value2, value)
float value1, value2, value;
{
  assert((value1 < value2 && value1 <= value && value <= value2) ||
	 (value1 > value2 && value2 <= value && value <= value1));
  return (value-value1) / (value2-value1);
}

/* The code of this function is UGLY. It finds the starting direction of an
open level line */ 
unsigned char first_direction(pPoint, pImage)
point_t* pPoint;
Fimage pImage;
{
  if(pPoint->x == 0)
    return EAST;
  if(pPoint->y == 0)
    return SOUTH;
  if(pPoint->x == pImage->ncol)
    return WEST;
  return NORTH;
}

/* Return the direction followed by the level line crossing the dual
edgel adjacent to the dual pixels of top left corners *'pPrevious' and
*'pPoint' */
unsigned char direction(pPrevious, pPoint)
point_t *pPrevious, *pPoint;
{
  if(pPoint->x > pPrevious->x)
    return EAST;
  if(pPoint->x < pPrevious->x)
    return WEST;
  if(pPoint->y < pPrevious->y)
    return NORTH;
  return SOUTH;
}

/* (*pX, *pY) being the coordinates of (the center of) a dual pixel,
modify them so they are the coordinates of the entry point of the level line
at 'value', knowing it follows direction 'cDirection' */
void entry_point(cDirection, value, pImage, pX, pY)
unsigned char cDirection;
float value;
Fimage pImage;
float *pX, *pY;
{
  int x = (int)(*pX), y = (int)*pY, iWidth = pImage->ncol;
  switch(cDirection) {
  case WEST:
    if(x == iWidth)
      --x;
    else
      *pX += (float)0.5;
    *pY += offset(VALUE((y-1)*iWidth+x), VALUE(y*iWidth+x), value)-(float)0.5;
    break;
  case EAST:
    if(x > 0) {
      -- x;
      *pX -= (float)0.5;
    }
    *pY += offset(VALUE((y-1)*iWidth+x), VALUE(y*iWidth+x), value)-(float)0.5;
    break;
  case NORTH:
    if(y == pImage->nrow)
      -- y;
    else
      *pY += (float)0.5;
    *pX += offset(VALUE(y*iWidth+x-1), VALUE(y*iWidth+x), value)-(float)0.5;
    break;
  case SOUTH:
    if(y > 0) {
      -- y;
      *pY -= (float)0.5;
    }
    *pX += offset(VALUE(y*iWidth+x-1), VALUE(y*iWidth+x), value)-(float)0.5;
    break;
  }
}

/* Discretize the level line inside the dual pixel of top left corner
(xDualPixel, yDualPixel). The entry point in the dual pixel is (x1, y1)
and the exit point (x2, y2). The first point is stored in 'pPoint', the
following ones in the following pointers. Return the number of discretized
points */
int discretize(iPrecision, pImage, pPoint, xDualPixel, yDualPixel,
	       x1, y1, x2, y2)
int iPrecision;
Fimage pImage;
point_t* pPoint;
int xDualPixel, yDualPixel;
float x1, y1, x2, y2;
{
  float alpha, beta, gamma, delta, xMaxCurvature, yMaxCurvature;
  float fStep;
  int iIndex, iNbPoints = 1, n;
  
  pPoint->x = x1;
  pPoint->y = y1;
  
  alpha = x2 - x1;
  beta = y2 - y1;
  if(xDualPixel != 0 && yDualPixel != 0 &&
     xDualPixel != pImage->ncol && yDualPixel != pImage->nrow) {
    iIndex = (yDualPixel-1) * pImage->ncol + xDualPixel-1;
    delta = VALUE(iIndex) + VALUE(iIndex+pImage->ncol+1) -
      (VALUE(iIndex+1) + VALUE(iIndex+pImage->ncol));
  }
  /* Normally, (alpha=0 or beta=0) equivalent to delta=0, but precision... */ 
  if((-EPSILON <= alpha && alpha <= EPSILON) ||
     (-EPSILON <= beta && beta <= EPSILON) ||
     (-EPSILON <= delta && delta <= EPSILON)) /* Straight line */
    if(ABS(alpha) > ABS(beta)) {
      n = (int)ceil(ABS(alpha)*iPrecision)-1;
      if(n != 0) {
	fStep = ABS(alpha)/(n+1);
	gamma = beta/alpha;
	delta = y1 - gamma*x1;
	if(alpha > 0)
	  while(--n >= 0) {
	    x1 += fStep;
	    pPoint[iNbPoints].x = x1;
	    pPoint[iNbPoints++].y = gamma*x1 + delta;
	  }
	else
	  while(--n >= 0) {
	    x1 -= fStep;
	    pPoint[iNbPoints].x = x1;
	    pPoint[iNbPoints++].y = gamma*x1 + delta;
	  }
      }
    } else {
      n = (int)ceil(ABS(beta)*iPrecision)-1;
      if(n != 0) {
	fStep = ABS(beta)/(n+1);
	gamma = alpha/beta;
	delta = x1 - gamma*y1;
	if(beta > 0)
	  while(--n >= 0) {
	    y1 += fStep;
	    pPoint[iNbPoints].x = gamma*y1 + delta;
	    pPoint[iNbPoints++].y = y1;
	  }
	else
	  while(--n >= 0) {
	    y1 -= fStep;
	    pPoint[iNbPoints].x = gamma*y1 + delta;
	    pPoint[iNbPoints++].y = y1;
	  }
      }
    }
  else { /* Not a straight line */
    gamma = (VALUE(iIndex+pImage->ncol)-VALUE(iIndex))/delta -
      xDualPixel+(float)0.5;
    if(x1+gamma + EPSILON >= 0 && x1+gamma <= EPSILON) { /* Saddle */
      n = (int)ceil(ABS(beta)*iPrecision)-1;
      if(n != 0) {
	fStep = ABS(beta)/(n+1);
	if(beta > 0)
	  while(--n >= 0) {
	    y1 += fStep;
	    pPoint[iNbPoints].x = x1;
	    pPoint[iNbPoints++].y = y1;
	  }
	else
	  while(--n >= 0) {
	    y1 -= fStep;
	    pPoint[iNbPoints].x = x1;
	    pPoint[iNbPoints++].y = y1;
	  }
      }
      pPoint[iNbPoints].x = x1; /* Position of saddle point */
      pPoint[iNbPoints++].y = y2;
      n = (int)ceil(ABS(alpha)*iPrecision)-1;
      if(n != 0) {
	fStep = ABS(alpha)/(n+1);
	if(alpha > 0)
	  while(--n >= 0) {
	    x1 += fStep;
	    pPoint[iNbPoints].x = x1;
	    pPoint[iNbPoints++].y = y2;
	  }
	else
	  while(--n >= 0) {
	    x1 -= fStep;
	    pPoint[iNbPoints].x = x1;
	    pPoint[iNbPoints++].y = y2;
	  }
      }
    } else if(x2+gamma + EPSILON >= 0 && x2+gamma <= EPSILON) { /* Saddle */
      n = (int)ceil(ABS(alpha)*iPrecision)-1;
      if(n != 0) {
	fStep = ABS(alpha)/(n+1);
	if(alpha > 0)
	  while(--n >= 0) {
	    x1 += fStep;
	    pPoint[iNbPoints].x = x1;
	    pPoint[iNbPoints++].y = y1;
	  }
	else
	  while(--n >= 0) {
	    x1 -= fStep;
	    pPoint[iNbPoints].x = x1;
	    pPoint[iNbPoints++].y = y1;
	  }
      }
      pPoint[iNbPoints].x = x2; /* Position of saddle point */
      pPoint[iNbPoints++].y = y1;
      n = (int)ceil(ABS(beta)*iPrecision)-1;
      if(n != 0) {
	fStep = ABS(beta)/(n+1);
	if(beta > 0)
	  while(--n >= 0) {
	    y1 += fStep;
	    pPoint[iNbPoints].x = x2;
	    pPoint[iNbPoints++].y = y1;
	  }
	else
	  while(--n >= 0) {
	    y1 -= fStep;
	    pPoint[iNbPoints].x = x2;
	    pPoint[iNbPoints++].y = y1;
	  }
      }
    } else { /* Equation of level line: y = alpha + beta/(x+gamma) */
      beta /= ((float)1.0)/(x2+gamma) - ((float)1.0)/(x1+gamma);
      alpha = y2 - beta/(x2+gamma);
      if(ABS(beta) <= (x1+gamma)*(x1+gamma) &&
	 ABS(beta) <= (x2+gamma)*(x2+gamma)) {
	n = (int)ceil(ABS(x2-x1)*iPrecision)-1;
	if(n != 0) {
	  fStep = ABS(x2-x1)/(n+1);
	  if(x1 < x2)
	    while(--n >= 0) {
	      x1 += fStep;
	      pPoint[iNbPoints].x = x1;
	      pPoint[iNbPoints++].y = alpha + beta / (x1+gamma);
	    }
	  else
	    while(--n >= 0) {
	      x1 -= fStep;
	      pPoint[iNbPoints].x = x1;
	      pPoint[iNbPoints++].y = alpha + beta / (x1+gamma);
	    }
	}
      } else if(ABS(beta) >= (x1+gamma)*(x1+gamma) &&
		ABS(beta) >= (x2+gamma)*(x2+gamma)) {
	n = (int)ceil(ABS(y2-y1)*iPrecision)-1;
	if(n != 0) {
	  fStep = ABS(y2-y1)/(n+1);
	  if(y1 < y2)
	    while(--n >= 0) {
	      y1 += fStep;
	      pPoint[iNbPoints].x = -gamma + beta / (y1-alpha);
	      pPoint[iNbPoints++].y = y1;
	    }
	  else
	    while(--n >= 0) {
	      y1 -= fStep;
	      pPoint[iNbPoints].x = -gamma + beta / (y1-alpha);
	      pPoint[iNbPoints++].y = y1;
	    }
	}
      } else { /* Max curvature point inside */
	if(x1+gamma > 0)
	  xMaxCurvature = (float)sqrt((float)ABS(beta));
	else
	  xMaxCurvature = -(float)sqrt((float)ABS(beta));
	yMaxCurvature = alpha + beta/xMaxCurvature;
	xMaxCurvature -= gamma;
	if((x1 > xMaxCurvature && x1+gamma > 0) ||
	   (x1 < xMaxCurvature && x1+gamma < 0)) {
	  n = (int)ceil(ABS(xMaxCurvature-x1)*iPrecision)-1;
	  if(n != 0) {
	    fStep = ABS(xMaxCurvature-x1)/(n+1);
	    if(x1 < xMaxCurvature)
	      while(--n >= 0) {
		x1 += fStep;
		pPoint[iNbPoints].x = x1;
		pPoint[iNbPoints++].y =  alpha + beta / (x1+gamma);
	      }
	    else
	      while(--n >= 0) {
		x1 -= fStep;
		pPoint[iNbPoints].x = x1;
		pPoint[iNbPoints++].y =  alpha + beta / (x1+gamma);
	      }
	  }
	  pPoint[iNbPoints].x = xMaxCurvature;
	  pPoint[iNbPoints++].y = yMaxCurvature;
	  n = (int)ceil(ABS(y2-yMaxCurvature)*iPrecision)-1;
	  if(n != 0) {
	    fStep = ABS(y2-yMaxCurvature)/(n+1);
	    y1 = yMaxCurvature;
	    if(yMaxCurvature < y2)
	      while(--n >= 0) {
		y1 += fStep;
		pPoint[iNbPoints].x = -gamma + beta / (y1-alpha);
		pPoint[iNbPoints++].y = y1;
	      }
	    else
	      while(--n >= 0) {
		y1 -= fStep;
		pPoint[iNbPoints].x = -gamma + beta / (y1-alpha);
		pPoint[iNbPoints++].y = y1;
	      }
	  }
	} else {
	  n = (int)ceil(ABS(yMaxCurvature-y1)*iPrecision)-1;
	  if(n != 0) {
	    fStep = ABS(yMaxCurvature-y1)/(n+1);
	    if(y1 < yMaxCurvature)
	      while(--n >= 0) {
		y1 += fStep;
		pPoint[iNbPoints].x = -gamma + beta / (y1-alpha);
		pPoint[iNbPoints++].y = y1;
	      }
	    else
	      while(--n >= 0) {
		y1 -= fStep;
		pPoint[iNbPoints].x = -gamma + beta / (y1-alpha);
		pPoint[iNbPoints++].y = y1;
	      }
	  }
	  pPoint[iNbPoints].x = xMaxCurvature;
	  pPoint[iNbPoints++].y = yMaxCurvature;
	  n = (int)ceil(ABS(x2-xMaxCurvature)*iPrecision)-1;
	  if(n != 0) {
	    fStep = ABS(x2-xMaxCurvature)/(n+1);
	    x1 = xMaxCurvature;
	    if(xMaxCurvature < x2)
	      while(--n >= 0) {
		x1 += fStep;
		pPoint[iNbPoints].x = x1;
		pPoint[iNbPoints++].y =  alpha + beta / (x1+gamma);
	      }
	    else
	      while(--n >= 0) {
		x1 -= fStep;
		pPoint[iNbPoints].x = x1;
		pPoint[iNbPoints++].y =  alpha + beta / (x1+gamma);
	      }
	  }
	}
      }
    }
  }
  
  return iNbPoints;
}

/* Fill pBoundary, the level line boundary of pShape */
void extract_line(iPrecision, pImage, pTree, pShape, pDualchain, pBoundary)
int iPrecision;
Fimage pImage;
Shapes pTree;
Shape pShape;
Flist pDualchain,pBoundary;
{
  point_t *pPoint, *pPointCurve;
  float x1, y1, x2, y2;
  unsigned char cDirection1, cDirection2;
  int iSize, i;
  
  iSize = 0;
  pPointCurve = (point_t*) pBoundary->values;
  pPoint = (point_t*) pDualchain->values;
  if(pShape->open)
    cDirection1 = first_direction(pPoint, pImage);
  else
    cDirection1 = direction(pPoint + (pDualchain->size - 1), pPoint);
  
  x1 = pPoint->x; y1 = pPoint->y;
  entry_point(cDirection1, pShape->value, pImage, &x1, &y1);
  for(i = 0; i < pDualchain->size; i++, pPoint++) {
    if(i+1 == pDualchain->size)
      if(pShape->open)
	if(pPoint->x == 0 || pPoint->x == pImage->ncol)
	  x2 = pPoint->x;
	else {
	  assert(pPoint->y == 0 || pPoint->y == pImage->nrow);
	  y2 = pPoint->y;
	}
      else {
	x2 = pDualchain->values[0];
	y2 = pDualchain->values[1];
	cDirection2 = direction(pPoint,         (point_t*)pDualchain->values);
	entry_point(cDirection2, pShape->value, pImage, &x2, &y2);
      }
    else {
      x2 = pPoint[1].x;
      y2 = pPoint[1].y;
      cDirection2 = direction(pPoint, pPoint+1);
      entry_point(cDirection2, pShape->value, pImage, &x2, &y2);
    }
    if(x1 != x2 || y1 != y2)
      iSize += discretize(iPrecision, pImage, pPointCurve+iSize,
			  (int)pPoint->x, (int)pPoint->y, x1, y1, x2, y2);
    cDirection1 = cDirection2; x1 = x2; y1 = y2;
  }
  if(iSize == 0) {
    iSize = 1;
    pPointCurve[0].x = x1;
    pPointCurve[0].y = y1;
  }
  if(pShape->open ||
     (pPointCurve[iSize-1].x != x1 || pPointCurve[iSize-1].y != y1)) {
    pPointCurve[iSize  ].x = x1;
    pPointCurve[iSize++].y = y1;
  }
  pBoundary->size = iSize;
}

/* Extract the discretized level line corresponding to the boundary of the
shape pShape in the tree pTree corresponding to bilinear interpolated image */
Flist flstb_boundary(pPrecision, pImage, pTree, pShape,
		     pDualchain, pBoundary, ctabtabSaddleValues)
int *pPrecision;
Fimage pImage;
Shapes pTree;
Shape pShape;
Flist pDualchain,pBoundary;
char* ctabtabSaddleValues; /* True type: (float**) */
{
  int i, iNbPoints;
  char bBuildDualchain = 0,bBuildBoundary;
  
  if(pTree->interpolation != 1)
    mwerror(USAGE, 1, "Please apply to a *bilinear* tree");
  /* Find the dual pixels where the level line passes */
  if(pDualchain == NULL) {
    bBuildDualchain = (char)1;
    if((pDualchain = mw_new_flist()) == NULL)
      mwerror(FATAL, 1, "Dualchain allocation error");
    flstb_dualchain(pTree, pShape, pDualchain, ctabtabSaddleValues);
  }
  iNbPoints = pDualchain->size * (1+2*(*pPrecision)); /* Upper bound */
  bBuildBoundary = (pBoundary==NULL);
  pBoundary = mw_change_flist(pBoundary, iNbPoints, 0, 2);
  if(pBoundary == NULL)
    mwerror(FATAL, 1, "Points allocation error");
  
  extract_line(*pPrecision, pImage, pTree, pShape, pDualchain, pBoundary);
  assert(pBoundary->size < iNbPoints); /* Otherwise, memory error happened */
  
  /* free over-allocated bytes if pBoundary was created in this module) */
  if (bBuildBoundary) mw_realloc_flist(pBoundary, pBoundary->size);
  
  if(bBuildDualchain)
    mw_delete_flist(pDualchain);
  
  return(pBoundary);
}

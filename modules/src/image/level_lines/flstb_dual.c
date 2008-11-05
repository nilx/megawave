/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
 name = {flstb_dual};
 version = {"1.1"}; 
 author = {"Pascal Monasse"}; 
 function = {"Level lines passing through centers of dual pixels in bilinear image"};
 usage = { 
    bilinear_tree->pTree  "The bilinear tree of the image",
    dual_tree<-pDualTree  "The new tree"
}; 
*/ 
/*----------------------------------------------------------------------
 v1.1: minor bug correction (P.Monasse)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "mw.h"
#include "mw-modules.h" /* for flst_pixels() */


/* Is shape 1 inside shape 2? */
static char is_included(pShape1, pShape2)
Shape pShape1, pShape2;
{
  return (pShape1->area <= pShape2->area &&
	  pShape1->pixels >= pShape2->pixels &&
	  pShape1->pixels < pShape2->pixels+pShape2->area);
}

/* Clear, no? */
static Shape common_ancestor(pShape1, pShape2)
Shape pShape1, pShape2;
{
  while(! is_included(pShape1, pShape2))
    pShape2 = pShape2->parent;
  return pShape2;
}

/* Build a new shape, of parent pParent, variation of pShapeTemplate */
static void copy_shape(pShapeTemplate, pShape)
Shape pShapeTemplate, pShape;
{
  *pShape = *pShapeTemplate;
  pShape->removed = 0;
  pShape->boundary = 0;
  pShape->pixels = NULL;
  pShape->data = NULL;
  pShape->data_size = 0;
}

/* `shapeInf' is a shape in original tree of same type as shape to
be created and contained in it. If new shape must be `shapeInf', create
it in new tree as parent of `shapeInf'. */
static void insert_shape(pDualTree, shapeInf, v)
Shapes pDualTree;
Shape shapeInf;
float v;
{
  Shape parent, newShape, previousSibling;
  float v0, deltav, d;

  if(shapeInf->parent == &pDualTree->the_shapes[0])
    return;

  /* Find existing shape shapeInf immediately contained in new shape */
  v0 = shapeInf->value;
  deltav = v - v0;
  if(deltav < 0)
    deltav = -deltav;
  parent = shapeInf->parent;
  while(1) {
    d = parent->value - v0;
    if(d < 0)
      d = -d;
    if(d == deltav && ! parent->removed) /* Shape already built */
      return;
    if(d > deltav)
      break;
    shapeInf = parent;
    parent = shapeInf->parent;
  }

  /* Insert new shape at adequate position */
  newShape = &pDualTree->the_shapes[pDualTree->nb_shapes++];
  copy_shape(shapeInf, newShape);
  newShape->value = v;
  newShape->child = shapeInf;
  shapeInf->parent = newShape;
  newShape->parent = parent;
  newShape->next_sibling = shapeInf->next_sibling;
  shapeInf->next_sibling = NULL;
  if(parent->child == shapeInf)
    parent->child = newShape;
  else {
    previousSibling = parent->child;
    while(previousSibling->next_sibling != shapeInf)
      previousSibling = previousSibling->next_sibling;
    previousSibling->next_sibling = newShape;
  }
}

/* Find shape passing through corner of dual pixel (i,j) */
static void find_shape(pTree, pDualTree, i, j)
Shapes pTree, pDualTree;
int i, j;
{
  Shape tabShapes[4], shapeInf, shapeTemp, shapeSaddle;
  int k, n;
  float v, x;

  /* Find adjacent centers of pixels */
  tabShapes[0] = (i == 0 || j == 0) ? NULL :
    pTree->smallest_shape[(i-1)*pTree->ncol+j-1];
  tabShapes[1] = (i == 0 || j == pTree->ncol) ? NULL :
    pTree->smallest_shape[(i-1)*pTree->ncol+j];
  tabShapes[2] = (i == pTree->nrow || j == pTree->ncol) ? NULL :
    pTree->smallest_shape[i*pTree->ncol+j];
  tabShapes[3] = (i == pTree->nrow || j == 0) ? NULL :
    pTree->smallest_shape[i*pTree->ncol+j-1];

  /* Compute value at center of dual pixel */
  n = 0; v = 0;
  for(k = 0; k < 4; k++)
    if(tabShapes[k]) {
      ++ n;
      v += tabShapes[k]->value;
    }
  v /= n;

  shapeInf = NULL;
  for(k = 0; k < 4; k++) { /* Examine the edgels */
    n = (k+1) % 4;
    if(tabShapes[k] == NULL || tabShapes[n] == NULL ||
       (tabShapes[k]->value < v && tabShapes[n]->value < v) ||
       (tabShapes[k]->value > v && tabShapes[n]->value > v))
      continue;
    if(tabShapes[k]->value == v || is_included(tabShapes[k], tabShapes[n]))
      shapeTemp = tabShapes[k];
    else if(tabShapes[n]->value == v || is_included(tabShapes[n], tabShapes[k]))
      shapeTemp = tabShapes[n];
    else {
      shapeSaddle = common_ancestor(tabShapes[n], tabShapes[k]);
      if(shapeSaddle->value == v)
	shapeTemp = shapeSaddle;
      else if(tabShapes[k]->value < shapeSaddle->value)
	shapeTemp = (v < shapeSaddle->value) ? tabShapes[k] : tabShapes[n];
      else
	shapeTemp = (v < shapeSaddle->value) ? tabShapes[n] : tabShapes[k];
    }
    if(shapeInf == NULL || is_included(shapeInf, shapeTemp))
      shapeInf = shapeTemp;
    else if(! is_included(shapeTemp, shapeInf)) { /* Ambiguity due to saddle point */
      shapeSaddle = common_ancestor(shapeInf, shapeTemp);
      if(shapeSaddle->value == v)
	shapeInf = shapeSaddle;
      else {
        x = shapeSaddle->value-shapeTemp->value;
        if(shapeTemp == tabShapes[k])
          x /= tabShapes[k-1]->value - shapeTemp->value;
        else
          x /= tabShapes[(n+1)%4]->value - shapeTemp->value;
	if(x > 0.5f)
	  shapeInf = shapeTemp;
      }
    }
    ++ k; /* Go to opposite edgel */
  }
  if(shapeInf == NULL) /* One of the 4 corners of image */
    for(i = 0; i < 4; i++)
      if(tabShapes[i] != NULL)
	shapeInf = tabShapes[i];
  insert_shape(pDualTree, shapeInf, v);
}


/*------------------------------ MAIN ------------------------------*/

void flstb_dual(pTree, pDualTree)
Shapes pTree, pDualTree;
{
  int i, j;
  if(pTree->interpolation != 1)
    mwerror(USAGE, 1, "Please apply this module to a *bilinear* tree");

  pDualTree->nrow = pTree->nrow; pDualTree->ncol = pTree->ncol;
  pDualTree->interpolation = 1;
  pDualTree->nb_shapes = 1;
  pDualTree->the_shapes = (Shape)
    malloc(((pTree->ncol+1)*(pTree->nrow+1)+1)*sizeof(struct shape));
  if(pDualTree->the_shapes == NULL)
    mwerror(FATAL, 1, "Error allocating shapes");
  pDualTree->the_shapes[0] = pTree->the_shapes[0];
  pDualTree->the_shapes[0].child = NULL;
  pDualTree->the_shapes[0].boundary = NULL;
  pDualTree->smallest_shape = (Shape*)
    malloc(pTree->nrow*pTree->ncol*sizeof(Shape));
  if(pDualTree->smallest_shape == NULL)
    mwerror(FATAL, 1, "Error allocating array");
  memcpy(pDualTree->smallest_shape, pTree->smallest_shape,
	 pTree->nrow*pTree->ncol*sizeof(Shape));

  if(pTree->the_shapes[0].pixels == NULL)
    flst_pixels(pTree);
  pDualTree->the_shapes[0].pixels = NULL;

  /* Remove shapes of original tree. Allows discrimination between
  new and old shapes */
  for(i = pTree->nb_shapes-1; i >= 0; i--)
    pTree->the_shapes[i].removed = (char)1;
  pTree->the_shapes[0].parent = &pDualTree->the_shapes[0];
  pDualTree->the_shapes[0].child = &pTree->the_shapes[0];

  /* Scan corners of pixels */
  for(i = 0; i <= pTree->nrow; i++)
    for(j = 0; j <= pTree->ncol; j++)
      find_shape(pTree, pDualTree, i, j);
}

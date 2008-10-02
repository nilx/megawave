/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
  name = {ll_sharp}; 
  version = {"1.5"}; 
  author = {"Pascal Monasse, Frederic Guichard"}; 
  function = {"Sharpen an image (select some of the parallel level lines)"}; 
  usage = { 
    'p': [percent_area=20]-> pPercentIncreaseArea [1,1000] 
                                    "% area change to glue parent and child",
    image_in -> pFloatImageInput    "Input fimage", 
    image_out <- pFloatImageOutput  "Output fimage" 
    }; 
*/ 
/*----------------------------------------------------------------------
 v1.2: Flist adaptation in remove_gradation (L.Moisan)
 v1.3: upgrade for new kernel (L.Moisan)
 v1.4: fixed size problem (L.Moisan)
 v1.5 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include "mw.h" 

extern void flst();
extern void flst_reconstruct();

 
/* An elementary shape is a shape having a child of a different type. To keep consistency 
   between the tree and the associated image, it is forbidden to remove such a shape. This 
   function detects if pShape is an elementary shape, and if the answer is positive, mark 
   the shape to be its own gradation. */ 
void keep_elementary_shape(pShape) 
     Shape pShape; 
{ 
  Shape pChild; 
   
  /* If one of the sons is of different type, the object has only one shape */ 
  for(pChild = mw_get_first_child_shape(pShape); pChild != NULL; pChild = mw_get_next_sibling_shape(pChild)) 
    if(pChild->inferior_type != pShape->inferior_type) 
      break; 
 
  if(pChild != NULL) 
    pShape->data = (char*)1; 
} 
 
/* Find the gradation containg pShape, remove it but keep only one representative shape of 
   the gradation. 
   A gradation is defined to be a family of shapes such that: 
   One shape S and its child C are in the same gradation iff: 
   - C is the only child of S; 
   - C and S have the same type; 
   - the areas of C and S do not differ by a factor greater than fFactorIncreaseArea; 
   - C is not an elementary shape. 
   It is not hard to see that this relation induces a partition of the family of shapes, each 
   class of the partition is then called a gradation. 
   The representative shape of the gradation is the shape of the gradation with smallest L²/A, 
   where A is the area and L is the length of its boundary */ 
void remove_gradation(pShape, fFactorIncreaseArea) 
     Shape pShape; 
     float fFactorIncreaseArea; 
{ 
  Shape pCurrentShape, pParent, pChild, pSibling, pRepresentativeShape; 
  float fCriterium, fOptimalCriterium, fMeanGrayLevel; 
 
  pCurrentShape = pRepresentativeShape = pShape; 
   
  /* 1) Go down the tree until the smallest shape in the gradation */ 
  pChild = mw_get_first_child_shape(pCurrentShape); 
  while(pChild != NULL && pChild->data == NULL && 
	pChild->inferior_type == pCurrentShape->inferior_type && 
	pChild->area * fFactorIncreaseArea >= pCurrentShape->area && 
	mw_get_next_sibling_shape(pChild) == NULL) 
    { 
      pCurrentShape = pChild; 
      pChild = mw_get_first_child_shape(pChild); 
    } 
  pChild = pCurrentShape; 
 
  /* 2) Go up the tree until the largest shape in the gradation */ 
  pParent = mw_get_parent_shape(pShape); 
  while(pParent != NULL && 
	pParent->inferior_type == pCurrentShape->inferior_type && 
	pParent->area <= pCurrentShape->area * fFactorIncreaseArea && 
	mw_get_first_child_shape(pParent) == pCurrentShape && 
	mw_get_next_sibling_shape(pCurrentShape) == NULL) 
    { 
      pCurrentShape = pParent; 
      pParent = mw_get_parent_shape(pParent); 
    } 
   
  /* 3) Remove the gradation, keeping only the representative shape */ 
  fMeanGrayLevel = pChild->area * pChild->value; 
  fOptimalCriterium = (float)(pChild->boundary?pChild->boundary->size:0); 
  fOptimalCriterium *= fOptimalCriterium; 
  fOptimalCriterium /= (float)pChild->area; 
  pChild->data = (void*)1; 
  pChild->removed = (char)1; 
  pCurrentShape = mw_get_parent_shape(pChild); 
  while(pCurrentShape != pParent) 
    { 
      fMeanGrayLevel += (pCurrentShape->area - pChild->area) * pCurrentShape->value; 
      fCriterium = (float)(pCurrentShape->boundary?pCurrentShape->boundary->size:0);
      fCriterium *= fCriterium; 
      fCriterium /= (float)pCurrentShape->area; 
      if(fCriterium < fOptimalCriterium) 
	  { 
	    fOptimalCriterium = fCriterium; 
	    pRepresentativeShape = pCurrentShape; 
	  } 
      pChild = pCurrentShape; 
      pCurrentShape = mw_get_parent_shape(pCurrentShape); 
      pChild->removed = (char)1; 
      pChild->data = (void*)1; 
    } 
  pRepresentativeShape->removed = 0; 
  pRepresentativeShape->value = fMeanGrayLevel / pChild->area; 
} 
 
/* This removes the gradations from an image. In the tree of shapes representing the input 
   image, the field data of each shape is used to mark the shapes whose gradation was 
   already found */ 
void ll_sharp(pPercentIncreaseArea, pFloatImageInput, pFloatImageOutput) 
     float *pPercentIncreaseArea; 
     Fimage pFloatImageInput, pFloatImageOutput; 
{ 
  int i; 
  Shapes pTree; 
  float fFactorIncreaseArea = (float)(1.0 + 0.01 * *pPercentIncreaseArea); 
 
  if(mw_change_fimage(pFloatImageOutput, pFloatImageInput->nrow, pFloatImageInput->ncol) == NULL) 
    mwerror(FATAL, 1, "fkill_grains --> Not enough memory to allocate the output image"); 
  if((pTree = mw_new_shapes()) == NULL) 
    mwerror(FATAL, 1, "fkill_grains --> Not enough memory to allocate the tree of shapes"); 
  /* Compute the Level Sets Transform of the input image, and store the associated level lines */ 
  flst(NULL, pFloatImageInput, pTree, (char*)1, NULL); 
 
  /* First find the shapes whose containing gradation is reduced to the shape itself */ 
  for(i = pTree->nb_shapes-1; i > 0; i--) 
    keep_elementary_shape(&pTree->the_shapes[i]); 
 
  /* Kill gradations. 
     We use the field data of a shape to track the shapes which were already dealt with */ 
  for(i = pTree->nb_shapes-1; i > 0; i--) 
    if(pTree->the_shapes[i].data == NULL) 
      remove_gradation(&pTree->the_shapes[i], fFactorIncreaseArea); 
 
  /* Reconstruct in pFloatImageOutput the modified tree of shapes */ 
  flst_reconstruct(pTree, pFloatImageOutput); 
 
  mw_delete_shapes(pTree); 
} 

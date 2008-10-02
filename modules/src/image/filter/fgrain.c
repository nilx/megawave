/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
  name = {fgrain}; 
  version = {"1.2"}; 
  author = {"Pascal Monasse, Frederic Guichard"}; 
  function = {"Grain filter of an image"}; 
  usage = { 
    'a': [min_area=20]-> pMinArea   "Min area of grains we keep", 
    image_in -> pFloatImageInput    "Input fimage", 
    image_out <- pFloatImageOutput  "Output fimage" 
    }; 
*/ 
/*----------------------------------------------------------------------
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include "mw.h" 

extern void flst();
extern void flst_reconstruct();
 
/* This removes the shapes from the tree associated to pFloatImageInput 
that are too small (threshold *pMinArea). As a consequence all the remaining 
shapes of pFloatImageOutput are of area larger or equal than *pMinArea */ 

void fgrain(pMinArea, pFloatImageInput, pFloatImageOutput) 
     int *pMinArea; 
     Fimage pFloatImageInput, pFloatImageOutput; 
{ 
  int i; 
  Shapes pTree; 
 
  if(mw_change_fimage(pFloatImageOutput, pFloatImageInput->nrow, 
		      pFloatImageInput->ncol) == NULL) 
    mwerror(FATAL, 1, 
	    "fgrain --> Not enough memory to allocate the output image"); 
  if((pTree = mw_new_shapes()) == NULL) 
    mwerror(FATAL, 1, 
	    "fgrain --> Not enough memory to allocate the tree of shapes"); 

  /* Compute the Level Sets Transform of the input image */ 
  flst(NULL, pFloatImageInput, pTree, NULL, NULL); 
 
  /* Kill too small grains. 
     Bound i>0 because it is forbidden to delete the root, at index 0 */ 
  for(i = pTree->nb_shapes-1; i > 0; i--) 
    if(pTree->the_shapes[i].area < *pMinArea) 
      pTree->the_shapes[i].removed = (char)1; 
 
  /* Reconstruct in pFloatImageOutput the modified tree of shapes */ 
  flst_reconstruct(pTree, pFloatImageOutput); 
 
  mw_delete_shapes(pTree); 
} 

/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
 name = {ll_extract}; 
 version = {"1.3"}; 
 author = {"Lionel Moisan"}; 
 function = {"Compute the level lines of a Fimage"};
 usage = { 
   'l':levels->levels  "levels of quantization (Fsignal) (bilinear)",
   'o':offset->offset  "quantization offset (bilinear, default 0.5)",
   's':step->step      "quantization step (bilinear, default 1.0)",
   'p':[prec=5]->prec  "precision (# pts per unit length, bilinear case only)",
   'a':area->area      "minimum area (parameter for the grain filter)",
   't':tree->tree      "to use a previoulsy computed tree",
   'z'->z              "use zero order instead of bilinear interpolation",
   in->in              "input Fimage",
   out<-ll_extract     "computed level lines (output Flists)"
}; 
*/ 
/*----------------------------------------------------------------------
 v1.1: added missing area parameter (P.Monasse)
 v1.2: fixed max_size bug due to change in list.h (L.Moisan)
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include "mw.h"

extern void flst_bilinear();
extern void flstb_quantize();
extern Flist flstb_boundary();
extern int fsaddles();


Flists ll_extract(in,levels,offset,step,prec,area,tree,z)
     Fimage in;
     Fsignal levels;
     float *offset,*step;
     int *area,*prec;
     Shapes tree;
     char *z;
{
  Flists ls;
  Flist boundary;
  Shapes ref_tree;
  Shape s;
  Fimage saddles,copy_in;
  int newtree=0,i;
  float **tabsaddles;

  /* check consistency */
  if (tree) {
    if (z && tree->interpolation!=0)
      mwerror(FATAL,1,"Please use a zero order tree with -z option\n");
    if (!z && tree->interpolation!=1)
      mwerror(FATAL,1,"Please use a bilinear tree without -z option\n");
  }
  if (z && (step || levels))
    mwerror(WARNING,0,"-s and -l options have no effect with -z option\n");

  /* compute FLST if needed */
  if (!tree) {
    newtree = 1;
    tree = mw_new_shapes();
    copy_in = mw_change_fimage(NULL,in->nrow,in->ncol);
    if (!tree || !copy_in) mwerror(FATAL,1,"Not enough memory");
    mw_copy_fimage(in,copy_in);
    if (z) 
      flst(area,copy_in,tree);
    else
      flst_bilinear(area,copy_in,tree);
    mw_delete_fimage(copy_in);
  } 

  if (!z) {
    /* bilinear case : compute saddle points and quantize FLST tree */
    ref_tree = mw_new_shapes();
    saddles = mw_new_fimage();
    if (!ref_tree || !saddles) mwerror(FATAL,1,"Not enough memory");
    fsaddles(in,saddles);
    tabsaddles = mw_newtab_gray_fimage(saddles);
    if (!tabsaddles) mwerror(FATAL,1,"Not enough memory");
    flstb_quantize(levels,offset,step,tree,ref_tree);
    if (newtree) mw_delete_shapes(tree);
  } else ref_tree = tree;
  mwdebug("Total number of shapes: %d\n",ref_tree->nb_shapes);

  /* allocate memory */
  boundary = mw_change_flist(NULL,2,0,2);
  ls = mw_change_flists(NULL,ref_tree->nb_shapes-1,0);
  if (!ls) mwerror(FATAL,1,"Not enough memory\n");
  
  /* MAIN LOOP : extract boundaries */
  for (i=0,ls->size=0;i<ref_tree->nb_shapes;i++) {
    s = &ref_tree->the_shapes[i];
    if (s->parent) {
      if (z)
      	flst_boundary(ref_tree,s,boundary);
      else 
	flstb_boundary(prec,in,ref_tree,s,NULL,boundary,tabsaddles);
      ls->list[ls->size++] = mw_copy_flist(boundary,NULL);
    }
  }

  /* free memory and exit */
  mw_delete_flist(boundary);
  if (!z) {
    free(tabsaddles);
    mw_delete_fimage(saddles);
    mw_delete_shapes(ref_tree);
  }
  mwdebug("%d boundaries written (alloc %d)\n",ls->size,ls->max_size);

  return(ls);
}


/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {wp2dchangetree};
version = {"1.0"};
author = {"Francois Malgouyres"};
function = {"Modify a 2D-wavelet packet quad-tree"};
usage = { 
 'e':size->new_tree_size
     "The ouput Cimage describes the same tree but has the prescribed size (when possible).",
 'p'->prune_tree  
     "The output Cimage describes the same tree but is as small as possible.",
 'u'->up_tree  
     "The ouput tree decomposes the less decomposed leaves of the input tree.",
 'd'->down_tree  
     "Every leaf of the input tree that can be cut is",
 'M':Cimage->tree_for_max
     "The ouput tree asks for a decomposition if the Cimage OR the input tree ask for it",
 'm':Cimage->tree_for_min
     "The ouput tree asks for a decomposition if the Cimage AND the input tree ask for it",
 tree_in->treeIn     "Cimage describing the input tree",
 tree_out<-treeOut   "Cimage describing the output tree"
};
*/
/*----------------------------------------------------------------------
 v1.0: adaptation from changeTree v0.1 (fpack) (JF)
----------------------------------------------------------------------*/


#include "mw.h"
#include "mw-modules.h" /* for wp2dchecktree() */

#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)])

/***************************************************************/
void  test_input(treeIn,up_tree,down_tree,new_tree_size,prune_tree,tree_for_max,tree_for_min)

     Cimage treeIn,tree_for_max,tree_for_min;
     int *new_tree_size;
     char *prune_tree,*up_tree,*down_tree;
    
{int nbOption;
 
/* check input trees */ 
 wp2dchecktree(treeIn,NULL);

 if(tree_for_max)
   wp2dchecktree(tree_for_max,NULL);

 if(tree_for_min)
   wp2dchecktree(tree_for_min,NULL);

 /*checks compatibility of options*/
 nbOption=0;
 if(new_tree_size) nbOption++;
 if(prune_tree) nbOption++;
 if(up_tree) nbOption++;
 if(down_tree) nbOption++;
 if(tree_for_max) nbOption++;
 if(tree_for_min) nbOption++;

 if(nbOption != 1)
   mwerror(FATAL, 1, "[wp2dchangetree]One and only one option is needed !\n");
}
/***************************************************************/
/*returns the maximum level of the decomposition corresponding to tree */

int treeLevel(tree)
     
     Cimage tree;
     
{int i;
 int size=  tree->ncol*tree->nrow;
 int level;
 
 level=tree->gray[0];
 for(i=1;i<size;i++)
   if(level < tree->gray[i]) 
     level = tree->gray[i];
 
 return(level);
}
/***************************************************************/
/*returns the minimum level of the decomposition corresponding to tree */

int treeMin(tree)
     
     Cimage tree;
     
{int size=tree->ncol*tree->nrow;
 int i;
 int min;
 
 min=tree->gray[0];
 for(i=1;i<size;i++)
   if(tree->gray[i]<min)
     min=tree->gray[i];
 
 return(min);
}
/***************************************************************/
/*  Prunes the content of 'tree_in' sothat it provides            */
/* the same decomposition                                                            */
/* but has the smallest possible size                                             */
/* the result is in 'tree_out'                                                            */
void pruneTree(tree_in,tree_out)

     Cimage tree_in,tree_out;

{int level;                                     /* level of the tree */
 int prunedTreeSize;                 /* size of tree_out */
 int i;                                             /* temporary value */
 int kx,ky;                                    /*indexes for tree_in*/
 int kx1,ky1;                               /*indexes for tree_out*/
 int extra;                                    /*ratio between the size of tree_in and the size of tree_out*/
 
 /*determines the size of the pruned tree */
 level=treeLevel(tree_in);
 
 prunedTreeSize=1;
 for(i=0;i<level;i++)
   prunedTreeSize*=2;

 /* allocates memory */

 tree_out=mw_change_cimage(tree_out,prunedTreeSize,prunedTreeSize);
 if  (tree_out== NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 /*prunes the tree */
 
 extra=tree_in->ncol/tree_out->ncol;
 
 for(kx=0,kx1=0;kx<tree_in->ncol;kx+=extra,kx1++)
   for(ky=0,ky1=0;ky<tree_in->nrow;ky+=extra,ky1++)
     _(tree_out,kx1,ky1)=_(tree_in,kx,ky);

}

/***************************************************************/
/*  Extends the content of 'tree_in' sothat it provides            */
/* the same decomposition                                                            */
/* but has the size of 'tree_out'                                                     */
/* the result is in 'tree_out'                                                            */

void extendTree(tree_in,tree_out)
     
     Cimage tree_in,tree_out;
     
{int kx,ky;                                 /*indexes for tree_in*/
 int kx1,ky1;                             /*indexes for tree_out*/
 int extra;                                 /*ratio between the size of tree_out and the size of tree_in*/
 int x,y;                                      /*to fill in extra terms*/
 int value;                                 /* temporary value */
 
 if(tree_in->ncol==0)
   mwerror(FATAL, 1, "Tree should not be of size 0!\n");
 
 if(tree_out->ncol <tree_in->ncol || tree_out->nrow<tree_in->nrow)
   mwerror(FATAL, 1, "The output tree Cimage is too small to contain the tree !\n");
 
 extra=tree_out->ncol/tree_in->ncol;
 
 for(kx=0,kx1=0;kx<tree_in->ncol;kx++,kx1+=extra)
   for(ky=0,ky1=0;ky<tree_in->nrow;ky++,ky1+=extra)
     {value=_(tree_in,kx,ky);
     
     for(x=0;x<extra;x++) for(y=0;y<extra;y++)
       _(tree_out,kx1+x,ky1+y)=value;
     }
 
}

/***************************************************************/
/* provides 'tree_out1' and  'tree_out2',                                                 */
/* which both have the same size  (the smallest possible)                 */
/* 'tree_out1' describes the same decomposition as    'tree_in1'      */
/* 'tree_out2' describes the same decomposition as    'tree_in2'      */

void   resize_trees(tree_in1,tree_in2,tree_out1,tree_out2)

     Cimage tree_in1,tree_in2,tree_out1,tree_out2;

{int finalSize;
 Cimage pruned1;
 Cimage pruned2;
 
 /*prune tree_in1 and tree_in2 before processing*/
 pruned1 = mw_new_cimage();   
 pruneTree(tree_in1,pruned1);
  
 pruned2 = mw_new_cimage();   
 pruneTree(tree_in2,pruned2);

 /*computes final size */
 
 finalSize= (pruned1->ncol > pruned2->ncol) ? pruned1->ncol : pruned2->ncol ;

 /* allocates memory for outputs */

 tree_out1= mw_change_cimage(tree_out1,finalSize,finalSize);
 if  (tree_out1 == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 tree_out2= mw_change_cimage(tree_out2,finalSize,finalSize);
 if  (tree_out2 == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 /*computes outputs */
 /*(notice that if input and output of 'extendTree' have same size*/
 /* the input is just copied into output )                                               */

 extendTree(pruned2, tree_out2);
 extendTree(pruned1, tree_out1);
 
 /* free useless memory */
 mw_delete_cimage(pruned1),pruned1=NULL;
 mw_delete_cimage(pruned2),pruned2=NULL;
 
}

/***************************************************************/
/* Computes the maximum of two Cimage                                            */
/* the three images must have the same size                                         */

void max_tree(tree_in1, tree_in2, tree_out)

     Cimage tree_in1, tree_in2, tree_out;

{int size=tree_in1->ncol*tree_in1->nrow;
 int i;

 for(i=0;i<size;i++)
   tree_out->gray[i] = ( tree_in1->gray[i] > tree_in2->gray[i] ) ?  tree_in1->gray[i] : tree_in2->gray[i];

}
 
/***************************************************************/
/* Computes the minimum of two Cimage                                            */
/* the three images must have the same size                                         */

void min_tree(tree_in1, tree_in2, tree_out)

     Cimage tree_in1, tree_in2, tree_out;

{int size=tree_in1->ncol*tree_in1->nrow;
 int i;

 for(i=0;i<size;i++)
   tree_out->gray[i] = ( tree_in1->gray[i] < tree_in2->gray[i] ) ?  tree_in1->gray[i] : tree_in2->gray[i];

}
 
/***************************************************************/
/* adds 1 at the smallest elements of 'tree'           */

void add_one_at_tree(tree)
     
     Cimage tree;
     
{int size=tree->ncol*tree->nrow;
 int i;
 int min;

 min=treeMin(tree);

 for(i=0;i<size;i++)
   if(tree->gray[i]==min)
     tree->gray[i]+=1;
 
}

/***************************************************************/
/* substract 1 at 'tree', when possible           */

void substract_one_at_tree(tree)
     
     Cimage tree;
     
{int former_level;
 int levelMax;
 int jump_2;
 int kx,ky;
 int x,y;
 int canSubstract;

 levelMax=treeLevel(tree);
 
 for(former_level=1;former_level<=levelMax;former_level++)
   { 
     /*jump_2 = tree->ncol/power(2,former_level-1);*/
     jump_2 = tree->ncol >> (former_level-1);
   
     for(kx = 0 ; kx<tree->ncol; kx=kx+jump_2)
       for(ky = 0 ; ky<tree->nrow ; ky=ky+jump_2)
	 {
	   canSubstract=1;
       
	   for(x=0;x<jump_2;x++) for(y=0;y<jump_2;y++)
	     if(_(tree,kx+x,ky+y) != former_level)
	       canSubstract=0;
	   
	   if(canSubstract)
	     for(x=0;x<jump_2;x++) for(y=0;y<jump_2;y++) 
	       _(tree,kx+x,ky+y)+=-1;
	 }
   }
}

/***************************************************************/
/*-------------------------------------------------------------*/
/*--------------MAIN PROGRAM-----------------------*/
/*-------------------------------------------------------------*/
/******** Modifies a tree ***********/
/* TreeIn can equal treeOut */
void wp2dchangetree(treeIn,treeOut,up_tree,down_tree,new_tree_size,prune_tree,tree_for_max,tree_for_min)
     
     Cimage treeIn,treeOut,tree_for_max,tree_for_min;
     int *new_tree_size;
     char *prune_tree,*up_tree,*down_tree;
     
{Cimage treeTmp;
 Cimage treeInPruned=NULL;
 Cimage otherTreeInPruned=NULL;
 int treeSize;
 int min,max;
 
 /* test entries */

 test_input(treeIn,up_tree,down_tree,new_tree_size,prune_tree,tree_for_max,tree_for_min);
 
 /* MAIN PROCESS */
 
 treeTmp = mw_new_cimage();
 
 /* the output tree decomposes the less decomposed leaves of input tree */
 if(up_tree)
   {treeInPruned = mw_new_cimage();   
   pruneTree(treeIn,treeInPruned);/*prune treeIn before processing*/
  
   min = treeMin(treeInPruned);
   max= treeLevel(treeInPruned);
   
   if(min==max) /* the tree needs to be extended first */
     {treeSize=2*treeInPruned->ncol;
     
     treeTmp = mw_change_cimage(treeTmp ,treeSize,treeSize);
     if  (treeTmp == NULL)
       mwerror(FATAL, 1, "Not enough memory !\n");
     
     extendTree(treeInPruned,treeTmp);
     
     add_one_at_tree(treeTmp);
     }
   else
     {add_one_at_tree(treeInPruned);
     
     treeTmp = mw_change_cimage(treeTmp ,treeInPruned->nrow,treeInPruned->ncol);
     if  (treeTmp == NULL)
       mwerror(FATAL, 1, "Not enough memory !\n");
     
     mw_copy_cimage(treeInPruned,treeTmp);
     }
   }
 
 /* the output tree does not decompose the more decomposed leaves of input tree */
 else if(down_tree)
   {otherTreeInPruned  = mw_change_cimage(NULL,treeIn->nrow,treeIn->ncol);
   if  (otherTreeInPruned == NULL)
     mwerror(FATAL, 1, "Not enough memory !\n");
   
   mw_copy_cimage(treeIn,otherTreeInPruned);
   substract_one_at_tree(otherTreeInPruned);
   
   pruneTree(otherTreeInPruned,treeTmp);
   }
 
 /* extending the size of tree*/
 else if(new_tree_size)
   {treeInPruned = mw_new_cimage();   
   
   treeTmp = mw_change_cimage(treeTmp,*new_tree_size,*new_tree_size);
   if  (treeTmp == NULL)
     mwerror(FATAL, 1, "Not enough memory !\n");
   
   pruneTree(treeIn,treeInPruned);/*prune treeIn before extending it*/
   
   extendTree(treeInPruned,treeTmp);
   }
 /* pruning tree*/
 
 else if(prune_tree)
   {treeInPruned = mw_new_cimage();
   pruneTree(treeIn,treeTmp);
   }
 
 /* Computes a tree which asks for a decomposition if treeIn OR tree_for_max asks for it */
 
 else if(tree_for_max)
   {treeInPruned = mw_new_cimage();
   otherTreeInPruned = mw_new_cimage();
   
   resize_trees(treeIn,tree_for_max,treeInPruned,otherTreeInPruned);
   
   treeTmp = mw_change_cimage(treeTmp,treeInPruned->nrow,treeInPruned->ncol);
   if  (treeTmp == NULL)
     mwerror(FATAL, 1, "Not enough memory !\n");

   max_tree(treeInPruned,otherTreeInPruned,treeTmp);  
   }

 /* Computes a tree which asks for a decomposition if treeIn OR tree_for_max asks for it */
 
 else if(tree_for_min)
   {treeInPruned = mw_new_cimage();
   otherTreeInPruned = mw_new_cimage();
   treeTmp =mw_new_cimage();

   resize_trees(treeIn,tree_for_min,treeInPruned,otherTreeInPruned);
   
   min_tree(treeInPruned,otherTreeInPruned,treeInPruned); /* treeInPruned is used for the output for simplicity */
   
   pruneTree(treeInPruned,treeTmp);
   }
 
 
 /* End of : MAIN PROCESS*/

 /*checks result is a tree (this should be OK, if 'wp2dchangetree' is correct)*/
 wp2dchecktree(treeTmp,NULL);
 
 /* the result is copied in treeOut */
 treeOut=mw_change_cimage(treeOut,treeTmp->nrow,treeTmp->ncol);
 if  (treeOut == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");

 mw_copy_cimage(treeTmp,treeOut);

 /* free useless memory */
 
 mw_delete_cimage(treeTmp);treeTmp=NULL;
 
 if(treeInPruned)
   {mw_delete_cimage(treeInPruned);
   treeInPruned=NULL;
   }
 if(otherTreeInPruned)
   {mw_delete_cimage(otherTreeInPruned);
   otherTreeInPruned=NULL;
   }
}













































































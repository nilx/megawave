/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   wpack2d.c
   
   Vers. 0.0
   Authors : Adrien Costagliola, David Serre, Francois Malgouyres, Jacques Froment
   Basic memory routines for the wpack2d internal type

   Versions history :
   v0.0 (JF): initial release (from Fpack)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <string.h>

#include "mw.h"

#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)])

/* ----- (Undocumented) external functions ----- */

/*
  Provides the number of coefficients at a given level, 
  given the initial size of the image
 */

int mw_bandsize_wpack2d(initial_size,current_level)

int initial_size;
int current_level;

{
  int res,i;
  
  res=initial_size;
  for(i=0 ; i<current_level ; i++)
    {
      if(res & 1)
	res=(res+1)/2;
      else
	res=res/2;
    }
  return(res);
}

/* ----- Internal functions ----- */

/*
  Complete memory allocation for Fimage
 */
static Fimage alloc_image(int kx, Wpack2d pack, int start_nrow, int start_ncol)
{
  pack->images[kx]=mw_change_fimage(pack->images[kx],  
				    mw_bandsize_wpack2d(start_nrow,pack->tree->gray[kx]),
				    mw_bandsize_wpack2d(start_ncol,pack->tree->gray[kx]));
  return(pack->images[kx]);
}

/*
  Allocates memory for images needing it
  Return 1 if OK, < 0 elsewhere.
*/
static int indice_alloc(pack,posx,posy,current_level,start_nrow,start_ncol)
     
Wpack2d pack ;
int posx;
int posy;
int current_level;
int start_nrow;
int start_ncol;

{
  int i1, i2 , i3, i4;
  int posx1, posy1;
  int shift,r;
  
  /* calculate upper left position of each square */
  /* shift=pack->tree->ncol/power(2,current_level); */
  shift=pack->tree->ncol >> current_level;

  posx1 = posx +shift;
  posy1 = posy + shift;
 
  i1 = posy*pack->tree->ncol+posx;
  i2 = posy*pack->tree->ncol+posx1;
  i3 = posy1*pack->tree->ncol+posx;
  i4 = posy1*pack->tree->ncol+ posx1;
  
  /* check if image should exist or not */
  if (current_level == pack->tree->gray[i1])
    {
      if ((pack->images[i1]=alloc_image(i1, pack, start_nrow,start_ncol))==NULL)
	return(-1);
    }
  else
    {
      r=indice_alloc(pack, posx, posy, current_level+1,start_nrow,start_ncol);
      if (r<0) return(r);
    }
  if (current_level) /*those allocation can only be performed if current_level != 0*/
    {
      if (current_level == pack->tree->gray[i2] )
	{
	  if ((pack->images[i2]=alloc_image(i2, pack, start_nrow,start_ncol))==NULL)
	    return(-2);
	}
      else
	{
	  r=indice_alloc(pack, posx1, posy, current_level+1,start_nrow,start_ncol);
	  if (r<0) return(r);
	}   
      if (current_level == pack->tree->gray[i3])
	{
	  if ((pack->images[i3]=alloc_image(i3, pack, start_nrow,start_ncol))==NULL)
	    return(-3);
	}
      else
	{
	  r=indice_alloc(pack, posx, posy1, current_level+1, start_nrow,start_ncol);
	  if (r<0) return(r);
	}   
      
      if (current_level == pack->tree->gray[i4])
	{
	  if((pack->images[i4]=alloc_image(i4, pack, start_nrow,start_ncol))==NULL)
	    return(-4);
	}
      else
	{
	  r=indice_alloc(pack, posx1, posy1, current_level+1,start_nrow,start_ncol);
	  if (r<0) return(r);
	}   	  
    }
  return(1); /* OK */
}


/* ----- External functions ----- */

/* 
 Allocates empty Wpack2d structure
 returns address of the new Wpack2d
 */

Wpack2d mw_new_wpack2d()
{
  Wpack2d newpack;

  /* allocate memory for structure */
  if ((newpack=malloc(sizeof(struct wpack2d)))==NULL)
    mwerror(FATAL,-1,"[mw_new_wpack2d] Not enough memory\n");
  
  /* set name to default */
  sprintf(newpack->name, "%s", WPACK2D_DEFAULT_NAME);
  
  /* set comment to default */
  sprintf(newpack->cmt , "%s", WPACK2D_DEFAULT_CMT);
  
  newpack->signal1 = NULL;
  newpack->signal2 = NULL;
  newpack->level = 0;
  newpack->img_array_size=0;
  newpack->img_ncol=0;
  newpack->img_nrow=0;
  newpack->tree = NULL;
  newpack->images = NULL;
  newpack->previous = NULL;
  newpack->next = NULL;

  return(newpack);
}

/*
  Check if an image can be considered a tree, for a wavelet packet decomposition. 
  If the image is a tree, the function returns its maximum level of decomposition
*/

int mw_checktree_wpack2d(tree)
     
Cimage tree;
     
{int i,x,y,tree_size,sub_tree_size,level,ratio;
 int v,k,kx,ky; 
 int sxy;

 /* Test on the image size */
 if(tree->ncol != tree->nrow)
   mwerror(FATAL, 1,"[mw_checktree_wpack2d] Not a tree : the image does not have the same number of column and raws\n");
 if(tree->ncol==0)
   mwerror(FATAL, 1, "[mw_checktree_wpack2d] Not a tree : empty image\n");

 tree_size=tree->ncol;
 level=0;
 while((tree_size&1)==0)
   {tree_size/=2;
   level++;
   }

 if(tree_size!=1)
  mwerror(FATAL, 1, "[mw_checktree_wpack2d] Not a tree : the number of columns and raws is not a power of 2\n");
  
 tree_size=tree->ncol;

 /* Test if the maximum of the image corresponds to a tree  structure*/
 for(x=0;x<tree_size;x++) for(y=0;y<tree_size;y++)
   if(_(tree,x,y)>level)
     mwerror(FATAL, 1, "[mw_checktree_wpack2d] Not a tree : an image value exceeds the maximum level (%d)\n",level);
      
 /* Test if the image corresponds to a tree structure.
  */
 for(k=0,sub_tree_size=tree_size,ratio=1;k<level;k++)
   {
     for(kx=0;kx<ratio;kx++) for(ky=0;ky<ratio;ky++)
       {
	 v=_(tree,kx*sub_tree_size,ky*sub_tree_size);
	 if(v>k)
	   /* Check if all the image values corresponding to children 
	      are more decomposed than the father 
	   */
	   {
	     for(x=0;x<sub_tree_size;x++) for(y=0;y<sub_tree_size;y++)
	       if(_(tree,kx*sub_tree_size+x,ky*sub_tree_size+y)<=k)
		 mwerror(FATAL, 1, "[mw_checktree_wpack2d] Not a tree : a child is less decomposed than its father\n");
	   }
	 else
	   if(v==k)
	     /* Check if all the image values corresponding to children 
		equals the decomposition level of the father, when the father is a terminal node 
	     */
	     {
	       for(x=0;x<sub_tree_size;x++) for(y=0;y<sub_tree_size;y++)
		 if(_(tree,kx*sub_tree_size+x,ky*sub_tree_size+y)!=k)
		   mwerror(FATAL, 1, "[mw_checktree_wpack2d] Not a tree : a father is not a consistent terminal mode\n");
	     }
       }
     sub_tree_size/=2;
     ratio*=2;
   }

 /*computes decomposition level */
 sxy=tree->ncol*tree->nrow;
 level=tree->gray[0];
 for(i=1;i<sxy;i++)
   {
     v=tree->gray[i];
     if(level<v) level=v;
   }
 return(level); 
}


/*
  Allocates memory for fully fonctionnal Wpack2d and fills fields according to the given tree
  pack : Wpack2d to use, mw_new_wpack2d must have been used first
  tree : Cimage coding the decomposition tree
  start_ncol : image to decompose number of columns, used to pre-alloc decomposition images
  start_nrow : image to decompose number of rows, used to pre-alloc decomposition images
  may be 0 if you don't want pre-allocation
  returns address of the pack
 */

Wpack2d mw_alloc_wpack2d(pack, tree, signal1, signal2, start_nrow, start_ncol)

Wpack2d pack;
Cimage tree;
Fsignal signal1;
Fsignal signal2;
int start_nrow;
int start_ncol;

{
  int i;
  int i1,i2;
  
  if (tree == NULL ||  signal1==NULL )
    /* structure useless for wavelet packet purpose */
    {
      mwerror(WARNING,-1,"[mw_alloc_wpack2d] NULL tree or signal structure\n");
      return(NULL);
    }

  /* checks tree and computes the maximum decomposition level */   
  pack->level = mw_checktree_wpack2d(tree);
      
  /* checks decomposition level versus  image size */
  /* First, we compute the smallest possible size of a packet*/
  if(signal2)
    {i1= (signal1->size+(int)signal1->shift) >  (2- (int)signal1->shift) ?    (signal1->size+(int)signal1->shift) :   (2- (int)signal1->shift)  ;
    i2= (signal2->size+(int)signal2->shift) >  (2- (int)signal2->shift) ?    (signal2->size+(int)signal2->shift) :   (2- (int)signal2->shift)  ;
    i = i1>i2 ? i1:i2 ;
    }
  else
    i= (signal1->size+(int)signal1->shift) >  (2- (int)signal1->shift) ?    (signal1->size+(int)signal1->shift) :   (2- (int)signal1->shift)  ;
    
  if(mw_bandsize_wpack2d(start_ncol, pack->level )<i || mw_bandsize_wpack2d(start_nrow, pack->level )<i)
    mwerror(FATAL,-1,"[mw_alloc_wpack2d] Decomposition level is too large\n");
    
  pack->img_ncol=start_ncol;
  pack->img_nrow=start_nrow;
  
  /* initializes pack->tree */
  if ((pack->tree = mw_change_cimage(pack->tree, tree->nrow , tree->ncol))==NULL)
    return(NULL);
    
  mw_copy_cimage(tree,pack->tree);
  sprintf(pack->tree->name, "%s", tree->name);
     
  /* initializes pack->signal1 */
  pack->signal1 = mw_change_fsignal(pack->signal1, signal1->size);
  mw_copy_fsignal(signal1,pack->signal1);
  sprintf(pack->signal1->name, "%s", signal1->name);
  pack->signal1->shift=signal1->shift;
    
    /* initializes pack->signal2 */
    if(signal2==NULL)
      {
	pack->signal2 = mw_change_fsignal(pack->signal2, signal1->size);
	if (!pack->signal2) return(NULL);
	mw_copy_fsignal(signal1,pack->signal2);
	sprintf(pack->signal2->name, "%s", signal1->name);
	pack->signal2->shift=signal1->shift;
      }
    else
      {
	pack->signal2 = mw_change_fsignal(pack->signal2, signal2->size);
	if (!pack->signal2) return(NULL);
	mw_copy_fsignal(signal2,pack->signal2);
	sprintf(pack->signal2->name, "%s", signal2->name);
	pack->signal2->shift=signal2->shift;
      }

    /* allocates memory for images, first we need to free those of  pack->images which are not NULL*/
    if (pack->images==NULL)
      {
	pack->img_array_size=pack->tree->ncol*pack->tree->nrow;
	if ((pack->images = malloc(pack->img_array_size*sizeof(Fimage)))==NULL)
	  mwerror(FATAL,-1,"[mw_alloc_wpack2d] Not enough memory for image array\n");

	for (i=0 ; i<pack->img_array_size ; i++)
	  pack->images[i] = NULL;
      }
    else
      {
	for (i=0 ; i<pack->img_array_size ; i++)
	  if(pack->images[i])
	    {
	      mw_delete_fimage(pack->images[i]);
	      pack->images[i]=NULL;
	    }
      
	pack->img_array_size=pack->tree->ncol*pack->tree->nrow;
      
	if ((pack->images = realloc(pack->images,pack->img_array_size*sizeof(Fimage)))==NULL)
	  mwerror(FATAL,-1,"[mw_alloc_wpack2d] Not enough memory for image array\n");
	
	for (i=0 ; i<pack->img_array_size ; i++)
	  pack->images[i] = NULL;
      }
  
    /*now pack->images is allocated and contains NULL images*/
    /* we allocate memory to those images */
    if (indice_alloc(pack , 0, 0,0, start_nrow, start_ncol)<0) /*recursive memory allocation*/
      return(NULL); 
    
  
    /*returns the result */
    return(pack);
}


/* Free all the memory used by a Wpack2d
   pack : Wpack2d to delete
 */
void mw_delete_wpack2d(pack)

Wpack2d pack;

{
  int i;

  if (pack!=NULL)
    { /* avoids crash with already deleted Wpack2d */
      if (pack->signal1!=NULL)
	{
	  mw_delete_fsignal(pack->signal1);
	  pack->signal1=NULL;
	}
      if (pack->signal2!=NULL)
	{
	  mw_delete_fsignal(pack->signal2);
	  pack->signal2=NULL;
	}
      if (pack->tree!=NULL)
	{
	  mw_delete_cimage(pack->tree);
	  pack->tree=NULL;
	}
      if (pack->images!=NULL)
	for(i=0 ; i<pack->img_array_size ;i++)
	  if (pack->images[i])
	    { 
	      mw_delete_fimage(pack->images[i]);
	      pack->images[i]=NULL;
	    }
      free(pack->images);
      pack->images=NULL;

      /* link previous with next if needed */
      if (pack->next != NULL)
	{
	  if(pack->previous!= NULL)
	    pack->next->previous=pack->previous;
	  else
	    pack->next->previous=NULL;
	}
      if (pack->previous !=NULL)
	{
	  if(pack->next!= NULL)
	    pack->previous->next=pack->next;
	  else
	    pack->previous->next=NULL;
	}
      
      free(pack);
    }
}

/*
  Realloc image array and related fields
  structure address doesn't change
  Returns structure address or NULL on errors
*/

Wpack2d mw_change_wpack2d(pack, tree, signal1, signal2, start_nrow, start_ncol)

Wpack2d pack;
Cimage tree;
Fsignal signal1;
Fsignal signal2;
int start_nrow;
int start_ncol;

{
  if (pack==NULL)
    { 
      /* create and alloc */  
      if (((pack=mw_new_wpack2d())==NULL)||
	  (mw_alloc_wpack2d(pack,tree,signal1,signal2,start_nrow,start_ncol)==NULL))
	return(NULL);
    }
  else
    { /* alloc only */
      if(mw_alloc_wpack2d(pack,tree,signal1,signal2,start_nrow,start_ncol)==NULL)
	return NULL;
    }
  return(pack);
}


/*
  This function copies the content of the image array of a Wpack2d
  in : copy source
  out : copy target
  new_tree_size : if we want our tree to be larger than in 'in' 
                                (with same wavelet packet decomposition)
*/
void mw_copy_wpack2d(in,out,new_tree_size)

Wpack2d in;
Wpack2d out;
int new_tree_size;

{
  int i,i1;
  int kx,ky,kx1,ky1,extra,x,y;
  unsigned char value;
  Cimage tree;
  
  /* modify tree if needed */ 
  if (in->tree->ncol < new_tree_size)
    {
      extra=new_tree_size/in->tree->ncol;
      tree=mw_change_cimage(NULL, new_tree_size,new_tree_size);
      if (!tree) mwerror(FATAL,-1,"[mw_copy_wpack2d] Not enough memory\n");
      
      for(kx=0,kx1=0;kx<in->tree->ncol;kx++,kx1+=extra)
	for(ky=0,ky1=0;ky<in->tree->nrow;ky++,ky1+=extra)
	  {
	    value=in->tree->gray[kx*in->tree->ncol+ky];
	    
	    for(x=0;x<extra;x++) for(y=0;y<extra;y++)
	      tree->gray[(kx1+x)*new_tree_size+ky1+y]=value;
	  }
      sprintf(tree->name, "%s",in->tree->name);
      
      out = mw_change_wpack2d(out, tree, in->signal1, in->signal2, in->img_nrow,in->img_ncol);
      if (!out) mwerror(FATAL,-1,"[mw_copy_wpack2d] Not enough memory\n");      
      mw_delete_cimage(tree);
  
      for(i=0;i<out->img_array_size;i++)
	if (out->images[i])
	  {
	    mw_delete_fimage(out->images[i]);
	    out->images[i] = NULL;
	  }
  
      for(kx=0,kx1=0;kx<in->tree->ncol;kx++,kx1+=extra)
	for(ky=0,ky1=0;ky<in->tree->nrow;ky++,ky1+=extra)
	  {
	    i=kx*in->tree->ncol+ky;
	
	    if (in->images[i] != NULL)
	      {
		i1=kx1*out->tree->ncol+ky1;
		if (!(out->images[i1]=mw_change_fimage(NULL, in->images[i]->nrow, in->images[i]->ncol)))
		  mwerror(FATAL,-1,"[mw_copy_wpack2d] Not enough memory\n");      		
		mw_copy_fimage(in->images[i], out->images[i1]);
	      }
	  }
    
    }
  else
    {
      /* change out to fit in */
      out = mw_change_wpack2d(out, in->tree, in->signal1, in->signal2, in->img_nrow,in->img_ncol);
      if (!out) mwerror(FATAL,-1,"[mw_copy_wpack2d] Not enough memory\n");           

      /*copy images */
      for(i=0 ; i<in->img_array_size ; i++)
	if (in->images[i] != NULL)
	  {
	    if (!(out->images[i]=mw_change_fimage(out->images[i], in->images[i]->nrow, in->images[i]->ncol)))
	      mwerror(FATAL,-1,"[mw_copy_wpack2d] Not enough memory\n");      			      
	    mw_copy_fimage(in->images[i], out->images[i]);
	  }
    }
  
  /* previous and next still need to be filled out*/
  out->previous = NULL;
  out->next = NULL;
  
}

/*
  Clear all images in the decomposition
  v : color for image filling
*/

void mw_clear_wpack2d(pack,v)

Wpack2d pack;
float v;
{
  int i;

  for (i=0; i<pack->img_array_size; i++)
    if (pack->images[i] != NULL)
      mw_clear_fimage(pack->images[i], v);
}


/*
  Prune the tree of the wpack2d in and write the result in the wpack2d out 
*/

void mw_prune_wpack2d(in,out,tree)

Wpack2d in;
Wpack2d out;
Cimage tree;

{
  int i,i1;
  int kx,ky,extra;

  if(in->tree->ncol <= tree->ncol)
    {
      /*copy the result if tree need not being modified */
      mw_copy_wpack2d(in, out, 0);    
      sprintf(out->tree->name, "%s", tree->name);
    }
  else 
    /* modify tree if needed */ 
    {
      extra=in->tree->ncol/tree->ncol;
      out=mw_change_wpack2d(out, tree, in->signal1, in->signal2, in->img_nrow,in->img_ncol);
      if (!out) mwerror(FATAL,-1,"[mw_prune_wpack2d] Not enough memory\n");      	

      /*AT THIS STAGE : 
	the tree, level, the signals, img_ncol,img_nrow and img_array_size are copied in "out"
	the name, comment, next, previous of out are preserved.
      */

      /* copy image array */
      for(kx=0;kx<out->tree->ncol;kx++)
	for(ky=0;ky<out->tree->nrow;ky++)
	  {i=kx*out->tree->ncol+ky;
	  if(out->images[i])
	    {
	      mw_delete_fimage(out->images[i]);
	      out->images[i] = NULL;
	    }
	  }

      for(kx=0;kx<in->tree->ncol;kx++)
	for(ky=0;ky<in->tree->nrow;ky++)
	  {i=kx*in->tree->ncol+ky;
	  i1=(kx*out->tree->ncol+ky)/extra;

	  if (in->images[i] != NULL)
	    {
	      if (!(out->images[i1]=mw_change_fimage(NULL, in->images[i]->nrow, in->images[i]->ncol)))
		mwerror(FATAL,-1,"[mw_prune_wpack2d] Not enough memory\n");      	
	      mw_copy_fimage(in->images[i], out->images[i1]);
	    }
	  }
      
    }
}

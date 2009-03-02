/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {wp2deigenval};
version = {"1.0"};
author = {"Francois Malgouyres"};
function = {"Compute the eigenvalues of an operator which is diagonal in a 2D-wavelet packet basis and approximate the convolution with a kernel (or its inverse)"};
usage = {
  'i':float->limit
     "Compute the approximation of the truncated inverse of the input filter",
  filter->filter
     "Real part of the Fourier transform of convolution kernel",
  tree->tree       "Quad-tree of the wavelet packet basis",
  Fimage<-pfilter  "The eigenvalues of the operator"
};
*/
/*----------------------------------------------------------------------
 v1.0: adaptation from filter2pfilter (fpack) (JF)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for wp2dfreqorder() */

#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)])

/***************************************************************/
static int my_log(int n)
{int count=0;
 
 if (n<1) 
   mwerror(FATAL, 1, "Tree size must be positive !\n");
 
 while ((n&1)==0) 
   {n=(n>>1);
   count++;
   }
 
 if(n==1) return(count);
 else 
   mwerror(FATAL, 1, "Tree size must be a power of 2!\n");
 return count;
}
/***************************************************************/
static float signedSquare(float input)   
{if(input>0) return(input*input);
 else return(-input*input);
}

/***************************************************************/
static void normalize(Fimage pfilter)
     /*normalizes sothat pfilter->gray[0]=1*/
                    
     
{int size = pfilter->ncol*pfilter->nrow;
 int i;
 float factor=1./pfilter->gray[0];
 
 for(i=0;i<size;i++) 
   pfilter->gray[i]*=factor;
 
}


/***************************************************************/

static void inverse_filter(Fimage filter, Fimage work, float *limit)
     /*Computes the inverse of the input filter, the result is in work*/
     /*The modulus of the inverse is truncated if larger the 1/limit */
                        
                  
     
{int i,size=filter->ncol*filter->nrow;
 float invLimit=1./(*limit),minusInvLimit=-1./(*limit);
 float Limit=*limit,minusLimit=-(*limit);
 float tmp;
 
 for(i=0;i<size;i++)
   {tmp=filter->gray[i];
   
   if(tmp>Limit)
     work->gray[i]=1./tmp;
   else if(tmp>0)
     work->gray[i]=invLimit;
   else if(tmp==0)
     work->gray[i]=0;
   else if(tmp> minusLimit)
     work->gray[i]=minusInvLimit;
   else
     work->gray[i]=1./tmp;
   }
 
}

/***************************************************************/

static void approximation(Fimage filter, Cimage tree, Fimage pfilter)
{Fsignal order;                                        /*for the correspondance between wavelet packets indexes and frequency bands */
 int maxLevel=my_log( tree->ncol ); /*maximum possible level */
 int level;                                                        /*current level */
 int incr_x_tree,incr_y_tree;                                              /* increments in tree coordinates*/
 int kx,ky,kx_tree,ky_tree;                 /* for coordinates in tree and pfilter*/
 int  kx1,ky1;                                         /*frequency band corresponding to tree leave (kx,ky)*/
 int x,y;                                                   /* used for the coordinates in filter */ 
 int x1,x2,y1,y2;                                   /* coordinates in filter */
 int sfx,sfy;                                            /*size of the frequency band (divided by 2), in x and in y*/
 float l2;                                                 /* computed value */
 int count;                                             /* number of frequencies in a frequency band */
 
 /*memory allocation */
 order=mw_change_fsignal(NULL,tree->ncol);
 if(order==NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 /* make correspondance between wavelet packet indexes and frequency bands*/
 
  wp2dfreqorder(maxLevel,order,NULL);
 
 /* computes the approximation */
 
 for(level=0,incr_x_tree=tree->ncol/2,incr_y_tree=tree->nrow/2,sfx=filter->ncol/4,sfy=filter->nrow/4;
     level<maxLevel;
     level++,incr_x_tree/=2,incr_y_tree/=2,sfx/=2,sfy/=2)
   {for (kx=0;kx<tree->ncol;kx+=incr_x_tree) for (ky=0;ky<tree->nrow;ky+=incr_y_tree)
     if(_(tree,kx,ky)==level+1)
       {kx1=(int) order->values[kx/incr_x_tree]-1;
       ky1=(int) order->values[ky/incr_y_tree]-1;
       
       l2=0.;
       count=0;
       for(x=0;x<sfx;x++)for(y=0;y<sfy;y++)
	 {x1=x+kx1*sfx;
	 y1=y+ky1*sfy;
	 x2=filter->ncol-x1-1;
	 y2=filter->nrow-y1-1;

	 l2+=signedSquare(_(filter,x1,y1));
	 l2+=signedSquare(_(filter,x1,y2));
	 l2+=signedSquare(_(filter,x2,y1));
	 l2+=signedSquare(_(filter,x2,y2));
	 
	 count+=4;
	 }
       
       if(count==0)
	 mwerror(FATAL, 1, "size of filter should be larger than size of pfilter !\n");
       
       if( l2 < 0)
	 l2 = (float) -sqrt(- (double) l2 / count);
       else
	 l2 = (float) sqrt((double) l2 / count);
       
       for(kx_tree=0;kx_tree<incr_x_tree;kx_tree++)
	 for(ky_tree=0;ky_tree<incr_y_tree;ky_tree++)
	   _(pfilter,kx+kx_tree,ky+ky_tree)=l2;
       }
   }

  mw_delete_fsignal(order); 

}

/***************************************************************/
/***************      MAIN      ********************************/
/***************************************************************/
void wp2deigenval(Fimage filter, Cimage tree, Fimage pfilter, float *limit)
{

  Fimage work;

  /*test inputs */ 
  mw_checktree_wpack2d(tree);

 if(limit)
   if(((*limit) < 0) || ((*limit) > 1)) 
     mwerror(FATAL, 1, "limit should be between 0 and 1 !\n");

/*memory allocations */
 work = mw_change_fimage(NULL,filter->nrow,filter->ncol);
 if (work == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 pfilter = mw_change_fimage(pfilter,tree->nrow,tree->ncol);
  
 /* inverse filter, when necessary */
 
 if(limit)
   inverse_filter(filter,work,limit);
 else
   mw_copy_fimage(filter,work);
 
 /* approximation */
 
 approximation(work,tree,pfilter);
 
 /*normalizes sothat pFilter->gray[0]=1*/
 
 normalize(pfilter);
 
 /*free memory */
 mw_delete_fimage(work);
 
}




/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {wp2dview};
version = {"1.1"};
author = {"Francois Malgouyres"};
function = {"Compute and display the wavelet packet transform of a Fimage"};
usage = {
 'b':   h_tilde->Ri_biortho
   "impulse response of h_tilde (biorthogonal wavelet)",
 'o'-> do_not_reorder_flag
   "in order NOT to reorder subbands according to the frequencial order",
 's'-> do_not_rescale_flag
   "in order NOT to rescale wavelet packet coefficients",
 'R':   Fimage<-toDisplay
   "does not display the result but saves it in a Fimage",
 'p': pack_in->input_pack
   "Input Wpack2d to display (do not compute wavelet packets from the Fimage)",
  {
    tree->tree   "input Cimage describing the tree",
     Fimage->A   "input Fimage",
     h->Ri       "input Fsignal giving the impulse response of the filter h"
   }
};
*/
/*----------------------------------------------------------------------
 v1.0 (03/2006): adaptation from fpackview v0.1 (fpack) (JF)
 v1.1 (04/2006): option -p added, change arguments to optional arg (JF)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for w2dfreqreorder(), wp2ddecomp() */

#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)])


static float max_of_wpack2d(pack)
     
     Wpack2d pack;
     
{int k;
 int i,size;
 float max;

 for(k=0;k<pack->img_array_size;k++)
   if(pack->images[k])
     {size=pack->images[k]->ncol * pack->images[k]->nrow;
     
     for(i=0;i<size;i++)
       max=(max>pack->images[k]->gray[i]) ? max:pack->images[k]->gray[i];

     }

 return(max);
}
 
/***************************************************************/
static float  increaseContrast(pack)

     Wpack2d pack;

{int k,i;
 int sxy;                                                /*size of the current image of the pack */
 float max0, min0,mean0;              /*mean, max and min values of pack->images[0]*/
 float max, min,mean;                     /*mean,max and min values of pack->images[k]*/
 float tmp,tmpSmaller,tmpLarger;            /*temporary values (see context)*/

 /*computes the mean, min and max values of pack->images[0]*/
 if(pack->images[0])
   {sxy=pack->images[0]->ncol*pack->images[0]->nrow;

   mean0=0.;
   min0=max0=pack->images[0]->gray[0];
   for(i=1;i<sxy;i++)
     {tmp=pack->images[0]->gray[i];

     mean0+=tmp;
     if(tmp>max0) max0=tmp;
     if(tmp<min0) min0=tmp;
     }
   mean0/=(float) sxy;
   }

 /*rescale all the other images */
 for(k=1;k<pack->img_array_size;k++)
   if(pack->images[k])
     {sxy=pack->images[k]->ncol*pack->images[k]->nrow;

     /*computes the min and max of pack->images[k]*/
     mean=0.;
     min=max=pack->images[k]->gray[0];
     for(i=1;i<sxy;i++)
       {tmp=pack->images[k]->gray[i];

       mean+=tmp;
       if(tmp>max) max=tmp;
       if(tmp<min) min=tmp;
       }
     mean/=(float) sxy;
     /* rescale pack->images[k] */
     /* the rescaling is affine on each side of mean*/
     
     if(max!=min)
       {tmpSmaller=(mean0-min0)/(mean-min);
       tmpLarger=(max0-mean0)/(max-mean);
       
       for(i=0;i<sxy;i++)
	 {tmp=pack->images[k]->gray[i];
	 if(tmp<mean)
	   pack->images[k]->gray[i]=tmpSmaller*(tmp-min)+min0;
	 else
	   pack->images[k]->gray[i]=tmpLarger*(tmp-mean)+mean0;
	 
	 }
       }
     else
       {for(i=0;i<sxy;i++)
	 pack->images[k]->gray[i]=mean0;
       }
     }

 return(max0);
}
/***************************************************************/
static void increaseSize(pack,backgroundColor)
     /*adds a line of one pixel width around each image */

     Wpack2d pack;
     float backgroundColor; /* color for the background */

{int k;                                            /* index for images in the Wpack2d*/
 int sx,sy;                                       /* initial size of the image */
 int x,y;                                          /* index for images */
 int sxFinal,syFinal;                  /* final image size */
 int tx=1,ty=1;                            /*minimum number on the sides of each image*/
 Fimage tmpImg;                      /*contains the result, before it is copied in the Wpack2d structure */

 tmpImg=mw_new_fimage();
 
 for(k=1;k<pack->img_array_size;k++)
   if(pack->images[k])
     {sx=pack->images[k]->ncol;
     sxFinal=sx+2*tx;

     sy=pack->images[k]->nrow;
     syFinal=sy+2*ty;
     
     tmpImg= mw_change_fimage(tmpImg,syFinal,sxFinal);
     
     mw_clear_fimage(tmpImg,backgroundColor);
 
     for(x=0;x<sx;x++) for(y=0;y<sy;y++)
       _(tmpImg,tx+x,ty+y)=_(pack->images[k],x,y);
     
     pack->images[k] = mw_change_fimage(pack->images[k],syFinal,sxFinal);
     mw_copy_fimage(tmpImg,pack->images[k]);
     
     }

 mw_delete_fimage(tmpImg);
}


/***************************************************************/
static void glue(pack, img, img11, img12, img21,img22,backgroundColor,level)
     /* glue four images together */

     Wpack2d pack; 
     int img;       /* index containing the result */
     int img11;  /* index of the low in x -low in y image */
     int img12;  /* index of the low in x -high in y image */
     int img21;  /* index of the high in x -low in y image */
     int img22;  /* index of the high in x -high in y image */
     float backgroundColor; /* color for the background */ 
     int level;

{int sx11=pack->images[img11]->ncol;/*size of the image11 */
 int sy11=pack->images[img11]->nrow;
 int sx12=pack->images[img12]->ncol;/*size of the image12 */
 int sy12=pack->images[img12]->nrow;
 int sx21=pack->images[img21]->ncol;/*size of the image21 */
 int sy21=pack->images[img21]->nrow;
 int sx22=pack->images[img22]->ncol;/*size of the image22 */
 int sy22=pack->images[img22]->nrow;
 int sx1=(sx11>sx21)? sx11:sx21; /* image11 must fit into a window of size sx1 and sy1*/
 int sy1=(sy11>sy12)? sy11:sy12; /* image12 must fit into a window of size sx2 and sy1*/
 int sx2=(sx12>sx22)? sx12:sx22; /* image21 must fit into the window of size sx1 and sy2*/
 int sy2=(sy21>sy22)? sy21:sy22; /* image22 must fit into the window of size sx2 and sy2*/
 int tx,ty;                                             /*translation vector*/
 int sx=sx1+ sx2,sy=sy1+sy2;      /*size of the final image */
 int x,y;                                              /*index in the image */                                        
 Fimage result;                                /* contains the result before it is copied in the Wpack2d structure */

 result = mw_change_fimage(NULL,sy,sx);

 /*We color the background*/
 mw_clear_fimage(result,backgroundColor);

 /*We write image11 in result*/

 tx=(sx1-sx11)/2;
 ty=(sy1-sy11)/2;

 for(x=0;x<sx11;x++) for(y=0;y<sy11;y++)
   _(result,tx+x,ty+y)=_(pack->images[img11],x,y);

 /*We write image12 in result*/

 tx=sx1+(sx2-sx12)/2;
 ty=(sy1-sy12)/2;

 for(x=0;x<sx12;x++) for(y=0;y<sy12;y++)
   _(result,tx+x,ty+y)=_(pack->images[img12],x,y);

 /*We write image21 in result*/

 tx=(sx1-sx21)/2;
 ty=sy1+(sy2-sy21)/2;

 for(x=0;x<sx21;x++) for(y=0;y<sy21;y++)
   _(result,tx+x,ty+y)=_(pack->images[img21],x,y);

 /*We write image22 in result*/

 tx=sx1+(sx2-sx22)/2;
 ty=sy1+(sy2-sy22)/2;

 for(x=0;x<sx22;x++) for(y=0;y<sy22;y++)
   _(result,tx+x,ty+y)=_(pack->images[img22],x,y);


 /*We update pack*/

 mw_delete_fimage(pack->images[img]);
 pack->images[img]=NULL;
 pack->images[img]=mw_change_fimage(NULL,sy,sx);
 mw_copy_fimage(result,pack->images[img]);

 mw_delete_fimage(result);result=NULL;

 if(img!=img11)
   {mw_delete_fimage(pack->images[img11]);
   pack->images[img11]=NULL;
   }

 if(img!=img12)
   {mw_delete_fimage(pack->images[img12]);
   pack->images[img12]=NULL;
   }

 if(img!=img21)
   {mw_delete_fimage(pack->images[img21]);
   pack->images[img21]=NULL;
   }

  if(img!=img22)
    {mw_delete_fimage(pack->images[img22]);
    pack->images[img22]=NULL;
    }

}
/***************************************************************/


/***************************************************************/
static void  wpack2dGlue(pack,backgroundColor,do_not_reorder_flag)
     /* glue all the images of pack->images together */

     Wpack2d pack;
     float backgroundColor;
     char* do_not_reorder_flag;

{int  level,treesize=pack->tree->ncol;
 int kx,ky,kx1,ky1;                     /*indexes to go through pack->images*/
 int lx,ly,lx1,ly1;                         /*index of the images to be glued*/
 int img;                                       /* index of the element of pack->images where the result is writen*/
 int img11,img12,img21,img22; /* index of the element of pack->images to be glued */
 int jump,jump_2;                        /*increments on kx,ky,kx1,ky1*/
 Fsignal order;

 /*initialisation*/
 if( !do_not_reorder_flag )
   {
     /*order = mw_change_fsignal(NULL,power(2,pack->level));*/
     order = mw_change_fsignal(NULL,1<<pack->level);
     wp2dfreqorder(pack->level,order,NULL);
   }

 level=pack->level-1;
 /*jump_2=treesize/power(2,level);*/
 jump_2=treesize >> level;
 jump=jump_2/2;

 /*we glue images of the Wpack2d structure (as in owave_recomp), */
 /* from larger levels to smaller ones.   */

  for(;level>=0;level--){ 
    for(ky = 0, ky1=jump  ; ky1<treesize ; ky+=jump_2,ky1+=jump_2)
      for(kx = 0, kx1=jump ; kx1<treesize ; kx+=jump_2,kx1+=jump_2)
  	{img=kx+ky*treesize;

	lx=kx;lx1=kx1;ly=ky;ly1=ky1;

	if(!do_not_reorder_flag) 
	  {if(order->values[kx/jump] > order->values[kx1/jump] )
	    {lx=kx1;
	    lx1=kx;
	    }
	 
	  if(order->values[ky/jump] > order->values[ky1/jump]  )
	    {ly=ky1;
	    ly1=ky;
	    }
	  }
	
	img11=lx+ly*treesize;
	img12=lx1+ly*treesize;
	img21=lx+treesize*ly1;
	img22=lx1+treesize*ly1;

	if(pack->images[img11] && pack->images[img21] 
	   && pack->images[img12] && pack->images[img22])
	  glue(pack, img, img11, img12, img21, img22, backgroundColor,level);
	  
	}
    jump_2*=2;
    jump*=2;
  }

  if( !do_not_reorder_flag) mw_delete_fsignal(order);
}

/***************************************************************/


/*-------------------------------------------------------------*/
/*-------------  MAIN                ----------------------------*/
/*-------------------------------------------------------------*/

void wp2dview(A,Ri,Ri_biortho,tree, do_not_reorder_flag,do_not_rescale_flag,toDisplay,input_pack)    
     /*displays a wavelet packet transform */

     Fimage A,toDisplay;
     Cimage tree;
     Fsignal Ri,Ri_biortho;
     char* do_not_reorder_flag,*do_not_rescale_flag;
     Wpack2d input_pack;

{
  Wpack2d pack;
  float backgroundColor;
  char command[256],name[]="AAAAAAAAAAAAAAA";

  if (input_pack)
    /* the wavelet packet is given */
    {
      if (tree || Ri_biortho)
	mwerror(USAGE,1,"When you give the wavelet packet with option -p, do not input also the tree/image/impulse response !\n");

      pack=input_pack;
    }
  else
    {
      if (!tree)
	mwerror(USAGE,1,"Option -p is needed when inputs tree/image/impulse response are not given !\n");

      /*creates data structures*/
      pack=mw_new_wpack2d();
      /*computes wpack2d*/
      wp2ddecomp(A,Ri,Ri_biortho,pack,tree);
    }


  /* increase the contrast in the images of the wpack2d*/
  /*backgroundColor contains the max of the images, 'xv' displays it white */

  if(do_not_rescale_flag)
    backgroundColor=max_of_wpack2d(pack);
  else
    backgroundColor= increaseContrast(pack);   
  
  /* increase the size of the images to separate subbands */
  
  increaseSize(pack,backgroundColor);
  
  /* glue the wavelet packet coefficients contained in the images of the wpack2d*/
  
  wpack2dGlue(pack,backgroundColor,do_not_reorder_flag);
  
  /* displays the result */
  if(toDisplay) 
    {
      mw_change_fimage(toDisplay,pack->images[0]->nrow,pack->images[0]->ncol);
      mw_copy_fimage(pack->images[0],toDisplay);
    }
  else
    {
      _mw_fimage_create_image(name,pack->images[0],"RIM");
      sprintf(command,"(xv %s ;  rm -f %s) &",name,name);
      system(command); 
    }

  if (pack!=input_pack) mw_delete_wpack2d(pack);

}


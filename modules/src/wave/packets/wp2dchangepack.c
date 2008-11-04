/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {wp2dchangepack};
version = {"1.0"};
author = {"Francois Malgouyres"};
function = {"Rapidely transform a 2D-wavelet packet decomposition into another one, by specifying a new tree"};
usage = {
    tree_out->tree_out
      "Cimage describing the target tree",
   Wpack2d_in->pack_in
      "Input Wpack2d containing the original wavelet packet decomposition",
   Wpack2d_out<-pack_out
      "Output Wpack2d containing the new wavelet packet transform"
};
*/
/*----------------------------------------------------------------------
 v1.0: adaptation from fpack2fpack v0.1 (fpack) (JF)
----------------------------------------------------------------------*/

#include "mw.h"
#include "mw-modules.h" /* for wpsconvolve(), wp2dchangetree() */

#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)])

/***************************************************************/
/* Decompose pack->images[img11]                                       */
/* the result is in pack->images[img11] , pack->images[img12], */
/* pack->images[img21], pack->images[img22]                       */

static void ondel(pack,img11,img12,img21,img22,futur_level)

     Wpack2d pack; /* the initial image corresponds to the index  img11*/
     int img11;  /* index of the low in x -low in y image */
     int img12;  /* index of the low in x -high in y image */
     int img21;  /* index of the high in x -low in y image */
     int img22;  /* index of the high in x -high in y image */
     int futur_level;
     
{ 
  int x,y;
  int dx,dy;
  int dx_new;
  int dy_new;
  Fsignal signal_in,signal_out;
  char *band;
  Fimage hor1,hor2;

  
 /*Initialization of parameters and memory allocation */
 
 dx_new=mw_bandsize_wpack2d(pack->img_ncol, futur_level);
 dy_new=mw_bandsize_wpack2d(pack->img_nrow, futur_level); 

  dx=pack->images[img11]->ncol;
 dy=pack->images[img11]->nrow;
 
 band=(char *) 1;
 
 hor1 = mw_change_fimage(NULL, dy_new, dx);
 if (hor1 == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 hor2 = mw_change_fimage(NULL, dy_new, dx);
 if (hor2 == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 signal_in = mw_change_fsignal(NULL,dy);
 if (signal_in == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 signal_out = mw_change_fsignal(NULL,dy_new);
 if (signal_out == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 /*End of : Initialization of parameters and memory allocation */
 
 /*Convolution of Columns */
 
 for(x=0;x<dx;x++){/*convolution of each column*/
   for(y=0;y<dy;y++)
     signal_in->values[y]=pack->images[img11]->gray[y*dx+x];
   
   /*low pass filter*/
   wpsconvolve(signal_in, signal_out, pack->signal1, NULL, NULL, NULL);
   
   for(y=0;y<dy_new;y++)
     hor1->gray[y*dx+x]=signal_out->values[y];
   
   /*high pass filter*/
   wpsconvolve(signal_in,signal_out,pack->signal2,NULL,band,NULL);
   
   for(y=0;y<dy_new;y++)
     hor2->gray[y*dx+x]=signal_out->values[y];    
   
 }
  
  /*End of : Convolution of Columns */

 /*memory allocation for the convolution of raws */

 signal_in = mw_change_fsignal(signal_in,dx);
 if (signal_in == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 signal_out = mw_change_fsignal(signal_out,dx_new);
 if (signal_out == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 pack->images[img11]=mw_change_fimage(pack->images[img11], dy_new, dx_new);
 if(pack->images[img11] == NULL) mwerror(FATAL, 1, "Not enough memory !\n");

 pack->images[img21]=mw_change_fimage(pack->images[img21], dy_new, dx_new);
 if(pack->images[img21] == NULL) mwerror(FATAL, 1, "Not enough memory !\n");

 pack->images[img12]=mw_change_fimage(pack->images[img12], dy_new, dx_new);
 if(pack->images[img12] == NULL) mwerror(FATAL, 1, "Not enough memory !\n");
 
 pack->images[img22]=mw_change_fimage(pack->images[img22], dy_new, dx_new);
  if(pack->images[img22] == NULL) mwerror(FATAL, 1, "Not enough memory !\n");

 /* Convolution of raws */

 for(y=0;y<dy_new;y++){/*convolution of each raw for hor1 (contains low pass)*/
   for(x=0;x<dx;x++)
     signal_in->values[x]=hor1->gray[y*dx+x];
   
   /*low pass filter*/
   wpsconvolve(signal_in,signal_out,pack->signal1,NULL,NULL,NULL);   
   for(x=0;x<dx_new;x++)
     pack->images[img11]->gray[y*dx_new+x]=signal_out->values[x];
   
   /*high pass filter*/
   wpsconvolve(signal_in,signal_out,pack->signal2,NULL,band,NULL);
   for(x=0;x<dx_new;x++)
     pack->images[img21]->gray[y*dx_new+x]=signal_out->values[x];
   
 }
 
 for(y=0;y<dy_new;y++){/*convolution of each raw for hor2 (contains high pass)*/
   
   for(x=0;x<dx;x++)
     signal_in->values[x]=hor2->gray[y*dx+x];
   
   /*low pass filter*/
   wpsconvolve(signal_in,signal_out,pack->signal1,NULL,NULL,NULL);
   for(x=0;x<dx_new;x++)
     pack->images[img12]->gray[y*dx_new+x]=signal_out->values[x];
   
   /*high pass filter*/
   wpsconvolve(signal_in,signal_out,pack->signal2,NULL,band,NULL);
   for(x=0;x<dx_new;x++)
     pack->images[img22]->gray[y*dx_new+x]=signal_out->values[x];
 }
 
 /* End of : Convolution of raws */
 /* free useless memory */ 

 band=NULL;
 mw_delete_fsignal(signal_in);signal_in=NULL;
 mw_delete_fsignal(signal_out);signal_out=NULL;
 mw_delete_fimage(hor1);hor1=NULL;
 mw_delete_fimage(hor2);hor2=NULL;
 
}
/***************************************************************/
/* Recompose pack->images[img11], pack->images[img12], */
/* pack->images[img21], pack->images[img22]                       */
/* the result is in pack->images[img11] */

static void iondel(pack, img11, img12, img21,img22,futur_level)
     
     Wpack2d pack; /* the initial image corresponds to the index  img11*/
     int img11;  /* index of the low in x -low in y image */
     int img12;  /* index of the low in x -high in y image */
     int img21;  /* index of the high in x -low in y image */
     int img22;  /* index of the high in x -high in y image */
     int futur_level;
     
{ int dx_in,dy_in,dx_out,dy_out;
 int x,y;
 Fsignal inx,outx,iny,outy;
 Fimage tmp_result;
 char *not_null,*oddx, *oddy;
 
 /*Initialization of parameters */
 not_null=(char *)1;
 
 dx_in=pack->images[img11]->ncol;
 dx_out=mw_bandsize_wpack2d(pack-> img_ncol, futur_level);
 if(dx_out & 1) 
   oddx =(char *)1;
 else
   oddx =NULL;
 
 dy_in=pack->images[img11]->nrow;
 dy_out=mw_bandsize_wpack2d(pack-> img_nrow, futur_level);
 if(dy_out & 1) 
   oddy =(char *)1;
 else
   oddy =NULL;
 
 /*End of : Initialization of parameters */
 /* Reconstruction of the columns : the result is in 'tmp_result' */
 
 inx = mw_change_fsignal(NULL,dx_in);
 if (inx ==NULL )
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 outx = mw_change_fsignal(NULL,dx_out);
 if (outx== NULL ) 
   mwerror(FATAL, 1, "Not enough memory !\n"); 
 
 tmp_result = mw_change_fimage(NULL,2*dy_in,dx_out);
 if (tmp_result == NULL ) 
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 /*first couple of images*/
 for(y=0;y<dy_in;y++){
   for(x=0;x<dx_in;x++)
     inx->values[x]=_(pack->images[img11],x,y);
   
   wpsconvolve(inx, outx, pack->signal2, not_null, NULL,oddx);
   
   for(x=0;x<dx_out;x++)
     _(tmp_result,x,y)=outx->values[x];
   
   for(x=0; x<dx_in ; x++)
     inx->values[x] = _(pack->images[img21],x,y);
   
   wpsconvolve(inx, outx, pack->signal1, not_null, not_null,oddx);
   
   for(x=0;x<dx_out;x++)
     _(tmp_result,x,y)+=outx->values[x];
 }
 /*End of : first couple of images*/
 /*second couple of images*/
 
 for(y=0;y<dy_in;y++){
   for(x=0;x<dx_in;x++)
     inx->values[x]=_(pack->images[img12],x,y);
   
   wpsconvolve(inx, outx, pack->signal2, not_null, NULL,oddx);
   
   for(x=0;x<dx_out;x++)
     _(tmp_result,x,y+dy_in)=outx->values[x];
   
   for(x=0; x<dx_in ; x++)
     inx->values[x] = _(pack->images[img22],x,y);
   
   wpsconvolve(inx, outx, pack->signal1, not_null, not_null,oddx);
   
   for(x=0;x<dx_out;x++)
     _(tmp_result,x,y+dy_in)+=outx->values[x];
 }
 /*End of : second couple of images*/
 /* End of : Reconstruction of the columns : the result is in 'tmp_result' */
 
 /* We erase useless images in the Wpack2d structure */
 mw_delete_fsignal(inx);inx=NULL;
 mw_delete_fsignal(outx);outx=NULL;
 
 mw_delete_fimage(pack->images[img22]);
 pack->images[img22]=NULL;
 mw_delete_fimage(pack->images[img21]);
 pack->images[img21]=NULL;
 mw_delete_fimage(pack->images[img12]);
 pack->images[img12]=NULL;
 
 pack->images[img11]=mw_change_fimage(pack->images[img11],dy_out,dx_out);
 if(pack->images[img11] == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 /* End of erasure */
 /* Reconstruction of the lines : the result is in 'pack->images[img11]' */
 
 iny = mw_change_fsignal(NULL,dy_in);
 if (iny == NULL ) 
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 outy = mw_change_fsignal(NULL,dy_out);
 if (outy == NULL ) 
   mwerror(FATAL, 1, "Not enough memory !\n");
 
 for(x=0;x<dx_out;x++){
   for(y=0;y<dy_in;y++)
     iny->values[y]=_(tmp_result,x,y);
   
   wpsconvolve(iny, outy, pack->signal2, not_null, NULL,oddy);
   
   for(y=0;y<dy_out;y++)
     _(pack->images[img11],x,y)=outy->values[y];
   
   for(y=0;y<dy_in;y++)
     iny->values[y]=_(tmp_result,x,y+dy_in);
   
   wpsconvolve(iny, outy, pack->signal1, not_null, not_null,oddy);
   
   for(y=0;y<dy_out;y++)
     _(pack->images[img11],x,y)+=outy->values[y];
 }
 
 /* End of the reconstruction */
 /* free useless memory */
 
 not_null=NULL;
 oddx=NULL;
 oddy=NULL;
 mw_delete_fsignal(iny);iny=NULL;
 mw_delete_fsignal(outy);outy=NULL;
 mw_delete_fimage(tmp_result);tmp_result=NULL;
 
}
/*-------------------------------------------------------------*/
/*-------------  MAIN               ----------------------------*/
/*-------------------------------------------------------------*/


void wp2dchangepack(pack_in,pack_out,tree_out)
     
Wpack2d pack_in;
Wpack2d pack_out;
Cimage tree_out;
     
{
  /* variable definition */
  
  int x,y;
  int level,pos;
  int futur_level,kx,ky;
  int jump,jump_2,treesize;
  Wpack2d temppack;
  Cimage extended_tree_out;
  
  /* End of : variable definition */
  /* initialization */
  
  level=mw_checktree_wpack2d(tree_out);

  treesize=(tree_out->ncol > pack_in->tree->ncol)?tree_out->ncol:pack_in->tree->ncol;
  
  extended_tree_out=mw_change_cimage(NULL,treesize,treesize);
  if( extended_tree_out == NULL)
    mwerror(FATAL, 1, "Error Not enough memory\n");
  
  temppack=mw_new_wpack2d();
  if (temppack == NULL)
    mwerror(FATAL, 1, "Error Not enough memory\n");
  
  /*--- extention of the tree sothat the two trees have the same size --*/
  mw_copy_wpack2d(pack_in,temppack,treesize);
  
  wp2dchangetree(tree_out,extended_tree_out,NULL,NULL,&treesize,NULL,NULL,NULL);

  /* End of : initialization */
  
  /* decomposition of the pack branches asking for it */
  /* the decomposition goes from smaller levels to larger ones */
  
  for(futur_level=1;futur_level<=level;futur_level++)
    {
      /*jump_2=treesize/power(2,futur_level-1);*/
      jump_2=treesize >> (futur_level-1);
      jump=jump_2/2;
    
    for (kx=0; kx<treesize;kx+=jump_2) for (ky=0; ky<treesize;ky+=jump_2)
      if(_(extended_tree_out,kx,ky)>=futur_level &&   _(temppack->tree,kx,ky)<futur_level  )
	{pos=kx+ky*treesize;
	
	ondel(temppack,pos,pos+jump,pos+jump*treesize,
	      pos+jump*(treesize+1),futur_level);
	
	for(x=0;x<jump_2;x++)for(y=0;y<jump_2;y++)
	  _(temppack->tree,kx+x,ky+y)=futur_level;
	
	}
    }

  /* End of : the decomposition */
  /* recomposition of the pack branches asking for it */
  /* the reconstruction goes from larger levels to smaller ones */
  
  futur_level= mw_checktree_wpack2d(pack_in->tree)-1;
  for(;futur_level>=0;futur_level--)
    {
      /*jump_2=treesize/power(2,futur_level);*/
      jump_2=treesize >> futur_level;
      jump=jump_2/2;
    
    for (kx=0; kx<treesize;kx+=jump_2)for (ky=0; ky<treesize;ky+=jump_2)
      if(_(extended_tree_out,kx,ky)<= futur_level &&   _(temppack->tree,kx,ky)>futur_level  )
	{pos=kx+ky*treesize;
	
	iondel(temppack,pos,pos+jump,pos+jump*treesize,
	       pos+jump*(treesize+1),futur_level);
	
	for(x=0;x<jump_2;x++)for(y=0;y<jump_2;y++)
	  _(temppack->tree,kx+x,ky+y)=futur_level;
	
	}
    }
  /* End of : reconstruction */
  /* End of the wavelet packet computation */

  /* pruning of the tree and the Wpack2d structure sothat it corresponds  to */
  /* the initial size of the 'tree_out'                               */
  mw_prune_wpack2d(temppack,pack_out,tree_out);

  /*free memory*/
  mw_delete_wpack2d(temppack);temppack=NULL;
  mw_delete_cimage(extended_tree_out);extended_tree_out=NULL;
}










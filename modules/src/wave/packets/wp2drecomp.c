/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {wp2drecomp};
version = {"1.0"};
author = {"Francois Malgouyres"};
function = {"Recomposition of a wavelet packet decomposition"};
usage = {
   Wpack2d->pack
     "Input Wpack2d containing the wavelet packet transform of an image",
   Fimage<-F
      "Output Fimage described by the input Wpack2d"
};
*/
/*----------------------------------------------------------------------
 v1.0: adaptation from owave_recomp v0.1 (fpack) (JF)
----------------------------------------------------------------------*/

#include "mw.h"
#include "mw-modules.h" /* for wpsconvolve() */

#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)])

/*-------------------------------------------------------------------------*/
/***************************************************************/
/* Recompose pack->images[img11], pack->images[img12], */
/* pack->images[img21], pack->images[img22]                       */
/* the result is in pack->images[img11] */

static void iondel(Wpack2d pack, int img11, int img12, int img21, int img22, int futur_level)
     
                   /* the initial image corresponds to the index  img11*/
                 /* index of the low in x -low in y image */
                 /* index of the low in x -high in y image */
                 /* index of the high in x -low in y image */
                 /* index of the high in x -high in y image */
                     
     
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
/*------------ MAIN                      -------------------------*/
/*-------------------------------------------------------------*/


void wp2drecomp(Wpack2d pack, Fimage F)
{ 
/* variable definition */
  int futur_level,kx,ky,treesize;
  Wpack2d pack1;
  int pos,jump,jump_2;
  /* End of : variable definition */
  /* initialization */
  
  if (!(pack1=mw_new_wpack2d()))
    mwerror(FATAL, 1, "Not enough memory !\n");       
  mw_copy_wpack2d(pack,pack1,0);
  
  treesize = pack1->tree->ncol;
  /* End of : initialization */
  
  /* reconstruction, from larger levels to smaller ones */
  futur_level=pack1->level-1;
  /*jump_2=treesize/power(2,futur_level);*/
  jump_2=treesize >> futur_level;
  jump=jump_2/2;
  for(;futur_level>=0;futur_level--)
    {
      for(kx=0;kx<treesize;kx+=jump_2)
	for(ky=0;ky<treesize;ky+=jump_2)
	  {
	    pos=kx+ky*pack->tree->ncol;
	    if (pack1->tree->gray[pos] >futur_level)
	      iondel(pack1,pos,pos+jump,pos+treesize*jump,pos+jump+treesize*jump ,futur_level);
	    
	  }
      jump_2*=2;
      jump*=2;
    }
  /* End of : reconstruction */
  /*save the result*/
  
  F=mw_change_fimage(F,pack1->img_nrow,pack1->img_ncol);
  mw_copy_fimage(pack1->images[0],F);
  
  mw_delete_wpack2d(pack1);pack1=NULL;
}



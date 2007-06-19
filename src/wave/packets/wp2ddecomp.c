/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {wp2ddecomp};
version = {"1.0"};
author = {"Francois Malgouyres"};
function = {"Compute the 2D-wavelet packet transform of an image"};
usage = {
   'b':   h_tilde->Ri_biortho
      "impulse response of h_tilde (biorthogonal wavelet)",
   tree->tree 
      "Cimage describing the quad-tree",
   Fimage->A
      "Input Fimage on which 2D-wavelet packet transform has to be computed",
   h->Ri
      "Impulse response of the filter h",
   OutWavePack<-pack
      "Output Wpack2d containing the computed 2D-wavelet packet transform"
};
*/
/*----------------------------------------------------------------------
 v1.0: adaptation from owave_decomp v0.1 (fpack) (JF)
----------------------------------------------------------------------*/

#include "mw.h"

#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)])

/*-------------------------------------------------------------------------*/
/***************************************************************/
/* Decompose pack->images[img11]                                       */
/* the result is in pack->images[img11] , pack->images[img12], */
/* pack->images[img21], pack->images[img22]                       */

void ondel(pack,img11,img12,img21,img22,futur_level)

     Wpack2d pack; /* the initial image corresponds to the index  img11*/
     int img11;  /* index of the low in x -low in y image */
     int img12;  /* index of the low in x -high in y image */
     int img21;  /* index of the high in x -low in y image */
     int img22;  /* index of the high in x -high in y image */
     int futur_level;
     
{ int x,y;
 int dx,dy;
 int dx_new;
 int dy_new;
 Fsignal signal_in,signal_out;
 char band=1;
 Fimage hor1,hor2;
 
 /*Initialization of parameters and memory allocation */
 
 dx_new=mw_bandsize_wpack2d(pack->img_ncol, futur_level);
 dy_new=mw_bandsize_wpack2d(pack->img_nrow, futur_level); 

 dx=pack->images[img11]->ncol;
 dy=pack->images[img11]->nrow;
 
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
   wpsconvolve(signal_in,signal_out,pack->signal2,NULL,&band,NULL);
   
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
   wpsconvolve(signal_in,signal_out,pack->signal2,NULL,&band,NULL);
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
   wpsconvolve(signal_in,signal_out,pack->signal2,NULL,&band,NULL);
   for(x=0;x<dx_new;x++)
     pack->images[img22]->gray[y*dx_new+x]=signal_out->values[x];
 }
 
 /* End of : Convolution of raws */
 /* free useless memory */ 

 mw_delete_fsignal(signal_in);signal_in=NULL;
 mw_delete_fsignal(signal_out);signal_out=NULL;
 mw_delete_fimage(hor1);hor1=NULL;
 mw_delete_fimage(hor2);hor2=NULL;
 
}
/***************************************************************/


/*-------------------------------------------------------------*/
/*-------------  MAIN                ----------------------------*/
/*-------------------------------------------------------------*/


void wp2ddecomp(A,Ri,Ri_biortho,pack,tree)
     
Fimage A;
Cimage tree;
Fsignal Ri,Ri_biortho;
Wpack2d pack;
     
{ 
  /* variable definition */
  
  int futur_level,kx,ky;
  int jump,jump_2;
  
  /* End of : variable definition */
  
  mw_checktree_wpack2d(tree);

  /* memory allocation and initialization */
  
  pack=mw_change_wpack2d(pack, tree, Ri, Ri_biortho, A->nrow,A->ncol);
  if(pack==NULL)
    mwerror(FATAL, 1, "[wp2ddecomp] Not enough memory for wpack2d!\n");
  
  pack->images[0]=mw_change_fimage(pack->images[0] , A->nrow, A->ncol);
  if(pack->images[0]==NULL)
    mwerror(FATAL, 1, "[wp2ddecomp] Not enough memory for fimage!\n");
  
  mw_copy_fimage(A,pack->images[0]);

  /* End of : initialization */
  
  /* decomposition : from smaller levels to larger ones */
  
  for(futur_level=1;futur_level<=pack->level;futur_level++)
    { 
      /*jump = pack->tree->ncol/power(2,futur_level);*/
      jump = pack->tree->ncol >> futur_level;
      jump_2=2*jump;
    
      for(kx = 0 ; kx<pack->tree->ncol ; kx=kx+jump_2)
	for(ky = 0 ; ky<pack->tree->nrow ; ky=ky+jump_2){
	  if(_(tree,kx,ky)>=futur_level)
	    ondel(pack,kx+ky*pack->tree->ncol,
		  kx+jump+ky*pack->tree->ncol,
		  kx+(ky+jump)*pack->tree->ncol,
		  kx+jump+(ky+jump)*pack->tree->ncol,
		  futur_level);
	}
    }
  /* End of : decomposition */
}
   
 


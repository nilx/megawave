/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {lsnakes_demo};
 version = {"1.1"};
 author = {"Francoise Dibos, Jacques Froment, Kamal Lakhiari"};
 function = {"Interactive demo for the Level Set Snakes Model (lsnakes)"};
 usage = {
   'n':[Niter=1]->Niter         "number of iterations between frames",
   'N':[Nframes=10]->Nframes    "number of output frames",
   't':[thre=1.0]->thre         "threshold to binarize mask images",
   'f':[force=0.000001]->force  "force term",
   u->u                         "input Cimage",
   out<-out                     "output Cmovie (optimized contours)"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for readpoly(), fillpolys(), emptypoly()
			 * fmask(), lsnakes() */

void lsnakes_demo(u,out,Niter,Nframes,thre,force)

     Cimage u; /* Original input image */
     Cmovie out; /* Output movie */
     float *force,*thre;
     int *Niter,*Nframes;
{
  Polygons polys; 
  Fimage fimage_org,fimage,fimage_polys0,fimage_polys1; 
  Cimage cimage_polys0,cimage_polys1,cimage0,cimage1;
  int DX = u->ncol;
  int DY = u->nrow;
  int fmaskopt1=255;
  float fmaskopt2=255.0;
  int k;

  out = mw_change_cmovie(out);
  if (out == NULL) mwerror(FATAL,1,"Not enough memory.\n");

  printf("Please select the polygons (initial snakes) on the image using the mouse.\n"); 
  printf("Click on the mouse left button to create a new point in the current coordinates(x,y).\n");
  printf("To remove the last recorded point, click on the mousse middle button.\n");
  printf("To terminate your polygon and to go ready for the next one, click on the mouse right button"); 
  printf("(it links the last recorded point to the first recorded point).\n");
  printf("Then to exit, type 'Q'...\n");
  mwdebug("Calling readpoly...\n");
  if ((polys = (Polygons) readpoly(u,NULL)) == NULL)
    mwerror(FATAL,1,"No polygons selected\n"); 
  
  printf("Computing initial frame #1...\n");

  if ( ((cimage_polys0 = mw_new_cimage()) == NULL)  ||
       (mw_alloc_cimage(cimage_polys0,DY,DX) == NULL)  || 
       ((cimage_polys1 = mw_new_cimage()) == NULL)  ||
       (mw_alloc_cimage(cimage_polys1,DY,DX) == NULL)  ||
       ((cimage0 = mw_new_cimage()) == NULL)  ||
       (mw_alloc_cimage(cimage0,DY,DX) == NULL)  ||
       ((cimage1 = mw_new_cimage()) == NULL)  ||
       (mw_alloc_cimage(cimage1,DY,DX) == NULL)  ||
       ((fimage_org = mw_new_fimage()) == NULL)  ||
       (mw_alloc_fimage(fimage_org,DY,DX) == NULL)  || 
       ((fimage_polys0 = mw_new_fimage()) == NULL)  ||
       (mw_alloc_fimage(fimage_polys0,DY,DX) == NULL)  ||
       ((fimage_polys1 = mw_new_fimage()) == NULL)  ||
       (mw_alloc_fimage(fimage_polys1,DY,DX) == NULL)  ||     
       ((fimage = mw_new_fimage()) == NULL) ||
       (mw_alloc_fimage(fimage,DY,DX) == NULL)) 
    mwerror(FATAL,1,"Not enough memory.\n");    
    
   mwdebug("Calling fillpolys...\n");
   fillpolys(&DX,&DY,polys,cimage_polys0); 
  
   mw_delete_polygons(polys);  
        

  /* First image in the movie */

  mwdebug("Calling emptypoly...\n");
  emptypoly(cimage_polys0,cimage_polys1);
  
  if ((fimage_polys1 = (Fimage) mw_conv_internal_type(cimage_polys1,"cimage","fimage")) == NULL)
    mwerror(FATAL,1,"Not enough memory.\n");
  
  if ((fimage_org = (Fimage) mw_conv_internal_type(u,"cimage","fimage")) == NULL)
    mwerror(FATAL,1,"Not enough memory.\n"); 
  
  mwdebug("Calling fmask...\n");
  fmask(fimage_polys1,fimage_org,NULL,fimage,NULL,&fmaskopt1,&fmaskopt2);  
  
  if ( (cimage0 = (Cimage) mw_conv_internal_type(fimage,"fimage","cimage")) == NULL)
    mwerror(FATAL,1,"Not enough memory.\n"); 
    
  out->first = cimage0;
  
  for (k=2;k<=*Nframes;k++) 
    {   
      printf("Computing frame #%d...\n",k);
            
      if ((fimage_polys0 = (Fimage) mw_conv_internal_type(cimage_polys0,"cimage","fimage")) == NULL)      
        mwerror(FATAL,1,"Not enough memory.\n"); 
            
      mwdebug("Calling lsnakes...\n");      
      lsnakes(fimage_polys0,fimage_org,Niter,thre,force);
       
      if ((cimage_polys0= (Cimage) mw_conv_internal_type(fimage_polys0,"fimage","cimage")) == NULL)
        mwerror(FATAL,1,"Not enough memory.\n"); 
            
      mwdebug("Calling emptypoly...\n");
      emptypoly(cimage_polys0,cimage_polys1);
      
      if ((fimage_polys1 = (Fimage) mw_conv_internal_type(cimage_polys1,"cimage","fimage")) == NULL)
        mwerror(FATAL,1,"Not enough memory.\n");

      mwdebug("Calling fmask...\n");            
      fmask(fimage_polys1,fimage_org,NULL,fimage,NULL,&fmaskopt1,&fmaskopt2);
      
      if ( (cimage1 = (Cimage) mw_conv_internal_type(fimage,"fimage","cimage")) == NULL)
        mwerror(FATAL,1,"Not enough memory.\n"); 
      
      
      cimage0->next = cimage1;
      cimage1->previous = cimage0;
      cimage1->next=NULL;
      cimage0 = cimage1;
	
    }
       
  mw_delete_fimage(fimage_org);
  mw_delete_fimage(fimage); 
  mw_delete_fimage(fimage_polys0);
  mw_delete_fimage(fimage_polys1);
  mw_delete_cimage(cimage_polys0);
  mw_delete_cimage(cimage_polys1);
    
}









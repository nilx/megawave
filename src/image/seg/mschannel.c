/*----------------------Megawave2 Module-----------------------------------*/
/*mwcommand
  name = {mschannel};
  version = {"1.2"};
  author = {"Yann Guyonvarc'h"}; 
  function = {"Build a multi-scales multi-channels decomposition of an image"};
  usage = {
            'N':[N=1]->N "Number of images per channel - involved in the local scale value",
	    'S':[S=1]->S "Sigma - Standard deviation of the smoothing filter",
	    'W':[W=1]->W "Weight of the considered pixel in the smoothing filter",
	    'p':[p=2.0]->p " - 1 for ABS - 2 for Quadratic difference, default 2",
            in->in "input Fimage",
            fmovieout<-fmovieout "output Fmovie"
           };
*/
/*----------------------------------------------------------------------
 v1.2: call to fblur() -> fsepconvol() (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h> 
#include <math.h>
#include "mw.h"

#ifdef __STDC__
extern void fsepconvol(Fimage,Fimage,Fsignal,Fsignal,int*,float*);
#else
extern void fsepconvol();
#endif

void mschannel(N,S,W,p,in,fmovieout)
int *N,*W, *S;
float *p;
Fimage in;
Fmovie fmovieout;

 {

int e;  
Fimage H,V,D;
register short i,j;
Fimage input; 
 
fmovieout=mw_change_fmovie(fmovieout);
if (fmovieout == NULL) mwerror(FATAL,1,"NOT ENOUGH MEMORY.\n");

(*N)=abs(2*(*N)-1);

for (e=1;e<=(*N);e=e+2)

{
   input=mw_change_fimage(NULL,in->ncol,in->nrow);
     
  
  H=mw_change_fimage(NULL,in->ncol,in->nrow); 
  
   if (e!=1) {D->next=H;H->previous=D;}        
  
  V=mw_change_fimage(NULL,in->ncol,in->nrow); 
  D=mw_change_fimage(NULL,in->ncol,in->nrow);
 
   if (e==1) 
     
      { 
       fmovieout->first=H;
       H->previous=NULL;
      }
      
      else {
	fsepconvol(in,input,NULL,NULL,&e,NULL);
	mw_copy_fimage(input,in);
      }
     
      H->next=V;
      V->previous=H;
      V->next=D;
      D->previous=V; 
  

/*-----------------------------Canal horizontal-----------------------------------------*/
 
{  for (j=0;j<(in->nrow);j++)
     for (i=e;i<((in->ncol)-e);i++)
      { mw_plot_fimage(H,i,j,(
pow(fabs(mw_getdot_fimage(in,i-e,j)-mw_getdot_fimage(in,i,j)),*p)+
pow(fabs(mw_getdot_fimage(in,i+e,j)-mw_getdot_fimage(in,i,j)),*p))/2);
     }
}  
 
 { for (j=0;j<(in->nrow);j++)
        for (i=0;i<e;i++)
       {mw_plot_fimage(H,i,j,(
pow(fabs(mw_getdot_fimage(in,i+e,j)-mw_getdot_fimage(in,i,j)),*p)+
pow(fabs(mw_getdot_fimage(in,0,j)-mw_getdot_fimage(in,i,j)),*p))/2);
            }
}  
     

{ for (j=0;j<(in->nrow);j++)
        for (i=((in->ncol)-e);i<(in->ncol);i++)
  
       {mw_plot_fimage(H,i,j,(
pow(fabs(mw_getdot_fimage(in,i-e,j)-mw_getdot_fimage(in,i,j)),*p) +
pow(fabs(mw_getdot_fimage(in,(in->ncol)-1,j)-mw_getdot_fimage(in,i,j)),*p))/2);

        }
}  
 
/*---------------------------------------------------------------------------------------*/



/*-----------------------------Canal Vertical--------------------------------------------*/
 
{  for (i=0;i<(in->ncol);i++)
     for (j=e;j<((in->nrow)-e);j++)
      {     mw_plot_fimage(V,i,j,(
      pow(fabs(mw_getdot_fimage(in,i,j-e)-mw_getdot_fimage(in,i,j)),*p)+
      pow(fabs(mw_getdot_fimage(in,i,j+e)-mw_getdot_fimage(in,i,j)),*p))/2);
     }
} 

 { for (i=0;i<(in->ncol);i++)
        for (j=0;j<e;j++)
  
         {     mw_plot_fimage(V,i,j,(
         pow(fabs(mw_getdot_fimage(in,i,j+e)-mw_getdot_fimage(in,i,j)),*p)+
         pow(fabs(mw_getdot_fimage(in,i,0)-mw_getdot_fimage(in,i,j)),*p))/2);
            }
}  
     

{ for (i=0;i<(in->ncol);i++)
        for (j=((in->nrow)-e);j<(in->nrow);j++)
  
         {     mw_plot_fimage(V,i,j,(
         pow(fabs(mw_getdot_fimage(in,i,j-e)-mw_getdot_fimage(in,i,j)),*p)+
         pow(fabs(mw_getdot_fimage(in,i,(in->nrow)-1)-mw_getdot_fimage(in,i,j)),*p))/2);
            }
}  
 
/*---------------------------------------------------------------------------------------*/ 

/*----------------------------Canal Diagonal---------------------------------------------*/
  
{  for (i=0;i<(in->ncol);i++)
     for (j=0;j<(in->nrow);j++)
      {     mw_plot_fimage(D,i,j,pow(fabs(mw_getdot_fimage(in,abs(i-e),abs(j-e))-mw_getdot_fimage(in,i,j)),*p));
     }
}  

{  for (i=e;i<(in->ncol)-e;i++)
     for (j=e;j<(in->nrow)-e;j++)
      {     mw_plot_fimage(D,i,j,(
      pow(fabs(mw_getdot_fimage(in,(i-e),(j-e))-mw_getdot_fimage(in,i,j)),*p)+
      pow(fabs(mw_getdot_fimage(in,(i+e),(j+e))-mw_getdot_fimage(in,i,j)),*p))/2);
     }
}
 
 


/*----------------------------------------------------------------------------------------*/

 
 
 fsmooth(S,W,H,H);
 fsmooth(S,W,V,V);
 fsmooth(S,W,D,D);
 
 
}
   
   
   D->next=NULL;
   
    

}


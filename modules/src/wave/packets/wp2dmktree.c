/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {wp2dmktree};
version = {"1.0"};
author = {"Francois Malgouyres"};
function = {"Create a 2D-wavelet packet quad-tree"};
usage = {
   'q'->quinc_flag   "create a quincunx tree",  
   's'->sinc_flag    "create a sinc tree", 
   'w'->wave_flag    "create a wavelet tree",
   'f'->full_flag    "create a fully decomposed tree",
   'm'->mirror_flag  "create a mirror tree",
   level->level      "maximum decomposition level",
   tree<-tree        "Cimage describing the output tree"
};
*/
/*----------------------------------------------------------------------
 v1.0: adaptation from mktree v.0.1 (fpack) (JF)
----------------------------------------------------------------------*/


#include "mw.h"
#include "mw-modules.h" /* for wp2dfreqorder() */

#define _(a,i,j) ((a)->gray[(j)*(a)->ncol+(i)])

/*-------------------------------------------------------------*/
/*--------------MAIN PROGRAM-----------------------*/
/*-------------------------------------------------------------*/
/******** Creates a tree ***********/
void wp2dmktree(level,tree,sinc_flag,quinc_flag,wave_flag,full_flag,mirror_flag)
     
     Cimage tree;
     int level;
     char *sinc_flag,*quinc_flag,*wave_flag,*full_flag,*mirror_flag;

{int i,tree_size,kx,ky,x,y,x1,y1,x2,y2;
 int tw,k,rap; 
 int nbOptions;
 Fsignal order;
 char * not_null;

 not_null=(char *) 1;

 for(i=1,tree_size=1;i<=level;i++)
   tree_size*=2;
 
 if  ((tree = mw_change_cimage(tree,tree_size,tree_size)) == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");

 if (((order = mw_new_fsignal()) == NULL ) ||
     ( order = mw_change_fsignal(order,tree_size)) == NULL)
   mwerror(FATAL, 1, "Not enough memory !\n");
  
 /* test entries */

 nbOptions=0;
 if(sinc_flag) nbOptions++;
 if(quinc_flag) nbOptions++;
 if(wave_flag) nbOptions++;
 if(full_flag) nbOptions++;
 if(mirror_flag) nbOptions++;
 if(nbOptions>1)  mwerror(FATAL, 1, "One and only one tree type has to be selected !\n");
 else if(nbOptions==0)  mwerror(FATAL, 1, "One and only one tree type has to be selected !\n");

 else if(quinc_flag) /* tree for quincunx images */
   {wp2dfreqorder(level,order,not_null);

   for(x=0;x<tree_size;x++) for(y=0;y<tree_size;y++)
     _(tree,x,y)=1;
   
   for(k=0,tw=tree_size/2,rap=2;k<level-1;k++)
     {for(kx=0;kx<rap;kx++) for(ky=0;ky<rap;ky++)
       if( kx+ky == rap-1)
	 for(x=0;x<tw;x++) for(y=0;y<tw;y++)
	   {x1=kx*tw+x;
	   y1=ky*tw+y;
	   x2=(int) order->values[x1]-1;
	   y2=(int)order->values[y1]-1;
	   _(tree,x2,y2)=k+2;
	   }
       
     tw/=2;
     rap*=2;
     }
   }

 else if(sinc_flag) /* tree for sinc deconvolution*/
   {wp2dfreqorder(level,order,not_null);

   for(x=0;x<tree_size;x++) for(y=0;y<tree_size;y++)
     _(tree,x,y)=1;
   
   for(k=0,tw=tree_size/2,rap=2;k<level-1;k++)
     {for(kx=0;kx<rap;kx++) for(ky=0;ky<rap;ky++)
       if((kx==0 && ky==0) || kx==rap-1 || ky==rap-1 
	   || 2*kx==rap || 2*kx+2==rap || 2*ky ==rap || 2*ky+2==rap )
	 for(x=0;x<tw;x++) for(y=0;y<tw;y++)
	      {x1=kx*tw+x;
	      y1=ky*tw+y;
	      x2=(int) order->values[x1]-1;
	      y2=(int)order->values[y1]-1;
	      _(tree,x2,y2)=k+2;
	      }
     
     tw/=2;
     rap*=2;
     }
   }

else if( wave_flag) /* wavelet tree */
  {/* We do not need to reorder the coefficient for a wavelet basis */

     for(x=0;x<tree_size;x++) for(y=0;y<tree_size;y++)
       _(tree,x,y)=1;
   
   for(k=0,tw=tree_size/2,rap=2;k<level-1;k++)
     {for(kx=0;kx<rap;kx++) for(ky=0;ky<rap;ky++)
       if(kx==0 && ky==0)
	 for(x=0;x<tw;x++) for(y=0;y<tw;y++)
	   _(tree,kx*tw+x,ky*tw+y)=k+2;

     
     tw/=2;
     rap*=2;
     }
   }
 
else if( full_flag) /* fully decomposed tree */
   for(x=0;x<tree_size;x++) for(y=0;y<tree_size;y++)
     /* We do not need to reorder the coefficient for a  fully decomposed tree*/
     _(tree,x,y)=level;
 
 else if(mirror_flag) /* anti-wavelet tree */
   {wp2dfreqorder(level,order,not_null);

   for(x=0;x<tree_size;x++) for(y=0;y<tree_size;y++)
     _(tree,x,y)=1;
   for(k=0,tw=tree_size/2,rap=2;k<level-1;k++)
     {for(kx=0;kx<rap;kx++) for(ky=0;ky<rap;ky++)
       if( kx==rap-1 || ky==rap-1 )
	 for(x=0;x<tw;x++) for(y=0;y<tw;y++)
	   {x1=kx*tw+x;
	   y1=ky*tw+y;
	   x2=(int) order->values[x1]-1;
	   y2=(int)order->values[y1]-1;
	   _(tree,x2,y2)=k+2;
	   }

     tw/=2;
     rap*=2;
     }
   } 

 /***************************/
 /* A particular case: level=0*/

 if(level==0)
   tree->gray[0]=0;

 /*free memory*/
 not_null=NULL;
 mw_delete_fsignal(order);

/*end of the program*/
}













































































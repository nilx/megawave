/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {cmextract};
  version = {"1.01"};
  author = {"Jacques Froment, Lionel Moisan"};
  function = {"Copy a part of a cmovie into another movie, in an optional background"};
  usage = {
       in->in          "input cmovie",
       out<-out        "output cmovie",
       Xo->X1          "X Coor. of upper left point of submovie in original movie",
       Yo->Y1          "Y Coor. of upper left point of submovie in original movie",
       To->T1          "Position of the first image of submovie in original movie",
       Xf->X2          "X Coor. of lower right point of submovie in original movie",              
       Yf->Y2          "Y Coor. of lower right point of submovie in original movie",
       Tf->T2          "Position of the last image of submovie in original movie",
       {
       Background->bg  "Background input movie",
       [Xc=0]->Xc "X Coor. of the upper left point in the background where to put the submovie",
       [Yc=0]->Yc "Y Coor. of the upper left point in the background where to put the submovie",
       [Tc=1]->Tc "Position of the image in the background where to put the submovie"
       }
       };
*/
/*-- MegaWave2 - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"


#define MAX(a,b) ((a)>(b)?(a):(b))

void cmextract(in,out,X1,Y1,T1,X2,Y2,T2,bg,Xc,Yc,Tc)
Cmovie in,out,bg;
int    X1,Y1,T1,X2,Y2,T2,*Xc,*Yc,*Tc;
{
  Cimage u,prev,source,back;
  int x1,y1,t1,sx,sy,st,k,Xcc,Ycc;
    
  if ((X1<0) || (Y1<0) || (T1 < 1) || (*Tc < 1) || (X2 < X1) || (Y2 < Y1) || 
      (T2 < T1) || (in->first->ncol < X2) || (in->first->nrow < Y2))
    mwerror(USAGE,1,"Illegal coordinates or positions\n");
      
  sx = *Xc+X2-X1+1;
  sy = *Yc+Y2-Y1+1;
  st = *Tc+T2-T1+1;

  Xcc = Ycc = 0;
  if (bg) {
    back = bg->first;
    if (back) {
      sx = MAX(back->ncol,sx);
      sy = MAX(back->nrow,sy);
    }
  } else back = NULL;

  source = in->first;
  k = T1; 
  while (k-- && source) source=source->next;

  prev = NULL;
  for (k=1;(k<=st) || back;k++) 
    {
      mwdebug("Output image #%d\n",k);
      u = mw_change_cimage(NULL,sy,sx);
      if (!u) mwerror(FATAL,1,"Not enough memory.\n");
      if (source && k>=*Tc)
	{
	  mwdebug("Extract source #%d\n",T1+k-(*Tc));
	  cextract(NULL,source,back,u,X1,Y1,X2,Y2,Xc,Yc);
	  source = source->next;
	}
      else if (back) 
	cextract(NULL,back,back,u,0,0,back->ncol-1,back->nrow-1,&Xcc,&Ycc);
	
      if (prev) prev->next = u; else out->first = u;
      u->previous = prev;
      prev = u;
      if (back) back = back->next;
    }
  u->next = NULL;
  
  printf("Size of output cmovie : %dx%dx%d\n",sx,sy,k-1);
}




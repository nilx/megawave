/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {make_fmovie};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Make a very simple fmovie (as a demo of fmovie use)"};
  usage = {
  's':[size=256]->size "size of each square image",
  fmovie_out<-Output   "output movie"
  };
*/
/*--- MegaWave - Copyright (C) 1992 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

Fmovie make_fmovie(size,Output)

int *size;
Fmovie Output;

{
  short dx,dy,l,x,y;
  Fimage Image,Image_prev;

  Output = mw_change_fmovie(Output);
  if (Output == NULL) mwerror(FATAL,1,"Not enough memory.");

  dx = dy = *size;
  Image_prev = Output->first;
  for (l=1;l< *size-4;l+= *size/20)
    {
      Image = mw_new_fimage();
      if (mw_change_fimage(Image,dy,dx) == NULL)  
	mwerror(FATAL,1,"Not enough memory.");
      if (l == 1) Output->first = Image;
      else 
	{
	  Image_prev->next = Image;
	  Image->previous = Image_prev;
	}
      
      Image_prev = Image;

      mw_clear_fimage(Image,0.0);
      for (x=l;x<=l+4;x++) for(y=l;y<=l+4;y++) 
	Image->gray[(long)dx*y+x] = 255;
    }

  Image->next=NULL;
  (Output->first)->previous = NULL;
}




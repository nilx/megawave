/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {make_cfmovie};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"Make a very simple color movie (as a demo of movie use)"};
  usage = {
  's':[size=256]->size "size of each square image",
  cfmovie_out<-Output   "output movie"
  };
*/
/*--- MegaWave - Copyright (C) 1992 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"

Cfmovie make_cfmovie(size,Output)

int *size;
Cfmovie Output;

{
  short dx,dy,l,x,y;
  unsigned char r,g,b;
  int i;
  Cfimage Image,Image_prev;

  Output = mw_change_cfmovie(Output);
  if (Output == NULL) mwerror(FATAL,1,"Not enough memory.");

  dx = dy = *size;
  Image_prev = Output->first;
  for (l=1;l< *size-4;l+= *size/19)
    {
      Image = mw_new_cfimage();
      if (mw_change_cfimage(Image,dy,dx) == NULL)  
	mwerror(FATAL,1,"Not enough memory.");
      if (l == 1) Output->first = Image;
      else 
	{
	  Image_prev->next = Image;
	  Image->previous = Image_prev;
	}
      
      Image_prev = Image;

      mw_clear_cfimage(Image, 0.0, 0.0, 0.0);
      r = 255 - l;
      g = 127 + l;
      b = l;
      for (x=l;x<=l+4;x++) for(y=l;y<=l+4;y++) 
	{
	  i = (int) dx*y+x;
	  Image->red[i] = r;
	  Image->green[i] = g;
	  Image->blue[i] = b;
	}
    }

  Image->next=NULL;
  (Output->first)->previous = NULL;
}




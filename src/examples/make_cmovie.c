/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {make_cmovie};
  version = {"1.01"};
  author = {"Jacques Froment"};
  function = {"Make a very simple movie (as a demo of movie use)"};
  usage = {
      's':[size=256]->size   "size of each square image",
      cmovie_out<-Output     "output movie"
  };
*/

#include <stdio.h>
#include "mw.h"

Cmovie make_cmovie(size,Output)
     
     int *size;
     Cmovie Output;
     
{
  short dx,dy,l,x,y;
  Cimage Image,Image_prev;

  Output = mw_change_cmovie(Output);
  if (Output == NULL) mwerror(FATAL,1,"Not enough memory.");

  dx = dy = *size;
  Image_prev = Output->first;
  for (l=1;l< *size-4;l+= *size/19)
    {
      Image = mw_new_cimage();
      if (mw_change_cimage(Image,dy,dx) == NULL)  
	mwerror(FATAL,1,"Not enough memory.");
      if (l == 1) Output->first = Image;
      else 
	{
	  Image_prev->next = Image;
	  Image->previous = Image_prev;
	}
      
      Image_prev = Image;

      mw_clear_cimage(Image,0);
      for (x=l;x<=l+4;x++) for(y=l;y<=l+4;y++) 
	Image->gray[(long)dx*y+x] = 255;
    }

  Image->next=NULL;
  (Output->first)->previous = NULL;
}




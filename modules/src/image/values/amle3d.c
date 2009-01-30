/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {amle3d};
 version = {"1.1"};
 author = {"Frederic Cao"};
 function = {"Compute the solution of the AMLE Model using the inf sup scheme in the 3d case"};
 usage = {
   'n':[num=100]->num     "Number of iterations",
   'i':movie_init->init   "Initial condition",
   in->in                 "Input quantized movie obtained from the amle3d_init procedure",
   out<-out               "Output fmovie solution of the amle equation"
};
*/
/*----------------------------------------------------------------------
 v1.1: changed createmovie() + amle3d returns void (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

/*create a movie with value 0.0 with the same number of images as the input*/

static void createmovie(Fmovie input, Fmovie output, int nl, int nc)
{
  Fimage u,cur,prev,*next;

  next = &(output->first);
  prev = NULL;
  for (u=input->first;u;u=u->next) {
    cur = mw_change_fimage(NULL,nl,nc);
    if (!cur) mwerror(FATAL,1,"Not enough memory.\n");
    cur->previous = prev;
    prev = *next = cur;
    next = &(cur->next);
  }
  *next = NULL;
}      


/*------------------ compute the value of the 4 neighbours of a pixel in the same image . 
                           If the pixel is at a border, a mirror effect is applied  ------*/

static void neighbor_4(register int x, register int y, register int xmax, register int ymax, register float *p, float **left, float **right, float **up, float **down)
{
  if (x>0)
    {
      *left = p-1;
      if (x < xmax)
        {
          *right = p+1;
          if (y>0)
            {
              *up =p-xmax-1;
              if (y < ymax)
                /* 0 < x < xmax  0 < y < ymax */
		*down=p+xmax+1;
              else /* 0 < x < xmax   y = ymax */
		*down=*up;
            }
          else /* 0 < x < xmax   y = 0 */
            {
              *down= p+xmax+1;
              *up=*down;
            }
        }
      else /* x = xmax */
        {
          *right=*left;
          if (y>0)
            {
              *up=p-xmax-1;
              if (y < ymax)
		/* x = xmax  0 < y < ymax */
		*down=p+xmax+1;
              else /* x = xmax   y = ymax */
		*down=*up;
            }
          else /* x = xmax  y = 0 */
            {
              *down=p+xmax+1;
              *up=*down;
            }
        }
    }
  else /* x = 0 */
    {
      *right=p+1;
      *left=*right;
      if (y>0)
        {
          *up=p-xmax-1;
          if (y < ymax)
	    /* x = 0  0 < y < ymax */
	    *down=p+xmax+1;
          else /* x = 0   y = ymax */
	    *down=*up;
        }
      else /* x = 0   y = 0 */
        {
          *down=p+xmax+1;
          *up=*down;
        }
    }
}


/*-------- compute an iteration of the AMLE model : 
we only change the pixels with value equal to zero in the init movie, 
compute the max and min in the 6 (3d) neighborhood of the pixel, 
then replace the value by 0.5(max+min).*/


static void iterate(Fmovie input, Fmovie output, int nl, int nc)
{
  Fmovie mov,del;
  Fimage iminput,ima,im,imoutput;
  int x,y;
  float *I,*O,*ptrmov,*left,*right,*up,*down,previ,nex,Max,Min;
  
  iminput=input->first;
  imoutput=output->first;
  mov=mw_new_fmovie();
  if (mov == NULL) mwerror(FATAL,1,"Not enough memory.\n");
  ima=mw_new_fimage();
  mov->first=ima;
  while (iminput != NULL)
    {
      I=iminput->gray;
      O=imoutput->gray;
      ima=mw_change_fimage(ima,nl,nc);
      if (ima == NULL) mwerror(FATAL,1,"Not enough memory.\n");
      mw_clear_fimage(ima,0.0);
      ptrmov=ima->gray;
      for (y=0;y<nl;y++)
	for(x=0;x<nc;x++,O++,I++,ptrmov++)
	  {
	    if (*I==0)
		{
		  neighbor_4(x,y,nc-1,nl-1,O,&left,&right,&up,&down);
		  if 
		    (imoutput->previous != NULL) previ=mw_getdot_fimage(imoutput->previous,x,y);
		  else
		    previ=mw_getdot_fimage(imoutput->next,x,y);
		  if 
		    (imoutput->next != NULL) nex=mw_getdot_fimage(imoutput->next,x,y);
		  else
		    nex=mw_getdot_fimage(imoutput->previous,x,y);
		  /*if im is the first or last image, apply a mirror effect */
		  Max = *O;
		  Min=*O;
		  if (*left > Max) Max=*left;
		  if (*right > Max) Max=*right;
		  if (*up > Max) Max=*up;
		  if (*down > Max) Max=*down;
		  if (previ > Max) Max=previ;
		  if (nex > Max) Max=nex;
		  if (*left < Min) Min=*left;
		  if (*right < Min) Min=*right;
		  if (*up < Min) Min=*up;
		  if (*down < Min) Min=*down;
		  if (previ < Min) Min=previ;
		  if (nex < Min) Min=nex;
		  *ptrmov=0.5*(Min+Max);
		}
	    else
	      *ptrmov=*I;
	  }
      im=mw_new_fimage();
      im->previous=ima;
      ima->next=im;
      ima=im;
      iminput=iminput->next;
      imoutput=imoutput->next;
    }
  ima->previous->next=NULL;
  del=mw_new_fmovie();
  del->first=output->first;
  output->first=mov->first;
  mw_delete_fmovie(del);
}


static void copymovie(Fmovie input, Fmovie output, int nl, int nc)
{
  Fimage imin,imout,im;
  float *I,*O;
  int x,y;

  imin=input->first;
  imout=mw_new_fimage();
  output->first=imout;
  while (imin != NULL)
    {
      imout=mw_change_fimage(imout,nl,nc);
      I=imin->gray;
      O=imout->gray;
      if (imout == NULL) mwerror(FATAL,1,"Not enough memory.\n");
      for(y=0;y<nl;y++)
	for(x=0;x<nc;x++,I++,O++)
	  *O=*I;
      im=mw_new_fimage();
      im->previous=imout;
      imout->next=im;
      imout=im;
      imin=imin->next;
    }
  imout->previous->next=NULL;
}
  
  





/*-----------------------------  Main Program --------------------------*/

void amle3d(int *num, Fmovie init, Fmovie in, Fmovie out)
{
  int NC,NL,i;
  
  if (in==NULL) mwerror(FATAL,1,"Null input fmovie.\n");
  NL=in->first->nrow;
  NC=in->first->ncol;
 
  out=mw_change_fmovie(out);
  if (out == NULL) mwerror(FATAL,1,"Not enough memory.\n");
  if (init) copymovie(init,out,NL,NC); /*copy initial condition in output*/
  else
    createmovie(in,out,NL,NC); /*create a movie with value 0.0 with the same number of images as the input*/
  for(i=0;i<*num;i++)
    iterate(in,out,NL,NC);
}
      
  

  
 


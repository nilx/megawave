/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {ccmview};
  version = {"1.04"};
  author = {"Jacques Froment"};
  function = {"View a ccmovie on a window"};
  usage = {
  'x':[pos_x=50]->x0
     "X coordinate for the upper-left corner of the Window",
  'y':[pos_y=50]->y0
     "Y coordinate for the upper-left corner of the Window",
 'z':[zoom=1.0]->zoom
      "Zoom factor (float value)",
  'l'->loop
      "Flag to loop on the sequence",
  ccmovie->input 
     "Input movie (should be a ccmovie)",
  notused->window 
      "Window to display the movie (internal use)"
  };
*/
/*--- MegaWave - Copyright (C) 1994 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>

/* Include always the MegaWave2 include file */
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"

#ifdef __STDC___

void cmzoom(Cmovie, Cmovie, char *, char *, float *);

#else

void cmzoom();

#endif

typedef struct ccmview_SParam {
   Ccimage image_work;
   Ccimage first_image;
   Ccimage last_image;
   int *loop_flag;
   }  *ccmview_Param;

char step_mode=0,direction=1;
unsigned char InfoPrint; /* Toggle to print info on the current image */
int CurrentFrameNumber;  /* Current number of the frame */
int FrameNumber;         /* Total number of frames */

void ccmview_notify_help()

{
  printf("\n\t\tHelp on line\n");

  printf("\nMouse:\n");
  printf("\tLeft button: Toggle to print information on the current frame.\n");

  printf("\nKeyboard:\n");
  printf("\tF: Play forward or show next frame in the step by step mode.\n");
  printf("\tB: Play backward or show previous frame in the step by step mode.\n");
  printf("\t+: Play faster.\n");
  printf("\t-: Play slower.\n");
  printf("\t<space>: Toggle step by step mode.\n");
  printf("\tQ: Quit.\n");
  printf("\tH: Help.\n");
}

/*     A notify function must return a value ....      */
/*       0 if there was no event catched               */
/*     > 0 if there was an event catched (but Destroy) */
/*      -1 if the event Destroy was catched (or 'Q')   */

int ccmview_notify(ImageWindow,param)

Wframe *ImageWindow;
void *param;          /* Users's parameters: don't forget the cast ! */

{
  ccmview_Param data;
  Ccimage image;
  Ccimage first,last;
  int *loop;
  int event,ret;
  char c,go;
  char mess[mw_cmtsize+10];

  data = (ccmview_Param) param;
  image = (Ccimage) data->image_work;
  first = (Ccimage) data->first_image;
  last = (Ccimage) data->last_image;
  loop = (int *) data->loop_flag;
  
  ret = 0;
  go = 0;
  if (image == NULL) 
  	{
  	  if (loop) 
  	  	{
		  if (direction==1) 
		    {
		      image = first;
		      CurrentFrameNumber = 1;
		    }
		  else 
		    {
		      image = last;
		      CurrentFrameNumber = FrameNumber;
		    }
		}
  	  else	ret =-1;
	}
  	 
  if (ret != -1)
    {
      event = WUserEvent(ImageWindow); /* User's event on ImageWindow */
      if (event < 0) ret=1; else ret=event;
      switch(event)
	{
	case W_DESTROY:
	  ret=-1;
	  break;
	case W_KEYPRESS:
	  c = (char) WGetKeyboard();
	  switch(c)
	    {
	    case 'q': case 'Q': ret=-1;
	      break;
	    case 'h': case 'H': ccmview_notify_help();
	      break;
	    case ' ':
	      step_mode = 1 - step_mode;
	      break;
	    case 'f': case 'F':
	      direction = 1; go=1;
	      break;
	    case 'b': case 'B':
	      direction = -1; go=1;	  
	      break;
	    case '+':
	      mwwindelay-=10;	      
	      if (mwwindelay < 0) mwwindelay=0;
	      break;
	    case '-':
	      mwwindelay+=10;	      
	      if (mwwindelay > 1000) mwwindelay=1000;
	      break;
	    }
	  break;
	case W_MS_LEFT:
	  InfoPrint = 1 - InfoPrint;
	  break;  
	}
    }

  if (ret != -1)
    {
      WLoadBitMapColorImage(ImageWindow,image->red,image->green,
			    image->blue,image->ncol,image->nrow); 
      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
      if (InfoPrint == 1) 
	{
	  sprintf(mess,"#%d  %s",
		  CurrentFrameNumber,image->cmt);
	  WDrawString(ImageWindow,0,10,mess);
	}
      WFlushWindow(ImageWindow);

      if ((step_mode == 0) || (go == 1))
	{
	  if (direction == 1) 
	    {
	      image = image->next;
	      CurrentFrameNumber++;
	    }
	  else 
	    {
	      image = image->previous;
	      CurrentFrameNumber--;
	    }
	}
    }
  else free(param);
  data->image_work=image;
  return(ret);
}


ccmview(input,x0,y0,zoom,loop,window)

Ccmovie input;
int *x0,*y0,*loop;
float *zoom;
char *window;

{
  Ccimage image,fst_image,lst_image;
  Wframe *ImageWindow;
  Ccmovie movie=NULL;
  ccmview_Param param;
  char text[BUFSIZ];

  mwwindelay=100;

  if (*zoom != 1.0) 
    {
      movie = mw_change_ccmovie(NULL);
      if (movie == NULL) mwerror(FATAL,1,"Not enough memory\n");
      ccmzoom(input,movie,NULL,NULL,zoom);
      sprintf(text,"%s (%.1fX)",input->name,*zoom);
    }
  else 
    {
      movie=input;
      strcpy(text,input->name);
    }

  image = fst_image = lst_image = movie->first;
  FrameNumber = 1;
  while (lst_image->next)
    {
      lst_image = lst_image->next;
      FrameNumber++;
    }

  ImageWindow = (Wframe *)
    mw_get_window((Wframe *) window,image->ncol,image->nrow,*x0,*y0,
		  text);

  if (ImageWindow == NULL)
    mwerror(INTERNAL,1,"NULL window returned by mw_getwindow\n");

  param = (ccmview_Param) malloc(sizeof(struct ccmview_SParam));
  if (param == NULL) mwerror(FATAL,1,"not enough memory\n");

  WSetUserEvent(ImageWindow,W_MS_BUTTON | W_KEYPRESS);

  WLoadBitMapColorImage(ImageWindow,image->red,image->green,image->blue,
			image->ncol,image->nrow); 
  WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
  WFlushWindow(ImageWindow);
  
  InfoPrint = 0;

  param->image_work = image;
  param->first_image = fst_image;
  param->last_image = lst_image;
  param->loop_flag = loop;
  
  CurrentFrameNumber = 1;

  mw_window_notify(ImageWindow,(void *)param,ccmview_notify);
  mw_window_main_loop();
}








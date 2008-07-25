/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {ccmview};
 version = {"1.3"};
 author = {"Jacques Froment"};
 function = {"View a ccmovie on a window"};
 usage = {
  'x':[pos_x=50]->x0    "upper-left corner of the Window (X coordinate)",
  'y':[pos_y=50]->y0    "upper-left corner of the Window (Y coordinate)",
  'z':[zoom=1.0]->zoom  "Zoom factor (float value)",
  'o':[order=0]->order  "Zoom order: 0,1=linear,-3=cubic,3,5..11=spline",
  'l'->loop             "Flag to loop on the sequence",
  'p'->pause            "pause the movie on start",
  ccmovie->input        "Input movie (should be a ccmovie)",
  notused->window       "Window to display the movie (internal use)"
};
*/
/*----------------------------------------------------------------------
 v1.06: added -p (pause) option and return void (L.Moisan)
 v1.1: added -o option + several minor modifications (L.Moisan)
 v1.2: fixed bug with non char input keys (L.Moisan)
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"

/* Include the window since we use windows facility */
#include "window.h"

extern void cmzoom();

typedef struct ccmview_SParam {
   Ccimage image_work;
   Ccimage first_image;
   Ccimage last_image;
   int *loop_flag;
   }  *ccmview_Param;

static char step_mode,direction=1;
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
  int event,ret,c;
  char go;
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
	  c = WGetKeyboard();
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


void ccmview(input,x0,y0,zoom,order,loop,pause,window)

Ccmovie input;
int *x0,*y0,*loop,*pause,*order;
float *zoom;
char *window;

{
  Ccimage image,fst_image,lst_image;
  Wframe *ImageWindow;
  Ccmovie movie=NULL;
  ccmview_Param param;
  char text[BUFSIZ];
  float inverse_zoom;

  mwwindelay=100;

  step_mode = (pause?1:0);

  if (*zoom != 1.0) 
    {
      movie = mw_change_ccmovie(NULL);
      if (movie == NULL) mwerror(FATAL,1,"Not enough memory\n");
      if (*zoom>1.0) 
	ccmzoom(input,movie,NULL,NULL,zoom,order,NULL);
      else {
	inverse_zoom = 1./(*zoom);
	ccmzoom(input,movie,NULL,NULL,&inverse_zoom,order,(char *)1);
      }
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








/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {readpoly};
  version = {"1.2"};
  author = {"Jacques Froment"};
  function = {"Read the contours of Polygons on a window"};
  usage = {
     cimage->image       "input Cimage",
     polygons<-readpoly  "output Polygons",
     notused->window     "Window to view the image (internal use)"
};
*/

#include <stdio.h>
#include <stdlib.h>
#include "mw.h"

/* Number of channels for the polygon: 1 (gray-level) */
#define NB_OF_CHANNELS 1

Polygons poly;
Polygon pl,npl;
Point_curve pc,npc;

int x0,y0,x1,y1,oldx1,oldy1,dx,dy;
static char first_point=0;


static void DrawLinePoly(ImageWindow,a0,b0,a1,b1)


Wframe *ImageWindow;
int a0,b0,a1,b1;

{
  WSetTypePencil(W_COPY);

  WSetSpecialColorPencil(ImageWindow);

  if (abs(a0-a1) >= abs(b0-b1))
    {
      WDrawLine(ImageWindow,a0,b0+1,a1,b1+1);	  
      WDrawLine(ImageWindow,a0,b0-1,a1,b1-1);	  
    }
  else
    {
      WDrawLine(ImageWindow,a0+1,b0,a1+1,b1);	  
      WDrawLine(ImageWindow,a0-1,b0,a1-1,b1);	  
    }


  WSetColorPencil(ImageWindow,127); 
  WDrawLine(ImageWindow,a0,b0,a1,b1);  

  WSetColorPencil(ImageWindow,0);
}

static void List_of_Poly()

{
  Polygon p;
  Point_curve c;
  int i;

  printf("\t\tList of recorded polygons:\n");

  for (p=poly->first,i =1; p; p=p->next, i++)
    {
      printf("\tPoly #%d\n",i);
      for (c=p->first; c; c=c->next)
	printf("\t\t(%d,%d)\n",c->x,c->y);
    }
  printf("\n");
}


static void Help()

{
  printf("\n\t\tHelp on line\n");

  printf("\nMouse:\n");
  printf("\tLeft button: Create a new point in the current coordinates (x,y).\n");
  printf("\tMiddle button: Remove the last recorded point.\n");
  printf("\tRight button: Terminate the polygon. Go ready for the next one.\n");

  printf("\nKeyboard:\n");
  printf("\tQ: Soft Quit (save the polygons).\n");
  printf("\tH: Help.\n");
  printf("\t<SPACE>: List of recorded polygons.\n");
  printf("\tR: Redraw the window according to the recorded polygons.\n");
}


static void Redraw(ImageWindow)

Wframe *ImageWindow;

{
  Polygon p;
  Point_curve c;
  int a0,b0,a1,b1;

  WSetTypePencil(W_COPY);
  WRestoreImageWindow(ImageWindow,0,0,dx,dy); 

  for (p=poly->first; p; p=p->next)
      for (c=p->first; c && (c->x != -1); c=c->next)
	{
	  if (c == p->first) {a0 = c->x; b0 = c->y;}
	  else 
	    {
	      a1 = c->x; b1 = c->y;
	      DrawLinePoly(ImageWindow,a0,b0,a1,b1);	  
	      a0 = a1; b0 = b1;
	      if (!c->next) 
		DrawLinePoly(ImageWindow,(p->first)->x,(p->first)->y,a0,b0);
	    }
	}

  WFlushWindow(ImageWindow);
}

/*     A notify function must return a value ....      */
/*       0 if there was no event catched               */
/*     > 0 if there was an event catched (but Destroy) */
/*      -1 if the event Destroy was catched (or 'Q')   */

static int readpoly_notify(ImageWindow)

Wframe *ImageWindow;

{
  int ret;
  int event,button_mask;
  
  oldx1=x1; oldy1=y1;

  event = WUserEvent(ImageWindow); /* User's event on ImageWindow */
  if (event < 0) ret=1; else ret=event;
  if (event != W_DESTROY)
    {
      WGetStateMouse(ImageWindow,&x1,&y1,&button_mask);
      if ((first_point != 0) && ((x1 != oldx1) || (y1 != oldy1)))
	{
	  WSetTypePencil(W_XOR); 
	  WDrawLine(ImageWindow,x0,y0,oldx1,oldy1);
	  WDrawLine(ImageWindow,x0,y0,x1,y1);
	  WSetTypePencil(W_COPY);
	  WFlushWindow(ImageWindow);
	}
    }

  switch(event)
    {
    case W_MS_LEFT:
      /* New point */
      if (first_point == 0) first_point=1;
      
      if ((pc->previous) && ((pc->previous)->x == x1) &&
	  ((pc->previous)->y == y1))
	mwerror(WARNING, 0, "Double point in (%d,%d) :  not created\n",x1,y1);
      else
	{
	  pc->x = x1; pc->y = y1;
	  x0=x1; y0=y1;
	  npc = mw_new_point_curve();
	  if (npc == NULL) mwerror(FATAL,0,"Not enough memory\n");
	  pc->next = npc;
	  npc->previous = pc;
	  pc = npc;

	  printf("Created a point in (%d,%d)\n",x1,y1);
	}

      break;
      
    case W_MS_MIDDLE: 
      /* Cancel */

      npc = pc->previous;
      if (npc != NULL)
	{
	  printf("Cancel point in (%d,%d)\n",npc->x,npc->y);
	  WSetTypePencil(W_XOR); 
	  WDrawLine(ImageWindow,x0,y0,oldx1,oldy1); 
	  if (npc->previous)
	    {
	      x0 = (npc->previous)->x; y0 = (npc->previous)->y;
	      x1 = npc->x; y1 = npc->y;
	    }
	  else first_point = 0;
	  
	  WSetTypePencil(W_COPY);
	  WFlushWindow(ImageWindow);      
	  free(pc);
	  pc = npc;
	  pc->next = NULL;
	  pc->x = pc->y = -1;
	}
      else mwerror(WARNING, 0, "Polygon completed: cannot delete\n");
      break;

    case W_MS_RIGHT:
      /* Next polygon */

      WSetTypePencil(W_XOR); 
      if (pc->x != -1)
	{
	  WDrawLine(ImageWindow,(pl->first)->x,(pl->first)->y,pc->x,pc->y);
	  pc = mw_new_point_curve();
	  if (pc == NULL) mwerror(FATAL,0,"Not enough memory\n");
	}
      else 
	{
	  if (!pc->previous) 
	    {
	      mwerror(WARNING, 0, "No polygon defined\n");
	      break;
	    }
	  (pc->previous)->next = NULL;
	  WDrawLine(ImageWindow,x0,y0,x1,y1);
	  WDrawLine(ImageWindow,(pl->first)->x,(pl->first)->y,
		    (pc->previous)->x,(pc->previous)->y);
	}

      WSetTypePencil(W_COPY);
      WFlushWindow(ImageWindow);      

      npl = mw_change_polygon(NULL,NB_OF_CHANNELS);
      if (npl == NULL)  mwerror(FATAL,0,"Not enough memory\n");      
      npl->channel[0] = 0.0;          /* Inside color is black 0.0 */
      pl->next = npl;
      npl->previous = pl;
      pl = npl;
      first_point = 0;
      pl->first = pc;      
      pc->previous = NULL;

      printf("Next polygon\n");
      Redraw(ImageWindow);
      break;

    case W_RESIZE:
      Redraw(ImageWindow);
      if (first_point != 0) 
	{
	  WSetTypePencil(W_XOR); 
	  WDrawLine(ImageWindow,x0,y0,x1,y1);
	}
      break;

    case W_DESTROY:
      ret=-1;
      break;

    case W_KEYPRESS:
      switch((char) WGetKeyboard())
	{
	case 'q': case 'Q': ret =-1;
	  break;

	case 'h': case 'H': Help();
	  break;

	case ' ': List_of_Poly();
	  break;

	case 'r': case 'R':  Redraw(ImageWindow);
	  if (first_point != 0)
	    {
	      WSetTypePencil(W_XOR); 
	      WDrawLine(ImageWindow,x0,y0,x1,y1);
	    }
	  break;
	}
      break;

    }


  if (ret == -1)
    {
      if (pc->previous) (pc->previous)->next = NULL;
      free(pc);
      pc = NULL;
      printf("Last point deleted\n");

      if (first_point == 0) 
	{
	  if (pl->previous) (pl->previous)->next = NULL;
	  if (poly->first == pl) poly->first = NULL;
	  mw_delete_polygon(pl);
	  pl = NULL;
	  printf("Last polygon deleted\n");
	 }
      /* List_of_Poly(); */
      printf("Soft Quit...\n");

    }

  return(ret);
}


Polygons readpoly(image,window)

Cimage image;
char *window;

{
  Wframe *ImageWindow;

  poly = mw_new_polygons();
  if (poly == NULL) mwerror(FATAL,0,"Not enough memory\n");

  pl = mw_change_polygon(NULL,NB_OF_CHANNELS);
  if (pl == NULL) mwerror(FATAL,0,"Not enough memory\n");
  pl->channel[0] = 0.0;          /* Inside color is black 0.0 */
  poly->first = pl;

  pc = mw_new_point_curve();
  if (pc == NULL)  mwerror(FATAL,0,"Not enough memory\n");
  pl->first = pc;

  ImageWindow = (Wframe *)
    mw_get_window((Wframe *) window,image->ncol,image->nrow,100,100,
		  image->name);

  if (ImageWindow == NULL)
    mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");

  WLoadBitMapImage(ImageWindow,image->gray,image->ncol,image->nrow); 
  WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
  WFlushWindow(ImageWindow);

  WSetUserEvent(ImageWindow,W_MS_BUTTON | W_KEYPRESS); 

  dx = image->ncol;
  dy = image->nrow;

  WSetColorPencil(ImageWindow,0);

  mw_window_notify(ImageWindow,NULL,readpoly_notify);
  mw_window_main_loop();

  return(poly);
}








/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {drawocclusion};
version = {"1.22"};
author = {"Simon Masnou"};
function = {"Interactive creation of occlusions on an image"};
usage = {
   'g':[gray=255]->gray         "Occlusions gray level in the occluded image",
   'h':hole_image<-hole_image   "Occluded image",
   'z':zoom->zoom     "zoom factor for image display (even integer)",
   cimage->image      "Input image",
   label<-labelimage  "Output image of labelled occlusions",
   notused->window    "Window for image viewing (internal use)"
 };
*/
/*----------------------------------------------------------------------
 v1.21: revision, fixed mw_delete_point_curve and czoom bugs (S.Masnou)
 v1.22 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdlib.h>
#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for czoom() */

/**********************************************************************/
/* IMPORTANT NOTE : at the end of the labeling process, labels of each 
   connected component are sorted increasingly according to their
   order of appearance */
/**********************************************************************/

/* We assume than there are no more than LABELS_NUMBER labels obtained at first pass
 (before updating) */
#define LABELS_NUMBER 100000

static int u_compar_i(const void *u, const void *v)  /*  Called by function qsort for sorting decreasingly */
          
  {
    int iu, iv;

    iu = *(int*)u;
    iv = *(int*)v;
    if ((iu)<(iv)) return (1);
    if ((iu)==(iv)) return (0);
    return (-1);
  }

/****************************************************************************/
/*
The algorithm used here is the following : let us consider the case where we want
to compute the connected components of the set of points with value g (the principle
is analogous for the computation of the connected components of the complement).
We create a subsidiary image, one pixel larger in each direction than the input 
image, where all pixels of input frame with value g have value 1, while the others 
are set to 0. Then we scan the interior of this subsidiary image (borders are not
taken into account) from top to bottom and left to right (only one scan is enough). 
Each pixel with value 1 whose 4 upper and left pixels (in case of 8-connectivity) 
are 0 is given a new label (as if it was a new component).
If some of these 4 pixels are nonzero then the current pixel is given the greatest label
among these 4 and we update the transcoding table which tells us which label is 
connected to which one, i.e. which components are part of the same connected components.
Once the image has been totally scanned, we simply have to replace the value (label)
of each nonzero pixel by the smallest label to which it is associated.
*/
/****************************************************************************/

static void mise_a_jour_transcode(int *transcode, int a, int b)
{
    if (!b) return;
    if (transcode[a]==0)
      transcode[a]=b;
    else
    {
      if (b!=transcode[a])
      {
	if (b>transcode[a])
	   mise_a_jour_transcode(transcode,b,transcode[a]);
	else
	{
	  mise_a_jour_transcode(transcode,transcode[a],b);
	  transcode[a]=b;
	}
      }
    }
  }
    
/****************************************************************************/

/* In the transcoding table, a label l1 may be associated with another label l2,
itself associated with a third label l3, etc... The following function simply
performs a recursive association of each label l1,l2,l3 with the smallest possible
label in the chain. */

static void refresh(int *transcode, int *refresh_transcode, int i, int *first)
{
    if (transcode[i])
      {
	refresh(transcode,refresh_transcode,transcode[i],first);
	refresh_transcode[i]=*first;
      }
    else
      *first=i;
  }

/****************************************************************************/

static void fconnected(float *In, int line, int col, float FOREGROUND, int *NUMBER, char *not_writing_on, char *complement, char *connectivity)
{
    int Line=line+2,Col=col+2;
    int *Output;
    register float *ptrin;
    register int *ptrout;
    register int dx,dy,i;
    int kernel[4];
    int label,first;
    int transcode[LABELS_NUMBER]; /* transcodage table obtained at first pass and constructed 
				     following the rule : if a point is connected with two different labels 
				     (more than two is impossible) one writes in the table the link
				     label_max->label_min if label_max is not yet linked, else, say we
				     already have label_max->label' one makes by recursivity 
				     label'->label_min or label_min->label' depending on 
				     max(label',label_min)
				     */
    int refresh_transcode[LABELS_NUMBER];  /* same as transcode except that each label is linked with
					      the smallest value in the chain to which it belongs */
    int normalize_transcode[LABELS_NUMBER];
    register int *ptr_trans,*ptr_ref,*ptr_norm;
    int norme;

    /* The frame used for processing is 2 pixels wider and higher than the input to avoid
       problems with borders */

    Output=(int*)malloc(Line*Col*sizeof(int));
    if (Output==NULL)
      mwerror(FATAL,1,"Not enough memory !\n");
    
    for (i=0,ptr_trans=transcode,ptr_ref=refresh_transcode,
	 ptr_norm=normalize_transcode;i<LABELS_NUMBER;i++,ptr_trans++,ptr_ref++,ptr_norm++)
      {*ptr_trans=0;*ptr_ref=0;*ptr_norm=0;}
    
    for (dy=0,ptrout=Output;dy<Col;dy++,ptrout++) *ptrout=0; 
    for (dx=2,ptrin=In;dx<Line;dx++)
      {
	*ptrout=0;ptrout++;
	for (dy=2;dy<Col;dy++,ptrout++,ptrin++)
	  if (complement)
	    if (*ptrin!=FOREGROUND) *ptrout=1;
	    else *ptrout=0;
	  else
	    if (*ptrin==FOREGROUND) *ptrout=1;
	    else *ptrout=0;
	*ptrout=0;ptrout++;
      }
    for (dy=0,ptrout=Output;dy<Col;dy++,ptrout++) *ptrout=0; 
   
    label=1;
    for (dx=2,ptrout=Output+(Col+1);dx<Line;dx++,ptrout+=2)
      for (dy=2;dy<Col;dy++,ptrout++)
	if (*ptrout)
	  {
	    if (label>LABELS_NUMBER)
	      mwerror(FATAL,1,"There are more than %d labels !\n",LABELS_NUMBER);
	    /* 'kernel' contains the four upper nearest neighbours of current point
	               * * *
		       * + x
		       x x x
	       if 4-connectivity is used then only 2 neighbors are taken into account */
	    kernel[0]=*(ptrout-1);
	    if (connectivity) kernel[1]=(-1);
	    else kernel[1]=*(ptrout-(Col+1));
	    kernel[2]=*(ptrout-Col);
	    if (connectivity) kernel[3]=(-1);
	    else kernel[3]=*(ptrout-(Col-1));
	    qsort(kernel,4,sizeof(int),u_compar_i);
	    /* kernel values are sorted decreasingly */
	    if (kernel[0]==0)
	      *ptrout=label++;   /* This point is given a new label */
	    else
	      {
		*ptrout=kernel[0];
		if (kernel[1])
		  if (kernel[1]!=kernel[0])
		    if (kernel[1]!=transcode[kernel[0]])
		      mise_a_jour_transcode(transcode,kernel[0],kernel[1]);
	      }
	  }
    first=0;
    for (i=label-1;i>0;i--)
      if ((transcode[i]) && (!(refresh_transcode[i])))
	refresh(transcode,refresh_transcode,i,&first);
    
    
    /* We want each connected component to be labelled between 1 and the number
       of components. This is done in the following step */
    norme=1;
    for (i=1,ptr_ref=refresh_transcode+1;i<label;i++,ptr_ref++){
      if (!(*ptr_ref))
	*ptr_ref=i;
      if (normalize_transcode[*ptr_ref]==0)
	{normalize_transcode[*ptr_ref]=norme++;}}
    *NUMBER=norme-1;

     /* Writing on image */
    if (!not_writing_on)
      {
	for (dx=2,ptrout=Output+(Col+1),ptrin=In;dx<Line;dx++,ptrout+=2)
	  for (dy=2;dy<Col;dy++,ptrout++,ptrin++)
	    {
	      if (*ptrout)
		*ptrin=(float)(normalize_transcode[refresh_transcode[*ptrout]]);
	      else
		*ptrin=0;
	    }
      }
    free(Output);
  }  

/* Number of channels for the polygon: 1 (gray-level) */
#define NB_OF_CHANNELS 1

#define min(A,B)     (((A)>(B)) ? (B) : (A))
#define max(A,B)     (((A)>(B)) ? (A) : (B))
#define Labels_Number 10000

Polygons poly;
Polygon pl,npl;
Point_curve pc,npc;
int max_number_of_points_in_poly=0;

int x0,y0,x1,y1,oldx1,oldy1,dx,dy;
static char first_point=0;
char print_mode=0;

static void DrawLinePoly(Wframe *ImageWindow, int a0, int b0, int a1, int b1)
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

static void List_of_Poly(void)
{
  Polygon p;
  Point_curve c;
  int i;

  printf("\n\t\tList of recorded polygons:\n");

  for (p=poly->first,i =1; p; p=p->next, i++)
    {
      printf("\tPoly #%d\n",i);
      for (c=p->first; c; c=c->next)
	printf("\t\t(%d,%d)\n",c->x,c->y);
    }
  printf("\n");
}

static void Unzoom_Poly(int zoom)
{
  Polygon p;
  Point_curve c;

  for (p=poly->first; p; p=p->next)
    {
      for (c=p->first; c; c=c->next)
	{
	  c->x=c->x/zoom;;
	  c->y=c->y/zoom;
	}
    }
}  


static void Help(void)
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
  printf("\tP: Resumes or stops printing point coordinates.\n");
  printf("\tR: Redraw the window according to the recorded polygons.\n");
}


static void Redraw(Wframe *ImageWindow)
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
/*       0 if there was no event caught               */
/*     > 0 if there was an event caught (but Destroy) */
/*      -1 if the event Destroy was caught (or 'Q')   */

static int readpoly_notify(Wframe *ImageWindow, void * foo)
{
  int ret;
  int event,button_mask;
  
  /* FIXME: foo is just here to comply with the definition of
     mw_window_notify */
  foo = foo;

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
	  max_number_of_points_in_poly++;
	  if (npc == NULL) mwerror(FATAL,0,"Not enough memory\n");
	  pc->next = npc;
	  npc->previous = pc;
	  pc = npc;

	  if (print_mode) printf("Created a point in (%d,%d)\n",x1,y1);
	}

      break;
      
    case W_MS_MIDDLE: 
      /* Cancel */

      npc = pc->previous;
      if (npc != NULL)
	{
	  max_number_of_points_in_poly--;
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
	  free((void*)pc);
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

      if (print_mode) printf("Next polygon\n");
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
	case 'p':case 'P':
	  print_mode=1-print_mode;
	  if (print_mode)  List_of_Poly();
	  break;
	}
      break;

    }


  if (ret == -1)
    {
      if (pl->previous) (pl->previous)->next = NULL;
      if (poly->first == pl) poly->first = NULL;
      mw_delete_polygon(pl);
      pl = NULL;
      if (print_mode)
	{
	  List_of_Poly();
	  printf("Soft Quit...\n");
	}
    }

  return(ret);
}

/**************************************************************************/

void drawocclusion(Cimage image, Fimage labelimage, Cimage *hole_image, char *window, int *zoom, int *gray)
{
  Fimage interlabelimage;
  Wframe *ImageWindow;
  Polygon p;
  Point_curve c;
  int i,j;
  Cimage image_zoom;
  float fzoom;
  register float *ptrlabel,*ptrinterlabel;
  register unsigned char *ptrhole;
  int interp=0;
  int Number=0;
  float foreground;

  labelimage=mw_change_fimage(labelimage,image->nrow,image->ncol);
  if (!labelimage) mwerror(FATAL,1,"Not enough memory !\n");
  if (((interlabelimage=mw_new_fimage())==NULL) || 
      (mw_alloc_fimage(interlabelimage,image->nrow+2,image->ncol+2)==NULL))
    mwerror(FATAL,1,"Not enough memory !\n");
  /* "interlabelimage" is 2 pixels higher and larger than "labelimage" in order
     to deal properly with borders */
  if (hole_image)
    {
      *hole_image=mw_change_cimage(*hole_image,image->nrow,image->ncol);
      if (!*hole_image)  mwerror(FATAL,1,"Not enough memory !\n");
      mw_copy_cimage(image,*hole_image);
    }
  if (zoom)
    {
      if ((*zoom<=0)||((double)(*zoom)/2!=(double)((*zoom)/2)))
	mwerror(USAGE,1,"-z option parameter must be a stricly positive even integer !\n");
      if ((image_zoom=mw_new_cimage())==NULL)
	mwerror(FATAL,0,"Not enough memory\n");
      fzoom=(float)*zoom;
      czoom(image,image_zoom,(char*)NULL,(char*)NULL,&fzoom,&interp,(char *)NULL);
    }

  poly = mw_new_polygons();
  if (poly == NULL) mwerror(FATAL,0,"Not enough memory\n");

  pl = mw_change_polygon(NULL,NB_OF_CHANNELS);
  if (pl == NULL) mwerror(FATAL,0,"Not enough memory\n");
  pl->channel[0] = 0.0;          /* Inside color is black 0.0 */
  poly->first = pl;

  pc = mw_new_point_curve();
  if (pc == NULL)  mwerror(FATAL,0,"Not enough memory\n");
  pl->first = pc;

  if (zoom)
    ImageWindow = (Wframe *)
      mw_get_window((Wframe *) window,image_zoom->ncol,image_zoom->nrow,100,100,
		    image->name);
  else
  ImageWindow = (Wframe *)
    mw_get_window((Wframe *) window,image->ncol,image->nrow,100,100,
		  image->name);

  if (ImageWindow == NULL)
    mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");

  if (zoom)
    {
      WLoadBitMapImage(ImageWindow,image_zoom->gray,image_zoom->ncol,image_zoom->nrow); 
      WRestoreImageWindow(ImageWindow,0,0,image_zoom->ncol,image_zoom->nrow); 
    }
  else
    {
      WLoadBitMapImage(ImageWindow,image->gray,image->ncol,image->nrow); 
      WRestoreImageWindow(ImageWindow,0,0,image->ncol,image->nrow); 
    }
  WFlushWindow(ImageWindow);

  WSetUserEvent(ImageWindow,W_MS_BUTTON | W_KEYPRESS); 

  dx = image->ncol;
  dy = image->nrow;

  WSetColorPencil(ImageWindow,0);

  printf("\nUse the mouse to draw the contours of your occlusions.\nType 'h' (while pointing in the image window) for more help\n");
  mw_window_notify(ImageWindow,NULL,readpoly_notify);
  mw_window_main_loop();

  if (!(poly->first))
    mwerror(FATAL,1,"No occlusion was defined. Aborted\n");

  if (zoom)
    {
      Unzoom_Poly(*zoom);
      if (print_mode) 
	{
	  printf("\t\tNew polygons coordinates after unzooming:\n");
	  List_of_Poly();
	}
      mw_delete_cimage(image_zoom);
      image_zoom=NULL;
    }


  mw_clear_fimage(interlabelimage,0.0);
  /* Step 1 : draws lines between vertices within each polygon */
  for (p=poly->first; p; p=p->next)
    {
      if (p->first->next)
	{
	  for (c=p->first; c->next; c=c->next)
	    mw_draw_fimage(interlabelimage,c->x+1,c->y+1,c->next->x+1,c->next->y+1,1.0);
	  mw_draw_fimage(interlabelimage,c->x+1,c->y+1,p->first->x+1,p->first->y+1,1.0);
	}
      else
	/* In this case, the polygon reduces to a single point */
	  mw_plot_fimage(interlabelimage,p->first->x+1,p->first->y+1,1.0);
    }

  /* Step 2 : occlusions are defined as those sets enclosed by the polygons */
  foreground=0.0;
  fconnected(interlabelimage->gray,interlabelimage->nrow,interlabelimage->ncol,
	     foreground,&Number,(char*)NULL,(char*)NULL,(char*)1);
 
  /* Remark that now only the background has value 1. The polygon borders have value 0 
     while the regions they enclose have value > 1. Thus it remains to compute the connected
     components of the complement of the background. */
  
  /* Step 3 : computation of connected components of occlusions */
  foreground=1.0;
  fconnected(interlabelimage->gray,interlabelimage->nrow,interlabelimage->ncol,
	     foreground,&Number,(char*)NULL,(char*)1,(char*)NULL);
 
  /* Step 4 : labelimage is written */
  ptrinterlabel=interlabelimage->gray+interlabelimage->ncol+1;
  for (i=0,ptrlabel=labelimage->gray;i<labelimage->nrow;i++,ptrinterlabel+=2)
    for (j=0;j<labelimage->ncol;j++,ptrlabel++,ptrinterlabel++)
      *ptrlabel=*ptrinterlabel;

  mw_delete_fimage(interlabelimage);
  interlabelimage=NULL;
  
  /* Hole_image is now updated */
  if (hole_image)
    {
      for (i=0,ptrlabel=labelimage->gray,ptrhole=(*hole_image)->gray;i<labelimage->nrow;i++)
	for (j=0;j<labelimage->ncol;j++,ptrlabel++,ptrhole++)
	  if (*ptrlabel) *ptrhole=(unsigned char)(*gray);
    }

  /* Removes polygons */
  mw_delete_polygons(poly);
  poly=NULL;
}








/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {flip};
 version = {"1.2"};
 author = {"Lionel Moisan"};
 function = {"Compare two images using a flip view"};
 usage = {
   'z':[z=2.]->z  "zoom factor",
   'o':[o=0]->o   "interpolation: 0,1=linear,-3=cubic,3,5,7=spline",
   in1->in1       "input Fimage 1",
   in2->in2       "input Fimage 2"
};
*/
/*----------------------------------------------------------------------
 v1.1 (03/2007): fixed minor bug (Y2 in unzoom case) (LM)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fcrop() */

/*------------------- GLOBAL CONSTANTS AND VARIABLES -------------------*/

int ordernum = 6;
int ordervalue[6]={0,1,-3,3,5,7};
char *ordername[6]=
  {"nearest neighbor","bilinear","Key's bicubic","bicubic spline",
   "spline order 5","spline order 7"};

Wframe *win;
int nx,ny,num,order,zoom,disp;
float X1,Y1,X2,Y2,min,max;
Fimage u1,u2,u;
Ccimage image1,image2;

static void max_contrast()
{
  int adr;
  float bg,a,c,fnx,fny;

  bg = 128.; a = -0.5; fnx=(float)nx; fny=(float)ny;
  if (num==1) 
    fcrop(u1,u,&fnx,&fny,NULL,&bg,&ordervalue[order],&a,X1,Y1,X2,Y2);
  else
    fcrop(u2,u,&fnx,&fny,NULL,&bg,&ordervalue[order],&a,X1,Y1,X2,Y2);
  min = max = u->gray[0];
  for (adr=nx*ny;adr--;) {
    c = u->gray[adr];
    if (c>max) max=c;
    if (c<min) min=c;
  }
}

static void recompute()
{
  int adr;
  float bg,a,c,fnx,fny;

  bg = 128.; a = -0.5; fnx=(float)nx; fny=(float)ny;

  /* zoom image 1 */
  fcrop(u1,u,&fnx,&fny,NULL,&bg,&ordervalue[order],&a,X1,Y1,X2,Y2);
  for (adr=nx*ny;adr--;) {
    c = 256.*(u->gray[adr]-min)/(max-min);
    if (c>255.) c=255.; else if (c<0.) c=0.;
    image1->red[adr]=image1->green[adr]=image1->blue[adr]= (unsigned char)c;
  }

  /* zoom image 2 */
  fcrop(u2,u,&fnx,&fny,NULL,&bg,&ordervalue[order],&a,X1,Y1,X2,Y2);
  for (adr=nx*ny;adr--;) {
    c = 256.*(u->gray[adr]-min)/(max-min);
    if (c>255.) c=255.; else if (c<0.) c=0.;
    image2->red[adr]=image2->green[adr]=image2->blue[adr]= (unsigned char)c;
  }
}

static void redisplay()
{
  /* 
   * FIXME: oversized str, but this is not safe, snprintf *is*
   * required
   */
  char str[512];

  if (num==1)
    WLoadBitMapColorImage(win,image1->red,image1->green,image1->blue,nx,ny);
  else
    WLoadBitMapColorImage(win,image2->red,image2->green,image2->blue,nx,ny);
  WRestoreImageWindow(win,0,0,nx,ny);
  switch(disp) {
  case 1:
    sprintf(str,"current image: %s",(num==1?u1->name:u2->name)); 
    str[80] = '\0';
    WDrawString(win,0,10,str);
    break;
  case 2:
    sprintf(str,"interpolation method: %s",ordername[order]);
    str[80] = '\0';
    WDrawString(win,0,10,str);
    break;
  case 3:
    sprintf(str,"currently displayed window: [%f,%f]x[%f,%f]",
	     X1,X2,Y1,Y2); 
    str[80] = '\0';
    WDrawString(win,0,10,str);
    break;
  }
  WFlushWindow(win);
}

static void help()
{
  printf("\n\t\tHelp on line\n");
  printf("\nMouse:\n");
  printf("\tLeft button:    Display parameters.\n");
  printf("\tMiddle button:  Restore default settings.\n");
  printf("\tRight button:   Zoom x2 on the selected location.\n");
  printf("\nKeyboard:\n");
  printf("\tQ:  Quit.\n");
  printf("\tH:  Help.\n");
  printf("\tU:  Unzoom x2.\n");
  printf("\tC:  Maximize contrast.\n");
  printf("\tO:  Change interpolation order.\n");
  printf("\tSpace:        Toggle images.\n");
  printf("\tLeft arrow:   Go Left.\n");
  printf("\tRight arrow:  Go Right.\n");
  printf("\tUp arrow:     Go Up.\n");
  printf("\tDown arrow:   Go Down.\n");
}


/* handle display events */
static int win_notify(window,param)
     Wframe *window;
     void *param;
{
  int event,ret,x,y,button_mask;
  float nc,nd;
  int c; /* Key code must be int and not char to handle non-printable keys */

  event = WUserEvent(window); 
  if (event<0) ret=1; else ret=event;
  WGetStateMouse(window,&x,&y,&button_mask);
  switch (event) 
    {

    case W_MS_LEFT: /* display parameters */
      disp = (disp+1)%4;
      redisplay();
      break;

    case W_MS_RIGHT: /* zoom x2 */
      zoom++;
      nc = .5*(float)x*(X2-X1)/(float)nx;
      nd = .5*(X2-X1);
      X1 += nc; X2 = X1+nd;
      nc = .5*(float)y*(Y2-Y1)/(float)ny;
      nd = .5*(Y2-Y1);
      Y1 += nc; Y2 = Y1+nd;
      recompute();
      redisplay();
      break;

    case W_MS_MIDDLE: /* restore orgiginal display */
      X1 = 0.; Y1 = 0.; X2 = (float)u1->ncol; Y2=(float)u1->nrow;
      zoom = 0; disp = 0;
      max_contrast();
      recompute();
      redisplay();
      break;

    case W_RESIZE:
      break;

    case W_DESTROY:
      ret = -1;
      break;

    case W_KEYPRESS:
      c = WGetKeyboard();
      switch(c)
	{
	  /* translate left */
	case XK_Left: case XK_KP_Left: 
	  nc = .1*(X2-X1); X1-=nc; X2-=nc; recompute(); redisplay();  break;

	  /* translate right */
	case XK_Right:case XK_KP_Right: 
	  nc = .1*(X2-X1); X1+=nc; X2+=nc; recompute(); redisplay();  break;

	  /* translate up */
	case XK_Up: case XK_KP_Up:
	  nc = .1*(Y2-Y1); Y1-=nc; Y2-=nc; recompute(); redisplay();  break;

	  /* translate down */
	case XK_Down: case XK_KP_Down:
	  nc = .1*(Y2-Y1); Y1+=nc; Y2+=nc; recompute(); redisplay();  break;

	  /* toggle images */
	case ' ':
	  num = 3-num;
	  redisplay();
	  break;

	  /* maximize contrast */
	case 'c': case 'C':       
	  max_contrast();
	  recompute();
	  redisplay();
	  break;
	    
	  /* change interpolation order */
	case 'o': case 'O':       
	  order = (order+1)%ordernum;
	  recompute();
	  redisplay();
	  break;
	    
	  /* unzoom x2 */
	case 'u': case 'U':       
	  zoom--;
	  if (zoom==0) {
	    X1 = 0.; Y1 = 0.; 
	    X2 = (float)u1->ncol; Y2=(float)u1->nrow;
	  } else {
	    nc = X1+(float)x*(X2-X1)/(float)nx;
	    nd = X2-X1;
	    X1 = nc-nd; X2 = nc+nd;
	    nc = Y1+(float)y*(Y2-Y1)/(float)ny;
	    nd = Y2-Y1;
	    Y1 = nc-nd; Y2 = nc+nd;
	  }
	  recompute(); 
	  redisplay();
	  break;

	  /* quit */
	case 'q': case 'Q': ret = -1; break;
	  
	  /* help */
	case 'h': case 'H': help(); break;
	}
      break;
    }

  return(ret);
}


/*------------------------------ MAIN MODULE ------------------------------*/

void flip(in1,in2,z,o)
     Fimage in1,in2;
     float *z;
     int *o;
{
  char str[512];

  /* Initializations */
  nx = (int)(*z*(float)in1->ncol);
  ny = (int)(*z*(float)in1->nrow);
  image1 = mw_change_ccimage(NULL,ny,nx);
  image2 = mw_change_ccimage(NULL,ny,nx);
  if (!image1 || !image2) mwerror(FATAL,1,"Not enough memory\n");

  X1 = 0.; Y1 = 0.; X2 = (float)in1->ncol; Y2=(float)in1->nrow;
  zoom = 0; num = 1; disp = 0;
  u1 = in1; u2 = in2;
  u = mw_new_fimage();
  for (order=0;ordervalue[order]!=*o && order<ordernum;order++);
  if (order==ordernum) mwerror(FATAL,1,"Unrecognized interpolation order.");

  sprintf(str,"flip %s and %s",u1->name,u2->name);
  str[80] = '\0';
  if (!(win=(Wframe *)mw_get_window(NULL,nx,ny,50,50,str)))
    mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");

  max_contrast();
  recompute();
  redisplay();

  /* interactive display */
  WSetUserEvent(win,W_MS_BUTTON | W_KEYPRESS);
  mw_window_notify(win,NULL,win_notify);
  mw_window_main_loop();

  mw_delete_fimage(u);
  mw_delete_ccimage(image2);
  mw_delete_ccimage(image1);
}


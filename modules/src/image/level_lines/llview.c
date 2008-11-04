/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {llview};
 version = {"2.3"};
 author = {"Lionel Moisan"};
 function = {"Interactive visualization of level lines"};
 usage = {
   'z':[z=2.]->z    "zoom factor for display",
   'l':[l=100]->l   "current level",
   's':[s=25]->s    "level step",
   'd':[d=2]->d     "display mode",
   'i':[i=1]->i     "interpolation mode",
   'b':[b=0]->b     "background mode",
   'o':out<-out     "to save the last view as a Ccimage",
   'n'->n           "no display (useful with -o option)",
   in->in           "input (Fimage)"
};
*/
/*----------------------------------------------------------------------
 v2.1: new interactive version (L.Moisan)
 v2.2: minor modifications (L.Moisan)
 v2.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "mw.h"
#include "mw-modules.h" /* for fcrop() */

/*------------------- GLOBAL VARIABLES -------------------*/

                    /* button handling */
Wpanel wp;
struct wp_toggle b1,b2,b3,b6;
struct wp_int b4,b5;

Wframe *win1;       /* control window */
Wframe *win2;       /* display window */

int nx,ny,zoom;     /* size of display and zoom level */
Fimage ref;         /* reference image (= input image) */
Fimage interpolate; /* current interpolated image */
Ccimage image;      /* image displayed */
float X1,Y1,X2,Y2;  /* location of displayed subimage */

/*--------------------------------------------------------*/


static void faded_fimage_to_ccimage(in,out,lambda)
     Fimage in;     
     Ccimage out;
     float lambda;
{
  int adr;
  float c;

  for (adr=in->nrow*in->ncol;adr--;) {
    c = in->gray[adr];
    c = 256.-lambda*(256.-c);
    if (c>255.) c=255.; else if (c<0.) c=0.;
    out->red[adr] = out->green[adr] = out->blue[adr] = (unsigned char)c;
  }
}
		  
static void my_llmap(in,out,ofs,step)
     Fimage in;
     Ccimage out;
     float ofs,step;
{
  float bg,v,w,a,NX,NY;
  int x,y,adr,ok,order;
  double fv;

  /* image interpolation */
  if (b3.button==2) order=-3; else 
    if (b3.button<2) order=b3.button; else 
      order=b3.button*2-3;
  bg = 128.; a = -0.5; NX = (float)out->ncol; NY = (float)out->nrow;
  fcrop(in,interpolate,&NX,&NY,NULL,&bg,&order,&a,X1,Y1,X2,Y2);

  /* background if needed */
  if (b2.button<2) 
    faded_fimage_to_ccimage(interpolate,out,(b2.button==0?1.:0.75));
  else mw_clear_ccimage(out,255,255,255);

  /* add levels */
  if (b1.button) 

    for (x=0;x<nx-1;x++)
      for (y=0;y<ny-1;y++) {
	
	adr = y*nx+x;
	v = interpolate->gray[adr];
	ok = 0;

	switch (b1.button) 
	  {
	    
	  case 1: /* level lines */
	    fv = floor((double)((v-ofs)/step));
	    w = interpolate->gray[adr+1];
	    if (floor((double)((w-ofs)/step)) != fv) ok = 1;
	    w = interpolate->gray[adr+nx];
	    if (floor((double)((w-ofs)/step)) != fv) ok = 1;
	    w = interpolate->gray[adr+nx+1];
	    if (floor((double)((w-ofs)/step)) != fv) ok = 1;
	    break;
	    
	  case 2: /* one level line */
	    w = interpolate->gray[adr+1];
	    if ((w-ofs)*(v-ofs)<=0. && v!=w) ok=1;
	    w = interpolate->gray[adr+nx];
	    if ((w-ofs)*(v-ofs)<=0. && v!=w) ok=1;
	    w = interpolate->gray[adr+nx+1];
	    if ((w-ofs)*(v-ofs)<=0. && v!=w) ok=1;
	    break;
	    
	  case 3: /* one lower level set */
	    ok = (v<ofs);
	    break;
	    
	  case 4: /* one upper level set */
	    ok = (v>=ofs);
	    break;
	    
	  case 5: /* one bi-level set */
	    ok = (v>=ofs && v<ofs+step);
	    break;

	  }
	if (ok) {
	  out->red[adr] = 255;
	  out->green[adr] = 0;
	  out->blue[adr] = 0;
	}
      }
}


static int redisplay(wt,n)
     Wp_toggle wt;
     short n;
{
  my_llmap(ref,image,(float)b4.value,(float)b5.value);
  WLoadBitMapColorImage(win2,image->red,image->green,image->blue,nx,ny);
  WRestoreImageWindow(win2,0,0,nx,ny);
  WFlushWindow(win2);
  return(0);
}

static int grey_level(wi,n)
     Wp_int wi;
     short n;
{
  wi->value = (wi->value+256)%256;
  redisplay(NULL,0);
  return(0);
}

static int step(wi,n)
     Wp_int wi;
     short n;
{
  if (wi->value<1) wi->value = 1;
  if (wi->value>255) wi->value = 255;
  redisplay(NULL,0);
  return(0);
}

static int quit(wt,n)
     Wp_toggle wt;
     short n;
{
  wp->state = -1;
  return(wp->state);
}

static void help()
{
  printf("\n\t\tHelp on line\n");
  printf("\nMouse:\n");
  printf("\tLeft button:    Select grey level.\n");
  printf("\tMiddle button:  Restore the original display region.\n");
  printf("\tRight button:   Zoom x2 on the selected location.\n");
  printf("\nKeyboard:\n");
  printf("\tQ:  Quit.\n");
  printf("\tH:  Help.\n");
  printf("\tU:  Unzoom x2.\n");
  printf("\tLeft arrow:   Go Left.\n");
  printf("\tRight arrow:  Go Right.\n");
  printf("\tUp arrow:     Go Up.\n");
  printf("\tDown arrow:   Go Down.\n");
}

/* handle display events */
static int win2_notify(window,param)
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

    case W_MS_LEFT: /* select level */
      b4.value =  (int)(.5+interpolate->gray[y*image->ncol+x]);
      Wp_SetButton(WP_INT,wp,(void *)(&b4));
      redisplay(NULL,0);
      break;

    case W_MS_RIGHT: /* zoom x2 */
      zoom++;
      nc = .5*(float)x*(X2-X1)/(float)nx;
      nd = .5*(X2-X1);
      X1 += nc; X2 = X1+nd;
      nc = .5*(float)y*(Y2-Y1)/(float)ny;
      nd = .5*(Y2-Y1);
      Y1 += nc; Y2 = Y1+nd;
      redisplay(NULL,0);
      break;

    case W_MS_MIDDLE: /* restore orgiginal display */
      zoom = 0;
      X1 = 0.; Y1 = 0.; 
      X2 = (float)ref->ncol; Y2=(float)ref->nrow;
      redisplay(NULL,0);
      break;

    case W_RESIZE:
      break;

    case W_DESTROY:
      wp->state = -1;
      break;

    case W_KEYPRESS:
      c = WGetKeyboard();
      switch(c)
	{
	  /* translate left */
	case XK_Left: case XK_KP_Left: 
	  nc = .1*(X2-X1); X1-=nc; X2-=nc; redisplay(NULL,0);  break;

	  /* translate right */
	case XK_Right:case XK_KP_Right: 
	  nc = .1*(X2-X1); X1+=nc; X2+=nc; redisplay(NULL,0);  break;

	  /* translate up */
	case XK_Up: case XK_KP_Up:
	  nc = .1*(Y2-Y1); Y1-=nc; Y2-=nc; redisplay(NULL,0);  break;

	  /* translate down */
	case XK_Down: case XK_KP_Down:
	  nc = .1*(Y2-Y1); Y1+=nc; Y2+=nc; redisplay(NULL,0);  break;

	  /* unzoom x2 */
	case 'u': case 'U':       
	  if (zoom>0) {
	    zoom--;
	    if (zoom==0) {
	      X1 = 0.; Y1 = 0.; 
	      X2 = (float)ref->ncol; Y2=(float)ref->nrow;
	    } else {
	      nc = X1+(float)x*(X2-X1)/(float)nx;
	      nd = X2-X1;
	      X1 = nc-nd; X2 = nc+nd;
	      nc = Y1+(float)y*(Y2-Y1)/(float)ny;
	      nd = Y2-Y1;
	      Y1 = nc-nd; Y2 = nc+nd;
	    }
	    redisplay(NULL,0);
	  }
	  break;

	  /* quit */
	case 'q': case 'Q': wp->state = -1; break;
	  
	  /* help */
	case 'h': case 'H': help(); break;
	}
      break;
    }

  return(wp->state);
}

/* initialize buttons on control window */
static void init_buttons()
{
  int line,linestep;

  wp = Wp_Init(win1);
  line = 0; linestep = 22;

  /* button 1 */
  b1.text = "Level "; b1.nbuttons=6; b1.color=WP_RED;
  b1.button_text = (char **)malloc(b1.nbuttons*sizeof(char *));
  b1.button_text[0]="none";
  b1.button_text[1]="lines";
  b1.button_text[2]="one level";
  b1.button_text[3]="lower set";
  b1.button_text[4]="upper set";
  b1.button_text[5]="bi-level set";
  b1.x=10; b1.y=5+linestep*line++; b1.proc=redisplay;
  Wp_SetButton(WP_TOGGLE,wp,(void *)(&b1));

  /* button 2 */
  b2.text = "Background "; b2.nbuttons=3; b2.color=WP_RED;
  b2.button_text = (char **)malloc(b2.nbuttons*sizeof(char *));
  b2.button_text[0]="original image";
  b2.button_text[1]="attenuated image";
  b2.button_text[2]="none";
  b2.x=10; b2.y=5+linestep*line++; b2.proc=redisplay;
  Wp_SetButton(WP_TOGGLE,wp,(void *)(&b2));

  /* button 3 */
  b3.text = "Interpolation "; b3.nbuttons=6; b3.color=WP_RED;
  b3.button_text = (char **)malloc(b3.nbuttons*sizeof(char *));
  b3.button_text[0]="block";
  b3.button_text[1]="linear";
  b3.button_text[2]="cubic";
  b3.button_text[3]="spline 3";
  b3.button_text[4]="spline 5";
  b3.button_text[5]="spline 7";
  b3.x=10; b3.y=5+linestep*line++; b3.proc=redisplay;
  Wp_SetButton(WP_TOGGLE,wp,(void *)(&b3));

  /* button 4 (int) */
  b4.text = "grey level : "; b4.format = "%3d  ";
  b4.nbuttons=4; b4.strsize=0; b4.color=WP_BLUE;
  b4.scale=150; b4.firstscale=0; b4.lastscale=255; b4.divscale=4;
  b4.button_text = (char **)malloc(b4.nbuttons*sizeof(char *));
  b4.button_inc = (int *)malloc(b4.nbuttons*sizeof(int));
  b4.button_text[0]="-20";   b4.button_inc[0]=-20;
  b4.button_text[1]="-1";    b4.button_inc[1]=-1;
  b4.button_text[2]="+1";    b4.button_inc[2]= 1;
  b4.button_text[3]="+20";   b4.button_inc[3]= 20;
  b4.x=10; b4.y=5+linestep*line++; b4.proc=grey_level;
  Wp_SetButton(WP_INT,wp,(void *)(&b4));

  /* button 5 (int) */
  b5.text = "      step : "; b5.format = "%3d  ";
  b5.nbuttons=4; b5.strsize=0; b5.color=WP_BLUE;
  b5.scale=100; b5.firstscale=1; b5.lastscale=50; b5.divscale=5;
  b5.button_text = (char **)malloc(b5.nbuttons*sizeof(char *));
  b5.button_inc = (int *)malloc(b5.nbuttons*sizeof(int));
  b5.button_text[0]="-10";   b5.button_inc[0]=-10;
  b5.button_text[1]="-1";    b5.button_inc[1]=-1;
  b5.button_text[2]="+1";    b5.button_inc[2]= 1;
  b5.button_text[3]="+10";   b5.button_inc[3]= 10;
  b5.x=10; b5.y=5+linestep*line++; b5.proc=step;
  Wp_SetButton(WP_INT,wp,(void *)(&b5));

  /* button 6 (QUIT) */
  b6.text = "";
  b6.nbuttons=1; b6.button=0; b6.color=WP_GREEN;
  b6.button_text = (char **)malloc(sizeof(char *));
  b6.button_text[0]="Quit";
  b6.x=416; b6.y=5+linestep*(line-1); b6.proc=quit;
  Wp_SetButton(WP_TOGGLE,wp,(void *)(&b6));
}

/*------------------------------ MAIN MODULE ------------------------------*/

void llview(in,z,s,l,d,i,b,out,n)
     Fimage in;
     float *z;
     int *s,*l,*d,*i,*b;
     Ccimage *out;
     char *n;
{
  /* Initialization */
  nx = (int)(*z*(float)in->ncol);
  ny = (int)(*z*(float)in->nrow);
  X1 = 0.; Y1 = 0.; X2 = (float)in->ncol; Y2=(float)in->nrow;
  zoom = 0; ref = in;
  image = mw_change_ccimage(NULL,ny,nx);
  interpolate = mw_change_fimage(NULL,ny,nx);
  if (!image || !interpolate) mwerror(FATAL,1,"Not enough memory\n");
  b1.button = *d; b2.button = *b; b3.button = *i;
  b4.value = *l; b5.value = *s;

  if (!n) { /* interactive mode */

    /* open control window and display window */
    if (!(win1=(Wframe *)mw_get_window(NULL,460,115,230,5,"llview (control)")))
      mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");
    if (!(win2=(Wframe *)mw_get_window(NULL,nx,ny,50,145,"llview (display)")))
      mwerror(INTERNAL,1,"NULL window returned by mw_get_window\n");
    
    /* initialize buttons and display */
    init_buttons();
    redisplay(NULL,0);
    
    /* set events for display window */
    WSetUserEvent(win2,W_MS_BUTTON | W_KEYPRESS);
    mw_window_notify(win2,NULL,win2_notify);
    
    /* interactive display */
    Wp_main_loop(wp);

  } else { /* non-interactive mode */

    my_llmap(ref,image,(float)b4.value,(float)b5.value);

  }

  /* return display image if requested */
  if (out) *out=image;
}








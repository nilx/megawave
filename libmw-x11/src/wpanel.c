/**
 * @file wpanel.c
 *
 * @version 1.2
 * @author Lionel Moisan
 *
 * Panel display facilities and buttons
 */

/**
 * @changes
 * - v1.1 (JF): "utils.h" and "window.h" added (J. Froment)
 * - v1.2 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "definitions.h"
#include "config.h"
#include "window.h"
#include "wdevice.h"

#include "wpanel.h"

/* FIXME: unsafe snprintf() hack */
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
static int snprintf(char * dest, int nb, const char * fmt, ...)
{
     /* arbitrary length, this isn't safe */
     static char tmp[1024];
     va_list args;

     va_start(args, fmt);
     sprintf(tmp, fmt, args);
     va_end(args);
     dest[0] = '\0';
     strncat(dest, tmp, nb);
     return strlen(dest);
}
#pragma GCC diagnostic error "-Wformat-nonliteral"
 
/* draw button and return width (height is 16) */
int Wp_DrawButton(Wframe *window, int x, int y, char *str, int color)
{
     int n;

     n = strlen(str)*7;
     WSetColorPencil(window,WP_BLACK);
     WDrawRectangle(window,x,y,x+n+4,y+15);
     WSetColorPencil(window,WP_GREY);
     WDrawLine(window,x+1,y+16,x+n+5,y+16);
     WDrawLine(window,x+n+5,y+1,x+n+5,y+16);
     WSetColorPencil(window,color);  
     WDrawString(window,x+3,y+12,str);
     return(n+5);
}

/* draw scale bar */
void Wp_DrawScale(Wframe *window, int x, int y, int pos, 
		  int divisions, int length, int color)
{
     int i;

     WSetColorPencil(window,WP_WHITE);
     WFillRectangle(window,x,y+3,x+length,y+5);
     WFillRectangle(window,x,y+10,x+length,y+12);
     WSetColorPencil(window,WP_BLACK);
     WDrawRectangle(window,x,y+6,x+length,y+9);
     for (i=1;i<divisions;i++) 
	  WDrawLine(window,x+(length*i)/divisions,y+7,
		    x+(length*i)/divisions,y+8);
     WSetColorPencil(window,color);
     WDrawLine(window,x+pos,y+3,x+pos,y+5);
     WDrawLine(window,x+pos,y+10,x+pos,y+12);
}

/* init Wp structure */
Wpanel Wp_Init(Wframe *window)
{
     Wpanel wp;
     int i;

     wp = (Wpanel)malloc(sizeof(struct wpanel));
     wp->window = window;
     wp->state = 0;
     wp->nx = window->dx;
     wp->ny = window->dy;
     i = wp->nx*wp->ny;
     wp->type = (char *)malloc(i);
     wp->action = (void **)malloc(i*sizeof(void *));
     wp->button = (short *)malloc(i*sizeof(short));
     for (;i--;) wp->type[i] = WP_NULL;

     WSetColorPencil(wp->window,WP_WHITE);
     WFillRectangle(wp->window,0,0,wp->nx-1,wp->ny-1);
     WFlushWindow(wp->window);

     return(wp);
}

/* init or actualize button */
void Wp_SetButton(int type, Wpanel wp, void *b)
{
     char str[WP_STRSIZE];
     int i,n,x,dx,dy,adr;
     Wp_toggle wt;
     Wp_int wi;
     Wp_float wf;

     switch (type) 
     {

	  /*----- TOGGLE -----*/

     case WP_TOGGLE:
	  wt = (Wp_toggle)b;
	  WSetColorPencil(wp->window,WP_BLACK);
	  WDrawString(wp->window,wt->x,wt->y+12,wt->text);
	  x = wt->x+strlen(wt->text)*7;
	  for (i=0;i<wt->nbuttons;i++) {
	       n = Wp_DrawButton(wp->window,x,wt->y,wt->button_text[i],
				 (i==wt->button?wt->color:WP_BLACK));
	       for (dx=0;dx<n;dx++)
		    for (dy=0;dy<16;dy++) {
			 adr = wp->nx*(wt->y+dy)+x+dx;
			 wp->type[adr] = type;
			 wp->action[adr] = (void *)wt;
			 wp->button[adr] = (short)i;
		    }
	       x += n+7;
	  }
	  WFlushAreaWindow(wp->window,wt->x,wt->y,x,wt->y+16); 
	  break;

	  /*----- INT -----*/

     case WP_INT:
	  wi = (Wp_int)b;
	  WSetColorPencil(wp->window,WP_BLACK);
	  snprintf(str,WP_STRSIZE,wi->text);
	  WDrawString(wp->window,wi->x,wi->y+12,str);
	  x = wi->x+strlen(str)*7;
	  WSetColorPencil(wp->window,wi->color);
	  if (!wi->strsize) {
	       snprintf(str,WP_STRSIZE,wi->format,wi->value);
	       wi->strsize = strlen(str)+1;
	  } else snprintf(str,wi->strsize,wi->format,wi->value);
	  WDrawString(wp->window,x,wi->y+12,str);
	  x += strlen(str)*7;
	  if (wi->scale) {
	       snprintf(str,WP_STRSIZE,"%d",wi->firstscale);
	       WSetColorPencil(wp->window,WP_BLACK);
	       WDrawString(wp->window,x,wi->y+12,str);
	       x += strlen(str)*7+10;
	       i = ( (wi->value-wi->firstscale)*wi->scale 
		     +(wi->lastscale-wi->firstscale)/2 )
		    /(wi->lastscale-wi->firstscale);
	       if (i<0) i=0;
	       if (i>wi->scale) i=wi->scale;
	       Wp_DrawScale(wp->window,x,wi->y,i,wi->divscale,wi->scale,wi->color);
	       for (dx=0;dx<=wi->scale;dx++)
		    for (dy=3;dy<=12;dy++) {
			 adr = wp->nx*(wi->y+dy)+x+dx;
			 wp->type[adr] = type;
			 wp->action[adr] = (void *)wi;
			 wp->button[adr] = (short)(-dx-1);
		    }
	       x += wi->scale+5;
	       snprintf(str,WP_STRSIZE,"%d",wi->lastscale);
	       WSetColorPencil(wp->window,WP_BLACK);
	       WDrawString(wp->window,x,wi->y+12,str);
	       x += strlen(str)*7+10;
	  }
	  for (i=0;i<wi->nbuttons;i++) {
	       n = Wp_DrawButton(wp->window,x,wi->y,wi->button_text[i],0);
	       for (dx=0;dx<n;dx++)
		    for (dy=0;dy<16;dy++) {
			 adr = wp->nx*(wi->y+dy)+x+dx;
			 wp->type[adr] = type;
			 wp->action[adr] = (void *)wi;
			 wp->button[adr] = (short)i;
		    }
	       x += n+7;
	  }
	  WFlushAreaWindow(wp->window,wi->x,wi->y,x,wi->y+16); 
	  break;
     
	  /*----- FLOAT -----*/

     case WP_FLOAT:
	  wf = (Wp_float)b;
	  WSetColorPencil(wp->window,WP_BLACK);
	  snprintf(str,WP_STRSIZE,wf->text);
	  WDrawString(wp->window,wf->x,wf->y+12,str);
	  x = wf->x+strlen(str)*7;
	  WSetColorPencil(wp->window,wf->color);
	  if (!wf->strsize) {
	       snprintf(str,WP_STRSIZE,wf->format,wf->value);
	       wf->strsize = strlen(str)+1;
	  } else snprintf(str,wf->strsize,wf->format,wf->value);
	  WDrawString(wp->window,x,wf->y+12,str);
	  x += strlen(str)*7;
	  for (i=0;i<wf->nbuttons;i++) {
	       n = Wp_DrawButton(wp->window,x,wf->y,wf->button_text[i],0);
	       for (dx=0;dx<n;dx++)
		    for (dy=0;dy<16;dy++) {
			 adr = wp->nx*(wf->y+dy)+x+dx;
			 wp->type[adr] = type;
			 wp->action[adr] = (void *)wf;
			 wp->button[adr] = (short)i;
		    }
	       x += n+7;
	  }
	  WFlushAreaWindow(wp->window,wf->x,wf->y,x,wf->y+16); 
	  break;
     
     default:
	 fprintf(stderr, "Wp_SetButton : no such button type (%d).", 
		 (int)type);
	 abort();
     }
}

/* handle event */
int Wp_handle(Wpanel wp, int event, int x, int y)
{
     int adr,ret;
     char type;
     short b;
     Wp_toggle wt;
     Wp_int wi;
     Wp_float wf;

     ret = wp->state;
     if (event==W_MS_LEFT && y>=0 && y<wp->ny && x>=0 && x<wp->nx) {
	  adr = y*wp->nx+x;
	  type = wp->type[adr];
	  switch (type)
	  {
	  case WP_TOGGLE:
	       wt = (Wp_toggle)(wp->action[adr]);
	       if (wt) {
		    wt->button = wp->button[adr];
		    if (wt->proc) ret=wt->proc(wt,wp->button[adr]);
		    if (ret==0) ret=event;
		    Wp_SetButton(type,wp,(void *)wt);
	       }
	       break;
	
	  case WP_INT:
	       wi = (Wp_int)(wp->action[adr]);
	       if (wi) {
		    b = wp->button[adr];
		    if (b<0) 
			 wi->value = wi->firstscale+
			      ( (wi->lastscale-wi->firstscale)*(-b-1)+wi->scale/2 )/wi->scale;
		    else wi->value += wi->button_inc[b];
		    if (wi->proc) ret=wi->proc(wi,wp->button[adr]);
		    if (ret==0) ret=event;
		    Wp_SetButton(type,wp,(void *)wi);
	       }
	       break;

	  case WP_FLOAT:
	       wf = (Wp_float)(wp->action[adr]);
	       if (wf) {
		    b = wp->button[adr];
		    if (b>=0) wf->value += wf->button_inc[b];
		    if (wf->proc) ret=wf->proc(wf,wp->button[adr]);
		    if (ret==0) ret=event;
		    Wp_SetButton(type,wp,(void *)wf);
	       }
	       break;
	  }
     }
     return(ret);
}


/* scan events */
int Wp_notify(Wframe *window, void *wp)
{
     int event,ret,x,y,button_mask;

     event = WUserEvent(window); 
     WGetStateMouse(window,&x,&y,&button_mask);
     ret = Wp_handle((Wpanel)wp,event,x,y);

     return(ret);
}

/*
 * FIXME:
 * void mw_window_notify(Wframe *Win, void *param, int (*proc)(void));
 */
/* main loop */
void Wp_main_loop(Wpanel wp)
{
     WSetUserEvent(wp->window,W_MS_BUTTON | W_KEYPRESS);
     mw_window_notify(wp->window,(void *)wp,Wp_notify);
     mw_window_main_loop();
}

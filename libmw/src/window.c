/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  window.c
   
  Vers. 1.4
  (C) 1993-2003 Jacques Froment
  Interconnexion between the Wdevice Library and MegaWave2

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* usleep() */
#define _BSD_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "libmw-defs.h"
#include "utils.h"

#include "window.h"

#define mw_nmax_windows 10 /* Max Numbers of windows */

int mwwindelay=100; /* Delay to refresh windows (in microseconds) */
int mw_n_windows=0; /* Current Number of windows */

Wframe *mw_ptr_window[mw_nmax_windows]; /* ptr to each window */
void *mw_ptr_param[mw_nmax_windows]; /* ptr to each user's parameter */

/* ptr to each notify proc associated to a window */
int (*mw_ptr_window_notify[mw_nmax_windows])(Wframe *, void *);

/* record the mwrunmode of each window */
int mw_window_runmode[mw_nmax_windows]; 

/* Filled by Wdevice: Maps bitmap pixel values to X pixel vals */
unsigned long _W_cols[256];

/*     Function used by XMegaWave: 
       defines the pixel level associated to a given gray level */

/*     Warning: depends to the WSetColorMap code */

unsigned char mw_CeldaGris(unsigned char gris)
{
     return(_W_cols[gris >> 2]);
}

/*     Function used by XMegaWave: 
       defines the pixel level associated to a given gray level */

/*     Warning: depends to the WSetColorMap code */

unsigned char mw_CeldaColor(unsigned char * color)
{
     int c;

     c = (color[2])/56 + 5*((color[1])/56) + 25*((color[0])/56) + 64;
     return(_W_cols[c]);
}

/*     Get a window available for drawing, new if window==NULL or if
       window->win == NULL (deleted window).
*/

Wframe *mw_get_window(Wframe * window, int dx, int dy,
		      int x0, int y0, char * title)
{
     Wframe *ImageWindow;
     char s[255];

     if (strlen(title) > 20) title[20] = '\0';
     sprintf(s,"\"%s\" - MegaWave2",title);

     if ((window == NULL)||(window->win == 0))
	  ImageWindow = (Wframe *) WOpenImageWindow(dx,dy,x0,y0,s);
     else
     {
	  WReOpenImageWindow(window,dx,dy,x0,y0,s);
	  ImageWindow = window;
     }
    
     if ((ImageWindow == NULL) || (ImageWindow->win == (Window) NULL)) 
	  mwerror(FATAL,1,"Cannot run the Wdevice library !\n");

     return(ImageWindow);
}


/*   Add a new window to notify */

void mw_window_notify(Wframe * Win, void * param, 
		      int (* proc)(Wframe *, void *))

{
     int i;

     /* First case: Window has already been notified */
     for (i=0; i<mw_n_windows; i++) 
	  if (mw_ptr_window[i] == Win) 
	  {
	       mw_ptr_window_notify[i] = proc;
	       mw_ptr_param[i] = param;
	       return;
	  }

     /* Second case: new window to notify */
     if (mw_n_windows >= mw_nmax_windows) 
     {
	  mwerror(ERROR, 0,
		  "Cannot notify window #%d: too many windows\n",mw_n_windows);
	  return;
     }

     mw_ptr_window[mw_n_windows] = Win;
     mw_ptr_window_notify[mw_n_windows] = proc;
     mw_ptr_param[mw_n_windows] = param;
     mw_window_runmode[mw_n_windows] = mwrunmode;
     mw_n_windows++;
}

/*   Manage the windows with a call to each notify procs */

void mw_window_main_loop(void)
{
     int i,j,r,cont,event_occured;

     if (mwrunmode <= 2)
     {
	  /* Code for modules for which the window events have to be managed */
	  cont=1;
	  while ((mw_n_windows > 0) && (cont==1))
	  {
	       cont=event_occured=0;
	       for (i=0; i<mw_n_windows; i++)
		    if (mw_ptr_window[i] && mw_ptr_window_notify[i] && 
			(mw_window_runmode[i] <= 2))
		    {
			 r = mw_ptr_window_notify[i](mw_ptr_window[i],mw_ptr_param[i]);
			 /*     A notify function must return a value ....      */
			 /*       0 if there was no event catched               */
			 /*     > 0 if there was an event catched (but Destroy) */
			 /*      -1 if the event Destroy was catched (or 'Q')   */
			 if (r == -1) /* the window has to be destroyed */
			 {
			      WDestroyWindow(mw_ptr_window[i]);	
			      mw_n_windows--;
			      if (mw_n_windows > 0 )
			      {
				   for (j=mw_n_windows-1; j>=i; j--)
				   {
					mw_ptr_window[j] = mw_ptr_window[j+1];
					mw_ptr_param[j] = mw_ptr_param[j+1];
					mw_ptr_window_notify[j] = 
					     mw_ptr_window_notify[j+1];
					mw_window_runmode[j] = 
					     mw_window_runmode[j+1];
				   }
			      }
			      else 
			      {
				   mw_ptr_window[mw_n_windows] = NULL;
				   mw_ptr_param[mw_n_windows] = NULL;
				   mw_ptr_window_notify[mw_n_windows] = NULL;
				   mw_window_runmode[mw_n_windows] = 0;
			      }
			      i--;
			      event_occured=1;
			 }
			 else 
			 {
			      cont=1;
			      if (r != 0) event_occured=1;
			 }
		    }
	       if (event_occured==0) usleep(mwwindelay);
	  }
     }
}

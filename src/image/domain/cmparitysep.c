/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {cmparitysep};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Separate even and odd frames of a cmovie"};
  usage = {
            'e'->e            "flag to extract odd frame first",
	    'l'->l            "flag for linear interpolation",
            input->u          "input movie",
            output<-cmparitysep  "output movie"
          };
*/
/*-- MegaWave2 - Copyright (C) 1994 Jacques Froment. All Rights Reserved. --*/

#include <stdio.h>

/* Include always the MegaWave2 Library */
#include "mw.h"
 

/*---------- Linear Interpolation ----------*/

void linear(dst,a,b,n)
unsigned char *dst,*a,*b;
int n;
{
    for (;n--;dst++,a++,b++)
      *dst=(unsigned char)(((int)*a+(int)*b)/2);
}


/*------------------- F R A M E S E P ----------------------


          SOURCE     ->   DESTINATION        line #

	 (if linear)  \                       2y+frame -2
                      /-->  if frame=1        2y+frame -1
         xxxxxxxxxxx    ->  xxxxxxxxxx        2y+frame
	              \-->  if frame=0        2y+frame +1
	 (if linear)  /                       2y+frame +2
-----------------------------------------------------------*/

Cmovie cmparitysep(u,e,l)
Cmovie	u;
char *e,*l;
{
    Cmovie v;
    Cimage src,dst,prev,*next;
    int nx,ny,frame,frame0,y,adr;

    /* Allocate memory */
    v = mw_new_cmovie();
    if (!v) mwerror(FATAL,1,"Not enough memory.\n");
    
    frame0 = e?1:0;
    src = u->first;
    next = &(v->first);
    prev = NULL;
    nx = src->ncol;
    ny = src->nrow/2;
    frame = frame0;
    /*----- MAIN LOOP */
    while (src) {
	dst = mw_change_cimage(NULL,ny*2,nx);
	if (!dst) mwerror(FATAL,1,"Not enough memory.");
	for (y=0;y<ny;y++) {
	    adr = y*nx*2;
	    /* duplicatation of the original frame */
	    memcpy(dst->gray+adr+nx*frame,src->gray+adr+nx*frame,nx);
	    /* duplication or interpolation for the other frame */
	    if (l && (y!=0 || frame!=1) && (y!=ny-1 || frame!=0)) 
	      linear(dst->gray+adr+nx*(1-frame),
		     src->gray+adr+nx*(2-3*frame),src->gray+adr+nx*frame,nx);
	    else memcpy(dst->gray+adr+nx*(1-frame),src->gray+adr+nx*frame,nx);
	}
	/* link frames */
	*next = dst;
	dst->previous = prev;	
	next = &(dst->next);
	prev = dst;
	frame = 1-frame;
	if (frame==frame0) src = src->next;
    }
    /*---------------*/
    *next = NULL;
    return v;
}

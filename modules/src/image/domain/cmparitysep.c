/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {cmparitysep};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"Separate even and odd fields of a cmovie"};
  usage = {
            'e'->e                "flag to extract odd field first",
	    'l'->l                "flag for linear interpolation",
            input->u              "input movie",
            output<-cmparitysep   "output movie"
          };
*/

#include <stdio.h>
#include <string.h>
#include "mw.h"
#include "mw-modules.h"
 

/*---------- Linear Interpolation ----------*/

static void linear(unsigned char *dst, unsigned char *a, unsigned char *b, int n)
{
    for (;n--;dst++,a++,b++)
      *dst=(unsigned char)(((int)*a+(int)*b)/2);
}


/*------------------- F R A M E S E P ----------------------


          SOURCE     ->   DESTINATION        line #

	 (if linear)  \                       2y+field -2
                      /-->  if field=1        2y+field -1
         xxxxxxxxxxx    ->  xxxxxxxxxx        2y+field
	              \-->  if field=0        2y+field +1
	 (if linear)  /                       2y+field +2
-----------------------------------------------------------*/

Cmovie cmparitysep(Cmovie u, char *e, char *l)
{
    Cmovie v;
    Cimage src,dst,prev,*next;
    int nx,ny,field,field0,y,adr;

    /* Allocate memory */
    v = mw_new_cmovie();
    if (!v) mwerror(FATAL,1,"Not enough memory.\n");
    
    field0 = e?1:0;
    src = u->first;
    next = &(v->first);
    prev = NULL;
    nx = src->ncol;
    ny = src->nrow/2;
    field = field0;
    /*----- MAIN LOOP */
    while (src) {
	dst = mw_change_cimage(NULL,ny*2,nx);
	if (!dst) mwerror(FATAL,1,"Not enough memory.");
	for (y=0;y<ny;y++) {
	    adr = y*nx*2;
	    /* duplicatation of the original field */
	    memcpy(dst->gray+adr+nx*field,src->gray+adr+nx*field,nx);
	    /* duplication or interpolation for the other field */
	    if (l && (y!=0 || field!=1) && (y!=ny-1 || field!=0)) 
	      linear(dst->gray+adr+nx*(1-field),
		     src->gray+adr+nx*(2-3*field),src->gray+adr+nx*field,nx);
	    else memcpy(dst->gray+adr+nx*(1-field),src->gray+adr+nx*field,nx);
	}
	/* link fields */
	*next = dst;
	dst->previous = prev;	
	next = &(dst->next);
	prev = dst;
	field = 1-field;
	if (field==field0) src = src->next;
    }
    /*---------------*/
    *next = NULL;
    return v;
}

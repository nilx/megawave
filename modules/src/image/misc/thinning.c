/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {thinning};
 version = {"2.2"};
 author = {"Jacques Froment, Denis Pasquignon"};
 function = {"homotopic thinning of a B&W cimage"};
 usage = {
   'n':niter->niter
      "number of iterations (default : until idempotent result)",
   'i'->inv
       "inversion (shape is black - 0 - , background is white - 255 -)",
   'm':cmovie_out<-M
       "output cmovie of successive iterations",
   cimage_in->imageI
     "input cimage",
   cimage_out<-imageO
     "output cimage (thinned input)" 
};
*/
/*----------------------------------------------------------------------
 v2.2: return result (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define VOID -1 /* not a gray level value */

/* Put imageO at the end of the movie M */

static void put_in_movie(Cimage imageO, Cmovie M)
{
  Cimage imageN=NULL;
  Cimage imageP;
  
  imageN = mw_change_cimage(imageN,imageO->nrow,imageO->ncol);
  if (imageN == NULL) mwerror(FATAL,1,"Not enough memory.\n");
  mw_copy_cimage(imageO,imageN);
  
  if (M->first == NULL)
    M->first = imageN;
  else
    {
      imageP = M->first;
      while (imageP->next != NULL) { imageP = imageP->next; }
      imageP->next = imageN;
      imageN->previous = imageP;
    }
}

/* One iteration of the thinning algorithm A -> O */
/* Return 1 if O!=A, 0 else.                      */

static int one_thinning(unsigned char *A, unsigned char *O, int dy, int dx, int ONE, int ZERO)
{
  int a,x,y,row,change;

  /* Neighbours conventions (c=center, l=left, r=right, u=up, d=down) 

        | ll | l  | c  | r  | rr |
     -----------------------------
     uu |    |    |    |    |    |
     -----------------------------
     u  |    |    |    |    |    |
     -----------------------------
     c  |    |    | cc |    |    |
     -----------------------------
     d  |    |    |    |    |    |
     -----------------------------
     dd |    |    |    |    |    |
     -----------------------------
     */
  int uull,uul,uuc,uur,uurr; /* row uu */
  int ull,ul,uc,ur,urr;      /* row u */
  int cll,cl,cr,crr;         /* row c */
  int dll,dl,dc,dr,drr;      /* row d */
  int ddll,ddl,ddc,ddr,ddrr; /* row dd */

  change=0;
  for (x=0;x<dx;x++)
    for (y=0;y<dy;y++)
      {
	a=y*dx+x;

        if (A[a] == ONE)  /* cc */
	  {
	    /* --- Compute neighbours --- */
	    /* row uu */
	    if (y<2) uull=uul=uuc=uur=uurr=VOID;
	    else
	      {
		row = a-2*dx;
		if (x>=2) uull=A[row-2]; else uull=VOID;
		if (x>=1) uul=A[row-1]; else uul=VOID;
		uuc=A[row];
		if (x<dx-1) uur=A[row+1]; else uur=VOID;
		if (x<dx-2) uurr=A[row+2]; else uurr=VOID;
	      }
	    /* row u */
	    if (y<1) ull=ul=uc=ur=urr=VOID;
	    else
	      {
		row = a-dx;
		if (x>=2) ull=A[row-2]; else ull=VOID;
		if (x>=1) ul=A[row-1]; else ul=VOID;
		uc=A[row];
		if (x<dx-1) ur=A[row+1]; else ur=VOID;
		if (x<dx-2) urr=A[row+2]; else urr=VOID;
	      }
	    /* row c */
	    row = a;
	    if (x>=2) cll=A[row-2]; else cll=VOID;
	    if (x>=1) cl=A[row-1]; else cl=VOID;
	    if (x<dx-1) cr=A[row+1]; else cr=VOID;
	    if (x<dx-2) crr=A[row+2]; else crr=VOID;
	    /* row d */
	    if (y>=dy-1) dll=dl=dc=dr=drr=VOID;
	    else
	      {
		row = a+dx;
		if (x>=2) dll=A[row-2]; else dll=VOID;
		if (x>=1) dl=A[row-1]; else dl=VOID;
		dc=A[row];
		if (x<dx-1) dr=A[row+1]; else dr=VOID;
		if (x<dx-2) drr=A[row+2]; else drr=VOID;
	      }
	    /* row dd */
	    if (y>=dy-2) ddll=ddl=ddc=ddr=ddrr=VOID;
	    else
	      {
		row = a+2*dx;
		if (x>=2) ddll=A[row-2]; else ddll=VOID;
		if (x>=1) ddl=A[row-1]; else ddl=VOID;
		ddc=A[row];
		if (x<dx-1) ddr=A[row+1]; else ddr=VOID;
		if (x<dx-2) ddrr=A[row+2]; else ddrr=VOID;
	      }

	    /* --- First Mask (4 rotations) --- */

	    if ( ((uc==ZERO) &&                           /* rot 0 */
		(cl==ONE)&&(cr==ONE)&&
		(dl==ONE)&&(dc==ONE)&&(dr==ONE)&&
		(ddl==ONE)&&(ddc==ONE)&&(ddr==ONE)
		  ) || (
		((cr==ZERO)&&                             /* rot -pi/2 */
		 (ull==ONE)&&(ul==ONE)&&(uc==ONE)&&
		 (cll==ONE)&&(cl==ONE)&&
		 (dll==ONE)&&(dl==ONE)&&(dc==ONE))
                        ) || (
                 ((dc==ZERO)&&                            /* rot -pi */
		  (cl==ONE)&&(cr==ONE)&&
		  (ul==ONE)&&(uc==ONE)&&(ur==ONE)&&
		  (uul==ONE)&&(uuc==ONE)&&(uur==ONE))
              	        ) || (                            
		 ((cl==ZERO)&&                            /* rot pi/2 */
		  (uc==ONE)&&(ur==ONE)&&(urr==ONE)&&
		  (cr==ONE)&&(crr==ONE)&&
		  (dc==ONE)&&(dr==ONE)&&(drr==ONE))
              	        ) || (                            

	    /* --- Second Mask (4 rotations) --- */

		 ((uc==ZERO)&&(ur==ZERO)&&(cr==ZERO)&&     /* rot 0 */
		  (cll==ONE)&&(cl==ONE)&&
		  (dll==ONE)&&(dl==ONE)&&(dc==ONE)&&
		  (ddll==ONE)&&(ddl==ONE)&&(ddc==ONE))
              	        ) || (                            
		 ((cr==ZERO)&&(dc==ZERO)&&(dr==ZERO)&&	   /* rot -pi/2 */
		  (uull==ONE)&&(uul==ONE)&&(uuc==ONE)&&
		  (ull==ONE)&&(ul==ONE)&&(uc==ONE)&&
		  (cll==ONE)&&(cl==ONE))
              	        ) || (                            
		 ((cl==ZERO)&&(dc==ZERO)&&(dl==ZERO)&&     /* rot -pi */
                  (uuc==ONE)&&(uur==ONE)&&(uurr==ONE)&&
		  (uc==ONE)&&(ur==ONE)&&(urr==ONE)&&
		  (cr==ONE)&&(crr==ONE))
              	        ) || (                            
		  ((cl==ZERO)&&(uc==ZERO)&&(ul==ZERO)&&    /* rot pi/2 */
		   (cr==ONE)&&(crr==ONE)&&
		   (dc==ONE)&&(dr==ONE)&&(drr==ONE)&&
		   (ddc==ONE)&&(ddr==ONE)&&(ddrr==ONE))
			      )
        	) { O[a]=ZERO; change=1;}
	  } 	 /* if (A[a] == ONE)  */ 
      } /* for ... */
  return(change);
}

Cimage thinning(int *niter, char *inv, Cmovie M, Cimage imageI, Cimage imageO)
{ 
  Cimage imageA=NULL;
  int ncol,nrow;
  int i,ONE,ZERO,change;

  ncol=imageI->ncol;
  nrow=imageI->nrow;

  /* Memory allocation for images */
  imageA = mw_change_cimage(imageA,nrow,ncol);
  if (imageA == NULL) mwerror(FATAL,1,"Not enough memory.\n");
  imageO = mw_change_cimage(imageO,nrow,ncol);
  if (imageO == NULL) mwerror(FATAL,1,"Not enough memory.\n");

  mw_copy_cimage(imageI,imageA);
  mw_copy_cimage(imageI,imageO);
  
  if (M) put_in_movie(imageO,M);	

  if (inv) { ONE=0; ZERO=255; } else { ONE=255; ZERO=0; }

  i=1;
  do
    { 
      change=one_thinning(imageA->gray,imageO->gray,nrow,ncol,ONE,ZERO);
      if (change == 1)
	{
	  mwdebug("iteration %d\n",i);
	  mw_copy_cimage(imageO,imageA);
	  if (M) put_in_movie(imageO,M);	
	  i++;
	}
    }
  while ((change==1)&&((niter==NULL)||(i<=*niter)));

  return(imageO);
}





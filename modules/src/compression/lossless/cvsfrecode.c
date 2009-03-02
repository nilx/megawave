/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {cvsfrecode};
version = {"1.2"};
author = {"Jacques Froment"};
function = {"Encode the change of direction of points in the curves : return bit rate for lossless compression"};
usage = {
 C->C  "input curves",
 N<-N  "output number of points",
 B<-B  "output number of bits to code the curves (without origin points)",
 brate<-cvsfrecode "compression rate (bit per point = B/N)"
        };
*/
/*----------------------------------------------------------------------
 v1.2: upgrade for new kernel (L.Moisan)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for arencode2(), fentropy() */


/* Fill dirchg by Freeman algorithm. Return the minimal size of dirchg.
   The Freeman encoding of a curve uses 4 symbols :
   for the second point (which always exists) : down, up, right, left;
   for other points : straight, right, left and end_of_curve.
*/
static unsigned int Freeman(Curves C, Fimage dirchg)
{
  Curve cv;
  Point_curve point, secondpoint;
  int x0,y0,x1,y1;
  int dir0,dir1,dx,dy,ddir;
  float *dptr;
  unsigned int size;

  size=1;
  dptr = dirchg->gray;
  for (cv = C->first; cv; cv = cv->next)
    {
      x0 = cv->first->x;
      y0 = cv->first->y;
      /* Begin to code at the second point (origin point coded elsewhere) */
      secondpoint = cv->first->next;
      if (secondpoint == NULL)
	mwerror(FATAL,1,
		"Cannot apply Freeman algorithm : a curve contains only one point !\n");
      else
	for (point=secondpoint ; point; point = point->next)
	  {
	    x1 = point->x;
	    y1 = point->y;
	    dx = x1 - x0;
	    dy = y1 - y0;
	    if (abs(dx) + abs(dy) != 1)
	      mwerror(FATAL,1,
		      "Cannot apply Freeman algorithm : two successive points are not neighbours for the 4-connexity !\n");
	    if (dx == 1) /* right */
	      dir1 = 1;  
	    else
	      if (dx == -1) /* left */
		dir1 = 3;
	      else 
		if (dy == 1)  /* down */
		  dir1 = 2;
		else
		  dir1 = 0;  /* up */
	    if (point == secondpoint) /* Code the direction itself */
	      *dptr = (float) dir1;
	    else /* Code the change of direction : straight, right or left */
	      {
		ddir = (dir1 - dir0 + 4) % 4;
		if (ddir == 0) /* straight ahead */
		  *dptr = 0.0;
		else if (ddir == 1) /* right */
		  *dptr = 1.0;
		else if (ddir == 3) /* left */
		  *dptr = 2.0;
		else
		  mwerror(FATAL,1,
			  "Cannot apply Freeman algorithm : a curve comes backs on its path !\n");		  
	      }
	    dptr++;
	    size++;
	    dir0 = dir1;
	    x0 = x1;
	    y0 = y1;
	  } /* end of for (point=secondpoint ; point; point = point->next) */
      *dptr = 3.0; /* end_of_curve symbol */
    }  /* end of for (cv = C->first; cv; cv = cv->next) */
  return(size);
}


double cvsfrecode(Curves C, unsigned int *N, double *B)
{
  Fimage dirchg=NULL;  /* Direction change of points in the curves */
  unsigned int npts;   /* Total number of points in the curves of C (=*N) */
  unsigned int ncvs;   /* Number of curves = Nb of origin points */
  unsigned int size;   /* Minimal size of dirchg */
  int nsymb=4;         /* Number of symbols for arencode2 */
  int optdef;          /* to set the option */
  double arate;        /* Arithmetic coding rate */
  double parate;       /* Predictive arithmetic coding rate */
  double erate;        /* Entropy rate */
  double rate;         /* Best rate */
  double aB,paB,eB;    /* Idem but Bits instead of rate */

  mwdebug("\n--- cvsfrecode --- code the points but the origins...\n");

  if ((!C)||(!C->first)||(!C->first->first))
    mwerror(FATAL,1,"Input curves does not contain any curve or point !\n");

  npts = mw_npoints_curves(C);
  ncvs = mw_length_curves(C);
  mwdebug("Total number of points in the curves : N = %d\n",npts);
  mwdebug("Number of curves (nb of origin points) : %d\n",ncvs);

  /* Initialisations */
  /* The maximal size of dirchg = nb of points but origins + nb of end_of_curves */
  dirchg = mw_change_fimage(dirchg,1,npts);
  if (!dirchg) mwerror(FATAL,1,"Not enough memory\n");
  mw_clear_fimage(dirchg,0.0);

  /* Fill dirchg by Freeman algorithm */
  size=Freeman(C,dirchg);
  dirchg->ncol = size;
  mwdebug("Length of the Freeman buffer = %d\n",size);
  
  mwdebug("Rates in bits per points to encode the curves (but the origins) :\n");

  /* Compute entropy rate of dirchg */
  eB = (double) fentropy(dirchg) * size;
  erate = eB / npts;
  mwdebug("Entropy rate  = %.4f (Nb of Bits = %.0f)\n",erate,eB);

  arencode2(&optdef, NULL, &nsymb, NULL, NULL, NULL, NULL, 
	    dirchg, &arate, (Cimage) NULL);
  aB = arate * size;
  arate = aB / npts;
  mwdebug("Arithmetic rate  = %.4f (Nb of Bits = %.0f)\n",arate,aB);

  arencode2(&optdef, NULL, &nsymb, NULL, &optdef, NULL, NULL, 
	    dirchg, &parate, (Cimage) NULL);
  paB = parate * size;
  parate = paB / npts;
  mwdebug("Predictive Arithmetic rate  = %.4f (Nb of Bits = %.0f)\n",
	  parate,paB);

  rate = erate; *B = eB;
  if (rate > arate) { rate = arate ; *B = aB; }
  if (rate > parate) { rate = parate; *B = paB; }

  *N = npts;

  mwdebug("--- cvsfrecode --- Terminated.\n");
  return(rate);
}



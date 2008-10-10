/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {cvsorgcode};
version = {"1.2"};
author = {"Jacques Froment"};
function = {"Encode the origins of curves : return bit rate for lossless compression"};
usage = {
 C->C  "input curves",
 N<-N  "output number of points",
 B<-B  "output number of bits to code the origin of the curves",
 brate<-cvsorgcode "compression rate (bit per point = B/N)"
        };
*/
/*----------------------------------------------------------------------
 v1.2: upgrade for new kernel (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include "mw.h"
#include "mw-modules.h" /* for arencode2(), fentropy() */


/* Compute in (*ncol, *nrow) the size of the smallest image containing 
   all points which are origins of the curves of C.
   Compute in (*mincol, *minrow) the translation point.
*/

void Get_Size_Org(C,ncol,nrow,mincol,minrow)

     Curves C;
     int *ncol, *nrow;
     int *mincol, *minrow;
     
{
  Curve cv;
  int maxcol,maxrow,x,y;
  
  *mincol = maxcol = C->first->first->x;
  *minrow = maxrow = C->first->first->y;

  for (cv = C->first; cv; cv = cv->next)
    {
      x=cv->first->x;
      y=cv->first->y;
      if (*mincol > x) *mincol=x;
      else if (maxcol < x) maxcol=x;      
      if (*minrow > y) *minrow=y;
      else if (maxrow < y) maxrow=y;      
    }
  *ncol = maxcol - *mincol + 1;
  *nrow = maxrow - *minrow + 1;
}

/* Fill the bitmap with the origins.
   Translate the coordinates of the origins of the points of C so that 
   they enter in the range (0..*ncol-1, 0..*nrow-1).
   Return the number of symbols needed (2 if no duplicated points, > 2 else)
*/

int Fill_Bitmap_With_Origins(bitmap,C,mincol,minrow)
     
     Fimage bitmap;
     Curves C;
     int mincol, minrow;

{
  int ncol,nsymb;
  Curve cv;
  float *ptr;

  nsymb = 2;
  ncol = bitmap->ncol;
  for (cv = C->first; cv; cv = cv->next)
    {
      ptr=&(bitmap->gray[(cv->first->y-minrow)*ncol + (cv->first->x-mincol)]);
      (*ptr)++;
      if (*ptr > nsymb-1) nsymb = (*ptr) + 1;
    }
  return(nsymb);
}


/* Fill distance with the distance of the successive origins,
   from the content of bitmap.
*/

void Fill_Distance(distance,bitmap,npxs)

     Fimage distance;
     Fimage bitmap;
     int npxs;         /* Number of pixels = size of bitmap */
     
{
  float *ptr;
  int n,l,l0,l1;


  n=l0=0;
  for (ptr=bitmap->gray, l=0; l < npxs; ptr++, l++)
    while (*ptr > 0.0) 
      {
	distance->gray[n]=l-l0;
	n++;
	l0=l;
	(*ptr)--;
      }
}

double cvsorgcode(C,N,B)

     Curves C;
     unsigned int *N;
     double *B;
     
{
  Fimage bitmap=NULL;  /* Bitmap to map the origin points of C */
  Fimage distance=NULL;/* Distance of two successive origin points */
  int ncol,nrow;       /* Size of the image containing all origins */
  int mincol, minrow;  /* Translation point to (0,0) */
  unsigned int npts;   /* Total number of points in the curves of C (=*N) */
  unsigned int nopts;  /* Number of origin points in the curves of */
  unsigned int npxs;   /* Total number of pixels in bitmap */
  int nsymb;           /* Number of symbols for arencode2 */
  int optdef;          /* to set the option */
  double arate;        /* Arithmetic coding rate */
  double parate;       /* Predictive arithmetic coding rate */
  double erate;        /* Entropy rate */
  double rate;         /* Best rate */
  double aB,paB,eB;    /* Idem but Bits instead of rate */

  mwdebug("\n--- cvsorgcode --- code the origin points...\n");

  if ((!C)||(!C->first)||(!C->first->first))
    mwerror(FATAL,1,"Input curves does not contain any curve or point !\n");

  /* Compute the size of the image containing all origins */
  Get_Size_Org(C,&ncol,&nrow,&mincol,&minrow);

  npxs = ncol * nrow;
  npts = mw_npoints_curves(C);
  nopts = mw_length_curves(C);
  mwdebug("Total number of points in the curves : N = %d\n",npts);
  mwdebug("Number of origin points (nb of curves) : %d\n",nopts);
  mwdebug("Rates in bits per points to encode the origins of curves :\n");

  /*-- Method #1 : code the bitmap of origin points --*/
  mwdebug("--- Method #1 : code the bitmap of origin points :\n");

  /* Initialisations */
  bitmap = mw_change_fimage(bitmap,nrow,ncol);
  if (!bitmap) mwerror(FATAL,1,"Not enough memory\n");
  mw_clear_fimage(bitmap,0.0);

  /* Fill the bitmap with the origins */
  nsymb = Fill_Bitmap_With_Origins(bitmap,C,mincol,minrow);
  
  /* Compute entropy rate of bitmap */
  eB = (double) fentropy(bitmap) * npxs;
  erate = eB / npts;
  mwdebug("Entropy rate  = %.4f (Nb of Bits = %.0f)\n",erate,eB);

  arencode2(&optdef, NULL, &nsymb, NULL, NULL, NULL, NULL, 
	    bitmap, &arate, (Cimage) NULL);
  aB = arate * npxs;
  arate = aB / npts;
  mwdebug("Arithmetic rate  = %.4f (Nb of Bits = %.0f)\n",arate,aB);

  arencode2(&optdef, NULL, &nsymb, NULL, &optdef, NULL, NULL, 
	    bitmap, &parate, (Cimage) NULL);
  paB = parate * npxs;
  parate = paB / npts;
  mwdebug("Predictive Arithmetic rate  = %.4f (Nb of Bits = %.0f)\n",
	  parate,paB);

  rate = erate; *B = eB;
  if (rate > arate) { rate = arate ; *B = aB; }
  if (rate > parate) { rate = parate; *B = paB; }

  /*-- Method #2 : code the distance between 2 successive points --*/

  mwdebug("\n--- Method #2 : code the distance between two successive origin points :\n");

  /* Initialisations */
  distance = mw_change_fimage(distance,1,nopts);
  if (!distance) mwerror(FATAL,1,"Not enough memory\n");
  mw_clear_fimage(distance,0.0);

  /* Fill distance with the distance of the successive origins */
  Fill_Distance(distance,bitmap,npxs);
  
  /* Compute entropy rate of distance */
  eB = (double) fentropy(distance) * nopts;
  erate = eB / npts;
  mwdebug("Entropy rate  = %.4f (Nb of Bits = %.0f)\n",erate,eB);

  arencode2(&optdef, NULL, NULL, NULL, NULL, NULL, NULL, 
	    distance, &arate, (Cimage) NULL);
  aB = arate * nopts;
  arate = aB / npts;
  mwdebug("Arithmetic rate  = %.4f (Nb of Bits = %.0f)\n",arate,aB);

  arencode2(&optdef, NULL, NULL, NULL, &optdef, NULL, NULL, 
	    distance, &parate, (Cimage) NULL);
  paB = parate * nopts;
  parate = paB / npts;
  mwdebug("Predictive Arithmetic rate  = %.4f (Nb of Bits = %.0f)\n",
	  parate,paB);

  if (rate > erate) {rate = erate; *B = eB; }
  if (rate > arate) { rate = arate ; *B = aB; }
  if (rate > parate) { rate = parate; *B = paB; }

  *N = npts;

  mw_delete_fimage(distance);
  mwdebug("--- cvsorgcode --- Terminated.\n");
  return(rate);
}



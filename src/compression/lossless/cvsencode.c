/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {cvsencode};
version = {"0.1"};
author = {"Jacques Froment"};
function = {"Encode a set of curves : return bit rate for lossless compression"};
usage = {
 's':L->L  "split curves with overlapping parts of length greater than L",
 'o':O<-O  "output the encoded curves (C=O if option -s is not selected)",
 C->C      "input curves",
 N<-N      "output number of points",
 B<-B      "output number of bits to code the curves",
 brate<-cvsencode "compression rate (bit per point = B/N)"
        };
*/

#include <stdio.h>
#include "mw.h"

extern double cvsfrecode();
extern double cvsorgcode();


/* Compute in (*ncol, *nrow) the size of the smallest image containing 
   all points which are in the curves of C.
   Compute in (*mincol, *minrow) the translation point.
*/

void Get_Size_Image(C,ncol,nrow,mincol,minrow)
     
     Curves C;
     int *ncol, *nrow;
     int *mincol, *minrow;
     
{
  Curve cv;
  Point_curve point;
  int maxcol,maxrow,x,y;
  
  *mincol = maxcol = C->first->first->x;
  *minrow = maxrow = C->first->first->y;

  for (cv = C->first; cv; cv = cv->next)
    for (point = cv->first; point; point = point->next)
      {
	x=point->x;
	y=point->y;
	if (*mincol > x) *mincol=x;
	else if (maxcol < x) maxcol=x;      
	if (*minrow > y) *minrow=y;
	else if (maxrow < y) maxrow=y;      
      }
  *ncol = maxcol - *mincol + 1;
  *nrow = maxrow - *minrow + 1;
}

/* Return the number of points between two successive non-multiple point
   p0->next and the next non-successive point p1.
*/

int Seek_Non_Multiple_Point(C,bitmap,mincol,minrow,p0,p1)

     Curves C;
     Cimage bitmap;
     int mincol,minrow;
     Point_curve p0,*p1;
     
{
  Point_curve p;
  int n,ncol;

  ncol = bitmap->ncol;
  p=p0->next;
  *p1=NULL;
  n=0;
  while (p && (bitmap->gray[(p->y-minrow)*ncol + (p->x-mincol)] != 0))
    {
      n++;
      *p1=p;
      p=p->next;
    }
  /* To make sure end of curve contains at least 2 points */
  if ((*p1) && !(*p1)->next)
    {
      *p1 = (*p1)->previous;
      n--;
    }      
  return(n);
}

/* Add in O a curve made by the part of the curve between fpoint and lpoint. 
   If lpoint=NULL, add the points until the end of the curve.
*/

void Add_Curve(O,fpoint,lpoint,bitmap,mincol,minrow)

     Curves O;
     Point_curve fpoint,lpoint;
     Cimage bitmap;
     int mincol,minrow;
     
{
  Curve cv,lcv;
  Point_curve p,p0,p1;
  int ncol;

  ncol = bitmap->ncol;
  cv = mw_new_curve();
  if (!cv) mwerror(FATAL,1,"Not enough memory\n");
  if (!O->first) O->first = cv;
  else
    {
      for (lcv=O->first; lcv && lcv->next; lcv=lcv->next);
      lcv->next = cv;
      cv->previous = lcv;
    }
  p=fpoint;
  p0=NULL;
  for (p=fpoint; p!=NULL; p=p->next)
    {
      p1 = mw_new_point_curve();
      if (!p1) mwerror(FATAL,1,"Not enough memory\n");
      p1->x = p->x;
      p1->y = p->y;
      bitmap->gray[(p->y-minrow)*ncol + (p->x-mincol)] = 1;
      if (!cv->first) cv->first = p1;
      else
	{
	  p0->next=p1;
	  p1->previous=p0;
	}
      p0=p1;
      if (p == lpoint) break;
    }
}

/* Split the curves in case of multiple points */

Curves Split_Curves(O,C,L,bitmap,mincol,minrow)


     Curves O,C;
     int L;
     Cimage bitmap;
     int mincol,minrow;
     
{
  Curve cv;
  Point_curve P, p0, p1, begp;
  int x0,y0,x1,y1;
  int ncol;
  float *dptr;
  unsigned char *bptr;
  unsigned int size;
  
  /* Split curve in 2 curves when 4 successive points or more are 
     already coded by other curves.
  */
  if (!O) O = mw_new_curves();
  if (!O) mwerror(FATAL,1,"Not enough memory\n");
  ncol = bitmap->ncol;
  for (cv = C->first; cv; cv = cv->next)
    {
      /* Begin at the second point (origin point coded elsewhere) */
      begp = cv->first;
      p0 = begp->next;
      while (p0)
	{
	  if (Seek_Non_Multiple_Point(C,bitmap,mincol,minrow,p0,&p1) >= L)
	    {
	      Add_Curve(O,begp,p0,bitmap,mincol,minrow);
	      begp = p1;
	      p0 = begp->next;
	    }
	  else p0=p0->next;
	}
      Add_Curve(O,begp,NULL,bitmap,mincol,minrow);
    }
  return(O);
}

double cvsencode(L,O,C,N,B)
     
     int *L;
     Curves O,C;
     unsigned int *N;
     double *B;

{
  Cimage bitmap=NULL;  /* Bitmap the points of the curves to detect multiple
			  points */
  Curves Oorg;         /* Initial value of O */
  unsigned int npts;   /* Total number of points in the curves of C (=*N) */
  unsigned int ncvs;   /* Number of curves = Nb of origin points */
  int nrow,ncol;       /* Size of bitmap, if used. */
  int minrow, mincol;  /* Translation point in bitmap, if used. */
  double Borg;         /* Nb of bits to code origin of curves */
  double Bfre;         /* Nb of bits for Freeman coding */
  double orgrate;      /* Rate to code origin of curves */
  double frerate;      /* Rate for Freeman coding */

  mwdebug("\n--- cvsencode --- code points of curves...\n");

  if ((!C)||(!C->first)||(!C->first->first))
    mwerror(FATAL,1,"Input curves does not contain any curve or point !\n");

  Oorg = O;
  if (L != NULL)  /* do not code multiple points : split overlapping curves */
    {
      /* Compute the size of bitmap */
      Get_Size_Image(C,&ncol,&nrow,&mincol,&minrow);
      bitmap = mw_change_cimage(bitmap,nrow,ncol);
      if (!bitmap) mwerror(FATAL,1,"Not enough memory\n");
      mw_clear_cimage(bitmap,0);
      /* Compute the new curves, without any large intersection */
      O = Split_Curves(O,C,*L,bitmap,mincol,minrow);
    }
  else O=C;

  orgrate = cvsorgcode(O,N,&Borg);
  frerate = cvsfrecode(O,N,&Bfre);

  *B = Borg + Bfre;
  if (!Oorg) { mw_delete_curves(O); O=NULL; }

  mwdebug("--- cvsencode --- Terminated.\n");
  return(orgrate + frerate);
}


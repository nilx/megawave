/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {fsplit_convex};
   author = {"Lionel Moisan"};
   version = {"1.1"};
   function = {"Split a curve (Flist) into convex components"};
   usage = {    
  'e':eps->eps   "numerical precision (default 1e-7) CHANGE WITH CAUTION",
  'c':ncc<-ncc   "number of convex components obtained (output)",
  in->in         "input (Flist)",
  out<-out       "output (Flists)"
};
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

#define ABS(x) ( (x)>0?(x):-(x) )
#define SGN(x) (((x)==0.0)?0:(((x)>0.0)?(1):(-1)))


/*** return +1, 0 or -1, the sign of det(b-a,c-b) modulo double precision ***/
static int dir(ax,ay,bx,by,cx,cy,eps)
     double ax,ay,bx,by,cx,cy,eps;
{
  double det,prec;
  
  det = (bx-ax)*(cy-by) - (by-ay)*(cx-bx);
  prec = eps*(   ABS(bx-ax)*(ABS(cy)+ABS(by))
	       + ABS(cy-by)*(ABS(bx)+ABS(ax))
	       + ABS(by-ay)*(ABS(cx)+ABS(bx))
	       + ABS(cx-bx)*(ABS(by)+ABS(ay)) );
  if (ABS(det)<=prec) det = 0.0;
  return SGN(det);
}
  	

/*---------------------------- MAIN MODULE -------------------------------*/

Flists fsplit_convex(in,out,ncc,eps)
     Flist   in;
     Flists  out;
     int     *ncc;
     double  *eps;
{
  Flist   l;
  int     j,d1,d2,ni,nr,is_closed,ok;
  float   *p,*q,*pmax,*first,px1,py1,px2,py2,px3,py3,px4,py4,mx,my;
  double  epsilon;

  if (!in) return(NULL);
  if (in->dim!=2) mwerror(FATAL,1,"Dlist dimension must be equal to 2\n");

  epsilon = (eps?*eps:1e-7);

  /*** initialization ***/
  p = in->values;
  pmax = p+in->size*2;
  is_closed = (*p==*(pmax-2) && *(p+1)==*(pmax-1));

  out = mw_change_flists(out,1,1);
  if (!out) mwerror(FATAL,1,"Not enough memory\n");
  out->list[0] = mw_change_flist(out->list[0],2,0,2);
  l = out->list[0];

  /*** dsplit_convex() is evaluated only if in has at least 4 points ***/
  if (in->size<4) {
    mw_copy_flist(in,l);
    return(out);
  }

  q = l->values;
  j = ni = nr = 0; 
  q[j++] = px1 = *(p++); 
  q[j++] = py1 = *(p++);
  px2 = *(p++); py2 = *(p++);
  px3 = *(p++); py3 = *(p++);
  d2 = dir((double)px1,(double)py1,(double)px2,(double)py2,
	   (double)px3,(double)py3,epsilon);

  /*** MAIN LOOP : d1=angle(P1-P2-P3) and d2=angle(P2-P3-P4) ***/
  
  ok = TRUE; first = NULL;
  l = out->list[out->size-1];

  while (ok) {

    d1 = d2;
    px4 = *(p++); py4 = *(p++);
    d2 = dir((double)px2,(double)py2,(double)px3,(double)py3,
	     (double)px4,(double)py4,epsilon);

    if (d1*d2>0) {
      
      /* convex part : store point and increment p */
      if (j/2+1>l->max_size) {mw_realloc_flist(l,j+1); q=l->values;}
      q[j++] = px1 = px2;
      q[j++] = py1 = py2;
      px2 = px3; py2 = py3;
      px3 = px4; py3 = py4;
      
    } else if (d1*d2<0) {
      
      /*** split curve ***/
      if (j/2+2>l->max_size) {mw_realloc_flist(l,j/2+2); q=l->values;}
      q[j++] = px2;
      q[j++] = py2;
      q[j++] = mx = .5*(px2+px3);
      q[j++] = my = .5*(py2+py3);
      l->size = j/2;
      if (p==first) ok = FALSE; 
      if (!first) first=p; 
      if (ok) {
	if (!is_closed || ni) {
	  if (out->size==out->max_size) mw_enlarge_flists(out);
	  l = mw_change_flist(out->list[out->size],5,0,2);
	  out->list[out->size] = l;
	  out->size++;
	  if (!l) mwerror(FATAL,1,"Not enough memory\n");
	}
	q = l->values; j = 0;
	q[j++] = mx;
	q[j++] = my;
	ni++;
	px1 = px2; py1 = py2;
	px2 = px3; py2 = py3;
	px3 = px4; py3 = py4;
      } 

    } else {
      
      /* indefined sign : remove one point */
      if (d1==0 || d2==0) {
	if (d1==0) {
	  px2 = px3; py2 = py3;
	} 
	px3 = px4; py3 = py4;
	d2 = dir((double)px1,(double)py1,(double)px2,(double)py2,
		 (double)px3,(double)py3,epsilon);
	nr++; 
      } else 
	mwerror(FATAL,1,"Dlist contains NaN coordinates !\n");
    }
    
    /* test end of loop */
    if (ok && p==pmax) {
      if (is_closed) p = in->values+2;
      else ok = FALSE;
    }

    /* stop for convex closed curves */
    if (p==in->values+6 && ni==0) ok=FALSE;
  } 
  /*** END OF MAIN LOOP ***/

  if (nr) mwdebug("%d point%s removed\n",nr,(nr==1?"":"s"));

  if (!is_closed) {
    if (j/2+2>l->max_size) {mw_realloc_flist(l,j/2+2); q=l->values;}
    q[j++] = px2;
    q[j++] = py2;
    q[j++] = px3;
    q[j++] = py3;
    l->size = j/2;
  } else if (ni==0) {
    /* convex closed curve */
    if (j==2) {
      q[j++] = q[0];
      q[j++] = q[1];
    } else {
      q[0] = q[j-2];
      q[1] = q[j-1];
    }
    l->size = j/2;
  }
  
  if (!ni) mwdebug("This is a convex%s curve\n",is_closed?" closed":"");
  else mwdebug("This is a non-convex%s curve with %d inflexion point%s.\n",
	       is_closed?" closed":"",ni,(ni==1?"":"s"));
  
  if (ncc) {
    if (is_closed && !ni) ni = 1;
    if (!is_closed) ni++;
    *ncc = ni;
  }

  return(out);
}

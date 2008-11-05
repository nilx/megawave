/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {km_bitangents};
version = {"1.0"};
author = {"Jose-Luis Lisani, Pablo Muse, Frederic Sur"};
function = {"Find bitangent-points of a curve"};
usage = {
   curve->curve         "input curve (2-Flist)",
   curve_IP->curve_IP   "input inflexion points indices (Flist)",
   curve_BP<-curve_BP   "output bitangent-points indices (2-Flist)"
        };
*/

#include<math.h>
#include "mw.h" 


#define qnorm(a,b) ((a)*(a)+(b)*(b))

#define _(a,i,j) ((a)->values[(i)*((a)->dim)+(j)])
/* if a is a Flist including elements constituted of dim components
   then _(a,i,j) is j-th component of i-th element */

#define qEPSDIST6 1E-8F
#define qMINDIST 2.25

static char Closed;  /* 1 if cloised curve, 0 otherwise */ 
static int N_Points;  /* number of points of the curve */
static float hmax;  /* maximal distance between bitangent and curve */


/* check the position of a point w.r.t. a line */ 
static float halfplane(x1, y1, x2, y2, x, y)
     float x1,y1,x2,y2,x,y;
{
  float u, v;
  double dx, dy;
  u=(x2-x1)*(y2-y);
  v=(x2-x)*(y2-y1);
  if ((u-v)*(u-v)<qEPSDIST6) 
     return(0); 
  else 
  {
     dx = x2 - x1;
     dy = y2 - y1;
     return (u-v) / sqrt(dx * dx + dy * dy);
  }
}


/* compute the first triplet of three consecutive points following i in the direction type */
static unsigned char get_first_triplet(iFirst, i, iP, iN, iLast, type)
     int iFirst;
     int *i, *iP, *iN;
     int iLast;
     unsigned char type;
{
  *iP=iFirst;
  if (type == 1) {
    if ((iLast == 0) && (Closed)) iLast=1;
    *i=iFirst+1;
    if (*i > N_Points-1) {
      if (Closed) *i=1;
      else return 0;
    }
    if (*i == iLast) return 0;
    *iN=*i+1;
    if (*iN > N_Points-1) {
      if (Closed) *iN=1;
      else return 0;
    }
  } else {
    if ((iLast == N_Points-1) && (Closed)) iLast=N_Points-2;
    *i=iFirst-1;
    if (*i < 0) {
      if (Closed) *i=N_Points-2;
      else return 0;
    }
    if (*i == iLast) return 0;
    *iN=*i-1;
    if (*iN < 0) {
      if (Closed) *iN=N_Points-2;
      else return 0;
    }
  }
  return 1;
}

static unsigned char get_next_triplet(i, iP, iN, last, type)
     int *i, *iP, *iN; 
     int last;
     unsigned char type;
{
  *iP=*i;
  *i=*iN;
  
  if (type == 1) {
    if ((last == 0) && (Closed)) last=1;
    *iN=*iN+1;
    if (*iN > N_Points-1) {
      if (Closed) *iN=1;
      else return 0;
    }
  } else {
    if ((last == N_Points-1) && (Closed)) last=N_Points-2;
    *iN=*iN-1;
    if (*iN < 0) {
      if (Closed) *iN=N_Points-2;
      else return 0;
    }
  }
  
  if (*i == last) return 0;
  
  return 1;
}

/* check if the points lie on the same halfplane */
static unsigned char check_triplet(fcrv, i, j, k1, k2)
     Flist fcrv;
     int i,j, k1, k2;
{
  float x1, y1, x2, y2, xk1, yk1, xk2, yk2;
  float h1, h2;

  x1=_(fcrv,i,0);
  y1=_(fcrv,i,1);
  x2=_(fcrv,j,0);
  y2=_(fcrv,j,1);
  xk1=_(fcrv,k1,0);
  yk1=_(fcrv,k1,1);
  xk2=_(fcrv,k2,0);
  yk2=_(fcrv,k2,1);
  
  h1=halfplane(x1, y1, x2, y2, xk1, yk1);
  h2=halfplane(x1, y1, x2, y2, xk2, yk2);
  
  return (h1*h2 >= 0); 
}

/* check if the curve cross the straight line between points i1 and i2 */
static unsigned char check_crossing_curve(fcrv, i1, i2)
     Flist fcrv;
     int i1,i2;
{
  int i;
  float h, h_prev;
  float x1, y1, x2, y2, x, y;
  
  x1=_(fcrv,i1,0);
  y1=_(fcrv,i1,1);
  x2=_(fcrv,i2,0);
  y2=_(fcrv,i2,1);
  h_prev=0;
  hmax=0;
  
  if (i2 > i1) {
    for (i=i1+1; i < i2; i++) {
      x=_(fcrv,i,0);
      y=_(fcrv,i,1);
      h=halfplane(x1, y1, x2, y2, x, y);
      if (h*h_prev < 0) return 1;
      if (h*h>hmax*hmax) hmax=h;
      h_prev=h;
    }
  } else {
    for (i=i1+1; i < N_Points-1; i++) {
      x=_(fcrv,i,0);
      y=_(fcrv,i,1);
      h=halfplane(x1, y1, x2, y2, x, y);
      if (h*h_prev < 0) return 1;
      if (h*h>hmax*hmax) hmax=h;
      h_prev=h;
    }
    for (i=0; i < i2; i++) {
      x=_(fcrv,i,0);
      y=_(fcrv,i,1);
      h=halfplane(x1, y1, x2, y2, x, y);
      if (h*h_prev < 0) return 1;
      if (h*h>hmax*hmax) hmax=h;
      h_prev=h;
    }
  }
  return 0;
}



static unsigned char get_bitangent(curve_IP,curve_BP,fcrv,IP1,IP2,nBts,nBts_max,nIPs)
     Flist curve_IP;
     Flist curve_BP;
     Flist fcrv;
     int IP1, IP2;
     int *nBts, nBts_max, nIPs;
{
  int iFirst, iLast, jFirst, jLast, last, i, iP, iN, j, jP, jN;
  unsigned char change_i, change_j, same_convex, ok;
  
  iFirst=_(curve_IP,IP1,0);
  if (IP1 == 0) {
    if (Closed) iLast=_(curve_IP,nIPs-1,0);
    else iLast=0;
  } else iLast=_(curve_IP,IP1-1,0);
  jFirst=_(curve_IP,IP2,0);
  if (IP2 == nIPs-1) {
    if (Closed) jLast=_(curve_IP,0,0);
    else jLast=N_Points-1;
  } else jLast=_(curve_IP,IP2+1,0);
  
  if (iLast == jFirst) same_convex=1;
  else same_convex=0;
  
  if (!get_first_triplet(iFirst, &i, &iP, &iN, iLast, 0)) return 1;
  if (!get_first_triplet(jFirst, &j, &jP, &jN, jLast, 1)) return 1;
  
  do {
    change_i=0;
    do {
      ok=check_triplet(fcrv, i, j, iP, iN);
      if (!ok) {
	if (same_convex==1) last=j; else last=iLast; 
	if (!get_next_triplet(&i, &iP, &iN, last, 0)) return 1;
	change_i=1;
      }
    } while (!ok);
    change_j=0;
    do {
      ok=check_triplet(fcrv, i, j, jP, jN);
      if (!ok) {
	if (same_convex==1) last=i; else last=jLast; 
	if (!get_next_triplet(&j, &jP, &jN, last, 1)) return 1;
	change_j=1;
      }
    } while (!ok);
    
    if ((!change_i) && (!change_j)) {
      if ((i==j)||(i==(j+1)%N_Points)||(i==(j-1)%N_Points)) return 1;
      if (check_crossing_curve(fcrv, i, j)) return 1;
      if (*nBts >= nBts_max) return 0;
      if (hmax*hmax>qMINDIST) {
	_(curve_BP,curve_BP->size,0)=i;
	_(curve_BP,curve_BP->size++,1)=j;
	(*nBts)++;
      }
      if (same_convex==1) last=j; else last=iLast; 
      if (!get_next_triplet(&i, &iP, &iN, last, 0)) return 1;
    }
  } while (1);
  
  return 1;
}


/*------------------------------ MAIN MODULE ------------------------------*/

Flist km_bitangents(curve,curve_IP,curve_BP)
     Flist curve;
     Flist curve_IP;
     Flist curve_BP; 
{  
  int i, i_next, i_next_next;
  int nBts, nBts_max, nIPs;
  unsigned char iterate, out_of_bounds;

  N_Points= (int) curve->size;
  if (N_Points==0) return NULL;
  Closed=((_(curve,0,0)==_(curve,N_Points-1,0))
	  &&(_(curve,0,1)==_(curve,N_Points-1,1))); 
  nIPs=curve_IP->size;
  nBts_max=nIPs*nIPs;
  nBts=0;
  if ((curve_BP=mw_change_flist(curve_BP,nBts_max,0,2))==NULL) 
    mwerror (FATAL,1,"error, not enough memory\n");
  
  /* search for bitangent: one tangent in a convex piece of the curve, 
     the other two convex pieces further (because there cannot be 
     a bitangent between two consecutive convex pieces) */

  do {
    out_of_bounds=0;
    for (i=0; (i < nIPs) && (!out_of_bounds); i++) {
      i_next=i+1;
      if ((i_next >= nIPs) && (Closed)) i_next=0;
      iterate=(i_next < nIPs); 
      if (Closed) {
        i_next_next=i_next+1;
        iterate=iterate &&(i_next_next != i+1);
        if ((i_next_next >= nIPs) && (Closed)) i_next_next=0;
      }
      while (iterate) { 
	if (get_bitangent(curve_IP,curve_BP,curve,i,i_next,&nBts,nBts_max,nIPs)) {
          i_next++;
          if ((i_next >= nIPs) && (Closed)) i_next=0;
          iterate=(i_next < nIPs);
          if (Closed) {
            i_next_next=i_next+1;
            iterate=iterate &&(i_next_next != i+1);
            if (i_next_next >= nIPs) i_next_next=0;
                }
          if (iterate) {
            i_next++;
            if ((i_next >= nIPs) && (Closed)) i_next=0;
            iterate=(i_next < nIPs); 
            if (Closed) {
              i_next_next=i_next+1;
              iterate=iterate &&(i_next_next != i+1);
              if ((i_next_next >= nIPs) && (Closed)) i_next_next=0;
                     }
          }
        } else {
          iterate=0;
          out_of_bounds=1;
        }
      }
    }
    
    if (out_of_bounds) {
      nBts_max*=2; 
      if ((curve_BP=mw_change_flist(curve_BP,nBts_max,0,2))==NULL) 
	mwerror (FATAL,1,"error, not enough memory\n"); 
      nBts=0;
    }
    
  } while (out_of_bounds);
  
  mw_realloc_flist(curve_BP,curve_BP->size);
  
  return(curve_BP);
}

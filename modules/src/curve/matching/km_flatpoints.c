/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {km_flatpoints};
version = {"1.0"};
author = {"Jose-Luis Lisani, Pablo Muse, Frederic Sur"};
function = {"Find flat points of a curve"};
usage = {
   curve->curve         "input curve (2-Flist)",
   curve_IP->curve_IP   "input inflexion points indices (Flist)",
   curve_FP<-curve_FP   "output flat points indices (1-Flist)",
   angle->angle         "maximum angle variation allowed (input)",
   dist->dist           "minimum length (input)"
        };
*/

#include<math.h>
#include "mw.h" 


#define qnorm(a,b) ((a)*(a)+(b)*(b))

#define _(a,i,j) ((a)->values[(i)*((a)->dim)+(j)])
/* if a is a Flist including elements constituted of dim components
   then _(a,i,j) is j-th component of i-th element */

#define EPSDIST 1E-4F

static char Closed;
static int N_Points;


/* cosinus of the angle between two vectors*/
static float cosangle(float vx1, float vy1, float vx2, float vy2)
{
  float c, qd1, qd2;
  
  qd1=vx1*vx1+vy1*vy1;
  qd2=vx2*vx2+vy2*vy2;
  if ((qd1 < EPSDIST) || (qd2 < EPSDIST)) {
    return 0.0f;
  }
  c=(vx1*vx2+vy1*vy2)/((float)sqrt(qd1*qd2));
  return c;
}


/* next index after i in the direction type. 
  iLast is the last index of the curve */

static int get_next_index(int i, int iLast, unsigned char type)
{
  int i_next;
  
  if (type == 1) {
    if ((Closed) && (iLast == 0)) iLast=N_Points-1;
  } else {
    if ((Closed) && (iLast == N_Points-1)) iLast=0;
  }
  
  if (type == 1) {
    i_next=i+1;
    if ((i_next > N_Points-1) && (!Closed)) return -1;
    if ((i_next > N_Points-1) && (Closed)) i_next=1;
  } else {
    i_next=i-1;
    if ((i_next < 0) && (!Closed)) return -1;
    if ((i_next < 0) && (Closed)) i_next=N_Points-2;
  }
  if (i_next == iLast) return -1;
  return i_next;
}


/* next point after i1 whose angle with tangent in i1 is less than angle, 
   in the direction type. */ 

static int get_next_point_angle(Flist fcrv, int i1, int i_max, float angle_max, unsigned char type)
{
  float cos_angle_max;
  int i, i_next;
  float x1, y1, x2, y2, vx0, vy0, vx, vy, alpha;
  
  if (type == 1) {
    if ((Closed) && (i_max == 0)) i_max=N_Points-1;
  } else {
    if ((Closed) && (i_max == N_Points-1)) i_max=0;
  }
  
  i_next=get_next_index(i1, i_max, type); 
  if (i_next < 0) return -1;
  x1=_(fcrv,i1,0);
  y1=_(fcrv,i1,1);
  x2=_(fcrv,i_next,0);
  y2=_(fcrv,i_next,1); 
  vx0=x2-x1;
  vy0=y2-y1;
  cos_angle_max=(float)cos(angle_max);
  
  do {
    i=i_next;
    x1=x2;
    y1=y2;
    i_next=get_next_index(i, i_max, type);
    if (i_next < 0) return -1;
    x2=_(fcrv,i_next,0);
    y2=_(fcrv,i_next,1);
    vx=x2-x1;
    vy=y2-y1;
    alpha=cosangle(vx0, vy0, vx, vy);
    
  } while (alpha > cos_angle_max);
  
  return i;
}


/* length of an arc between index i1 and i2 */

static float arc_length(Flist fcrv, int i1, int i2)
{
  int i;
  float x1, y1, x2, y2, length;
  
  x1=_(fcrv,i1,0);
  y1=_(fcrv,i1,1);
  length=0;
  if (i2 > i1) {
    for (i=i1+1; i <= i2; i++) {
      x2=_(fcrv,i,0);
      y2=_(fcrv,i,1);
      length+=(float) sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
      x1=x2;
      y1=y2;
    }
  } else {
    for (i=i1+1; i < N_Points; i++) {
      x2=_(fcrv,i,0);
      y2=_(fcrv,i,1);
      length+=(float) sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
      x1=x2;
      y1=y2;
    }
    for (i=0; i <= i2; i++) {
      x2=_(fcrv,i,0);
      y2=_(fcrv,i,1);
      length+=(float) sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
      x1=x2;
      y1=y2;
    }
  }
  return length;
}


/* get central index between i1 and i2 (closed curve) */

static int get_central_index(int i1, int i2)
{
  int i;
  
  if (i2 > i1) return ((i1+i2)/2);
  i=(N_Points-1-i1+i2)/2;
  if (i1+i > N_Points-1) return (i1+i-(N_Points-1));
  return (i1+i);
}


/* flat points of a closed curve without inflexion point */

static void get_flat_points_closed_curve(Flist curve_FP, Flist fcrv, float dist, float angle)
{
  int i, i_angle, i_next, i1, i2, i2_limit;
  float d;
  
  i1=i2=0;
  i_next=i1;
  
  i2_limit=i2;
  
  do {
    i=i_next;
    i_angle=get_next_point_angle(fcrv, i, i2_limit, angle, 1);
    if (i_angle < 0) return;
    d=arc_length(fcrv, i, i_angle);
    if (d > dist) {
      _(curve_FP,curve_FP->size++,0)=get_central_index(i, i_angle);
      i_next=get_next_point_angle(fcrv, i_angle, i2, angle/2, 1);
      if (i_next < 0) return;
      i2=get_next_point_angle(fcrv, i2, i_angle, angle/2, 0);
      if (i2 < 0) return;
      if (curve_FP->size == 1) i2_limit=i2;
    } else {
      i_next=get_next_index(i,0,1);
      if (i_next < 0) return;
      if (curve_FP->size == 0) i2=i_next;
      else i2=i2_limit;
    }
    
  } while (1);
}


/* flat points of a convex piece of curve */

static void get_flat_points_convex(Flist curve_FP, Flist fcrv, int iFirst, int iLast, float dist, float angle, unsigned char noIPs)
{
  int i1, i2, i, i_next, i_angle;
  float d;
  
  if (!noIPs) { 
    i1=get_next_point_angle(fcrv, iFirst, iLast, angle/2, 1);
    if (i1 < 0) return; 
    i2=get_next_point_angle(fcrv, iLast, i1, angle/2, 0);
    if (i2 < 0) return; 
  } else {
    i1=iFirst;
    i2=iLast;
  }
  if ((noIPs) && (Closed)) {
    get_flat_points_closed_curve(curve_FP,fcrv, dist,angle);
    return;
  }
  i_next=i1;
  do {
    i=i_next;
    i_angle=get_next_point_angle(fcrv, i, i2, angle, 1);
    if (i_angle < 0) return;
    d=arc_length(fcrv, i, i_angle);
    if (d > dist) {
      _(curve_FP,curve_FP->size++,0)=get_central_index(i, i_angle);
      i_next=get_next_point_angle(fcrv, i_angle, i2, angle/2, 1);
      if (i_next < 0) return;
      i2=get_next_point_angle(fcrv, i2, i_next, angle/2, 0);
      if (i2 < 0) return;
    } else {
      i_next=get_next_index(i, i2, 1);
      if (i_next < 0) return;
    }
  } while (1);
}


/*------------------------------ MAIN MODULE ------------------------------*/

Flist km_flatpoints(Flist curve, Flist curve_IP, Flist curve_FP, float angle, float dist)
{ 
  int i;
  
  if ((curve_FP=mw_change_flist(curve_FP,curve->size,0,1))==NULL) 
    mwerror (FATAL,1,"error, not enough memory\n");
  N_Points=curve->size;
  if (N_Points==0) return NULL;
  Closed=((_(curve,0,0)==_(curve,N_Points-1,0))
	  &&(_(curve,0,1)==_(curve,N_Points-1,1)));
  
  /* search for flat points in convex pieces of curve */
  if (curve_IP->size!=0) { 
    
    if (!Closed)
      get_flat_points_convex(curve_FP, curve, 0, (int)_(curve_IP,0,0), dist, angle, 0); 
    
    for (i=0; i < curve_IP->size-1; i++)
      get_flat_points_convex(curve_FP, curve, (int)_(curve_IP,i,0), (int)_(curve_IP,i+1,0), dist, angle, 0);
    
    if (!Closed)
      get_flat_points_convex(curve_FP, curve, (int)_(curve_IP,curve_IP->size-1,0), N_Points-1, dist, angle, 0); 
    else 
      get_flat_points_convex(curve_FP, curve, (int)_(curve_IP,curve_IP->size-1,0), (int)_(curve_IP,0,0), dist, angle, 0);
    
  } else {
    get_flat_points_convex(curve_FP, curve, 0, N_Points-1, dist, angle, 1);
  }
  mw_realloc_flist(curve_FP,curve_FP->size);
  
  return(curve_FP);
}


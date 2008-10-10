/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {ksplines};
  version = {"1.4"};
  author = {"Jacques Froment"};
  function = {"Generate a set of splines-curves from control points-curves"};
  usage = {
'j':[order=3]->C           "order of the spline (default: 3 = cubic spline)",
's':[step=0.1]->Step       "step between two parameter values t",
cvs_control_pts->ctrl_pts  "set of curves of control points (curves input)",
ksplines<-splines          "B-spline curves (curves output)"
  };
*/
/*----------------------------------------------------------------------
 v1.2: corrected allocation bug + minor modifications (L.Moisan)
 v1.3: upgrade for new kernel (L.Moisan)
 v1.4 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "mw.h"

#define min(A,B)     (((A)>(B)) ? (B) : (A))
#define max(A,B)     (((A)>(B)) ? (A) : (B))

/* Return the maximum number of points in a set of curves */

int give_max_number_of_points_in_curves(curves)

Curves curves;

{
  Curve cv;
  int n;

  n=0;
  for (cv=curves->first; cv; cv=cv->next)
    n = max(n,mw_length_curve(cv));
  return(n);
}


void init_nodes(X,j,n)   /* Init the node vector */
     
     int *X,j,n;
     
{
  int i;

  for (i=0; i<j; i++) X[i]=0;
  for (i=j; i<=n; i++) X[i] = i-j+1;
  for (i=n+1; i<= j+n; i++) X[i] = n-j+2;

  if (mwdbg == 1)
    {
      mwdebug("\tNode Vector:\n");
      for (i=0;i<=j+n;i++) mwdebug("\t\tX[%d]=%d\n",i,X[i]);
    }
}

/* Compute the spline */

void compute_spline(S,P,X,N,C,M,NPC,Step)
     
     Curve S;    /* Spline to compute */
     Curve P;    /* Control points */
     int *X;     /* Node vector */
     Fimage N;   /* Spline function */
     int C;      /* Order j of the spline */
     int M,NPC;  /* M = NPC+1 = number of control pts */
     float Step; /* Step in the discretization of the spline */

{
  int k;     /* number of the point in the spline */
  int a,i,j;
  float t;   /* time in the discretization of the spline */
  float x,y; /* Real (x,y) coordinates */
  long xx,yy,xx0,yy0; /* Quantized (x,y) coordinates */
  Point_curve pc,newp,oldp;
  float d,e,s,v;

  mwdebug("\n*** Enter compute_spline ***\n");

  mwdebug("Size of array N = (%d,%d)\n",N->ncol,N->nrow);

  k=-1;
  newp=oldp=NULL;

  for (a=C-1; a<=NPC; a++)
    {
      mw_clear_fimage(N,0.0); /* N(i,j)=0 for all i,j */
      for (i=0;i<NPC+C;i++)
	if ((a == i) && (X[i] != X[i+1])) mw_plot_fimage(N,i,1,1.0);

      for (t=X[a]; t<= X[a+1]-Step+0.1*Step; t+=Step)
	{
	  for (j=2; j<=C; j++)
	    {
	      x=y=0.0;
	      for (i=0, pc=P->first; i<= NPC; i++, pc=pc->next)
		{
		  /*
		    mwdebug("\tControl Point #%d\n",i);
		  */

		  v=mw_getdot_fimage(N,i,j-1);
		  if (v == 0.0) d=0.0;
		  else d=((t-X[i])*v)/(X[i+j-1]-X[i]);
		  v=mw_getdot_fimage(N,i+1,j-1);
		  if (v == 0.0) e=0.0;
		  else e=((X[i+j]-t)*v)/(X[i+j]-X[i+1]);
		  s=d+e;
		  mw_plot_fimage(N,i,j,s);
		  x += s*pc->x;
		  y += s*pc->y;
		}
	    }
	  k++;
	  xx = floor((double) x + .5);
	  yy = floor((double) y + .5);
	  mwdebug("\tSpline point #%d computed: (%f,%f) quantized to (%d,%d)\n",k,x,y,xx,yy);

	  if ((k==1) || ((xx != xx0) || (yy != yy0)))
	    {
	      /* Create a new spline point */
	      oldp = newp;
	      newp = mw_new_point_curve();
	      if (newp == NULL)
		{
		  mwerror(ERROR,1,"Not enough memory to create spline point #%d: Abort.\n",k);
		  return;
		}
	      if (S->first == NULL) S->first = newp;
	      if (oldp != NULL) oldp->next = newp;
	      newp->previous = oldp;
	      newp->next = NULL;
	      
	      newp->x = xx;
	      newp->y = yy;
	    }
	  else
	    {
	      k--;
	      mwdebug("\tSame point than before: don't add it\n");
	    }
	  xx0 = xx;
	  yy0 = yy;
	}
    }

  /* Add the last control point to the spline curve */
  k++;
  for (i=0, pc=P->first; i< NPC; i++, pc=pc->next);

  if ((k==1) || ((pc->x != xx0) || (pc->y != yy0)))
    {  
      mwdebug("\tPut the last control point (%d,%d) as the spline point #%d\n",
	      pc->x,pc->y,k);

      /* Create a new spline point */
      oldp = newp;
      newp = mw_new_point_curve();
      if (newp == NULL)
	{
	  mwerror(ERROR,1,"Not enough memory to create spline point #%d: Abort.\n",k);
	  return;
	}
      if (S->first == NULL) S->first = newp;
      if (oldp != NULL) oldp->next = newp;
      newp->previous = oldp;
      newp->next = NULL;
  
      newp->x = pc->x;
      newp->y = pc->y;
    }

  mwdebug("*** Leaving compute_spline ***\n\n");

}


/*------------------------------ Main function ------------------------------*/

void ksplines(C,Step,ctrl_pts,splines)

     int *C;      /* Order j of the spline */
     float *Step; /* Step in the discretization of the spline */
     Curves ctrl_pts,splines;
     
{ Curve P,newsp,oldsp;    
  int *X;            /* Node vector */
  int cp_number;     /* Control points number */
  int sp_number;     /* Spline number */
  int M,NPC;         /* M = NPC+1 = number of control pts */
  Fimage N;          /* Spline function as a Fimage */

  if (*C < 2) mwerror(USAGE,1,"Illegal spline order j=%d\n",*C);

  splines = mw_change_curves(splines);
  if (splines == NULL)
    mwerror(FATAL,1,"Not enough memory\n");
  newsp=oldsp=NULL;
  sp_number=0;

  M = give_max_number_of_points_in_curves(ctrl_pts);
  mwdebug("Maxi. number of control points = %d\n",M);

  X = (int *) malloc((M+*C)*sizeof(int));
  if (X == NULL) mwerror(FATAL,1,"Not enough memory\n");

  N = mw_change_fimage(NULL,*C+1,M+*C);
  if (N == NULL) mwerror(FATAL,1,"Not enough memory\n");

  for (P=ctrl_pts->first, cp_number=1; P; P=P->next, cp_number++)
    {
      /* Control Points P #cp_number */
      
      mwdebug("-> Scanning Control points number #%d\n",cp_number);

      M = mw_length_curve(P);
      NPC = M-1;
      mwdebug("\tM=%d \t NPC=%d\n",M,NPC);

      if (M < *C) 
	mwerror(WARNING,1,"Cannot generate spline associated to the control\npoints curve #%d: only %d control point(s) found !\n",cp_number,M);
      else
	{
	  /* Create a new spline structure */
	  oldsp = newsp;
	  newsp = mw_new_curve();
	  if (newsp == NULL) mwerror(FATAL,1,"Not enough memory\n");
	  if (splines->first == NULL) splines->first = newsp;
	  if (oldsp != NULL) oldsp->next = newsp;
	  newsp->previous = oldsp;
	  newsp->next = NULL;
	  sp_number++;
	  mwdebug("\t New spline number #%d created\n",sp_number);

	  /* Fill the node vector */
	  init_nodes(X,*C,NPC);

	  /* Resize the array N(i,j) */
	  N = mw_change_fimage(N,*C+1,M+*C);	  
	  /* Compute the spline */
	  compute_spline(newsp,P,X,N,*C,M,NPC,*Step);
	}
    }

  free(X);
  mw_delete_fimage(N); 
}








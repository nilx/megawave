/*--------------------------- MegaWave2 Command -----------------------------*/
/* mwcommand
name = {disocclusion};
version = {"2.0"};
author = {"Simon Masnou"};
function = {"Disocclusion using global minimisation of cost by dynamic recursive programing"};
usage = {
  'e':[energy_type=1]->energy_type    
       "Energy of a level line : 0 = only length, 1 = only angle (default), otherwise = angle+length",
  'a'->angle        "If used then the orientation of each entering level line is computed more accurately on a ball of radius 4",
  input->Input      "Input occluded Cimage",
  holes->Holes      "Input Fimage containing the only occlusions",
  output<-Output    "Output disoccluded Cimage"
  };
*/
/*----------------------------------------------------------------------
 v2.0: revised version, added -e option, no more -m option (S.Masnou)
----------------------------------------------------------------------*/


#include <math.h>
#include <stdio.h>
#include "mw.h"



/***************************************************************************/
/** This program performs disocclusion of simply connected sets. Any set with
    a hole will be automatically filled before the disocclusion process

    Crucial convention : x denotes VERTICAL axis and y HORIZONTAL axis !! **/
/***************************************************************************/

/**********************************************************************/
/* IMPORTANT NOTE : at the end of the labeling process, labels of each 
   connected component are sorted increasingly according to their
   order of appearance */
/**********************************************************************/

/* We assume than there are no more than LABELS_NUMBER labels obtained at first pass
 (before updating) */
#define LABELS_NUMBER 100000

int u_compar_i(u,v)  /*  Called by function qsort for sorting decreasingly */
int *u,*v;
  {
    if ((*u)<(*v)) return (1);
    if ((*u)==(*v)) return (0);
    return (-1);
  }

/****************************************************************************/
/*
The algorithm used here is the following : let us consider the case where we want
to compute the connected components of the set of points with value g (the principle
is analogous for the computation of the connected components of the complement).
We create a subsidiary image, one pixel larger in each direction than the input 
image, where all pixels of input frame with value g have value 1, while the others 
are set to 0. Then we scan the interior of this subsidiary image (borders are not
taken into account) from top to bottom and left to right (only one scan is enough). 
Each pixel with value 1 whose 4 upper and left pixels (in case of 8-connectivity) 
are 0 is given a new label (as if it was a new component).
If some of these 4 pixels are nonzero then the current pixel is given the greatest label
among these 4 and we update the transcoding table which tells us which label is 
connected to which one, i.e. which components are part of the same connected components.
Once the image has been totally scanned, we simply have to replace the value (label)
of each nonzero pixel by the smallest label to which it is associated.
*/
/****************************************************************************/

void mise_a_jour_transcode(transcode,a,b)

int transcode[LABELS_NUMBER];
int a,b;

  {
    if (!b) return;
    if (transcode[a]==0)
      transcode[a]=b;
    else
      {
	if (b!=transcode[a])
	  if (b>transcode[a])
	    mise_a_jour_transcode(transcode,b,transcode[a]);
	  else
	    {
	      mise_a_jour_transcode(transcode,transcode[a],b);
	      transcode[a]=b;
	    }
      }
  }
    
/****************************************************************************/

/* In the transcoding table, a label l1 may be associated with another label l2,
itself associated with a third label l3, etc... The following function simply
performs a recursive association of each label l1,l2,l3 with the smallest possible
label in the chain. */

void refresh(transcode,refresh_transcode,i,first)

int transcode[LABELS_NUMBER],refresh_transcode[LABELS_NUMBER];
int i;
int *first;  
    
  {
    if (transcode[i])
      {
	refresh(transcode,refresh_transcode,transcode[i],first);
	refresh_transcode[i]=*first;
      }
    else
      *first=i;
  }

/****************************************************************************/

void fconnected(In,line,col,FOREGROUND,NUMBER,not_writing_on,complement,connectivity)
float *In;
int line,col;
float FOREGROUND;
int *NUMBER;
char *not_writing_on,*complement,*connectivity;

  {
    int Line=line+2,Col=col+2;
    int *Output;
    register float *ptrin;
    register int *ptrout;
    register int dx,dy,i;
    int kernel[4];
    int label,first;
    int transcode[LABELS_NUMBER]; /* transcodage table obtained at first pass and constructed 
				     following the rule : if a point is connected with two different labels 
				     (more than two is impossible) one writes in the table the link
				     label_max->label_min if label_max is not yet linked, else, say we
				     already have label_max->label' one makes by recursivity 
				     label'->label_min or label_min->label' depending on 
				     max(label',label_min)
				     */
    int refresh_transcode[LABELS_NUMBER];  /* same as transcode except that each label is linked with
					      the smallest value in the chain to which it belongs */
    int normalize_transcode[LABELS_NUMBER];
    register int *ptr_trans,*ptr_ref,*ptr_norm;
    int norme;

    /* The frame used for processing is 2 pixels wider and higher than the input to avoid
       problems with borders */

    Output=(int*)malloc(Line*Col*sizeof(int));
    if (Output==NULL)
      mwerror(FATAL,1,"Not enough memory !\n");
    
    for (i=0,ptr_trans=transcode,ptr_ref=refresh_transcode,
	 ptr_norm=normalize_transcode;i<LABELS_NUMBER;i++,ptr_trans++,ptr_ref++,ptr_norm++)
      {*ptr_trans=0;*ptr_ref=0;*ptr_norm=0;}
    
    for (dy=0,ptrout=Output;dy<Col;dy++,ptrout++) *ptrout=0; 
    for (dx=2,ptrin=In;dx<Line;dx++)
      {
	*ptrout=0;ptrout++;
	for (dy=2;dy<Col;dy++,ptrout++,ptrin++)
	  if (complement)
	    if (*ptrin!=FOREGROUND) *ptrout=1;
	    else *ptrout=0;
	  else
	    if (*ptrin==FOREGROUND) *ptrout=1;
	    else *ptrout=0;
	*ptrout=0;ptrout++;
      }
    for (dy=0,ptrout=Output;dy<Col;dy++,ptrout++) *ptrout=0; 
   
    label=1;
    for (dx=2,ptrout=Output+(Col+1);dx<Line;dx++,ptrout+=2)
      for (dy=2;dy<Col;dy++,ptrout++)
	if (*ptrout)
	  {
	    if (label>LABELS_NUMBER)
	      mwerror(FATAL,1,"There are more than %d labels !\n",LABELS_NUMBER);
	    /* 'kernel' contains the four upper nearest neighbours of current point
	               * * *
		       * + x
		       x x x
	       if 4-connectivity is used then only 2 neighbors are taken into account */
	    kernel[0]=*(ptrout-1);
	    if (connectivity) kernel[1]=(-1);
	    else kernel[1]=*(ptrout-(Col+1));
	    kernel[2]=*(ptrout-Col);
	    if (connectivity) kernel[3]=(-1);
	    else kernel[3]=*(ptrout-(Col-1));
	    qsort(kernel,4,sizeof(int),u_compar_i);
	    /* kernel values are sorted decreasingly */
	    if (kernel[0]==0)
	      *ptrout=label++;   /* This point is given a new label */
	    else
	      {
		*ptrout=kernel[0];
		if (kernel[1])
		  if (kernel[1]!=kernel[0])
		    if (kernel[1]!=transcode[kernel[0]])
		      mise_a_jour_transcode(transcode,kernel[0],kernel[1]);
	      }
	  }
    first=0;
    for (i=label-1;i>0;i--)
      if ((transcode[i]) && (!(refresh_transcode[i])))
	refresh(transcode,refresh_transcode,i,&first);
    
    
    /* We want each connected component to be labelled between 1 and the number
       of components. This is done in the following step */
    norme=1;
    for (i=1,ptr_ref=refresh_transcode+1;i<label;i++,ptr_ref++){
      if (!(*ptr_ref))
	*ptr_ref=i;
      if (normalize_transcode[*ptr_ref]==0)
	{normalize_transcode[*ptr_ref]=norme++;}}
    *NUMBER=norme-1;

     /* Writing on image */
    if (!not_writing_on)
      {
	for (dx=2,ptrout=Output+(Col+1),ptrin=In;dx<Line;dx++,ptrout+=2)
	  for (dy=2;dy<Col;dy++,ptrout++,ptrin++)
	    {
	      if (*ptrout)
		*ptrin=(float)(normalize_transcode[refresh_transcode[*ptrout]]);
	      else
		*ptrin=0;
	    }
      }
    free(Output);
  }  




/****************************************************************************/

#define Square(u) (u)*(u)
#define Norm(u,v) sqrt(Square(u)+Square(v))
#define Min(u,v) (((u)<(v))?(u):(v))
#define Max(u,v) (((u)<(v))?(v):(u))
#define Sgn(u)   (((u)<(-1e-8))?(-1):1)
#define CPL 0.2

typedef struct a_jordan {  /* structure used to describe the T-junctions */
  double x,y; /* coordinates of a point of the Jordan curve */
  double vx,vy;  /* vector coordinates corresponding to the direction of the level line */
  char direction; /* direction of the associated line = either 0,2,4,6 (see below) */
  unsigned char gray1,gray2; /* values separated by current node (sorted counterclockwise) */
  struct a_jordan *junction;  /* jordan element it is linked to (NULL if no link) */
  struct a_jordan *previous,*next;
} jordan;

/* Numbering convention for direction

                      1 0 7  
                       \|/   
                     2-- --6 
                       /|\	
                      3 4 5
*/

	
/*  Called by function qsort for sorting unsigned char values by increasing values */
static int compar_uc(u,v)  
unsigned char *u,*v;
{
  if ((*u)>(*v)) return (1);
  if ((*u)==(*v)) return (0);
  return (-1);
}

int line_number,col_number;  /* image line and column numbers  */
int IONumber; /* Number of T-junctions asspciated with different level lines */

/* These three arrays are used for the computation of the optimal set of T-junctions
   (see ComputeOptimalSet) */
double **energy; 
int **correspond;
jordan **element;

char *anglep;
int energy_criterion;
int globx,globy;

static double CCost();
static void ComputeOptimalSet();

unsigned char *IImage,*OImage; /* Pointers to Input and Output images */
float *LImage; /* Pointer to Label image (image containing the only occlusions) */
float lvalue;  /* Current occlusion label */

typedef struct occlusionPoint{ /* structure used to describe the occlusion */
  int pos; /* coordinates of point */
  } occlusionPoint;
occlusionPoint *occlusion;
int numcolored; /* number of points already colorized */


typedef struct a_frontier { /* structure used to describe the vertices of the occlusion boundary */
  double x,y; /* coordinates of a point of the Jordan curve */
  struct a_frontier *previous,*next;
} frontier;

frontier *jb; /* First element of frontier */
int numfrontier=0;
char *pivots; /* A geodesic path can be made of different segments; 
		 at each segment apex (pivot) one needs the local conformation 
		 of the level line */
int *fpivots;

typedef struct Pxy_t {
    double x, y;
} Pxy_t;

typedef Pxy_t Ppoint_t;

typedef struct Ppoly_t {
    Ppoint_t *ps;
    int pn;
} Ppoly_t;

typedef Ppoly_t Ppolyline_t;

#define ISCCW 1
#define ISCW  2
#define ISON  3

#define DQ_FRONT 1
#define DQ_BACK  2

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define prerror(msg) mwerror(FATAL,1,"\nlibpath/%s:%d: %s\n", __FILE__, __LINE__, (msg))

#define POINTSIZE sizeof (Ppoint_t)

typedef struct pointnlink_t {
    Ppoint_t *pp;
    struct pointnlink_t *link;
} pointnlink_t;

#define POINTNLINKSIZE sizeof (pointnlink_t)
#define POINTNLINKPSIZE sizeof (pointnlink_t *)

typedef struct tedge_t {
    pointnlink_t *pnl0p;
    pointnlink_t *pnl1p;
    struct triangle_t *ltp;
    struct triangle_t *rtp;
} tedge_t;

typedef struct triangle_t {
    int mark;
    tedge_t e[3];
} triangle_t;

#define TRIANGLESIZE sizeof (triangle_t)

typedef struct deque_t {
    pointnlink_t **pnlps;
    int pnlpn, fpnlpi, lpnlpi, apex;
} deque_t;

pointnlink_t *pnls, **pnlps;
int pnll;

triangle_t *tris;
int trin, tril;

deque_t dq;

Ppoint_t *ops;

/****************************************************************************/

/* ccw test: counterclockwise, clockwise or collinear */
int ccw (p1p,p2p,p3p)
Ppoint_t *p1p,*p2p,*p3p;
  {
    double d;

    d = ((p1p->y - p2p->y) * (p3p->x - p2p->x)) -
            ((p3p->y - p2p->y) * (p1p->x - p2p->x));
    return (d > 0) ? ISCCW : ((d < 0) ? ISCW : ISON);
  }

/****************************************************************************/

/* is pbp between pap and pcp */
int between (pap,pbp,pcp)
Ppoint_t *pap,*pbp,*pcp;
  {
    Ppoint_t p1, p2;

    p1.x = pbp->x - pap->x, p1.y = pbp->y - pap->y;
    p2.x = pcp->x - pap->x, p2.y = pcp->y - pap->y;
    if (ccw (pap, pbp, pcp) != ISON)
        return FALSE;
    return (p2.x * p1.x + p2.y * p1.y >= 0) &&
            (p2.x * p2.x + p2.y * p2.y <= p1.x * p1.x + p1.y * p1.y);
  }

/****************************************************************************/

/* line to line intersection */
int intersects (pap,pbp,pcp,pdp)
Ppoint_t *pap,*pbp,*pcp,*pdp;
  {
    int ccw1, ccw2, ccw3, ccw4;

    if (ccw (pap, pbp, pcp) == ISON || ccw (pap, pbp, pdp) == ISON ||
            ccw (pcp, pdp, pap) == ISON || ccw (pcp, pdp, pbp) == ISON) {
        if (between (pap, pbp, pcp) || between (pap, pbp, pdp) ||
                between (pcp, pdp, pap) || between (pcp, pdp, pbp))
            return TRUE;
    } else {
        ccw1 = (ccw (pap, pbp, pcp) == ISCCW) ? 1 : 0;
        ccw2 = (ccw (pap, pbp, pdp) == ISCCW) ? 1 : 0;
        ccw3 = (ccw (pcp, pdp, pap) == ISCCW) ? 1 : 0;
        ccw4 = (ccw (pcp, pdp, pbp) == ISCCW) ? 1 : 0;
        return (ccw1 ^ ccw2) && (ccw3 ^ ccw4);
    }
    return FALSE;
  }

/****************************************************************************/

/* check if (i, i + 2) is a diagonal */
int isdiagonal (pnli,pnlip2,pnln)
int pnli,pnlip2,pnln;
  {
    int pnlip1, pnlim1, pnlj, pnljp1, res;

    /* neighborhood test */
    pnlip1 = (pnli + 1) % pnln;
    pnlim1 = (pnli + pnln - 1) % pnln;
    /* If P[pnli] is a convex vertex [ pnli+1 left of (pnli-1,pnli) ]. */
    if (ccw (pnlps[pnlim1]->pp, pnlps[pnli]->pp, pnlps[pnlip1]->pp) == ISCCW)
        res = (ccw (pnlps[pnli]->pp, pnlps[pnlip2]->pp,
                pnlps[pnlim1]->pp) == ISCCW) &&
                (ccw (pnlps[pnlip2]->pp, pnlps[pnli]->pp,
                pnlps[pnlip1]->pp) == ISCCW);
    /* Assume (pnli - 1, pnli, pnli + 1) not collinear. */
    else
        res = (ccw (pnlps[pnli]->pp, pnlps[pnlip2]->pp,
                pnlps[pnlip1]->pp) == ISCW);
    if (!res)
        return FALSE;

    /* check against all other edges */
    for (pnlj = 0; pnlj < pnln; pnlj++) {
        pnljp1 = (pnlj + 1) % pnln;
        if (!((pnlj == pnli) || (pnljp1 == pnli) ||
                (pnlj == pnlip2) || (pnljp1 == pnlip2)))
            if (intersects (pnlps[pnli]->pp, pnlps[pnlip2]->pp,
                    pnlps[pnlj]->pp, pnlps[pnljp1]->pp))
                return FALSE;
    }
    return TRUE;
  }

/****************************************************************************/

void growtris (newtrin)
int newtrin;
  {
    if (newtrin <= trin) return;
    if (!tris) {
        if (!(tris = (triangle_t *) malloc (TRIANGLESIZE * newtrin))) {
            prerror ("cannot malloc tris");
            abort ();
        }
    } else {
        if (!(tris = (triangle_t *) realloc ((void *) tris,
                TRIANGLESIZE * newtrin))) {
            prerror ("cannot realloc tris");
            abort ();
        }
    }
    trin = newtrin;
  }

/****************************************************************************/

void loadtriangle (pnlap,pnlbp,pnlcp)
pointnlink_t *pnlap,*pnlbp,*pnlcp;
  {
    triangle_t *trip;
    int ei;

    /* make space */
    if (tril >= trin) growtris (trin + 20);
    trip = &tris[tril++];
    trip->mark = 0;
    trip->e[0].pnl0p = pnlap, trip->e[0].pnl1p = pnlbp, trip->e[0].rtp = NULL;
    trip->e[1].pnl0p = pnlbp, trip->e[1].pnl1p = pnlcp, trip->e[1].rtp = NULL;
    trip->e[2].pnl0p = pnlcp, trip->e[2].pnl1p = pnlap, trip->e[2].rtp = NULL;
    for (ei = 0; ei < 3; ei++)
        trip->e[ei].ltp = trip;
}

/****************************************************************************/

/* triangulate polygon */
void triangulate (pnln)
int pnln; 
  {
    int pnli, pnlip1, pnlip2;

    if (pnln > 3) {
        for (pnli = 0; pnli < pnln; pnli++) {
            pnlip1 = (pnli + 1) % pnln;
            pnlip2 = (pnli + 2) % pnln;
	    if (isdiagonal (pnli, pnlip2,  pnln)) {
		loadtriangle (pnlps[pnli], pnlps[pnlip1], pnlps[pnlip2]);
		for (pnli = pnlip1; pnli < pnln - 1; pnli++)
                    pnlps[pnli] = pnlps[pnli + 1];
                triangulate (pnln - 1);
                return;
            }
        }
	prerror ("the polygon frontier is not a simple curve");
        abort ();
    } else
        loadtriangle (pnlps[0], pnlps[1], pnlps[2]);
  }

/****************************************************************************/

/* connect a pair of triangles at their common edge (if any) */
void connecttris (tri1,tri2)
int tri1,tri2;
  {
    triangle_t *tri1p, *tri2p;
    int ei, ej;

    for (ei = 0 ; ei < 3; ei++) {
        for (ej = 0; ej < 3; ej++) {
            tri1p = &tris[tri1], tri2p = &tris[tri2];
            if ((tri1p->e[ei].pnl0p->pp == tri2p->e[ej].pnl0p->pp &&
                    tri1p->e[ei].pnl1p->pp == tri2p->e[ej].pnl1p->pp) ||
                    (tri1p->e[ei].pnl0p->pp == tri2p->e[ej].pnl1p->pp &&
                    tri1p->e[ei].pnl1p->pp == tri2p->e[ej].pnl0p->pp))
                tri1p->e[ei].rtp = tri2p, tri2p->e[ej].rtp = tri1p;
        }
    }
  }

/****************************************************************************/

/* find and mark path from trii, to trij */
int marktripath (trii,trij)
int trii,trij;
  {
    int ei;

    if (tris[trii].mark)
        return FALSE;
    tris[trii].mark = 1;
    if (trii == trij)
        return TRUE;
    for (ei = 0; ei < 3; ei++)
        if (tris[trii].e[ei].rtp &&
                marktripath (tris[trii].e[ei].rtp - tris, trij))
            return TRUE;
    tris[trii].mark = 0;
    return FALSE;
}

/****************************************************************************/

/* add a new point to the deque, either front or back */
void add2dq (side,pnlp)
int side;
pointnlink_t *pnlp;
  {
    if (side == DQ_FRONT) {
        if (dq.lpnlpi - dq.fpnlpi >= 0)
            pnlp->link = dq.pnlps[dq.fpnlpi]; /* shortest path links */
        dq.fpnlpi--;
        dq.pnlps[dq.fpnlpi] = pnlp;
    } else {
        if (dq.lpnlpi - dq.fpnlpi >= 0)
            pnlp->link = dq.pnlps[dq.lpnlpi]; /* shortest path links */
        dq.lpnlpi++;
        dq.pnlps[dq.lpnlpi] = pnlp;
    }
  }

/****************************************************************************/

void splitdq (side,index)
int side,index;
 {
    if (side == DQ_FRONT)
        dq.lpnlpi = index;
    else
        dq.fpnlpi = index;
 }

/****************************************************************************/

int finddqsplit (pnlp)
pointnlink_t *pnlp;
  {
    int index;

    for (index = dq.fpnlpi; index < dq.apex; index++)
        if (ccw (dq.pnlps[index + 1]->pp, dq.pnlps[index]->pp, pnlp->pp) == ISCCW)
            return index;
    for (index = dq.lpnlpi; index > dq.apex; index--)
        if (ccw (dq.pnlps[index - 1]->pp, dq.pnlps[index]->pp, pnlp->pp) == ISCW)
            return index;
    return dq.apex;
  }

/****************************************************************************/

int pointintri (trii,pp)
int trii;
Ppoint_t *pp;
  {
    int ei, sum;

    for (ei = 0, sum = 0; ei < 3; ei++)
      if (ccw (tris[trii].e[ei].pnl0p->pp,
	       tris[trii].e[ei].pnl1p->pp, pp) != ISCW)
	sum++;
    return (sum == 3 || sum == 0);
  } 

/****************************************************************************/

void growpnls (newpnln)
int newpnln;
  {
    if (!pnls) {
      if (!(pnls = (pointnlink_t *) malloc (POINTNLINKSIZE * newpnln))) {
	prerror ("cannot malloc pnls");
	abort ();}
      if (!(pnlps = (pointnlink_t **) malloc (POINTNLINKPSIZE * newpnln))) {
	prerror ("cannot malloc pnlps");
	abort ();}}
  }

/****************************************************************************/

void growdq (newdqn)
int newdqn;
  {
    if (newdqn <= dq.pnlpn) return;
    if (!dq.pnlps) {
      dq.pnlps = (pointnlink_t **)malloc(POINTNLINKPSIZE * newdqn);
      if (!dq.pnlps){
	prerror ("cannot malloc dq.pnlps");
	abort ();}}
    else
      if (!(dq.pnlps = (pointnlink_t **)realloc ((void *) dq.pnlps,
						 POINTNLINKPSIZE * newdqn))) {
	prerror ("cannot realloc dq.pnlps");
	abort ();}
    dq.pnlpn = newdqn;
  }

/****************************************************************************/

void growops (newopn)
int newopn;
  {
    ops=NULL;
    if (!(ops = (Ppoint_t *) malloc (POINTSIZE * newopn))) {
      prerror ("cannot malloc ops");
      abort ();}
  }

/****************************************************************************/

void Triangulation(polyp)
Ppoly_t *polyp;

  {
    int pi, minpi;
    double minx;
    Ppoint_t p1, p2, p3;
    int trii, trij;
    int ei;
    int pnli;
   
    /* make space */
    growpnls (polyp->pn);
    pnll = 0;
    tril = 0;
	
    /* make sure polygon is counterclockwise and load pnls array */
    for (pi = 0, minx = 1e+200, minpi = -1; pi < polyp->pn; pi++) {
        if (minx > polyp->ps[pi].x)
            minx = polyp->ps[pi].x, minpi = pi;
    }
    p2 = polyp->ps[minpi];
    p1 = polyp->ps[((minpi == 0) ? polyp->pn - 1: minpi - 1)];
    p3 = polyp->ps[((minpi == polyp->pn - 1) ? 0 : minpi + 1)];
    if (((p1.x == p2.x && p2.x == p3.x) && (p3.y > p2.y)) ||
            ccw (&p1, &p2, &p3) != ISCCW) {
        for (pi = polyp->pn - 1; pi >= 0; pi--) {
            if (pi < polyp->pn - 1 && polyp->ps[pi].x == polyp->ps[pi + 1].x &&
                    polyp->ps[pi].y == polyp->ps[pi + 1].y)
                continue;
            pnls[pnll].pp = &polyp->ps[pi];
            pnls[pnll].link = &pnls[pnll % polyp->pn];
            pnlps[pnll] = &pnls[pnll];
            pnll++;
        }
    } else {
        for (pi = 0; pi < polyp->pn; pi++) {
            if (pi > 0 && polyp->ps[pi].x == polyp->ps[pi - 1].x &&
                    polyp->ps[pi].y == polyp->ps[pi - 1].y)
                continue;
            pnls[pnll].pp = &polyp->ps[pi];
            pnls[pnll].link = &pnls[pnll % polyp->pn];
            pnlps[pnll] = &pnls[pnll];
            pnll++;
        }
    }

    mwdebug("points\n%d\n", pnll);
    for (pnli = 0; pnli < pnll; pnli++)
        mwdebug("%f %f\n", pnls[pnli].pp->x, pnls[pnli].pp->y);

    /* generate list of triangles */
    trin=0;
    triangulate (pnll);
    mwdebug("triangles\n%d\n", tril);
    for (trii = 0; trii < tril; trii++)
        for (ei = 0; ei < 3; ei++)
            mwdebug("%f %f\n", tris[trii].e[ei].pnl0p->pp->x,
                    tris[trii].e[ei].pnl0p->pp->y);

    /* connect all pairs of triangles that share an edge */
    for (trii = 0; trii < tril; trii++)
        for (trij = trii + 1; trij < tril; trij++)
            connecttris (trii, trij);
    dq.pnlpn=0;
    dq.pnlps=NULL;
    growdq (polyp->pn * 2);

  }
/****************************************************************************/
/* return an angle between -pi and pi */

double AngleFix(a)
double a;
{
    while (a > M_PI)
        a -= 2 * M_PI;
    while (a < -M_PI)
        a += 2 * M_PI;
    return a;
}
/****************************************************************************/

/* compare the current angle a with reference angle b */

double AngleCheck(a,b)
double a,b;
{
    if ((a+M_PI>=b+M_PI_2)&&(a+M_PI<=b+3*M_PI_2)) return a;
    return AngleFix(a+M_PI);
}


/****************************************************************************/
/* eps1 and eps2 are the T-junctions. 
   meandir1 is the geodesic curve mean direction at eps1
   meandir2 is the geodesic curve mean direction at eps2 */

double GeodesicDistance(jc1,jc2,TVangle)
jordan *jc1,*jc2;
double *TVangle;
  {
    
    Ppoint_t eps1,eps2;
    int trii, ftrii, ltrii;
    int ei;
    pointnlink_t epnls[2], *lpnlp, *rpnlp, *pnlp;
    triangle_t *trip;
    int splitindex;
    double gdistance;
    double gdx1,gdy1,gdx2,gdy2;
    double v1x=jc1->vx,v1y=jc1->vy,v2x=jc2->vx,v2y=jc2->vy;
    double vx,vy,vxref,vyref; 

    eps1.x=jc1->x+globx;eps1.y=jc1->y;
    eps2.x=jc2->x+globx;eps2.y=jc2->y;
    
    /* find first and last triangles */
    for (trii = 0; trii < tril; trii++)
      if (pointintri (trii, &eps1))
	break;
    if (trii == tril) {
      prerror ("source point not in any triangle");
      return -1;}
    ftrii = trii;
    for (trii = 0; trii < tril; trii++)
      if (pointintri (trii, &eps2))
            break;
    if (trii == tril) {
      prerror ("destination point not in any triangle");
      return -1;}
    ltrii = trii;
    
    /* mark the strip of triangles from eps1 to eps2 */
    if (!marktripath (ftrii, ltrii)) {
      prerror ("cannot find triangle path");
      return -1;}

    /* if endpoints in same triangle, compute the euclidean distance */
    if (ftrii == ltrii) 
      {
	for (trii = 0; trii < tril; trii++) 
	  tris[trii].mark=0; /* unmark all triangles */
	vx=eps2.x-eps1.x;vy=eps2.y-eps1.y;
	*TVangle=fabs(atan2(v1x*vy-v1y*vx,v1x*vx+v1y*vy))+fabs(atan2(v2y*vx-v2x*vy,-v2x*vx-v2y*vy));
	return (Norm(vx,vy));
      }
    dq.fpnlpi = dq.pnlpn / 2, dq.lpnlpi = dq.fpnlpi - 1;
    /* build funnel and shortest path linked list (in add2dq) */
    epnls[0].pp = &eps1, epnls[0].link = NULL;
    epnls[1].pp = &eps2, epnls[1].link = NULL;
    add2dq (DQ_FRONT, &epnls[0]);
    dq.apex = dq.fpnlpi;
    trii = ftrii;
    while (trii != -1) {
      trip = &tris[trii];
      trip->mark = 2;
      
      /* find the left and right points of the exiting edge */
      for (ei = 0; ei < 3; ei++)
	if (trip->e[ei].rtp && trip->e[ei].rtp->mark == 1)
	  break;
      if (ei == 3) { /* in last triangle */
	if (ccw (&eps2, dq.pnlps[dq.fpnlpi]->pp,dq.pnlps[dq.lpnlpi]->pp) == ISCCW)
	  {lpnlp = dq.pnlps[dq.lpnlpi]; rpnlp = &epnls[1];}
	else
	  {lpnlp = &epnls[1]; rpnlp = dq.pnlps[dq.lpnlpi];}}
      else {
	pnlp = trip->e[(ei + 1) % 3].pnl1p;
	if (ccw (trip->e[ei].pnl0p->pp, pnlp->pp,trip->e[ei].pnl1p->pp) == ISCCW)
	  {lpnlp = trip->e[ei].pnl1p; rpnlp = trip->e[ei].pnl0p;}
	else
	  {lpnlp = trip->e[ei].pnl0p; rpnlp = trip->e[ei].pnl1p;}}

      /* update deque */
      if (trii == ftrii) {
	add2dq (DQ_BACK, lpnlp);
	add2dq (DQ_FRONT, rpnlp);}
      else {
	if (dq.pnlps[dq.fpnlpi] != rpnlp && dq.pnlps[dq.lpnlpi] != rpnlp) {
	  /* add right point to deque */
	  splitindex = finddqsplit (rpnlp);
	  splitdq (DQ_BACK, splitindex);
	  add2dq (DQ_FRONT, rpnlp);
	  /* if the split is behind the apex, then reset apex */
	  if (splitindex > dq.apex)
	    dq.apex = splitindex;}
	else {
	  /* add left point to deque */
	  splitindex = finddqsplit (lpnlp);
	  splitdq (DQ_FRONT, splitindex);
	  add2dq (DQ_BACK, lpnlp);
	  /* if the split is in front of the apex, then reset apex */
	  if (splitindex < dq.apex)
	    dq.apex = splitindex;}}
      
      trii = -1;
      for (ei = 0; ei < 3; ei++)
	if (trip->e[ei].rtp && trip->e[ei].rtp->mark == 1) {
	  trii = trip->e[ei].rtp - tris;
	  break;}
    }
     
    pnlp=&epnls[1];
    vxref=v2x;vyref=v2y;(*TVangle)=0.0;
    gdx1=pnlp->pp->x;gdy1=pnlp->pp->y;
    mwdebug("Path : %f %f\n",gdx1,gdy1);
    gdistance=0.0;
    while (pnlp->link)
      {
	pnlp=pnlp->link;
	gdx2=pnlp->pp->x;gdy2=pnlp->pp->y;
	vx=gdx2-gdx1;vy=gdy2-gdy1;
	gdistance+=Norm(vx,vy);
	(*TVangle)+=fabs(atan2(vxref*vy-vyref*vx,vxref*vx+vyref*vy));
	vxref=vx;vyref=vy;
	if (!(pnlp->link)){
	    vx=-v1x;vy=-v1y;
	    (*TVangle)+=fabs(atan2(vxref*vy-vyref*vx,vxref*vx+vyref*vy));
	    if ((eps1.x!=gdx2)||(eps1.y!=gdy2))
	      mwerror(FATAL,1,"Erreur dans GeodesicPath\n");}
	gdx1=gdx2;gdy1=gdy2;
	mwdebug("Path : %f %f\n",gdx1,gdy1);
      }
    for (trii = 0; trii < tril; trii++) 
      tris[trii].mark=0; /* unmark all triangles */

    return gdistance;
  }

/****************************************************************************/

int Pshortestpath (eps1,eps2,output)
Ppoint_t eps1,eps2;
Ppolyline_t *output;
  {
    int pi;
    int trii, ftrii, ltrii;
    int ei;
    pointnlink_t epnls[2], *lpnlp, *rpnlp, *pnlp;
    triangle_t *trip;
    int splitindex;
    
    /*-------------------------------------------------------
      The shortest path between T-junctions 'eps1' and 'eps2'
       is computed
    ---------------------------------------------------------*/
    
    /* find first and last triangles */
    for (trii = 0; trii < tril; trii++)
      if (pointintri (trii, &eps1))
	break;
    if (trii == tril) {
      prerror ("source point not in any triangle");
      return -1;}
    ftrii = trii;
    for (trii = 0; trii < tril; trii++)
      if (pointintri (trii, &eps2))
            break;
    if (trii == tril) {
      prerror ("destination point not in any triangle");
      return -1;}
    ltrii = trii;
    
    /* mark the strip of triangles from eps1 to eps2 */
    if (!marktripath (ftrii, ltrii)) {
      prerror ("cannot find triangle path");
      return -1;}
    
    /* if endpoints in same triangle, use a single line */
    if (ftrii == ltrii) 
      {
	growops (2);
	output->pn = 2;
	ops[0] = eps1, ops[1] = eps2;
	output->ps = ops;
	for (trii = 0; trii < tril; trii++) 
	  tris[trii].mark=0; /* unmark all triangles */
	return 0;
      }
    
    dq.fpnlpi = dq.pnlpn / 2, dq.lpnlpi = dq.fpnlpi - 1;
    /* build funnel and shortest path linked list (in add2dq) */
    epnls[0].pp = &eps1, epnls[0].link = NULL;
    epnls[1].pp = &eps2, epnls[1].link = NULL;
    add2dq (DQ_FRONT, &epnls[0]);
    dq.apex = dq.fpnlpi;
    trii = ftrii;
    while (trii != -1) {
      trip = &tris[trii];
      trip->mark = 2;
      
      /* find the left and right points of the exiting edge */
      for (ei = 0; ei < 3; ei++)
	if (trip->e[ei].rtp && trip->e[ei].rtp->mark == 1)
	  break;
      if (ei == 3) { /* in last triangle */
	if (ccw (&eps2, dq.pnlps[dq.fpnlpi]->pp,
		 dq.pnlps[dq.lpnlpi]->pp) == ISCCW)
	  {lpnlp = dq.pnlps[dq.lpnlpi]; rpnlp = &epnls[1];}
	else
	  {lpnlp = &epnls[1]; rpnlp = dq.pnlps[dq.lpnlpi];}}
      else {
	pnlp = trip->e[(ei + 1) % 3].pnl1p;
	if (ccw (trip->e[ei].pnl0p->pp, pnlp->pp,
		 trip->e[ei].pnl1p->pp) == ISCCW)
	  {lpnlp = trip->e[ei].pnl1p; rpnlp = trip->e[ei].pnl0p;}
	else
	  {lpnlp = trip->e[ei].pnl0p; rpnlp = trip->e[ei].pnl1p;}}
      
      /* update deque */
      if (trii == ftrii) {
	add2dq (DQ_BACK, lpnlp);
	add2dq (DQ_FRONT, rpnlp);}
      else {
	if (dq.pnlps[dq.fpnlpi] != rpnlp && dq.pnlps[dq.lpnlpi] != rpnlp) {
	  /* add right point to deque */
	  splitindex = finddqsplit (rpnlp);
	  splitdq (DQ_BACK, splitindex);
	  add2dq (DQ_FRONT, rpnlp);
	  /* if the split is behind the apex, then reset apex */
	  if (splitindex > dq.apex)
	    dq.apex = splitindex;}
	else {
	  /* add left point to deque */
	  splitindex = finddqsplit (lpnlp);
	  splitdq (DQ_FRONT, splitindex);
	  add2dq (DQ_BACK, lpnlp);
	  /* if the split is in front of the apex, then reset apex */
	  if (splitindex < dq.apex)
	    dq.apex = splitindex;}}
      
      trii = -1;
      for (ei = 0; ei < 3; ei++)
	if (trip->e[ei].rtp && trip->e[ei].rtp->mark == 1) {
	  trii = trip->e[ei].rtp - tris;
	  break;}
    }
    
    mwdebug("polypath");
    for (pnlp = &epnls[1]; pnlp; pnlp = pnlp->link)
      mwdebug("%d %d  %f %f\n",pnlp,pnlp->pp,pnlp->pp->x, pnlp->pp->y);
    mwdebug("\n\n");
    
    for (pi = 0, pnlp = &epnls[1]; pnlp; pnlp = pnlp->link)  pi++;
    growops(pi);
    output->pn = pi;
    for (pi = pi - 1, pnlp = &epnls[1]; pnlp; pi--, pnlp = pnlp->link)
      ops[pi] = *pnlp->pp;
    output->ps = ops;
    
    for (trii = 0; trii < tril; trii++) 
      tris[trii].mark=0; /* unmark all triangles */
    
    return 0;
}
/****************************************************************************/

void FreeTPath()
  {
    if (tris) {free((void*)tris);tris=NULL;}
    if (pnls) {free((void*)pnls);pnls=NULL;}
    if (pnlps) {free((void*)pnlps);pnlps=NULL;}
    if (dq.pnlps) {free((void*)(dq.pnlps));dq.pnlps=NULL;}
  }

/****************************************************************************/

Ppoint_t *polypoints; 
    /* Copy of the frontier as a polygonal line whose structure 
       is compatible with Mitchell's shortest path algorithm */
Ppoint_t *endTjunctions; /* All the geodesics endpoints */

/****************************************************************************/

/* Adds a new element to the list of vertices of occlusion boundary */

void add_to_frontier(x,y)
double x,y;

  {
    frontier *inter;
  
    inter=jb;
    jb=(frontier*)malloc((size_t)sizeof(frontier));
    if (!jb) mwerror(FATAL,1,"Not enough memory (1)!\n");
    numfrontier++;
    jb->x=x;jb->y=y;
    jb->next=inter;
    if (inter) inter->previous=jb;
    jb->previous=(frontier*)NULL;
  }
/****************************************************************************/

/* Adds a new element to the list of T-junctions if current point is a T-junction
   and in any case, adds a new element to the list of occlusion frontier points
   Note that level line direction is at this level of algorithm either equal to 
   0, 2, 4 or 6 (see convention above) */

jordan *add_to_jordan(nouv,x,y,direction,gray1,gray2,dofrontier)
jordan *nouv;
double x,y;
char direction;
unsigned char gray1,gray2;
char dofrontier;

  {
    jordan *inter;
  
    if (gray1!=gray2){
      inter=nouv;
      nouv=(jordan*)malloc((size_t)sizeof(jordan));
      if (!nouv) mwerror(FATAL,1,"Not enough memory (2)!\n");
      nouv->x=x;nouv->y=y;nouv->direction=direction;
      nouv->gray1=gray1;nouv->gray2=gray2;
      nouv->junction=(jordan*)NULL;
      switch (direction){
      case 0:nouv->vx=1;nouv->vy=0;break;
      case 2:nouv->vx=0;nouv->vy=1;break;
      case 4:nouv->vx=(-1);nouv->vy=0;break;
      case 6:nouv->vx=0;nouv->vy=(-1);break;  
      default:mwerror(FATAL,1,"Impossible dans add_to_jordan\n");break;}
      /* Remember convention : x denotes vertical axis and y horizontal axis */
      nouv->next=inter;
      if (inter) inter->previous=nouv;
      nouv->previous=(jordan*)NULL;}
    if (dofrontier) add_to_frontier(x,y);
    /* dofrontier is useful to avoid redundancy : recall that a point can be associated 
       with several level lines. It is also used for taking only those points which  are vertices
       of the polygonal line describing the occlusion boundary */
    return nouv;
  }

/****************************************************************************/

/* Inserts a new element in the chain defining the structuring element*/

jordan *insert_jordan(jc,gray1,gray2)
jordan *jc;
unsigned char gray1,gray2;

  {
    jordan *new;  /* new is inserted between jc and jc->next */
  
    new=(jordan*)malloc((size_t)sizeof(jordan));
    if (!new) mwerror(FATAL,1,"Not enough memory (3)!\n");
    new->x=jc->x;new->y=jc->y;
    new->vx=jc->vx;new->vy=jc->vy;
    new->gray1=gray1;new->gray2=gray2;
    new->next=jc->next;
    if (jc->next) jc->next->previous=new;
    new->direction=jc->direction;
    new->junction=(jordan*)NULL;
    jc->next=new;
    new->previous=jc;
    return new;
  }
/****************************************************************************/

/* Deletes an element in the chain defining the structuring element*/

jordan *delete_jordan(jc)
jordan *jc;

  {
    if (jc->previous) jc->previous->next=jc->next;
    if (jc->next) jc->next->previous=jc->previous;
    free((void*)jc);
    return (jordan*)NULL;
  }

/***************************************************************************/

/*  Frees the chain defining the T-junctions */
/*  Be careful that this function is valid only if jordan curve is "broken" */

void free_jordan(jc)
jordan *jc;
  
  {
    if (jc->next) free_jordan(jc->next);
    free((void*)jc);
    jc=(jordan*)NULL;
  }

/***************************************************************************/

/*  Frees the frontier chain  */
/*  Be careful that this function is valid only if frontier curve is "broken" */

void free_frontier(jbi)
frontier *jbi;
  
  {
    if (jbi->next) free_frontier(jbi->next);
    free((void*)jbi);
    jbi=(frontier*)NULL;
  }

/***************************************************************************/

/* Copies the frontier into a polygonal line whose structure is compatible 
   with Mitchell's shortest path algorithm. The frontier structure was built
   in such a way that it contains only those points of bifurcation of 
   the polygonal line enclosing the occlusion */

void copy_frontier(x)
int x;
  {
    frontier *jbi;
    int n;

    if (!(polypoints = (Ppoint_t *) malloc (POINTSIZE * numfrontier)))
      prerror ("cannot malloc polypoints");
    for (n=0,jbi=jb;n<numfrontier;n++,jbi=jbi->next)
      {polypoints[n].x=jbi->x+x;polypoints[n].y=jbi->y;}
  }

/***************************************************************************/

/* Colorization of pixels on both sides of the current geodesic path */

void geodfill(p1,p2,gray1,gray2,change)
int p1,p2;
unsigned char gray1,gray2;
char change;
  {
    /* LImage == (-1) means that the point has already been modified */

    if (LImage[p1]==lvalue) {OImage[p1]=gray1;LImage[p1]=(-1);occlusion[numcolored++].pos=p1;}
    if (LImage[p2]==lvalue) {OImage[p2]=gray2;LImage[p2]=(-1);occlusion[numcolored++].pos=p2;}
    if (!change)
      {if (LImage[p2]==(-1)) OImage[p2]=gray2;}
    else
      if (LImage[p1]==(-1)) OImage[p1]=gray1;
  }
	
/***************************************************************************/

/* Colorization of peculiar pixels in the neighborhood of geodesic path nodes  */

void pivotfill(p,gray,force)
int p;
unsigned char gray;
char force;

  {
    if (LImage[p]==lvalue) 
      {OImage[p]=gray;LImage[p]=(-1);occlusion[numcolored++].pos=p;return;}
    if (force)
      if (LImage[p]==(-1)) OImage[p]=gray;
  }

/***************************************************************************/

/* Colorization of peculiar pixels in the neighborhood of geodesic path nodes */

void fpivotfill(piv,p1,p2,gray1,gray2)
int piv,p1,p2;
unsigned char gray1,gray2;

  {
    if (p1<4) fpivots[piv+p1]=(int)gray1;
    if (p2<4) fpivots[piv+p2]=(int)gray2;
  }

/***************************************************************************/

/* Draws a two pixels wide straight line between two points. The algorithm is 
   very similar to Bresenham algorithm except that we force the line to be 
   two pixels wide and that we need some more work at each vertex of 
   the polygonal line. We use the following convention :
       xf=Horizontal coordinate of first point
       yf=Vertical coordinate of first point
       xe=Horizontal coordinate of end point
       ye=Vertical coordinate of end point */

void drawgeod(xf,yf,xe,ye,gray1,gray2,piv)
int xf,yf,xe,ye;
unsigned char gray1,gray2;
int piv; /* Current pivot number */
  {
    int dx=xe-xf,dy=ye-yf,vect=0,d2x,d2y;
    int i,j,epsilon,oldi,oldj;
    char change=0; /* 1 in case orientation is changed */
    unsigned char g;

    if (dy<0){
      change=1;
      g=gray1;gray1=gray2;gray2=g;
      dx=-dx;
      epsilon=xe;xe=xf;xf=epsilon;
      dy=-dy;
      epsilon=ye;ye=yf;yf=epsilon;}
    d2x=2*dx;d2y=2*dy;
    
    /* vect=dx*(i-yf)-dy*(j-xf) is the vectorial product (careful to the orientation!!)*/

    if (abs(dx)>=dy) /* dx is the horizontal gap, dy the vertical gap */
      {
	/*epsilon=dy-abs(dx);*/
	epsilon=-abs(dx);
	if (dx>0)
	  {
	    for (j=xf,i=yf;j<=xe;j++){
	      if (j==xe)
		if (i!=oldi) pivots[8*(piv+1-change)]=1+change;
		else pivots[8*(piv+1-change)+7]=1+change;
	      if (2*(j/2)==j)
		if (2*(i/2)!=i)
		  {
		    if (j==xf+1) fpivotfill(4*(piv+change),1,2,gray1,gray2);
		    if (j==xe-1) fpivotfill(4*(piv+1-change),0,3,gray1,gray2);
		    geodfill((i-1)/2*col_number+j/2,(i+1)/2*col_number+j/2,gray1,gray2,change);
		  }
		else
		  if (vect<=0)
		    {
		      if (j==xf+1) fpivotfill(4*(piv+change),2,4,gray1,gray2);
		      if (j==xe-1) fpivotfill(4*(piv+1-change),0,3,gray1,gray2);
		      geodfill(i/2*col_number+j/2,(i+2)/2*col_number+j/2,gray1,gray2,change);
		    }
		  else 
		    {
		      if (j==xf+1) fpivotfill(4*(piv+change),1,2,gray1,gray2);
		      if (j==xe-1) fpivotfill(4*(piv+1-change),4,0,gray1,gray2);
		      geodfill((i-2)/2*col_number+j/2,i/2*col_number+j/2,gray1,gray2,change);
		    }
	      /*else
		if (j==xe && i==ye)
		geodfill((i-1)/2*col_number+(j+1)/2,(i+1)/2*col_number+(j-1)/2,gray1,gray2,change);*/
	      oldi=i;
	      epsilon+=d2y;vect-=dy;
	      if (epsilon>=0) {i++;epsilon-=d2x;vect+=dx;}
	      if (j==xf)
		if (i!=oldi) pivots[8*(piv+change)+4]=2-change;
		else pivots[8*(piv+change)+3]=2-change;}
	  }
	else
	  {
	    for (j=xf,i=yf;j>=xe;j--){
	      if (j==xe)
		if (i!=oldi) pivots[8*(piv+1-change)+2]=1+change;
		else pivots[8*(piv+1-change)+3]=1+change;
	      if (2*(j/2)==j)
		if (2*(i/2)!=i)
		  {
		    if (j==xf-1) fpivotfill(4*(piv+change),3,0,gray1,gray2);
		    if (j==xe+1) fpivotfill(4*(piv+1-change),2,1,gray1,gray2);
		    geodfill((i+1)/2*col_number+j/2,(i-1)/2*col_number+j/2,gray1,gray2,change);
		  }
		else
		  if (vect>=0) 
		    {
		      if (j==xf-1) fpivotfill(4*(piv+change),4,3,gray1,gray2);
		      if (j==xe+1) fpivotfill(4*(piv+1-change),2,1,gray1,gray2);
		      geodfill((i+2)/2*col_number+j/2,i/2*col_number+j/2,gray1,gray2,change);
		    }
		  else
		    {
		      if (j==xf-1) fpivotfill(4*(piv+change),3,0,gray1,gray2);
		      if (j==xe+1) fpivotfill(4*(piv+1-change),1,4,gray1,gray2);
		      geodfill(i/2*col_number+j/2,(i-2)/2*col_number+j/2,gray1,gray2,change);
		    }
	      /*else
		if (j==xe && i==ye)*/ /*if (2*(i/2)!=i && (j!=xf))*/
	      /*geodfill((i+1)/2*col_number+(j+1)/2,(i-1)/2*col_number+(j-1)/2,gray1,gray2,change);*/
	      oldi=i;
	      epsilon+=d2y;vect+=dy;
	      if (epsilon>=0) {i++;epsilon+=d2x;vect+=dx;}
	      if (j==xf)
		if (i!=oldi) pivots[8*(piv+change)+6]=2-change;
		else pivots[8*(piv+change)+7]=2-change;}
	  }
      }
    else 
      {
	/*epsilon=abs(dx)-dy;*/
	epsilon=-dy;
	if (dx>0)
	  {
	    for (j=xf,i=yf;i<=ye;i++){
	      if (i==ye)
		if (j!=oldj) pivots[8*(piv+1-change)]=1+change;
		else pivots[8*(piv+1-change)+1]=1+change;
	      if (2*(i/2)==i)
		if (2*(j/2)!=j)
		  {
		    if (i==yf+1) fpivotfill(4*(piv+change),2,3,gray1,gray2);
		    if (i==ye-1) fpivotfill(4*(piv+1-change),1,0,gray1,gray2);
		    geodfill(i/2*col_number+(j+1)/2,i/2*col_number+(j-1)/2,gray1,gray2,change);
		  }
		else
		  if (vect<=0) 
		    {
		      if (i==yf+1) fpivotfill(4*(piv+change),2,3,gray1,gray2);
		      if (i==ye-1) fpivotfill(4*(piv+1-change),0,4,gray1,gray2);
		      geodfill(i/2*col_number+j/2,i/2*col_number+(j-2)/2,gray1,gray2,change);
		    }
		  else 
		    {
		      if (i==yf+1) fpivotfill(4*(piv+change),4,2,gray1,gray2);
		      if (i==ye-1) fpivotfill(4*(piv+1-change),1,0,gray1,gray2);
		      geodfill(i/2*col_number+(j+2)/2,i/2*col_number+j/2,gray1,gray2,change);
		    }
	      /*else
		if (j==xe && i==ye)*/ /*if (2*(i/2)!=i && (i!=yf))*/
	      /*geodfill((i-1)/2*col_number+(j+1)/2,(i+1)/2*col_number+(j-1)/2,gray1,gray2,change);*/
	      oldj=j;
	      vect+=dx;epsilon+=d2x;
	      if (epsilon>=0) {j++;epsilon-=d2y;vect-=dy;}
	      if (i==yf)
		if (j!=oldj) pivots[8*(piv+change)+4]=2-change;
		else pivots[8*(piv+change)+5]=2-change;}
	  }
	else
	  {
	    for (j=xf,i=yf;i<=ye;i++){
	      if (i==ye)
		if (j!=oldj) pivots[8*(piv+1-change)+2]=1+change;
		else pivots[8*(piv+1-change)+1]=1+change;
	      if (2*(i/2)==i)
		if (2*(j/2)!=j)
		  {
		    if (i==yf+1) fpivotfill(4*(piv+change),2,3,gray1,gray2);
		    if (i==ye-1) fpivotfill(4*(piv+1-change),1,0,gray1,gray2);
		    geodfill(i/2*col_number+(j+1)/2,i/2*col_number+(j-1)/2,gray1,gray2,change);
		  }
		else
		  if (vect>=0)
		    {
		      if (i==yf+1) fpivotfill(4*(piv+change),2,3,gray1,gray2);
		      if (i==ye-1) fpivotfill(4*(piv+1-change),4,1,gray1,gray2);
		      geodfill(i/2*col_number+(j+2)/2,i/2*col_number+j/2,gray1,gray2,change);
		    }
		  else 
		    {
		      if (i==yf+1) fpivotfill(4*(piv+change),3,4,gray1,gray2);
		      if (i==ye-1) fpivotfill(4*(piv+1-change),1,0,gray1,gray2);
		      geodfill(i/2*col_number+j/2,i/2*col_number+(j-2)/2,gray1,gray2,change);
		    }
	      /*else
		if (j==xe && i==ye)*/ /*if (2*(i/2)!=i && (i!=yf))*/
	      /*geodfill((i+1)/2*col_number+(j+1)/2,(i-1)/2*col_number+(j-1)/2,gray1,gray2,change);*/
	      oldj=j;
	      vect+=dx;epsilon-=d2x;
	      if (epsilon>=0) {j--;epsilon-=d2y;vect+=dy;}
	      if (i==yf)
		if (j!=oldj) pivots[8*(piv+change)+6]=2-change;
		else pivots[8*(piv+change)+5]=2-change;}
	  }
      }
  }


/***************************************************************************/

/* Here we draw a geodesic path between each optimal couple of T-junctions
   Each geodesic path is actually a two-pixels wide polygonal line where the 
   two gray values associated with the level line are represented.
   In order to draw this polygonal line, we simply draw straight lines between 
   the vertices using Bresenham algorithm. However, since this line is two pixels
   wide and since we want afterwards to propagate the values, we have to be careful
   at each vertex. */

/* The geodesic paths are successively drawn with respect to the following rule : 
   gray values are set on both sides of the first path, which can be any path whose 
   starting point differs from the starting point of the path immediately before. 
   Then, taking the starting points one after the other when the occlusion boundary 
   is walked counterclockwise, we draw the corresponding paths by forcing the value 
   on the "right" of the path and setting it on the left only if it has not been set yet. */

void geodesic(jc,x)
jordan *jc;
int x;

  {
    jordan *jci;
    Ppolyline_t *geodPoints;
    int n,pi,npivots,npi,npi1,npi2,npi3,npi4,npi5,pos,npipos;
    int gx1,gy1,gx2,gy2;
    char *ptrpiv;
    int *ptrfpiv;
    unsigned char gray1,gray2;
 
    /* Be careful : at the end of this function, jci->junction->junction=NULL !! */

    /* The two following pointers are needed for the computation of geodesic paths.
     These paths have actually already been computed in the dynamic programing stage but
     it would have been much too expensive to try to save them at that time (due to the
     necessity of performing successive updates). */
    if (!(geodPoints = (Ppolyline_t *) malloc ((IONumber/2) * sizeof(Ppolyline_t))))
      prerror ("cannot malloc geodPoints");
    if (!(endTjunctions = (Ppoint_t *) malloc (POINTSIZE * IONumber)))
      prerror ("cannot malloc endTjunctions");
    
    /* This is to ensure that the first examined level line is the first line 
       starting at the associated T-junction (ie the first line reached when 
       curve is described counterclockwise). */
    while ((jc->x==jc->previous->x) && (jc->y==jc->previous->y)) jc=jc->next;
    
    /* To avoid a double examination of the same level line */
     jci=jc;n=0;
    do{
      if (jci->junction){
	endTjunctions[n].x=jci->x+x;endTjunctions[n++].y=jci->y;
	endTjunctions[n].x=jci->junction->x+x;endTjunctions[n++].y=jci->junction->y;
	jci->junction->junction=NULL;}
      jci=jci->next;}
    while (jci!=jc);
    
    pivots=NULL;npivots=0;
    for (n=0;n<IONumber;n+=2)
      {
	Pshortestpath(endTjunctions[n],endTjunctions[n+1], &geodPoints[n/2]);
	gray1=jci->gray1;gray2=jci->gray2;
	if (geodPoints[n/2].pn>npivots)
	  {
	    npivots=geodPoints[n/2].pn;
	    if (!pivots)
	      {
		/* Each point of the line has 8 neighbors in the semi-grid */
		/* This array gives the local conformation of the line */
		if (!(pivots = (char *) malloc (npivots*8*sizeof(char)))) 
		  prerror ("cannot malloc pivots");
		/* Each point of the line has 4 integer neighbors */
		/* This array says where to put which gray value */
		if (!(fpivots = (int *) malloc (npivots*4*sizeof(int)))) 
		  prerror ("cannot malloc fpivots");
	      }
	    else
	      {
		if (!(pivots = (char *) realloc((void*)pivots,npivots*8*sizeof(char))))
		  prerror ("cannot realloc pivots");
		if (!(fpivots = (int *) realloc((void*)fpivots,npivots*4*sizeof(int))))
		  prerror ("cannot realloc fpivots");
	      }
	  }
	else npivots=geodPoints[n/2].pn;
	for (npi=0,ptrpiv=pivots;npi<npivots*8;npi++,ptrpiv++) *ptrpiv=0;
	for (npi=0,ptrfpiv=fpivots;npi<npivots*4;npi++,ptrfpiv++) *ptrfpiv=(-1);
	
	/* Pivots neighborhood      0  1  2 
	                            7  x  3 
		                    6  5  4  */
	/* "pivots[i]" takes value 1 if the line is entering at point i, 2 it the
	   line is outgoing at i. */

	/* fpivots neighborhood     0  1
	                            3  2  */
	switch (jci->direction)
	  {
	  case 0:pivots[1]=1;fpivots[1]=gray1;fpivots[0]=gray2;break;
	  case 2:pivots[7]=1;fpivots[0]=gray1;fpivots[3]=gray2;break;
	  case 4:pivots[5]=1;fpivots[3]=gray1;fpivots[2]=gray2;break;
	  case 6:pivots[3]=1;fpivots[2]=gray1;fpivots[1]=gray2;break;
	  default: prerror("Probleme de direction");break;
	  }
	switch (jci->junction->direction)
	  {
	  case 0:pivots[(npivots-1)*8+1]=2;fpivots[(npivots-1)*4]=gray1;
	    fpivots[(npivots-1)*4+1]=gray2;break;
	  case 2:pivots[(npivots-1)*8+7]=2;fpivots[(npivots-1)*4+3]=gray1;
	    fpivots[(npivots-1)*4]=gray2;break;
	  case 4:pivots[(npivots-1)*8+5]=2;fpivots[(npivots-1)*4+2]=gray1;
	    fpivots[(npivots-1)*4+3]=gray2;break;
	  case 6:pivots[(npivots-1)*8+3]=2;fpivots[(npivots-1)*4+1]=gray1;
	    fpivots[(npivots-1)*4+2]=gray2;break;
	  default:prerror("Probleme de direction");break;
	  }
	gx1=(int)(rint(2*geodPoints[n/2].ps[0].y));
	gy1=(int)(rint(2*geodPoints[n/2].ps[0].x));
	for (pi = 1; pi < geodPoints[n/2].pn; pi++)
	  {
	    gx2=(int)(rint(2*geodPoints[n/2].ps[pi].y));
	    gy2=(int)(rint(2*geodPoints[n/2].ps[pi].x));
	    drawgeod(gx1,gy1,gx2,gy2,gray1,gray2,pi-1);
	    gx1=gx2;gy1=gy2;
	  }

	/* Now we examine each node of the polygonal line and check if it is necessary
	   to colorize furthermore. This is required by the propagation function if we
	   want to avoid conflicts between gray values */
	for (pi = 0; pi < geodPoints[n/2].pn; pi++)
	  {
	    pos=(int)(rint(geodPoints[n/2].ps[pi].x-0.5))*col_number+(int)(rint(geodPoints[n/2].ps[pi].y-0.5));
	    ptrpiv=pivots+8*pi;
	    npi1=0;
	    while (*ptrpiv!=1) {npi1++;ptrpiv++;}
	    ptrpiv-=npi1;
	    npi2=0;
	    while (*ptrpiv!=2) {npi2++;ptrpiv++;}
	    ptrfpiv=fpivots+pi*4;
	    for (npi=0;npi<4;npi++)
	      if (*(ptrfpiv+npi)==(-1))
		{
		  switch (npi)
		    {
		    case 0:npipos=pos;break;
		    case 1:npipos=pos+1;break;
		    case 2:npipos=pos+col_number+1;break;
		    default:npipos=pos+col_number;break;
		    }
		  npi3=(npi+1)%4;
		  while (*(ptrfpiv+npi3)==(-1)) npi3=(npi3+1)%4;
		  npi4=(npi+3)%4;
		  while (*(ptrfpiv+npi4)==(-1)) npi4=(npi4+3)%4;
		  if (npi*2==npi1 || npi*2==npi2) prerror("Direction fpivots = probleme !!");
		  if (npi3!=npi4)
		    if (*(ptrfpiv+npi3)!=*(ptrfpiv+npi4))
		      {
			npi5=(2*npi+1)%8;
			/* The previous setting allows to switch from "fpivots" neighborhood to 
			   "pivots" neighborhood */
			while ((npi5!=npi1)&&(npi5)!=npi2) npi5=(npi5+1)%8;
			if (npi5==npi1) {pivotfill(npipos,gray2,(char)1);*(ptrfpiv+npi)=gray2;}
			else {pivotfill(npipos,gray1,(char)0);*(ptrfpiv+npi)=gray1;}
		      }
		}
	  }
	do jci=jci->next;
	while (jci->junction==NULL);
      }
    if (pivots) {free((void*)pivots);pivots=NULL;}
    if (fpivots) {free((void*)fpivots);fpivots=NULL;}
    FreeTPath();
    if (polypoints) 
      {free((void*)polypoints);polypoints=NULL;}
    if (endTjunctions) 
      {free((void*)endTjunctions);endTjunctions=NULL;}
    if (geodPoints){
      for (n=0;n<IONumber/2;n++)
	{free((void*)(geodPoints[n].ps));geodPoints[n].ps=NULL;}
      free((void*)geodPoints);
      geodPoints=NULL;}
  }

/***************************************************************************/


/* Once the geodesic paths have been drawn, it remains to let the values propagate
   within occlusion */

void propagation(occlusionArea)
int occlusionArea;

  {
    int k,l,pos;
    int oldnum=numcolored;
    unsigned char gray;


    /* First step : 4-connectivity geodesic dilation */
    for (k=0;k<oldnum;k++){
      gray=OImage[occlusion[k].pos];
      pos=occlusion[k].pos-col_number;
      for (l=0;l<4;l++){
	if (LImage[pos]==lvalue)
	  {OImage[pos]=gray;LImage[pos]=(-1);occlusion[numcolored++].pos=pos;}
	switch (l){
	case 0:case 2: pos+=col_number-1;break;
	case 1 : pos+=2;break;}}}
    
    /* Second step : 8-connectivity dilation */
    for (k=0;k<oldnum;k++){
      gray=OImage[occlusion[k].pos];
      pos=occlusion[k].pos-col_number-1;
      for (l=0;l<4;l++){
	if (LImage[pos]==lvalue)
	  {OImage[pos]=gray;LImage[pos]=(-1);occlusion[numcolored++].pos=pos;}
	switch (l){
	case 0:case 2: pos+=2;break;
	case 1 : pos+=2*col_number-2;break;}}}

    for (k=oldnum;k<occlusionArea;k++){
      gray=OImage[occlusion[k].pos];
      pos=occlusion[k].pos-col_number-1;
      for (l=0;l<8;l++){
	if (LImage[pos]==lvalue)
	  {OImage[pos]=gray;LImage[pos]=(-1);occlusion[numcolored++].pos=pos;}
	switch (l){
	case 0:case 1:case 5:case 6 : pos++;break;
	case 2:case 4 : pos+=col_number-2;break;
	case 3 : pos+=2;break;}}}
  }




/***************************************************************************/

/* This procedure is used to "clean" the jordan curve. Zero-length line are removed, 
   as well as lines that have the same endpoints */

jordan *update_jordan_curve(jc,ExternVal)
jordan *jc;
unsigned char *ExternVal;


  {
    jordan *jlook1,*jlook2,*jnext,*jprev;
    double ox1,oy1,ox2,oy2;
    char go_on;

    /* First step : find if possible a line with stricly positive length and start from this line */
    jlook1=jc;
    while ((jlook1->x==jlook1->junction->x) && (jlook1->y==jlook1->junction->y)){
      jlook1=jlook1->next;
      if (jlook1==jc){ /* All the junctions have zero-length ! */
	if (jc->vx*jc->junction->vy-jc->vy*jc->junction->vx>0) *ExternVal=jc->gray1;
	else *ExternVal=jc->gray2;
	IONumber=0;
	return ((jordan*)NULL);}}
    jc=jlook1;
    /* At that point the line jc -- jc->junction has a stricly positive length */

    /* Second step : remove all lines (and the in-between lines) whose length is zero */
    jlook1=jc->next;
    while (jlook1!=jc)
      if ((jlook1->x==jlook1->junction->x) && (jlook1->y==jlook1->junction->y)){
	jlook2=jlook1->junction->next;
	jlook1->previous->next=jlook2;
	jlook2->previous=jlook1->previous;
	do{
	  jnext=jlook1->next;
	  IONumber-=1;
	  free((void*)jlook1);
	  jlook1=jnext;}
	while (jlook1!=jlook2);}
      else jlook1=jlook1->next;

    /*  Third step : Merge lines which have the same endpoints 
	(but possibly with different directions at endpoints !!) */
    
    /* jlook1 equals jc */
    go_on=1;
    do{
      if (IONumber==2) return (jc);
      if (jlook1->next==jlook1->junction){
	jlook1=jlook1->junction;
	if (jlook1==jc) break;}
      ox1=jlook1->x;oy1=jlook1->y;ox2=jlook1->junction->x;oy2=jlook1->junction->y;
      jlook2=jlook1->next;
      if ((jlook2->x==ox1) && (jlook2->y==oy1) && 
	  (jlook2->junction->x==ox2) && (jlook2->junction->y==oy2)){
	jlook1->gray2=jlook2->gray2;
	jlook2->junction->next->gray1=jlook1->gray2;
	if (jlook2->next==jlook2->junction){
	  jlook1->next=jlook1->junction;
	  jlook1->junction->previous=jlook1;}
	else {
	  jnext=jlook2->next;
	  jprev=jlook2->junction->previous;
	  jlook1->next=jnext;
	  jnext->previous=jlook1;
	  jlook2->junction->next->previous=jprev;
	  jprev->next=jlook2->junction->next;}
	free((void*)(jlook2->junction));
	free((void*)(jlook2));
	IONumber-=2;}
      else{
	jlook1=jlook1->next;
	if (jlook1==jc) go_on=0;}}
    while (go_on);
    return (jc);
  }

/***************************************************************************/

/* In the same time that we construct the chain "jordan" of level lines, we construct the 
   jordan curve made of the vertices of the polygonal lines enclosing the occlusion. */

jordan *allocation_jordan_curve(instart,jc,previous_direction,next_direction,i,j)
unsigned char *instart;
jordan *jc;
char previous_direction,next_direction;
int i,j;

  {
    unsigned char *ptrin;
    unsigned char gray1,gray2;

    /* The eight neighbors of a pixel are numbered using the following convention
                                1 0 7  
                                 \|/   
                               2-- --6 
                                 /|\	
                                3 4 5

       Each pixel has four vertices whose coordinates are half-integer. Level lines pass
       through this type of points
                            s1              s2
                     (i-0.5,j-0.5)  (i-0.5,j+0.5)
                                *----*
                                |    |
                                |    |
                                *----*
                     (i+0.5,j-0.5)  (i+0.5,j+0.5)
                       	    s3             s4
    */
    
    switch (previous_direction)
      {
      case 0:case 7:
	if (next_direction==6) {mwerror(FATAL,1,"Impossible 1\n");break;}
	if (previous_direction==0)
	  if (next_direction==7) {mwerror(FATAL,1,"Impossible 2\n");break;}
	/* jc=add_to_jordan(jc,(double)i-0.5,(double)j+0.5,6);*/ /* predecessor already made */
	if (next_direction==5) {add_to_frontier((double)i+0.4999,(double)j+0.5001);break;}
	ptrin=instart+(i*col_number+j+1);gray2=*ptrin;ptrin+=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j+0.5,(char)6,gray1,gray2,(char)0);
	if (next_direction==4) break;
	gray2=gray1;ptrin--;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j+0.5,(char)4,gray1,gray2,(char)1);
	if (next_direction==3) {add_to_frontier((double)i+0.5001,(double)j-0.4999);break;}
	gray2=gray1;ptrin--;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j-0.5,(char)4,gray1,gray2,(char)0);
	if (next_direction==2) break;
	gray2=gray1;ptrin-=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j-0.5,(char)2,gray1,gray2,(char)1);
	if (next_direction==1) {add_to_frontier((double)i-0.4999,(double)j-0.5001);break;}
	gray2=gray1;ptrin-=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j-0.5,(char)2,gray1,gray2,(char)0);
	if (next_direction==0) break;
	gray2=gray1;ptrin++;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j-0.5,(char)0,gray1,gray2,(char)1);
	add_to_frontier((double)i-0.5001,(double)j+0.4999); 
	/* This slight translation is used to obtain a simple curve */
	break; /* next_direction and previous_direction equal 7 */
      case 5:case 6:
	if (next_direction==4) {mwerror(FATAL,1,"Impossible 3\n");break;}
	if (previous_direction==6)
	  if (next_direction==5) {mwerror(FATAL,1,"Impossible 4\n");break;}
	/*jc=add_to_jordan(jc,(double)i+0.5,(double)j+0.5,4);*/ /* predecessor already made */
	if (next_direction==3) {add_to_frontier((double)i+0.5001,(double)j-0.4999);break;}
	ptrin=instart+((i+1)*col_number+j);gray2=*ptrin;ptrin--;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j-0.5,(char)4,gray1,gray2,(char)0);
	if (next_direction==2) break;
	gray2=gray1;ptrin-=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j-0.5,(char)2,gray1,gray2,(char)1);
	if (next_direction==1) {add_to_frontier((double)i-0.4999,(double)j-0.5001);break;}
	gray2=gray1;ptrin-=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j-0.5,(char)2,gray1,gray2,(char)0);
	if (next_direction==0) break;
	gray2=gray1;ptrin++;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j-0.5,(char)0,gray1,gray2,(char)1);
	if (next_direction==7) {add_to_frontier((double)i-0.5001,(double)j+0.4999);break;}
	gray2=gray1;ptrin++;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j+0.5,(char)0,gray1,gray2,(char)0);
	if (next_direction==6) break;
	gray2=gray1;ptrin+=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j+0.5,(char)6,gray1,gray2,(char)1);
	add_to_frontier((double)i+0.4999,(double)j+0.5001);
	break;  /* next_direction and previous_direction equal 5 */
      case 3:case 4:
	if (next_direction==2) {mwerror(FATAL,1,"Impossible 5\n");break;}
	if (previous_direction==4)
	  if (next_direction==3) {mwerror(FATAL,1,"Impossible 5\n");break;}
	/*jc=add_to_jordan(jc,(double)i+0.5,(double)j-0.5,2);*/ /* predecessor already made */
	if (next_direction==1) {add_to_frontier((double)i-0.4999,(double)j-0.5001);break;}
	ptrin=instart+(i*col_number+j-1);gray2=*ptrin;ptrin-=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j-0.5,(char)2,gray1,gray2,(char)0);
	if (next_direction==0) break;
	gray2=*ptrin;ptrin++;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j-0.5,(char)0,gray1,gray2,(char)1);
	if (next_direction==7) {add_to_frontier((double)i-0.5001,(double)j+0.4999);break;}
	gray2=*ptrin;ptrin++;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j+0.5,(char)0,gray1,gray2,(char)0);
	if (next_direction==6) break;
	gray2=*ptrin;ptrin+=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j+0.5,(char)6,gray1,gray2,(char)1);
	if (next_direction==5) {add_to_frontier((double)i+0.4999,(double)j+0.5001);break;}
	gray2=*ptrin;ptrin+=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j+0.5,(char)6,gray1,gray2,(char)0);
	if (next_direction==4) break;
	gray2=*ptrin;ptrin--;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j+0.5,(char)4,gray1,gray2,(char)1);
	add_to_frontier((double)i+0.5001,(double)j-0.4999);
	break;  /* next_direction and previous_direction equal 3 */
      case 1:case 2:
	if (next_direction==0) {mwerror(FATAL,1,"Impossible 7\n");break;}
 	if (previous_direction==2)
	  if (next_direction==1) {mwerror(FATAL,1,"Impossible 8\n");break;}
	/*jc=add_to_jordan(jc,(double)i-0.5,(double)j-0.5,0);*/ /* predecessor already made */
	if (next_direction==7) {add_to_frontier((double)i-0.5001,(double)j+0.4999);break;}
	ptrin=instart+((i-1)*col_number+j);gray2=*ptrin;ptrin++;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j+0.5,(char)0,gray1,gray2,(char)0);
	if (next_direction==6) break;
	gray2=*ptrin;ptrin+=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i-0.5,(double)j+0.5,(char)6,gray1,gray2,(char)1);
	if (next_direction==5) {add_to_frontier((double)i+0.4999,(double)j+0.5001);break;}
	gray2=*ptrin;ptrin+=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j+0.5,(char)6,gray1,gray2,(char)0);
	if (next_direction==4) break;
	gray2=*ptrin;ptrin--;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j+0.5,(char)4,gray1,gray2,(char)1);
	if (next_direction==3) {add_to_frontier((double)i+0.5001,(double)j-0.4999);break;}
	gray2=*ptrin;ptrin--;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j-0.5,(char)4,gray1,gray2,(char)0);
	if (next_direction==2) break;
	gray2=*ptrin;ptrin-=col_number;gray1=*ptrin;
	jc=add_to_jordan(jc,(double)i+0.5,(double)j-0.5,(char)2,gray1,gray2,(char)1);
	add_to_frontier((double)i-0.4999,(double)j-0.5001);
	break;  /* next_direction and previous_direction equal 1 */
      default:
	mwerror(FATAL,1,"Impossible global\n");
	break;
      }
    return jc;
  }
/***************************************************************************/

/* Here we scan image for computing level lines input and output, construct the
jordan curve, compute the optimal set of junctions and computes the final result */

void perform_disocclusion(ptrin,gray_value,ptr_label,value,ptrout)

register unsigned char *ptrin,*ptrout;
unsigned char gray_value; /* value of a pixel connected to the occlusion but out of it */
register float *ptr_label;
float value;

  {
    int i,j;
    int istart,jstart;
    char next_direction,previous_direction;
    char previous_direction_start,next_direction_start;
    jordan *jc,*jci;
    frontier *jbi;
    unsigned char *instart;
    float *label_start,*central_label;
    int level_number,new_level_number;
    unsigned char *level_sort,*ptr_sort,*level_array;
    unsigned char gray1,gray2;
    char neighbors; /* equal to 1 if the first point of the jordan curve has neighbors, 0 else */
    char encore;
    unsigned char ExternVal=0;
    int Line;
    int baire;
    double bx,by,norm_v,bvalue;
    Ppoly_t poly;
    int occlusionArea; /* Number of occluding points */
    int Number=0;

    /*        1   0
            . | . | .
	  2 - ----- - 7
	    . | x | .
          3 - ----- - 6
	    . | . | .
	      4   5
    */

    label_start=ptr_label;
    instart=ptrin;
    lvalue=value;

    /* Coordinates of level lines refer to the first line of the set 
       (equal to globx) and to column 0 */
    occlusionArea=0;
    for (i=0;i<line_number-globx;i++){
      encore=0; /* if there is no pixel of occlusion on a line, it is unnecessary to 
		   study the next line */
      for (j=0;j<col_number;j++,ptr_label++)
	if (*ptr_label==value) {encore=1;occlusionArea++;}
      if (!encore) break;}

    /*
      We are going to compute the set of T-junctions, starting from the first pixel encountered 
      when the image is scanned from top to bottom and left to right. This point is used to 
      initialize the jordan curve. One looks for its predecessor and successor on the 
      occlusion boundary walked along clockwise, and computes all nodes associated to this pixel
      that could be input or output nodes for a level lines (there are at most 4 nodes for 
      each pixel, see figure below).
                            s1              s2
                     (i-0.5,j-0.5)  (i-0.5,j+0.5)
                                *----*
                                |    |
                                |    |
                                *----*
                     (i+0.5,j-0.5)  (i+0.5,j+0.5)
                       	    s3             s4
      One node can be reached by lines issuing from different directions, thus one needs
      the following convention :
      At each node, one distinguishes possible directions by using numbers (see figure below)
                           0
			 2 * 6   ( * denotes the node)
                           4
      We add to structure "jordan" each possible combination 'node+direction' if it corresponds
      to a level line. Coordinates of the node are half-integer.
      
      The figure below shows all possible combinations predecessor(1)->current pixel(2)->successor(3)
      (up to pi/2 rotations). Recall that the occlusion boundary is walked along clockwise.

          3
      1 2   --   1 2 3   --   1 2   --   1 2    --  1 2     -- 1(=3) 2
                                  3        3        3

      3           3               3     
        2         2             2          2 3        2            2           2
      1    --   1        --   1     --   1      --  1   3  --    1 3   -- 1(=3)


     Examples of possible level lines nodes for one example of configuration :
                     3
              1 . 2 ._
                |   |

     Now a new current point is a former successor; progression is made clockwise in order to have 
     at the end a "jordan" chain described counterclockwise (this is only valid for an "extern" curve)

     We already know the predecessor of this new current point. We only have to search for its successor 
     (clockwise). When we know it the process is as before. We go on until getting back to the first
     point. Remark that this point may produce new nodes (in so far as we follow the other side of
     the point)
     Each node contains 3 informations : coordinates, direction of the associated level line, 
     gray levels associated to the level line and given counterclockwise when one walks from 
     the line towards the node.

     At each pixel we look for the position of the successor with respect to the position of
     the predecessor. We take as a convention that these positions are numbered in the following
     way :
                   1 0 7  
                    \|/   
                  2-- --6 
                    /|\	
                   3 4 5
      Then, depending on the predecessor position we can restrict the range of possibilities
      for the successor position.
           predecessor position -> initial position for seeking the successor position
                    0 or  7     ->  start at 5
	            1 or 2      ->  start at 7
	            3 or 4      ->  start at 1
	            5 or 6      ->  start at 3
     The scan is performed by "decreasing" the position rank since the occlusion boundary is 
     walked along clockwise 


     Simultaneously to the computation of the set of T-junctions, we also construct the polygonal line 
     describing the occlusion boundary */

    jc=(jordan*)NULL;jb=(frontier*)NULL;
    numfrontier=0;
    ptr_label=label_start+globy;
    i=0;j=globy;
    previous_direction=2;neighbors=1;
    central_label=ptr_label;
    /* We first seek for the predecessor */
    do{
      ptr_label=central_label;
      previous_direction=(previous_direction+1)%8;
      switch (previous_direction){
      case 3:{ptr_label+=col_number-1;break;}
      case 4:{ptr_label+=col_number;break;}
      case 5:{ptr_label+=col_number+1;break;}
      case 6:{ptr_label++;break;}
      default:
	neighbors=0;
	break;}}
    while (((*ptr_label)!=value) && (neighbors));
    if (!neighbors){ /* "neighbors" equals 1 if the first occlusion point has neighbors 
			(i.e. the occlusion does not reduce to a single point), 0 else */
      ptrin=instart+((i-1)*col_number+j-1);
      gray2=*ptrin;ptrin++;gray1=*ptrin;
      jc=add_to_jordan(jc,(double)i-0.5,(double)j-0.5,(char)0,gray1,gray2,(char)1);
      gray2=gray1;ptrin++;gray1=*ptrin;
      jc=add_to_jordan(jc,(double)i-0.5,(double)j+0.5,(char)0,gray1,gray2,(char)1);
      gray2=gray1;ptrin+=col_number;gray1=*ptrin;
      jc=add_to_jordan(jc,(double)i-0.5,(double)j+0.5,(char)6,gray1,gray2,(char)0);
      gray2=gray1;ptrin+=col_number;gray1=*ptrin;
      jc=add_to_jordan(jc,(double)i+0.5,(double)j+0.5,(char)6,gray1,gray2,(char)1);
      gray2=*ptrin;ptrin--;gray1=*ptrin;
      jc=add_to_jordan(jc,(double)i+0.5,(double)j+0.5,(char)4,gray1,gray2,(char)0);
      gray2=*ptrin;ptrin--;gray1=*ptrin;
      jc=add_to_jordan(jc,(double)i+0.5,(double)j-0.5,(char)4,gray1,gray2,(char)1);
      gray2=*ptrin;ptrin-=col_number;gray1=*ptrin;
      jc=add_to_jordan(jc,(double)i+0.5,(double)j-0.5,(char)2,gray1,gray2,(char)0);
      gray2=*ptrin;ptrin-=col_number;gray1=*ptrin;
      jc=add_to_jordan(jc,(double)i-0.5,(double)j-0.5,(char)2,gray1,gray2,(char)0);}
    else{
      /* Initially previous_direction=2 */
      next_direction=7;
      do{
	ptr_label=central_label;
	next_direction=(next_direction+7)%8;
	switch (next_direction){
	case 6:{ptr_label++;break;}
	case 5:{ptr_label+=col_number+1;break;}
	case 4:{ptr_label+=col_number;break;}
	case 3:{ptr_label+=col_number-1;break;}
	case 2:{ptr_label--;break;}
	default:
	  mwerror(FATAL,1,"Cas de figure impossible !\n");
	  break;}}
      while ((*ptr_label)!=value);
      istart=i;jstart=j;
      previous_direction_start=previous_direction;next_direction_start=next_direction;
      do{
	jc=allocation_jordan_curve(instart,jc,previous_direction,next_direction,i,j);
	previous_direction=(next_direction+4)%8;
	switch (next_direction){
	case 0:i--;break;
	case 1:i--;j--;break;
	case 2:j--;break;
	case 3:i++;j--;break;
	case 4:i++;break;
	case 5:i++;j++;break;
	case 6:j++;break;
	case 7:i--;j++;break;}
	next_direction=((((previous_direction+1)%8)/2)*2+6)%8;
	central_label=ptr_label;
	do{
	  ptr_label=central_label;
	  next_direction=(next_direction+7)%8;
	  switch (next_direction){
	  case 0:{ptr_label-=col_number;break;}
	  case 7:{ptr_label-=col_number-1;break;}
	  case 6:{ptr_label++;break;}
	  case 5:{ptr_label+=col_number+1;break;}
	  case 4:{ptr_label+=col_number;break;}
	  case 3:{ptr_label+=col_number-1;break;}
	  case 2:{ptr_label--;break;}
	  case 1:{ptr_label-=col_number+1;break;}}}
	while ((*ptr_label)!=value);
	if (i==istart)
	  if (j==jstart)
	    if (previous_direction==previous_direction_start)
	      if (next_direction==next_direction_start)
		break;}
      while (1);}

    /* The jordan curve contains now all level lines inputs and outputs. Careful that
       one input point can be attained by several level lines. The distinction will me made later*/

    if (!jc){
      /*mwerror(WARNING,1,"All level lines are parallel to occlusion %d %d\n",globx,globy);*/
      for (i=0,ptr_label=label_start;i<line_number-globx;i++){
	encore=0;
	for (j=0;j<col_number;j++,ptr_label++,ptrout++)
	  if (*ptr_label==value){
	    encore=1;
	    *ptr_label=(-2);  /*  Avoids a future reexamination of this occlusion */
	    *ptrout=gray_value;}
	if (!encore) break;}
      return;}
    
    /* We now examine all gray values in the jordan curve; we make a list of these values,
       perform a quick sort and remove redundancy. The reason for such an operation is given 
       below */
    jci=jc;level_number=0;
    while (jci){
      level_number+=2;
      jci=jci->next;}
    level_sort=(unsigned char*)malloc((size_t)(level_number*sizeof(unsigned char)));
    if (!level_sort) mwerror(FATAL,1,"Not enough memory (4)!\n");
    for (jci=jc,ptr_sort=level_sort;(jci);jci=jci->next){
      *ptr_sort++=jci->gray1;
      *ptr_sort++=jci->gray2;}
    qsort((void*)level_sort,(size_t)level_number,(size_t)sizeof(unsigned char),compar_uc);
    /* We now remove every redundancy in array level_sort */
    new_level_number=1;
    i=1;
    while (i<level_number){
      if (level_sort[i]!=level_sort[i-1]) new_level_number++;
      i++;}
    level_array=(unsigned char*)malloc((size_t)(new_level_number*sizeof(unsigned char)));
    if (!level_array) mwerror(FATAL,1,"Not enough memory (5)!\n");
    level_array[0]=level_sort[0];
    i=1;j=1;
    while (i<level_number){
      if (level_sort[i]!=level_sort[i-1]) level_array[j++]=level_sort[i];
      i++;}
    level_number=new_level_number;
    free((void*)level_sort);
    level_sort=NULL;
    
    /* "level_number" is now the total number of gray levels corresponding to level lines
       entering the occlusion. "level_array" is the array of those gray levels sorted increasingly*/


    /* We shall now update the jordan curve representing T-junctions : after 
       the previous construction of the jordan curve, two neighbors with 
       different brightnesses (b1 and b2) may be separated by a single level line. 
       But if there is another T-junction associated with a gray level b3 such that b1<b3<b2 
       then we have to split the level line (b1,b2) into two level lines (b1,b3) and (b3,b2) 
       and thus we obtain "two" T-junctions. This creation of a new T-junction is performed 
       through the function "insert_jordan" */
    
    jci=jc;
    while (jci)
      {
	if (jci->gray1<jci->gray2){
	  i=0;
	  while (level_array[i]<jci->gray1) i++;
	  gray2=jci->gray2;i++;jci->gray2=level_array[i];
	  while (level_array[i]<gray2){
	    jci=insert_jordan(jci,level_array[i],level_array[i+1]);
	    i++;}}
	else{
	  i=level_number-1;
	  while (jci->gray1<level_array[i]) i--;
	  i--;gray2=jci->gray2;jci->gray2=level_array[i];
	  while (gray2<level_array[i]){
	    i--;
	    jci=insert_jordan(jci,level_array[i+1],level_array[i]);}}
	jci=jci->next;
      }
    
    jci=jc;
    IONumber=0;
    while (jci){
      jci=jci->next;
      IONumber++;}
  
    free((void*)level_array);
    level_array=NULL;

    /* Now we compute an approximation of the real direction : the computation is exact when 
       the angular sector in the vicinity of T-junction that does not intersect the occlusion is 
       exactly divided into two lower and upper connected components. 
       We denote by (bx,by) the coordinates of the barycenter of every point in the angular 
       sector with radius 4, which shall be considered as the central point of the level line */

    if (anglep){
      jci=jc;
      while (jci){
	baire=0;
	bx=0.0;by=0.0;
	bvalue=((double)(jci->gray1+jci->gray2))/2;
	for (i=Max(0,(int)(jci->x+globx-3.5));i<Min(line_number,(int)(jci->x+globx+3.5));i++)
	  for (j=Max(0,(int)(jci->y-3.5));j<Min(col_number,(int)(jci->y+3.5));j++)
	    {
	      ptrin=IImage+i*col_number+j;
	      ptr_label=LImage+i*col_number+j;
	      if (!(*ptr_label))
		{
		  if (!(*(ptr_label+1)))
		    if (Sgn(*ptrin-bvalue)==Sgn(bvalue-*(ptrin+1)))
		      {baire++;bx+=(double)i;by+=(double)j+0.5;}
		  if (!(*(ptr_label+col_number)))
		    if (Sgn(*ptrin-bvalue)==Sgn(bvalue-*(ptrin+col_number)))
		      {baire++;bx+=(double)i+0.5;by+=(double)j;}
		}
	    }
	if (!baire) prerror("Error in direction computation");
	bx=bx/baire;
	by=by/baire;
	if ((fabs(jci->x+globx-bx)>1e-8)||(fabs(by-jci->y)>1e08))
	  {
	    jci->vx=jci->x+globx-bx;jci->vy=jci->y-by;
	    norm_v=Norm(jci->vx,jci->vy);
	    jci->vx=(jci->vx)/norm_v;jci->vy=(jci->vy)/norm_v;
	  }
	jci=jci->next;}}

    /* We close the jordan curve (required by ComputeOptimalSet for testing the 
       validity of a junction) */
    jci=jc;
    while (jci->next) jci=jci->next;
    jci->next=jc;
    jc->previous=jci;

    /* We close the frontier curve */
    jbi=jb;
    while (jbi->next) jbi=jbi->next;
    jbi->next=jb;
    jb->previous=jbi;
    
    copy_frontier(globx);
    poly.ps = &polypoints[0];
    poly.pn = numfrontier;

    Triangulation(&poly); /* Triangulation of the occlusion boundary */
    
    if (IONumber>2) ComputeOptimalSet(jc); /* Computation of the optimal set of T-junctions */
    else {
      printf("energy=%.2f  ",CCost(jc,jc->next));fflush(stdout);
      jc->junction=jc->next;jc->junction->junction=jc;}

    jc=update_jordan_curve(jc,&ExternVal);
    
    if (!jc){ /* After reduction the set of T-junctions is empty thus disocclusion is trivial */
      FreeTPath();
      if (polypoints) 
	{free((void*)polypoints);polypoints=NULL;}
      jb->previous->next=(frontier*)NULL;
      jb->previous=(frontier*)NULL;
      free_frontier(jb);jb=(frontier*)NULL;
      for (i=0,ptr_label=label_start;i<line_number-globx;i++){
	encore=0;
	for (j=0;j<col_number;j++,ptr_label++,ptrout++)
	  if (*ptr_label==value){
	    encore=1;
	    *ptr_label=(-2);  /*  Avoids a future reexamination of this occlusion */
	    *ptrout=ExternVal;}
	if (!encore) break;}
      return;}

    numcolored=0;
    occlusion=(occlusionPoint*)malloc((size_t)occlusionArea*sizeof(occlusionPoint));
    if (!occlusion) mwerror(FATAL,1,"Not enough memory (6)!\n");
    geodesic(jc,globx); /* Geodesic paths are drawn between optimal couple of T-junctions */
    propagation(occlusionArea); /* The remaining part of the occlusion is processed */
    free((void*)occlusion);
    occlusion=NULL;

    jc->previous->next=NULL;
    jc->previous=NULL;
    free_jordan(jc);jc=(jordan*)NULL;

    jb->previous->next=(frontier*)NULL;
    jb->previous=(frontier*)NULL;
    free_frontier(jb);jb=(frontier*)NULL;

    /***********************************************/
    /* DO NOT FORGET (-1) !!!!!!!!!!!!!!!!!!!!!!!! */
    /***********************************************/
    for (i=0,ptr_label=label_start;i<line_number-globx;i++){
      encore=0;
      for (j=0;j<col_number;j++,ptr_label++)
	if ((*ptr_label==value)||(*ptr_label==(-1))){
	  encore=1;
	  *ptr_label=(-2);} /*  Avoids a future reexamination of this occlusion */
    if (!encore) break;}
    return;
  }

/***************************************************************************/

/* Computes cost of the junction ll1->ll2, i.e. the length of the geodesic path joining 
   these two T-junctions + the two angles between the direction of the path and the 
   directions of the entering level lines*/

/* We have as a convention for level lines vectors to take them
   always "coming in". Thus, if v is null, the best case is
   ==><== (v1=-v2) and the worst case is :
   =====> (v1=v2)  */

/*
  a    |      |   a
-------|   x  |------   /\
  b    |<---->|   b     ][ y
-------|      |------   \/
  c    |      |   c

The parallel junctions are prefered to orthogonal ones if 2+y*CPL>x*CPL
If x<y inequality is always satisfied.
If x>y we must have x-y < 2/CPL. Assume that we want it to be satisfied when y<x<10y.
So we need CPL < 2/9.
*/

double CCost(jc1,jc2)
jordan *jc1,*jc2;

  {
    double v1x=jc1->vx,v1y=jc1->vy,v2x=jc2->vx,v2y=jc2->vy; /* input vectors */
    double vx=jc2->x-jc1->x,vy=jc2->y-jc1->y;
    double norm_v;
    int weight;
    double TVangle=0.0;

    /*   --> ----> <---
	 v1    v      v2   */
    if (Norm(v1x,v1y)<1e-7 || Norm(v2x,v2y)<1e-7) mwerror(FATAL,1,"Angle problem !\n");

    weight=abs((int)(jc1->gray2)-(int)(jc1->gray1));

    norm_v=Norm(vx,vy);
    if (norm_v>1e-7) 
      {
	norm_v=GeodesicDistance(jc1,jc2,&TVangle);
	switch (energy_criterion)
	  {
	  case 0 : 
	    return (weight*norm_v);
	    break;
	  case 1 : 
	    return (weight*TVangle);
	    break;
	  default : 
	    return (weight*(TVangle+CPL*norm_v));
	    break;
	  }
      }
    else
      {
	switch (energy_criterion)
	  {
	  case 0: 
	    return (0.0);
	    break;
	  default: 
	    if (anglep) return (weight*fabs(atan2(v1y*v2x-v1x*v2y,-v1x*v2x-v1y*v2y)));
	    else return (weight*M_PI_2);
	    break;
	  }
      }
  }

/***************************************************************************/

/* After computation of minimal energy, the associated set of junctions is updated */

void joinback(i,j)
int i,j;

  {
    if (correspond[i][(j-1)/2]==(-1))
      {
	(element[i])->junction=element[(i+j)%IONumber];
	(element[(i+j)%IONumber])->junction=element[i];
	/* This line has to be removed if jc is transformed in a junction set */
	/*jcj->junction=jci;*/
	if (j>1) joinback((i+1)%IONumber,j-2);
	return;
      }
    joinback(i,correspond[i][(j-1)/2]);
    joinback((i+correspond[i][(j-1)/2]+1)%IONumber,j-correspond[i][(j-1)/2]-1);
    return;
  }
/***************************************************************************/

/* Computes minimal energy within the arc defined by segment [i, i+j] */

double ComputeEnergy(i,j)
int i,j;

  {
    jordan *jci,*jcj;
    int *corresp;
    int k;
    double Emin,Eminold,Etool1,Etool2;

    if (energy[i][(j-1)/2]>(-2)) return(energy[i][(j-1)/2]);
    jci=element[i];
    jcj=element[(i+j)%IONumber];
    corresp=correspond[i]+(j-1)/2;
    if ((jci->gray1==jcj->gray2)&&(jci->gray2==jcj->gray1)){
      Etool1=CCost(jci,jcj);
      Etool2=ComputeEnergy((i+1)%IONumber,j-2);
      if (Etool2>(-1)){
	Emin=Etool1+Etool2;
	*corresp=(-1); 
	/* means that it is equivalent to make junction (i,i+j) and 
	   to construct optimal set within [i+1,i+j-1] */}
      else Emin=(-1);}
    else Emin=(-1);
    for (k=1;k<j-1;k+=2)
      {
	Eminold=Emin;
	Etool1=ComputeEnergy(i,k);
	if (Etool1>(-1))
	  if ((Etool1<Emin)||(Emin==(-1))){
	    Etool2=ComputeEnergy((i+k+1)%IONumber,j-k-1);
	    if (Etool2>(-1))
	      if (Emin>(-1)) Emin=Min(Emin,Etool1+Etool2);
	      else Emin=Etool1+Etool2;}
	if (Eminold!=Emin) *corresp=k; 		
      }
    energy[i][(j-1)/2]=Emin;
    return(Emin);
  }
/***************************************************************************/

/* Computes the optimal set of connections */

void ComputeOptimalSet(jc)
jordan *jc;
  {
    jordan *jci,*jcj;
    int i,j,k;
    double Emin,Eminold,Etool1,Etool2;
    int *corresp;
    
/* The three arrays used here have the following structure, where the line index refers to the position 
   on the Jordan curve whose vertices are the T-junctions and the column index is associated with the length
   of the current arc

               0  1  2  .....  IONumber/2-1
          0  |                      |
          1  |                      |
          2  |                      |
          :  |                      |
IONumber-1---|----------------------  

   By energy[i][j] we mean the energy of the arc between the T-junctions i and i+(j*2+1), taken
   counterclockwise on the Jordan curve. Remark that energy[i][j] and energy[j][i] needs not be equal !
   The reason why the column index does not go between 0 and IONumber-1 is that, for obvious reasons, we are 
   interested only in those arcs with an even number of T-junctions.
   energy[i][j]=(-2) means it has not been yet computed.
   energy[i][j]=(-1) means infinite energy (no possible connection)

   correspond[i][j] gives the position where to cut the arc in order to compute the optimal set of connections.
   For example correspond[i][j]=k (>=0) means that the optimal solution is given by splitting the 
   arc [i,i+(j*2+1)] into the two arcs [i,i+k] and [i+k+1,i+(j*2+1)].
   By correspond[i][j]=(-1) we mean that the optimal solution is given by connecting the two T-junctions i and
   i+(j*2+1) and taking the optimal solution within [i+1,i+(j*2)]
   
   Finally element[i] is simply the element of the Jordan structure of T-junctions associated with
   the T-junction i

 */

    energy=(double**)malloc(IONumber*sizeof(double*));
    correspond=(int**)malloc(IONumber*sizeof(int*));
    element=(jordan**)malloc(IONumber*sizeof(jordan*));
    printf("%4d T-junctions  ",IONumber);fflush(stdout);
    if (!energy || !correspond || !element) mwerror(FATAL,1,"Not enough memory (7)!\n");
    jci=jc;i=0;
    do{
      element[i]=jci;
      i++;
      jci=jci->next;}
    while (jci!=jc);
    for (i=0,jci=jc;i<IONumber;i++,jci=jci->next){
      energy[i]=(double*)malloc((IONumber/2)*sizeof(double));
      correspond[i]=(int*)malloc((IONumber/2)*sizeof(int));
      if (!(energy[i]) || !(correspond[i])) mwerror(FATAL,1,"Not enough memory (8)!\n");
      for (j=1;j<IONumber/2;j++) energy[i][j]=(-2); /* means it has not been calculated */
      jcj=jci->next;
      if ((jci->gray1==jcj->gray2)&&(jci->gray2==jcj->gray1)) energy[i][0]=CCost(jci,jcj);
      else energy[i][0]=(-1);
      correspond[i][0]=(-1); /* so that junction j_i -> j_{i+1} be not further examined */}
    corresp=correspond[0]+(IONumber-2)/2;
    if ((jc->gray1==jc->previous->gray2)&&(jc->gray2==jc->previous->gray1)){
      Etool1=CCost(jc,jc->previous);
      Etool2=ComputeEnergy(1,IONumber-3);
      if (Etool2>(-1)){
	Emin=Etool1+Etool2;
	*corresp=(-1);
	/* means that it is equivalent to cut with k=(-1) */}
      else Emin=(-1);}
    else Emin=(-1);
    for (k=1;k<IONumber-2;k+=2){
      Eminold=Emin; 
      Etool1=ComputeEnergy(0,k);
      if (Etool1>(-1))
	if ((Etool1<Emin)||(Emin==(-1))){
	  Etool2=ComputeEnergy((k+1)%IONumber,IONumber-k-2);
	  if (Etool2>(-1))
	    if (Emin>(-1)) Emin=Min(Emin,Etool1+Etool2);
	    else Emin=Etool1+Etool2;}
      if (Eminold!=Emin) *corresp=k;}
	
    if (Emin==(-1)) mwerror(FATAL,1,"Code error (please contact administrator)\n");
    printf("energy=%5.2f",Emin);fflush(stdout);
    joinback(0,IONumber-1);
    
    for (i=0;i<IONumber;i++){
      free((void*)(energy[i]));
      free((void*)(correspond[i]));}
    free((void*)energy);
    free((void*)correspond);
  }

/****************************************************************************/

void disocclusion(Input,Output,Holes,energy_type,angle)
Cimage Input,Output;
Fimage Holes;
char *angle;
int *energy_type;

  {
    unsigned char *BInput,*BOutput; /* Correspond to an extension of one pixel in every direction 
				       of Input->gray and Output->gray */
    register unsigned char *ptrin,*ptrout,*ptrBin,*ptrBout;
    register int x,y;
    float *BHoles; /* Correspond to an extension of one pixel in every direction 
		      of Holes->gray */
    register float *ptr_label,*ptr_Bholes;
    float value;
    int Line,Col,Number=0;
    int bordersize,border,border1,border2,lastborder;
    int *Border,*ptrBord,*ptrBord1,*ptrBord2;
    int value1,value2,attrib;
    
    energy_criterion=*energy_type;
    anglep=angle;

    line_number=Input->nrow;col_number=Input->ncol;

    if ((Holes->nrow!=line_number)||(Holes->ncol!=col_number))
      mwerror(FATAL,1,"Input and Holes images must have same dimensions\n");
    
    Output = mw_change_cimage(Output,line_number,col_number);
    if (Output==NULL) mwerror(FATAL,1,"Not enough memory (9)!\n");

    if ((line_number<2)||(col_number<2))
      mwerror(FATAL,1,"This case cannot be processed. Please send any complaint to the following URL : /dev/null !\n");
    

    Line=line_number+2;Col=col_number+2;
    BInput=(unsigned char*)malloc((size_t)(Line*Col*sizeof(unsigned char)));
    BOutput=(unsigned char*)malloc((size_t)(Line*Col*sizeof(unsigned char)));
    BHoles=(float*)malloc((size_t)(Line*Col*sizeof(float)));
    if (!BHoles || !BInput || !BOutput) mwerror(FATAL,1,"Not enough memory !\n");

    /* The following operations may be redundant with the labelling operation performed by  
       function "readregion" (if used for generating occlusions !). Here we actually fill every 
       occlusion since our method is valid only for simply connected occlusions. Remark that we 
       allow the occlusions to be connected to image borders and that we shall NOT consider as a 
       hole any region enclosed by an occlusion boundary and the image border */

    for (y=0,ptr_Bholes=BHoles;y<Col;y++,ptr_Bholes++) *ptr_Bholes=0.0;
    ptrBin=BInput+Col;
    for (x=0,ptr_label=Holes->gray,ptrin=Input->gray;x<line_number;x++)
      {
	*ptr_Bholes=0.0;ptr_Bholes++;ptrBin++;
	for (y=0;y<col_number;y++,ptr_label++,ptr_Bholes++,ptrin++,ptrBin++)
	  {
	    if ((*ptr_label)!=0.0) *ptr_Bholes=1.0;
	    else *ptr_Bholes=0.0;
	    *ptrBin=*ptrin;
	  }
	*ptr_Bholes=0.0;ptr_Bholes++;ptrBin++;
      }
    for (y=0;y<Col;y++,ptr_Bholes++) *ptr_Bholes=0.0;
    
    /* First step : holes are filled */
    value=0.0;
    fconnected(BHoles,Line,Col,value,&Number,(char*)NULL,(char*)NULL,(char*)1);

    /* Second step : occlusions are labelled */
    value=1.0;
    fconnected(BHoles,Line,Col,value,&Number,(char*)NULL,(char*)1,(char*)NULL);

    /* Extension of occluded image by addition of a one pixel wide line all around. We simply 
       replicate the values at the boundaries except at points which are 4-connected to occluding 
       points. In such a case, we interpolate by propagating the values already set (thus we must 
       assume that the image boundary is not an entire occlusion !). If the missing interval is made 
       of an odd number of points we take as a convention that the greatest value is always prefered. 
       sigmaB denotes the position of a point on the nonextended image boundary.
       sigmaB= a*col_number+b*(line_number-1) with a in [0,2] and b in [0,2[) assuming that the origin 
       has coordinates (1,1) in the extended image and the boundaries are walked along clockwise. */

    bordersize=2*col_number+2*line_number+4;
    Border=(int*)malloc((size_t)(bordersize*sizeof(int))); /* This array will describe the border */
    if (!Border) mwerror(FATAL,1,"Not enough memory !\n");
    for (border=0,ptrBin=BInput+Col+1,ptr_Bholes=BHoles+Col+1,ptrBord=Border;
	 border<bordersize-1;border++,ptrBord++)
      {
	if ((*ptr_Bholes)==0.0) attrib=(int)(*ptrBin);
	else attrib=(-1); /* means that the value is missing */
	if (border<=col_number) 
	  if (border==0) 
	    {Border[bordersize-1]=attrib;
	    *ptrBord=attrib;ptrBord++;border++;
	    *ptrBord=attrib;ptrBin++;ptr_Bholes++;}
	  else
	    if (border==col_number)
	      {
		*ptrBord=attrib;ptrBord++;border++;
		*ptrBord=attrib;ptrBord++;border++;
		*ptrBord=attrib;ptrBin+=Col;ptr_Bholes+=Col;
	      }
	    else {*ptrBord=attrib;ptrBin++;ptr_Bholes++;}
	else
	  if (border<=col_number+line_number+1) 
	    if (border==col_number+line_number+1)
	      {
		*ptrBord=attrib;ptrBord++;border++;
		*ptrBord=attrib;ptrBord++;border++;
		*ptrBord=attrib;ptrBin--;ptr_Bholes--;
	      }
	    else {*ptrBord=attrib;ptrBin+=Col;ptr_Bholes+=Col;}
	  else
	    if (border<=2*col_number+line_number+2)
	      if (border==2*col_number+line_number+2)
		{
		  *ptrBord=attrib;ptrBord++;border++;
		  *ptrBord=attrib;ptrBord++;border++;
		  *ptrBord=attrib;ptrBin-=Col;ptr_Bholes-=Col;
		}
	      else {*ptrBord=attrib;ptrBin--;ptr_Bholes--;}
	    else {*ptrBord=attrib;ptrBin-=Col;ptr_Bholes-=Col;}
      }
    
    /* Now we interpolate at these border points whose value is still missing */

    border=0;
    lastborder=bordersize-1;
    if (Border[bordersize-1]==(-1))
      {
	border1=bordersize-2;ptrBord=Border+border1;
	while (((*ptrBord)==(-1))&&(border1>1)) {ptrBord--;border1--;}
	if (border1==1) 
	  mwerror(FATAL,1,"This program cannot restore images whose borders are fully occluded\n");
	ptrBord1=ptrBord;value1=*ptrBord1;
	lastborder=border1;
	border2=2;ptrBord=Border+2;
	while ((*ptrBord)==(-1)) {ptrBord++;border2++;}
	ptrBord2=ptrBord;value2=*ptrBord2;
	border=border2;
	do
	  {
	    border2--;
	    if (border2<0) {border2=bordersize-1;ptrBord2=Border+border2;}
	    else ptrBord2--;
	    if (border2!=border1) /* If equality holds then there is an even number of points between 
				     the initial border1 and border2 */
				     
	      {
		*ptrBord2=value2;
		border1++;
		if (border1>=bordersize) {border1=0;ptrBord1=Border;}
		else ptrBord1++;
		if (border1!=border2) *ptrBord1=value1;
		else
		  if (value1>value2) *ptrBord1=value1; /* odd number of points between the initial border1
							  and border 2 */
	      }
	  }
	while (border1!=border2);
      }

    ptrBord=Border+border;
    while (border<lastborder)
      {
	do {ptrBord++;border++;}
	while (((*ptrBord)!=(-1))&&(border<lastborder));
	if (border<lastborder)
	  {
	    border1=border-1;ptrBord1=ptrBord-1;value1=*ptrBord1;
	    border2=border;ptrBord2=ptrBord;
	    while ((*ptrBord2)==(-1)) 
	      {ptrBord2++;border2++;
	      if (border2>lastborder) mwerror(FATAL,1,"Impossible !!!!\n");}
	    value2=*ptrBord2;
	    border=border2;ptrBord=ptrBord2;
	    do
	      {
		border2--;ptrBord2--;
		if (border2!=border1)				     
		  {
		    *ptrBord2=value2;
		    border1++;ptrBord1++;
		    if (border1!=border2) *ptrBord1=value1;
		    else
		      if (value1>value2) *ptrBord1=value1;
		  }
	      }
	    while (border1!=border2);
	  }
      }

    for (border=0,ptrBord=Border;border<bordersize;border++,ptrBord++)
      if ((*ptrBord)==(-1)) mwerror(FATAL,1,"Probleme !!!\n");
    

    /* The border of the big image "BInput" is given the values of the array "border" */
    for (border=0,ptrBord=Border,ptrBin=BInput;border<bordersize;border++,ptrBord++)
      {
	*ptrBin=(unsigned char)(*ptrBord);
	if (border<=col_number) ptrBin++;
	else
	  if (border<=col_number+line_number+1) ptrBin+=Col;
	  else
	    if (border<=2*col_number+line_number+2) ptrBin--;
	    else ptrBin-=Col;
      }

    for (x=0,ptrBin=BInput,ptrBout=BOutput;x<Line*Col;x++,ptrBin++,ptrBout++)
      *ptrBout=*ptrBin;
    

    /***** CAREFUL !! *****/
    line_number=Line;col_number=Col;
    /*************************/

    IImage=BInput;OImage=BOutput;LImage=BHoles;

    for (x=0,ptr_label=LImage,ptrin=IImage,ptrout=OImage;x<line_number;x++)
      for (y=0;y<col_number;y++,ptrin++,ptr_label++,ptrout++)
	if ((*ptr_label)>0)
	  {
	    printf("\rline=%3d  column=%3d  label=%4d  ",x-1,y-1,(int)(*ptr_label));fflush(stdout);
	    globx=x;globy=y;
	    /* We start from the beginning of the line */
	    perform_disocclusion(ptrin-y,*(ptrin-1),ptr_label-y,*ptr_label,ptrout-y);
	  }
    printf("\n");
    
    /* We now recover the original image size and write the output */
    line_number-=2;col_number-=2;
    for (x=0,ptrout=Output->gray,ptrBout=BOutput+Col+1;x<line_number;x++,ptrBout+=2)
      for (y=0;y<col_number;y++,ptrout++,ptrBout++)
	*ptrout=*ptrBout;

    free((void*)BInput);
    free((void*)BOutput);
    free((void*)BHoles);
    free((void*)Border);
  }

/***************************************************************************/


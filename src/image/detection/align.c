/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {align};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"Detect (maximal) meaningful segments"};
  usage = {  
 'a'->all           "to get all meaningful segments (maximal or not)",
 'd':[d=16]->d      "number of allowed gradient directions, default 16",
 'n':[nl=96]->nl    "number of line directions to scan, default 96)",
 'g':[g=2.0]->g     "minimum gradient norm to define a direction, default 2.",
 'e':[eps=0.]->eps  "-log10(max. number of false alarms), default 0",
 'c':crv<-crv       "store segments as curves (output, Flists)",
  u->u              "input Fimage",
  out<-align        "result: 5-Flist (x1 y1 x2 y2 -log10(nfa))"
          };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

struct one_segment
{
  short start;    /* starting position (distance from border) */
  short end;      /* ending position (hence, length is end-start+1) */
  double nfa;     /* number of false alarms */
  char ok;       
};


/*------------------------------------------------------------*/
/*       compute P(k,l) : array out[] of size n+1 * n+1       */
/*   P[k][l] = m * sum(i=k..l) binom(l,i) p^i (1-p)^(l-i)     */
/*------------------------------------------------------------*/


double *tab(n,p,m)
int n;
double p;
double m;
{
  double *out;
  int adr1,adr2,x,y;
  double lambda,q;

  q = 1.0-p;
  out = (double *)calloc((n+1)*(n+1),sizeof(double));

  /*** compute proba (=x among y) ***/
  out[0] = 1.0;
  for (y=1,adr2=0;y<=n;y++) {
    adr1 = adr2;
    adr2 += n+1;    
    out[adr2] = q*out[adr1];
    for (x=1;x<=y;x++) 
      out[adr2+x] = p*out[adr1+x-1] + q*out[adr1+x];
  }  

  /*** sum to obtain proba (>=k among y) ***/
  for (y=1,adr1=n+1;y<=n;y++,adr1+=n+1) 
    for (x=y-1;x>=0;x--) 
      out[adr1+x] += out[adr1+x+1];

  /*** multiply by m (number of segments) to obtain expectation***/
  for (adr1=(n+1)*(n+1);--adr1>=0;)
    out[adr1] *= m;

  return out;
}


/*------------------------------------------------------------*/
/*    compute the direction of the level line at each point   */
/*------------------------------------------------------------*/

Fimage ll_angle(a,threshold)
Fimage a;
float threshold;
{
  Fimage g;
  int n,p,x,y,adr;
  double com1,com2,gx,gy,norm2;

  threshold *= threshold;
  n = a->nrow;
  p = a->ncol;
  g = mw_change_fimage(NULL,n,p);

  /*** downright boundary (orientation is not defined) ***/
  for (x=0;x<p;x++) 
    g->gray[(n-1)*p+x] = -1000.0;
  for (y=0;y<n;y++) 
    g->gray[p*y+p-1]   = -1000.0;
  
  /*** remaining part ***/
  for (x=0;x<p-1;x++) 
    for (y=0;y<n-1;y++) {
      adr = y*p+x;

      com1 = (double)(a->gray[adr+p+1])-(double)(a->gray[adr]);
      com2 = (double)(a->gray[adr+1])-(double)(a->gray[adr+p]);
      gx = 0.5*(com1+com2);
      gy = 0.5*(com1-com2);
      norm2 = gx*gx+gy*gy;

      if (norm2 <= threshold)
	g->gray[adr] = -1000.0;  /* orientation is not defined */
      else g->gray[adr] = (float)atan2(gx,-gy);
    }

  return g;
}


/*------------------------------------------------------------*/
/*                         MAIN MODULE                        */
/*------------------------------------------------------------*/


Flist align(u,d,nl,eps,g,all,crv)
Fimage u;
int    *d,*nl;
double *eps;
float  *g;
char   *all;
Flists crv;
{
  Flist result;
  Fimage angle;
  float *seglist;               /* list of recorded segments */
  int   iseglist,size_seglist;  /* associated counter and dynamic size */
  struct one_segment *seg;
  int iseg,size_seg;
  double *test,nfa,max_nfa;
  int *count,*startbloc,*endbloc;
  int mx,my,ox,oy,nx,ny,n;
  int xx,yy,pos,posmax,nblocs,inbloc,max_nblocs;
  int cur,i,j,side,tmp,l,lphase;
  int itheta,ntheta;
  float theta,theta0,dtheta,dx,dy,prec,error;

  max_nfa = pow(10.0,-(*eps));

  nx = u->ncol;
  ny = u->nrow;
  /* maximal length for a line */
  n = (int)ceil(hypot((double)nx,(double)ny))+1;

  /*** compute angle map of u ***/
  angle = ll_angle(u,*g);

  /*** compute P(k,l) ***/
  test = tab(n,1.0/(double)(*d),(double)(nx*ny)*(double)(nx*ny));


  /*** initialization ***/
  prec = M_PI/(float)(*d);
  ntheta = *nl/2;  /* i.e. # directions of NON-ORIENTED lines */
  dtheta = M_PI/(float)ntheta;


  /******************** memory allocation ********************/

  max_nblocs = n/2+1; /* maximal number of blocs */
  count =     (int *)malloc(max_nblocs*sizeof(int));
  startbloc = (int *)malloc(max_nblocs*sizeof(int));
  endbloc =   (int *)malloc(max_nblocs*sizeof(int));
  size_seg = 10000; /* initial allocation (may reallocate later) */
  seg = (struct one_segment *)malloc(size_seg*sizeof(struct one_segment));
  size_seglist = 10000; /* initial allocation (may reallocate later) */
  seglist = (float *)malloc(5*size_seglist*sizeof(float));
  if (!count || !startbloc|| !endbloc || !seg || !seglist) 
    mwerror(FATAL,1,"Not enough memory.");


  /* counter for recorded segments (seglist) */
  iseglist = 0;


  /******************** first loop : the four sides ********************/

  for (side=0;side<4;side++) {
    printf("side %d/4 ",side+1);

    theta0 = 0.5*M_PI*(double)side;
    mx = ((side==0 || side==2)?1:0);
    my = ((side==1 || side==3)?1:0);
    ox = ((side==1)?nx-1:0);
    oy = ((side==2)?ny-1:0);

    posmax = nx*mx+ny*my;
    
    
    /*** second loop : angles ***/
    for (itheta = 0; itheta<ntheta; itheta++) {
      printf(".");
      fflush(stdout);
      theta = theta0 + (float)(itheta)*dtheta;
      dx = (float)cos((double)theta);
      dy = (float)sin((double)theta);
      
      /*** third loop : start positions ***/
      for (pos=0;pos<posmax;pos++) {

	/* clear segment array */
	iseg = 0;

	/*** fourth loop : phase for two-spaced pixels ***/
        for (lphase=0;lphase<2;lphase++) {
	  
	  /*** detect aligned points by blocs ***/
	  inbloc = nblocs = cur = l = count[0] = 0;
	  xx = ox+pos*mx + (int)(dx*(float)(l*2+lphase));
	  yy = oy+pos*my + (int)(dy*(float)(l*2+lphase));

	  for (;xx>=0 && xx<nx && yy>=0 && yy<ny;) {

	    error = angle->gray[yy*nx+xx];
	    if (error>-100.0) {
	      error -= theta;
	      while (error<=-M_PI) error += 2.0*M_PI;
	      while (error>M_PI) error -= 2.0*M_PI;
	      if (error<0.0) error = -error;
	      if (error<prec) {
		cur++;
		if (!inbloc) {
		  startbloc[nblocs]=l;
		  inbloc=1;
		}
	      } else {
		if (inbloc) {
		  endbloc[nblocs] = l-1;
		  nblocs++;
		  count[nblocs] = cur;
		}
		inbloc=0;
	      }
	    }
	    /* compute next point */
	    l++;
	    xx = ox+pos*mx + (int)(dx*(float)(l*2+lphase));
	    yy = oy+pos*my + (int)(dy*(float)(l*2+lphase));
	  }
	  
	  /*** detect meaningful segments ***/
	  for (i=0;i<nblocs;i++) 
	    for (j=i;j<nblocs;j++) 
	      if ((nfa = test[count[j+1]-count[i]
			     +(n+1)*(1+endbloc[j]-startbloc[i])]) < max_nfa) {
		seg[iseg].start = startbloc[i]*2+lphase;
		seg[iseg].end = endbloc[j]*2+lphase;
		seg[iseg].nfa = nfa;
		seg[iseg].ok = 1;
		iseg++;
		/* reallocate if necessary */
		if (iseg==size_seg) {
		  size_seg = (size_seg*3)/2;
		  seg = (struct one_segment *)
		    realloc(seg,size_seg*sizeof(struct one_segment));
		  if (!seg) 
		    mwerror(FATAL,1,"Not enough memory.");
		}
	      }
	}
	/*** end of phase loop ***/

	/*** remove non-maximal segments ***/
	if (!all) 
	  for (i=0;i<iseg;i++) 
	    for (j=0;j<iseg;j++)
	      if (i!=j) 
		
		/* seg[i] is included in seg[j] ? */
		if (seg[i].start>=seg[j].start && seg[i].end<=seg[j].end) {
		  
		  /* remove the less meaningful of seg[i] and seg[j] */
		  if (seg[i].nfa<seg[j].nfa) seg[j].ok=0;	
		  else seg[i].ok=0;
		  
		}
	
	/*** store detected segments ***/
	for (i=0;i<iseg;i++) 
	  if (seg[i].ok) {
	    seglist[iseglist*5  ]=(float)(ox+pos*mx)+dx*(float)(seg[i].start);
	    seglist[iseglist*5+1]=(float)(oy+pos*my)+dy*(float)(seg[i].start);
	    seglist[iseglist*5+2]=(float)(ox+pos*mx)+dx*(float)(seg[i].end);
	    seglist[iseglist*5+3]=(float)(oy+pos*my)+dy*(float)(seg[i].end);
	    seglist[iseglist*5+4]=-(float)log10(seg[i].nfa);
	    iseglist++; 
	    /* reallocate seglist if necessary */
	    if (iseglist==size_seglist) {
	      size_seglist = (size_seglist*3)/2;
	      seglist = (float *)
		realloc(seglist,5*size_seglist*sizeof(float));
	      if (!seglist) 
		mwerror(FATAL,1,"Not enough memory.");
	    }
	  }
      }
    }
    /*** end of second loop ***/
    
    printf("\n");
  }
  /******************** end of first loop ********************/
    
  /*** free memory ***/
  free(seg);
  free(endbloc);
  free(startbloc);
  free(count);
  free(test);

  /* build segments list */
  seglist = (float *)realloc(seglist,5*iseglist*sizeof(float));
  result = mw_new_flist();
  result->size = result->max_size = iseglist;
  result->dim = 5;
  result->values = seglist;

  /* build curves if requested */
  if (crv) {
    crv = mw_change_flists(crv,iseglist,iseglist);
    for (i=0;i<iseglist;i++) {
      crv->list[i] = mw_change_flist(NULL,2,2,2);
      for (j=0;j<4;j++) 
	crv->list[i]->values[j] = seglist[i*5+j]+.5;
    }
  }

  return(result);
}






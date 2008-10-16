/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
   name = {ll_boundaries2};
   version = {"1.1"};
   author = {"Lionel Moisan, Frederic Cao"};
   function = {"Extract (local or not) meaningful boundaries from a Fimage"};
   usage = {
 'e':[eps=0.]->eps   "-log10(max number of false alarms)",
 's':[step=1.]->step "quantization step",
 'p':[prec=2]->prec  "sampling precision for flst_bilinear",
 'G':[G=0.5]->std    "standard dev. for preliminary convolution",
 'a'->all            "keep all meaningful level lines (not only maximal ones)",
 'H':[hstep=0.01]->hstep   "step for contrast histogram",
 't':tree->tree            "use a precomputed bilinear tree, default NULL", 
 'v':[visit=100]->visit    "maximal number of visits for a boundary",
 'L'->loc                  "force local research",
 'k':keep_tree<-keep_tree  "keep meaningful tree",
 'o':image_out<-image_out  "image reconstructed by meaningful tree",
 in->in                    "input (Fimage)",
 out<-ll_boundaries2       "output boundaries (Flists)"
}
;
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fderiv(), flst(), flst_bilinear(),
			 * flst_reconstruct(), flstb_quantize(),
			 * flstb_boundary(), flst_pixels(),
			 * fsaddles(), sintegral(), fsepconvol() */

#define ABS(x) ((x)>0?(x):-(x))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MIN_HIST_AREA 10000

typedef struct mydata{
  int visit;
  int ndetect; 
  char type;
  float nfa; /* number of false alarms */
  float min_contrast; 
  float p_contrast;
  float length;
  Fsignal histo; /* histogram of gradient inside the shape */
  char meaningful; 
  char max; /* maximal meaningful boundary */
  char notmax; /* meaningful boundary known to be NOT maximal */
} *Mydata;

/* A type containing fixed values. This prevents from global variables 
   and huge function declaration.   */


struct myglobal{
  Shapes Tree;
  float **tabsaddles,threshold;
  Fimage image;
  int *prec;
  char *local,*all;
};



/* store meaningful boundaries */
void store_boundaries(cs,tree,image,prec,tabsaddles,visit,all,
		      NormofDu,eps2)
Flists cs;
Shapes tree;
int *prec,visit;
float **tabsaddles,eps2;
Fimage image,NormofDu;
char *all;
{
  Shape s;
  Flist l,lhisto;
  int i,j;
  Mydata sdata; 
  float proba,probastep,lognbtests,proba2;
  

  lognbtests = (float)log10((double)tree->nb_shapes);
  proba2 = (float) pow(10.,(double)eps2);
  l = mw_new_flist();
  if(!l) mwerror(FATAL,1,"Not enough memory.\n");
  for(i=1;i<tree->nb_shapes;i++){
    s = tree->the_shapes+i;
    sdata = (Mydata) s->data;
    if(sdata->max || (all && sdata->meaningful)){   
      s->removed = 0;
      mwdebug("#%d shape #%d c=%.2f pc=%.4f l=%.2f PFA=%.3f\n",
	      cs->size,i,sdata->min_contrast,sdata->p_contrast,
	      sdata->length,sdata->nfa);  
      flstb_boundary(prec,image,tree,s,NULL,l,tabsaddles);
      cs->list[cs->size] = mw_copy_flist(l,NULL);
      
      /* store nfa as a double in the data field */
      cs->list[cs->size]->data_size = sizeof(double);
      cs->list[cs->size]->data = malloc(sizeof(double));
      *((double*)(cs->list[cs->size]->data)) = 
	(double)(sdata->nfa+lognbtests);
      cs->size++;
    }
    else s->removed = 1;
  }
  mw_delete_flist(l);
  
  /* store the initial number of curve in the data field of the Flists*/
  cs->data_size = sizeof(double);
  cs->data = malloc(sizeof(double));
  *((double*)cs->data) = (double) tree->nb_shapes;
}


/*===== Compute the minimum contrast and the length of the curve l =====*/

float min_contrast(l,length,NormofDu)
Flist l;
float *length;
Fimage NormofDu;
{
  double per;
  float mu,minmu,x,y,ox,oy;
  int i,ix,iy,minx,miny;

  per = 0.;
  minmu = FLT_MAX;

  for (i=0;i<l->size;i++) {

    x = l->values[i*2];
    y = l->values[i*2+1];

    if (i>0) per += sqrt((double)(x-ox)*(x-ox)+(y-oy)*(y-oy));
    ox = x; oy = y;
    
    ix = (int) floor((double) x + .5) - 1;
    iy = (int) floor((double) y + .5) - 1;
    if (ix>=0 && iy>=0 && ix<NormofDu->ncol && iy<NormofDu->nrow) {
      mu = NormofDu->gray[NormofDu->ncol*iy+ix];
      if (mu<minmu) {
	minmu= mu;
	minx = ix;
	miny = iy;
      }
    }
  }
  if (minmu == FLT_MAX) minmu = 0.;
  *length = (float)per;
  return(minmu);
}

float dist2(p,q)
float *p,*q;
{
  double x, y;
  x = *p - *q;
  y = *(p + 1) - *(q + 1);
  return (float) sqrt(x * x + y * y);
}


/* allocate memory for boundaries data */
void pixels_and_data(tree,NormofDu,image,prec,tabsaddles,sumsqper)
Shapes tree;
Fimage NormofDu,image;
float **tabsaddles,*sumsqper;
int *prec;
{
  Shape s;
  int i,k;
  Mydata data;
  Flist l;

  /* fill the field pixels of each shape*/
  mwdebug("Fill pixels field of each shape...");
  flst_pixels(tree);
  mwdebug("done.\n");
  /* compute the boundaries and allocate data field */
  mwdebug("Boundary length computation ...");
  l = mw_new_flist();
  *sumsqper = 0.;
  for(i=0;i<tree->nb_shapes;i++){
    s = tree->the_shapes+i;
    s->data = malloc(sizeof(struct mydata));
    if(!s->data) mwerror(FATAL,1,"Cannot allocate memory.\n");
    data = (Mydata) s->data;
    data->nfa = FLT_MAX;
    data->visit = 0;
    if(i>0) {
      flstb_boundary(prec,image,tree,s,NULL,l,tabsaddles);
      data->min_contrast = min_contrast(l,&data->length,NormofDu);
      *sumsqper += data->length*data->length;
    } else data->min_contrast = 0.;
    data->p_contrast = 0.;
    data->meaningful = (char) 0;
    data->notmax = (char) 0;
    data->max = (char) 0;
    data->type = (char) 0;
    data->ndetect = 0;
    data->histo = NULL;
    if(s->parent) s->removed = (char) 1;
  }
  mwdebug("done.\n");
  mw_delete_flist(l);
  fflush(stdout);
}


float image_max(image)
Fimage image;
{
  int i;
  float output;
  
  output = image->gray[0];
  for(i=0;i<image->ncol*image->nrow;i++)
    output = MAX(output,image->gray[i]);
  return(output);
}


/* compute gradient histogram  in a shape */
Fsignal shape_histo(shape,NormofDu,size,scale)
Shape shape;
Fimage NormofDu;
int size;
float scale;
{
  Point_plane p;
  int i,ncol,nrow,k,nbpt=0,ox,oy,x,y;
  float value,*tabvalues;
  
  Fsignal histo;
  
  ncol = NormofDu->ncol;
  nrow = NormofDu->nrow;
  
  histo = mw_change_fsignal(NULL,size);
  if(!histo) mwerror(FATAL,1,"Not enough memory.\n");
  mw_clear_fsignal(histo,0.);
  histo->scale = scale;
  /* compute histogram of gradient in shape */
  for(i=0,p=shape->pixels;i<shape->area;i++,p++){
    value = NormofDu->gray[p->x+ncol*p->y]/histo->scale;
    k = (int)value;
    if(k>=0) histo->values[k]+=1.;
  }
  
  /* because Du != 0 on level lines  */
  histo->values[0] = 0.;     
  return(histo);
}

/* compute log10 of repartition function from a histogram */
void update_repart(local_histo,local_repart)
Fsignal local_histo,local_repart;
{
  int nbins,i;

  nbins = local_histo->size;
  /* compute log10 of Proba(|Du|>x) */
  local_repart->values[nbins-1] = local_histo->values[nbins-1]+1;
  for(i=nbins-1;i>0;i--)
    local_repart->values[i-1] = local_repart->values[i]
      +local_histo->values[i-1];
  for(i=nbins-1;i>=0;i--) 
    local_repart->values[i] /= local_repart->values[0];
/*   affiche_repart(local_repart);  */
  for(i=0;i<nbins;i++)
    local_repart->values[i] = (float) log10(local_repart->values[i]);
}



float logH(mu,local_repart)
float mu;
Fsignal local_repart;
{
  int i;
  
  i = (int) (mu/local_repart->scale);
  if(i>= local_repart->size-1) i = local_repart->size-1;
  return(local_repart->values[i]);
}


/* compute IN PLACE the difference of two signals and take the positive part*/
void signal_pos_diff(sig1,sig2)
Fsignal sig1,sig2;
{
  int i;
  
  for(i=0;i<sig1->size;i++){
    sig1->values[i] -= sig2->values[i];
    if(sig1->values[i]<0) sig1->values[i] = 0.;
  }
}

void update_root_histo(local_root,local_histo,NormofDu)
Shape local_root;
Fsignal local_histo;
Fimage NormofDu;
{
  Shape s;
  Mydata sdata;
  int ncol,nrow,x,y,ox,oy,i,k,tmp;
  float *tabvalues,value;
  
  ncol = NormofDu->ncol;
  nrow = NormofDu->nrow;
  
  s = mw_get_first_child_shape(local_root);
  while(s){
    sdata = (Mydata) s->data;
    signal_pos_diff(local_histo,sdata->histo);
    s = mw_get_next_sibling_shape(s);
  }
  
  /*remove root boundary points from histogram (unless shape is absolut root)*/
  if(local_root->boundary){
    tabvalues = local_root->boundary->values;
    tmp = (int) floor(tabvalues[0] + .5);
    ox = MIN(ncol - 1, MAX(0, tmp - 1));
    tmp = (int) floor(tabvalues[1] + .5);
    oy = MIN(nrow - 1, MAX(0, tmp - 1));
    for(i=0;i<local_root->boundary->size;i++){
      tmp = (int) floor(tabvalues[2 * i] + .5);
      x = MIN(ncol - 1, MAX(0, tmp - 1));  
      tmp = (int) floor(tabvalues[2 * i + 1] + .5);
      y = MIN(nrow - 1, MAX(0, tmp - 1));
      if(x!= ox || y!=oy){
	ox = x; oy = y;
	value = NormofDu->gray[x+ncol*y]/local_histo->scale;
	k = (int)value;
	local_histo->values[k]-=1.;
	if(local_histo->values[k]<0.) local_histo->values[k]=0.;
      } 
    }
  }
}


/* compute nfa of a given shape boundary */
int update_mydata(s,type,local_root,local_repart,Global,new_open)
Shape s,local_root;
char type;
Fsignal local_repart;
struct myglobal Global;
int *new_open;
{ 
  Shape t;
  float mu,length,threshold;
  struct mydata *sdata, *tdata;
  int detect=0;
  
  if (!s) return 0;

  sdata = (Mydata) s->data;
  mu = sdata->min_contrast;
  threshold = Global.threshold;
  /* update nfa only if shape is not already as maximal or meaningful and not
     maximal.
     This is only useful for a local research where a curve may be 
     visited several times. 
  */
  if(!sdata->notmax && !sdata->max){     
    if (s != local_root) {
      
      /*** compute nfa (-log10) ***/
      sdata->p_contrast = logH(mu,local_repart);
      sdata->nfa = sdata->length*sdata->p_contrast/2.;
      sdata->visit++;
      
      /* update type */
      if (sdata->nfa < threshold) {
	sdata->type = s->inferior_type;
	sdata->meaningful = (char) 1;
	detect = 1;
	/* detect new open meaningful boundary; 
	   in the local research, we first deal with all open shapes before
	   detecting closed one. This is because the interior of an open shape 
	   is not clearly defined;
	   useful for local research only*/
	if(s->open) *new_open = 1;
      }
      else sdata->type = type;
    } else {
      sdata->nfa = threshold;
      sdata->type = s->inferior_type;
    }
  }
  
  /* visit children and update bestnfa_inf from children */
  sdata->ndetect = 0;
  if(s == local_root || !sdata->max)
    for (t=s->child;t;t=t->next_sibling){
      detect += update_mydata(t,sdata->type,local_root,local_repart,
			      Global,new_open);
      tdata = (struct mydata *)t->data;
      if (tdata->nfa<threshold) {
	/* useful for local research only:
	   increment the number of newly detected in s*/ 
	if (sdata->type==t->inferior_type &&
	    !tdata->notmax) sdata->ndetect++;
      } else sdata->ndetect += tdata->ndetect;
    }
  return(detect);
}

/*===== second pass : compute and select maximal meaningful boundaries =====*/
/* classical maximality,  if research is not local */
void add_boundary(s,local_root,bestnfa_inf,bestnfa_sup,type,Global)
Shape s,local_root;
float *bestnfa_inf,bestnfa_sup;
char type;
struct myglobal Global;
{ 
  Shape t;
  float mu,nfa,new_bestnfa_inf,old_bestnfa_sup,threshold;
  int length,detect;
  Mydata sdata,tdata;
  char *all;

  
  if (!s) return;
  sdata = (Mydata)s->data;

  threshold = Global.threshold;
  all = Global.all;

  if (s!=local_root) {
    if(!sdata->max){

      nfa = sdata->nfa;
 
      /* update bestnfa_sup.
	 If local research, only go down in the tree until we reach 
	 an already detected maximal boundary*/
      if (nfa<=bestnfa_sup && (!sdata->notmax && !sdata->max))
	bestnfa_sup=nfa;
      old_bestnfa_sup = bestnfa_sup;
      if (type != s->inferior_type || sdata->ndetect!=1) bestnfa_sup=threshold;
      
      /* visit children and update bestnfa_inf from children */
      new_bestnfa_inf = threshold;
      for (t=s->child;t;t=t->next_sibling) {
	tdata = (Mydata)t->data;
	*bestnfa_inf = threshold;
	add_boundary(t,local_root,bestnfa_inf,bestnfa_sup,
		       sdata->type,Global); 
	if (*bestnfa_inf<new_bestnfa_inf 
	    && sdata->ndetect==1
	    && sdata->type==tdata->type) 
	  new_bestnfa_inf=*bestnfa_inf;
      }
      *bestnfa_inf = new_bestnfa_inf;
      
      /* test if this shape has to be kept */
      if ((nfa<threshold && all) || 
	  (nfa<threshold && nfa <= old_bestnfa_sup && nfa < *bestnfa_inf)){
	if(!sdata->notmax){
	  s->removed = (char) 0;
	  sdata->max = 1;
	  s->boundary = mw_new_flist();
	  if(!s->boundary)  mwerror(FATAL,1,"Not enough memory.\n");
	  flstb_boundary(Global.prec,Global.image,Global.Tree,s,
			 NULL,s->boundary,Global.tabsaddles);
	}
      } else {
	s->removed = (char)1;
	if(sdata->meaningful) sdata->notmax = 1;
      }
      if (nfa<*bestnfa_inf && (!sdata->notmax)) *bestnfa_inf = nfa;
    } else {
      /* no new detection monotonically containing a maximal meaningful 
	 boundary. Applies for local research but no need to precise */
      if(!s->open) *bestnfa_inf = -FLT_MAX;
    }
  } else {
    for (t=s->child;t;t=t->next_sibling) 
      add_boundary(t,local_root,bestnfa_inf,bestnfa_sup,
		   s->inferior_type,Global);
  }
}


/* second pass in case of local research :
   compute maximal boundaries but do not explore closed shapes 
   if they are contained in a meaningful open one
 */

/* a total maximal boundary does not contain and is not contained in a 
   boundary which is more meaningful.
   This is stronger than maximal meaningful, since this acts independently of 
   bifurcations and contrast reversal.
   This is used to give a partition of the image taken as support of the local 
   contrast histograms*/

void total_maximal(s,local_root,bestnfa_inf,bestnfa_sup,Global,new_open)
Shape s,local_root;
float *bestnfa_inf,bestnfa_sup;
struct myglobal Global;
int new_open;
{
  Shape t;
  Mydata sdata,tdata;
  float nfa,threshold,new_bestnfa_inf;


  threshold = Global.threshold;
  if(s != local_root){
    sdata = (Mydata) s->data;
    nfa = sdata->nfa;
    /* explore tree only if s is not total maximal */
    if(!sdata->max){
      if(!sdata->notmax && nfa<bestnfa_sup) bestnfa_sup = nfa;
	
      new_bestnfa_inf = threshold;
      for(t=s->child;t;t=t->next_sibling){
	*bestnfa_inf = threshold;
	/* scan downward if current shape is open
	   or if it is closed and not contained in any 
	   new open meaningful boundary  */
	if(t->open || !new_open || !s->open || bestnfa_sup>=threshold)
	  total_maximal(t,local_root,bestnfa_inf,bestnfa_sup,Global,new_open);
	/* no restriction on bifurcation and contrast reversal for 
	   total maximal boundaries */
	if(*bestnfa_inf<new_bestnfa_inf) 
	  new_bestnfa_inf = *bestnfa_inf;
      }
      *bestnfa_inf = new_bestnfa_inf;
      
      if(nfa<threshold && nfa<=bestnfa_sup && nfa<*bestnfa_inf && 
	 !sdata->notmax && (s->open || !new_open)){
	/* s is total maximal with respect to local root 
	   and local histogram */
	s->removed = (char)0;
	sdata->max = (char)1;
	s->boundary = mw_new_flist();
	if(!s->boundary)  mwerror(FATAL,1,"Not enough memory.\n");
	flstb_boundary(Global.prec,Global.image,Global.Tree,s,
		       NULL,s->boundary,Global.tabsaddles);
      }
      
      if(!sdata->notmax && nfa<*bestnfa_inf) *bestnfa_inf = nfa;
    } 
  } else {
    for(t=s->child;t;t=t->next_sibling)
      total_maximal(t,local_root,bestnfa_inf,bestnfa_sup,Global,new_open);
  }
}

/* eliminate maximal monotone interval around total maximal boundaries in the
   tree of shapes. (only keep maximal curve) 
   These boundaries cannot be scanned in further local detection */

void clear_total_maximal(s,local_root,threshold)
Shape s,local_root;
float threshold;
{
  Shape t;
  Mydata sdata,tdata;
  
  sdata = (Mydata) s->data;

  /* scan tree upward */
  t = s->parent;
  tdata = (Mydata) t->data;
  while(t!=local_root && !tdata->notmax && tdata->ndetect==1 
	&& tdata->type == sdata->type){
    if(tdata->nfa<threshold){
      tdata->notmax = (char) 1;
      t->removed = (char) 1;
    }
    t = t->parent;
    tdata = (Mydata) t->data;
  }
  
  /* scan tree downward */
  if(sdata->ndetect ==1){
    t = s->child;
    tdata = (Mydata) t->data;
    while(t && !tdata->notmax && tdata->ndetect==1 && 
	tdata->type == sdata->type){
      while(tdata->nfa>=threshold && tdata->ndetect != 1)
	t = t->next_sibling;
      if(tdata->nfa<threshold){
	tdata->notmax = (char) 1;
	t->removed = (char) 1;
      }
      t = t->child;
      tdata = (Mydata) t->data;
    }
  }
}

void fathom_tree(local_root,NormofDu,local_histo,local_repart,Global)
Shape local_root;
Fimage NormofDu;
Fsignal local_histo,local_repart;
struct myglobal Global;
{
  int detect,sumarea,new_open;
  float nfa_inf,nfa_sup,nfils=0.;
  Shape s;
  Mydata sdata,rdata;
  float newbestrootnfa;

  if(!local_root) return;

  /* compute histogram in root\union(detected shapes)*/
  rdata = (Mydata)local_root->data;
  if(!rdata->histo)
    rdata->histo = shape_histo(local_root,NormofDu,
			       local_histo->size,local_histo->scale);
  
  /* if needed, compute histogram in shape child in order to substract it 
     to shape histogram */
  s = mw_get_first_child_shape(local_root);
  while(s){
    nfils+=1.;
    sdata = (Mydata) s->data;
    if(!sdata->histo)
      sdata->histo = shape_histo(s,NormofDu,
				 local_histo->size,local_histo->scale);
    s = mw_get_next_sibling_shape(s);
  }


  mw_copy_fsignal(rdata->histo,local_histo);
  update_root_histo(local_root,local_histo,NormofDu);
  update_repart(local_histo,local_repart);


  nfa_inf = nfa_sup = Global.threshold;
  new_open = 0;
  detect=update_mydata(local_root,rdata->type,local_root,local_repart,
		       Global,&new_open);
  
  /* two possible solutions: local research or not */
  if(Global.local){
    total_maximal(local_root,local_root,&nfa_inf,nfa_sup,Global,new_open);
    s = mw_get_first_child_shape(local_root);
    while(s){
      clear_total_maximal(s,local_root,Global.threshold);
      s = mw_get_next_sibling_shape(s);
    }
  } else 
    add_boundary(local_root,local_root,&nfa_inf,nfa_sup,0,Global);
  

  if(detect && Global.local)
    fathom_tree(local_root,NormofDu,local_histo,local_repart,Global);
  else{
    mw_delete_fsignal(rdata->histo); /* no longer need this histogram */
    s = mw_get_first_child_shape(local_root);
    while(s && Global.local){
      fathom_tree(s,NormofDu,local_histo,local_repart,Global);
      s = mw_get_next_sibling_shape(s);
    }
  }
}

Flists ll_boundaries2(in,eps,tree,step,prec,std,hstep,all,visit,loc,image_out,keep_tree)
Fimage in,image_out;
Shapes tree,keep_tree;
float *eps,*step,*hstep,*std;
int *prec,*visit;
char *all,*loc;
{
  Fimage copy_in,saddles,NormofDu;
  Shapes ref_tree;
  float **tabsaddles,offset,fzero,maxDu;
  float threshold,lognbtests,sumsqper,eps2;
  int newtree,nsize,ndetect,maxvisit,hsize,i,i_one;
  Fsignal local_histo,local_repart,child_histo;
  Shape root,cur_s;
  Flists boundaries,out;
  Mydata data;
  struct myglobal Global;
  
  /* preliminary smoothing to prevent gradient quantization 
     only a very small value is necessary (default 0.5) */
  i_one = 1;
  fsepconvol(in,in,NULL,NULL,NULL,std,&i_one);
  
  Global.local = loc;
  
  /* accept only bilinear tree */
  if(tree)
    if(tree->interpolation!=1)
      mwerror(FATAL,1,"Please use a bilinear tree.\n");
  
  /* compute FLST if needed */
  if(!tree){
    tree = mw_new_shapes();
    copy_in = mw_change_fimage(NULL,in->nrow,in->ncol);
    if (!tree || !copy_in) mwerror(FATAL,1,"Not enough memory");
    mw_copy_fimage(in,copy_in);
    flst_bilinear(NULL,copy_in,tree);
    mw_delete_fimage(copy_in);
   } 
  ref_tree = mw_new_shapes();
  if (!ref_tree) mwerror(FATAL,1,"Not enough memory");
  offset = 0.5;
  flstb_quantize(NULL,&offset,step,tree,ref_tree);
  
  
  /* compute saddle points  */
  saddles = mw_new_fimage();
  if (!saddles) mwerror(FATAL,1,"Not enough memory");
  fsaddles(in,saddles);
  tabsaddles = mw_newtab_gray_fimage(saddles);
  if (!tabsaddles) mwerror(FATAL,1,"Not enough memory");

  mwdebug("Total number of shapes: %d\n",ref_tree->nb_shapes);


  /* compute NormofDu*/
  NormofDu = mw_change_fimage(NULL,in->nrow,in->ncol);
  fzero = 0.; nsize = 3; i_one = 1; 
  fderiv(in,NULL,NULL,NULL,NULL,NULL,NULL,NormofDu,NULL,&fzero,&nsize); 


  /* compute lists of pixels and allocate data field */
  pixels_and_data(ref_tree,NormofDu,in,prec,tabsaddles,&sumsqper);

  maxDu = image_max(NormofDu);
  hsize = 1+(int) floor((double) maxDu/(*hstep));

  local_histo = mw_new_fsignal();
  local_repart = mw_new_fsignal();
  local_histo = mw_change_fsignal(local_histo,hsize);
  local_repart = mw_change_fsignal(local_repart,hsize);
  if(!local_histo || !local_repart) mwerror(FATAL,1,"Not enough memory.\n");
  local_histo->scale = local_repart->scale = *hstep;
  
  threshold = -*eps-(float)log10((double)(ref_tree->nb_shapes));
  /* multiply NFA by maximal number of visits for a shape*/
  if(Global.local) threshold -= (float) log10((double) *visit);

  mwdebug("NFA threshold: %g\n",threshold);
  ndetect = 0;
  root = ref_tree->the_shapes;
  
  
  /* The Global variable contains several variable used everywhere. 
     This is a bit ugly but prevents from using a global variable */
  Global.Tree = ref_tree;
  Global.tabsaddles = tabsaddles;
  Global.image = in;
  Global.prec = prec;
  Global.threshold = threshold;
  Global.all = all;
  /* add boundaries recursively */
  fathom_tree(ref_tree->the_shapes,NormofDu,local_histo,local_repart,Global);
  
  /*allocate and store result */
  boundaries = mw_new_flists();
  boundaries = mw_change_flists(boundaries,ref_tree->nb_shapes-1,0);
  if(!boundaries) mwerror(FATAL,1,"Not enough memory.\n");
  
  lognbtests = -threshold-(*eps);
  eps2 = -(*eps)-(float)log10((float)sumsqper);
  store_boundaries(boundaries,ref_tree,in,prec,tabsaddles,*visit,all,
		   NormofDu,eps2);

  printf("%d %s boundaries detected over %d\n",boundaries->size,(all?"":"maximal"),ref_tree->nb_shapes);
  

  if(loc){
    maxvisit = 0;
    for(i=1;i<ref_tree->nb_shapes;i++){
      cur_s = ref_tree->the_shapes+i;
      maxvisit = MAX(maxvisit,((Mydata)cur_s->data)->visit);
    }
    printf("The most visited shape has been visited %d times\n",maxvisit);
    if(maxvisit>*visit){
      printf("WARNING: some boundaries have been visited more than allowed\n");
      printf("NFAs may be underestimated\n");
    }
  }
  if(image_out)
    flst_reconstruct(ref_tree,image_out);

  /* free memory and exit*/
  mw_delete_fsignal(local_histo);
  mw_delete_fsignal(local_repart);
  free(tabsaddles);
  mw_delete_fimage(saddles);
  mw_delete_fimage(NormofDu);
  /* keep meaningful tree if requested */
  if(keep_tree) *keep_tree = *ref_tree;
  else mw_delete_shapes(ref_tree);
  return(boundaries);
}









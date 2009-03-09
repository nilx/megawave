/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand 
name = {ll_edges}; 
version = {"1.2"}; 
author = {"Lionel Moisan"}; 
function = {"Extract maximal meaningful edges (contrasted pieces of level lines) from a Fimage"}; 
usage = { 

 'e':[eps=0]->eps      "-log10(max. number of false alarms)",
 's':step->step        "quantization step (bilinear), default 1.",
 'p':p->precision      "sampling precision (bilinear), default 2",
 't':tree->tree        "use a precomputed FLST tree",
 'z'->z                "use zero order instead of bilinear interpolation",
 in->in                "input Fimage",
 out<-ll_edges         "output edges (Flists)"

}; 
*/ 
/*----------------------------------------------------------------------
 v1.1: upgraded fhisto() call (L.Moisan)
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "mw.h" 
#include "mw-modules.h" /* for fderiv(), flst(), flst_bilinear(),
			 * flstb_quantize(), flst_boundary(),
			 * flstb_boundary(), fsaddles(), 
			 * sintegral(), fhisto() */

#define HISTO_STEP 0.01

/*----- global variables -----*/
Fimage   NormOfDu;
int      precision1,zero_order;
Fsignal  logProbaOfDu;
Flists   edges;
float    *contrast;
double   *length;
int      n_alloc;


/*===== Fill contrast[] and length[] arrays  =====*/
/* return the index of the point with minimal contrast 
   (first point where the curve will be cut) */

static int contrast_and_length(Flist c, char open)
{
  double per;
  float x,y,ox,oy,min;
  int i0,i,j,n, ix, iy;

  n = c->size;

  if (n>n_alloc) {
    contrast = (float *)realloc(contrast,n*sizeof(float));
    if (!contrast) mwerror(FATAL,1,"Not enough memory\n");
    if (!zero_order) {
      length = (double *)realloc(length,n*sizeof(double));
      if (!length) mwerror(FATAL,1,"Not enough memory\n");
    }
  }

  /* contrast[] */
  for (i=0;i<n;i++) {
    ix = floor(c->values[i * 2] + .5) - 1;
    iy = floor(c->values[i * 2 + 1] + .5) - 1;
    if (ix>=0 && iy>=0 && ix<NormOfDu->ncol && iy<NormOfDu->nrow) 
      contrast[i] = NormOfDu->gray[NormOfDu->ncol*iy+ix];
    else contrast[i] = 0.;
    if (i==0 || contrast[i]<min) {
      i0 = i;
      min = contrast[i];
    }
  }

  if (open) i0=0;

  /* length[] */
  if (!zero_order) {
    length[i0] = 0.;
    per = 0.;
    for (i=0;i<n;i++) {
      j = (i0+i)%n;
      x = c->values[j*2];
      y = c->values[j*2+1];
      if (i>0) {
	ox -= x; oy -= y;
	per += sqrt((double)(ox*ox+oy*oy));
	length[j] = (float)per;
      }
      ox = x; oy = y;
    }
  }

  return(i0);
}


/*===== compute NFA term associated to contrast mu =====*/

static float logH(float mu)
{
  int i;

  i = (int)(mu/logProbaOfDu->scale);
  if (i>=logProbaOfDu->size) i=logProbaOfDu->size-1;
  return(logProbaOfDu->values[i]);
}


/*===== detect maximal meaningful edges recursively =====*/
/* since c can be closed, i and j are defined modulo c->size */

static float add_edge(Flist c, float bestnfa, int i, int j)
{
  Flist  l;
  int    k,mink,n;
  float  min,lg,nfa,nfa0,nfa1,nfa2,*p;

  n = c->size;
  if (j<=i) return(bestnfa+1.);
  for (k=i;k<=j;k++) 
    if (k==i || contrast[k%n]<min) {
      min = contrast[k%n];
      mink = k;
    }
  if (zero_order) lg = (float)(j-i);
  else lg = (float)(length[j%n]-length[i%n]);
  nfa = (1.+lg/(zero_order?3.:2.))*logH(min);
  if (nfa<bestnfa) nfa0 = nfa; else nfa0 = bestnfa;

  nfa1 = add_edge(c,nfa0,i,mink-1);
  nfa2 = add_edge(c,nfa0,mink+1,j);

  if (nfa<nfa1 && nfa<nfa2 && nfa<=bestnfa) {
    /* we've got a maximal meaningful edge here */
    l = mw_change_flist(NULL,j-i+1,j-i+1,2);
    p = l->values;
    for (k=i;k<=j;k++) {
      *(p++) = c->values[(k%n)*2];
      *(p++) = c->values[(k%n)*2+1];
    }
    edges->list[edges->size++] = l;
    if (edges->size == edges->max_size) {
      mw_enlarge_flists(edges);
      if (!edges->list) mwerror(FATAL,1,"Not enough memory\n");
    }
  }
  if (nfa1<nfa) nfa = nfa1;
  if (nfa2<nfa) nfa = nfa2;

  return(nfa);
}    


/*------------------------------ MAIN MODULE ------------------------------*/

Flists ll_edges(Fimage in, Shapes tree, float *eps, float *step, int *precision, char *z)
{
  Shapes   ref_tree;
  Shape    s;
  Flist    boundary;
  Fimage   saddles,copy_in;
  float    threshold,offset,histo_step,fzero;
  float    **tabsaddles;
  int      newtree=0,nsize,i,i0;

  /* check consistency */
  zero_order = (z != NULL);
  if (tree) {
    if (zero_order && tree->interpolation!=0)
      mwerror(FATAL,1,"Please use a zero order tree with -z option\n");
    if (!zero_order && tree->interpolation!=1)
      mwerror(FATAL,1,"Please use a bilinear tree without -z option\n");
  }
  if (zero_order && step)
    mwerror(WARNING,0,"-s option have no effect with -z option\n");
  

  /* compute bilinear FLST if needed */
  if (!tree) {
    newtree = 1;
    tree = mw_new_shapes();
    copy_in = mw_change_fimage(NULL,in->nrow,in->ncol);
    if (!tree || !copy_in) mwerror(FATAL,1,"Not enough memory");
    mw_copy_fimage(in,copy_in);
    if (zero_order) 
      flst(NULL,copy_in,tree);
    else
      flst_bilinear(NULL,copy_in,tree);
    mw_delete_fimage(copy_in);
  } 

  if (!zero_order) {
    /* bilinear case : compute saddle points and quantize FLST tree */
    ref_tree = mw_new_shapes();
    saddles = mw_new_fimage();
    if (!ref_tree || !saddles) mwerror(FATAL,1,"Not enough memory");
    fsaddles(in,saddles);
    tabsaddles = mw_newtab_gray_fimage(saddles);
    if (!tabsaddles) mwerror(FATAL,1,"Not enough memory");
    offset = 0.5;
    flstb_quantize(NULL,&offset,step,tree,ref_tree);
    if (newtree) mw_delete_shapes(tree);
  } else ref_tree = tree;

  mwdebug("Total number of shapes: %d\n",ref_tree->nb_shapes);

  /* initialization and memory allocation */
  precision1 = (precision?*precision:2);
  edges = mw_change_flists(NULL,1,0);
  if (!edges) mwerror(FATAL,1,"Not enough memory");
  boundary = mw_change_flist(NULL,2,0,2);
  n_alloc = 1000;
  contrast = (float *)malloc(n_alloc*sizeof(float));
  if (!zero_order) length = (double *)malloc(n_alloc*sizeof(double));

  /* compute NormOfDu */
  NormOfDu = mw_new_fimage();
  fzero = 0.; nsize = 3;
  fderiv(in,NULL,NULL,NULL,NULL,NULL,NULL,NormOfDu,NULL,&fzero,&nsize);

  /* compute logProbaOfDu */
  histo_step = HISTO_STEP;
  logProbaOfDu = fhisto(NormOfDu,NULL,&fzero,NULL,NULL,&histo_step,NULL,NULL);
  logProbaOfDu->values[0]=0.; /* because Du!=0 on level lines */
  sintegral(logProbaOfDu, (char *) 1, (char *) 1);
  for (i=0;i<logProbaOfDu->size;i++)
    logProbaOfDu->values[i] = (float)log10((double)logProbaOfDu->values[i]);

  /*----- MAIN LOOP -----*/
  threshold = -*eps-2.*(float)log10((double)(ref_tree->nb_shapes));
  for (i=0;i<ref_tree->nb_shapes;i++) {
    s = &ref_tree->the_shapes[i];
    if (s->parent) {

      /* extract boundary */
      if (zero_order) 
	flst_boundary(ref_tree,s,boundary);
      else
	/* FIXME: cast */
	flstb_boundary(&precision1, in, ref_tree, s, NULL,
		       boundary, (char *) tabsaddles);

      /* compute contrast and length */
      i0 = contrast_and_length(boundary,s->open);

      /* find maximal edges recursively */
      add_edge(boundary,threshold,i0,i0+boundary->size-1);
    }
  }
  printf("%d edges detected\n",edges->size);

  /* free memory and exit */
  mw_delete_flist(boundary);
  free(contrast);
  mw_delete_fimage(NormOfDu);
  mw_delete_fsignal(logProbaOfDu);
  if (!zero_order) {
    free(length);
    free(tabsaddles);
    mw_delete_fimage(saddles);
    mw_delete_shapes(ref_tree);
  }

  return(edges);
} 



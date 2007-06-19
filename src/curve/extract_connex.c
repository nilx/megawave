/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
   name = {extract_connex};
   version = {"1.5"};
   author = {"Thierry Cohignac, Lionel Moisan"};
   function = {"Extract connected components"};
   usage = {            
            'g':[g=128]->g      "thresholding grey level",
            in->in              "input cimage",
            out<-curves         "output connected components (fcurves)"
   };
*/
/*----------------------------------------------------------------------
 v1.5 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

/* maximum number of connected components */
#define MAXSHAPES 10000


#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* add a point to a fcurve */

void add_fpoint(c,x,y)
Fcurve c;
float  x,y;
{
  Point_fcurve p;

  p = mw_new_point_fcurve();
  if (!p) mwerror(FATAL,1,"not enough memory \n");
  p->x = x;
  p->y = y;

  p->previous = NULL;
  p->next = c->first;
  if (c->first) c->first->previous = p;
  c->first = p;
}


/* put together two shapes
*/
void MemeForme(tab,fold,fnew,start,lg)
long *tab;
long fold;
long fnew;
long start;
long lg;

{
  long *end;

  end = tab+lg;
  tab += start;
  
  while (tab<end) {
    if (*tab==fold) *tab=fnew;
    else if (*tab>fold && *tab!=MAXSHAPES) (*tab)--;
    tab++;
  }
}


void DecaleStart(start,ou,nbforme)
long *start;
int  ou;
int  nbforme;

{
  start += ou;
  /* Does not exist on Sun 4.x 
     memmove(start+1,start,nbforme-ou);
  */
  bcopy(start,start+1,nbforme-ou);
}

/* Allocate the array that associates the points to the shapes */
unsigned long *AllocTab(size)
unsigned long size;
{
  unsigned long *e,*p,*tab;
  
  p=tab=(unsigned long *) malloc(sizeof(unsigned long)*size);
  if (tab==NULL){
    mwerror(FATAL,1,"not enough memory\n");
    
  }
  e=p+size;
  while (p<e) *p++=MAXSHAPES;

  return tab;
}



Fcurves extract_connex(in,curves,g)
Cimage  in;
Fcurves curves;
int     *g;
{
  unsigned long  *tab,*p;
  unsigned char  *bits,grey;
  int            i,j,w,h,nbforme;
  unsigned long  f,fg,fsg,fs,fsd,fmin1,fmin2;
  Fcurve         liste[MAXSHAPES],*next,prev;
  long           *start;
  
  w = in->ncol;
  h = in->nrow;
  nbforme = 0;
  
  grey = (char)*g;

  /* PHASE 1
     
     build an array (tab) which has one cell per pixel of the input image
     after this stage :
     - the number of shapes (8-connected components) is known
     - each shape has a number
     - tab identifies the corresponfing shape for each pixel
     
     */
  
  tab = AllocTab(w*h);
  
  start = (long *)malloc(sizeof(long)*65535);
  
  if (start==NULL)
    mwerror(FATAL,1,"not enough memory\n");
  
  p = tab;
  for (i=0;i<h;i++) 
    for (j=0;j<w;j++) {
      
      if (in->gray[i*w+j] <= grey){
	
	if (i==0) {
	  fsg=fs=fsd=MAXSHAPES;
	  if (j==0) fg=MAXSHAPES;
	  else fg=*(p-1);
	} else {
	  fs=*(p-w);
	  if (j < w-1) fsd=*(p-w+1);
	  else fsd=MAXSHAPES;
	  if (j==0) fg=fsg=MAXSHAPES;
	  else {
	    fg=*(p-1);
	    fsg=*(p-w-1);
	  }
	}
	fmin1 = MIN(fs,fsd);
	fmin2 = MIN(fsg,fg);
	f = MIN(fmin1,fmin2);
	
	if (f==MAXSHAPES) {
	  f = nbforme++;
	  *(start+f)=(long)(i*w+j);
	} else {
	  if (fsg!=MAXSHAPES && fsg!=f) {
	    MemeForme(tab,fsg,f,*(start+fsg),i*w+j);
	    DecaleStart(start,fsg,nbforme);
	    nbforme--;
	  }
	  if (fs!=MAXSHAPES && fs!=f && fs!=fsg) {
	    MemeForme(tab,fs,f,*(start+fs),i*w+j);
	    DecaleStart(start,fs,nbforme);
	    nbforme--;
	  }
	  if (fsd!=MAXSHAPES && fsd!=f && fsd!=fsg & fsd!=fs) {
	    MemeForme(tab,fsd,f,*(start+fsd),i*w+j);
	    DecaleStart(start,fsd,nbforme);
	    nbforme--;
	  }
	  if (fg!=MAXSHAPES && fg!=f && fg!=fsg && fg!=fs && fg!=fsd) {
	    MemeForme(tab,fg,f,*(start+fg),i*w+j);
	    DecaleStart(start,fg,nbforme);
	    nbforme--;
	  }
	}
	*p=f;
	
      }
      
      p++;
      
      /* next point */
      
      bits++;
      
    }
    
  mwdebug("%d connected components found (max = %d)\n",nbforme,MAXSHAPES);

  /*-----------------------------------------------*/
  
  /* PHASE 2
     build curves
     */
  
  curves = mw_change_fcurves(curves);
  next = &(curves->first);
  prev = NULL;
  
  for(i=0;i<nbforme;i++) {
    liste[i] = mw_new_fcurve();
    liste[i]->first = NULL;
    liste[i]->previous = prev;
    *next = prev = liste[i];
    next = &(liste[i]->next);
  }
  
  p = tab;
  for (i=0;i<h;i++) {
    for(j=0;j<w;j++) {
      f = *p++;
      if (f!=MAXSHAPES){
	add_fpoint(liste[f],(float)j,(float)i);
      }
    }
  }
  
  free(tab);
  free(start);
  
  return curves;
}

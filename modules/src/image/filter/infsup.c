/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
 name = {infsup};
 version = {"2.3"};
 author = {"Frederic Guichard, Denis Pasquignon, Jacques Froment"};
 function = {"InfSup scheme or median-median filtering"};
 usage = {
  'n':[Niter=2]->Niter[0,1000]   "number of iterations",
  'i':[deginf=0.0]->deginf[0,1]  "second level med (0..1, default: sup=0.)",
  's':[degsup=1.0]->degsup[0,1]  "first level med (0..1, default: inf=1.)",
  'a'->average     "swap first and second level med and take the average",
  input-> image    "input cimage",
  fmovie -> fmovie "masks sequence",
  output<- output  "filtered image"};
*/
/*----------------------------------------------------------------------
 v2.1: return void (L.Moisan)
 v2.2: version syntax fixed (JF)
 v2.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"


static int diff(const void *a, const void *b)
{
  if (*(const unsigned char *)a < *(const unsigned char *)b) {
    return (-1);
  }
  else {
    return (1);
  }
}

static unsigned char TheInf(Cimage image, Fimage maskn, int x, int y)
{
  int xd,yd,xf,yf,ntx,nty,Mdx,Mdy;
  int dx,dy;      /* taille de l'image */
  register int i,j;
  int aa,mm;      /* intermediaires de calcul */
  unsigned char value;

  value=255;
  dx= image->ncol;
  dy= image->nrow;

  Mdx = maskn->ncol;
  Mdy = maskn->nrow;
  ntx= ((Mdx-1)/2);
  nty= ((Mdy-1)/2);

  xd=x-ntx;
  yd=y-ntx;
  xf=x+nty;
  yf=y+nty;
  if (xd<0) xd=0; else if (xf>=dx) xf=dx-1;
  if (yd<0) yd=0; else if (yf>=dy) yf=dy-1;
  aa= yd*dx;
  mm = -x+ntx+Mdx*(yd-y+nty);
  for(j=yd;j<=yf;j++) {
    for(i=xd;i<=xf;i++) {
      if (maskn->gray[mm+i]>0){ 
	if (value> (image-> gray[aa+i]))
	  value=image-> gray[aa+i];}
    }
    aa+=dx;
    mm+=Mdx;
  }
  
  return(value);
}

static unsigned char TheSup(Cimage image, Fimage maskn, int x, int y)
{
  int xd,yd,xf,yf,ntx,nty,Mdx,Mdy;
  int dx,dy;      /* taille de l'image */
  register int i,j;
  int aa,mm;      /* intermediaires de calcul */
  unsigned char value;

  value=0;
  dx= image->ncol;
  dy= image->nrow;

  Mdx = maskn->ncol;
  Mdy = maskn->nrow;
  ntx= ((Mdx-1)/2);
  nty= ((Mdy-1)/2);

  xd=x-ntx;
  yd=y-ntx;
  xf=x+nty;
  yf=y+nty;
  if (xd<0) xd=0; else if (xf>=dx) xf=dx-1;
  if (yd<0) yd=0; else if (yf>=dy) yf=dy-1;
  aa= yd*dx;
  mm = -x+ntx+Mdx*(yd-y+nty);
  for(j=yd;j<=yf;j++) {
    for(i=xd;i<=xf;i++) {
      if (maskn->gray[mm+i]>0) { 
	if (value< (image-> gray[aa+i]))
	  value=image-> gray[aa+i];}
    }
    aa+=dx;
    mm+=Mdx;
  }
  
  return(value);
}



static float  Histogram(Cimage image, Fimage maskn, int x, int y, float *Hist)
{
  int xd,yd,xf,yf,ntx,nty,Mdx,Mdy;
  int dx,dy;      /* taille de l'image */
  register int i,j;
  int aa,mm;      /* intermediaires de calcul */
  int l;
  float sum;      /* sum of histogram  */

  dx= image->ncol;
  dy= image->nrow;

  Mdx = maskn->ncol;
  Mdy = maskn->nrow;
  ntx= ((Mdx-1)/2);
  nty= ((Mdy-1)/2);

  xd=x-ntx;
  yd=y-ntx;
  xf=x+nty;
  yf=y+nty;
  if (xd<0) xd=0; else if (xf>=dx) xf=dx-1;
  if (yd<0) yd=0; else if (yf>=dy) yf=dy-1;
  
  /* --- Histogramme est remis a zero */
  for(i=0;i<256;i++) Hist[i]=0.0;
  
  /* --- Puis on le calcule   */
  
  sum=0;
  aa= yd*dx;
  mm = -x+ntx+Mdx*(yd-y+nty);
  for(j=yd;j<=yf;j++) {
    for(i=xd;i<=xf;i++) {
      l=image-> gray[aa+i];
      Hist[l] += maskn->gray[mm+i];
      sum += maskn->gray[mm+i];
    }
    aa+=dx;
    mm+=Mdx;
  }
  
  return(sum);
}



/* Procedure InfSup  */

/* t1, t2 are the level of the median */
/* t1=1, t2=0, process a InfSup... */

static void ResolutionInfSup(Cimage image, Cimage A, unsigned char *G, float t1, float t2, Fmovie movie, int Nmask)
                  
                            /* Image Buffer */
                            /* contains the values for each mask */     
                  
                            /* Number of masks   */
                 
{
  Fimage maskn;
  int i,j,l;
  register int k=0;
  int dx,dy;                /* size of the image  */
  int n,Num;
  int Mdx,Mdy;              /* size of a mask     */

  float Hist[257];          /* Histogramm         */
  float v, vmax;
  float presquezero;

  presquezero=0.00001;

  /* Init                                         */

  dx=image->ncol;
  dy=image->nrow;
  maskn=movie->first;
  Mdx=maskn->ncol;
  Mdy=maskn->nrow;
  
  if ((A == NULL) || (A->gray == NULL))
    mwerror(INTERNAL,1,"[ResolutionInfSup] Null buffer image !\n");

  if (G == NULL)
    mwerror(INTERNAL,1,"[ResolutionInfSup] Null G buffer !\n");

  Num= (int) ( (float) Nmask-1) * t2;

  /* For each point of the image : */

  for(j=0;j<dy;j++) {
    if ((j/10)*10==j) mwdebug("\t[%3.0f %%]\n",100.0 * (float) j/dy);
    for(i=0;i<dx;i++){

      
      maskn=movie->first;
      for(n=0;n<Nmask;n++) {                 /* For each mask */
      
	
	if ((t1>0.02)&&(t1<0.98))  /* -> median of level t1 */ 
	  {
	    vmax=Histogram( image,maskn,i,j,Hist);  /* Histogramm */
	    /* FIXME : array subscript is above array bounds */
	    /* Hist[257]=100000.0; */
	    fprintf(stderr, "there is a bug in the code");
	    exit(EXIT_FAILURE);
	    vmax=vmax*t1;
	    v=presquezero; k=0; l=0;
	    while(Hist[k]<presquezero) k++; 
	    if (v<=vmax) {
	      while(v<=vmax) {   
		if (Hist[k]>presquezero) {
		  v+=Hist[k];
		}
		k++;
	      }
	      k--;
	      if (k==256) { k=255; while(Hist[k]<presquezero) k--;}
	    }
	  }
      
     

	/* if t1<0.02 -> inf */

	if (t1<=0.02) {
	  k=TheInf(image,maskn,i,j);
	}
	
	/* if t1>0.98 -> sup */
	
	if (t1>=0.98)
	  {
	    k=TheSup(image,maskn,i,j);
	  }
	
        /* we put the median into G */
	
	G[n]=k;
	
	maskn= maskn->next;
      
      }

      /* we estimate the median of level t2, on the */
      /* values of G                                */
      

      /* if (t2>0.02)&&(t2<0.98) -> median */

      if ((t2>0.02)&&(t2<0.98)) {
	qsort((void *) G, Nmask, sizeof(unsigned char), diff);
	k=G[Num];
      }
      else
	{
	  /* if t2<0.02 -> inf  */

	  if (t2<=0.02)
	    {
	      k=255;
	      for(l=0;l<Nmask;l++)
		if (k>G[l]) k=G[l];
	    }

	  /* if t2>0.98 -> sup */

	  if (t2>=0.98)
	    {
	      k=0;
	      for(l=0;l<Nmask;l++)
		if (k<G[l]) k=G[l];
	    }
	}
      /* We copy the result into the image A */

      A->gray[i+j*dx]=k;
    }
  }
  
  /* Finish */

  /* we copy the result in the original image */

  mw_copy_cimage(A,image);
}

/* ----------------------------------------------------------*/

/* The main procedure */

/* ----------------------------------------------------------*/


Cimage infsup(int *Niter, float *deginf, float *degsup, char *average, Cimage image, Fmovie fmovie, Cimage output)
{
  int n,j,size,Nmask;
  Fimage i;
  float t1,t2;
  unsigned char *G=NULL;
  Cimage image_work=NULL;
  Cimage image_save=NULL;
  unsigned char *ptr1,*ptr2;

  /*------------------------  Number of images in the fmovie...  ------*/
  
  if ((fmovie -> first)==NULL) mwerror(FATAL,1,"empty masks movie !\n");
  Nmask=1;
  i = fmovie->first;
  while (i->next != NULL) { i=i->next; Nmask++; }
  mwdebug("Number of masks : %i \n",Nmask);

  /* ------------------------ Memory allocation for the buffers -----------*/

  G= (unsigned char *) calloc(Nmask+2 , sizeof(unsigned char));
  if (G == NULL) mwerror(FATAL, 1, "Not enough memory\n");

  image_work = mw_change_cimage(NULL,image->nrow,image->ncol);
  if (image_work == NULL) mwerror(FATAL, 1, "Not enough memory\n");  

  if ((output=mw_change_cimage(output,image->nrow,image->ncol)) == NULL)
    mwerror(FATAL, 1, "Not enough memory\n");  

  if (average)
    {
      image_save = mw_change_cimage(NULL,image->nrow,image->ncol);
      if (image_save == NULL) mwerror(FATAL, 1, "Not enough memory\n");
      mw_copy_cimage(image,image_save);
    }

  /* ----------------------- Main loop  ----------------------------------- */
  
  t1=(*degsup);
  t2=(*deginf);
  for(n=0;n<(*Niter);n++) 
    {
      mwdebug("iteration : %i / %i \n", n+1,(*Niter));     
      mwdebug("Running InfSup...\n");
      ResolutionInfSup(image,image_work,G,t1,t2,fmovie,Nmask);

      if (average)
	{
	  mwdebug("Running SupInf...\n");
	  ResolutionInfSup(image_save,image_work,G,t2,t1,fmovie,Nmask);	  
	  size = image->ncol*image->nrow;
	  mwdebug("Averaging...\n");
	  for (ptr1=image->gray, ptr2=image_save->gray, j=0; 
	       j<size; ptr1++,ptr2++,j++)
	    *ptr1 = floor(((double) *ptr1 + *ptr2)/2.0 + .5);
	  mw_copy_cimage(image,image_save);
	}
    }

  mw_copy_cimage(image,output); 

  /* -------------- Free buffers ------------ */
  
  free(G);
  if (image_save) mw_delete_cimage(image_save);
  mw_delete_cimage(image_work);

  return(output);
}

























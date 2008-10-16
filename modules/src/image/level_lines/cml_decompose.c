/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {cml_decompose};
 version = {"1.0"};
 author = {"Jacques Froment, Georges Koepfler"};
 function = {"Compute all the cmorpho_lines of a color image"};
 usage = {
   'c':cmimage_in->cmimage_in   "original image in Cmimage structure",
   'o':[ml_opt=0]->ml_opt [0,2] "choose form of morpho_lines",
   'l':[L=0]->L                 "Minimal level lines length to be kept",
   image_in->image_in           "original color image",
   cmimage<-cml_decompose       "cmimage with all morpho_lines"
};
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mw.h"

#define POINT_OK(P,Y,X)  (((P)->x>=0)&&((P)->x<=X)&&((P)->y>=0)&&((P)->y<=Y))
#define BAD_POINT(P,Y,X) (!POINT_OK(P,Y,X))


/* Like Fsignal but for colors */
typedef struct cfsignal {
  int size;        /* Number of samples */
  Color *values;   /* The samples */
} *Cfsignal;


static int ascending_order=1;

/* ------------------------------------------------------------------
   Compare two colors : return -1 if v1<v2, 1 if v1>v2 and 0 if v1=v2
            (if ascending_order=-1 and not 1 the values are opposed)
   ------------------------------------------------------------------ */


int cmpcolor(c1,c2)
Color *c1,*c2;
{
  if (c1->model != c2->model)
    mwerror(INTERNAL,1,"[cmpcolor] Two different color models %d and %d !\n",
	    c1->model,c2->model);
  switch(c1->model)
    {
    case MODEL_HSI :
      /* I is blue */
      if (c1->blue < c2->blue) return(-ascending_order); 
      if (c1->blue > c2->blue) return(ascending_order);
      /* H is red (between 0 and 360 deg. 0 is red) */
      if (c1->red < c2->red) return(-ascending_order);
      if (c1->red > c2->red) return(ascending_order);      
      /* S is green (between 0 and 1) */
      if (c1->green < c2->green) return(-ascending_order);      
      if (c1->green > c2->green) return(ascending_order);      
      return(0);
      
    default:
      mwerror(INTERNAL,1,"[cmpcolor] No color order defined for color model %d !\n",c1->model);
    }
  return 0;
}

/* -------------------------------------------------------
   Get and sort all the colors of a cfimage (like fvalues)
   ------------------------------------------------------- */

Cfsignal cfvalues(image)

Cfimage image;

{

  unsigned long i,j,smax;
  float *r,*g,*b;
  Cfsignal levels,nmlevels;

  smax = image->ncol * image->nrow;

  /* Get all colors */
  if(!(levels = (Cfsignal) (malloc(sizeof(struct cfsignal)))))
    mwerror(FATAL, 1, "Not enough memory (for cfvalues)\n");
  levels->values = (Color *) malloc(smax*sizeof(Color));
  if (!levels->values) mwerror(FATAL, 1, "Not enough memory (for cfvalues)\n");

  for (i=0, r=image->red, g=image->green, b=image->blue; i<smax; i++,r++,g++,b++)
    {
      levels->values[i].model = image->model;
      levels->values[i].red = *r;
      levels->values[i].green = *g;      
      levels->values[i].blue = *b;            
    }

  levels->size=smax;
  /* Sort in ascending order if ascending_order=1, descending if = -1 */
  qsort((Color *)levels->values,smax,sizeof(Color),cmpcolor);
    
  /* Remove multiplicity of values */
  if(!(nmlevels = (Cfsignal) (malloc(sizeof(struct cfsignal)))))
    mwerror(FATAL, 1, "Not enough memory (for cfvalues)\n");
  nmlevels->values = (Color *) malloc(smax*sizeof(Color));
  if (!nmlevels->values) mwerror(FATAL, 1, "Not enough memory (for cfvalues)\n");

  nmlevels->values[0] = levels->values[0];
  j=0;
  for (i=1; i<levels->size; i++,j++)
    {
      nmlevels->values[j] = levels->values[i-1];
      while ((i<levels->size)&&
	     (cmpcolor(&levels->values[i-1],&levels->values[i])==0)) i++;
    }
  nmlevels->size=j;
  free(levels->values);
  free(levels);
  mwdebug("[cfvalues] smax=%d  nmlevels->size=%d\n",smax,nmlevels->size);
  return(nmlevels);
}

/* ---------------------
   Check if mimage is OK
   --------------------- */

void llcheck(mimage)

Cmimage mimage;

{  Point_curve point;
   Cmorpho_line ll;
   int NC,NL;

   NC=mimage->ncol;
   NL=mimage->nrow;
   for (ll=mimage->first_ml; ll; ll=ll->next)
     {
       if ((ll->minvalue.model != ll->maxvalue.model) ||
	   (ll->minvalue.model > 4))
	 mwerror(WARNING,0,"[llcheck] Inconsistent model number (%d for minvalue, %d for maxvalue).\n",(int)ll->minvalue.model,(int)ll->maxvalue.model);
       point=ll->first_point;
       while(point!=NULL)
         {
           if(BAD_POINT(point,NL,NC))
             {
               mwdebug("Morpho Line number %d :\n   point->x=%d (NC=%d) \t point->y=%d (NL=%d)\n",ll->num,point->x,NC,point->y,NL);
               mwerror(WARNING,0,"[llcheck] Point out of image.\n");
             }
           point=point->next;
         }
     }
}


/* -------------------------------------------------------
   cml_extract
   ------------------------------------------------------- */


/* ---------------------------------------------------------------------------
   Return 1 if the color value im(c,l) is in [minvalue, maxvalue], 0 elsewhere
   --------------------------------------------------------------------------- */

int Inside(im,minvalue,maxvalue,l,c)

Cfimage im;
Color minvalue,maxvalue;
int l,c;

{
  unsigned long p;
  float r,g,b;

  p = c + l*im->ncol;
  r=im->red[p]; g=im->green[p]; b=im->blue[p]; 
  switch(im->model)
    {
    case MODEL_HSI :
      /*
      mwdebug("[Inside] (r,g,b)=(%.5f,%.5f,%.5f)\n",r,g,b);
      mwdebug("          minvalue=(%.5f,%.5f,%.5f) maxvalue=(%.5f,%.5f,%.5f)\n",
	      minvalue.red,minvalue.green,minvalue.blue,
	      maxvalue.red,maxvalue.green,maxvalue.blue);
      */
      /* ### Check if im(c,l) <= maxvalue */
      if ((b>maxvalue.blue)||((b==maxvalue.blue)&&(r>maxvalue.red))||
	 ((b==maxvalue.blue)&&(r==maxvalue.red)&&(g>maxvalue.green))) return(0); 
      /* ### Check if im(c,l) >= minvalue */
      if ((b<minvalue.blue)||((b==minvalue.blue)&&(r<minvalue.red))||
	 ((b==minvalue.blue)&&(r==minvalue.red)&&(g<minvalue.green))) return(0); 
      return(1);
      
    default:
      mwerror(INTERNAL,1,"[Inside] No color order defined for color model %d !\n",(int)im->model);
    }
  return 0;
}


void
produce_HV(im,NL,NC,minvalue,maxvalue,H,V)
Cfimage im;
Color minvalue,maxvalue;
unsigned int NL,NC;
unsigned char **H, **V;
{
  unsigned int l,c;

  /*
  mwdebug("[produce_HV] minvalue=(%.5f,%.5f,%.5f) maxvalue=(%.5f,%.5f,%.5f)\n",
	  minvalue.red,minvalue.green,minvalue.blue,
	  maxvalue.red,maxvalue.green,maxvalue.blue);
  */
  for(l=1;l<NL;l++) for(c=1;c<NC;c++) {
    if(Inside(im,minvalue,maxvalue,l,c)) {
      if(!Inside(im,minvalue,maxvalue,l,c-1))  V[l][c-1]=1; 
      if(!Inside(im,minvalue,maxvalue,l-1,c))  H[l-1][c]=1;
    }
    else {
      if(Inside(im,minvalue,maxvalue,l,c-1)) V[l][c-1]=1; 
      if(Inside(im,minvalue,maxvalue,l-1,c)) H[l-1][c]=1;
    }
  }
  /* oops: almost forgot these */
  for(l=1;l<NL;l++) {
    if(Inside(im,minvalue,maxvalue,l,0)) 
      {if(!Inside(im,minvalue,maxvalue,l-1,0))  H[l-1][0]=1;}
    else
      {if(Inside(im,minvalue,maxvalue,l-1,0)) H[l-1][0]=1;}
  }
  for(c=1;c<NC;c++) {
    if(Inside(im,minvalue,maxvalue,0,c))
      {if(!Inside(im,minvalue,maxvalue,0,c-1))  V[0][c-1]=1;}
    else
      {if(Inside(im,minvalue,maxvalue,0,c-1)) V[0][c-1]=1;}
  }
}

unsigned long
count_X(H,V,NL,NC)
unsigned char **H,**V;
unsigned int NL,NC;
{
  unsigned int l,c,sum;
  unsigned long count_points=0L;

  for(l=0;l<NL-1;l++) for(c=0;c<NC-1;c++) {
    sum=V[l][c]+H[l][c]+V[l+1][c]+H[l][c+1];
    if(sum!=0) {
      count_points++;
      if(sum==4) count_points++;
    }
  }
  for(c=0;c<NC-1;c++) {
    if(V[0][c]==1)    count_points++;
    if(V[NL-1][c]==1) count_points++;
  }
  for(l=0;l<NL-1;l++) {
    if(H[l][0]==1)    count_points++;
    if(H[l][NC-1]==1) count_points++;
  }
  return(count_points);
}

Cmorpho_line produce_lline(minvalue,maxvalue,open)

Color minvalue,maxvalue;
unsigned char open;
{
  Cmorpho_line lline=NULL;

  if(!(lline=mw_change_cmorpho_line(lline)))
    mwerror(FATAL,1,"Not enough memory for produce_lline.");
  lline->minvalue=minvalue;
  lline->maxvalue=maxvalue;
  lline->open=open;
  return(lline);
}


void
follow_open_line(NL,NC,H,V,ll,cc,sum,lline)
unsigned int NL,NC;
unsigned char **H,**V;
int ll,cc,sum;
Cmorpho_line lline;
{
  Point_curve p0,p1;

  p0=lline->first_point;

  while(sum!=0) 
    {
      if ((p1=p0->next)==NULL)
	{
	  p1=mw_new_point_curve();
	  if (p1 == NULL) mwerror(FATAL,1,"Not enough memory.\n");
	  p1->previous=p0;
	  p0->next=p1;
	}
      p1->x=cc+1;
      p1->y=ll+1;
      p0=p1;
      if(sum==1) 
	{
	  if      (V[ll][cc]==1)     {V[ll][cc]=0;ll--;}
	  else if (H[ll][cc]==1)     {H[ll][cc]=0;cc--;}
	  else if (V[ll+1][cc]==1)   {V[ll+1][cc]=0;ll++;}
	  else  /*(H[ll][cc+1]==1)*/ {H[ll][cc+1]=0;cc++;}
	}
      else 
	{ /* sum==3 */
	  if     (V[ll][cc]==0)     {H[ll][cc+1]=0;cc++;}
	  else if(H[ll][cc]==0)     {V[ll][cc]=0;ll--;}
	  else if(V[ll+1][cc]==0)   {H[ll][cc]=0;cc--;}
	  else /*(H[ll][cc+1]==0)*/ {V[ll+1][cc]=0;ll++;}
	}
      sum=((ll<0)||(cc<0)||(ll==NL-1)||(cc==NC-1)) ?
	0:(V[ll][cc]+H[ll][cc]+V[ll+1][cc]+H[ll][cc+1]);
    } /* end while */
  if ((p1=p0->next)==NULL)
    {
      p1=mw_new_point_curve();
      if (p1 == NULL) mwerror(FATAL,1,"Not enough memory.\n");
      p1->previous=p0;
      p0->next=p1;
    }
  p1->y=(ll<0)? 0:((ll==NL-1)? NL:ll+1);
  p1->x=(cc<0)? 0:((cc==NC-1)? NC:cc+1);
}

void get_open_lines(im,NL,NC,minvalue,maxvalue,H,V,lline,L,Nll,Nllrm)

Cfimage im;
Color minvalue,maxvalue;
unsigned int NL,NC;
unsigned char **H,**V;
Cmorpho_line *lline;
int L;
int *Nll,*Nllrm;

{
  Cmorpho_line oldll,newll=NULL;
  Point_curve p;
  int sum,l,c;

  /* open lines starting in the first row */
  for(c=0;c<NC-1;c++) 
    if((V[0][c]==1)&&!Inside(im,minvalue,maxvalue,0,c)) 
      { /* such that levelset at left */
	oldll=*lline;
	/* Define a new lline but if newll corresponds to a former
	   computed lline with length < L
	*/
	if ((newll==NULL)||(newll==*lline) ) 
	  newll=produce_lline(minvalue,maxvalue,1);
	(*Nll)++;
	if ((p=newll->first_point)==NULL)
	  {
	    p=mw_new_point_curve();
	    if (p == NULL) mwerror(FATAL,1,"Not enough memory.\n");
	    newll->first_point=p;
	  }
	p->x=c+1;
	p->y=0;
	V[0][c]=0;
	sum=H[0][c]+V[1][c]+H[0][c+1];
	follow_open_line(NL,NC,H,V,0,c,sum,newll);
	if (mw_length_cmorpho_line(newll) < L) (*Nllrm)++;
	else
	  {
	    *lline=newll;
	    if(oldll!=NULL) oldll->previous=*lline;
	    newll->next=oldll;
	  }
      }

  /* open lines starting in the last row */
  for(c=0;c<NC-1;c++) 
    if((V[NL-1][c]==1)&&Inside(im,minvalue,maxvalue,NL-1,c)) 
      { /* ...levelset at left */
	oldll=*lline;
	/* Define a new lline but if newll corresponds to a former
	   computed lline with length < L
	*/
	if ((newll==NULL)||(newll==*lline) ) 
	  newll=produce_lline(minvalue,maxvalue,1);
	(*Nll)++;
	if ((p=newll->first_point)==NULL)
	  {
	    p=mw_new_point_curve();
	    if (p == NULL) mwerror(FATAL,1,"Not enough memory.\n");
	    newll->first_point=p;
	  }
	p->x=c+1;
	p->y=NL;
	V[NL-1][c]=0;
	sum=H[NL-2][c+1]+V[NL-2][c]+H[NL-2][c];
	follow_open_line(NL,NC,H,V,(int)NL-2,c,sum,newll);
	if (mw_length_cmorpho_line(newll) < L) (*Nllrm)++;
	else
	  {
	    *lline=newll;
	    if(oldll!=NULL) oldll->previous=*lline;
	    newll->next=oldll;
	  }
      }

  /* open lines starting in the first column */
  for(l=0;l<NL-1;l++)
    if((H[l][0]==1)&&Inside(im,minvalue,maxvalue,l,0)) 
      { /* ...levelset at left       */
	oldll=*lline;
	/* Define a new lline but if newll corresponds to a former
	   computed lline with length < L
	*/
	if ((newll==NULL)||(newll==*lline) ) 
	  newll=produce_lline(minvalue,maxvalue,1);
	(*Nll)++;
	if ((p=newll->first_point)==NULL)
	  {
	    p=mw_new_point_curve();
	    if (p == NULL) mwerror(FATAL,1,"Not enough memory.\n");
	    newll->first_point=p;
	  }
	p->x=0;
	p->y=l+1;
	H[l][0]=0;
	sum=V[l+1][0]+H[l][1]+V[l][0];
	follow_open_line(NL,NC,H,V,l,0,sum,newll);
	if (mw_length_cmorpho_line(newll) < L)  (*Nllrm)++;
	else
	  {
	    *lline=newll;
	    if(oldll!=NULL) oldll->previous=*lline;
	    newll->next=oldll;
	  }
      }
  
  /* open lines starting in the last column */
  for(l=0;l<NL-1;l++)
    if((H[l][NC-1]==1)&&!Inside(im,minvalue,maxvalue,l,NC-1)) 
      { /* ...levelset at left  */
	oldll=*lline;
	/* Define a new lline but if newll corresponds to a former
	   computed lline with length < L
	*/
	if ((newll==NULL)||(newll==*lline) ) 
	  newll=produce_lline(minvalue,maxvalue,1);
	(*Nll)++;
	if ((p=newll->first_point)==NULL)
	  {
	    p=mw_new_point_curve();
	    if (p == NULL) mwerror(FATAL,1,"Not enough memory.\n");
	    newll->first_point=p;
	  }
	p->x=NC;
	p->y=l+1;
	H[l][NC-1]=0;
	sum=V[l][NC-2]+H[l][NC-2]+V[l+1][NC-2];
	follow_open_line(NL,NC,H,V,l,(int)NC-2,sum,newll);
	if (mw_length_cmorpho_line(newll) < L) (*Nllrm)++;
	else
	  {
	    *lline=newll;
	    if(oldll!=NULL) oldll->previous=*lline;
	    newll->next=oldll;
	  }
    }
}

void
follow_closed_line(H,V,ll,cc,sum,lline)
unsigned char **H,**V;
int ll,cc,sum;
Cmorpho_line lline;
{
  Point_curve p0,p1;

  p0=lline->first_point;
  
  while(sum!=0) 
    {
      if ((p1=p0->next)==NULL)
	{
	  p1=mw_new_point_curve();
	  if (p1 == NULL) mwerror(FATAL,1,"Not enough memory.\n");
	  p1->previous=p0;
	  p0->next=p1;
	}
      p1->x=cc+1;
      p1->y=ll+1;
      p0=p1;
      if(sum==1) {
	if      (V[ll][cc]==1)     {V[ll][cc]=0;ll--;}
	else if (H[ll][cc]==1)     {H[ll][cc]=0;cc--;}
	else if (V[ll+1][cc]==1)   {V[ll+1][cc]=0;ll++;}
	else  /*(H[ll][cc+1]==1)*/ {H[ll][cc+1]=0;cc++;}
      }
      else { /* sum==3 */
	if     (V[ll][cc]==0)     {H[ll][cc+1]=0;cc++;}
	else if(H[ll][cc]==0)     {V[ll][cc]=0;ll--;}
	else if(V[ll+1][cc]==0)   {H[ll][cc]=0;cc--;}
	else /*(H[ll][cc+1]==0)*/ {V[ll+1][cc]=0;ll++;}
      }
      sum=V[ll][cc]+H[ll][cc]+V[ll+1][cc]+H[ll][cc+1];
  } /* end while */
}

void get_closed_lines(im,NL,NC,minvalue,maxvalue,H,V,lline,L,Nll,Nllrm)

Cfimage im;
Color minvalue,maxvalue;
unsigned int NL,NC;
unsigned char **H,**V;
Cmorpho_line *lline;
int L;
int *Nll,*Nllrm;

{
  Cmorpho_line oldll,newll=NULL;
  Point_curve p;
  int sum,l,c;

  for(l=0;l<NL-1;l++) for(c=0;c<NC-1;c++) 
    if(H[l][c+1]!=0) 
      {                    /* start a closed line */
	oldll=*lline;
	/* Define a new lline but if newll corresponds to a former
	   computed lline with length < L
	*/
	if ((newll==NULL)||(newll==*lline) ) 
	  newll=produce_lline(minvalue,maxvalue,0);
	(*Nll)++;
	if ((p=newll->first_point)==NULL)
	  {
	    p=mw_new_point_curve();
	    if (p == NULL) mwerror(FATAL,1,"Not enough memory.\n");
	    newll->first_point=p;
	  }
      p->x=c+1;
      p->y=l+1;
      if(!Inside(im,minvalue,maxvalue,l,c+1)) 
	{ /* such that the levelset is at left */
	  V[l+1][c]=0; 
	  sum=H[l+1][c]+V[l+2][c]+H[l+1][c+1];
	  follow_closed_line(H,V,l+1,c,sum,newll);
	}
      else 
	{
	  H[l][c+1]=0;
	  sum=V[l+1][c+1]+H[l][c+2]+V[l][c+1];
	  follow_closed_line(H,V,l,c+1,sum,newll);
	}
      if (mw_length_cmorpho_line(newll) < L) (*Nllrm)++;
      else
	{
	  *lline=newll;
	  if(oldll!=NULL) oldll->previous=*lline;
	  newll->next=oldll;
	}
      }
}

/******************************************************************************/
/*  Possible values for ml_opt :                                              */
/*  0  --> boundary{   levels[i]   <= im[x,y] <= MORPHO_INFTY } , level lines */
/*  1  --> boundary{-MORPHO_INFTY  <= im[x,y] <= levels[i]    } , inv. l. l.  */
/*  2  --> boundary{      levels[i]   == im[x,y]              } , iso lines   */
/*  3  --> boundary{   levels[i]   <= im[x,y] <= levels[i+1]  } , general ml  */
/*  4  --> boundary{  levels[2i]   <= im[x,y] <= levels[2i+1] } , id.         */
/*                                                       where i=0,1,...      */
/******************************************************************************/

void cml_extract(levels,opt,L,image_org,m_image)
Cfsignal levels;
int *opt;
int L;
Cfimage image_org;
Cmimage m_image;
{
  Cmorpho_line current_lline=NULL;
  unsigned long nb_points, l;
  unsigned long Nll, Nllrm;
  unsigned int NL=image_org->nrow, NC=image_org->ncol;
  unsigned char **V, **H, *cptr;
  int i,ml_opt=*opt;
  Color minvalue, maxvalue;

  /* memory for horizontal boundaries */
  H=(unsigned char **)malloc((NL-1)*sizeof(unsigned char*));
  H[0]=(unsigned char *)malloc((NL-1)*NC*sizeof(unsigned char));
  for(i=1;i<NL-1;i++) H[i]=H[i-1]+NC;

  /* memory for vertical boundaries */
  V=(unsigned char **)malloc(NL*sizeof(unsigned char*));
  V[0]=(unsigned char *)malloc(NL*(NC-1)*sizeof(unsigned char));
  for(i=1;i<NL;i++) V[i]=V[i-1]+(NC-1); 

  if((H[0]==NULL)||(V[0]==NULL)) {
    free((void*)H[0]);free((void*)V[0]);
    mwerror(FATAL,1,"Not enough memory.");
  }

  /* initialize V[][],H[][] to 0, i.e. no lines */
  cptr=V[0]+NL*(NC-1);      while(cptr-->*V) *cptr=0;
  cptr=H[0]+(NL-1)*NC;      while(cptr-->*H) *cptr=0;

  i=levels->size-1;
  Nll=Nllrm=0;
  do {
    if ((i%100)==0) mwdebug("--> Level i=%d\n",i);
    switch (ml_opt) 
      {
      case 0 : 
	minvalue=levels->values[i]; 
	maxvalue.red=maxvalue.green=maxvalue.blue=MORPHO_INFTY;  
	maxvalue.model=image_org->model;
	break;
      case 1 : 
	minvalue.red=minvalue.green=minvalue.blue=-MORPHO_INFTY; 
	minvalue.model=image_org->model;
	maxvalue=levels->values[i]; 
	break;
      case 2 : 
	minvalue=maxvalue=levels->values[i]; 
	break;
      default : mwerror(FATAL,1,"Bad option number");
      }

    produce_HV(image_org,NL,NC,minvalue,maxvalue,H,V);
    nb_points=count_X(H,V,NL,NC);
    /*
    printf("i=%d nb_points=%d L=%d\n",i,nb_points,L);
    */
    if(nb_points>=L) 
      {
	get_open_lines(image_org,NL,NC,minvalue,maxvalue,H,V,&current_lline,L,&Nll,&Nllrm);
	get_closed_lines(image_org,NL,NC,minvalue,maxvalue,H,V,&current_lline,L,&Nll,&Nllrm);
      }
    else   /* nb_points < L */
      {
	if (nb_points == 0)
	  {
	    switch (ml_opt) 
	      {
	      case 0 : case 1:
		mwerror(WARNING,1,
			"Value (%.5f,%.5f,%.5f) is smaller or bigger than values of image_in.\n",
			levels->values[i].red,levels->values[i].green,levels->values[i].blue);
		break;
		
	      case 2 : 
		mwerror(WARNING,1,"Value (%.5f,%.5f,%.5f) doesn't appear image_in.\n",
			levels->values[i].red,levels->values[i].green,levels->values[i].blue);
		break;
		
	      default : mwerror(FATAL,1,"Bad option number");
	      }
	    mwerror(WARNING,1,"No morpho_line generated.\n");
	  }
      }
    i--;
    if((ml_opt==3)&&(i==0))   i--;
    if((ml_opt==4)&&(--i==0)) i--;

  } while(i>=0);    

  mwdebug("Removed %d level lines of length < %d over %d (%3.1f %%).\n      Remain %d level lines.\n",Nllrm,L,Nll,(100.0*Nllrm)/Nll,Nll-Nllrm); 

  if(m_image==NULL) m_image=mw_change_cmimage(m_image);
  m_image->nrow=NL;
  m_image->ncol=NC;
  m_image->first_ml=current_lline;
  /* Other channels of mimage not to be completed by this module!! */

  free((void*)H[0]);free((void*)V[0]);
  free((void*)V);free((void*)H);
}

/* --------------------
   Main module function
   -------------------- */

Cmimage cml_decompose(cmimage_in,ml_opt,L,image_in)
Cmimage cmimage_in;
int* ml_opt;
int *L;
Cfimage image_in;
{
  Cmimage cmimage=NULL;
  Cfsignal levels,tmp_levels;
  long l;
  
  cmimage=mw_change_cmimage(cmimage);
  if (cmimage == NULL) mwerror(FATAL,1,"Not enough memory.\n");

  if(cmimage_in) 
    {
      if(cmimage_in->first_ml!=NULL)
	mwerror(WARNING,1,"Level lines of cmimage_in not copied!\n");
      if((cmimage_in->nrow!=image_in->nrow)||
	 (cmimage_in->ncol!=image_in->ncol))
	mwerror(WARNING,1,"image_in and cmimage_in not of same dimension!\n Dimensions of image_in are kept.\n");
      cmimage->first_fml=cmimage_in->first_fml;
      cmimage->first_ms=cmimage_in->first_ms;
    }
  else 
    {
      cmimage->first_fml=NULL;
      cmimage->first_ms=NULL;
    }
  cmimage->nrow=image_in->nrow;
  cmimage->ncol=image_in->ncol;

  /* Color of input image should be in MODEL_HSI */
  if (image_in->model != MODEL_HSI)
    mwerror(WARNING,0,"Color model of input cfimage is not HSI !\n");

  /* construct levels according to the ml_opt */
  if(*ml_opt==1) ascending_order=-1; else ascending_order=1;
  levels = cfvalues(image_in);
  cmimage->minvalue=levels->values[0];
  cmimage->maxvalue=levels->values[levels->size-1];
  if (*ml_opt!=2)
    {
      levels->size--;
      for(l=0;l<levels->size;l++) levels->values[l]=levels->values[l+1];
    }

  if(2*levels->size>cmimage->nrow*cmimage->ncol)
    printf("\n Warning : %d different pixel values (for %d pixels) !\n",levels->size,cmimage->nrow*cmimage->ncol);

  cml_extract(levels,ml_opt,*L,image_in,cmimage);

  free(levels->values);
  free(levels);

  if (mwdbg == 1)
    {
      mwdebug("Checking mimage in ml_decompose (%d level lines)...\n",
              mw_num_cmorpho_line(cmimage->first_ml));
      llcheck(cmimage);
      mwdebug("End of checking mimage\n");
    }
  return(cmimage);
}


/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {ml_extract};
   version = {"8.1"};
   author = {"Georges Koepfler"};
   function = {"Extract morpho_lines of image"};
   usage = {
   'L':level->level
       "for the value `level' compute morpho_lines (float)",
   'l':levels->levels
       "for each value in `levels' compute morpho_lines (Fsignal)",
   'o':[ml_opt=0]->opt [0,4]
       "select the content of the morpho_lines (min-max value)",
   'd':c_out<-c_out
       "draw the output mimage using ll_draw",
   'm'->m_flag    
       "optimize memory occupation during extraction",
   image_in->image_org
       "original image",
   mimage<-m_image
       "mimage with morpho_lines of image_in"
       };
*/
/*--- MegaWave2 - Copyright (C) 1994 Jacques Froment. All Rights Reserved. ---*/
#include <stdio.h>
#include <math.h>

#define NDEBUG             /* comment this line out to enable assert() */
#include <assert.h>
#include "mw.h"

#ifdef __STDC__
extern Cimage ml_draw(Mimage,Cimage,char*,Cmovie);
#else
extern Cimage ml_draw();
#endif

#define INSIDE(A)   ((minvalue<=(A))&&((A)<=maxvalue))
#define OUTSIDE(A)  (!INSIDE(A))

void
produce_HV(im,NL,NC,minvalue,maxvalue,H,V)
float **im,minvalue,maxvalue;
unsigned int NL,NC;
unsigned char **H, **V;
{
  unsigned int l,c;

  for(l=1;l<NL;l++) for(c=1;c<NC;c++) {
    if(INSIDE(im[l][c])) {
      if(OUTSIDE(im[l][c-1]))  V[l][c-1]=1; 
      if(OUTSIDE(im[l-1][c]))  H[l-1][c]=1;
    }
    else {
      if(INSIDE(im[l][c-1])) V[l][c-1]=1; 
      if(INSIDE(im[l-1][c])) H[l-1][c]=1;
    }
  }
  /* oops: almost forgot these */
  for(l=1;l<NL;l++) {
    if(INSIDE(im[l][0])) 
      {if(OUTSIDE(im[l-1][0]))  H[l-1][0]=1;}
    else
      {if(INSIDE(im[l-1][0])) H[l-1][0]=1;}
  }
  for(c=1;c<NC;c++) {
    if(INSIDE(im[0][c]))
      {if(OUTSIDE(im[0][c-1]))  V[0][c-1]=1;}
    else
      {if(INSIDE(im[0][c-1])) V[0][c-1]=1;}
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
      assert((sum==2)||(sum==4));
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

Morpho_line
produce_lline(old_lline,minvalue,maxvalue,current_point,current_type,open)
Morpho_line old_lline;
float minvalue,maxvalue;
Point_curve current_point;
Point_type  current_type;
unsigned char open;
{
  Morpho_line lline=NULL;

  if(!(lline=mw_change_morpho_line(lline)))
	  mwerror(FATAL,1,"Not enough memory for produce_lline.");
  lline->previous=NULL;
  lline->next=old_lline;
  lline->minvalue=minvalue;
  lline->maxvalue=maxvalue;
  lline->open=open;
  lline->first_point=current_point;
  lline->first_type=current_type;
  return(lline);
}


void
follow_open_line(NL,NC,H,V,ll,cc,sum,p,t)
unsigned int NL,NC;
unsigned char **H,**V;
int ll,cc,sum;
Point_curve *p;
Point_type  *t;
{
  Point_curve current_point=*p;
  Point_type  current_type=*t;

  while(sum!=0) {
    assert((sum==1)||(sum==3));
    current_point->x=cc+1;
    current_point->y=ll+1;
    current_point=current_point->next;
    current_type->type=0;                /* point inside image */
    current_type=current_type->next;
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
    sum=((ll<0)||(cc<0)||(ll==NL-1)||(cc==NC-1)) ?
      0:(V[ll][cc]+H[ll][cc]+V[ll+1][cc]+H[ll][cc+1]);
  } /* end while */
  assert((ll<0)||(cc<0)||(ll==NL-1)||(cc==NC-1));
  current_point->y=(ll<0)? 0:((ll==NL-1)? NL:ll+1);
  current_point->x=(cc<0)? 0:((cc==NC-1)? NC:cc+1);
  *p=current_point->next;
  current_point->next=NULL;
  current_type->type=1;                    /* point on border of image */
  *t=current_type->next;
  current_type->next=NULL;
}

void
get_open_lines(im,NL,NC,minvalue,maxvalue,H,V,lline,p,t)
float **im, minvalue,maxvalue;
unsigned int NL,NC;
unsigned char **H,**V;
Morpho_line *lline;
Point_curve *p;
Point_type *t;
{
  Morpho_line old_lline;
  Point_curve current_point=*p;
  Point_type  current_type=*t;
  int sum,l,c;

  /* open lines starting in the first row */
  for(c=0;c<NC-1;c++) 
    if((V[0][c]==1)&&OUTSIDE(im[0][c])) { /* such that levelset at left */
      old_lline=*lline;
      *lline=produce_lline(old_lline,minvalue,maxvalue,current_point,current_type,1);
      if(old_lline!=NULL) old_lline->previous=*lline;
      current_point->x=c+1;
      current_point->y=0;
      current_point->previous=NULL;
      current_point=current_point->next;
      current_type->type=1;                /* point on border of image */
      current_type->previous=NULL;
      current_type=current_type->next;
      V[0][c]=0;
      sum=H[0][c]+V[1][c]+H[0][c+1];
      assert(sum!=0);
      follow_open_line(NL,NC,H,V,0,c,sum,&current_point,&current_type);
    }

  /* open lines starting in the last row */
  for(c=0;c<NC-1;c++) 
    if((V[NL-1][c]==1)&&INSIDE(im[NL-1][c])) { /* ...levelset at left */
      old_lline=*lline;
      *lline=produce_lline(old_lline,minvalue,maxvalue,current_point,current_type,1);
      if(old_lline!=NULL) old_lline->previous=*lline;
      current_point->x=c+1;
      current_point->y=NL;
      current_point->previous=NULL;
      current_point=current_point->next;      
      current_type->type=1;                /* point on border of image */
      current_type->previous=NULL;
      current_type=current_type->next;
      V[NL-1][c]=0;
      sum=H[NL-2][c+1]+V[NL-2][c]+H[NL-2][c];
      assert(sum!=0);
      follow_open_line(NL,NC,H,V,(int)NL-2,c,sum,&current_point,&current_type);
    }

  /* open lines starting in the first column */
  for(l=0;l<NL-1;l++)
    if((H[l][0]==1)&&INSIDE(im[l][0])) { /* ...levelset at left       */
      old_lline=*lline;
      *lline=produce_lline(old_lline,minvalue,maxvalue,current_point,current_type,1);
      if(old_lline!=NULL) old_lline->previous=*lline;
      current_point->x=0;
      current_point->y=l+1;
      current_point->previous=NULL;
      current_point=current_point->next;
      current_type->type=1;                /* point on border of image */
      current_type->previous=NULL;
      current_type=current_type->next;
      H[l][0]=0;
      sum=V[l+1][0]+H[l][1]+V[l][0];
      assert(sum!=0);
      follow_open_line(NL,NC,H,V,l,0,sum,&current_point,&current_type);
    }

  /* open lines starting in the last column */
  for(l=0;l<NL-1;l++)
    if((H[l][NC-1]==1)&&OUTSIDE(im[l][NC-1])) { /* ...levelset at left  */
      old_lline=*lline;
      *lline=produce_lline(old_lline,minvalue,maxvalue,current_point,current_type,1);
      if(old_lline!=NULL) old_lline->previous=*lline;
      current_point->x=NC;
      current_point->y=l+1;
      current_point->previous=NULL;
      current_point=current_point->next;
      current_type->type=1;                /* point on border of image */
      current_type->previous=NULL;
      current_type=current_type->next;
      H[l][NC-1]=0;
      sum=V[l][NC-2]+H[l][NC-2]+V[l+1][NC-2];
      assert(sum!=0);
      follow_open_line(NL,NC,H,V,l,(int)NC-2,sum,&current_point,&current_type);
    }
  *p=current_point;
  *t=current_type;
}

void
follow_closed_line(H,V,ll,cc,sum,p,t)
unsigned char **H,**V;
int ll,cc,sum;
Point_curve *p;
Point_type *t;
{
  Point_curve current_point=*p;
  Point_type  current_type=*t;

  while(sum!=0) {
    assert((sum==1)||(sum==3));
    current_point->x=cc+1;
    current_point->y=ll+1;
    current_point=current_point->next;
    current_type->type=0;                /* point inside image */
    current_type=current_type->next;    
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
  *p=current_point;
  if(current_point)
    current_point->previous->next=NULL;
  *t=current_type;
  if(current_type)
    current_type->previous->next=NULL;
}

void
get_closed_lines(im,NL,NC,minvalue,maxvalue,H,V,lline,p,t)
float **im, minvalue,maxvalue;
unsigned int NL,NC;
unsigned char **H,**V;
Morpho_line *lline;
Point_curve *p;
Point_type *t;
{
  Morpho_line old_lline;
  Point_curve current_point=*p;
  Point_type current_type=*t;
  int sum,l,c;

  for(l=0;l<NL-1;l++) for(c=0;c<NC-1;c++) 
    if(H[l][c+1]!=0) {                    /* start a closed line */
      assert(V[l+1][c]!=0);
      old_lline=*lline;
      *lline=produce_lline(old_lline,minvalue,maxvalue,current_point,current_type,0);
      if(old_lline!=NULL) old_lline->previous=*lline;
      current_point->x=c+1;
      current_point->y=l+1;
      current_point->previous=NULL;
      current_point=current_point->next;      
      current_type->type=0;                /* point inside image */
      current_type->previous=NULL;
      current_type=current_type->next;
      if(OUTSIDE(im[l][c+1])) { /* such that the levelset is at left */
	V[l+1][c]=0; 
	sum=H[l+1][c]+V[l+2][c]+H[l+1][c+1];
	assert(sum!=0);
	follow_closed_line(H,V,l+1,c,sum,&current_point,&current_type);
      }
      else {
	H[l][c+1]=0;
	sum=V[l+1][c+1]+H[l][c+2]+V[l][c+1];
	assert(sum!=0);
	follow_closed_line(H,V,l,c+1,sum,&current_point,&current_type);
      }
    }
  *p=current_point;   /* no more needed ? */
  *t=current_type;    /* no more needed ? */
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

void
ml_extract(level,levels,opt,c_out,m_flag,image_org,m_image)
float*  level;
Fsignal levels;
int *opt;
Cimage c_out;
char* m_flag;
Fimage image_org;
Mimage m_image;
{
  Morpho_line current_lline=NULL;
  Point_curve current_point, next_point;
  Point_type  current_type , next_type;
  unsigned long nb_points, l;
  unsigned int NL=image_org->nrow, NC=image_org->ncol;
  unsigned char **V, **H, *cptr;
  int i,ml_opt=*opt;
  float **im, minvalue, maxvalue;

  if((level==NULL)==(levels==NULL))
         mwerror(USAGE,1,"Bad level values input.");

  if(level) 
    if(ml_opt<=2) {
      levels=mw_change_fsignal(levels,1);
      levels->size=1;
      levels->values[0]=*level;
    }
    else
      mwerror(USAGE,1,"Bad combination of -L and -o .");

  if(levels) {
    if( (ml_opt==4) && (levels->size % 2 != 0) )
      mwerror(USAGE,1,"Even number of values needed for ml_opt 4");
    if(((ml_opt==3)||(ml_opt==4))&&(levels->size<2))
      mwerror(USAGE,1,"At least two values needed for ml_opt 3 and 4");
  }

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

  /* get easy access to image_org->gray */
  im=(float **)malloc(NL*sizeof(float*));
  im[0]=image_org->gray;
  for(i=1;i<NL;i++) im[i]=im[i-1]+NC;
  
  i=levels->size-1;
  do {
    switch (ml_opt) {
      /*    case 0 : {minvalue=levels->values[i]; maxvalue=MORPHO_INFTY;  break;}*/
    case 0 : {minvalue=levels->values[i]; maxvalue=MORPHO_INFTY;  break;}
    case 1 : {minvalue=-MORPHO_INFTY; maxvalue=levels->values[i]; break;}
    case 2 : {minvalue=maxvalue=levels->values[i]; break;}
    case 3 : {minvalue=levels->values[i-1]; maxvalue=levels->values[i]; break;}
    case 4 : {minvalue=levels->values[i-1]; maxvalue=levels->values[i]; break;}
    default : mwerror(FATAL,1,"Bad option number");
    }

    produce_HV(im,NL,NC,minvalue,maxvalue,H,V);
    nb_points=count_X(H,V,NL,NC);
    if(nb_points!=0) {
      if(m_flag) { /* optimize memory occupation... allows no free on points structure */
	current_point=
	  (Point_curve)malloc(nb_points*sizeof(struct point_curve));
	current_type=
	  (Point_type)malloc(nb_points*sizeof(struct point_type));
	if((current_point==NULL)||(current_type==NULL)) {
          free((void*)H[0]);free((void*)V[0]);
          mwerror(FATAL,1,"Not enough memory.");
	}
	for(l=1;l<nb_points-1;l++) {
	  (current_point+l)->previous=current_point+l-1;
	  (current_point+l)->next=current_point+l+1;
	  (current_type+l)->previous=current_type+l-1;
	  (current_type+l)->next=current_type+l+1;
	}
	current_point->previous=NULL;
	current_point->next=current_point+1;
	current_point[nb_points-1].previous=current_point+nb_points-2;
	current_point[nb_points-1].next=NULL;
	current_type[0].previous=NULL;
	current_type[0].next=current_type+1;
	current_type[nb_points-1].previous=current_type+nb_points-2;
	current_type[nb_points-1].next=NULL;
      }
      else {       /* dont optimize... but possibility to free(points)                 */
	next_point=NULL;
	next_type=NULL;
	while(nb_points-->0) {
	  current_point=mw_new_point_curve();
	  current_type=mw_new_point_type();
	  if((current_point==NULL)||(current_type==NULL)) {
	    free((void*)H[0]);free((void*)V[0]);
	    mwerror(FATAL,1,"Not enough memory.");
	  }
	  if(next_point==NULL)
	    current_point->next=NULL;
	  else {
	    current_point->next=next_point;
	    next_point->previous=current_point;
	  }
	  current_point->previous=NULL;
	  next_point=current_point;
	  if(next_type==NULL)
	    current_type->next=NULL;
	  else {
	    current_type->next=next_type;
	    next_type->previous=current_type;
	  }
	  current_type->previous=NULL;
	  next_type=current_type;
	}
      }

      get_open_lines(im,NL,NC,minvalue,maxvalue,H,V,
		       &current_lline,&current_point,&current_type);

      get_closed_lines(im,NL,NC,minvalue,maxvalue,H,V,
		       &current_lline,&current_point,&current_type);

    }
    else {
      switch (ml_opt) {
      case 0 : {
	mwerror(WARNING,1,"Value %.5f is smaller or bigger than values of image_in.\n",levels->values[i]);
	break;
      }
      case 1 : {
	mwerror(WARNING,1,"Value %.5f is smaller or bigger than values of image_in.\n",levels->values[i]);
	break;
      }
      case 2 : {
	mwerror(WARNING,1,"Value %.5f doesn't appear image_in.\n",levels->values[i]);
	break;
      }
      case 3 :{
	mwerror(WARNING,1,"No pixels with values between %.5f and %.5f in image_in.\n",levels->values[i-1],levels->values[i]);
	break;
      }
      case 4 :{
	mwerror(WARNING,1,"No pixels with values between %.5f and %.5f in image_in.\n",levels->values[i-1],levels->values[i]);
	break;
      }
      default : mwerror(FATAL,1,"Bad option number");
      }
      mwerror(WARNING,1,"No morpho_line generated.\n");
    }
    i--;
    if((ml_opt==3)&&(i==0))   i--;
    if((ml_opt==4)&&(--i==0)) i--;

  } while(i>=0);    

  if(m_image==NULL)
    m_image=mw_change_mimage(m_image);
  m_image->nrow=NL;
  m_image->ncol=NC;
  m_image->first_ml=current_lline;
  /* Other channels of mimage not to be completed by this module!! */

  if(c_out)
    *c_out=*ml_draw(m_image,(Cimage) NULL,(char *) NULL, (Cmovie) NULL);

  if(level) mw_delete_fsignal(levels);

  free((void*)H[0]);free((void*)V[0]);
  free((void*)V);free((void*)H);free((void*)im);
}

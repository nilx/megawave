/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
  name = {ml_reconstruct};
  version = {"2.5"};
  author = {"Georges Koepfler"};
  function = {"Reconstruct image from morpho_lines of mimage"};
  usage = {
  'i'->v_flag
         "use the maxvalue of the morpho_lines (default minvalue)",
  m_image->m_image
         "input mimage",
  image_out<-ml_reconstruct
        "reconstructed fimage"
  };
*/
/*--- MegaWave - Copyright (C) 1992 Jacques Froment. All Rights Reserved. ---*/

#include <stdio.h>
#include <assert.h>
#include "mw.h"

struct closest_lline {
         Morpho_line  lline;
	 Point_curve point;
	 int         distance;
       };

#define FALSE 0
#define TRUE  1

#define POINT_OK(P,Y,X)  (((P)->x>=0)&&((P)->x<=X)&&((P)->y>=0)&&((P)->y<=Y))
#define BAD_POINT(P,Y,X) (!POINT_OK(P,Y,X))
#define INC_OK(D1,D2)    (((abs(D1)==1)&&(D2==0))||((D1==0)&&(abs(D2)==1)))
#define BAD_INC(D1,D2)   (!INC_OK(D1,D2))

#define N_EQUAL(A,B)     (((A)<(B))||((A)>(B)))
#define EQUAL(A,B)       (!N_EQUAL(A,B))
#define GET_VALUE(A)     ((v_flag==NULL)? (A)->minvalue : (A)->maxvalue)

extern int mwdbg;

void llcheck(mimage)

Mimage mimage;

{  Point_curve point;
   Morpho_line ll;
   int NC,NL;
   
   NC=mimage->ncol;
   NL=mimage->nrow;
   for (ll=mimage->first_ml; ll; ll=ll->next)
     {
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
void
draw_lline(lline_ptr,c_lline,V,H1,NL,NC)
Morpho_line lline_ptr;
struct closest_lline *c_lline;
unsigned char **V,*H1;
int NL,NC;
{
  Point_curve point;
  int l,c,dl,dc;
  float d;

  point=lline_ptr->first_point;
  if(BAD_POINT(point,NL,NC))
    {
      mwdebug("Morpho Line number = %d : \t First point->x=%d (NC=%d) \t point->y=%d (NL=%d)\n",lline_ptr->num,point->x,NC,point->y,NL);
      mwerror(FATAL,1,"Point out of image.\n");
    }
  if((d=point->x+point->y)<c_lline->distance) {
    c_lline->distance=d;
    c_lline->point   =point;
    c_lline->lline   =lline_ptr;
  }
  l=point->y;
  c=point->x;
  while(point->next!=NULL) {
    point=point->next;
    if(BAD_POINT(point,NL,NC))
      {
	mwdebug("Morpho Line number = %d : \t point->x=%d (NC=%d) \t point->y=%d (NL=%d)\n",lline_ptr->num,point->x,NC,point->y,NL);
	mwerror(FATAL,1,"Point out of image.\n");
      }
    if((d=point->x+point->y)<c_lline->distance) {
      c_lline->distance=d;
      c_lline->point   =point;
      c_lline->lline   =lline_ptr;
    }
    dl=point->y-l;
    dc=point->x-c;
    if(BAD_INC(dl,dc)) 
      mwerror(FATAL,1,"Points are not 4-adjacent.\n");
    if(dl==1)
      V[l][c-1]=TRUE;
    else if(dl==-1)
      V[l-1][c-1]=TRUE;
    else if(dc==1)
      {if(c==0) H1[l-1]=TRUE;}
    else /* dc==-1 */
      {if(c==1) H1[l-1]=TRUE;}
    l+=dl;
    c+=dc;
  }
  if(lline_ptr->open==0) {  /* closed polygon */
    dl=lline_ptr->first_point->y-l;
    dc=lline_ptr->first_point->x-c;
    if(BAD_INC(dl,dc))
      mwerror(FATAL,1,"Points are not 4-adjacent.\n");
    if(dl==1)
      V[l][c-1]=TRUE;
    else if(dl==-1)
      V[l-1][c-1]=TRUE;
    else if(dc==1)
      {if(c==0) H1[l-1]=TRUE;}
    else /* dc==-1 */
      {if(c==1) H1[l-1]=TRUE;}
  }
}

unsigned char
zero_inside(c_lline)
struct closest_lline *c_lline;
{
  Point_curve c_p,p_p,n_p;

  c_p=c_lline->point;
  if(c_lline->lline->open==0) {           /* closed polygon */
    if(c_p->previous==NULL)   /* get the last point */
      for(p_p=c_p;p_p->next!=NULL;p_p=p_p->next);
    else
      p_p=c_p->previous;
    n_p=(c_p->next    ==NULL) ? c_lline->lline->first_point:c_p->next;
    if(((p_p->x-c_p->x)*(n_p->y-c_p->y)
	                          - (p_p->y-c_p->y)*(n_p->x-c_p->x))>0)
      return(FALSE);
    else
      return(TRUE);
  }
  else {                                            /* open polygon   */
    if((p_p=c_p->previous)==NULL) {
      return(c_p->x<c_p->y);    /*=c_p starts in last line or first column*/
    }
    else if((n_p=c_p->next)==NULL){
      return(c_p->x>c_p->y);    /*=c_p starts in first line or last column*/
    }
    else {
      if(((p_p->x-c_p->x)*(n_p->y-c_p->y)
	                            - (p_p->y-c_p->y)*(n_p->x-c_p->x))>0)
	return(FALSE);
      else
	return(TRUE);
    }
  }
}

void
fill_lline(V,H1,level,inside,im,NL,NC)
unsigned char **V,*H1,inside;
float level,**im;
int NL,NC;
{
  int l,c;
  unsigned char prec_line;

  prec_line=inside;
  for(l=0;l<NL;l++)  {
    if(l!=0) 
      if(H1[l-1]==TRUE)
	{inside=!prec_line;H1[l-1]=FALSE;prec_line=inside;}
    else 
      inside=prec_line;
    if(inside) im[l][0]=level;
    for(c=1;c<NC;c++) 
      {
	if(V[l][c-1]==TRUE) 
	  {
	    inside=!(inside);
	    V[l][c-1]=FALSE;
	  }
	if (inside) im[l][c]=level;
      }
  }
}

Fimage
ml_reconstruct(v_flag,m_image)
char* v_flag;
Mimage m_image;
{ 
  unsigned char **V,*H1,*cptr,inc_val,inside,zero_inside();
  int NC,NL,i;
  float **im,current_level;
  Fimage image_out=NULL;
  Morpho_line lline_ptr;
  struct closest_lline c_lline;

  if(m_image==NULL) {
    mwerror(WARNING,1,"Mimage is NULL.");
    return(image_out);
  }
  if(m_image->minvalue>=m_image->maxvalue)
    mwerror(WARNING,1,"Bad level set representation (minvalue>=maxvalue).\n");
  NC=m_image->ncol;
  NL=m_image->nrow;
  if((NC<=0)||(NL<=0))
    mwerror(WARNING,1,"Check Mimage data.");

  if(m_image->first_ml==NULL)
    mwerror(USAGE,1,"No morpho_lines in mimage.");
  else
    lline_ptr=m_image->first_ml;

  /* memory for vertical boundaries */
  V=(unsigned char **)malloc(NL*sizeof(unsigned char*));
  V[0]=(unsigned char *)malloc(NL*(NC-1)*sizeof(unsigned char));
  for(i=1;i<NL;i++) V[i]=V[i-1]+(NC-1); 

  /* memory for first column of horizontal boundaries */
  H1=(unsigned char *)malloc((NL-1)*sizeof(unsigned char));

  if((H1==NULL)||(V[0]==NULL)) {
    free((void*)H1);free((void*)V[0]);
    mwerror(FATAL,1,"Not enough memory.\n");
  }

  /* initialize V[][],H1[] to 0, i.e. no lines */
  cptr=V[0]+NL*(NC-1);      while(cptr-->*V) *cptr=FALSE;
  cptr=H1+(NL-1);           while(cptr-->H1) *cptr=FALSE;

  if((image_out=mw_change_fimage(NULL,NL,NC))==NULL)
    mwerror(FATAL,1,"Not enough memory.\n");
  current_level=GET_VALUE(m_image);
  mw_clear_fimage(image_out,current_level);

  /* get easy access to image_out->gray */
  im=(float **)malloc(NL*sizeof(float*));
  im[0]=image_out->gray;
  for(i=1;i<NL;i++) im[i]=im[i-1]+NC;

  if (mwdbg == 1)
    {
      mwdebug("Checking mimage in ml_reconstruct (%d level lines)...\n",
	      mw_morpho_line_num(m_image->first_ml));
      llcheck(m_image);
      mwdebug("End of checking mimage\n");
    }

  inc_val=TRUE;
  do{ 
    if(inc_val&&(  ((!v_flag)&&(current_level>lline_ptr->minvalue))
                 ||( (v_flag)&&(current_level<lline_ptr->maxvalue)) ))
       inc_val=FALSE;    
    current_level=GET_VALUE(lline_ptr);
    c_lline.distance=NL+NC;
    while((lline_ptr!=NULL)&&EQUAL(GET_VALUE(lline_ptr),current_level)) {
      draw_lline(lline_ptr,&c_lline,V,H1,NL,NC);
      lline_ptr=lline_ptr->next;
    }
    inside=zero_inside(&c_lline);
    fill_lline(V,H1,current_level,inside,im,NL,NC);
  }while(lline_ptr!=NULL);
  if(!inc_val)
    if(!v_flag)
      mwerror(WARNING,1,"Values of level lines not increasing.");
    else
      mwerror(WARNING,1,"Values of level lines not decreasing.");

  free((void*)V[0]);free((void*)H1);
  free((void*)V);free((void*)im);
  return(image_out);
}

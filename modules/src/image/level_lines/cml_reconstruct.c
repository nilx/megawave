/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {cml_reconstruct};
 version = {"2.6"};
 author = {"Jacques Froment, Georges Koepfler"};
 function = {"Reconstruct a color image from cmorpho_lines of cmimage"};
 usage = {
   'i'->v_flag     "use the maxvalue of the cmorpho_lines (default minvalue)",
   m_image->m_image            "input cmimage",
   image_out<-cml_reconstruct  "reconstructed cfimage"
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mw.h"

struct closest_lline {
         Cmorpho_line  lline;
	 Point_curve point;
	 int         distance;
       };

#define FALSE 0
#define TRUE  1

#define POINT_OK(P,Y,X)  (((P)->x>=0)&&((P)->x<=X)&&((P)->y>=0)&&((P)->y<=Y))
#define BAD_POINT(P,Y,X) (!POINT_OK(P,Y,X))
#define INC_OK(D1,D2)    (((abs(D1)==1)&&(D2==0))||((D1==0)&&(abs(D2)==1)))
#define BAD_INC(D1,D2)   (!INC_OK(D1,D2))

#define N_EQUAL(A,B)     ((cmpcolor((A),(B))<0)||(cmpcolor((A),(B))>0))
#define EQUAL(A,B)       (!N_EQUAL(A,B))
#define GET_VALUE(A)     ((v_flag==NULL)? (A)->minvalue : (A)->maxvalue)

extern int mwdbg;

static int ascending_order=1;

/* ------------------------------------------------------------------
   Compare two colors : return -1 if v1<v2, 1 if v1>v2 and 0 if v1=v2
            (if ascending_order=-1 and not 1 the values are opposed)
   ------------------------------------------------------------------ */


int cmpcolor(c1,c2)
Color c1,c2;
{
  if (c1.model != c2.model)
    mwerror(INTERNAL,1,"[cmpcolor] Two different color models %d and %d !\n",
	    c1.model,c2.model);
  switch(c1.model)
    {
    case MODEL_HSI :
      /* I is blue */
      if (c1.blue < c2.blue) return(-ascending_order); 
      if (c1.blue > c2.blue) return(ascending_order);
      /* H is red (between 0 and 360 deg. 0 is red) */
      if (c1.red < c2.red) return(-ascending_order);
      if (c1.red > c2.red) return(ascending_order);      
      /* S is green (between 0 and 1) */
      if (c1.green < c2.green) return(-ascending_order);      
      if (c1.green > c2.green) return(ascending_order);      
      return(0);
      
    default:
      mwerror(INTERNAL,1,"[cmpcolor] No color order defined for color model %d !\n",(int)c1.model);
    }
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


void
draw_lline(lline_ptr,c_lline,V,H1,NL,NC)
Cmorpho_line lline_ptr;
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
      mwdebug("Cmorpho Line number = %d : \t First point->x=%d (NC=%d) \t point->y=%d (NL=%d)\n",lline_ptr->num,point->x,NC,point->y,NL);
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
	mwdebug("Cmorpho Line number = %d : \t point->x=%d (NC=%d) \t point->y=%d (NL=%d)\n",lline_ptr->num,point->x,NC,point->y,NL);
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
fill_lline(V,H1,level,inside,image,NL,NC)
unsigned char **V,*H1,inside;
Color level;
Cfimage image;
int NL,NC;
{
  int l,c;
  unsigned char prec_line;
  unsigned long p;

  prec_line=inside;
  for(l=0;l<NL;l++)  {
    if(l!=0) 
      if(H1[l-1]==TRUE)
	{inside=!prec_line;H1[l-1]=FALSE;prec_line=inside;}
    else 
      inside=prec_line;
    if(inside) 
      {
	p =  l*image->ncol;
	image->red[p]=level.red;
	image->green[p]=level.green;
	image->blue[p]=level.blue;
      }
    for(c=1;c<NC;c++) 
      {
	if(V[l][c-1]==TRUE) 
	  {
	    inside=!(inside);
	    V[l][c-1]=FALSE;
	  }
	if (inside) 
	  {
	    p = c + l*image->ncol;
	    image->red[p]=level.red;
	    image->green[p]=level.green;
	    image->blue[p]=level.blue;
	  }
      }
  }
}

Cfimage
cml_reconstruct(v_flag,m_image)
char* v_flag;
Cmimage m_image;
{ 
  unsigned char **V,*H1,*cptr,inc_val,inside,zero_inside();
  int NC,NL,i;
  Color current_level;
  Cfimage image_out=NULL;
  Cmorpho_line lline_ptr;
  struct closest_lline c_lline;

  if(m_image==NULL) {
    mwerror(WARNING,1,"Cmimage is NULL.");
    return(image_out);
  }
  if(cmpcolor(m_image->minvalue,m_image->maxvalue) >= 0)
    mwerror(WARNING,1,"Bad level set representation (minvalue>=maxvalue).\n");
  NC=m_image->ncol;
  NL=m_image->nrow;
  if((NC<=0)||(NL<=0))
    mwerror(WARNING,1,"Check Cmimage data.");

  if(m_image->first_ml==NULL)
    mwerror(USAGE,1,"No cmorpho_lines in cmimage.");
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

  if((image_out=mw_change_cfimage(NULL,NL,NC))==NULL)
    mwerror(FATAL,1,"Not enough memory.\n");
  current_level=GET_VALUE(m_image);
  mw_clear_cfimage(image_out,current_level.red,current_level.green,
		   current_level.blue);
  image_out->model = m_image->minvalue.model;

  if (mwdbg == 1)
    {
      mwdebug("Checking cmimage in cml_reconstruct (%d level lines)...\n",
	      mw_num_cmorpho_line(m_image->first_ml));
      llcheck(m_image);
      mwdebug("End of checking cmimage\n");
    }

  inc_val=TRUE;
  do{ 
    if(inc_val&&(  ((!v_flag)&&(cmpcolor(current_level,lline_ptr->minvalue)>0))
                 ||( (v_flag)&&(cmpcolor(current_level,lline_ptr->maxvalue)<0)) ))
       inc_val=FALSE;    
    current_level=GET_VALUE(lline_ptr);
    c_lline.distance=NL+NC;
    while((lline_ptr!=NULL)&&EQUAL(GET_VALUE(lline_ptr),current_level)) {
      draw_lline(lline_ptr,&c_lline,V,H1,NL,NC);
      lline_ptr=lline_ptr->next;
    }
    inside=zero_inside(&c_lline);
    fill_lline(V,H1,current_level,inside,image_out,NL,NC);
  }while(lline_ptr!=NULL);
  if(!inc_val)
    if(!v_flag)
      mwerror(WARNING,1,"Values of level lines not increasing.");
    else
      mwerror(WARNING,1,"Values of level lines not decreasing.");

  free((void*)V[0]);free((void*)H1);
  free((void*)V);

  return(image_out);
}

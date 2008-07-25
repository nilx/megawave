/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mimage.c
   
  Vers. 1.9
  Author : Jacques Froment
  Basic memory routines for the morpho_line, morpho_set & mimage internal types


  Main changes :
  v1.9 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <string.h>

#include "mw.h"

/*--- Point-type ---*/

/* creates a new point_type structure */

Point_type mw_new_point_type(void)
{
     Point_type point;

     if(!(point = (Point_type) (malloc(sizeof(struct point_type)))))
     {
	  mwerror(ERROR, 0, "[mw_new_point_type] Not enough memory\n");
	  return(NULL);
     }
     point->type = 0;
     point->previous = NULL;
     point->next = NULL;

     return(point);
}

/* Define the struct if it's not defined */

Point_type mw_change_point_type(Point_type point)
{
     if (point == NULL) point = mw_new_point_type();
     return(point);
}

/* desallocate all the point_type structures from a starting point */

void mw_delete_point_type(Point_type point)
{   
     Point_type point_next,point_first;

     if (point == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_point_type] cannot delete : point_type structure is NULL\n");
	  return;
     }

     point_first=point;
     do
     {
	  point_next = point->next;
	  free(point);
	  point = point_next;
     } while ((point != NULL)&&(point != point_first));
}

Point_type mw_copy_point_type(Point_type in, Point_type out)
{ 
     Point_type pt,qt0,qt1;

     if (!in)
     {
	  mwerror(ERROR, 0,"[mw_copy_point_type] NULL input point_type\n");
	  return(NULL);
     }
     if (!out)
     {
	  out = mw_new_point_type();
	  if (!out) mwerror(ERROR, 0,"[mw_copy_point_type] Not enough memory to create a point_type\n");
	  return(NULL);      
     }
  
     out->type = in->type;
     out->previous = NULL;

     qt0 = out;
     qt1 = NULL;
     for (pt=in->next; pt; pt=pt->next)
     {
	  qt1 = mw_new_point_type();
	  if (qt1 == NULL)
	  {
	       mw_delete_point_type(qt1);
	       mwerror(ERROR, 0,"Not enough memory to create a point_type\n");
	       return(NULL);
	  }
	  qt1->type = pt->type;
	  qt1->previous = qt0;
	  qt0->next = qt1;
	  qt0 = qt1;
     }
     return(out);
}

/*--- Morpho_line ---*/

/* Creates a new morpho_line structure */

Morpho_line mw_new_morpho_line(void)
{
     Morpho_line morpho_line;

     if(!(morpho_line = (Morpho_line) (malloc(sizeof(struct morpho_line)))))
     {
	  mwerror(ERROR, 0, "[mw_new_morpho_line] Not enough memory\n");
	  return(NULL);
     }
  
     morpho_line->first_point = NULL;
     morpho_line->first_type = NULL;
     morpho_line->minvalue = morpho_line->maxvalue = 0.0;
     morpho_line->open = 0;
     morpho_line->data = 0.0;
     morpho_line->pdata = NULL;
     morpho_line->previous = NULL;
     morpho_line->next = NULL;
     morpho_line->num = 0;
     morpho_line->morphosets = NULL;
     return(morpho_line);
}

/* Define the struct if it's not defined */

Morpho_line mw_change_morpho_line(Morpho_line ll)
{
     if (ll == NULL) ll = mw_new_morpho_line();
     return(ll);
}

/* deallocate the morpho_line structure */

void mw_delete_morpho_line(Morpho_line morpho_line)
{
     if (morpho_line == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_morpho_line] cannot delete : morpho_line structure is NULL\n");
	  return;
     }

     if (morpho_line->first_point) mw_delete_point_curve(morpho_line->first_point);
     morpho_line->first_point = NULL;
     if (morpho_line->first_type) mw_delete_point_type(morpho_line->first_type);
     morpho_line->first_type = NULL;

     free(morpho_line);
     morpho_line=NULL;
}

/* Copy a morpho_line into another morpho_line */

Morpho_line mw_copy_morpho_line(Morpho_line in, Morpho_line out)
{ 
     if (!in)
     {
	  mwerror(ERROR, 0,"[mw_copy_morpho_line] NULL input morpho_line\n");
	  return(NULL);
     }

     if (!out)
     {
	  out=mw_new_morpho_line();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_morpho_line] Not enough memory to create a morpho_line !\n");
	       return(NULL);
	  }
     }
     out->minvalue = in->minvalue;
     out->maxvalue = in->maxvalue;
     out->open = in->open;
     out->data = in->data;
     if ( ((out->first_point = mw_new_point_curve()) == NULL) ||
	  ((out->first_type = mw_new_point_type()) == NULL) )
     {
	  mwerror(ERROR, 0,"Not enough memory to create a morpho_line\n");
	  return(NULL);
     }
     mw_copy_point_curve(in->first_point,out->first_point);
     mw_copy_point_type(in->first_type,out->first_type);
     return(out);
}

/* Return the number of points into a morpho_line */

unsigned int mw_length_morpho_line(Morpho_line morpho_line)
{ 
     unsigned int n,m;
     Point_curve p,pfirst;
     Point_type t,tfirst;

     if ((!morpho_line) || (!morpho_line->first_point)) return(0);

     for (p=pfirst=morpho_line->first_point, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);

     if (!morpho_line->first_type)
     {
	  for (t=tfirst=morpho_line->first_type, m=0; 
	       (t != NULL)&&(t->next != tfirst); m++, t=t->next);

	  if ( (n*m != 0) && (n != m) )
	       mwerror(INTERNAL,1,"[mw_length_morpho_line] Inconsistent Morpho_line structure \n");
     }
     return(n);
}

/* Compute the num field of a morpho_line chain */
/* Return the number of morpho_lines             */

unsigned int mw_num_morpho_line(Morpho_line ml_first)
{ 
     Morpho_line ml;
     unsigned int n;

     if (!ml_first)
     {
	  mwerror(ERROR, 0,
		  "[mw_num_morpho_line] NULL input morpho_line\n");
	  return(0);
     }  
     for (ml=ml_first, n=1; ml; ml->num=n, ml=ml->next, n++);
     return(n-1);
}

/*--- Fmorpho_line ---*/

/* Creates a new fmorpho_line structure */

Fmorpho_line mw_new_fmorpho_line(void)
{
     Fmorpho_line fmorpho_line;

     if(!(fmorpho_line = (Fmorpho_line) (malloc(sizeof(struct fmorpho_line)))))
     {
	  mwerror(ERROR, 0, "[mw_new_fmorpho_line] Not enough memory\n");
	  return(NULL);
     }
  
     fmorpho_line->first_point = NULL;
     fmorpho_line->first_type = NULL;
     fmorpho_line->minvalue = fmorpho_line->maxvalue = 0.0;
     fmorpho_line->open = 0;
     fmorpho_line->data = 0.0;
     fmorpho_line->pdata = NULL;
     fmorpho_line->previous = NULL;
     fmorpho_line->next = NULL;
     return(fmorpho_line);
}

/* Define the struct if it's not defined */

Fmorpho_line mw_change_fmorpho_line(Fmorpho_line ll)
{
     if (ll == NULL) ll = mw_new_fmorpho_line();
     return(ll);
}

/* desallocate the fmorpho_line structure */

void mw_delete_fmorpho_line(Fmorpho_line fmorpho_line)
{
     if (fmorpho_line == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_fmorpho_line] cannot delete : fmorpho_line structure is NULL\n");
	  return;
     }

     if (fmorpho_line->first_point) mw_delete_point_fcurve(fmorpho_line->first_point);
     fmorpho_line->first_point = NULL;
     if (fmorpho_line->first_type) mw_delete_point_type(fmorpho_line->first_type);
     fmorpho_line->first_type = NULL;

     free(fmorpho_line);
     fmorpho_line=NULL;
}

/* Copy a fmorpho_line into another fmorpho_line */

Fmorpho_line mw_copy_fmorpho_line(Fmorpho_line in, Fmorpho_line out)
{ 
     Point_fcurve pc,qc0,qc1;
     Point_type pt,qt0,qt1;

     if (!in)
     {
	  mwerror(ERROR, 0,"[mw_copy_fmorpho_line] NULL input fmorpho_line\n");
	  return(NULL);
     }
  
     if (!out)
     {
	  out=mw_new_fmorpho_line();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_fmorpho_line] Not enough memory to create a fmorpho_line !\n");
	       return(NULL);
	  }
     }
     out->minvalue = in->minvalue;
     out->maxvalue = in->maxvalue;
     out->open = in->open;
     out->data = in->data;

     qc0 = qc1 = NULL;
     for (pc=in->first_point; pc; pc=pc->next)
     {
	  qc1 = mw_new_point_fcurve();
	  if (qc1 == NULL)
	  {
	       mw_delete_point_fcurve(qc1);
	       mwerror(ERROR, 0,"Not enough memory to create a fmorpho_line\n");
	       return(NULL);
	  }
	  qc1->x = pc->x;
	  qc1->y = pc->y;
	  qc1->previous = qc0;
	  if (qc0 == NULL) out->first_point = qc1;
	  else qc0->next = qc1;
	  qc0 = qc1;
     }

     qt0 = qt1 = NULL;
     for (pt=in->first_type; pt; pt=pt->next)
     {
	  qt1 = mw_new_point_type();
	  if (qt1 == NULL)
	  {
	       mw_delete_point_fcurve(out->first_point);
	       mw_delete_point_type(qt1);
	       mwerror(ERROR, 0,"Not enough memory to create a fmorpho_line\n");
	       return(NULL);
	  }
	  qt1->type = pt->type;
	  qt1->previous = qt0;
	  if (qt0 == NULL) out->first_type = qt1;
	  else qt0->next = qt1;
	  qt0 = qt1;
     }
     return(out);
}

/* Return the number of points into a fmorpho_line */

unsigned int mw_length_fmorpho_line(Fmorpho_line fmorpho_line)
{ 
     unsigned int n,m;
     Point_fcurve p,pfirst;
     Point_type t,tfirst;

     if ((!fmorpho_line) || (!fmorpho_line->first_point)) return(0);

     for (p=pfirst=fmorpho_line->first_point, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);

     if (!fmorpho_line->first_type)
     {
	  for (t=tfirst=fmorpho_line->first_type, m=0; 
	       (t != NULL)&&(t->next != tfirst); m++, t=t->next);

	  if ( (n*m != 0) && (n != m) )
	       mwerror(INTERNAL,1,"[mw_length_fmorpho_line] Inconsistent Fmorpho_line structure \n");
     }
     return(n);
}

/*--- Hsegment ---*/

/* creates a new segment structure */

Hsegment mw_new_hsegment(void)
{
     Hsegment segment;

     if(!(segment = (Hsegment) (malloc(sizeof(struct hsegment)))))
     {
	  mwerror(ERROR, 0, "[mw_new_hsegment] Not enough memory\n");
	  return(NULL);
     }
     segment->xstart = segment->xend = segment->y = 0;
     segment->previous = segment->next = NULL;
     return(segment);
}

/* Define the struct if it's not defined */

Hsegment mw_change_hsegment(Hsegment segment)
{
     if (segment == NULL) segment = mw_new_hsegment();
     return(segment);
}

/* desallocate all the segment structures from a starting segment */

void mw_delete_hsegment(Hsegment segment)
{   
     Hsegment segment_next,segment_first;

     if (segment == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_hsegment] cannot delete : segment structure is NULL\n");
	  return;
     }

     segment_first=segment;
     do
     {
	  segment_next = segment->next;
	  free(segment);
	  segment = segment_next;
     } while ((segment != NULL)&&(segment != segment_first));
}

/*--- Morpho_set ---*/

/* Creates a new morpho_set structure with empty area */

Morpho_set mw_new_morpho_set(void)
{
     Morpho_set morpho_set;

     if(!(morpho_set = (Morpho_set) (malloc(sizeof(struct morpho_set)))))
     {
	  mwerror(ERROR, 0, "[mw_new_morpho_set] Not enough memory\n");
	  return(NULL);
     }
     morpho_set->num = 0;
     morpho_set->first_segment = morpho_set->last_segment = NULL;
     morpho_set->minvalue = morpho_set->maxvalue = 0.0;
     morpho_set->stated = 0;
     morpho_set->area = 0;
     morpho_set->neighbor = NULL;
     return(morpho_set);
}

/* Define the struct if it's not defined */

Morpho_set mw_change_morpho_set(Morpho_set is)
{
     if (is == NULL) is = mw_new_morpho_set();
     return(is);
}

/* desallocate the morpho_set structure but not its neighbors */

void mw_delete_morpho_set(Morpho_set morpho_set)
{
     if (morpho_set == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_morpho_set] cannot delete : morpho_set structure is NULL\n");
	  return;
     }

     if (morpho_set->first_segment) mw_delete_hsegment(morpho_set->first_segment);
     morpho_set->first_segment = morpho_set->last_segment = NULL;
     morpho_set->neighbor = NULL;
     free(morpho_set);
     morpho_set=NULL;
}

/* Copy a morpho_set into another morpho_set */

Morpho_set mw_copy_morpho_set(Morpho_set in, Morpho_set out)
{ 
     Hsegment pc,qc0,qc1;
     Hsegment s,s0,s1;

     if (!in)
     {
	  mwerror(ERROR, 0,"[mw_copy_morpho_set] NULL input morpho_set\n");
	  return(NULL);
     }
  
     if (!out)
     {
	  out=mw_new_morpho_set();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_morpho_set] Not enough memory to create a morpho_set !\n");
	       return(NULL);
	  }
     }

     out->minvalue = in->minvalue;
     out->maxvalue = in->maxvalue;
     out->stated = in->stated;
     out->area = in->area;

     /* Copy segments */
     s0 = s1 = NULL;
     for (s=in->first_segment; s; s=s->next)
     {
	  s1 = mw_new_hsegment();
	  if (s1 == NULL)
	  {
	       mw_delete_hsegment(s1);
	       mwerror(ERROR, 0,"Not enough memory to create a morpho_set\n");
	       return(NULL);
	  }
	  s1->xstart = s->xstart;
	  s1->xend = s->xend;
	  s1->y = s->y;
	  s1->previous = s0;
	  if (s0 == NULL) out->first_segment = s1;
	  else s0->next = s1;
	  s0 = s1;
     }
     return(out);
}

/* Return the number of segments into a morpho_set */

unsigned int mw_length_morpho_set(Morpho_set morpho_set)
{ 
     unsigned int n;
     Hsegment s,sfirst;

     if ((!morpho_set) || (!morpho_set->first_segment)) return(0);

     for (s=sfirst=morpho_set->first_segment, n=0; 
	  (s != NULL)&&(s->next != sfirst); n++, s=s->next);

     return(n);
}

/*--- Morpho_sets ---*/

/* Creates a new morpho_sets structure */

Morpho_sets mw_new_morpho_sets(void)
{
     Morpho_sets morpho_sets;

     if(!(morpho_sets = (Morpho_sets) (malloc(sizeof(struct morpho_sets)))))
     {
	  mwerror(ERROR, 0, "[mw_new_morpho_sets] Not enough memory\n");
	  return(NULL);
     }

     morpho_sets->morphoset = NULL;
     morpho_sets->next = morpho_sets->previous = NULL;
     morpho_sets->morpholine = NULL;
     return(morpho_sets);
}

/* Define the struct if it's not defined */

Morpho_sets mw_change_morpho_sets(Morpho_sets is)
{
     if (is == NULL) is = mw_new_morpho_sets();
     return(is);
}

/* desallocate the morpho_sets structure from the given starting point */

void mw_delete_morpho_sets(Morpho_sets morpho_sets)
{
     Morpho_sets is,next_is;

     if (morpho_sets == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_morpho_sets] cannot delete : morpho_sets structure is NULL\n");
	  return;
     }

     is=morpho_sets;
     while (is)
     {
	  mw_delete_morpho_set(is->morphoset);
	  next_is = is->next;
	  is->previous = is->next = NULL;
	  free(is);
	  is = next_is;
     }
}

/* Copy a morpho_sets into another morpho_sets */

Morpho_sets mw_copy_morpho_sets(Morpho_sets in, Morpho_sets out)
{ 
     Morpho_sets iss,oldiss,newiss,p,q,qq,nin,nout;
     unsigned int i,n,num;

     if (!in)
     {
	  mwerror(ERROR, 0,"[mw_copy_morpho_sets] NULL input morpho_sets\n");
	  return(NULL);
     }

     if (!out)
     {
	  out=mw_new_morpho_sets();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_morpho_sets] Not enough memory to create a morpho_sets !\n");
	       return(NULL);
	  }
     }

     /* First, copy the Morpho set */
     newiss = out;
     oldiss = NULL;
     for (p=in; p && p->morphoset; p=p->next)
     {
	  if (newiss != out) newiss = mw_new_morpho_sets();
	  if (!newiss)
	  {
	       mwerror(ERROR, 0,"Not enough memory to create a morpho_sets\n");
	       return(NULL);
	  }     
	  if (!newiss->morphoset) newiss->morphoset = mw_new_morpho_set();
	  if (!newiss->morphoset) 
	  {
	       mwerror(ERROR, 0,"Not enough memory to create a morpho_sets\n");
	       return(NULL);
	  }     
	  mw_copy_morpho_set(p->morphoset, newiss->morphoset);      
	  newiss->previous = oldiss;
	  if (oldiss) oldiss->next = newiss;
	  oldiss=newiss;
	  newiss = NULL;
     }

     /* Compute the Morpho set numbers of in and out */
     for (p=in, q=out, n=1; p&&q; 
	  p->morphoset->num=n, q->morphoset->num=n, p=p->next, q=q->next, n++); 

     /* Copy the neighbor morpho sets */
     for (p=in, q=out; p&&q; p=p->next, q=q->next) 
	  for (nin = p->morphoset->neighbor; nin; nin=nin->next)
	  {  
	       oldiss=newiss=NULL;
	       n = nin->morphoset->num;
	       for (qq=out; qq && qq->morphoset && (qq->morphoset->num != n); qq=qq->next);
	       if (!qq || !(qq->morphoset)) 
	       {
		    mwerror(ERROR, 0,"Cannot copy morpho sets : unconsistent neighbor list in input\n");
		    return(NULL);
	       }
	       if ((newiss=mw_new_morpho_sets())==NULL)
	       {
		    mwerror(ERROR,0,"Not enough memory to create a morpho_sets\n");	  
		    return(NULL);
	       }
	       newiss->morphoset = qq->morphoset;
	       if (q->morphoset->neighbor == NULL) q->morphoset->neighbor=newiss;
	       if (oldiss) oldiss->next = newiss;
	       newiss->previous = oldiss;
	       oldiss = newiss;
	  }
     return(out);
}

/* Return the number of morpho sets into a morpho_sets */

unsigned int mw_length_morpho_sets(Morpho_sets morpho_sets)
{ 
     unsigned int n;
     Morpho_sets s,sfirst;

     if ((!morpho_sets) || (!morpho_sets->morphoset)) return(0);

     for (s=sfirst=morpho_sets, n=0; 
	  s && s->morphoset && (s->next != sfirst); n++, s=s->next);

     return(n);
}

/* Compute the num field of a morpho_sets chain                      */
/* Return the number of morpho_sets                                    */

unsigned int mw_num_morpho_sets(Morpho_sets mss_first)
{ 
     Morpho_sets mss;
     unsigned int n;


     if (!mss_first)
     {
	  mwerror(ERROR, 0,
		  "[mw_num_morpho_sets] NULL input morpho_sets\n");
	  return;
     }  
     for (mss=mss_first, n=1; mss && mss->morphoset; 
	  mss->morphoset->num=n, mss=mss->next, n++);
     if (mss && !mss->morphoset) 
	  mwerror(ERROR, 0,"Morphosets chain cut due to the morphosets #%d which has no morphoset field\n",n);
     return(n-1);
}

/* Clear the stated flag of all the morpho_sets             */

void mw_morpho_sets_clear_stated(Morpho_sets mss_first)
{ 
     Morpho_sets mss;


     if (!mss_first)
     {
	  mwerror(ERROR, 0,
		  "[mw_morpho_sets_clear_stated] NULL input morpho_sets\n");
	  return;
     }  
     for (mss=mss_first; mss && mss->morphoset; mss->morphoset->stated=0, mss=mss->next);

}

/*-- Mimage --*/

/* creates a new mimage structure */

Mimage mw_new_mimage(void)
{
     Mimage mimage;

     if(!(mimage = (Mimage) (malloc(sizeof(struct mimage)))))
     {
	  mwerror(ERROR, 0, "[mw_new_mimage] Not enough memory\n");
	  return(NULL);
     }
     mimage->first_ml = NULL;
     mimage->first_fml = NULL;
     mimage->first_ms = NULL;
     mimage->minvalue = mimage->maxvalue = 0.0;
     mimage->nrow = mimage->ncol = 0;
     strcpy(mimage->cmt,"?");
     strcpy(mimage->name,"?");
     return(mimage);
}

/* Define the struct if it's not defined */

Mimage mw_change_mimage(Mimage mi)
{
     if (mi == NULL) mi = mw_new_mimage();
     return(mi);
}


/* desallocate the mimage structure */

void mw_delete_mimage(Mimage mimage)
{
     Morpho_line ll, ll_next;
     Fmorpho_line fll, fll_next;

     if (mimage == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_mimage] cannot delete : mimage structure is NULL\n");
	  return;
     }

     if ((ll = mimage->first_ml))
	  while (ll != NULL)
	  {
	       ll_next = ll->next;
	       mw_delete_morpho_line(ll);
	       ll = ll_next;
	  }

     if ((fll = mimage->first_fml))
	  while (fll != NULL)
	  {
	       fll_next = fll->next;
	       mw_delete_fmorpho_line(fll);
	       fll = fll_next;
	  }

     if (mimage->first_ms) mw_delete_morpho_sets(mimage->first_ms);

     free(mimage);
     mimage=NULL;
}

/* Copy a mimage into another one */

Mimage mw_copy_mimage(Mimage in, Mimage out)
{ 
  
     Morpho_line ml,mlc0,mlc1;
     Fmorpho_line fml,fmlc0,fmlc1;
     Morpho_sets ms,ms0,ms1;

     if (!in)
     {
	  mwerror(ERROR, 0,"[mw_copy_mimage] NULL input mimage !\n");
	  return(NULL);
     }

     if (!out)
     {
	  out=mw_new_mimage();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_mimage] Not enough memory to create a mimage !\n");
	       return(NULL);
	  }
     }

     strcpy(out->cmt,in->cmt);
     strcpy(out->name,in->name);
  
     out->nrow = in->nrow;
     out->ncol = in->ncol;
     out->minvalue = in->minvalue;
     out->maxvalue = in->maxvalue;
  
     /* Copy morpho lines */
     for (mlc0=NULL, ml=in->first_ml; ml; ml=ml->next)
     {
	  mlc1=mw_new_morpho_line();
	  if (!mlc1)
	  {
	       mwerror(ERROR, 0,"Not enough memory to create a mimage\n");  
	       mw_delete_mimage(out);
	       return(NULL);
	  }
	  mlc1->previous=mlc0;
	  if (mlc0) mlc0->next=mlc1;
	  else out->first_ml=mlc1;
	  mw_copy_morpho_line(ml,mlc1);
	  mlc0=mlc1;
     }

     /* Copy fmorpho lines */
     for (fmlc0=NULL, fml=in->first_fml; fml; fml=fml->next)
     {
	  fmlc1=mw_new_fmorpho_line();
	  if (!fmlc1)
	  {
	       mwerror(ERROR, 0,"Not enough memory to create a mimage\n");    
	       mw_delete_mimage(out);
	       return(NULL);
	  }
	  fmlc1->previous=fmlc0;
	  if (fmlc0) fmlc0->next=fmlc1;
	  else out->first_fml=fmlc1;
	  mw_copy_fmorpho_line(fml,fmlc1);
	  fmlc0=fmlc1;
     }

     /* Copy morpho sets */
     for (ms0=NULL, ms=in->first_ms; ms; ms=ms->next)
     {
	  ms1=mw_new_morpho_sets();
	  if (!ms1)
	  {
	       mwerror(ERROR, 0,"Not enough memory to create a mimage\n");    
	       mw_delete_mimage(out);
	       return(NULL);
	  }
	  ms1->previous=ms0;
	  if (ms0) ms0->next=ms1;
	  else out->first_ms=ms1;
	  mw_copy_morpho_sets(ms,ms1);
	  ms0=ms1;
     }
     return(out);
}

/* Return the number of morpho lines into a mimage */

unsigned int mw_length_ml_mimage(Mimage mimage)
{ 
     unsigned int n;
     Morpho_line pfirst,p;

     if ((!mimage) || (!mimage->first_ml)) return(0);

     for (p=pfirst=mimage->first_ml, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);
     return(n);
}

/* Return the number of fmorpho lines into a mimage */

unsigned int mw_length_fml_mimage(Mimage mimage)
{ 
     unsigned int n;
     Fmorpho_line pfirst,p;

     if ((!mimage) || (!mimage->first_fml)) return(0);

     for (p=pfirst=mimage->first_fml, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);
     return(n);
}

/* Return the number of morpho sets into a mimage */

unsigned int mw_length_ms_mimage(Mimage mimage)
{ 
     unsigned int n;
     Fmorpho_line pfirst,p;

     if ((!mimage) || (!mimage->first_ms)) return(0);
     return(mw_length_morpho_sets(mimage->first_ms));
}

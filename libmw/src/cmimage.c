/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  cmimage.c
   
  Vers. 1.3
  Author : Jacques Froment
  Basic memory routines for the cmorpho_line, cmorpho_set & cmimage internal types

  Main changes :
  v1.3 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdlib.h>
#include <string.h>

#include "libmw-defs.h"
#include "utils.h"
#include "curve.h"
#include "fcurve.h"
#include "mimage.h"

#include "cmimage.h"

/*--- Cmorpho_line ---*/

/* Creates a new cmorpho_line structure */

Cmorpho_line mw_new_cmorpho_line(void)
{
     Cmorpho_line cmorpho_line;

     if(!(cmorpho_line = (Cmorpho_line) (malloc(sizeof(struct cmorpho_line)))))
     {
	  mwerror(ERROR, 0, "[mw_new_cmorpho_line] Not enough memory\n");
	  return(NULL);
     }
  
     cmorpho_line->first_point = NULL;
     cmorpho_line->first_type = NULL;
     cmorpho_line->minvalue.model = cmorpho_line->maxvalue.model = MODEL_RGB;
     cmorpho_line->minvalue.red=cmorpho_line->minvalue.green=cmorpho_line->minvalue.blue=0.0;
     cmorpho_line->open = 0;
     cmorpho_line->data = 0.0;
     cmorpho_line->pdata = NULL;
     cmorpho_line->previous = NULL;
     cmorpho_line->next = NULL;
     cmorpho_line->num = 0;
     cmorpho_line->cmorphosets = NULL;
     return(cmorpho_line);
}

/* Define the struct if it's not defined */

Cmorpho_line mw_change_cmorpho_line(Cmorpho_line ll)
{
     if (ll == NULL) ll = mw_new_cmorpho_line();
     return(ll);
}

/* desallocate the cmorpho_line structure */

void mw_delete_cmorpho_line(Cmorpho_line cmorpho_line)
{
     if (cmorpho_line == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_cmorpho_line] cannot delete : cmorpho_line structure is NULL\n");
	  return;
     }

     if (cmorpho_line->first_point) mw_delete_point_curve(cmorpho_line->first_point);
     cmorpho_line->first_point = NULL;
     if (cmorpho_line->first_type) mw_delete_point_type(cmorpho_line->first_type);
     cmorpho_line->first_type = NULL;

     free(cmorpho_line);
     cmorpho_line=NULL;
}

/* Copy a cmorpho_line into another cmorpho_line */

Cmorpho_line mw_copy_cmorpho_line(Cmorpho_line in, Cmorpho_line out)
{ 
     if ((!in) || (!out))
     {
	  mwerror(ERROR, 0,"[mw_copy_cmorpho_line] NULL input cmorpho_line\n");
	  return(NULL);
     }

     if (!out)
     {
	  out=mw_new_cmorpho_line();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_cmorpho_line] Not enough memory to create a cmorpho_line !\n");
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
	  mwerror(ERROR, 0,"Not enough memory to create a cmorpho_line\n");
	  return(NULL);
     }
     mw_copy_point_curve(in->first_point,out->first_point);
     mw_copy_point_type(in->first_type,out->first_type);
     return(out);
}

/* Return the number of points into a cmorpho_line */

unsigned int mw_length_cmorpho_line(Cmorpho_line cmorpho_line)
{ 
     unsigned int n,m;
     Point_curve p,pfirst;
     Point_type t,tfirst;

     if ((!cmorpho_line) || (!cmorpho_line->first_point)) return(0);

     for (p=pfirst=cmorpho_line->first_point, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);

     if (!cmorpho_line->first_type)
     {
	  for (t=tfirst=cmorpho_line->first_type, m=0; 
	       (t != NULL)&&(t->next != tfirst); m++, t=t->next);

	  if ( (n*m != 0) && (n != m) )
	       mwerror(INTERNAL,1,"[mw_length_cmorpho_line] Inconsistent Cmorpho_line structure \n");
     }
     return(n);
}

/* Compute the num field of a cmorpho_line chain */
/* Return the number of cmorpho_lines             */

unsigned int mw_num_cmorpho_line(Cmorpho_line ml_first)
{ 
     Cmorpho_line ml;
     unsigned int n;

     if (!ml_first)
     {
	  mwerror(ERROR, 0,
		  "[mw_num_cmorpho_line] NULL input cmorpho_line\n");
	  return(0);
     }  
     for (ml=ml_first, n=1; ml; ml->num=n, ml=ml->next, n++);
     return(n-1);
}

/*--- Cfmorpho_line ---*/

/* Creates a new cfmorpho_line structure */

Cfmorpho_line mw_new_cfmorpho_line(void)
{
     Cfmorpho_line cfmorpho_line;

     if(!(cfmorpho_line = (Cfmorpho_line) (malloc(sizeof(struct cfmorpho_line)))))
     {
	  mwerror(ERROR, 0, "[mw_new_cfmorpho_line] Not enough memory\n");
	  return(NULL);
     }
  
     cfmorpho_line->first_point = NULL;
     cfmorpho_line->first_type = NULL;
     cfmorpho_line->minvalue.model = cfmorpho_line->maxvalue.model = MODEL_RGB;
     cfmorpho_line->minvalue.red=cfmorpho_line->minvalue.green=cfmorpho_line->minvalue.blue=0.0;
     cfmorpho_line->open = 0;
     cfmorpho_line->data = 0.0;
     cfmorpho_line->pdata = NULL;
     cfmorpho_line->previous = NULL;
     cfmorpho_line->next = NULL;
     return(cfmorpho_line);
}

/* Define the struct if it's not defined */

Cfmorpho_line mw_change_cfmorpho_line(Cfmorpho_line ll)
{
     if (ll == NULL) ll = mw_new_cfmorpho_line();
     return(ll);
}

/* desallocate the cfmorpho_line structure */

void mw_delete_cfmorpho_line(Cfmorpho_line cfmorpho_line)
{
     if (cfmorpho_line == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_cfmorpho_line] cannot delete : cfmorpho_line structure is NULL\n");
	  return;
     }

     if (cfmorpho_line->first_point) mw_delete_point_fcurve(cfmorpho_line->first_point);
     cfmorpho_line->first_point = NULL;
     if (cfmorpho_line->first_type) mw_delete_point_type(cfmorpho_line->first_type);
     cfmorpho_line->first_type = NULL;

     free(cfmorpho_line);
     cfmorpho_line=NULL;
}

/* Copy a cfmorpho_line into another cfmorpho_line */

Cfmorpho_line mw_copy_cfmorpho_line(Cfmorpho_line in, Cfmorpho_line out)
{ 
     Point_fcurve pc,qc0,qc1;
     Point_type pt,qt0,qt1;

     if (!in) 
     {
	  mwerror(ERROR, 0,"[mw_copy_cfmorpho_line] NULL input cfmorpho_line\n");
	  return(NULL);
     }

     if (!out)
     {
	  out=mw_new_cfmorpho_line();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_cfmorpho_line] Not enough memory to create a cfmorpho_line !\n");
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
	       mwerror(ERROR, 0,"Not enough memory to create a cfmorpho_line\n");
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
	       mwerror(ERROR, 0,"Not enough memory to create a cfmorpho_line\n");
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

/* Return the number of points into a cfmorpho_line */

unsigned int mw_length_cfmorpho_line(Cfmorpho_line cfmorpho_line)
{ 
     unsigned int n,m;
     Point_fcurve p,pfirst;
     Point_type t,tfirst;

     if ((!cfmorpho_line) || (!cfmorpho_line->first_point)) return(0);

     for (p=pfirst=cfmorpho_line->first_point, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);

     if (!cfmorpho_line->first_type)
     {
	  for (t=tfirst=cfmorpho_line->first_type, m=0; 
	       (t != NULL)&&(t->next != tfirst); m++, t=t->next);

	  if ( (n*m != 0) && (n != m) )
	       mwerror(INTERNAL,1,"[mw_length_cfmorpho_line] Inconsistent Cfmorpho_line structure \n");
     }
     return(n);
}

/*--- Cmorpho_set ---*/

/* Creates a new cmorpho_set structure with empty area */

Cmorpho_set mw_new_cmorpho_set(void)
{
     Cmorpho_set cmorpho_set;

     if(!(cmorpho_set = (Cmorpho_set) (malloc(sizeof(struct cmorpho_set)))))
     {
	  mwerror(ERROR, 0, "[mw_new_cmorpho_set] Not enough memory\n");
	  return(NULL);
     }
     cmorpho_set->num = 0;
     cmorpho_set->first_segment = cmorpho_set->last_segment = NULL;
     cmorpho_set->minvalue.model = cmorpho_set->maxvalue.model = MODEL_RGB;
     cmorpho_set->minvalue.red=cmorpho_set->minvalue.green=cmorpho_set->minvalue.blue=0.0;
     cmorpho_set->stated = 0;
     cmorpho_set->area = 0;
     cmorpho_set->neighbor = NULL;
     return(cmorpho_set);
}

/* Define the struct if it's not defined */

Cmorpho_set mw_change_cmorpho_set(Cmorpho_set is)
{
     if (is == NULL) is = mw_new_cmorpho_set();
     return(is);
}

/* desallocate the cmorpho_set structure but not its neighbors */

void mw_delete_cmorpho_set(Cmorpho_set cmorpho_set)
{
     if (cmorpho_set == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_cmorpho_set] cannot delete : cmorpho_set structure is NULL\n");
	  return;
     }

     if (cmorpho_set->first_segment) mw_delete_hsegment(cmorpho_set->first_segment);
     cmorpho_set->first_segment = cmorpho_set->last_segment = NULL;
     cmorpho_set->neighbor = NULL;
     free(cmorpho_set);
     cmorpho_set=NULL;
}

/* Copy a cmorpho_set into another cmorpho_set */

Cmorpho_set mw_copy_cmorpho_set(Cmorpho_set in, Cmorpho_set out)
{ 
     Hsegment s,s0,s1;

     if (!in)
     {
	  mwerror(ERROR, 0,"[mw_copy_cmorpho_set] NULL input cmorpho_set\n");
	  return(NULL);
     }

     if (!out)
     {
	  out=mw_new_cmorpho_set();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_cmorpho_set] Not enough memory to create a cmorpho_set !\n");
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
	       mwerror(ERROR, 0,"Not enough memory to create a cmorpho_set\n");
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

/* Return the number of segments into a cmorpho_set */

unsigned int mw_length_cmorpho_set(Cmorpho_set cmorpho_set)
{ 
     unsigned int n;
     Hsegment s,sfirst;

     if ((!cmorpho_set) || (!cmorpho_set->first_segment)) return(0);

     for (s=sfirst=cmorpho_set->first_segment, n=0; 
	  (s != NULL)&&(s->next != sfirst); n++, s=s->next);

     return(n);
}

/*--- Cmorpho_sets ---*/

/* Creates a new cmorpho_sets structure */

Cmorpho_sets mw_new_cmorpho_sets(void)
{
     Cmorpho_sets cmorpho_sets;

     if(!(cmorpho_sets = (Cmorpho_sets) (malloc(sizeof(struct cmorpho_sets)))))
     {
	  mwerror(ERROR, 0, "[mw_new_cmorpho_sets] Not enough memory\n");
	  return(NULL);
     }

     cmorpho_sets->cmorphoset = NULL;
     cmorpho_sets->next = cmorpho_sets->previous = NULL;
     cmorpho_sets->cmorpholine = NULL;
     return(cmorpho_sets);
}

/* Define the struct if it's not defined */

Cmorpho_sets mw_change_cmorpho_sets(Cmorpho_sets is)
{
     if (is == NULL) is = mw_new_cmorpho_sets();
     return(is);
}

/* desallocate the cmorpho_sets structure from the given starting point */

void mw_delete_cmorpho_sets(Cmorpho_sets cmorpho_sets)
{
     Cmorpho_sets is,next_is;

     if (cmorpho_sets == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_cmorpho_sets] cannot delete : cmorpho_sets structure is NULL\n");
	  return;
     }

     is=cmorpho_sets;
     while (is)
     {
	  mw_delete_cmorpho_set(is->cmorphoset);
	  next_is = is->next;
	  is->previous = is->next = NULL;
	  free(is);
	  is = next_is;
     }
}

/* Copy a cmorpho_sets into another cmorpho_sets from the given starting point */

Cmorpho_sets mw_copy_cmorpho_sets(Cmorpho_sets in, Cmorpho_sets out)
{ 
     Cmorpho_sets oldiss,newiss,p,q,qq,nin;
     unsigned int n;

     if (!in)
     {
	  mwerror(ERROR, 0,"[mw_copy_cmorpho_sets] NULL input cmorpho_sets\n");
	  return(NULL);
     }
     if (!out)
     {
	  out=mw_new_cmorpho_sets();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_cmorpho_sets] Not enough memory to create a cmorpho_sets !\n");
	       return(NULL);
	  }
     }

     /* First, copy the Cmorpho set */
     newiss = out;
     oldiss = NULL;
     for (p=in; p && p->cmorphoset; p=p->next)
     {
	  if (newiss != out) newiss = mw_new_cmorpho_sets();
	  if (!newiss)
	  {
	       mwerror(ERROR, 0,"Not enough memory to create a cmorpho_sets\n");
	       return(NULL);
	  }     
	  if (!newiss->cmorphoset) newiss->cmorphoset = mw_new_cmorpho_set();
	  if (!newiss->cmorphoset) 
	  {
	       mwerror(ERROR, 0,"Not enough memory to create a cmorpho_sets\n");
	       return(NULL);
	  }     
	  mw_copy_cmorpho_set(p->cmorphoset, newiss->cmorphoset);      
	  newiss->previous = oldiss;
	  if (oldiss) oldiss->next = newiss;
	  oldiss=newiss;
	  newiss = NULL;
     }

     /* Compute the Cmorpho set numbers of in and out */
     for (p=in, q=out, n=1; p&&q; 
	  p->cmorphoset->num=n, q->cmorphoset->num=n, p=p->next, q=q->next, n++); 

     /* Copy the neighbor cmorpho sets */
     for (p=in, q=out; p&&q; p=p->next, q=q->next) 
	  for (nin = p->cmorphoset->neighbor; nin; nin=nin->next)
	  {  
	       oldiss=newiss=NULL;
	       n = nin->cmorphoset->num;
	       for (qq=out; qq && qq->cmorphoset && (qq->cmorphoset->num != n); qq=qq->next);
	       if (!qq || !(qq->cmorphoset)) 
	       {
		    mwerror(ERROR, 0,"Cannot copy cmorpho sets : unconsistent neighbor list in input\n");
		    return(NULL);
	       }
	       if ((newiss=mw_new_cmorpho_sets())==NULL)
	       {
		    mwerror(ERROR, 0,"Not enough memory to create a cmorpho_sets\n");	  
		    return(NULL);
	       }
	       newiss->cmorphoset = qq->cmorphoset;
	       if (q->cmorphoset->neighbor == NULL) q->cmorphoset->neighbor=newiss;
	       if (oldiss) oldiss->next = newiss;
	       newiss->previous = oldiss;
	       oldiss = newiss;
	  }
     return(out);
}

/* Return the number of cmorpho sets into a cmorpho_sets */

unsigned int mw_length_cmorpho_sets(Cmorpho_sets cmorpho_sets)
{ 
     unsigned int n;
     Cmorpho_sets s,sfirst;

     if ((!cmorpho_sets) || (!cmorpho_sets->cmorphoset)) return(0);

     for (s=sfirst=cmorpho_sets, n=0; 
	  s && s->cmorphoset && (s->next != sfirst); n++, s=s->next);

     return(n);
}

/* Compute the num field of a cmorpho_sets chain                      */
/* Return the number of cmorpho_sets                                    */

unsigned int mw_num_cmorpho_sets(Cmorpho_sets mss_first)
{ 
     Cmorpho_sets mss;
     unsigned int n;


     if (!mss_first)
     {
	  mwerror(ERROR, 0,
		  "[mw_num_cmorpho_sets] NULL input cmorpho_sets\n");
	  return -1;
     }  
     for (mss=mss_first, n=1; mss && mss->cmorphoset; 
	  mss->cmorphoset->num=n, mss=mss->next, n++);
     if (mss && !mss->cmorphoset) 
	  mwerror(ERROR, 0,"Cmorphosets chain cut due to the cmorphosets #%d which has no cmorphoset field\n",n);
     return(n-1);
}

/* Clear the stated flag of all the cmorpho_sets             */

void mw_cmorpho_sets_clear_stated(Cmorpho_sets mss_first)
{ 
     Cmorpho_sets mss;


     if (!mss_first)
     {
	  mwerror(ERROR, 0,
		  "[mw_cmorpho_sets_clear_stated] NULL input cmorpho_sets\n");
	  return;
     }  
     for (mss=mss_first; mss && mss->cmorphoset; mss->cmorphoset->stated=0, mss=mss->next);

}

/*-- Cmimage --*/

/* creates a new cmimage structure */

Cmimage mw_new_cmimage(void)
{
     Cmimage cmimage;

     if(!(cmimage = (Cmimage) (malloc(sizeof(struct cmimage)))))
     {
	  mwerror(ERROR, 0, "[mw_new_cmimage] Not enough memory\n");
	  return(NULL);
     }
     cmimage->first_ml = NULL;
     cmimage->first_fml = NULL;
     cmimage->first_ms = NULL;
     cmimage->minvalue.model = cmimage->maxvalue.model = MODEL_RGB;
     cmimage->minvalue.red=cmimage->minvalue.green=cmimage->minvalue.blue=0.0;
     cmimage->nrow = cmimage->ncol = 0;
     strcpy(cmimage->cmt,"?");
     strcpy(cmimage->name,"?");
     return(cmimage);
}

/* Define the struct if it's not defined */

Cmimage mw_change_cmimage(Cmimage mi)
{
     if (mi == NULL) mi = mw_new_cmimage();
     return(mi);
}


/* desallocate the cmimage structure */

void mw_delete_cmimage(Cmimage cmimage)
{
     Cmorpho_line ll, ll_next;
     Cfmorpho_line fll, fll_next;

     if (cmimage == NULL)
     {
	  mwerror(ERROR, 0,
		  "[mw_delete_cmimage] cannot delete : cmimage structure is NULL\n");
	  return;
     }

     if ((ll = cmimage->first_ml))
	  while (ll != NULL)
	  {
	       ll_next = ll->next;
	       mw_delete_cmorpho_line(ll);
	       ll = ll_next;
	  }

     if ((fll = cmimage->first_fml))
	  while (fll != NULL)
	  {
	       fll_next = fll->next;
	       mw_delete_cfmorpho_line(fll);
	       fll = fll_next;
	  }

     if (cmimage->first_ms) mw_delete_cmorpho_sets(cmimage->first_ms);

     free(cmimage);
     cmimage=NULL;
}

/* Copy a cmimage into another one */

Cmimage mw_copy_cmimage(Cmimage in, Cmimage out)
{ 
     if (!in) 
     {
	  mwerror(ERROR, 0,"[mw_copy_cmimage] NULL input cmimage\n");
	  return(NULL);
     }

     if (!out)
     {
	  out=mw_new_cmimage();
	  if (!out) 
	  {
	       mwerror(ERROR,0,"[mw_copy_cmimage] Not enough memory to create a cmimage !\n");
	       return(NULL);
	  }
     }

     strcpy(out->cmt,in->cmt);
     strcpy(out->name,in->name);
  
     out->nrow = in->nrow;
     out->ncol = in->ncol;
     out->minvalue = in->minvalue;
     if (in->first_ml)
     {
	  out->first_ml = mw_new_cmorpho_line();
	  if (!out->first_ml)
	  {
	       mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");    
	       return(NULL);
	  }
	  mw_copy_cmorpho_line(in->first_ml,out->first_ml);
     }
     if (in->first_fml)
     {
	  out->first_fml = mw_new_cfmorpho_line();
	  if (!out->first_fml)
	  {
	       mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");    
	       return(NULL);
	  }
	  mw_copy_cfmorpho_line(in->first_fml,out->first_fml);
     }
     if (in->first_ms)
     {
	  out->first_ms = mw_new_cmorpho_sets();
	  if (!out->first_ms)
	  {
	       mwerror(ERROR, 0,"Not enough memory to create a cmimage\n");    
	       return(NULL);
	  }
	  mw_copy_cmorpho_sets(in->first_ms,out->first_ms);
     }
     return(out);
}

/* Return the number of cmorpho lines into a cmimage */

unsigned int mw_length_ml_cmimage(Cmimage cmimage)
{ 
     unsigned int n;
     Cmorpho_line pfirst,p;

     if ((!cmimage) || (!cmimage->first_ml)) return(0);

     for (p=pfirst=cmimage->first_ml, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);
     return(n);
}

/* Return the number of cfmorpho lines into a cmimage */

unsigned int mw_length_fml_cmimage(Cmimage cmimage)
{ 
     unsigned int n;
     Cfmorpho_line pfirst,p;

     if ((!cmimage) || (!cmimage->first_fml)) return(0);

     for (p=pfirst=cmimage->first_fml, n=0; 
	  (p != NULL)&&(p->next != pfirst); n++, p=p->next);
     return(n);
}

/* Return the number of cmorpho sets into a cmimage */

unsigned int mw_length_ms_cmimage(Cmimage cmimage)
{ 
     if ((!cmimage) || (!cmimage->first_ms)) return(0);
     return(mw_length_cmorpho_sets(cmimage->first_ms));
}

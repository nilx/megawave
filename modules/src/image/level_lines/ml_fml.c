/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {ml_fml};
 version = {"1.5"};
 author = {"Georges Koepfler"};
 function = {"Transform morpho_lines into fmorpho_lines with a (-0.5,-0.5) translation"};
 usage = {
    morpho_line_in->lline     "input morpho_line",
    fmorpho_line_out<-ml_fml  "output fmorpho_line"
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mw.h"
#include "mw-modules.h"


Fmorpho_line ml_fml(Morpho_line lline)
{ 
  Fmorpho_line  flline=NULL,old_flline,first_flline=NULL;
  Point_fcurve fpoint;
  Point_curve point;
  Point_type type,ftype;
  int nb_points,nb_types;

  if(lline==NULL)
      mwerror(FATAL,1,"No integer level lines in input.");

  old_flline=NULL;
  while(lline!=NULL) {
    for(point=lline->first_point,nb_points=0;point!=NULL;point=point->next) 
      nb_points++;
    for(type=lline->first_type,nb_types=0;type!=NULL;type=type->next) 
      nb_types++;
    if(nb_types!=nb_points) 
      mwerror(FATAL,1,"Points / types mismatch");
    fpoint=(Point_fcurve)malloc(nb_points*sizeof(struct point_fcurve));
    ftype=(Point_type)malloc(nb_types*sizeof(struct point_type));
    if(!(fpoint&&ftype))
      mwerror(FATAL,1,"Not enough memory.");
    flline=mw_change_fmorpho_line(NULL);
    flline->minvalue=lline->minvalue;    
    flline->maxvalue=lline->maxvalue;    
    flline->open=lline->open;
    flline->first_point=fpoint;
    flline->first_type=ftype;
    flline->previous=old_flline;
    flline->next=NULL;
    for(point=lline->first_point,type=lline->first_type;
	point!=NULL;
	point=point->next,type=type->next) {
      fpoint->x=point->x-.5;
      fpoint->y=point->y-.5;
      fpoint->previous=(point->previous==NULL)? NULL:fpoint-1;
      fpoint->next=(point->next==NULL)? NULL:fpoint+1;
      fpoint++;
      ftype->type=type->type;
      ftype->previous=(type->previous==NULL)? NULL : ftype-1;
      ftype->next=(type->next==NULL)? NULL:ftype+1;
      ftype++;
    }
    if(old_flline==NULL) 
      first_flline=flline;
          else 
      old_flline->next=flline;
    old_flline=flline;
    lline=lline->next;
  }

  return(first_flline);
}









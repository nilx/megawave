/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {sr_genhypo};
 version = {"1.2"};
 author = {"Thierry Cohignac, Lionel Moisan"};
 function = {"Produce hypotheses for shape recognition"};
 usage = {
    'o':hypo_list<-hypo_list   "hypotheses (Fimage nx1)",
    sg -> sg                   "input signature (Fimage)",
    img -> img                 "input image of the shape to recognize",
    out <- sr_genhypo	       "normalized Shape"
};
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mw.h"
#include "mw-modules.h" /* for extract_connex(), sr_normalize(),
			 * sr_signature() */


/*** to interpret a Cimage ***/
#define GREY  128

/*** number of hypotheses to produce ***/
#define NB_HYPO 10          


#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define ABS(x)   ((x)>0?(x):(-(x)))

static Fimage compute_distance(Fimage sg, Fimage param)
{
  Fimage dist;
  int    i,j;
  int    nx,ny;
  float  val,diff;
  
  nx = sg->ncol;
  ny = sg->nrow;
  
  dist = mw_change_fimage(NULL,1,ny);
  
  for(j=0;j<ny;j++){
    val = 0.0;
    for(i=0;i<nx;i++) {
      diff = sg->gray[nx*j+i] - param->gray[i];
      val += ABS(diff);
    }
    dist->gray[j] = val;
  }
  
  return(dist);
}



static int comparefloat(const void *v1, const void *v2)
{
  double a,b;

  a = (double)*(const float*)v1;
  a -= floor((double)*(const float*)v1);
  b = (double)*(const float*)v2;
  b -= floor((double)*(const float*)v2);
  
  return a>b?1:(a<b ? -1 : 0);
}


static Fimage sort_hypo(Fimage hypo_list, Fimage dist, int nb_hypo)
{
  float  vmax;
  int    dx,i;
  
  dx = dist->ncol;
  vmax = 0.0;
  for(i=0;i<dx;i++)
    if (dist->gray[i]>vmax) vmax = dist->gray[i];
  
  vmax += 1.0;
  for(i=0;i<dx;i++) 
    dist->gray[i] /= vmax;

  for(i=0;i<dx;i++) dist->gray[i] += (float)(i+1);
  
  qsort((float *)dist->gray,dx,sizeof(float),comparefloat);
  
  hypo_list = mw_change_fimage(hypo_list,1,nb_hypo);
  for(i=nb_hypo;i--; ) 
    hypo_list->gray[i]=(float)floor((double)dist->gray[i]);

  return hypo_list;
  
}

/* MAIN MODULE */

Fcurves sr_genhypo(Fimage sg, Cimage img, Fimage hypo_list)
{
  Fcurves  c0,fn;
  int      i,ASCII_flag,thre,nb_hypo;
  Fimage   param,distance;
  char     *name,*reponse;
  
  ASCII_flag = (hypo_list==NULL);
  
  name = (char *)malloc(sizeof(float)+2);
  reponse = (char *)malloc((sizeof(float)+10)*NB_HYPO);
  
  thre = GREY;
  c0 = extract_connex(img,NULL,&thre);
  
  fn = sr_normalize(c0);
  
  nb_hypo = MIN(NB_HYPO,sg->nrow);
  param = sr_signature(fn,NULL,NULL);
  distance = compute_distance(sg,param);
  hypo_list = sort_hypo(hypo_list,distance,nb_hypo);
  
  mw_delete_fcurves(c0);
  mw_delete_fimage(param);
  mw_delete_fimage(distance);
  
  for(i=0;i<nb_hypo;i++){
    sprintf(name,"%d",(unsigned int)hypo_list->gray[i]);
    strcat(reponse,name);
    strcat(reponse," ");
  }
  
  if (ASCII_flag) {
    fprintf(stdout,"%s \n",reponse);
    mw_delete_fimage(hypo_list);
  }
  
  free(name); 

  return fn;
}


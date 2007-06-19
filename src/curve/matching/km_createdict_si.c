/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {km_createdict_si};
version = {"1.1"};
author = {"Jose-Luis Lisani, Pablo Muse, Frederic Sur"};
function = {"Encode a list of curves into an similitude-invariant dictionnary"};
usage = {
 'F':[FNorm=2.0]->FNorm   "length factor",
 'N':[NNorm=9]->NNorm     "number of points per code (odd number)",
 list_curves->list_curves "input list of curves (Flists)",
 dict<-dict               "output dictionary (Flists)"
        };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include<math.h>
#include<time.h>
#include "mw.h"  

extern Flist km_inflexionpoints();
extern Flist km_flatpoints();
extern Flist km_bitangents();
extern Flists km_codecurve_si();


#define _(a,i,j) ((a)->values[(i)*((a)->dim)+(j)])
/* if a is a Flist including elements constituted of dim components
   then _(a,i,j) is j-th component of i-th element */

struct NormDataSIconcat { 
  int Numcurve_in_llconcat;
  int Numimage;
  int Numcurve; 
  int i_left, i_right; 
  float xC, yC; 
  int iL1, iL2; 
  float xR1, yR1, xR2, yR2; 
  float disc; 
}; 


/*------------------------------ MAIN MODULE ------------------------------*/

void km_createdict_si(FNorm,NNorm,list_curves,dict)
     float *FNorm;
     int *NNorm;
     Flists list_curves;
     Flists dict; 
{ 
  Flist curve, curveIP, curveFP, curveBP;
  Flists dictaux;
  int j,i,t1,t2,num_curves;
  float angle, dist;

  t1=clock();
  num_curves=list_curves->size;
  angle=0.2; dist=15.0;

  if (2*(*NNorm/2)==*NNorm) mwerror(FATAL,1,"error, NNorm is not odd\n");
  if ((dict=mw_change_flists(dict,num_curves*50,0))==NULL) 
    mwerror (FATAL,1,"error, not enough memory\n");

  /* for each curve in list_curves, compute the IP FP, and BP, 
     then add the codes to the dictionary */

  for (i=0;i<num_curves;i++) { 
    if ((curveIP=mw_new_flist())==NULL) 
      mwerror (FATAL,1,"error, not enough memory\n");
    if ((curveFP=mw_new_flist())==NULL) 
      mwerror (FATAL,1,"error, not enough memory\n");
    if ((curveBP=mw_new_flist())==NULL) 
      mwerror (FATAL,1,"error, not enough memory\n");
    if ((dictaux=mw_new_flists())==NULL) 
      mwerror (FATAL,1,"error, not enough memory\n");
    if (list_curves->list[i]->size==0) continue;

    km_inflexionpoints(list_curves->list[i],curveIP); 
    km_flatpoints(list_curves->list[i],curveIP,curveFP,angle,dist); 
    km_bitangents(list_curves->list[i],curveIP,curveBP); 

    mwdebug("%d %d %d %d\n",i,curveIP->size,curveFP->size,curveBP->size);

    km_codecurve_si(list_curves->list[i],curveIP,curveFP,curveBP,dictaux,i,*NNorm,*FNorm);   

    for (j=0;j<dictaux->size;j++) {
      dict->list[dict->size]=mw_copy_flist(dictaux->list[j],NULL);
      dict->size++;
      if (dict->size==dict->max_size) mw_enlarge_flists(dict);           
    }
    mw_delete_flists(dictaux);
    mw_delete_flist(curveIP);
    mw_delete_flist(curveFP);
    mw_delete_flist(curveBP);
  }
  mw_realloc_flists(dict,dict->size);
  dict->data_size=sizeof(int);
  dict->data=(void*)(NNorm);
  t2=clock();
  mwdebug("number of words in the dictionary: %d\n",dict->size);
  mwdebug("elapsed time: %f\n",(float)(t2-t1)/CLOCKS_PER_SEC);

}

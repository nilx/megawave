/*----------------------------MegaWave2 module----------------------------*/
/* mwcommand
name = {km_prematchings};
version = {"1.0"};
author = {"Jose Luis Lisani, Pablo Muse, Frederic Sur"};
function = {"Get pre-matching codes of two dictionaries"};
usage = {
   maxError->maxError     "maximum allowed Hausdorff distance between two codes to be considered as pre-matching",
   dict1->dict1           "dictionary codes of image 1 (Flists)",
   dict2->dict2           "dictionary codes of image 2 (Flists)",
   matchings<-matchings   "output Flists: for each code of dict1, the indices of the dict2 codes which are closer than maxError in Hausdorff distance"
        };
*/

#include<math.h>
#include "mw.h" 


#define qnorm(a,b) ((a)*(a)+(b)*(b))

#define _(a,i,j) ((a)->values[(i)*((a)->dim)+(j)])
/* if a is a Flist including elements constituted of dim components
   then _(a,i,j) is j-th component of i-th element */


/* Given a point (X,Y) and a dictionary DICT, this function saves 
   in LIST_MATCHINGS all the indices of the dict codes whose j-th point 
   euclidean distance to (x,y) is less than maxErr. 
   The function returns the number of such codes.*/

unsigned char search_value(x,y,dict,maxErr,list_matchings,j)
     float x,y;
     Flists dict;
     float maxErr;
     Flist list_matchings;
     int j;
{
  int i;

  for (i=0;i<dict->size;i++)
    if ((fabs(x-_(dict->list[i],j,0))<maxErr)
	&&(fabs(y-_(dict->list[i],j,1))<maxErr)) 
      _(list_matchings,list_matchings->size++,0)=i;

  return (list_matchings->size>0);
}
  
/* Given a code LISTE={L_i, 0 <= i <= Nnorm}, this function returns a
   list MATCHAUX which contains all the DICT codes' indices whose Nnorm
   points P_i(X_i,Y_i) satisfy : eucl-dist(P_i,L_i) < maxErr 
   for all i in {0,...,Nnorm-1} */

void matchings_code(liste,dict,maxErr,Nnorm,matchaux)
     Flist liste;
     Flists dict;
     float maxErr;
     int Nnorm;
     Flist matchaux;
{
  int i, n, m;
  unsigned char ok;
  Flist list_matchings, matchs_counter;

  if ((list_matchings=mw_change_flist(NULL,dict->size,0,1))==NULL) 
    mwerror (FATAL,1,"error, not enough memory\n");
  if ((matchs_counter=mw_change_flist(NULL,dict->size,dict->size,1))==NULL) 
    mwerror (FATAL,1,"error, not enough memory\n");
  mw_clear_flist(matchs_counter,0);
  i=0;
  do {
    ok=search_value(_(liste,i,0),_(liste,i,1), dict, maxErr,list_matchings,i);
    if (ok) {
      for (n=0; n < list_matchings->size; n++) {
        m=_(list_matchings,n,0);
        _(matchs_counter,m,0)++;
      }
    }
    i++;
    mw_delete_flist(list_matchings);
    if ((list_matchings=mw_change_flist(NULL,dict->size,0,1))==NULL) 
      mwerror (FATAL,1,"error, not enough memory\n");
  } while ((i < Nnorm) && ok); 

  /* the search ends if the i-th point of LISTE has no i-th point 
     in any DICT code closer than maxError, or if all the Nnorm LISTE's 
     points have been treated.*/
  
  /* Only the codes whose Nnorm points are closer than maxError from its 
     corresponding Nnorm LISTE points are kept */

  for (m=0;m<matchs_counter->size;m++) {
    if (_(matchs_counter,m,0) == Nnorm)
      _(matchaux,matchaux->size++,0)=m;
  }
  mw_delete_flist(list_matchings);
  mw_delete_flist(matchs_counter);
}



/*------------------------------ MAIN MODULE ------------------------------*/

Flists km_prematchings(maxError,dict1,dict2,matchings)
     float maxError;
     Flists dict1,dict2,matchings; 
{
  int k;
  int Nnorm;
  Flist matchaux;

  if(maxError < 0.0) 
    mwerror(FATAL,1,"invalid argument type : maxError must be non-negative\n");
  if(dict1->size == 0) mwerror(FATAL,1,"error : Flists dict1 is empty\n");
  if(dict2->size == 0) mwerror(FATAL,1,"error : Flists dict2 is empty\n");
  if(dict1->list[0]->size != dict2->list[0]->size)
    mwerror(FATAL,1,"Codes in dict1 and dict2 must have the same number of points\n");
 
  if ((matchings=mw_change_flists(matchings,dict1->size,dict1->size))==NULL) 
    mwerror (FATAL,1,"error, not enough memory\n");
  if ((matchaux=mw_change_flist(NULL,dict2->size,0,1))==NULL) 
    mwerror (FATAL,1,"error, not enough memory\n");
  Nnorm=dict1->list[0]->size;
 
  /* for each code in dict1, save a list of indices that 
     correspond to the codes in dict2 that are closer than maxError */ 

  for (k=0;k<dict1->size;k++) {
     matchings_code(dict1->list[k],dict2,maxError,Nnorm,matchaux);
     matchings->list[k]=mw_copy_flist(matchaux,NULL);
     mw_delete_flist(matchaux);
    if ((matchaux=mw_change_flist(NULL,dict2->size,0,1))==NULL) 
      mwerror (FATAL,1,"error, not enough memory\n");
  }
  
  return(matchings);
}


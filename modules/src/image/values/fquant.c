/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {fquant};
 author = {"Jacques Froment"};
 version = {"1.2"};
 function = {"Uniform quantization of a fimage"};
 usage = {
   'l'->left          "Set left value of the interval instead of middle value",
   'm':minimum->min  "force the minimum (don't compute it)",
   'M':maximum->max  "force the maximum (don't compute it)",
   A->A              "Input fimage",
   Q<-Q              "Output quantized fimage",
   M->M              "Number of quantized levels",
   delta<-fquant     "Width step of the uniform quantization used"
};
*/
/*----------------------------------------------------------------------
 v1.2: added -m and -M options (S.Ladjal)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include  "mw.h"

float fquant(Fimage A, Fimage Q, int M, char *left, float *min, float *max)
{
  register float *ptrA,*ptrQ;
  register int i,r;
  float Max,Min,D2,v,w,w2,delta;
  
  if (M <= 0) mwerror(USAGE,1,"Bad number of quantized levels %d\n",M);
  
  if (min && max) {
    Min=*min; 
    Max=*max;
  } else {
    Min = 1e30; Max = -Min;
    for (i=0,ptrA=A->gray; i<A->ncol*A->nrow; i++,ptrA++)
      {
	if (*ptrA > Max) Max=*ptrA;
	if (*ptrA < Min) Min=*ptrA;
      }
  }

  Q = mw_change_fimage(Q, A->nrow, A->ncol);
  if (Q == NULL) mwerror(FATAL,1,"Not enough memory.\n");
  
  delta = (Max - Min) / M; D2 = delta / 2.0;

  if (left) /* value in [ v , w [ is set to v */
    for (r=0; r< M; r++)
      {
	v = Min + r * delta;
	if (r < M-1) w = v + delta;
	else w = Max;                 /* Needed because of computation errors */
	mwdebug("Set values in [%f,%f] to %f\n",v,w,v);
	if (r!=0 && r!=M-1)
	  for (i=0,ptrA=A->gray,ptrQ=Q->gray; i<A->ncol*A->nrow; i++,ptrA++,ptrQ++)
	    if ((*ptrA >= v) && (*ptrA <= w)) *ptrQ = v;
	if (r==0)
	  for (i=0,ptrA=A->gray,ptrQ=Q->gray; i<A->ncol*A->nrow; i++,ptrA++,ptrQ++)
	    if (*ptrA <= w) *ptrQ = v;
	if (r==M-1)
	  for (i=0,ptrA=A->gray,ptrQ=Q->gray; i<A->ncol*A->nrow; i++,ptrA++,ptrQ++)
	    if (*ptrA >= v) *ptrQ = v;
	  

      }
  else /* value in [ v , w [ is set to v + (w-v)/2 */
    for (r=0; r< M; r++)
      {
	v = Min + r * delta;
	if (r < M-1) w = v + delta;
	else w = Max;                 /* Needed because of computation errors */
	w2 = v + D2;
	mwdebug("Set values in [%f,%f] to %f\n",v,w,v);
	if (r!=0 && r!=M-1)
	  for (i=0,ptrA=A->gray,ptrQ=Q->gray; i<A->ncol*A->nrow; i++,ptrA++,ptrQ++)
	    if ((*ptrA >= v) && (*ptrA <= w)) *ptrQ = w2;
	if (r==0)
	  for (i=0,ptrA=A->gray,ptrQ=Q->gray; i<A->ncol*A->nrow; i++,ptrA++,ptrQ++)
	    if (*ptrA <= w) *ptrQ = w2;
	if (r==M-1)
	  for (i=0,ptrA=A->gray,ptrQ=Q->gray; i<A->ncol*A->nrow; i++,ptrA++,ptrQ++)
	    if (*ptrA >= v) *ptrQ = w2;
      }
  mwdebug("delta=%g\n",delta);
  return(delta);
}



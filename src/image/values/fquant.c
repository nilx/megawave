/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {fquant};
author = {"Jacques Froment"};
function = {"Uniform quantization of a fimage"};
usage = {
'l'->left "Set left value of the interval instead of middle value",
A->A "Input fimage",
Q<-Q "Output quantized fimage",
M->M "Number of quantized levels",
delta<-fquant "Width step of the uniform quantization used"
};
version = {"1.1"};
*/

#include <stdio.h>
#include <math.h>

#include  "mw.h"

float fquant(A,Q,M,left)

Fimage A,Q;
int M;
char *left;

{
  register float *ptrA,*ptrQ;
  register int i,r;
  float Max,Min,D2,v,w,w2,delta;

  if (M <= 0) mwerror(USAGE,1,"Bad number of quantized levels %d\n",M);

  Min = 1e30; Max = -Min;
  Q = mw_change_fimage(Q, A->nrow, A->ncol);
  if (Q == NULL) mwerror(FATAL,1,"Not enough memory.\n");

  for (i=0,ptrA=A->gray; i<A->ncol*A->nrow; i++,ptrA++)
    {
      if (*ptrA > Max) Max=*ptrA;
      if (*ptrA < Min) Min=*ptrA;
    }

  delta = (Max - Min) / M; D2 = delta / 2.0;

  if (left) /* value in [ v , w [ is set to v */
    for (r=0; r< M; r++)
      {
	v = Min + r * delta;
	if (r < M-1) w = v + delta;
	else w = Max;                 /* Needed because of computation errors */
	mwdebug("Set values in [%f,%f] to %f\n",v,w,v);
	for (i=0,ptrA=A->gray,ptrQ=Q->gray; i<A->ncol*A->nrow; i++,ptrA++,ptrQ++)
	  if ((*ptrA >= v) && (*ptrA <= w)) *ptrQ = v;
      }
  else /* value in [ v , w [ is set to v + (w-v)/2 */
    for (r=0; r< M; r++)
      {
	v = Min + r * delta;
	if (r < M-1) w = v + delta;
	else w = Max;                 /* Needed because of computation errors */
	w2 = v + D2;
	mwdebug("Set values in [%f,%f] to %f\n",v,w,v);
	for (i=0,ptrA=A->gray,ptrQ=Q->gray; i<A->ncol*A->nrow; i++,ptrA++,ptrQ++)
	  if ((*ptrA >= v) && (*ptrA <= w)) *ptrQ = w2;
      }
  mwdebug("delta=%g\n",delta);
  return(delta);
}



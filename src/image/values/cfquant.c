/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
name = {cfquant};
author = {"Jacques Froment"};
function = {"Uniform quantization of a color float image"};
usage = {
'l'->left "Set left value of the interval instead of middle value",
A->A "Input cfimage",
Q<-Q "Output quantized cfimage",
Mr->Mr "Number of quantized levels for the red channel",
Mg->Mg "Number of quantized levels for the green channel",
Mb->Mb "Number of quantized levels for the blue channel"
};
version = {"1.0"};
*/

#include <stdio.h>
#include <math.h>

#include  "mw.h"

#ifdef __STDC__
extern float fquant(Fimage,Fimage,int,char *);
#else
extern float fquant();
#endif

void cfquant(A,Q,Mr,Mg,Mb,left)

Cfimage A,Q;
int Mr,Mg,Mb;
char *left;

{
  Fimage channel=NULL,Qr,Qg,Qb;
  float *ptrA,*ptr;
  int i;
  
  /*
  Q = mw_change_cfimage(Q, A->nrow, A->ncol);
  if (Q == NULL) mwerror(FATAL,1,"Not enough memory.\n");
  */
  channel = mw_change_fimage(channel, A->nrow, A->ncol);
  if (channel == NULL) mwerror(FATAL, 1, "Not enough memory.\n");

  /* Red channel */
  Qr = mw_new_fimage();
  if (Qr == NULL) mwerror(FATAL, 1, "Not enough memory.\n");  

  for (i=0, ptrA=A->red, ptr=channel->gray; i< A->nrow*A->ncol; i++, ptrA++, ptr++)
    *ptr = *ptrA;
  fquant(channel,Qr,Mr,left);

  /* Green channel */
  Qg = mw_new_fimage();
  if (Qg == NULL) mwerror(FATAL, 1, "Not enough memory.\n");  

  for (i=0, ptrA=A->green, ptr=channel->gray; i< A->nrow*A->ncol; i++, ptrA++, ptr++)
    *ptr = *ptrA;
  fquant(channel,Qg,Mg,left);

  /* Blue channel */
  Qb = mw_new_fimage();
  if (Qb == NULL) mwerror(FATAL, 1, "Not enough memory.\n");  

  for (i=0, ptrA=A->blue, ptr=channel->gray; i< A->nrow*A->ncol; i++, ptrA++, ptr++)
    *ptr = *ptrA;
  fquant(channel,Qb,Mb,left);

  Q->red=Qr->gray;
  Q->green=Qg->gray;
  Q->blue=Qb->gray;
  Q->nrow = A->nrow;
  Q->ncol = A->ncol;
  Q->allocsize = A->allocsize;
  Q->model = A->model;
}



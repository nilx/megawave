/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
  name = {ridgthres};
  version = {"1.0"};
  author = {"Claire Jonchery, Amandine Robin"};
  function ={"Image denoising by means of ridgelet thresholding"};
  usage = {
  'I':in_im->in_im        "imaginary input (Fimage N*N)",
  'C':output_im<-out_im   "imaginary output (Fimage N*N)",
  in_re->in_re            "real input (Fimage N*N)",
  np->np                  "resolution np",
  pourcent->pourcent "percent of conserved coeff",
  out_re<-out_re          "result (Fimage N*N)"
};
*/
/*----------------------------------------------------------------------
 v1.0: initial revision before publication (J.Froment)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "mw.h"

extern Fsignal fvalues();
extern void ridgelet();
extern void iridgelet();

void ridgthres(in_re,in_im,np,out_re,out_im,pourcent)
  int np,pourcent;
  Fimage in_re,in_im;
  Fimage out_re,out_im;

{
  int N,i,s,somme=0;
  float seuil_im,seuil_re;
  Fimage rid_re=NULL,rid_im=NULL,abs_rid_re=NULL,abs_rid_im=NULL,rank=NULL;
  Fsignal rid_re_ord=NULL,rid_im_ord=NULL,mult=NULL;
  char c;
  
  N=(int)(in_re->nrow);

  out_re=mw_change_fimage(out_re,N,N);
  if (!out_re) mwerror(FATAL,1,"not enough memory !\n");

  out_im=mw_change_fimage(out_im,N,N);
  if (!out_im) mwerror(FATAL,1,"not enough memory !\n");

  rid_re=mw_change_fimage(rid_re,2*N,2*N);
  if (!rid_re) mwerror(FATAL,1,"not enough memory !\n");

  rid_im=mw_change_fimage(rid_im,2*N,2*N);
  if (!rid_im) mwerror(FATAL,1,"not enough memory !\n");

  abs_rid_re=mw_change_fimage(abs_rid_re,2*N,2*N);
  if (!abs_rid_re) mwerror(FATAL,1,"not enough memory !\n");

  abs_rid_im=mw_change_fimage(abs_rid_im,2*N,2*N);
  if (!rid_im) mwerror(FATAL,1,"not enough memory !\n");

  rid_re_ord=mw_change_fsignal(rid_re_ord,4*N*N+1);
  if (!rid_re_ord) mwerror(FATAL,1,"not enough memory !\n");

  rid_im_ord=mw_change_fsignal(rid_im_ord,4*N*N+1);
  if (!rid_im_ord) mwerror(FATAL,1,"not enough memory !\n");

  mult=mw_change_fsignal(mult,4*N*N+1);
  if (!mult) mwerror(FATAL,1,"not enough memory !\n");

  rank=mw_change_fimage(rank,2*N,2*N);
  if (!rank) mwerror(FATAL,1,"not enough memory !\n");

  ridgelet(in_re,in_im,np,rid_re,rid_im);

  for(i=0;i<4*N*N;i++)
    abs_rid_re->gray[i]=fabs(rid_re->gray[i]);
    
    /* thresholding */
    /* real part */
  rid_re_ord=fvalues(&c,mult,rank,abs_rid_re);
  s=(int)(pourcent*4*N*N/100);
  i=0;
  while((somme <s)&&(i<mult->size))
    {
      somme+=(mult->values[i]);
      i++;
    }
  seuil_re=(rid_re_ord->values[i]);
 
  for(i=0;i<(4*N*N);i++)
    if((fabs(( rid_re->gray[i])))<seuil_re) 
      (rid_re->gray[i])=0;

  /* imaginary part */
  for(i=0;i<4*N*N;i++)
    abs_rid_im->gray[i]=fabs(rid_im->gray[i]);

  rid_im_ord=fvalues(&c,mult,rank,abs_rid_im);

  somme=0;
  i=0;
  while(somme < s)
    {
      somme+=(mult->values[i]);
      i++;
    }	       
  seuil_im=(rid_im_ord->values[i]);

  for(i=0;i<(4*N*N);i++)
    if( (fabs((rid_im->gray[i])))<seuil_im) 
      (rid_im->gray[i])=0;

  iridgelet(rid_re,rid_im,np,out_re,out_im);

  mw_delete_fimage(rid_re);
  mw_delete_fimage(rid_im);
  mw_delete_fimage(abs_rid_re);
  mw_delete_fimage(abs_rid_im);
  mw_delete_fimage(rank);
  mw_delete_fsignal(rid_re_ord);
  mw_delete_fsignal(rid_im_ord);
  mw_delete_fsignal(mult);
}


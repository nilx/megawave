/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {faxpb};
 version = {"1.3"};
 author = {"Lionel Moisan"};
 function = {"Gain/Offset correction to a Fimage (a x plus b)"};
 usage = {            
    'a':a->a   "set a directly",
    's':s->s   "set a indirectly by selecting the output standart deviation s",
    'M':M->M   "set a indirectly (and b=0) by selecting total mass M",
    'N':N->N   "set a indirectly (and b=0) by selecting L2 norm N",
    'b':b->b   "set b directly",
    'm':m->m   "set b indirectly by selecting the output mean m",
    'k'->k     "set b indirectly by keeping input mean",
    in->in     "input Fimage",
    out<-out   "output Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1: added -M option (L.Moisan)
 v1.2: added -N option and upgraded fvar() call (L.Moisan)
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"


extern float fmean(), fvar();

/*** NB: Calling this module with out=in is possible ***/

void faxpb(in,out,a,s,b,m,k,M,N)
     Fimage in,out;
     float *a,*s,*b,*m,*M,*N;
     char *k;
{
  double sum;
  float gain,ofs1,ofs2,*pin,*pout;
  int   i,adr;
  
  if (M) {

    if ((s?1:0) + (a?1:0) + (m?1:0) + (b?1:0) + (k?1:0) + (N?1:0) > 0) 
      mwerror(USAGE,1,"please use -M option alone");
    ofs1 = ofs2 = 0.;
    gain = *M/(fmean(in)*(float)in->ncol*(float)in->nrow);

  } else if (N) {

    if ((s?1:0) + (a?1:0) + (m?1:0) + (b?1:0) + (k?1:0) + (M?1:0) > 0) 
      mwerror(USAGE,1,"please use -N option alone");
    ofs1 = ofs2 = 0.;
    for (sum=0.,adr=in->ncol*in->nrow;adr--;) 
      sum += (double)(in->gray[adr]*in->gray[adr]);
    gain = *N/(float)sqrt(sum);

  } else {

    if (s && a) 
      mwerror(USAGE,1,"-a and -s options cannot be used together");
    if ((m?1:0) + (b?1:0) + (k?1:0) > 1) 
      mwerror(USAGE,1,"please select no more than one of -b -m -k options");
    
    if (s) 
      gain = *s / fvar(in,1,1);
    else gain = (a?*a:1.0);
    
    if (m) {
      ofs1 = fmean(in);
      ofs2 = *m;
    } else if (k) {
      ofs1 = fmean(in);
      ofs2 = ofs1;
    } else {
      ofs1 = 0.0;
      ofs2 = (b?*b:0.0);
    }
  }

  mwdebug("a = %f\n",gain);
  mwdebug("b = %f\n",ofs2-gain*ofs1);

  out = mw_change_fimage(out,in->nrow,in->ncol);
  if (!out) mwerror(FATAL,1,"Not enough memory.");

  for (i=in->nrow*in->ncol,pin=in->gray,pout=out->gray; i-- ; pin++,pout++) 
    *pout = gain * (*pin - ofs1) + ofs2;  
}

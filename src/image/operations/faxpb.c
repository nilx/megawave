/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {faxpb};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Gain/Offset correction to a Fimage (a x plus b)"};
   usage = {            
    'a':a->a   "set a directly (default: 1.0)",
    's':s->s   "set a indirectly by selecting the output standart deviation s",
    'b':b->b   "set b directly (default: 0.0)",
    'm':m->m   "set b indirectly by selecting the output mean m",
    'k'->k     "set b indirectly by keeping input mean",
    in->in     "input Fimage",
    out<-out   "output Fimage"
   };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"


extern float fmean(), fvar();

/*** NB: Calling this module with out=in is possible ***/

void faxpb(in,out,a,s,b,m,k)
Fimage in,out;
float *a,*s,*b,*m;
char *k;
{
  float gain,ofs1,ofs2,*pin,*pout;
  int   i;

  if (s && a) 
    mwerror(USAGE,1,"please select no more than one of the -a and -s options");
  if ((m?1:0) + (b?1:0) + (k?1:0) > 1) 
    mwerror(USAGE,1,"please select no more than one of the -b -m -k options");
  
  if (s) 
    gain = *s / (float)sqrt((double)fvar(in));
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

  mwdebug("a = %f\n",gain);
  mwdebug("b = %f\n",ofs2-gain*ofs1);

  out = mw_change_fimage(out,in->nrow,in->ncol);
  if (!out) mwerror(FATAL,1,"Not enough memory.");

  for (i=in->nrow*in->ncol,pin=in->gray,pout=out->gray; i-- ; pin++,pout++) 
    *pout = gain * (*pin - ofs1) + ofs2;  
}

/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {saxpb};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Gain/Offset correction to a Fsignal (a x plus b)"};
   usage = {            
    'a':a->a   "set a directly (default: 1.0)",
    's':s->s   "set a indirectly by selecting the output standart deviation s",
    'b':b->b   "set b directly (default: 0.0)",
    'm':m->m   "set b indirectly by selecting the output mean m",
    'k'->k     "set b indirectly by keeping input mean",
    in->in     "input Fsignal",
    out<-out   "output Fsignal"
   };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"

/*** NB: Calling this module with out=in is possible ***/

void saxpb(in,out,a,s,b,m,k)
     Fsignal in,out;
     float *a,*s,*b,*m;
     char *k;
{
  double mean,var;
  float gain,ofs1,ofs2,*pin,*pout;
  int   i;

  if (s && a) 
    mwerror(USAGE,1,"please select no more than one of the -a and -s options");
  if ((m?1:0) + (b?1:0) + (k?1:0) > 1) 
    mwerror(USAGE,1,"please select no more than one of the -b -m -k options");
  
  mean = var = 0.;
  for (i=in->size;i--;) mean += (double)in->values[i];
  mean /= (double)in->size;
  
  if (s) {
    var = 0.;
    for (i=in->size;i--;) 
      var += ((double)in->values[i]-mean)*((double)in->values[i]-mean);
    var /= (double)in->size;
    gain = *s / (float)sqrt(var);
  } else gain = (a?*a:1.0);

  if (m) {
    ofs1 = (float)mean;
    ofs2 = *m;
  } else if (k) {
    ofs1 = (float)mean;
    ofs2 = ofs1;
  } else {
    ofs1 = 0.0;
    ofs2 = (b?*b:0.0);
  }

  mwdebug("a = %f\n",gain);
  mwdebug("b = %f\n",ofs2-gain*ofs1);

  out = mw_change_fsignal(out,in->size);
  if (!out) mwerror(FATAL,1,"Not enough memory.");

  for (i=in->size,pin=in->values,pout=out->values; i-- ; pin++,pout++) 
    *pout = gain * (*pin - ofs1) + ofs2;  
}

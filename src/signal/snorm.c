/*----------------------------- MegaWave Module -----------------------------*/
/* mwcommand
  name = {snorm};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Compute the norm of a Fsignal"};
  usage = {
    'p':p->p      "compute average L^p norm",
    's'->s        "compute L^infinity norm (sup of absolute values)",
    'v'->v        "compute average total variation",
    'b':[b=0]->b  "number of samples to crop on the borders (default: 0)",
    'c':ref->ref  "compare with Fsignal ref (ie compute ||in - ref||)",
    'n'->n        "normalized comparison (with -c), ie ||in - ref||/ ||ref||)",
    't':t->t      "force result to 0 if less or equal than threshold t",
    in->in        "input Fsignal",
    out<-snorm    "computed norm"
    };
*/
#include <stdio.h>
#include <math.h>
#include "mw.h"

float snorm(in,ref,p,s,v,b,n,t)
Fsignal in,ref;
float *p;
char *s,*v;
int *b;
char *n;
float *t;
{
  int x,y,num;
  double sum,diff,val;

  /* check options */
  if ((p?1:0) + (s?1:0) + (v?1:0) != 1)
    mwerror(USAGE,1,"Please use exactly one of the -p / -s / -v options");
  if (n && !ref) 
    mwerror(USAGE,1,"Option -n does not make sense without option -c");
  if (ref) {
    /* check size compatibility */
    if (in->size!=ref->size)
      mwerror(FATAL,1,"both signals must have the same sizes");
  }

  /* MAIN LOOP */
  sum = 0.0; num = 0;
  for (x=*b;x<in->size-(*b);x++) {
    if (p || s) {
      val = (double)in->values[x];
      if (ref) val-= (double)ref->values[x];
      if (val<0) val=-val;
      if (p) sum += pow(val,(double)(*p));
      else if (val>sum) sum=val;
    } else if (v && x!=*b) {
      diff = in->values[x]-in->values[x-1];
      sum += (diff<0.?-diff:diff);
    }
    num++;
  }

  /* normalize and threshold result if needed */
  if (!s) {
    if (num) sum /= (double)num; else sum=0.0;
  }
  if (n) sum /= (double)snorm(ref,NULL,p,s,v,b,NULL);
  if (t) if (sum<=*t) sum=0.0;
  if (p) sum = pow(sum,1.0/(double)(*p));

  return ((float)sum);
}




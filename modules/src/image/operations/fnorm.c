/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {fnorm};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"Compute the norm of a Fimage"};
  usage = {
    'p':p->p      "compute average L^p norm",
    's'->s        "compute L^infinity norm (sup of absolute values)",
    'v'->v        "compute average total variation",
    'b':[b=0]->b  "number of lines to crop on the borders",
    'c':ref->ref  "compare with Fimage ref (ie compute ||in - ref||)",
    'n'->n        "normalized comparison (with -c), ie ||in - ref||/ ||ref||)",
    't':t->t      "force result to 0 if less or equal than threshold t",
    in->in        "input Fimage",
    out<-fnorm    "computed norm"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* fop(), fderiv() */

float fnorm(Fimage in, Fimage ref, float *p, char *s, char *v, int *b, char *n, float *t)
{
  Fimage diff=NULL, gradn=NULL;
  int x,y,num,four;
  float zero;
  double sum,val;

  /* check options */
  if ((p?1:0) + (s?1:0) + (v?1:0) != 1)
    mwerror(USAGE,1,"Please use exactly one of the -p / -s / -v options");
  if (n && !ref) 
    mwerror(USAGE,1,"Option -n does not make sense without option -c");
  if (ref) {
    /* check size compatibility */
    if (in->ncol!=ref->ncol || in->nrow!=ref->nrow)
      mwerror(FATAL,1,"both images must have the same sizes");
  }

  /* compute gradient norm for total variation */
  if (v) {
    gradn = mw_new_fimage();
    zero = 0.0; four = 4;
    if (ref) {
      diff = mw_new_fimage();
      /* gradn = || D(in-ref) || */ 
      fop(ref,diff,in,
	NULL,NULL,(char *)1,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
      fderiv(diff,NULL,NULL,NULL,NULL,NULL,NULL,gradn,NULL,&zero,&four);
    } 
    else 
      /* gradn = || D(in) || */ 
      fderiv( in ,NULL,NULL,NULL,NULL,NULL,NULL,gradn,NULL,&zero,&four);
  }

  /* MAIN LOOP */
  sum = 0.0; num = 0;
  for (x=*b;x<in->ncol-(*b);x++)
    for (y=*b;y<in->nrow-(*b);y++) {
      if (p || s) {
	val = (double)in->gray[y*in->ncol+x];
	if (ref) val-= (double)ref->gray[y*in->ncol+x];
	if (val<0) val=-val;
	if (p) sum += pow(val,(double)(*p));
	else if (val>sum) sum=val;
      } else if (v) 
	sum += gradn->gray[y*in->ncol+x];
      num++;
    }

  /* normalize and threshold result if needed */
  if (!s) {
    if (num) sum /= (double)num; else sum=0.0;
  }
  if (n) sum /= (double) fnorm(ref, NULL, p, s, v, b, NULL, NULL);
  if (t) if (sum<=*t) sum=0.0;
  if (p) sum = pow(sum,1.0/(double)(*p));

  /* free memory */
  if (v) {
    if (s) mw_delete_fimage(diff);
    mw_delete_fimage(gradn);
  }
  
  return ((float)sum);
}




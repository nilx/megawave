/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {frandphase};
   version = {"1.2"};
   author = {"Lionel Moisan"};
   function = {"Phase Randomization of a Fimage"};
   usage = {
	    'i'->i_flag  "in order NOT to reinitialize the random seed",
            in->in       "input Fimage",
            out<-out     "output Fimage"
   }; 
*/
/*----------------------------------------------------------------------
 v1.2: fmeanvar() replaced by faxpb() (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "mw.h"

#ifdef __STDC__
#include <stdlib.h>
#else
extern double  drand48();
extern void    srand48();
extern time_t  time();
#endif

extern void   fft2d();
extern float  fmean(), fvar();
extern void   faxpb();

#define SQR(x) ((x)*(x))


/* NB: Calling this module with out=in is possible */

void frandphase(in,out,i_flag)
Fimage in,out;
char   *i_flag;
{
  Fimage re,im;
  double rho,theta;
  int    x,y,n,p,ad,ymax;
  float  m,std;

  /*** Initialize random seed if necessary ***/
  if (!i_flag) srand48( (long int) time (NULL) );

  m = fmean(in);
  std = (float)sqrt((double)fvar(in));

  re = mw_new_fimage();
  im = mw_new_fimage();

  /*** FFT ***/
  fft2d(in,NULL,re,im,0);
  n = re->nrow;
  p = re->ncol;
  out = mw_change_fimage(out,n,p);

  /*** phase randomization ***/
  for (x = -n/2; x<=0; x++) {
    ymax = ((x==0)||(x==-n/2))?0:(n/2-1);
    for (y = -n/2; y<=ymax; y++) {
      ad = ((y+n)%n)*n + (x+n)%n;
      rho = sqrt( (double)( SQR(re->gray[ad]) + SQR(im->gray[ad]) ));
      if ( ((x==0)||(x==-n/2)) && ((y==0)||(y==-n/2)) ) {
	re->gray[ad] = (float)rho;
	im->gray[ad] = 0.0;
      } else {
	theta = 2.0*M_PI*drand48();
	re->gray[ad] = (float)( rho*cos( theta ) );
	im->gray[ad] = (float)( rho*sin( theta ) );
	ad = ((-y+n)%n)*n + (-x+n)%n;
	re->gray[ad] = (float)( rho*cos( theta ) );
	im->gray[ad] = (float)( -rho*sin( theta ) );
      }
    }
  }
  
  /*** inverse FFT ***/
  fft2d(re,im,out,NULL,1);

  /*** impose mean and variance ***/
  faxpb(out,out,NULL,&std,NULL,&m,NULL);

  /*** free memory ***/
  mw_delete_fimage(im);
  mw_delete_fimage(re);
}




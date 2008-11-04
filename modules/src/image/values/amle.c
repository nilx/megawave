/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {amle};
 version = {"1.3"};
 author = {"Jean-Pierre D'Ales, Jacques Froment, Catalina Sbert"};
 function = {"Level line image interpolation using the AMLE model (implicit Euler scheme)"};
 usage = {
   'i':image_init->Init                "Initial condition image",
   'n':[n=1000]->n                     "number of iterations",
   'w':[omega=1.8]->omega [0.01,1.99]  "Relaxation parameter",
   't':[ht=1.0]->ht                    "Time increment",
   's':mse->mse   "Stop if the MSE between two iterations is lower than mse",
   input->Input   "Original cimage with missing level lines",
   output<-Output "Output fimage with interpolated level lines"
};
*/
/*----------------------------------------------------------------------
 v1.3 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fmse() */

#define EPS 1e-4

/* Compute the 8 neighbour pixels of the current pixel p              */
/* When p touches the border of the image, a mirror effect is applied */

static void neighbor(x,y,xmax,ymax,p,left,right,up,down,right_down,left_up,right_up,left_down)

register int x,y,xmax,ymax;
register float *p;
float **left,**right,**up,**down,**right_down,**left_up,**right_up,**left_down;

{
  if (x>0) 
    {
      *left = p-1; 
      if (x < xmax) 
	{
	  *right = p+1; 
	  if (y>0) 
	    {
	      *up =p-xmax-1; 
	      if (y < ymax) 
		{ /* 0 < x < xmax  0 < y < ymax */
		  *down=p+xmax+1; 
		  *right_down=p+xmax+2;
		  *left_up=p-xmax-2;
		  *right_up=p-xmax;
		  *left_down=p+xmax;
		}
	      else /* 0 < x < xmax   y = ymax */
		{
		  *down=*up;
		  *right_up = *right_down= p-xmax;
		  *left_up = *left_down= p-xmax-2;
		}
	    } 
	  else /* 0 < x < xmax   y = 0 */ 
	    {
	      *down= p+xmax+1;
	      *up=*down;
	      *right_up = *right_down=p+xmax+2;
	      *left_up = *left_down=p+xmax;
	    }
	}
      else /* x = xmax */
	{
	  *right=*left;
	  if (y>0) 
	    {
	      *up=p-xmax-1; 
	      if (y < ymax) 
		{ /* x = xmax  0 < y < ymax */
		  *down=p+xmax+1; 
		  *right_down = *left_down = p+xmax;
		  *right_up = *left_up = p-xmax-2;
		}
	      else /* x = xmax   y = ymax */
		{
		  *down=*up;
		  *right_up = *left_up = *left_down = *right_down = p-xmax-2;
		}
	    }
	  else /* x = xmax  y = 0 */
	    {
	      *down=p+xmax+1;
	      *up=*down;
	      *right_up = *right_down = *left_up = *left_down= p+xmax;
	    }
	}
    }
  else /* x = 0 */
    {
      *right=p+1;
      *left=*right;
      if (y>0) 
	{
	  *up=p-xmax-1; 
	  if (y < ymax) 
	    { /* x = 0  0 < y < ymax */
	      *down=p+xmax+1; 
	      *right_down=p+xmax+2;
	      *right_up=p-xmax;
	      *left_up=*right_up;
	      *left_down=*right_down;
	      }
	  else /* x = 0   y = ymax */
	    {
	      *down=*up;
	      *right_up = *left_up = *left_down = *right_down = p-xmax;
	    }
	} 
      else /* x = 0   y = 0 */ 
	{
	  *down=p+xmax+1;
	  *up=*down;
	  *right_up = *left_up = *left_down = *right_down = p+xmax+2;
	}
    }
}

/* Compute the new gray value of the current point (x,y) by solving an implicit Euler scheme */
/* of the PDE du/dt = D2u (Du/|Du|, Du/|Du|)                                                 */

static void compute_point(x,y,xmax,ymax,p,prev_value,ht,omega)
register int x,y,xmax,ymax;
register float *p;
float prev_value,ht,omega;

  {
    float *left,*right,*up,*down,*right_down,*left_up,*right_up,*left_down;
    float ux,uy,uxx,uyy,uxy;
    double A,B,NG2;

    neighbor(x,y,xmax,ymax,p,&left,&right,&up,&down,&right_down,&left_up,&right_up,&left_down);

    ux = *right - *left;
    uy = *down - *up;
    uxx = *right + *left;
    uyy = *down + *up;
    uxy = ( *right_down + *left_up) - ( *right_up + *left_down);

    NG2 = ux*ux + uy*uy;
    if ( fabs(NG2) > EPS )
      {
	A=(2+4*ht)*NG2*(*p) - 2*prev_value*NG2 - 2*ht*uxx*ux*ux - 2*ht*uyy*uy*uy - ht*ux*uy*uxy;
	B=2*NG2*(1+2*ht)+EPS;
      }
    else
      {
	A=*p - 0.25*(*left + *right + *up + *down);
	B=omega;
      }

    *p -= (omega * A) / B;
  }


void amle(Init,Input,Output,omega,n,ht,mse)

Cimage Init;
Cimage Input;
Fimage Output;
int *n;
float *omega,*ht;
float *mse;

{
  unsigned char *ptrIn;
  float *ptrOut,*ptrPrev;
  int iter,x,y,xmax,ymax;
  Fimage Prev=NULL;			/* image at current iteration -1 */
  double msef, mrd, snr, psnr;
  float absdiff,MSE;

  xmax = Input->ncol-1;
  ymax = Input->nrow-1;

  Output = mw_change_fimage(Output,Input->nrow,Input->ncol);
  if (Output==NULL) mwerror(FATAL,1,"Not enough memory.\n");

  /*--- Copy the initial condition Cimage into the output Fimage ---*/

  if (Init) 
    {
      if ((Init->ncol == Input->ncol) && (Init->nrow == Input->nrow))
	for (x=0, ptrIn=Init->gray, ptrOut=Output->gray; x < Input->ncol*Input->nrow; x++) 
	  *(ptrOut++) = *(ptrIn++);
      else
	mwerror(WARNING, 0, "Bad size for initial condition image image_init!\n -> -i option ignored.\n");
    }

  /*--- Copy the input Cimage into the output Fimage ---*/

  for (x=0, ptrIn=Input->gray, ptrOut=Output->gray; x < Input->ncol*Input->nrow; x++, ptrOut++, ptrIn++) 
    if (*ptrIn != 0) 
      *ptrOut = *ptrIn;

  Prev = mw_change_fimage(NULL,Input->nrow,Input->ncol);
  if (Prev==NULL) mwerror(FATAL,1,"Not enough memory.\n");

  for (iter=1;iter<=*n;iter++) {

    mw_copy_fimage(Output,Prev);

    /*--- Compute the interpolation only at points not belonging ---*/
    /*--- to a level line (i.e. points with gray levels = 0) ---*/

    /*--- First pass : from left to right and up to down ---*/

    ptrIn=Input->gray;
    ptrOut=Output->gray;
    ptrPrev=Prev->gray;
    for (y=0; y<Input->nrow;y++)
      for (x=0; x<Input->ncol;x++,ptrOut++,ptrPrev++)
	if (*(ptrIn++) == 0) compute_point(x,y,xmax,ymax,ptrOut,*ptrPrev,*ht,*omega);

    /*--- Second pass : from right to left and down to up ---*/

    ptrIn--; 
    ptrOut--;
    ptrPrev--;
    for (y=Input->nrow-1; y>=0; y--)
      for (x=Input->ncol-1; x>=0; x--,ptrOut--,ptrPrev--)
	if (*(ptrIn--) == 0) compute_point(x,y,xmax,ymax,ptrOut,*ptrPrev,*ht,*omega);

    if ((iter % 1000)==0) mwdebug("%d/%d done.\n",iter,*n);

    /*--- Stop before the end if maxabsdiff <= *eps ---*/

    if (mse != NULL)
      {
	MSE=0.0;
	ptrOut=Output->gray;
	ptrPrev=Prev->gray;
	for (y=0; y<Input->nrow;y++)
	  for (x=0; x<Input->ncol;x++,ptrOut++,ptrPrev++)
	    {
	      absdiff = fabs((double) *ptrOut - *ptrPrev);
	      MSE += absdiff*absdiff;
	    } 
	MSE /= ((float) Input->nrow * Input->ncol);
	if (MSE <= *mse) 
	  {
	    mwdebug("MSE = %f : stop at iteration %d\n",MSE,iter);
	    iter=*n+1;
	  }
      }
  }

  mwdebug("Images difference between iterations %d and %d\n",*n-1,*n); 
  fmse(Prev, Output, NULL, NULL, &msef, &mrd, &snr, &psnr);

  mw_delete_fimage(Prev);
}




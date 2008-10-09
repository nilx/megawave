/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
  name = {finvspline};
  version = {"1.1"};
  author = {"Lionel Moisan"};
  function = {"2D inverse B-spline transform"};
  usage = {
     in->in               "input (Fimage)",
     order->order[2,11]   "spline order",
     out<-out             "output (Fimage of coefficients)"
          };
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include "mw.h"

double initcausal(c,n,z)
     double *c;
     int n;
     double z;
{
  double zk,z2k,iz,sum;
  int k;

  zk = z; iz = 1./z;
  z2k = pow(z,(double)n-1.);
  sum = c[0] + z2k * c[n-1];
  z2k = z2k*z2k*iz;
  for (k=1;k<=n-2;k++) {
    sum += (zk+z2k)*c[k];
    zk *= z;
    z2k *= iz;
  }
  return (sum/(1.-zk*zk));
}

double initanticausal(c,n,z)
     double *c;
     int n;
     double z;
{
  return((z/(z*z-1.))*(z*c[n-2]+c[n-1]));
}


void invspline1D(c,size,z,npoles)
     double *c;
     int size;
     double *z;
     int npoles;
{
  double lambda;
  int n,k;

  /* normalization */
  for (k=npoles,lambda=1.;k--;) lambda *= (1.-z[k])*(1.-1./z[k]);
  for (n=size;n--;) c[n] *= lambda;

  /*----- Loop on poles -----*/
  for (k=0;k<npoles;k++) {

    /* forward recursion */
    c[0] = initcausal(c,size,z[k]);
    for (n=1;n<size;n++) 
      c[n] += z[k]*c[n-1];

    /* backwards recursion */
    c[size-1] = initanticausal(c,size,z[k]);
    for (n=size-1;n--;) 
      c[n] = z[k]*(c[n+1]-c[n]);
    
  }
}


/*------------------------------ MAIN MODULE ------------------------------*/

void finvspline(in,order,out)
     Fimage in,out;
     int order;
{
  double *c,*d,z[5],tmp;
  int npoles,nx,ny,x,y;
 
  /* initialize poles of associated z-filter */
  switch (order) 
    {
    case 2: z[0]=-0.17157288;  /* sqrt(8)-3 */
      break;

    case 3: z[0]=-0.26794919;  /* sqrt(3)-2 */ 
      break;

    case 4: z[0]=-0.361341; z[1]=-0.0137254;
      break;

    case 5: z[0]=-0.430575; z[1]=-0.0430963;
      break;
      
    case 6: z[0]=-0.488295; z[1]=-0.0816793; z[2]=-0.00141415;
      break;

    case 7: z[0]=-0.53528; z[1]=-0.122555; z[2]=-0.00914869;
      break;
      
    case 8: z[0]=-0.574687; z[1]=-0.163035; z[2]=-0.0236323; z[3]=-0.000153821;
      break;

    case 9: z[0]=-0.607997; z[1]=-0.201751; z[2]=-0.0432226; z[3]=-0.00212131;
      break;
      
    case 10: z[0]=-0.636551; z[1]=-0.238183; z[2]=-0.065727; z[3]=-0.00752819;
      z[4]=-0.0000169828;
      break;
      
    case 11: z[0]=-0.661266; z[1]=-0.27218; z[2]=-0.0897596; z[3]=-0.0166696; 
      z[4]=-0.000510558;
      break;
      
     default:
      mwerror(FATAL,1,"finvspline: order should be in 2..11.\n");
    }
  npoles = order/2;

  /* initialize double array containing image */
  nx = in->ncol;
  ny = in->nrow;
  c = (double *)malloc(nx*ny*sizeof(double));
  d = (double *)malloc(nx*ny*sizeof(double));
  for (x=nx*ny;x--;) 
    c[x] = (double)in->gray[x];

  /* apply filter on lines */
  for (y=0;y<ny;y++) 
    invspline1D(c+y*nx,nx,z,npoles);

  /* transpose */
  for (x=0;x<nx;x++)
    for (y=0;y<ny;y++) 
      d[x*ny+y] = c[y*nx+x];
      
  /* apply filter on columns */
  for (x=0;x<nx;x++) 
    invspline1D(d+x*ny,ny,z,npoles);

  /* transpose directy into image */
  out = mw_change_fimage(out,ny,nx);
  for (x=0;x<nx;x++)
    for (y=0;y<ny;y++) 
      out->gray[y*nx+x] = (float)(d[x*ny+y]);

  /* free array */
  free(d);
  free(c);
}

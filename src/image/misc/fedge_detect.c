/*-------------------------MegaWave2 module---------------------------------*/
/* mwcommand
  name = {fedge_detect};
  version = {"1.01"};
  author = {"Yann Guyonvarc'h"};
  function = {"Deriche's Edge Detector : Using Canny's Criteria to Derive a Recursively Implemented Optimal Edge Detector"};
  usage = {
  'a':[alpha=1.] -> alpha  "Width of the impulse response, default 1",
   IN -> IN   "input fimage",
   OUT <- OUT "output fimage"
};
*/
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"


void gradient ( IN, gradx,grady,alpha )
Fimage IN;
Fimage gradx,grady;
float *alpha;
 
 {

#define CARRE(x)  ( (x) *(x) )

  float *a1,*a2,*a3,*a4;
  register int i,j;
  register float an11,an1,an2,an3,an4,ad1,ad2;
      
  int  lig_1,lig_2,lig_3,col_1,col_2,col_3;
  float beta, delta, salp;
  int icolonnes;

  
  /** Allocation de place pour les calculs intermediaires **/  

  a1=(float *) malloc(sizeof(float)*(IN->nrow)*(IN->ncol));
  a2=(float *) malloc(sizeof(float)*(IN->nrow)*(IN->ncol));
  a3=(float *) malloc(sizeof(float)*(IN->nrow)*(IN->ncol));
  a4=(float *) malloc(sizeof(float)*(IN->nrow)*(IN->ncol));



  /** Copie de I dans a1 **/

  for (i=0; i<(IN->nrow); i++) {
    icolonnes=i*(IN->ncol);
    for (j=0; j<(IN->ncol); j++)
      a1[icolonnes+j]=(float) mw_getdot_fimage(IN,j,i);
  }
  


  /** Calcul des constantes **/

  lig_1 = (IN->nrow) -1;
  lig_2 = (IN->nrow) -2;
  lig_3 = (IN->nrow) -3;
  col_1 = (IN->ncol) -1 ;
  col_2 = (IN->ncol) -2 ;
  col_3 = (IN->ncol) -3 ;
  
  beta= CARRE((1.-exp((double)(-(*alpha)))));
  delta = 1.+2.*(*alpha)*exp((double)(-(*alpha)))-exp((double)(-2.*(*alpha)));
  
  salp =beta/delta;
  
  ad1  = -2 * exp((double)(-(*alpha)));
  ad2  = exp((double)(-2.*(*alpha)));
  an1  = salp;
  an2  = salp * ((*alpha) - 1.) * exp((double)(-(*alpha)));
  an3  = salp * ((*alpha) + 1.) *exp((double)(-(*alpha)));
  an4  = -salp * exp ((double)(-2. * (*alpha)));
  an11 = -beta;
	

	
  /** Premier passage derivee en Y **/
  
  for (i = 0; i < (IN->nrow); ++i)
    {
      icolonnes=i*(IN->ncol);
      a2[icolonnes] = an1 * a1[icolonnes];
      a2[icolonnes+1] = an1 * a1[icolonnes+1] + 
	an2 * a1[icolonnes] - ad1 * a2[icolonnes];
      for (j = 2; j < (IN->ncol); ++j)
	a2[icolonnes+j] = an1 * a1[icolonnes+j] + an2 * a1[icolonnes+j-1]
	  -ad1 * a2[icolonnes+j-1] - ad2 * a2[icolonnes+j-2];
    }
  
  for (i = 0; i < (IN->nrow); ++i)
    {
      icolonnes=i*(IN->ncol);
      a3[icolonnes+col_1] = 0;
      a3[icolonnes+col_2] = an3 * a1[icolonnes+col_1];
      for (j = col_3; j >= 0; --j)
	a3[icolonnes+j] = an3 * a1[icolonnes+j+1] + an4 * a1[i*(IN->ncol)+j+2]-ad1 * a3[icolonnes+j+1] - ad2 * a3[icolonnes+j+2];
    }
  
  for (i = 0; i < (IN->nrow); ++i){
    icolonnes=i*(IN->ncol);
    for (j = 0; j < (IN->ncol); ++j)
      a2[icolonnes+j] += a3[icolonnes+j];
  }
  
  for (j = 0; j < (IN->ncol); ++j)
    {
      a3[j] = 0;
      a3[(IN->ncol)+j] = an11 * a2[j];
      for (i = 2; i < (IN->nrow); ++i)
	a3[i*(IN->ncol)+j] = an11 * a2[(i-1)*(IN->ncol)+j] - ad1 * a3[(i-1)*(IN->ncol)+j]-ad2 * a3[(i-2)*(IN->ncol)+j];
    }
  
  
  for (j = 0; j < (IN->ncol); ++j)
    {
      a4[lig_1*(IN->ncol)+j] = 0;
      a4[(lig_2*(IN->ncol))+j] = -an11 * a2[lig_1*(IN->ncol)+j];
      for (i = lig_3; i >= 0; --i)
	a4[i*(IN->ncol)+j] = -an11 * a2[(i+1)*(IN->ncol)+j] - ad1 * a4[(i+1)*(IN->ncol)+j]-ad2 * a4[(i+2)*(IN->ncol)+j];
    }
  
  
  for (i = 0; i < (IN->nrow); ++i){
    icolonnes=i*(IN->ncol);
    for (j = 0; j < (IN->ncol); ++j)
      a4[icolonnes+j] += a3[icolonnes+j];
  }
	

  
  /** Stockage du resultat du premier passage **/
 
  for (i = 0; i < (IN->nrow); ++i){
    icolonnes=i*(IN->ncol);
    for (j = 0; j < (IN->ncol); ++j)
      mw_plot_fimage(grady,i,j, a4[icolonnes+j]);
  }
  
  /** Deuxieme passage : derivee en X **/
  
  for (i = 0; i < (IN->nrow); ++i)
    {
      icolonnes=i*(IN->ncol);
      a2[icolonnes] = 0;
      a2[icolonnes+1] = an11 * a1[icolonnes];
      for (j = 2; j < (IN->ncol); ++j)
	a2[icolonnes+j] = an11 * a1[icolonnes+j-1] -
	  ad1 * a2[icolonnes+j-1] - ad2 * a2[icolonnes+j-2];
    }
  
  for (i = 0; i < (IN->nrow); ++i)
    {
      icolonnes=i*(IN->ncol);
      a3[icolonnes+col_1] = 0;
      a3[icolonnes+col_2] = -an11 * a1[icolonnes+col_1];
      for (j = col_3; j >= 0; --j)
	a3[icolonnes+j] = -an11 * a1[icolonnes+j+1] -
	  ad1 * a3[icolonnes+j+1] - ad2 * a3[icolonnes+j+2];
    }
  
  for (i = 0; i < (IN->nrow); ++i){
    icolonnes=i*(IN->ncol);
    for (j = 0; j < (IN->ncol); ++j)
      a2[icolonnes+j] += a3[icolonnes+j];
  }
  
  
  for (j = 0; j < (IN->ncol); ++j)
    {
      a3[j] = an1 * a2[j];
      a3[(IN->ncol)+j] = an1 * a2[(IN->ncol)+j] + an2 * a2[j] - ad1 * a3[j];
      for (i = 2; i < (IN->nrow); ++i)
	a3[i*(IN->ncol)+j] = an1 * a2[i*(IN->ncol)+j] + an2 * a2[(i-1)*(IN->ncol)+j]-ad1 * a3[(i-1)*(IN->ncol)+j] - ad2 * a3[(i-2)*(IN->ncol)+j];
    }
  
  for (j = 0; j < (IN->ncol); ++j)
    {
      a4[lig_1*(IN->ncol)+j] = 0;
      a4[lig_2*(IN->ncol)+j] = an3 * a2[lig_1*(IN->ncol)+j];
      for (i = lig_3; i >= 0; --i)
	a4[i*(IN->ncol)+j] = an3 * a2[(i+1)*(IN->ncol)+j] + an4 * a2[(i+2)*(IN->ncol)+j]-ad1 * a4[(i+1)*(IN->ncol)+j] - ad2 * a4[(i+2)*(IN->ncol)+j];
    }
  
  for (i = 0; i < (IN->nrow); ++i){
    icolonnes=i*(IN->ncol);
    for (j = 0; j < (IN->ncol); ++j)
      a4[icolonnes+j] += a3[icolonnes+j];
  }



  /** Stockage du resultat du premier passage **/
   
  for (i = 0; i < (IN->nrow); ++i) {
    icolonnes=i*(IN->ncol);
    for (j = 0; j < (IN->ncol); ++j)
      mw_plot_fimage(gradx,i,j, a4[icolonnes+j]);
  }  
  
  /** liberation de la place memoire **/

  free(a1);
  free(a2);
  free(a3);
  free(a4);

}


/*--------------------------------------------------------------------------*/

void maxima ( gradx,grady,  OUT, IN )
 Fimage OUT,IN;
 Fimage gradx,grady;
  {
 
#define CARRE(x)  ( (x) *(x) )
#define MODUL(x,y) (sqrt( CARRE(x) + CARRE(y) ))


  int icolonnes,icoll;
  int  lig_2,col_2,jp1,jm1,im1,ip1;
  register int i,j;
  register float wd,gzr,gun;
  
  float *a2,*a3,*a4;
  int icol_1;


  /** a4 contient le gradient en x **/
  /** a3 contient le gradient en y **/
      
  a4 =(float *) malloc(sizeof(float)*(IN->ncol)*(IN->nrow));
  a3 =(float *) malloc(sizeof(float)*(IN->ncol)*(IN->nrow)); 

  for (i=0; i<(IN->nrow); i++) {
    icolonnes=i*(IN->ncol);
    for (j=0; j<(IN->ncol); j++) {
      a4[icolonnes+j]= mw_getdot_fimage(gradx,i,j);
      a3[icolonnes+j]= mw_getdot_fimage(grady,i,j);
    }
  }
      

  
  /** a2 contient le module du gradient **/

  a2 =(float *) malloc(sizeof(float)*(IN->ncol)*(IN->nrow));
  
  icol_1=(IN->ncol)*(IN->nrow);
  for (i=0;i<icol_1;++i) a2[i] = MODUL (a3[i],a4[i]);
  


  /** Constantes **/
  
  lig_2 = (IN->nrow) -2;
  col_2 = (IN->ncol) -2 ;	



     
  for (i = 1; i <= lig_2; ++i)
    {
      icoll=i*(IN->ncol);
      for (j = 1; j <= col_2; ++j)
	{
	  jp1=j+1;
	  jm1=j-1;
	  ip1=i+1;
	  im1=i-1;
	  if ( a3[icoll+j] > 0. )
	    {
	      wd = a4[icoll+j] / a3[icoll+j];
	      a3[icoll+j]=0;
	      if ( wd >= 1 )
		{
		  gun = a2[icoll+jp1] + (a2[ip1*(IN->ncol)+jp1] - a2[icoll+jp1]) / wd;
		  if ( a2[icoll+j] <= gun ) continue;
		  gzr = a2[icoll+jm1] + (a2[im1*(IN->ncol)+jm1] - a2[icoll+jm1]) / wd;
		  if ( a2[icoll+j] < gzr ) continue;
		  a3[icoll+j] = a2[icoll+j];
		  continue;
		}
	      if ( wd >= 0 )
		{
		  gun = a2[ip1*(IN->ncol)+j] + (a2[ip1*(IN->ncol)+jp1] - a2[ip1*(IN->ncol)+j]) * wd;
		  if ( a2[icoll+j] <= gun ) continue;
		  gzr = a2[im1*(IN->ncol)+j] + (a2[im1*(IN->ncol)+jm1] - a2[im1*(IN->ncol)+j]) * wd;
		  if ( a2[icoll+j] < gzr ) continue;
		  a3[icoll+j] = a2[icoll+j];
		  continue;
		}
	      if ( wd >= -1)
		{
		  icolonnes=ip1*(IN->ncol);
		  gun = a2[icolonnes+j] - (a2[icolonnes+jm1] - a2[icolonnes+j]) * wd;
		  if ( a2[icoll+j] <= gun ) continue;
		  icolonnes=im1*(IN->ncol);
		  gzr = a2[icolonnes+j] - (a2[icolonnes+jp1] - a2[icolonnes+j]) * wd;
		  if ( a2[icoll+j] < gzr ) continue;
		  a3[icoll+j] = a2[icoll+j];
		  continue;
		}
	      gun = a2[icoll+jm1] - (a2[ip1*(IN->ncol)+jm1] - a2[icoll+jm1]) / wd;
	      if ( a2[icoll+j] <= gun ) continue;
	      gzr = a2[icoll+jp1] - (a2[im1*(IN->ncol)+jp1] - a2[icoll+jp1]) / wd;
	      if ( a2[icoll+j] < gzr ) continue;
	      a3[icoll+j] = a2[icoll+j];
	      continue;
	    }
	  if (  (a3[icoll+j]) == 0.)
	    {
	      if ( a4[icoll+j] == 0 ) continue;
	      if ( a4[icoll+j] < 0  )
		{
		  gzr = a2[icoll+jp1];
		  if ( a2[icoll+j] < gzr ) continue;
		  gun = a2[icoll+jm1];
		  if ( a2[icoll+j] <= gun ) continue;
		  a3[icoll+j] = a2[icoll+j];
		  continue;
		}
	      gzr = a2[icoll+jm1];
	      if ( a2[icoll+j] < gzr ) continue;
	      gun = a2[icoll+jp1];
	      if ( a2[icoll+j] <= gun ) continue;
	      a3[icoll+j] = a2[icoll+j];
	      continue;
	    }
	  wd = a4[icoll+j] / a3[icoll+j];
	  a3[icoll+j]=0;
	  if ( wd >= 1 )
	    {
	      gzr = a2[icoll+jp1] + (a2[ip1*(IN->ncol)+jp1] - a2[icoll+jp1]) / wd;
	      if ( a2[icoll+j] < gzr ) continue;
	      gun = a2[icoll+jm1] + (a2[im1*(IN->ncol)+jm1] - a2[icoll+jm1]) / wd;
	      if ( a2[icoll+j] <= gun ) continue;
	      a3[icoll+j] = a2[icoll+j];
	      continue;
	    }
	  if ( wd >= 0 )
	    {
	      gzr = a2[ip1*(IN->ncol)+j] + (a2[ip1*(IN->ncol)+jp1] - a2[ip1*(IN->ncol)+j]) * wd;
	      if ( a2[icoll+j] < gzr ) continue;
	      gun = a2[im1*(IN->ncol)+j] + (a2[im1*(IN->ncol)+jm1] - a2[im1*(IN->ncol)+j]) * wd;
	      if ( a2[icoll+j] <= gun ) continue;
	      a3[icoll+j] = a2[icoll+j];
	      continue;
	    }
	  if ( wd >= -1)
	    {
	      icolonnes=ip1*(IN->ncol);
	      gzr = a2[icolonnes+j] - (a2[icolonnes+jm1] - a2[icolonnes+j]) * wd;
	      if ( a2[icoll+j] < gzr ) continue;
	      icolonnes=im1*(IN->ncol);
	      gun = a2[icolonnes+j] - (a2[icolonnes+jp1] - a2[icolonnes+j]) * wd;
	      if ( a2[icoll+j] <= gun ) continue;
	      a3[icoll+j] = a2[icoll+j];
	      continue;
	    }
	  gzr = a2[icoll+jm1] - (a2[ip1*(IN->ncol)+jm1] - a2[icoll+jm1]) / wd;
	  if ( a2[icoll+j] < gzr )    continue;
	  gun = a2[icoll+jp1] - (a2[im1*(IN->ncol)+jp1] - a2[icoll+jp1]) / wd;
	  if ( a2[icoll+j] <= gun )   continue;
	  a3[icoll+j] = a2[icoll+j];
	}
    }



  /** Marquage des bords pour ne pas avoir de valeurs negatives **/
  
  for (i = 0; i < (IN->ncol); i++)a3[i]=0;
  for (i = (IN->ncol)*((IN->nrow)-1); i < (IN->ncol)*(IN->nrow); ++i)a3[i]=0;
  for (i = 0; i < (IN->nrow); i++)a3[i*(IN->ncol)]=0;
  for (i = 1; i < (IN->nrow); i++)a3[i*(IN->ncol)-1]=0;
  
  

  /** Stokage des maxima locaux **/
    OUT=mw_change_fimage(OUT,(IN->nrow),(IN->ncol));
      if (OUT==NULL) mwerror(FATAL,1,"NOT ENOUGH MEMORY.\n");

  for (i = 0; i < (IN->nrow); ++i) {
    icolonnes=i*(IN->ncol);
    for (j = 0; j < (IN->ncol); ++j)
    {if (a3[icolonnes+j]<0.) a3[icolonnes+j]=0;
     if (a3[icolonnes+j]>255.) a3[icolonnes+j]=255;
      mw_plot_fimage(OUT,j,i,a3[icolonnes+j]);
  }  }


  /** liberation de la place memoire **/
  free(a2);
  free(a3);
  free(a4);
  
}

/*--------------------------------------------------------------------------*/

void fedge_detect ( alpha,IN, OUT )
float *alpha;
Fimage IN, OUT;

{

  Fimage gradx,grady;

  gradx=mw_change_fimage(NULL,IN->ncol,IN->nrow);
   if (gradx==NULL) mwerror(FATAL,1,"NOT ENOUGH MEMORY.\n");
  grady=mw_change_fimage(NULL,IN->ncol,IN->nrow);
   if (grady==NULL) mwerror(FATAL,1,"NOT ENOUGH MEMORY.\n");

  /* passage des filtres ainsi que calcul du gradient de l'image */

  gradient ( IN,gradx,grady,  alpha);
  
  /* detection des maxima locaux */

  maxima ( gradx,grady,OUT,IN);

mw_delete_fimage(gradx);
mw_delete_fimage(grady);
   
}


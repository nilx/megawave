/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand 
name = {rotaffin};
version = {"2.2"};
author = {"Frederic Guichard, Denis Pasquignon"};
function = {"rotations and orthogonal affinities of binary images"};
usage = {
 'r':[Nrotations=8]->Nrota [1, 360]    "Number of rotations",
 'a':[Naffi=2]->Naffi[1,100]           "Number of affinities",
 't':[Size=10]->Size [1,1000]          "Size of the masks",
 'T':[Type=0]->Type [0,3]              "Type of object, default is square (0)",
 'A':[Area=15]->Area[1,5000]           "Square root of the Area of object",
 'd':[Definition=10]->Definition [1,10000]   "Precision of the calculs",
 'o':[OptSym=2.0]->OptSym[0.0,1000.0]  "2pi/OptSym symmetry of mask",
 'f':[Factor=1.0]->Factor[0.01,100.0]  "Factor of affinity, should be 1",
 'M':cimage->cimage   "a mask",
 cmovie<-cmovie       "rotations and affinities of the mask (movie of masks)"
};
*/
/*----------------------------------------------------------------------
 v2.1: return void (L.Moisan)
 v2.2: version syntax fixed (JF)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h"

static void Final(Fimage pict, Cmovie sortie)
{
register int i;
Cimage image1, image2;
int dx,dy,Size;
register float *b;
register unsigned char *a;
float g;

image1=NULL;
dx= pict->ncol;
dy=pict->nrow;
image1= mw_change_cimage(image1, dx,dy);
if (sortie -> first == NULL) {
	sortie -> first = image1;
}
else {
	image2 = sortie->first;
	while (image2 ->next != NULL) { image2 = image2 -> next; }
	image2 -> next = image1;
	image1 -> previous = image2;
}
a = image1 -> gray;
b = pict -> gray;
Size = dx*dy;
for(i=0;i<Size;i++) {
	g =  (*b)*256.0;
	if (g<0.0) g=0.0;
	if (g>255.0) g=255.0;
	*a = (unsigned char) g;
	a++;
	b++;
}
}

static int in_circle(float x, float y)
{
  if ((x*x+y*y)< 1) return(1); else return(0);
}

static int in_rect(float x, float y)
{
  if ((x<0.4)&&(x>-0.4)&&(y<0.4)&&(y>-0.4)) return(1); else return(0);
}

static int in_srect(float x, float y)
{
  if ((x<0.4)&&(x>=0)&&(y<0.1)&&(y>-0.1)) return(1); else return(0);
}

void rotaffin(int *Nrota, int *Naffi, int *Size, int *Type, float *Area, int *Definition, double *OptSym, float *Factor, Cimage cimage, Cmovie cmovie)
{
  int size;
  Fimage pict;
  int i,j,Nr,Na,kx,ky,Nr2;
  float Area2,Ndef,Ndefarea;
  float x,y, x1, y1,x2,y2, C1, S1, scale,rx,ry;
  double theta;
  int somme;
  int pos;
  int fact_x, fact_y;
  float f_x,f_y;
  int ii,jj;
  
  if (cimage!=NULL) *Type=5;
  if (*Type==5) 
    {
      fact_x= cimage->ncol;
      fact_y= cimage->nrow;
      f_x= (float) fact_x;
      f_y= (float) fact_y;
    }
 
  size = (*Size) + (*Size) +1;
  Ndef= (float) (*Definition);
  Ndefarea = Ndef*Ndef;
  Area2 = (float) ( (*Area)*(*Area));
  
  pict = mw_new_fimage();
  if (pict== NULL) mwerror(FATAL, 1, "Not enough memory \n");
  pict = mw_change_fimage(pict, size, size);
  
  /*  Boucle sur les affinites */
  
  for(Na=0;Na<(*Naffi);Na++)
    {
      scale =   ((float) Na)/( (float) (*Naffi)-0.95);
      rx= (*Area)*(1-scale)+ (1+ (Na==((*Naffi)-1))*((*Factor)-1))*size*scale;
      if (((*Naffi)==1)&&(*Factor!=1.0)) rx= (*Area)*(1-scale)+ (1+ (Na==((*Naffi)-1))*((*Factor)-1))*size;
      ry= 2.*rx/Area2;
      rx= 2.0/rx;
      Nr2= (*Nrota);
      if ((*Type==1)&&(Na==0)) Nr2=1;  /* cas du cercle, rotations inutiles */

      /* Boucle sur les rotations */
      for(Nr=0; Nr<Nr2; Nr++)
	{
	  mwdebug(" Objet : %i\n", Na*(*Nrota)+Nr);
	  /* ANGLE */
	  theta = 2.0*3.1415926535* ((double) Nr)/((double) (*Nrota))/(*OptSym);
	  C1= (float) cos(theta);
	  S1 = (float) sin(theta);
	  
	  
	  for(j=-(*Size);j<=(*Size);j++)
	    {
	      pos= ((j+ (*Size))*size) + *Size;
	      for(i=-(*Size);i<=(*Size);i++)
		{
		  somme=0;
		  for(kx=0;kx<(*Definition);kx++)
		    for(ky=0;ky<(*Definition);ky++) {
		      x= ((float) i) + (((float) kx + 0.5)/Ndef -0.50);
		      y= ((float) j) + (((float) ky + 0.5)/Ndef -0.50);
		      
		      /* Rotation inverse */
		      x1= C1*x + S1*y;
		      y1= - S1*x + C1*y;
	   
		      /* Affinite inverse */
		      x2=x1*rx;
		      y2=y1*ry; 
/*		      printf(" C1=%f S1=%f \n ",C1,S1);
		      printf(" x2=%f y2=%f \n ",x2,y2); */
	  		
		      
		      if (*Type==0)  somme += in_rect(x2,y2);
		      if (*Type==1)  somme += in_circle(x2,y2);
		      if (*Type==2)  somme += in_srect(x2,y2);
		      if (*Type==3)  somme ++;
		      if (*Type==5)
			{
			  if ((x2>-1.0)&&(x2<1.0)&&(y2>-1.0)&&(y2<1.0)) {
			    x2=(1.0 +x2)*f_x*0.5 ;
			    y2=(1.0 +y2)*f_y*0.5 ;
			    ii= (int) x2;
			    jj= (int) y2;
/*			    printf(" ii=%i jj=%i \n ",ii,jj);*/			   
/*			    if (somme==0) somme=1;  */
			    if (cimage->gray[ii+jj*fact_x]>0) somme++;
			  }
			}
		    }
		  pict->gray[pos+i]= ((float) somme)/Ndefarea;
		}
	    }

mwdebug(" %f %f %f %f %f\n",pict->gray[0],pict->gray[1],pict->gray[2],pict->gray[3],pict->gray[4]);
mwdebug(" %f %f %f %f %f\n",pict->gray[5],pict->gray[6],pict->gray[7],pict->gray[8],pict->gray[9]);
mwdebug(" %f %f %f %f %f\n",pict->gray[10],pict->gray[11],pict->gray[12],pict->gray[13],pict->gray[14]);
mwdebug(" %f %f %f %f %f\n",pict->gray[15],pict->gray[16],pict->gray[17],pict->gray[18],pict->gray[19]);
mwdebug(" %f %f %f %f %f\n",pict->gray[20],pict->gray[21],pict->gray[22],pict->gray[23],pict->gray[24]);

	  Final(pict,cmovie);
	}
    }
  
  
  
}








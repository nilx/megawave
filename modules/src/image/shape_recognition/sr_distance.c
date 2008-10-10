/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {sr_distance};
 version = {"1.3"};
 author = {"Thierry Cohignac, Lionel Moisan"};
 function = {"Compute distance between two shapes (binary product)"};
 usage = {            
     in1->Shape1         "input shape 1 (Fcurves)",
     in2->Shape2         "input shape 2 (Fcurves)",
     out<-sr_distance    "result"
};
*/
/*----------------------------------------------------------------------
 v1.3: changed Shape to the_shape, fkzrt, fkcenter, fkplot (L.Moisan)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw.h"
#include "mw-modules.h" /* for fkplot(), erosion(), fkzrt(),
			 * fkcenter() */

#define INT(x) (floor(x + 5))
#define BLACK 0
#define WHITE 255

/* Compute the size (number of points) of a Fcurves structure */

int size_fcurves(cs)
Fcurves cs;
{
  Fcurve        c;
  Point_fcurve  p;
  int           n;

  n = 0;

  for (c=cs->first; c; c=c->next) 
    for (p=c->first; p; p=p->next) 
      n++;

  return n;
}

/*** Compute the barycenter of a cimage ***/

void bary_img(image,xg,yg)
Cimage image;
float  *xg,*yg;
{
  int x,y,dx,dy;
  int i,j,cpt;
  unsigned char *pgray;

  dx = image->ncol;
  dy = image->nrow;
  
  x=y=cpt=0;
  pgray=image->gray;

  for(i=0;i<dy;i++)
    for(j=0;j<dx;j++)
      if(*pgray++ == BLACK){
	x+=j;
	y+=i;
	cpt++;
      }
  
  *xg = (float)x/(float)cpt;
  *yg = (float)y/(float)cpt;
}


/* compute the lowest product up to rotations */

int Product_curves_rot(image,XG,YG,the_shape)
Cimage   image;
float    XG,YG;
Fcurves  the_shape;
{
  Point_fcurve  pixel;
  Fcurve        curve;
  float         x,y,xf,yf,cs,sn,theta,theta_min;
  long           xx,yy,i,dx,dy,k,l,prod,prod_min;
  
  prod_min = 100000;
  
  dx = image->ncol;
  dy = image->nrow;
  
  for(i=0;i<45;i++) {
    
    theta = 2.0*M_PI*(float)i/45.0;
    
    cs = (float)cos((double)theta);
    sn = (float)sin((double)theta);
    
    prod = 0;
    
    for (curve=the_shape->first; curve; curve=curve->next) 
      for (pixel=curve->first; pixel; pixel=pixel->next) {
	
	x = pixel->x;
	y = pixel->y;
	
	xx = INT( XG + cs*x-sn*y );
	yy = INT( YG + sn*x+cs*y );
	if ((xx >= 0)&&(xx < dx)&&(yy >=0)&&(yy <dy)) {
	  if ( image->gray[dx*yy+xx] == WHITE) prod++;
	} else prod++;
	
      }
    
    if (prod < prod_min) {
      theta_min = theta;
      prod_min = prod;
    }
  }
    
  /*************** refining by translation **************/
  
  fkzrt(the_shape,1.0,theta_min*180.0/M_PI,0.0,0.0);
  
  for(k=-3;k<4;k+=2 )
    for(l=-3;l<4;l+=2) {

      prod=0;
      
      for (curve=the_shape->first; curve; curve=curve->next) 
	for (pixel=curve->first; pixel; pixel=pixel->next) {
	  
	  xx = k + INT(XG + pixel->x);
	  yy = l + INT(YG + pixel->y);
	  
	  if ((xx >= 0)&&(xx < dx)&&(yy >=0)&&(yy <dy)) {
	    if ( image->gray[dx*yy+xx] == WHITE) prod++;
	  } else prod++;

	}
      
      if (prod<prod_min) prod_min = prod;
    }
  
  return prod_min;
}



/*------------------------------ MAIN MODULE ------------------------------*/

float sr_distance(Shape1,Shape2)
Fcurves Shape1,Shape2;
{
  int     product1_2,product2_1,product,surface_shape1,surface_shape2,n_iter;
  Cimage  image1,image2,image1_dilat,image2_dilat;
  float   xg,yg,radius,dist_max,dist1_2,dist2_1,dist;
  

  dist_max = 0.1; /* threshold for the product */ 
  
  surface_shape1 = size_fcurves(Shape1);
  surface_shape2 = size_fcurves(Shape2);
  
  image1 = fkplot(Shape1,NULL,(char *)1);
  image2 = fkplot(Shape2,NULL,(char *)1);
    
  /* dilation by a disc of radius 1 */
  
  radius = 1.0;
  n_iter = 1;
  image1_dilat = erosion(image1,NULL,&radius,NULL,&n_iter,NULL);
  image2_dilat = erosion(image2,NULL,&radius,NULL,&n_iter,NULL);
  
  /* centering */

  fkcenter(Shape2,&xg,&yg);
  fkzrt(Shape2,1.0,0.0,-xg,-yg);
  fkcenter(Shape1,&xg,&yg);
  fkzrt(Shape1,1.0,0.0,-xg,-yg);


  /*--- Product (dilated image 1)*(shape 2) ---*/
  
  bary_img(image1_dilat,&xg,&yg);      
  product1_2 = Product_curves_rot(image1_dilat,xg,yg,Shape2);
  dist1_2 = (float)product1_2/(float)surface_shape2;

  /*--- Product (dilated image 2)*(shape 1) ---*/
  
  bary_img(image2_dilat,&xg,&yg);
  product2_1 = Product_curves_rot(image2_dilat,xg,yg,Shape1);
  dist2_1 = (float)product2_1/(float)surface_shape1;

  mw_delete_cimage(image1);
  mw_delete_cimage(image2);
  mw_delete_cimage(image1_dilat);
  mw_delete_cimage(image2_dilat);
  
  /* returns the larger distance */
  dist = (dist1_2 < dist2_1) ? dist2_1 : dist1_2;
  return dist*100.0;
}	



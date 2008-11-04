/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
 name = {cfchgchannels};
 version = {"1.2"};
 author = {"Jean-Pierre D'Ales, Jacques Froment"};
 function = {"Convert a color float image from one coordinate system to another"};
 usage = {
   'c':[Conv=0]->Conv [0,2]  "0:RGB-YUV 1:RGB-HSI 2:RGB-HSV", 
   'i'->Inverse              "Perform inverse conversion", 
   'n'->Norm                 "Normalize channel to fit in [0,255]", 
   in->Image                 "Input cfimage", 
   out<-TImage               "Output cfimage"
};
*/
/*----------------------------------------------------------------------
 v1.2 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/


#include <stdio.h>
#include <math.h>
#include  "mw.h"

/* Define DEGRES to compute HUE in degres (else computed in radian) */

#define DEGRES

/*--- Constants ---*/

#define HALF_SHIFT 128.0
#define RED 0
#define GREEN 1
#define BLUE 2

#define P2 (2.0 * M_PI) 
#define P3 (M_PI / 3.0) 
#define P23 (2.0 * M_PI / 3.0) 
#define P43 (4.0 * M_PI / 3.0) 

/*--- Coefficients for RGB->YUV and YUV->RGB conversion (CCIR 601-1) ---*/

#define COEFF_YR 0.299
#define COEFF_YG 0.587
#define COEFF_YB 0.114

#define COEFF_NORM_B ((float) 2.0 - 2.0 * COEFF_YB)
#define COEFF_NORM_R ((float) 2.0 - 2.0 * COEFF_YR)

/*--- Global variables ---*/ 


static void
YUV_RGB(r, g, b, tr, tg, tb)

float      *r, *g, *b;   /* Pointers to red, green and blue channels 
				   * in image */
float      *tr, *tg, *tb;  /* Pointers to red, green and blue 
				   * channels in transformed image */

{

  *tg = (*g - COEFF_YR * (*r + *g) - COEFF_YB * (*b + *g)) / COEFF_YG;
  *tb = *b + *g;
  *tr = *r + *g;

}


static void
RGB_YUV(r, g, b, tr, tg, tb)

float      *r, *g, *b;   /* Pointers to red, green and blue channels 
				   * in image */
float      *tr, *tg, *tb;  /* Pointers to red, green and blue 
				   * channels in transformed image */

{

  *tg = COEFF_YR * *r + COEFF_YG * *g + COEFF_YB * *b;
  *tb = *b - *tg;
  *tr = *r - *tg;

}


static void
YUV_RGB_NORM(r, g, b, tr, tg, tb)

float      *r, *g, *b;   /* Pointers to red, green and blue channels 
				   * in image */
float      *tr, *tg, *tb;  /* Pointers to red, green and blue 
				   * channels in transformed image */

{

  *tb = (*b - HALF_SHIFT) * COEFF_NORM_B + *g;
  *tr = (*r - HALF_SHIFT) * COEFF_NORM_R + *g;
  *tg = (*g - COEFF_YR * *tr - COEFF_YB * *tb) / COEFF_YG;

}


static void
RGB_YUV_NORM(r, g, b, tr, tg, tb)

float      *r, *g, *b;   /* Pointers to red, green and blue channels 
				   * in image */
float      *tr, *tg, *tb;  /* Pointers to red, green and blue 
				   * channels in transformed image */

{

  *tg = COEFF_YR * *r + COEFF_YG * *g + COEFF_YB * *b;
  *tb = (*b - *tg) / COEFF_NORM_B + HALF_SHIFT;
  *tr = (*r - *tg) / COEFF_NORM_R + HALF_SHIFT;

}

/* ----- HSI ----- */

static void
RGB_HSI(r, g, b, tr, tg, tb)

float      *r, *g, *b;   /* Pointers to red, green and blue channels 
				   * in image */
float      *tr, *tg, *tb;  /* Pointers to red, green and blue 
				   * channels in transformed image */

{
  float m;
  double d,R,G,B;

  R=*r / 255.0;
  G=*g / 255.0;
  B=*b / 255.0;

  /* Intensity put in the B channel */
  *tb = (R + G + B)/3.0; 

  /* Saturation put in the G channel */
  if (*tb == 0.0) *tg = -1e30;
  else
    {
      m=R;
      if (m > G) m=G;
      if (m > B) m=B;
      *tg = 1.0 - (m / *tb);
    }

  /* Hue put in the R channel */
  if (*tg == 0.0) *tr=0.0;
  else
    {
      d = (R - G);
      d = (2.0* R - G - B) / (2.0 * sqrt(d*d + (R-B)*(G-B)));
#ifdef DEGRES         
      *tr = 180.0 * acos(d) / M_PI;
      if (B > G) *tr = 360.0 - *tr;
#else
      *tr = acos(d);
      if (B > G) *tr = P2 - *tr;
#endif
    }
}

#define alpha(S) (1.0 - (S))
#ifdef DEGRES
#define beta(S,H) (1.0 + ((S) * cos ((H) * M_PI / 180.0)) / cos(P3-((H) * M_PI / 180.0)))
#else
#define beta(S,H) (1.0 + ((S) * cos ((H))) / cos(P3-(H)))
#endif

static void
HSI_RGB(r, g, b, tr, tg, tb)

float      *r, *g, *b;   /* Pointers to red, green and blue channels 
				   * in image */
float      *tr, *tg, *tb;  /* Pointers to red, green and blue 
				   * channels in transformed image */

{
  double R,G,B;


  /* 0 < H <= 120 deg : sector RG */
#ifdef DEGRES
      if ((*r > 0) && (*r <= 120)) 
#else
  if ((*r > 0) && (*r <= P23)) 
#endif
    {
      R = beta((double) *g, (double) *r);
      B = alpha((double) *g);
      G = 3.0  - (R + B);
    } 
  else
    /* 120 deg < H <= 240 deg : sector GB */
#ifdef DEGRES
    if ((*r > 120) && (*r <= 240)) 
#else
    if ((*r > P23) && (*r <= P43)) 
#endif
      {
      R = alpha((double) *g);
#ifdef DEGRES
      G = beta((double) *g, (double) *r - 120);
#else
      G = beta((double) *g, (double) *r - P23);
#endif
      B = 3.0 - (R + G);
      } 
    else /* 240 deg < H <= 360 deg : sector BR */
      {
	G = alpha((double) *g);
#ifdef DEGRES
	B = beta((double) *g, (double) *r - 240);
#else
	B = beta((double) *g, (double) *r - P43);
#endif
	R = 3.0 - (G + B);
      }     
  *tr = *b * R * 255.0; *tg = *b * G * 255.0; *tb = *b * B * 255.0;
}


/* ----- HSV ----- */

#define NOHUE -1

static void
RGB_HSV(r, g, b, tr, tg, tb)

float      *r, *g, *b;   /* Pointers to red, green and blue channels 
				   * in image */
float      *tr, *tg, *tb;  /* Pointers to red, green and blue 
				   * channels in transformed image */

{
  float R,G,B,d,max,min,rr,bb,gg;

  R=*r / 255.0;
  G=*g / 255.0;
  B=*b / 255.0;
  
  /* Max R,G,B value */
  max=R;
  if (max < G) max=G;
  if (max < B) max=B;
  /* Min R,G,B value */
  min=R;
  if (min > G) min=G;
  if (min > B) min=B;

  /* Value put in the B channel */
  *tb=max;

  /* Saturation put in the G channel */
  d=max-min; 
  if (d != 0.0) *tg=d/max; else *tg=0.0;

  /* Hue put in the R channel */
  *tr = NOHUE;
  if (d != 0.0)
    {
      rr=(max-R)/d;
      gg=(max-G)/d;
      bb=(max-B)/d;
      if (R==max) *tr = bb-gg;
      else if (G==max) *tr = 2 + rr-bb;
      else if (B==max) *tr = 4 + gg-rr;
#ifdef DEGRES
      *tr = *tr * 60.0; 
      if (*tr < 0.0) *tr += 360.0;
      if (*tr == 360.0) *tr=0.0;
#else
      *tr = *tr * P3; 
      if (*tr < 0.0) *tr += P2;
      if (*tr == P2) *tr=0.0;
#endif
    }
}

static void
HSV_RGB(r, g, b, tr, tg, tb)

float      *r, *g, *b;   /* Pointers to red, green and blue channels 
				   * in image */
float      *tr, *tg, *tb;  /* Pointers to red, green and blue 
				   * channels in transformed image */

{
  float R,G,B,x,y,z,t;
  double h;
  int sector;

  if (*r == NOHUE || *g == 0.0)
    R=G=B=*b;
  else
    {
#ifdef DEGRES
      h = *r/60.0;
#else
      h = *r/P3;      
#endif
      sector=(int) floor(h);
      if (sector < 0) sector=0;
      x=h-sector;
      y = *b * (1.0 - *g);
      z = *b * (1.0 - *g * x);
      t = *b * (1.0 - *g * (1.0 - x));
      switch (sector)
	{
	case 0: 
	  R=*b; G=t; B=y;
	  break;

	case 1:
	  R=z; G=*b; B=y;
	  break;

	case 2:
	  R=y; G=*b; B=t;
	  break;

	case 3:
	  R=y; G=z; B=*b;
	  break;

	case 4:
	  R=t; G=y; B=*b;
	  break;

	case 5:
	  R=*b; G=y; B=z;
	  break;

	default:
	  mwerror(FATAL,1,"[HSV_RGB] Illegal sector = %d\n",sector);
	  break;
	}
    }
  *tr = R * 255.0;
  *tg = G * 255.0;
  *tb = B * 255.0;
}

/* ----------------------- */

static void
COLOR_CONVERT(image, timage, convert_point)

Cfimage     image;		/* Input color image */
Cfimage     timage;		/* Output transformed image */
void        (*convert_point) ();  /* Pointer to the conversion
				        * function */

{
  float      *r, *g, *b;  /* Pointers to red, green and blue channels 
				    * in image */
  float      *tr, *tg, *tb;  /* Pointers to red, green and blue 
				    * channels in transformed image */
  int         c, size;

  size = image->nrow * image->ncol;
  r = image->red;
  g = image->green;
  b = image->blue;
  tr = timage->red;
  tg = timage->green;
  tb = timage->blue;
  for (c = 0; c < size; c++, r++, g++, b++, tr++, tg++, tb++)
    convert_point(r, g, b, tr, tg, tb);


}




void
cfchgchannels(Conv, Inverse, Norm, Image, TImage)

	/*--- Computes the orthogonal wavelet transform of image `Image` ---*/

int        *Conv;		/* Conversion type */
int        *Inverse;	        /* Perform inverse transform */
int        *Norm;               /* Perform normalisation */
Cfimage     Image;		/* Input color image */
Cfimage     TImage;		/* Output transformed image */

{
  void        (*convert_point) ();  /* Pointer to the conversion
				        * function */
  float       redrate, greenrate, bluerate; /* Target rates for each channel */
  float      *RedR, *GreenR, *BlueR; /* Target rates for each channel */
  double      psnr;	        /* Peak signal to noise ratio / `Image` */

  /*--- Memory allocation for transformed color image ---*/

  TImage = mw_change_cfimage(TImage, Image->nrow, Image->ncol);
  if (TImage == NULL)
    mwerror(FATAL, 1, "Not enough memory for transformed color image!\n");


  /*--- Compute transformed color image ---*/

  if (Inverse)
    switch (*Conv) {
  case 0:
      if (Image->model != MODEL_YUV)
        mwerror(WARNING, 1, "colorimetric system of input image may not be YUV !\n");
      
      TImage->model=MODEL_RGB;
      if (Norm)
	convert_point = YUV_RGB_NORM;
      else
	convert_point = YUV_RGB;
      break;
      
  case 1:
       if (Image->model != MODEL_HSI)
        mwerror(WARNING, 1, "colorimetric system of input image is not HSI !\n");
       TImage->model=MODEL_RGB;
       convert_point = HSI_RGB;
      break;    

  case 2:
       if (Image->model != MODEL_HSV)
        mwerror(WARNING, 1, "colorimetric system of input image is not HSV !\n");
       TImage->model=MODEL_RGB;
       convert_point = HSV_RGB;
      break;    
    }
  else
    switch (*Conv) {
  case 0:
      if (Image->model != MODEL_RGB)
        mwerror(WARNING, 1, "colorimetric system of input image is not RGB !\n");
      TImage->model=MODEL_YUV;
      if (Norm)
	convert_point = RGB_YUV_NORM;
      else
	convert_point = RGB_YUV;
      break;
      
  case 1:
       if (Image->model != MODEL_RGB)
        mwerror(WARNING, 1, "colorimetric system of input image is not RGB !\n");
       TImage->model=MODEL_HSI;
       convert_point = RGB_HSI;
      break;

  case 2:
       if (Image->model != MODEL_RGB)
        mwerror(WARNING, 1, "colorimetric system of input image is not RGB !\n");
       TImage->model=MODEL_HSV;
       convert_point = RGB_HSV;
      break;
    }

  COLOR_CONVERT(Image, TImage, convert_point);
}

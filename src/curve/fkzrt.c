/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fkzrt};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Zoom, Rotate then translate IN PLACE a set of Fcurves"};
   usage = {
    in->cs       "input Fcurves",
    out<-fkzrt   "output Fcurves (modifed input)",
    zoom->zoom   "zoom factor",
    angle->angle "rotation angle (in degrees, counterclockwise)",
    x->x         "translation vector (x coordinate)",
    y->y         "translation vector (y coordinate)"
   };
*/

#include <stdio.h>
#include <math.h>
#include "mw.h"


Fcurves fkzrt(cs,zoom,angle,x,y)
Fcurves cs;
float   zoom,angle,x,y;
{
  Fcurve        c;
  Point_fcurve  p;
  double        theta,ct,st,px,py;

  theta = (double)(angle*M_PI/180.0);
  ct = (double)zoom * cos(theta);
  st = (double)zoom * sin(theta);

  for (c=cs->first; c; c=c->next) 
    for (p=c->first; p; p=p->next) {
      
      px = (double)p->x;
      py = (double)p->y;
      
      p->x = (float)( ct*px - st*py ) + x;
      p->y = (float)( st*px + ct*py ) + y;
      
    }

  return cs;
}





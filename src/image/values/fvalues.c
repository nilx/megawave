/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {fvalues};
   author = {"Georges Koepfler"};
   function = {"Get and sort all the pixel values of an fimage"};
   usage = {
   'i'->i_flag
        "decreasing values (default is increasing)",
   'm':multiplicity<-multiplicity
        "output the multiplicity of the values (fsignal)",
   image_in->image_in 
        "input fimage",
   values<-fvalues 
        "output set of sorted values (fsignal)"
};
version = {"3.0"};
*/

#include <stdio.h>
#include <math.h>

/*#define NDEBUG               comment this line in/out to un/enable assert() */
#include <assert.h>
#include  "mw.h"

#ifndef u_long
#define u_long unsigned long
#endif
#define h_value(A)      (*(heap+(u_long)(A))) 
#define h_up(A)         ((u_long)((u_long)(A)-1)>>1)
#define h_left(A)       ((u_long)((u_long)(A)<<1)+1)
#define h_right(A)      ((u_long)((u_long)(A)+1)<<1)

Fsignal
fvalues(i_flag,multiplicity,image_in)
char *i_flag;
Fsignal multiplicity;
Fimage image_in;
{
  Fsignal values;
  float *ptr, *heap , tmp;
  u_long nb_values, size_max, l, h0, h1, h2;

  size_max=image_in->ncol*image_in->nrow;
  if(size_max<=1)
    mwerror(FATAL,1,"image_in too small");

  if(!(heap=(float*)malloc(size_max*sizeof(float))))
    mwerror(FATAL,1,"\n Not enough memory for initialisation!!\n");

  l=0; ptr=image_in->gray;
  do {
    h1=l++;
    while((h1>(u_long)0)&&(*ptr<h_value(h2=h_up(h1)))) { 
      h_value(h1)=h_value(h2);
      h1=h2;
    }
    h_value(h1)=*(ptr++);
  }while(ptr<image_in->gray+size_max);
  assert(ptr==image_in->gray+size_max);
  assert(l==size_max);
  assert(l>(u_long)0);
  if(l==1)
    nb_values=1;
  else {
    nb_values=0;
    do {
      tmp=h_value(--l);
      h_value(l)=h_value(0);
      if((nb_values==0)||(h_value(l)>h_value(l+1))) nb_values++;
      h0=0;
      while((h1=h_left(h0))<l)  {
	if(((h2=h_right(h0))<l)&&(h_value(h2)<h_value(h1)))  h1=h2;
	if(h_value(h1)>tmp) break; 
	h_value(h0)=h_value(h1);
	h0=h1;
      }
      h_value(h0)=tmp;
    }while(l>1);
    assert(l==1);
    if(h_value(0)>h_value(1)) nb_values++;
  }
  assert(nb_values>(u_long)0);
  if(!(values=mw_change_fsignal(NULL,nb_values)))
    mwerror(FATAL,1,"\n Not enough memory for values!!\n");
  ptr=heap;
  if(i_flag) {
    values->values[0]=*ptr;
    for(l=1;l<nb_values;l++) {
      do { ptr++; } while(!(*(ptr-1)>*ptr));
      /*      while(!(*ptr>*(++ptr))) ;*/
      values->values[l]=*ptr;
    }
    assert(ptr<heap+size_max);
  }
  else {
    values->values[nb_values-1]=*ptr;
    for(l=nb_values-1;l>(u_long)0;l--) {
      do { ptr++; } while(!(*(ptr-1)>*ptr));
      values->values[l-1]=*ptr;
    }
    assert(ptr<heap+size_max);
  }
  if(multiplicity) {
    if(!(multiplicity=mw_change_fsignal(multiplicity,nb_values)))
          mwerror(FATAL,1,"\n Not enough memory for multiplicity!!\n");
    ptr=heap+size_max-1;
    h0=1;
    while((ptr!=heap)&&(!(*ptr<*(ptr-1)))) {ptr--; h0++;}
    if(ptr!=heap) ptr--;
    l=(i_flag)? nb_values-1:0;
    multiplicity->values[l]=h0;
    while(ptr!=heap) {
      h0=1;
      while((ptr!=heap)&&(!(*ptr<*(ptr-1)))) {ptr--;h0++;}
      if(ptr!=heap) ptr--;
      if(i_flag) multiplicity->values[--l]=h0;
      else       multiplicity->values[++l]=h0;
    }
    assert(ptr==heap);
    if(*ptr>*(ptr+1)) {
      if(i_flag) multiplicity->values[--l]=1;
      else       multiplicity->values[++l]=1;
    }
    assert((i_flag && l==0)||(!i_flag && l==nb_values-1));
#ifndef NDEBUG
    for(l=0,tmp=0;l<nb_values;l++) size_max-=multiplicity->values[l];
    assert(size_max==0);
#endif
  }
  free((void*)heap);
  return(values);
}



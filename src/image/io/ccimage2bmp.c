/*--------------------------- MegaWave2 module -----------------------------*/
/* mwcommand
  name = {ccimage2bmp};
  version = {"1.0"};
  author = {"Lionel Moisan"};
  function = {"Convert a Ccimage to BMP format"};
  usage = {
     in->in      "input Ccimage",
     out<-out    "output BMP file (Rawdata)"
          };
*/


#include "mw.h"

void rputc(r,i,c)
Rawdata r;
int *i;
char c;
{
  if (*i<r->size) r->data[(*i)++] = c;
  else mwerror(FATAL,1,"Cannot write outside Rawdata allocated zone.");
}

void rputint(r,i,n)
Rawdata r;
int *i;
unsigned int n;
{
  rputc(r,i,(char)(  n      & 0xff ));  
  rputc(r,i,(char)( (n>>8)  & 0xff ));
  rputc(r,i,(char)( (n>>16) & 0xff ));
  rputc(r,i,(char)( (n>>24) & 0xff ));
}

/*-------------------- MAIN MODULE --------------------*/

Rawdata ccimage2bmp(in,out)
Ccimage in;
Rawdata out;
{
  int i,n,linesize,x,y,adr;

  linesize = ((in->ncol*3+3)/4)*4;   /* number of bytes per line */
  n = 54+linesize*in->nrow;          /* size of output */
  out = mw_change_rawdata(out,n);
  i = 0;

  /* main header (14 bytes) */
  rputc(out,&i,'B'); rputc(out,&i,'M');      /* BMP file magic number */
  rputint(out,&i,n); rputint(out,&i,0);
  rputint(out,&i,54);  /* offset to bitmap */

  /* info header (40 bytes) */
  rputint(out,&i,40);  /* size of info header */
  rputint(out,&i,in->ncol); rputint(out,&i,in->nrow); /* size of image */
  rputint(out,&i,1+24*(1<<16)); rputint(out,&i,0);    
  rputint(out,&i,linesize*in->nrow);  /* size of data */
  rputint(out,&i,75*39); rputint(out,&i,75*39); 
  rputint(out,&i,0); rputint(out,&i,0);          

  /* write data */
  for (y=in->nrow;y--;) {
    adr = y*in->ncol;
    for (x=0; x<in->ncol; x++,adr++) {
      rputc(out,&i,in->blue[adr]);
      rputc(out,&i,in->green[adr]);
      rputc(out,&i,in->red[adr]);
    }
    /* padding bytes */
    for (x=0; x<linesize-in->ncol*3; x++) rputc(out,&i,0);
  }

  if (i!=out->size) mwerror(FATAL,1,"Inconsistant data size.");
  return(out);
}




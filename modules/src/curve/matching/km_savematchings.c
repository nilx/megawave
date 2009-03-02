/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {km_savematchings};
version = {"1.0"};
author = {"Jose-Luis Lisani, Pablo Muse, Frederic Sur"};
function = {"Save matching pieces of curves for display"};
usage = {
   matching_pieces->matching_pieces 
      "second output of km_match_ai or km_match_si (a 7-dim Flist)",
   levlines1->levlines1 
      "meaningful boundaries of image 1",
   levlines2->levlines2 
      "meaningful boundaries of image 2",
   matching_lines1<-aux1
      "output Flists of curves from image 1 that contain at least one matching piece of curve",
   matching_lines2<-aux2
      "output Flists of curves from image 2 that contain at least one matching piece of curve",
   matching_pieces1<-pieceaux1
      "output Flists of matching pieces of curve from image 1",   
   matching_pieces2<-pieceaux2
      "output Flists of matching pieces of curve from image 2"   
        };
*/

#include "mw.h"
#include "mw-modules.h" 


#define _(a,i,j) ((a)->values[(i)*((a)->dim)+(j)])

static char Closed;
static int N_Points;

static int get_next_index(int i, int iLast, unsigned char type)
{
  int i_next;

  if (type == 1) {
    if ((Closed) && (iLast == 0)) iLast=N_Points-1;
  } else {
    if ((Closed) && (iLast == N_Points-1)) iLast=0;
  }
  
  if (type == 1) {
    i_next=i+1;
    if ((i_next > N_Points-1) && (!Closed)) return -1;
    if ((i_next > N_Points-1) && (Closed)) i_next=1;
  } else {
    i_next=i-1;
    if ((i_next < 0) && (!Closed)) return -1;
    if ((i_next < 0) && (Closed)) i_next=N_Points-1;
  }
  if (i_next == iLast) return -1;
  
  return i_next;
}


/*------------------------------ MAIN MODULE ------------------------------*/

void km_savematchings(Flist matching_pieces, Flists levlines1, Flists levlines2, Flists aux1, Flists aux2, Flists pieceaux1, Flists pieceaux2)
{ 
  int m, n, piecesize, inext, ilast;
  
  if(matching_pieces->size == 0) 
    mwerror(FATAL,1,"error : Flists matching_pieces is empty\n");
  if(levlines1->size == 0) 
    mwerror(FATAL,1,"error : Flists levlines1 is empty\n");
  if(levlines2->size == 0) 
    mwerror(FATAL,1,"error : Flists levlines2 is empty\n");
  
  if((aux1=mw_change_flists(aux1,matching_pieces->size,matching_pieces->size))==NULL) mwerror (FATAL,1,"error, not enough memory\n");
  
  if ((aux2=mw_change_flists(aux2,matching_pieces->size,matching_pieces->size))==NULL) mwerror (FATAL,1,"error, not enough memory\n");
  if ((pieceaux1=mw_change_flists(pieceaux1,matching_pieces->size,matching_pieces->size))==NULL) mwerror (FATAL,1,"error, not enough memory\n");
  if ((pieceaux2=mw_change_flists(pieceaux2,matching_pieces->size,matching_pieces->size))==NULL) mwerror (FATAL,1,"error, not enough memory\n");


  for(m=0;m<matching_pieces->size;m++){
    
    aux1->list[m] = mw_copy_flist(levlines1->list[(int)_(matching_pieces,m,0)],NULL);
    aux2->list[m] = mw_copy_flist(levlines2->list[(int)_(matching_pieces,m,1)],NULL);
    
    Closed=((_(levlines1->list[(int)_(matching_pieces,m,0)],0,0)==
	     _(levlines1->list[(int)_(matching_pieces,m,0)],levlines1->list[(int)_(matching_pieces,m,0)]->size-1,0))
	    &&(_(levlines1->list[(int)_(matching_pieces,m,0)],0,1)==
	       _(levlines1->list[(int)_(matching_pieces,m,0)],levlines1->list[(int)_(matching_pieces,m,0)]->size-1,1)));
    
    N_Points=levlines1->list[(int)_(matching_pieces,m,0)]->size;
    
    piecesize = (int)(_(matching_pieces,m,3)-_(matching_pieces,m,2));
    if (piecesize < 0) piecesize = piecesize + N_Points;
    
    if (piecesize == 0)
      pieceaux1->list[m] = mw_copy_flist(levlines1->list[(int)_(matching_pieces,m,0)],NULL);
    else {
      pieceaux1->list[m] = mw_change_flist(NULL,piecesize,piecesize,2);
      inext = (int)_(matching_pieces,m,2);
      ilast = (int)_(matching_pieces,m,3)+1;
      
      for(n=0;n<piecesize;n++){
	_(pieceaux1->list[m],n,0) = _(levlines1->list[(int)_(matching_pieces,m,0)],inext,0);
	_(pieceaux1->list[m],n,1) = _(levlines1->list[(int)_(matching_pieces,m,0)],inext,1);
	inext = get_next_index(inext, ilast, 1);
      }
    }
    
    Closed=((_(levlines2->list[(int)_(matching_pieces,m,1)],0,0)==
	     _(levlines2->list[(int)_(matching_pieces,m,1)],levlines2->list[(int)_(matching_pieces,m,1)]->size-1,0))
	    &&(_(levlines2->list[(int)_(matching_pieces,m,1)],0,1)==
	       _(levlines2->list[(int)_(matching_pieces,m,1)],levlines2->list[(int)_(matching_pieces,m,1)]->size-1,1)));
    
    N_Points=levlines2->list[(int)_(matching_pieces,m,1)]->size;
    
    piecesize = (int)(_(matching_pieces,m,5)-_(matching_pieces,m,4));
    if (piecesize < 0) piecesize = piecesize + N_Points;
    
    if (piecesize == 0)
      pieceaux2->list[m] = mw_copy_flist(levlines2->list[(int)_(matching_pieces,m,1)],NULL);
    else {
      pieceaux2->list[m] = mw_change_flist(NULL,piecesize,piecesize,2);
      inext = (int)_(matching_pieces,m,4);
      ilast = (int)_(matching_pieces,m,5)+1;
      
      for(n=0;n<piecesize;n++){
	_(pieceaux2->list[m],n,0) = _(levlines2->list[(int)_(matching_pieces,m,1)],inext,0);
	_(pieceaux2->list[m],n,1) = _(levlines2->list[(int)_(matching_pieces,m,1)],inext,1);
	inext = get_next_index(inext, ilast, 1);
      }
    }
  }
}

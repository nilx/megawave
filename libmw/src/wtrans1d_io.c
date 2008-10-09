/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  wtrans1d_io.c
   
  Vers. 1.2
  (C) 1993-2000 Jacques Froment
  Input/Output private functions for the Wtrans1d structure

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>

#include "libmw-defs.h"
#include "utils.h"

#include "wtrans1d.h"
#include "ascii_file.h"
#include "fsignal_io.h"
#include "file_type.h"

#include "wtrans1d_io.h"

/*  Load the Header Ascii file and define the wtrans */

Wtrans1d _mw_load_wtrans1d_header(char *fname)
{
     FILE    *fp;
     Wtrans1d wtrans;
     int i;
     char field[15];

     fp = _mw_open_data_ascii_file(fname);
     if (fp == NULL) return(NULL);

     if (_mw_fascii_search_string(fp,"def Wtrans1d\n") == EOF)
     {
	  mwerror(ERROR, 0,
		  "No Wtrans1d description found in the file \"%s\"\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     wtrans = mw_new_wtrans1d();
     if (wtrans == NULL) { fclose(fp); return(wtrans); }

     if ((_mw_fascii_get_field(fp,fname,"comments:","%[^\n]",wtrans->cmt) != 1)
	 ||
	 (_mw_fascii_get_field(fp,fname,"type:","%d\n",&wtrans->type) != 1)
	 ||
	 (_mw_fascii_get_field(fp,fname,"edges:","%d\n",&wtrans->edges) != 1)
	 ||
	 (_mw_fascii_get_field(fp,fname,"size:","%d\n",&wtrans->size) != 1)
	 ||
	 (_mw_fascii_get_field(fp,fname,"nlevel:","%d\n",&wtrans->nlevel) != 1)
	 ||
	 (_mw_fascii_get_field(fp,fname,"nvoice:","%d\n",&wtrans->nvoice) != 1)
	 ||
	 (_mw_fascii_get_field(fp,fname,"complex:","%d\n",&wtrans->complex) != 1)
	 ||
	 (_mw_fascii_get_field(fp,fname,"nfilter:","%d\n",&wtrans->nfilter) != 1)
	  )
     {
	  mw_delete_wtrans1d(wtrans);
	  fclose(fp);
	  return(NULL);
     }

     if (wtrans->nlevel > mw_max_nlevel)
     {
	  mwerror(ERROR, 0,
		  "Too many levels defined in the Wtrans1d description\n");
	  mw_delete_wtrans1d(wtrans);
	  fclose(fp);
	  return(NULL);
     }	

     if (wtrans->size <= 0)
     {
	  mwerror(ERROR, 0,
		  "Illegal image size %d in the Wtrans1d description\n",
		  wtrans->size);
	  mw_delete_wtrans1d(wtrans);
	  fclose(fp);
	  return(NULL);
     }	

     if (wtrans->nvoice > mw_max_nvoice)
     {
	  mwerror(ERROR, 0,
		  "Too many voices defined in the Wtrans1d description\n");
	  mw_delete_wtrans1d(wtrans);
	  fclose(fp);
	  return(NULL);
     }	

     if (wtrans->nfilter > mw_max_nfilter_1d)
     {
	  mwerror(ERROR, 0,
		  "Too many filters defined in the Wtrans1d description\n");
	  mw_delete_wtrans1d(wtrans);
	  fclose(fp);
	  return(NULL);
     }	

     for (i=0;i<wtrans->nfilter;i++)
     {
	  sprintf(field,"filter #%d:",i+1);
	  if (_mw_fascii_get_field(fp,fname,field,"%[^\n]",wtrans->filter_name[i]) != 1)
	  {
	       mw_delete_wtrans1d(wtrans);
	       fclose(fp);
	       return(NULL);
	  }
     }
      
     fclose(fp);

     /* Alloc the signal fields */

     switch (wtrans->type)
     {
     case mw_orthogonal:
	  if (mw_alloc_ortho_wtrans1d(wtrans,wtrans->nlevel,wtrans->size) == NULL)
	  {
	       mw_delete_wtrans1d(wtrans);
	       return(NULL);
	  }
	  break;

     case mw_biorthogonal:
	  if (mw_alloc_biortho_wtrans1d(wtrans,wtrans->nlevel,wtrans->size)==NULL)
	  {
	       mw_delete_wtrans1d(wtrans);
	       return(NULL);
	  }
	  break;

     case mw_dyadic:
	  if (mw_alloc_dyadic_wtrans1d(wtrans,wtrans->nlevel,wtrans->size) == NULL)
	  {
	       mw_delete_wtrans1d(wtrans);
	       return(NULL);
	  }
	  break;

     case mw_continuous:
	  if (mw_alloc_continuous_wtrans1d(wtrans,wtrans->nlevel,
					   wtrans->nvoice,wtrans->size,wtrans->complex) == NULL)
	  {
	       mw_delete_wtrans1d(wtrans);
	       return(NULL);
	  }
	  break;

     default:
	  mwerror(ERROR, 0,
		  "Unrecognized type #%d in the Wtrans1d description\n",
		  wtrans->type);
	  mw_delete_wtrans1d(wtrans);
	  return(NULL);
	  break;
     }

     return(wtrans);
}


/*  Create the Header Ascii file from a wtrans */

short _mw_create_wtrans1d_header(char *fname, Wtrans1d wtrans)
{
     FILE *fp;
     int i;

     if (wtrans == NULL)
	  mwerror(INTERNAL,1,"Cannot create file: Wtrans1d structure is NULL\n");

     fp =_mw_create_data_ascii_file(fname);
     if (fp == NULL) return(-1);

     fprintf(fp,"%%----- Wtrans1d -----\n");
     fprintf(fp,"def Wtrans1d\n");

     fprintf(fp,"comments: %s\n",wtrans->cmt);
     fprintf(fp,"type: %d\n",wtrans->type);
     fprintf(fp,"edges: %d\n",wtrans->edges);
     fprintf(fp,"size: %d\n",wtrans->size);
     fprintf(fp,"nlevel: %d\n",wtrans->nlevel);
     fprintf(fp,"nvoice: %d\n",wtrans->nvoice);
     fprintf(fp,"complex: %d\n\n",wtrans->complex);
     fprintf(fp,"nfilter: %d\n",wtrans->nfilter);

     for (i=0;i<wtrans->nfilter;i++) 
	  fprintf(fp,"filter #%d: %s\n",i+1,wtrans->filter_name[i]);
      
     fclose(fp);
     return(0);
}


/* Load a signal field A[][], AP[][], D[][] or DP[][] */

void *_mw_wtrans1d_load_signal_wtrans(char *fname, char *type, 
				      Wtrans1d wtrans, Fsignal (*S)[50], 
				      char *Sname)
{
     FILE * fp;
     int j,v,size;
     char wfname[BUFSIZ];
     char type_in[mw_ftype_size];
     Fsignal signal;
     char sizedif = 0;         /* 1 if not the expected size in the signal */

     if (wtrans == NULL) return(NULL);

     for (j=0;j<=wtrans->nlevel;j++) 
	  for (v=0;v<wtrans->nvoice;v++) 
	       if ((j>0)||(v>0))
	       {
		    if (v == 0)
			 sprintf(wfname,"%s_%2.2d_%s.wtrans1d",fname,j,Sname);
		    else
			 sprintf(wfname,"%s_%2.2d.%2.2d_%s.wtrans1d",fname,j,v,Sname);
		    
		    fp = fopen(wfname,"r");
		    if (NULL == fp)
		    {
			 mwerror(ERROR, 0,
				 "Cannot find wavelet coeff. file \"%s\"\n",wfname);
			 mw_delete_wtrans1d(wtrans);	
			 return(NULL);
		    }
		    fclose(fp);
		    
		    signal = S[j][v];
		    if (signal == NULL)
			 mwerror(INTERNAL,0,"[_mw_wtrans1d_load_wtrans] NULL wtrans->%s[%d][%d]\n",Sname,j,v);
		    
		    size = signal->size; 
		    
		    /* Load without any new memory allocation */
		    signal = (Fsignal) _mw_load_fsignal(wfname, type_in, signal);
		    _mw_make_type(type,type_in,"fsignal");
		    
		    if (signal == NULL)
			 mwerror(INTERNAL,0,"[_mw_wtrans1d_load_wtrans] NULL signal returned in loading file \"%s\"\n",wfname);
		    if (size != signal->size) 
		    {
			 if (sizedif == 0)
			      mwerror(WARNING, 0,"Unexpected size %d of the signal in \"%s\" (disable size control)\n",signal->size, wfname);
			 sizedif = 1;
/* Erreur Fatale supprimee suite a la demande de Jean-Pierre d'Ales 
   mw_delete_wtrans1d(wtrans);
   return(NULL);
*/
			 
		    }
		    
/* Erreur Fatale supprimee suite a la demande de Jean-Pierre d'Ales 
   if (signal != S[j][v])
   mwerror(INTERNAL,0,"[_mw_wtrans1d_load_wtrans] New memory allocation done by _mw_load_fsignal in loading \"%s\"\n",wfname);
*/
		    
	       }
     return(wtrans);
}


Wtrans1d _mw_wtrans1d_load_wtrans(char *fname, char *type)
{
     Wtrans1d wtrans;               
     int use_average;

     wtrans = _mw_load_wtrans1d_header(fname);
     if (wtrans == NULL) return(NULL);

     if (wtrans->nlevel > 0)
     {
	  if (wtrans->A[1][0] != NULL) use_average = 1; else
	       use_average = 0;
     }
     else
     {
	  if (wtrans->A[0][1] != NULL) use_average = 1; else
	       use_average = 0;
     }

     if ((use_average == 1) && 
	 (_mw_wtrans1d_load_signal_wtrans(fname,type,wtrans,wtrans->A,"A") == NULL)) return(NULL);

     if (_mw_wtrans1d_load_signal_wtrans(fname,type,wtrans,wtrans->D,"D") == NULL)
	  return(NULL);

     if (wtrans->complex == 1)
     {
	  if ((use_average == 1) &&
	      (_mw_wtrans1d_load_signal_wtrans(fname,type,wtrans,wtrans->AP,"AP")
	       == NULL)) return(NULL);
	  if (_mw_wtrans1d_load_signal_wtrans(fname,type,wtrans,wtrans->DP,"DP")
	      == NULL) return(NULL);
     }

     return(wtrans);
}

/* Create a signal field A[][], AP[][], D[][] or DP[][] */

void _mw_wtrans1d_create_signal_wtrans(char *fname, char *type, 
				       Wtrans1d wtrans, Fsignal (*S)[50], 
				       char *Sname)
{
     int j,v;
     char wfname[BUFSIZ];
     Fsignal signal;

     for (j=0;j<=wtrans->nlevel;j++) 
	  for (v=0;v<wtrans->nvoice;v++) 
	       if ((j>0)||(v>0))
	       {
		    if (v == 0)
			 sprintf(wfname,"%s_%2.2d_%s.wtrans1d",fname,j,Sname);
		    else
			 sprintf(wfname,"%s_%2.2d.%2.2d_%s.wtrans1d",fname,j,v,Sname);
		    signal = S[j][v];
		    if (signal == NULL)
			 mwerror(INTERNAL,0,"[_mw_wtrans1d_create_signal_wtrans] NULL wtrans->%s[%d][%d]\n",Sname,j,v);
		    
		    _mw_create_fsignal(wfname,signal,type);
		    
	       }
}


short _mw_wtrans1d_create_wtrans(char *fname, Wtrans1d wtrans, char *type)
{
     int use_average;

     if (wtrans == NULL) 
     {
	  mwerror(INTERNAL, 0,"[_mw_wtrans1d_create_wtrans] Cannot create wtrans1d : NULL wtrans.\n");
	  return(-1);
     }

     if (_mw_create_wtrans1d_header(fname,wtrans) != 0) return(-1);

     if (wtrans->nlevel > 0)
     {
	  if (wtrans->A[1][0] != NULL) use_average = 1; else
	       use_average = 0;
     }
     else
     {
	  if (wtrans->A[0][1] != NULL) use_average = 1; else
	       use_average = 0;
     }
  
     if (use_average == 1)
	  _mw_wtrans1d_create_signal_wtrans(fname,type,wtrans,wtrans->A,"A");
     _mw_wtrans1d_create_signal_wtrans(fname,type,wtrans,wtrans->D,"D");

     if (wtrans->complex == 1)
     {
	  if (use_average == 1)
	       _mw_wtrans1d_create_signal_wtrans(fname,type,wtrans,wtrans->AP,"AP");
	  _mw_wtrans1d_create_signal_wtrans(fname,type,wtrans,wtrans->DP,"DP");
     }

     return(0);
}

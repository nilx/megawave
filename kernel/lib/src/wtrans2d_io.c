/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   wtrans2d_io.c
   
   Vers. 1.2
   (C) 1993-2000 Jacques Froment
   Input/Output private functions for the Wtrans2d structure

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>

#include "ascii_file.h"
#include "mw.h"

/*  Load the Header Ascii file and define the wtrans */

Wtrans2d _mw_load_wtrans2d_header(fname)

char *fname; /* Name of the generic wavelet decomposition (Header Ascii file)*/


{ FILE    *fp;
  Wtrans2d wtrans;
  int i;
  char field[15];

  fp = _mw_open_data_ascii_file(fname);
  if (fp == NULL) return(NULL);

  if (_mw_fascii_search_string(fp,"def Wtrans2d\n") == EOF)
    {
      mwerror(ERROR, 0,
	      "No Wtrans2d description found in the file \"%s\"\n",fname);
      fclose(fp);
      return(NULL);
    }

  wtrans = mw_new_wtrans2d();
  if (wtrans == NULL) { fclose(fp); return(wtrans); }

  if ((_mw_fascii_get_field(fp,fname,"comments:","%[^\n]",wtrans->cmt) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"type:","%d\n",&wtrans->type) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"edges:","%d\n",&wtrans->edges) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"nrow:","%d\n",&wtrans->nrow) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"ncol:","%d\n",&wtrans->ncol) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"nlevel:","%d\n",&wtrans->nlevel) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"norient:","%d\n",&wtrans->norient) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"nfilter:","%d\n",&wtrans->nfilter) != 1)
      )
    {
      mw_delete_wtrans2d(wtrans);
      fclose(fp);
      return(NULL);
    }

  if (wtrans->nlevel > mw_max_nlevel)
    {
      mwerror(ERROR, 0,
	      "Too many levels defined in the Wtrans2d description\n");
      mw_delete_wtrans2d(wtrans);
      fclose(fp);
      return(NULL);
    }	

  if ((wtrans->nrow <= 0) || (wtrans->ncol <=0))
    {
      mwerror(ERROR, 0,
	      "Illegal image size (%d,%d) in the Wtrans2d description\n",
	      wtrans->ncol,wtrans->nrow);
      mw_delete_wtrans2d(wtrans);
      fclose(fp);
      return(NULL);
    }	

  if (wtrans->norient > mw_max_norient)
    {
      mwerror(ERROR, 0,
	      "Too many orientations defined in the Wtrans2d description\n");
      mw_delete_wtrans2d(wtrans);
      fclose(fp);
      return(NULL);
    }	

  if (wtrans->nfilter > mw_max_nfilter)
    {
      mwerror(ERROR, 0,
	      "Too many filters defined in the Wtrans2d description\n");
      mw_delete_wtrans2d(wtrans);
      fclose(fp);
      return(NULL);
    }	

  for (i=0;i<wtrans->nfilter;i++)
    {
      sprintf(field,"filter #%d:",i+1);
      if (_mw_fascii_get_field(fp,fname,field,"%[^\n]",wtrans->filter_name[i]) != 1)
	{
	  mw_delete_wtrans2d(wtrans);
	  fclose(fp);
	  return(NULL);
	}
    }
      
  fclose(fp);

  /* Alloc the wtrans images[][] field */

  /*
  switch (wtrans->type)
    {
    case mw_orthogonal:
      if (mw_alloc_ortho_wtrans2d(wtrans,wtrans->nlevel,wtrans->nrow,
				  wtrans->ncol) == NULL)
	{
	  mw_delete_wtrans2d(wtrans);
	  return(NULL);
	}
      break;

    case mw_biorthogonal:
      if (mw_alloc_biortho_wtrans2d(wtrans,wtrans->nlevel,wtrans->nrow,
				  wtrans->ncol) == NULL)
	{
	  mw_delete_wtrans2d(wtrans);
	  return(NULL);
	}
      break;

    case mw_dyadic:
      if (mw_alloc_dyadic_wtrans2d(wtrans,wtrans->nlevel,wtrans->nrow,
				  wtrans->ncol) == NULL)
	{
	  mw_delete_wtrans2d(wtrans);
	  return(NULL);
	}
      break;

    default:
      mwerror(ERROR, 0,
	      "Unrecognized type #%d in the Wtrans2d description\n",
	      wtrans->type);
      mw_delete_wtrans2d(wtrans);
      return(NULL);
      break;
    }
  */

  return(wtrans);
}


/*  Create the Header Ascii file from a wtrans */

short _mw_create_wtrans2d_header(fname,wtrans)

char  *fname;                        /* file name */
Wtrans2d wtrans;

{
  FILE *fp;
  int i;

  if (wtrans == NULL)
    mwerror(INTERNAL,1,"Cannot create file: Wtrans2d structure is NULL\n");

  fp =_mw_create_data_ascii_file(fname);
  if (fp == NULL) return(-1);

  fprintf(fp,"%%----- Wtrans2d -----\n");
  fprintf(fp,"def Wtrans2d\n");

  fprintf(fp,"comments: %s\n",wtrans->cmt);

  fprintf(fp,"type: %d\n",wtrans->type);
  fprintf(fp,"edges: %d\n",wtrans->edges);
  fprintf(fp,"nrow: %d\n",wtrans->nrow);
  fprintf(fp,"ncol: %d\n",wtrans->ncol);
  fprintf(fp,"nlevel: %d\n",wtrans->nlevel);
  fprintf(fp,"norient: %d\n\n",wtrans->norient);
  fprintf(fp,"nfilter: %d\n",wtrans->nfilter);

  for (i=0;i<wtrans->nfilter;i++) 
    fprintf(fp,"filter #%d: %s\n",i+1,wtrans->filter_name[i]);
      
  fclose(fp);
  return(0);
}



Wtrans2d _mw_wtrans2d_load_wtrans(fname,type)

char *fname; /* Name of the generic wavelet decomposition (Header Ascii file)*/
char *type;  /* External type of the wavelet coefficients files */
             
{
  Wtrans2d wtrans;               
  int f,j,d,dx,dy;
  char wfname[BUFSIZ];
  char type_in[mw_ftype_size];
  Fimage image;
  char sizedif = 0;         /* 1 if not the expected size in the image */

  wtrans = _mw_load_wtrans2d_header(fname);
  if (wtrans == NULL) return(NULL);

  for (j=1;j<=wtrans->nlevel;j++) for (d=0;d<=wtrans->norient;d++)
    {
      if (d == 0)
	sprintf(wfname,"%s_%2.2d_S.wtrans2d",fname,j);
      else
	sprintf(wfname,"%s_%2.2d_D%d.wtrans2d",fname,j,d);
      f = open(wfname,O_RDONLY);
      if ((f == -1) && ((d != 0) || (j == wtrans->nlevel)))
	{
	  mwerror(ERROR, 0,"Cannot find wavelet coeff. file \"%s\"\n",wfname);
	  mw_delete_wtrans2d(wtrans);	
	  return(NULL);
	}
      close(f);

      /* Load with new memory allocation */
      image = (Fimage) _mw_fimage_load_image(wfname, type_in);
      _mw_make_type(type,type_in,"fimage");

      if (image == NULL)
	mwerror(INTERNAL,0,"[_mw_wtrans2d_load_wtrans] NULL image returned in loading file \"%s\"\n",wfname);

      wtrans->images[j][d]=image;
    }

  return(wtrans);
}


short _mw_wtrans2d_create_wtrans(fname,wtrans,type)

char *fname; /* Name of the generic wavelet decomposition (Header Ascii file)*/
char *type;  /* External type of the wavelet coefficients files */
Wtrans2d wtrans;

{
  int j,d;
  char wfname[BUFSIZ];
  Fimage image;

  if (wtrans == NULL) 
    {
      mwerror(INTERNAL, 0,"[_mw_wtrans2d_create_wtrans] Cannot create wtrans2d : NULL wtrans.\n");
      return(-1);
    }

  if (_mw_create_wtrans2d_header(fname,wtrans) != 0) return(-1);
  
  for (j=1;j<=wtrans->nlevel;j++) for (d=0;d<=wtrans->norient;d++)
    {
      if (d == 0)
	sprintf(wfname,"%s_%2.2d_S.wtrans2d",fname,j);
      else
	sprintf(wfname,"%s_%2.2d_D%d.wtrans2d",fname,j,d);

      image = wtrans->images[j][d];
      if (image == NULL)
	mwerror(INTERNAL,0,"[_mw_wtrans2d_create_wtrans] NULL wtrans->image[%d][%d]\n",j,d);

      _mw_fimage_create_image(wfname, image, type);
    }

  return(0);
}








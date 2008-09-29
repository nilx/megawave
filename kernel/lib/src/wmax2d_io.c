/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  wmax2d_io.c
   
  Vers. 1.0
  (C) 1993 Jacques Froment
  Input/Output functions for the 2D Wavelet Maxima structures

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
#include "ascii_file.h"
#include "wmax2d.h"

#include "wmax2d_io.h"

/* ----- Virtual Maxima points & chains ----- */

/* Set of vchain */

Vchains_wmax _mw_load_vchains_wmax(char *fname)
{
     FILE    *fp;
     Vchains_wmax vchains;
     register Vchain_wmax newvc;
     Vchain_wmax oldvc;
     register Vpoint_wmax newvp;
     Vpoint_wmax oldvp;
     int px,py,vcs_size;
     float pargp;
     register int n;

     fp = _mw_open_data_ascii_file(fname);
     if (fp == NULL) return(NULL);

     if (_mw_fascii_search_string(fp,"def Vchains_wmax\n") == EOF)
     {
	  mwerror(ERROR, 0,
		  "No Vchains_wmax description found in the file \"%s\"\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     vchains = mw_new_vchains_wmax();
     if (vchains == NULL) return(vchains);

     if ((_mw_fascii_get_field(fp,fname,"comments:","%[^\n]",vchains->cmt) != 1)
	 ||
	 (_mw_fascii_get_field(fp,fname,"size:","%d",&vchains->size) != 1) 
	 ||
	 (_mw_fascii_get_field(fp,fname,"ref_level:","%d",&vchains->ref_level) != 1) 
	 ||
	 (_mw_fascii_get_field(fp,fname,"nlevel:","%d",&vchains->nlevel) != 1) 
	 ||
	 (_mw_fascii_get_field(fp,fname,"nrow:","%d",&vchains->nrow) != 1) 
	 ||
	 (_mw_fascii_get_field(fp,fname,"ncol:","%d",&vchains->ncol) != 1) 
	  )
     {
	  mw_delete_vchains_wmax(vchains);
	  fclose(fp);
	  return(NULL);
     }
  
     if (_mw_fascii_search_string(fp,"def Vchain_wmax\n") == EOF)
     {
	  mwerror(ERROR, 0,
		  "No Vchain_wmax description found in the file \"%s\"\n",fname);
	  mw_delete_vchains_wmax(vchains);
	  fclose(fp);
	  return(NULL);
     }


     oldvc = newvc = NULL;
     vcs_size = 0;

     do 
     {
	  vcs_size++;

	  newvc = mw_new_vchain_wmax();
	  if (newvc == NULL)
	  {
	       mw_delete_vchains_wmax(vchains);
	       fclose(fp);
	       return(NULL);
	  }
	  if (vchains->first == NULL) vchains->first = newvc;
	  if (oldvc != NULL) oldvc->next = newvc;
	  newvc->previous = oldvc;
	  newvc->next = NULL;
	  newvc->first = NULL;
	  oldvp = newvp = NULL;

	  if (_mw_fascii_get_field(fp,fname,"size:","%d",&newvc->size) != 1) 
	  {
	       mw_delete_vchains_wmax(vchains);
	       fclose(fp);
	       return(NULL);
	  }

	  while (fscanf(fp,"%d,%d: argp=%f\n",&px,&py,&pargp) ==  3)
	  {
	       newvp = mw_new_vpoint_wmax();
	       if (newvp == NULL)
	       {
		    mw_delete_vchains_wmax(vchains);
		    fclose(fp);
		    return(NULL);
	       }
	       if (newvc->first == NULL) newvc->first = newvp;
	       if (oldvp != NULL) oldvp->next = newvp;
	       newvp->previous = oldvp;
	       newvp->next = NULL;
	       newvp->x = px; newvp->y = py;
	       newvp->argp = pargp;
	       for (n=1;n <= vchains->nlevel;n++)
		    if (fscanf(fp,"\t%f,%f\n",&newvp->mag[n-1],&newvp->arg[n-1]) !=  2)
		    {
			 mw_delete_vchains_wmax(vchains);
			 fclose(fp);
			 return(NULL);
		    }
	       oldvp = newvp;
	  } 
	  oldvc = newvc;
     } while (_mw_fascii_search_string(fp,"def Vchain_wmax\n") != EOF);
      
     fclose(fp);

     if (vcs_size != vchains->size)
     {
	  mwerror(WARNING,1,"%d vchains have be found in the file \"%s\" but the header records %d vchains.\n",vcs_size,fname,vchains->size);
	  vchains->size = vcs_size;
     }

     return(vchains);
}

short _mw_create_vchains_wmax(char *fname, Vchains_wmax vchains)
{
     FILE *fp;
     Vchain_wmax vc;
     register Vpoint_wmax vp;
     register int n,l;

     if (vchains == NULL)
	  mwerror(INTERNAL,1,"Cannot create file: Vchains_wmax structure is NULL\n");

     if (vchains->first == NULL)
	  mwerror(INTERNAL,1,
		  "Cannot create file: No vchain_wmax in the Vchains_wmax structure\n");

     fp =_mw_create_data_ascii_file(fname);
     if (fp == NULL) return(-1);
  
     fprintf(fp,"%%\n");
     fprintf(fp,"%%----- Vchains_wmax -----\n");
     fprintf(fp,"def Vchains_wmax\n");

     fprintf(fp,"comments: %s\n",vchains->cmt);
     fprintf(fp,"size: %d\n",vchains->size);
     fprintf(fp,"ref_level: %d\n",vchains->ref_level);
     fprintf(fp,"nlevel: %d\n",vchains->nlevel);
     fprintf(fp,"nrow: %d\n",vchains->nrow);
     fprintf(fp,"ncol: %d\n",vchains->ncol);
  
     for (vc=vchains->first, n=1; vc; vc=vc->next, n++)
     {
	  if (vc->first == NULL)
	       mwerror(INTERNAL,1,"Vchain_wmax #%d has no vpoint !\n",n);
	  fprintf(fp,"%%\n");
	  fprintf(fp,"def Vchain_wmax\n");
	  fprintf(fp,"size: %d\n",vc->size);
      
	  for (vp=vc->first; vp; vp=vp->next)
	  {
	       fprintf(fp,"%d,%d: argp=%e\n",vp->x,vp->y,vp->argp);
	       for (l=1;l<=vchains->nlevel;l++)
		    fprintf(fp,"\t%e,%e\n",vp->mag[l-1],vp->arg[l-1]);
	  }
     }
      
     fclose(fp);
     return(0);
}
   

/* A vchain alone */

Vchain_wmax _mw_load_vchain_wmax(char *fname)
{
     FILE    *fp;
     Vchain_wmax vchain;
     register Vpoint_wmax newvp;
     Vpoint_wmax oldvp;
     int px,py;
     register int n;
     int nlevel;
     float pargp;

     fp = _mw_open_data_ascii_file(fname);
     if (fp == NULL) return(NULL);

     if (_mw_fascii_search_string(fp,"def Vchain_wmax\n") == EOF)
     {
	  mwerror(ERROR, 0,
		  "No Vchain_wmax description found in the file \"%s\"\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     vchain = mw_new_vchain_wmax();
     if (vchain == NULL)
     {
	  mw_delete_vchain_wmax(vchain);
	  fclose(fp);
	  return(NULL);
     }
     vchain->previous = NULL;
     vchain->next = NULL;
     vchain->first = NULL;
  
     if ((_mw_fascii_get_field(fp,fname,"size:","%d",&vchain->size) != 1) 
	 ||
	 (_mw_fascii_get_field(fp,fname,"nlevel:","%d",&nlevel) != 1) )
     {
	  mw_delete_vchain_wmax(vchain);
	  fclose(fp);
	  return(NULL);
     }
  
     oldvp = newvp = NULL;
     while (fscanf(fp,"%d,%d: argp=%f\n",&px,&py,&pargp) ==  3)
     {
	  newvp = mw_new_vpoint_wmax();
	  if (newvp == NULL)
	  {
	       mw_delete_vchain_wmax(vchain);
	       fclose(fp);
	       return(NULL);
	  }
	  if (vchain->first == NULL) vchain->first = newvp;
	  if (oldvp != NULL) oldvp->next = newvp;
	  newvp->previous = oldvp;
	  newvp->next = NULL;
	  newvp->x = px; newvp->y = py;
	  newvp->argp = pargp;

	  for (n=1;n<=nlevel;n++)
	       if (fscanf(fp,"\t%f,%f\n",&newvp->mag[n-1],&newvp->arg[n-1]) !=  2)
	       {
		    mw_delete_vchain_wmax(vchain);
		    fclose(fp);
		    return(NULL);
	       }
	  oldvp = newvp;
     } 
      
     fclose(fp);

     return(vchain);
}

short _mw_create_vchain_wmax(char *fname, Vchain_wmax vchain)
{
     FILE *fp;
     register Vpoint_wmax vp;
     register int l,nlevel;

     if (vchain == NULL)
	  mwerror(INTERNAL,1,"Cannot create file: Vchain_wmax structure is NULL\n");
  
     if (vchain->first == NULL)
	  mwerror(INTERNAL,1,"Cannot create file: Vchain_wmax has no vpoint !\n");

     fp =_mw_create_data_ascii_file(fname);
     if (fp == NULL) return(-1);

     /* Retrieve the nlevel value */
     for (nlevel=0; (nlevel < mw_max_nlevel) && 
	       ((vchain->first)->mag[nlevel] != -1); nlevel++);

     fprintf(fp,"%%\n");
     fprintf(fp,"%%----- Vchain_wmax -----\n");
     fprintf(fp,"%%\n");
     fprintf(fp,"def Vchain_wmax\n");
     fprintf(fp,"size: %d\n",vchain->size);
     fprintf(fp,"nlevel: %d\n",nlevel);
      
     for (vp=vchain->first; vp; vp=vp->next)
     {
	  fprintf(fp,"%d,%d: argp=%e\n",vp->x,vp->y,vp->argp);
	  for (l=1;l<=nlevel;l++)
	       fprintf(fp,"\t%e,%e\n",vp->mag[l-1],vp->arg[l-1]);
     }
      
     fclose(fp);
     return(0);
}
   






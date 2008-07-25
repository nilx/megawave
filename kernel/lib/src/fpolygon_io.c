/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  fpolygon_io.c
   
  Vers. 1.9
  (C) 1995-2002 Jacques Froment
  Input/Output functions for the fpolygon & fpolygons structures

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <sys/file.h>

#include "ascii_file.h"
#include "mw.h"


/* ---- I/O for Fpolygon ---- */

Fpolygon _mw_load_fpolygon_a_fpoly(char *fname)
{
     FILE    *fp;
     Fpolygon fpoly;
     Point_fcurve newpc,oldpc;
     float px,py;
     int i,nc;
     char channel[20];

     fp = _mw_open_data_ascii_file(fname);
     if (fp == NULL) return(NULL);

     if (_mw_fascii_search_string(fp,"def Fpolygon\n") == EOF)
     {
	  mwerror(ERROR, 0,
		  "No Fpolygon description found in the file \"%s\"",fname);
	  fclose(fp);
	  return(NULL);
     }

     fpoly = mw_new_fpolygon();
     if (fpoly == NULL) return(fpoly);

     if ((_mw_fascii_get_field(fp,fname,"nb_channels:","%d",&nc) != 1)
	 || (nc < 0))
     {
	  mw_delete_fpolygon(fpoly);
	  fclose(fp);
	  return(NULL);
     }
     
     if (nc > 0) fpoly = mw_change_fpolygon(fpoly,nc);
     if (fpoly == NULL)
     {
	  mw_delete_fpolygon(fpoly);
	  fclose(fp);
	  return(NULL);
     }

     for (i=0;i<nc;i++)
     {
	  sprintf(channel,"channel #%d:",i+1);
	  if (_mw_fascii_get_field(fp,fname,channel,"%f",&fpoly->channel[i]) != 1) 
	  {
	       mw_delete_fpolygon(fpoly);
	       fclose(fp);
	       return(NULL);
	  }
     }

     oldpc = NULL;
     while (fscanf(fp,"%f,%f\n",&px,&py) ==  2)
     {
	  newpc = mw_new_point_fcurve();
	  if (newpc == NULL)
	  {
	       mw_delete_fpolygon(fpoly);
	       fclose(fp);
	       return(NULL);
	  }
	  if (fpoly->first == NULL) fpoly->first = newpc;
	  if (oldpc != NULL) oldpc->next = newpc;
	  newpc->previous = oldpc;
	  newpc->next = NULL;
	  newpc->x = px; newpc->y = py;
	  oldpc = newpc;
     }

     fclose(fp);
     return(fpoly);
}


/* Native formats (without conversion of the internal type) */

Fpolygon _mw_fpolygon_load_native(char *fname, char *type)
{
     if (strcmp(type,"A_FPOLY") == 0)
	  return((Fpolygon)_mw_load_fpolygon_a_fpoly(fname));

     return(NULL);
}


/* All available formats */

Fpolygon _mw_load_fpolygon(char *fname, char *type)
{ 
     Fpolygon poly;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     poly = _mw_fpolygon_load_native(fname,type);
     if (poly != NULL) return(poly);

     /* If failed, try other formats with memory conversion */
     poly = (Fpolygon) _mw_load_etype_to_itype(fname,mtype,"fpolygon",type);
     if (poly != NULL) return(poly);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Fpolygon !\n",fname,type);
}

/* Write file in A_POLY format */  

short _mw_create_fpolygon_a_fpoly(char *fname, Fpolygon fpoly)
{
     FILE *fp;
     Point_fcurve pc;
     int i;

     if (fpoly == NULL)
	  mwerror(INTERNAL,1,"Cannot create file: Fpolygon structure is NULL\n");

     if (fpoly->first == NULL)
	  mwerror(INTERNAL,1,
		  "Cannot create file: No point in the Fpolygon structure\n");

     fp =_mw_create_data_ascii_file(fname);
     if (fp == NULL) return(-1);

     fprintf(fp,"%%\n");
     fprintf(fp,"%%----- Fpolygon -----\n");
     fprintf(fp,"def Fpolygon\n");
     fprintf(fp,"nb_channels: %d\n",fpoly->nb_channels);
     for (i=0;i<fpoly->nb_channels;i++)
	  fprintf(fp,"channel #%d: %f\n",i+1,fpoly->channel[i]);
     for (pc=fpoly->first; pc; pc=pc->next)
	  fprintf(fp,"%f,%f\n",pc->x,pc->y);
  
     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_fpolygon_create_native(char *fname, Fpolygon fpoly, char *Type)
{
     if (strcmp(Type,"A_FPOLY") == 0)
	  return(_mw_create_fpolygon_a_fpoly(fname,fpoly));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_fpolygon(char *fname, Fpolygon fpoly, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_fpolygon_create_native(fname,fpoly,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,fpoly,"fpolygon",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
}

/* ---- I/O for Fpolygons ---- */

/* Load fpolygons from a file of A_FPOLY format */

Fpolygons _mw_load_fpolygons_a_fpoly(char *fname)
{
     FILE    *fp;
     Fpolygons poly;
     Fpolygon newp,oldp;
     Point_fcurve newpc,oldpc;
     int i,nc;
     float px,py;
     char channel[20];

     fp = _mw_open_data_ascii_file(fname);
     if (fp == NULL) return(NULL);

     if (_mw_fascii_search_string(fp,"def Fpolygon\n") == EOF)
     {
	  mwerror(ERROR, 0,
		  "No Fpolygon description found in the file \"%s\"",fname);
	  fclose(fp);
	  return(NULL);
     }

     poly = mw_new_fpolygons();
     if (poly == NULL) return(poly);
     oldp = newp = NULL;

     do 
     {
	  if ((_mw_fascii_get_field(fp,fname,"nb_channels:","%d",&nc) != 1)
	      || (nc <= 0))
	  {
	       mw_delete_fpolygons(poly);
	       fclose(fp);
	       return(NULL);
	  }
      
	  newp = mw_change_fpolygon(NULL,nc);
	  if (newp == NULL)
	  {
	       mw_delete_fpolygons(poly);
	       fclose(fp);
	       return(NULL);
	  }
	  if (poly->first == NULL) poly->first = newp;
	  if (oldp != NULL) oldp->next = newp;
	  newp->previous = oldp;
	  newp->next = NULL;
	  newp->first = NULL;
	  oldpc = newpc = NULL;

	  for (i=0;i<nc;i++)
	  {
	       sprintf(channel,"channel #%d:",i+1);
	       if (_mw_fascii_get_field(fp,fname,channel,"%f",&newp->channel[i]) != 1) 
	       {
		    mw_delete_fpolygons(poly);
		    fclose(fp);
		    return(NULL);
	       }
	  }

	  while (fscanf(fp,"%f,%f\n",&px,&py) ==  2)
	  {
	       newpc = mw_new_point_fcurve();
	       if (newpc == NULL)
	       {
		    mw_delete_fpolygons(poly);
		    fclose(fp);
		    return(NULL);
	       }
	       if (newp->first == NULL) newp->first = newpc;
	       if (oldpc != NULL) oldpc->next = newpc;
	       newpc->previous = oldpc;
	       newpc->next = NULL;
	       newpc->x = px; newpc->y = py;
	       oldpc = newpc;
	  }
	  oldp = newp;
     } while (_mw_fascii_search_string(fp,"def Fpolygon\n") != EOF);
      
     fclose(fp);

     return(poly);
}

/* Native formats (without conversion of the internal type) */

Fpolygons _mw_fpolygons_load_native(char *fname, char *type)
{
     if (strcmp(type,"A_FPOLY") == 0)
	  return((Fpolygons)_mw_load_fpolygons_a_fpoly(fname));

     return(NULL);
}


/* All available formats */

Fpolygons _mw_load_fpolygons(char *fname, char *type)
{ 
     Fpolygons poly;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     poly = _mw_fpolygons_load_native(fname,type);
     if (poly != NULL) return(poly);

     /* If failed, try other formats with memory conversion */
     poly = (Fpolygons) _mw_load_etype_to_itype(fname,mtype,"fpolygons",type);
     if (poly != NULL) return(poly);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Fpolygons !\n",fname,type);
}


/* Write file in A_FPOLY format */  

short _mw_create_fpolygons_a_fpoly(char *fname, Fpolygons poly)
{
     FILE *fp;
     Fpolygon pl;
     Point_fcurve pc;
     int i,n;

     if (poly == NULL)
	  mwerror(INTERNAL,1,"Cannot create file: Fpolygons structure is NULL\n");

     if (poly->first == NULL)
	  mwerror(INTERNAL,1,
		  "Cannot create file: No fpolygon in the Fpolygons structure\n");

     fp =_mw_create_data_ascii_file(fname);
     if (fp == NULL) return(-1);

     for (pl=poly->first, n=1; pl; pl=pl->next, n++)
     {
	  if (pl->first == NULL)
	       mwerror(INTERNAL,1,"Fpolygon #%d has no point fcurve\n",n);
	  fprintf(fp,"%%\n");
	  fprintf(fp,"%%----- Fpolygon #%d -----\n",n);
	  fprintf(fp,"def Fpolygon\n");
	  fprintf(fp,"nb_channels: %d\n",pl->nb_channels);
	  for (i=0;i<pl->nb_channels;i++)
	       fprintf(fp,"channel #%d: %f\n",i+1,pl->channel[i]);
      
	  for (pc=pl->first; pc; pc=pc->next)
	       fprintf(fp,"%f,%f\n",pc->x,pc->y);
     }
      
     fclose(fp);
     return(0);
}
   
/* Create native formats (without conversion of the internal type) */

short _mw_fpolygons_create_native(char *fname, Fpolygons fpoly, char *Type)
{
     if (strcmp(Type,"A_FPOLY") == 0)
	  return(_mw_create_fpolygons_a_fpoly(fname,fpoly));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_fpolygons(char *fname, Fpolygons fpoly, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_fpolygons_create_native(fname,fpoly,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,fpoly,"fpolygons",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
}

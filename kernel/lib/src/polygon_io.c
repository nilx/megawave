/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  polygon_io.c
   
  Vers. 1.11
  (C) 1993-2002 Jacques Froment
  Input/Output functions for the polygon & polygons structure

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <string.h>

#include "libmw-defs.h"
#include "utils.h"
#include "ascii_file.h"
#include "file_type.h"
#include "type_conv.h"
#include "polygon.h"
#include "curve.h"

#include "polygon_io.h"

/* ---- I/O for Polygon ---- */

/* Load polygon from a file of A_POLY format */

Polygon _mw_load_polygon_a_poly(char *fname)
{
     FILE    *fp;
     Polygon poly;
     Point_curve newpc,oldpc;
     int px,py,i,nc;
     char channel[20];

     fp = _mw_open_data_ascii_file(fname);
     if (fp == NULL) return(NULL);

     if (_mw_fascii_search_string(fp,"def Polygon\n") == EOF)
     {
	  mwerror(ERROR, 0,
		  "No Polygon description found in the file \"%s\"",fname);
	  fclose(fp);
	  return(NULL);
     }

     poly = mw_new_polygon();
     if (poly == NULL) return(poly);

     if ((_mw_fascii_get_field(fp,fname,"nb_channels:","%d",&nc) != 1)
	 || (nc < 0))
     {
	  mw_delete_polygon(poly);
	  fclose(fp);
	  return(NULL);
     }
      
     if (nc > 0) poly = mw_change_polygon(poly,nc);
     if (poly == NULL)
     {
	  mw_delete_polygon(poly);
	  fclose(fp);
	  return(NULL);
     }

     for (i=0;i<nc;i++)
     {
	  sprintf(channel,"channel #%d:",i+1);
	  if (_mw_fascii_get_field(fp,fname,channel,"%f",&poly->channel[i]) != 1) 
	  {
	       mw_delete_polygon(poly);
	       fclose(fp);
	       return(NULL);
	  }
     }

     oldpc = NULL;
     while (fscanf(fp,"%d,%d\n",&px,&py) ==  2)
     {
	  newpc = mw_new_point_curve();
	  if (newpc == NULL)
	  {
	       mw_delete_polygon(poly);
	       fclose(fp);
	       return(NULL);
	  }
	  if (poly->first == NULL) poly->first = newpc;
	  if (oldpc != NULL) oldpc->next = newpc;
	  newpc->previous = oldpc;
	  newpc->next = NULL;
	  newpc->x = px; newpc->y = py;
	  oldpc = newpc;
     }
     fclose(fp);

     return(poly);
}


/* Native formats (without conversion of the internal type) */

Polygon _mw_polygon_load_native(char *fname, char *type)
{
     if (strcmp(type,"A_POLY") == 0)
	  return((Polygon)_mw_load_polygon_a_poly(fname));

     return(NULL);
}


/* All available formats */

Polygon _mw_load_polygon(char *fname, char *type)
{ 
     Polygon poly;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     poly = _mw_polygon_load_native(fname,type);
     if (poly != NULL) return(poly);

     /* If failed, try other formats with memory conversion */
     poly = (Polygon) _mw_load_etype_to_itype(fname,mtype,"polygon",type);
     if (poly != NULL) return(poly);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Polygon !\n",fname,type);
     return NULL;
}

/* Write file in A_POLY format */  

short _mw_create_polygon_a_poly(char *fname, Polygon poly)
{
     FILE *fp;
     Point_curve pc;
     int i;

     if (poly == NULL)
	  mwerror(INTERNAL,1,"Cannot create file: Polygon structure is NULL\n");

     if (poly->first == NULL)
	  mwerror(INTERNAL,1,
		  "Cannot create file: No point in the Polygon structure\n");

     fp =_mw_create_data_ascii_file(fname);
     if (fp == NULL) return(-1);

     fprintf(fp,"%%\n");
     fprintf(fp,"%%----- Polygon -----\n");
     fprintf(fp,"def Polygon\n");
     fprintf(fp,"nb_channels: %d\n",poly->nb_channels);
     for (i=0;i<poly->nb_channels;i++)
	  fprintf(fp,"channel #%d: %f\n",i+1,poly->channel[i]);
     for (pc=poly->first; pc; pc=pc->next)
	  fprintf(fp,"%d,%d\n",pc->x,pc->y);
  
     fclose(fp);
     return(0);
}
   
/* Create native formats (without conversion of the internal type) */

short _mw_polygon_create_native(char *fname, Polygon poly, char *Type)
{
     if (strcmp(Type,"A_POLY") == 0)
	  return(_mw_create_polygon_a_poly(fname,poly));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_polygon(char *fname, Polygon poly, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_polygon_create_native(fname,poly,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,poly,"polygon",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}


/* --- I/O for Polygons ---- */

/* Load polygons from a file of A_POLY format */

Polygons _mw_load_polygons_a_poly(char *fname)
{
     FILE    *fp;
     Polygons poly;
     Polygon newp,oldp;
     Point_curve newpc,oldpc;
     int px,py,i,nc;
     char channel[20];

     fp = _mw_open_data_ascii_file(fname);
     if (fp == NULL) return(NULL);

     if (_mw_fascii_search_string(fp,"def Polygon\n") == EOF)
     {
	  mwerror(ERROR, 0,
		  "No Polygon description found in the file \"%s\"",fname);
	  fclose(fp);
	  return(NULL);
     }

     poly = mw_new_polygons();
     if (poly == NULL) return(poly);
     oldp = newp = NULL;

     do 
     {
	  if ((_mw_fascii_get_field(fp,fname,"nb_channels:","%d",&nc) != 1)
	      || (nc < 0))
	  {
	       mw_delete_polygons(poly);
	       fclose(fp);
	       return(NULL);
	  }
      
	  newp = mw_change_polygon(NULL,nc);
	  if (newp == NULL)
	  {
	       mw_delete_polygons(poly);
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
		    mw_delete_polygons(poly);
		    fclose(fp);
		    return(NULL);
	       }
	  }

	  while (fscanf(fp,"%d,%d\n",&px,&py) ==  2)
	  {
	       newpc = mw_new_point_curve();
	       if (newpc == NULL)
	       {
		    mw_delete_polygons(poly);
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
     } while (_mw_fascii_search_string(fp,"def Polygon\n") != EOF);
      
     fclose(fp);
     return(poly);
}

/* Native formats (without conversion of the internal type) */

Polygons _mw_polygons_load_native(char *fname, char *type)
{
     if (strcmp(type,"A_POLY") == 0)
	  return((Polygons)_mw_load_polygons_a_poly(fname));

     return(NULL);
}


/* All available formats */

Polygons _mw_load_polygons(char *fname, char *type)
{ 
     Polygons poly;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     poly = _mw_polygons_load_native(fname,type);
     if (poly != NULL) return(poly);

     /* If failed, try other formats with memory conversion */
     poly = (Polygons) _mw_load_etype_to_itype(fname,mtype,"polygons",type);
     if (poly != NULL) return(poly);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Polygons !\n",fname,type);
     return NULL;
}


/* Write file in A_POLY format */  

short _mw_create_polygons_a_poly(char *fname, Polygons poly)
{
     FILE *fp;
     Polygon pl;
     Point_curve pc;
     int i,n;

     if (poly == NULL)
	  mwerror(INTERNAL,1,"Cannot create file: Polygons structure is NULL\n");

     if (poly->first == NULL)
	  mwerror(INTERNAL,1,
		  "Cannot create file: No polygon in the Polygons structure\n");

     fp =_mw_create_data_ascii_file(fname);
     if (fp == NULL) return(-1);

     for (pl=poly->first, n=1; pl; pl=pl->next, n++)
     {
	  if (pl->first == NULL)
	       mwerror(INTERNAL,1,"Polygon #%d has no point curve\n",n);
	  fprintf(fp,"%%\n");
	  fprintf(fp,"%%----- Polygon #%d -----\n",n);
	  fprintf(fp,"def Polygon\n");
	  fprintf(fp,"nb_channels: %d\n",pl->nb_channels);
	  for (i=0;i<pl->nb_channels;i++)
	       fprintf(fp,"channel #%d: %f\n",i+1,pl->channel[i]);
      
	  for (pc=pl->first; pc; pc=pc->next)
	       fprintf(fp,"%d,%d\n",pc->x,pc->y);
     }
      
     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_polygons_create_native(char *fname, Polygons poly, char *Type)
{
     if (strcmp(Type,"A_POLY") == 0)
	  return(_mw_create_polygons_a_poly(fname,poly));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_polygons(char *fname, Polygons poly, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_polygons_create_native(fname,poly,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,poly,"polygons",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}

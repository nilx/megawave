/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  cmimage_io.c
   
  Vers. 1.13
  Author : Jacques Froment
  Input/output functions for the 
  Cmorpho_line
  Cfmorpho_line
  Cmorpho_set
  Cmorpho_sets
  Cmimage
  structures

  Main changes :
  v1.1 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
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
#include "mw.h"

#include "cmimage.h"
#include "curve.h"
#include "mimage.h"
#include "file_type.h"
#include "mimage_io.h"
#include "type_conv.h"
#include "mwio.h"
#include "fcurve.h"

#include "cmimage_io.h"

/* ---- I/O for cmorpho_line ---- */

/* Read one cmorpho_line from the file fp */

static Cmorpho_line _mw_read_cml_mw2_cml(char * fname, FILE * fp, 
					 int need_flipping)
{
     Cmorpho_line ll;
     Point_curve newpc,oldpc;
     Point_type newpt,oldpt;
     unsigned int npc, npt, i;

     ll = mw_new_cmorpho_line();
     if (ll == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }

     if (
	  (fread(&(ll->minvalue.model),sizeof(unsigned char),1,fp) == 0) || 
	  (fread(&(ll->minvalue.red),sizeof(float),1,fp) == 0) || 
	  (fread(&(ll->minvalue.green),sizeof(float),1,fp) == 0) || 
	  (fread(&(ll->minvalue.blue),sizeof(float),1,fp) == 0) || 
	  (fread(&(ll->maxvalue.model),sizeof(unsigned char),1,fp) == 0) || 
	  (fread(&(ll->maxvalue.red),sizeof(float),1,fp) == 0) || 
	  (fread(&(ll->maxvalue.green),sizeof(float),1,fp) == 0) || 
	  (fread(&(ll->maxvalue.blue),sizeof(float),1,fp) == 0) || 
	  (fread(&(ll->open),sizeof(unsigned char),1,fp) == 0) ||
	  (fread(&(ll->data),sizeof(float),1,fp) == 0) ||
	  (fread(&(npc),sizeof(unsigned int),1,fp) == 0) ||
	  (fread(&(npt),sizeof(unsigned int),1,fp) == 0)
	  )
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  return(NULL);
     }

     fflush(fp);  /* Should not be here, but this may avoid error on the next
		     read value (newpc->x), on HP-UX 10.20                   */

     if (need_flipping == 1)
     {
	  _mw_in_flip_float( (ll->minvalue.red) );
	  _mw_in_flip_float( (ll->minvalue.green) );
	  _mw_in_flip_float( (ll->minvalue.blue) );
	  _mw_in_flip_float( (ll->maxvalue.red) );
	  _mw_in_flip_float( (ll->maxvalue.green) );
	  _mw_in_flip_float( (ll->maxvalue.blue) );
	  _mw_in_flip_float( (ll->data) );
	  _mw_in_flip_b4(npc);
	  _mw_in_flip_b4(npt);
     }


     if ( (npc*npt != 0) && (npc != npt) )
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" : inconsistent Cmorpho_line structure (# pt curve %d != # pt type %d)\n",fname,npc,npt);
	  return(NULL);
     }
  
     oldpc = newpc = NULL;
     for (i = 1; i <= npc; i++)
     {
	  newpc = mw_new_point_curve();
	  if (newpc == NULL)
	  {
	       mw_delete_cmorpho_line(ll);
	       return(NULL);
	  }
	  if (ll->first_point == NULL) ll->first_point = newpc;
	  if (oldpc != NULL) oldpc->next = newpc;
	  newpc->previous = oldpc;
	  newpc->next = NULL;
	  if (
	       (fread(&(newpc->x),sizeof(int),1,fp) == 0) || 
	       (fread(&(newpc->y),sizeof(int),1,fp) == 0) 
	       )
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       mw_delete_cmorpho_line(ll);
	       return(NULL);
	  }
	  if (need_flipping == 1)
	  {
	       _mw_in_flip_b4(newpc->x);
	       _mw_in_flip_b4(newpc->y);
	  }
	  oldpc = newpc;
     }

     oldpt = newpt = NULL;
     for (i = 1; i <= npt; i++)
     {
	  newpt = mw_new_point_type();
	  if (newpt == NULL)
	  {
	       mw_delete_cmorpho_line(ll);
	       return(NULL);
	  }
	  if (ll->first_type == NULL) ll->first_type = newpt;
	  if (oldpt != NULL) oldpt->next = newpt;
	  newpt->previous = oldpt;
	  newpt->next = NULL;
	  if (fread(&(newpt->type),sizeof(unsigned char),1,fp) == 0)
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       mw_delete_cmorpho_line(ll);
	       return(NULL);
	  }
	  oldpt=newpt;
     }
     return(ll);
}

/* Load one cmorpho_line from a file of MW2_CMORPHO_LINE format */

Cmorpho_line _mw_load_cml_mw2_cml(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Cmorpho_line ll;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_CMORPHO_LINE",16) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_cml_mw2_cml] File \"%s\" is not in the MW2_CMORPHO_LINE format\n",fname);

     if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* read header = "MW2_CMORPHO_LINE" */
     if (fread(header,hsize,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     ll = _mw_read_cml_mw2_cml(fname,fp,need_flipping);

     fclose(fp);
     return(ll);
}


/* Native formats (without conversion of the internal type) */

Cmorpho_line _mw_cmorpho_line_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_CMORPHO_LINE") == 0)
	  return((Cmorpho_line)_mw_load_ml_mw2_ml(fname));

     return(NULL);
}


/* All available formats */

Cmorpho_line _mw_load_cmorpho_line(char *fname, char *type)
{ 
     Cmorpho_line ml;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     ml = _mw_cmorpho_line_load_native(fname,type);
     if (ml != NULL) return(ml);

     /* If failed, try other formats with memory conversion */
     ml = (Cmorpho_line) _mw_load_etype_to_itype(fname,mtype,"cmorpho_line",type);
     if (ml != NULL) return(ml);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Cmorpho_line !\n",fname,type);
     return NULL;
}


/* Write one cmorpho line in the file fp */  

void _mw_write_cml_mw2_cml(FILE *fp, Cmorpho_line ll, unsigned int nml)
{
     Point_curve pc;
     Point_type pt;
     unsigned int npc,npt;

     /* FIXME : unused param, dirty fix */
     nml = nml;

     fwrite(&(ll->minvalue.model),sizeof(unsigned char),1,fp);
     fwrite(&(ll->minvalue.red),sizeof(float),1,fp);
     fwrite(&(ll->minvalue.green),sizeof(float),1,fp);
     fwrite(&(ll->minvalue.blue),sizeof(float),1,fp);
     fwrite(&(ll->maxvalue.model),sizeof(unsigned char),1,fp);
     fwrite(&(ll->maxvalue.red),sizeof(float),1,fp);
     fwrite(&(ll->maxvalue.green),sizeof(float),1,fp);
     fwrite(&(ll->maxvalue.blue),sizeof(float),1,fp);
     fwrite(&(ll->open),sizeof(unsigned char),1,fp);
     fwrite(&(ll->data),sizeof(float),1,fp);

     /* Record the number of point curve */
     npc=0;
     if (ll->first_point) for (pc=ll->first_point; pc; pc=pc->next, npc++);

/*
  fprintf(stderr,"npc=%d\n",npc);
*/

     /* Record the number of point type */
     npt=0;
     if (ll->first_type) for (pt=ll->first_type; pt; pt=pt->next, npt++);

/*
  fprintf(stderr,"npt=%d\n",npt);
*/

     if ( (npc*npt != 0) && (npc != npt) )
	  mwerror(INTERNAL,1,"[_mw_write_cml_mw2_cml] Cannot create file: inconsistent Cmorpho_line structure (# pt curve %d != # pt type %d)\n",npc,npt);

     fwrite(&(npc),sizeof(unsigned int),1,fp);
     fwrite(&(npt),sizeof(unsigned int),1,fp);
     /* debug 
	if ((nml >= 422)&&(nml <=426))
	{
	fprintf(stderr,"nml=%d \t npc=%d \t npt=%d\n",nml,npc,npt);
	}
     */

     for (pc=ll->first_point; pc; pc=pc->next)
     {
	  fwrite(&(pc->x),sizeof(int),1,fp);
	  fwrite(&(pc->y),sizeof(int),1,fp);
	  /* debug 
	     if (((nml >= 422)&&(nml <=426))&&(pc==ll->first_point))
	     {
	     fprintf(stderr,"nml=%d \t x=%d \t y=%d\n",nml,pc->x,pc->y);
	     }
	  */
     }

     for (pt=ll->first_type; pt; pt=pt->next)
	  fwrite(&(pt->type),sizeof(unsigned char),1,fp);
}

/* Write file in MW2_CMORPHO_LINE format */  

short _mw_create_cml_mw2_cml(char *fname, Cmorpho_line ll)
{
     FILE *fp;

     if (ll == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_cml_mw2_cml] Cannot create file: Cmorpho_line structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_CMORPHO_LINE",1.00);
     if (fp == NULL) return(-1);

     _mw_write_cml_mw2_cml(fp,ll,1);

     fclose(fp);
     return(0);
}


/* Create native formats (without conversion of the internal type) */

short _mw_cmorpho_line_create_native(char *fname, Cmorpho_line ll, char *Type)
{
     if (strcmp(Type,"MW2_CMORPHO_LINE") == 0)
	  /* FIXME: wrong types, dirty temporary fix */
	  return(_mw_create_ml_mw2_ml(fname, (Morpho_line) ll));
  
     return(-1);
}

/* Write file in different formats */
   
short _mw_create_cmorpho_line(char *fname, Cmorpho_line ll, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_cmorpho_line_create_native(fname,ll,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,ll,"cmorpho_line",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}

/* ---- I/O for Cfmorpho_line ---- */

/* Read one cfmorpho_line from the file fp */

static Cfmorpho_line _mw_read_cfml_mw2_cfml(char *fname, FILE *fp, 
					    int need_flipping)
{ 
     Cfmorpho_line fll;
     Point_fcurve newpc,oldpc;
     Point_type newpt,oldpt;
     unsigned int npc, npt, i;

     fll = mw_new_cfmorpho_line();
     if (fll == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }

     if (
	  (fread(&(fll->minvalue.model),sizeof(unsigned char),1,fp) == 0) || 
	  (fread(&(fll->minvalue.red),sizeof(float),1,fp) == 0) || 
	  (fread(&(fll->minvalue.green),sizeof(float),1,fp) == 0) || 
	  (fread(&(fll->minvalue.blue),sizeof(float),1,fp) == 0) || 
	  (fread(&(fll->maxvalue.model),sizeof(unsigned char),1,fp) == 0) || 
	  (fread(&(fll->maxvalue.red),sizeof(float),1,fp) == 0) || 
	  (fread(&(fll->maxvalue.green),sizeof(float),1,fp) == 0) || 
	  (fread(&(fll->maxvalue.blue),sizeof(float),1,fp) == 0) || 
	  (fread(&(fll->open),sizeof(unsigned char),1,fp) == 0) ||
	  (fread(&(fll->data),sizeof(float),1,fp) == 0) ||
	  (fread(&(npc),sizeof(unsigned int),1,fp) == 0) ||
	  (fread(&(npt),sizeof(unsigned int),1,fp) == 0)
	  )
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     if (need_flipping == 1)
     {
	  _mw_in_flip_float( (fll->minvalue.red) );
	  _mw_in_flip_float( (fll->minvalue.green) );
	  _mw_in_flip_float( (fll->minvalue.blue) );
	  _mw_in_flip_float( (fll->maxvalue.red) );
	  _mw_in_flip_float( (fll->maxvalue.green) );
	  _mw_in_flip_float( (fll->maxvalue.blue) );
	  _mw_in_flip_float( (fll->data) );
	  _mw_in_flip_b4(npc);
	  _mw_in_flip_b4(npt);
     }
/*
  fprintf(stderr,"npc=%d\n",npc);
  fprintf(stderr,"npt=%d\n",npt);
*/

     oldpc = newpc = NULL;
     for (i = 1; i <= npc; i++)
     {
	  newpc = mw_new_point_fcurve();
	  if (newpc == NULL)
	  {
	       mw_delete_cfmorpho_line(fll);
	       return(NULL);
	  }
	  if (fll->first_point == NULL) fll->first_point = newpc;
	  if (oldpc != NULL) oldpc->next = newpc;
	  newpc->previous = oldpc;
	  newpc->next = NULL;
	  if (
	       (fread(&(newpc->x),sizeof(float),1,fp) == 0) || 
	       (fread(&(newpc->y),sizeof(float),1,fp) == 0) 
	       )
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       mw_delete_cfmorpho_line(fll);
	       return(NULL);
	  }
	  if (need_flipping == 1)
	  {
	       _mw_in_flip_float((newpc->x));
	       _mw_in_flip_float((newpc->y));
	  }
	  oldpc = newpc;
     }

     oldpt = newpt = NULL;
     for (i = 1; i <= npt; i++)
     {
	  newpt = mw_new_point_type();
	  if (newpt == NULL)
	  {
	       mw_delete_cfmorpho_line(fll);
	       return(NULL);
	  }
	  if (fll->first_type == NULL) fll->first_type = newpt;
	  if (oldpt != NULL) oldpt->next = newpt;
	  newpt->previous = oldpt;
	  newpt->next = NULL;
	  if (fread(&(newpt->type),sizeof(unsigned char),1,fp) == 0)
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       mw_delete_cfmorpho_line(fll);
	       return(NULL);
	  }
	  oldpt = newpt;
     }
     return(fll);
}

/* Load Cfmorpho_line from a file of MW2_CFMORPHO_LINE format */

Cfmorpho_line _mw_load_cfml_mw2_cfml(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Cfmorpho_line fll;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_CFMORPHO_LINE",17) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_cfml_mw2_cfml] File \"%s\" is not in the MW2_CFMORPHO_LINE format\n",fname);

     if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* read header = "MW2_CFMORPHO_LINE" */
     if (fread(header,hsize,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     fll = _mw_read_cfml_mw2_cfml(fname,fp,need_flipping);

     fclose(fp);
     return(fll);
}


/* Native formats (without conversion of the internal type) */

Cfmorpho_line _mw_cfmorpho_line_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_CFMORPHO_LINE") == 0)
	  return((Cfmorpho_line)_mw_load_fml_mw2_fml(fname));

     return(NULL);
}


/* All available formats */

Cfmorpho_line _mw_load_cfmorpho_line(char *fname, char *type)
{ 
     Cfmorpho_line ml;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     ml = _mw_cfmorpho_line_load_native(fname,type);
     if (ml != NULL) return(ml);

     /* If failed, try other formats with memory conversion */
     ml = (Cfmorpho_line) _mw_load_etype_to_itype(fname,mtype,"cfmorpho_line",type);
     if (ml != NULL) return(ml);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Cfmorpho_line !\n",fname,type);
     return NULL;
}


/* Write one cfmorpho line in the file fp */  

void _mw_write_cfml_mw2_cfml(FILE *fp, Cfmorpho_line fll)
{
     Point_fcurve pc;
     Point_type pt;
     unsigned int npc,npt;

     fwrite(&(fll->minvalue.model),sizeof(unsigned char),1,fp);
     fwrite(&(fll->minvalue.red),sizeof(float),1,fp);
     fwrite(&(fll->minvalue.green),sizeof(float),1,fp);
     fwrite(&(fll->minvalue.blue),sizeof(float),1,fp);
     fwrite(&(fll->maxvalue.model),sizeof(unsigned char),1,fp);
     fwrite(&(fll->maxvalue.red),sizeof(float),1,fp);
     fwrite(&(fll->maxvalue.green),sizeof(float),1,fp);
     fwrite(&(fll->maxvalue.blue),sizeof(float),1,fp);
     fwrite(&(fll->open),sizeof(unsigned char),1,fp);
     fwrite(&(fll->data),sizeof(float),1,fp);

     /* Record the number of point fcurve */
     for (pc=fll->first_point, npc=0; pc; pc=pc->next, npc++);

/*
  fprintf(stderr,"npc=%d\n",npc);
*/

     /* Record the number of point type */
     for (pt=fll->first_type, npt=0; pt; pt=pt->next, npt++);

/*
  fprintf(stderr,"npt=%d\n",npt);
*/

     if ( (npc*npt != 0) && (npc != npt) )
	  mwerror(INTERNAL,1,"[_mw_write_cfml_mw2_cfml] Cannot create file: inconsistent Cfmorpho_line structure \n");

     fwrite(&(npc),sizeof(unsigned int),1,fp);
     fwrite(&(npt),sizeof(unsigned int),1,fp);

     for (pc=fll->first_point; pc; pc=pc->next)
     {
	  fwrite(&(pc->x),sizeof(float),1,fp);
	  fwrite(&(pc->y),sizeof(float),1,fp);
     }

     for (pt=fll->first_type; pt; pt=pt->next)
	  fwrite(&(pt->type),sizeof(unsigned char),1,fp);
}

/* Write file in MW2_CFMORPHO_LINE format */  

short _mw_create_cfml_mw2_cfml(char *fname, Cfmorpho_line fll)
{
     FILE *fp;

     if (fll == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_cfml_mw2_cfml] Cannot create file: Cfmorpho_line structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_CFMORPHO_LINE",1.00);
     if (fp == NULL) return(-1);

     _mw_write_cfml_mw2_cfml(fp,fll);

     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_cfmorpho_line_create_native(char *fname, Cfmorpho_line ll, char *Type)
{
     if (strcmp(Type,"MW2_CFMORPHO_LINE") == 0)
	  /* FIXME: wrong types, dirty temporary fix */
	  return(_mw_create_fml_mw2_fml(fname, (Fmorpho_line) ll));
  
     return(-1);
}

/* Write file in different formats */
   
short _mw_create_cfmorpho_line(char *fname, Cfmorpho_line ll, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_cfmorpho_line_create_native(fname,ll,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,ll,"cfmorpho_line",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}


/* ---- I/O for cmorpho_set ---- */

/* Read one cmorpho_set from the file fp */

Cmorpho_set _mw_read_cms_mw2_cms(char *fname, FILE *fp, int need_flipping)
{ 
     Cmorpho_set is;
     Hsegment news,olds;
     unsigned int ns, i;

     is = mw_new_cmorpho_set();
     if (is == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }

     if (
	  (fread(&(is->minvalue.model),sizeof(unsigned char),1,fp) == 0) || 
	  (fread(&(is->minvalue.red),sizeof(float),1,fp) == 0) || 
	  (fread(&(is->minvalue.green),sizeof(float),1,fp) == 0) || 
	  (fread(&(is->minvalue.blue),sizeof(float),1,fp) == 0) || 
	  (fread(&(is->maxvalue.model),sizeof(unsigned char),1,fp) == 0) || 
	  (fread(&(is->maxvalue.red),sizeof(float),1,fp) == 0) || 
	  (fread(&(is->maxvalue.green),sizeof(float),1,fp) == 0) || 
	  (fread(&(is->maxvalue.blue),sizeof(float),1,fp) == 0) || 
	  (fread(&(is->stated),sizeof(unsigned char),1,fp) == 0) ||
	  (fread(&(is->area),sizeof(int),1,fp) == 0) || 
	  (fread(&(ns),sizeof(unsigned int),1,fp) == 0)
	  )
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  return(NULL);
     }
     if (need_flipping == 1)
     {
	  _mw_in_flip_float( (is->minvalue.red) );
	  _mw_in_flip_float( (is->minvalue.green) );
	  _mw_in_flip_float( (is->minvalue.blue) );
	  _mw_in_flip_float( (is->maxvalue.red) );
	  _mw_in_flip_float( (is->maxvalue.green) );
	  _mw_in_flip_float( (is->maxvalue.blue) );
	  _mw_in_flip_b4(is->area);
	  _mw_in_flip_b4(ns);
     }
     /* printf("Number of segments ns=%d\n",ns); */

     /* Load the segments */
     olds = news = NULL;
     for (i = 1; i <= ns; i++)
     {
	  news = mw_new_hsegment();
	  if (news == NULL)
	  {
	       mw_delete_cmorpho_set(is);
	       mwerror(ERROR, 0,"Not enough memory to load file \"%s\"\n",fname);
	       return(NULL);
	  }
	  if (is->first_segment == NULL) is->first_segment = news;
	  if (olds != NULL) olds->next = news;
	  news->previous = olds;
	  news->next = NULL;
	  if (
	       (fread(&(news->xstart),sizeof(int),1,fp) == 0) || 
	       (fread(&(news->xend),sizeof(int),1,fp) == 0) || 
	       (fread(&(news->y),sizeof(int),1,fp) == 0) 
	       )
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       mw_delete_cmorpho_set(is);
	       return(NULL);
	  }
	  if (need_flipping == 1)
	  {
	       _mw_in_flip_b4(news->xstart);
	       _mw_in_flip_b4(news->xend);
	       _mw_in_flip_b4(news->y);
	  }
	  olds = news;
     }   
     is->last_segment = olds;

     return(is);
}

/* Load cmorpho_set from a file of MW2_CMORPHO_SET format */

Cmorpho_set _mw_load_cms_mw2_cms(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Cmorpho_set is;
     char ftype[mw_mtype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_CMORPHO_SET",15) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_cms_mw2_cms] File \"%s\" is not in the MW2_CMORPHO_SET format\n",fname);

     if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* read header = "MW2_CMORPHO_SET" */
     if (fread(header,hsize,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     is = _mw_read_cms_mw2_cms(fname,fp,need_flipping);

     fclose(fp);
     return(is);
}

/* Native formats (without conversion of the internal type) */

Cmorpho_set _mw_cmorpho_set_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_CMORPHO_SET") == 0)
	  return((Cmorpho_set)_mw_load_ms_mw2_ms(fname));

     return(NULL);
}


/* All available formats */

Cmorpho_set _mw_load_cmorpho_set(char *fname, char *type)
{ 
     Cmorpho_set ms;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     ms = _mw_cmorpho_set_load_native(fname,type);
     if (ms != NULL) return(ms);

     /* If failed, try other formats with memory conversion */
     ms = (Cmorpho_set) _mw_load_etype_to_itype(fname,mtype,"cmorpho_set",type);
     if (ms != NULL) return(ms);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Cmorpho_set !\n",fname,type);
     return NULL;
}


/* Write one cmorpho set in the file fp */  

void _mw_write_cms_mw2_cms(FILE *fp, Cmorpho_set is)
{
     Hsegment s;
     unsigned int ns;

     /* Record the number of segments */
     for (s=is->first_segment, ns=0; s; s=s->next, ns++);

     fwrite(&(is->minvalue.model),sizeof(unsigned char),1,fp);
     fwrite(&(is->minvalue.red),sizeof(float),1,fp);
     fwrite(&(is->minvalue.green),sizeof(float),1,fp);
     fwrite(&(is->minvalue.blue),sizeof(float),1,fp);
     fwrite(&(is->maxvalue.model),sizeof(unsigned char),1,fp);
     fwrite(&(is->maxvalue.red),sizeof(float),1,fp);
     fwrite(&(is->maxvalue.green),sizeof(float),1,fp);
     fwrite(&(is->maxvalue.blue),sizeof(float),1,fp);
     fwrite(&(is->stated),sizeof(unsigned char),1,fp);
     fwrite(&(is->area),sizeof(int),1,fp);
     fwrite(&(ns),sizeof(unsigned int),1,fp);

     for (s=is->first_segment; s; s=s->next)
     {
	  fwrite(&(s->xstart),sizeof(int),1,fp);
	  fwrite(&(s->xend),sizeof(int),1,fp);
	  fwrite(&(s->y),sizeof(int),1,fp);
     }
}

/* Write file in MW2_CMORPHO_SET format */  

short _mw_create_cms_mw2_cms(char *fname, Cmorpho_set is)
{
     FILE *fp;

     if (is == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_cms_mw2_cms] Cannot create file: Cmorpho_set structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_CMORPHO_SET",1.00);
     if (fp == NULL) return(-1);

     _mw_write_cms_mw2_cms(fp,is);

     fclose(fp);
     return(0);
}


/* Create native formats (without conversion of the internal type) */

short _mw_cmorpho_set_create_native(char *fname, Cmorpho_set ms, char *Type)
{
     if (strcmp(Type,"MW2_CMORPHO_SET") == 0)
	  /* FIXME: wrong types, dirty temporary fix */
	  return(_mw_create_ms_mw2_ms(fname, (Morpho_set) ms));
  
     return(-1);
}

/* Write file in different formats */
   
short _mw_create_cmorpho_set(char *fname, Cmorpho_set ms, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_cmorpho_set_create_native(fname,ms,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,ms,"cmorpho_set",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}


/* ---- I/O for cmorpho_sets  ---- */

/* Read one cmorpho_sets from the file fp */

Cmorpho_sets _mw_read_cmss_mw2_cmss(char *fname, FILE *fp, int need_flipping)
{
     Cmorpho_sets iss,oldiss,newiss,p,q;
     unsigned int i,n,num;

     /* Get the number of cmorpho sets */
     if (fread(&(n),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  return(NULL);
     }

     if (need_flipping == 1) _mw_in_flip_b4(n);
     if (n == 0) return(NULL);
     /* printf("Number of cmorpho sets = %d\n",n); */

     iss = mw_new_cmorpho_sets();
     if (iss == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }

     /* Load the cmorpho sets */
     oldiss = NULL;
     newiss = iss;
     for (i = 1; i <= n; i++)
     {
	  if ((newiss == NULL)&&((newiss=mw_new_cmorpho_sets())==NULL))
	  {
	       mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	       mw_delete_cmorpho_sets(iss);
	       return(NULL);
	  }
	  newiss->cmorphoset = _mw_read_cms_mw2_cms(fname,fp,need_flipping);
	  if (newiss->cmorphoset== NULL)
	  {
	       mw_delete_cmorpho_sets(newiss);
	       mw_delete_cmorpho_sets(iss);
	       return(NULL);
	  }
	  if (oldiss != NULL) oldiss->next = newiss;
	  newiss->previous = oldiss;
	  newiss->next = NULL;
	  oldiss=newiss;
	  newiss=NULL;
     }
  
     /* Compute the Cmorpho set numbers */
     mw_num_cmorpho_sets(iss);

     /* Read the neighbor cmorpho sets (given by the Cmorpho set numbers list) */
     for (p=iss; p; p=p->next)
     {  
	  oldiss=newiss=NULL;
	  /* Read first the number of neighbors */
	  fread(&(n),sizeof(unsigned int),1,fp);
	  if (need_flipping == 1) _mw_in_flip_b4(n);
	  /* 
	     fprintf(stderr,"Number of neighbors = %d\n",n); 
	  */
	  for (i = 1; i <= n; i++)
	  {
	       fread(&(num),sizeof(unsigned int),1,fp);
	       if (need_flipping == 1) _mw_in_flip_b4(num);
	       /* Search the cmorpho sets containing the given Cmorpho set number */
	       for (q=iss; q && q->cmorphoset && (q->cmorphoset->num != num); q=q->next);
	       if (!q || !(q->cmorphoset)) 
	       {
		    mw_delete_cmorpho_sets(iss);
		    mwerror(INTERNAL, 0,"[_mw_load_cmss_mw2_cmss] Cannot find Cmorpho set number %d in file \"%s\"\n",num,fname);
	       }

	       if ((newiss=mw_new_cmorpho_sets())==NULL)
	       {
		    mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
		    mw_delete_cmorpho_sets(iss);
		    return(NULL);
	       }
	       newiss->cmorphoset = q->cmorphoset;
	       if (p->cmorphoset->neighbor == NULL) p->cmorphoset->neighbor=newiss;
	       if (oldiss) oldiss->next = newiss;
	       newiss->previous = oldiss;
	       oldiss = newiss;
	  }
     }
     return(iss);
}

/* Load cmorpho_sets from a file of MW2_CMORPHO_SETS format */

Cmorpho_sets _mw_load_cmss_mw2_cmss(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Cmorpho_sets iss;
     char ftype[mw_mtype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_CMORPHO_SETS",16) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_cmss_mw2_cmss] File \"%s\" is not in the MW2_CMORPHO_SETS format\n",fname);

     if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     /* read header = "MW2_CMORPHO_SETS" */
     if (fread(header,hsize,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     iss = _mw_read_cmss_mw2_cmss(fname,fp,need_flipping);

     fclose(fp);
     return(iss);
}

/* Native formats (without conversion of the internal type) */

Cmorpho_sets _mw_cmorpho_sets_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_CMORPHO_SETS") == 0)
	  return((Cmorpho_sets)_mw_load_mss_mw2_mss(fname));

     return(NULL);
}


/* All available formats */

Cmorpho_sets _mw_load_cmorpho_sets(char *fname, char *type)
{ 
     Cmorpho_sets ms;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     ms = _mw_cmorpho_sets_load_native(fname,type);
     if (ms != NULL) return(ms);

     /* If failed, try other formats with memory conversion */
     ms = (Cmorpho_sets) _mw_load_etype_to_itype(fname,mtype,"cmorpho_sets",type);
     if (ms != NULL) return(ms);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Cmorpho_sets !\n",fname,type);
     return NULL;
}


/* Write one cmorpho sets in the file fp */  

void _mw_write_cmss_mw2_cmss(FILE *fp, Cmorpho_sets iss)
{
     unsigned int n;
     Cmorpho_sets p,q,neig;

     /* Compute the Cmorpho set numbers */
     if (iss) n=mw_num_cmorpho_sets(iss); else n=0;
     /* fprintf(stderr,"Number of cmorpho sets = %d\n",n); */

     /* Record the number of cmorpho sets */
     fwrite(&(n),sizeof(unsigned int),1,fp);

     if (!iss || !iss->cmorphoset) return;

     /* Record each cmorpho set */
     for (p=iss; p; p=p->next) _mw_write_cms_mw2_cms(fp,p->cmorphoset);      

     /* Add the neighbor cmorpho sets (given by the Cmorpho set numbers list) */
     for (p=iss; p; p=p->next)
     {
	  /* Write first the number of neighbors */
	  neig = p->cmorphoset->neighbor;
	  n = mw_length_cmorpho_sets(neig);
	  /*
	    fprintf(stderr,"Number of neighbors = %d\n",n);
	  */
	  fwrite(&(n),sizeof(unsigned int),1,fp);
	  for (q=neig; q; q=q->next)
	       fwrite(&(q->cmorphoset->num),sizeof(unsigned int),1,fp);
     }
}

/* Write file in MW2_CMORPHO_SETS format */  

short _mw_create_cmss_mw2_cmss(char *fname, Cmorpho_sets iss)
{
     FILE *fp;

     if (iss == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_cmss_mw2_cmss] Cannot create file: Cmorpho_sets structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_CMORPHO_SETS",1.00);
     if (fp == NULL) return(-1);

     _mw_write_cmss_mw2_cmss(fp,iss);

     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_cmorpho_sets_create_native(char *fname, Cmorpho_sets mss, char *Type)
{
     if (strcmp(Type,"MW2_CMORPHO_SETS") == 0)
	  /* FIXME: wrong types, dirty temporary fix */
	  return(_mw_create_mss_mw2_mss(fname, (Morpho_sets) mss));
  
     return(-1);
}

/* Write file in different formats */
   
short _mw_create_cmorpho_sets(char *fname, Cmorpho_sets mss, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_cmorpho_sets_create_native(fname,mss,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,mss,"cmorpho_sets",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}


/* ---- I/O for Cmimage ---- */

/* Load Cmimage from a file of MW2_CMIMAGE format */

Cmimage _mw_load_cmimage_mw2_cmimage(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Cmimage cmimage;
     Cmorpho_line ll,oldll,newll;
     Cmorpho_sets mss;
     Cfmorpho_line oldfll,newfll;
     unsigned int size,nll,i,mlnum,msnum;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_CMIMAGE",11) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_cmimage_mw2_cmimage] File \"%s\" is not in the MW2_CMIMAGE format\n",fname);

     if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     /* read header = "MW2_CMIMAGE" */
     if (fread(header,hsize,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (header)...\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     cmimage = mw_new_cmimage();
     if (cmimage == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     /* Read the cmt field */

     if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (cmt size)...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(size);
     if ((size > 0)&& (fread(cmimage->cmt,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (cmt; cmt size=%d)...\n",fname,size);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the name field */

     if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name size)...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(size);
     if ((size > 0)&&(fread(cmimage->name,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name)...\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the other fields */
     if (
	  (fread(&(cmimage->nrow),sizeof(int),1,fp) == 0) || 
	  (fread(&(cmimage->ncol),sizeof(int),1,fp) == 0) || 
	  (fread(&(cmimage->minvalue.model),sizeof(unsigned char),1,fp) == 0) || 
	  (fread(&(cmimage->minvalue.red),sizeof(float),1,fp) == 0) || 
	  (fread(&(cmimage->minvalue.green),sizeof(float),1,fp) == 0) || 
	  (fread(&(cmimage->minvalue.blue),sizeof(float),1,fp) == 0) || 
	  (fread(&(cmimage->maxvalue.model),sizeof(unsigned char),1,fp) == 0) || 
	  (fread(&(cmimage->maxvalue.red),sizeof(float),1,fp) == 0) || 
	  (fread(&(cmimage->maxvalue.green),sizeof(float),1,fp) == 0) || 
	  (fread(&(cmimage->maxvalue.blue),sizeof(float),1,fp) == 0) || 
	  (fread(&(nll),sizeof(unsigned int),1,fp) == 0)
	  )
     {      mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     if (need_flipping == 1)
     {
	  _mw_in_flip_b4(cmimage->nrow);
	  _mw_in_flip_b4(cmimage->ncol);
	  _mw_in_flip_float( (cmimage->minvalue.red) );
	  _mw_in_flip_float( (cmimage->minvalue.green) );
	  _mw_in_flip_float( (cmimage->minvalue.blue) );
	  _mw_in_flip_float( (cmimage->maxvalue.red) );
	  _mw_in_flip_float( (cmimage->maxvalue.green) );
	  _mw_in_flip_float( (cmimage->maxvalue.blue) );
	  _mw_in_flip_b4(nll);
     }


/*
  fprintf(stderr,"Number of Cmorpho lines nll=%d\n",nll);
*/

     oldll = newll = NULL;
     for (i = 1; i <= nll; i++)
     {
	  newll = _mw_read_cml_mw2_cml(fname,fp,need_flipping);
	  if (newll == NULL)
	  {
	       mw_delete_cmimage(cmimage);
	       fclose(fp);
	       return(NULL);
	  }
	  if (cmimage->first_ml == NULL) cmimage->first_ml = newll;
	  if (oldll != NULL) oldll->next = newll;
	  newll->previous = oldll;
	  newll->next = NULL;
	  oldll = newll;
     }

     /* Read the Cfmorpho lines */

     if (fread(&(nll),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(nll);
/*
  fprintf(stderr,"Number of Cfmorpho lines nll=%d\n",nll);
*/

     oldfll = newfll = NULL;
     for (i = 1; i <= nll; i++)
     {
	  newfll = _mw_read_cfml_mw2_cfml(fname,fp,need_flipping);
	  if (newfll == NULL)
	  {
	       mw_delete_cmimage(cmimage);
	       fclose(fp);
	       return(NULL);
	  }
	  if (cmimage->first_fml == NULL) cmimage->first_fml = newfll;
	  if (oldfll != NULL) oldfll->next = newfll;
	  newfll->previous = oldfll;
	  newfll->next = NULL;
	  oldfll = newfll;
     }

     /* Read the cmorpho sets */
     cmimage->first_ms = _mw_read_cmss_mw2_cmss(fname,fp,need_flipping);

     /* Read link between cmorpho lines and cmorpho sets and set pointers */
     if (cmimage->first_ml)
     {
	  mw_num_cmorpho_line(cmimage->first_ml);
	  /* First, read the number of couples */
	  if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
	  {
	       mw_delete_cmimage(cmimage);
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       fclose(fp);
	       return(NULL);
	  }
	  if (need_flipping == 1) _mw_in_flip_b4(size);
	  for (i=1,ll=cmimage->first_ml; i<=size; i++)
	  {
	       if ((fread(&(mlnum),sizeof(unsigned int),1,fp) == 0) ||
		   (fread(&(msnum),sizeof(unsigned int),1,fp) == 0))
	       {
		    mw_delete_cmimage(cmimage);
		    mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
		    fclose(fp);
		    return(NULL);
	       }
	       if (need_flipping == 1) 
	       {
		    _mw_in_flip_b4(mlnum);
		    _mw_in_flip_b4(msnum);
	       }
	       while (ll && (ll->num != mlnum)) ll=ll->next;
	       if (!ll) 
	       {
		    mw_delete_cmimage(cmimage);
		    mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
		    fclose(fp);
		    return(NULL);
	       }
	       /* Search the cmorpho sets containing the given cmorpho set number msnum */
	       for (mss=cmimage->first_ms; 
		    mss && mss->cmorphoset && (mss->cmorphoset->num != msnum); mss=mss->next);
	       if (!mss || !(mss->cmorphoset)) 
	       {
		    mw_delete_cmimage(cmimage);
		    mwerror(INTERNAL, 0,"[_mw_load_cmimage_mw2_cmimage] Cannot find cmorpho set number %d in file \"%s\"\n",msnum,fname);
		    fclose(fp);
		    return(NULL);
	       }
	       ll->cmorphosets = mss;
	       mss->cmorpholine = ll;
	  }
     }

     fclose(fp);
     return(cmimage);
}


/* Native formats (without conversion of the internal type) */

Cmimage _mw_cmimage_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_CMIMAGE") == 0)
	  return((Cmimage)_mw_load_cmimage_mw2_cmimage(fname));

     return(NULL);
}


/* All available formats */

Cmimage _mw_load_cmimage(char *fname, char *type)
{ 
     Cmimage mim;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     mim = _mw_cmimage_load_native(fname,type);
     if (mim != NULL) return(mim);

     /* If failed, try other formats with memory conversion */
     mim = (Cmimage) _mw_load_etype_to_itype(fname,mtype,"cmimage",type);
     if (mim != NULL) return(mim);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Cmimage !\n",fname,type);
     return NULL;
}

/* Write file in MW2_CMIMAGE format */  

short _mw_create_cmimage_mw2_cmimage(char *fname, Cmimage cmimage)
{
     FILE *fp;
     Cmorpho_line ll;
     Cfmorpho_line fll;
     unsigned int size,nll,n;

     if (cmimage == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_cmimage_mw2_cmimage] Cannot create file: Cmimage structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_CMIMAGE",1.00);
     if (fp == NULL) return(-1);

     size = strlen(cmimage->cmt);
     fwrite(&(size),sizeof(unsigned int),1,fp);  
     if (size > 0) fwrite(cmimage->cmt,sizeof(char),size,fp);
     size = strlen(cmimage->name);
     fwrite(&(size),sizeof(unsigned int),1,fp);  
     if (size > 0) fwrite(cmimage->name,sizeof(char),size,fp);

     fwrite(&(cmimage->nrow),sizeof(int),1,fp);
     fwrite(&(cmimage->ncol),sizeof(int),1,fp);
     fwrite(&(cmimage->minvalue.model),sizeof(unsigned char),1,fp);
     fwrite(&(cmimage->minvalue.red),sizeof(float),1,fp);
     fwrite(&(cmimage->minvalue.green),sizeof(float),1,fp);
     fwrite(&(cmimage->minvalue.blue),sizeof(float),1,fp);
     fwrite(&(cmimage->maxvalue.model),sizeof(unsigned char),1,fp);
     fwrite(&(cmimage->maxvalue.red),sizeof(float),1,fp);
     fwrite(&(cmimage->maxvalue.green),sizeof(float),1,fp);
     fwrite(&(cmimage->maxvalue.blue),sizeof(float),1,fp);

     /* Record the number of cmorpho lines */
     for (ll=cmimage->first_ml, nll=0; ll; ll=ll->next, nll++);

     fwrite(&(nll),sizeof(unsigned int),1,fp);

     for (ll=cmimage->first_ml, n=1; ll; ll=ll->next, n++)
	  _mw_write_cml_mw2_cml(fp,ll,n);

     /* Record the number of cfmorpho lines */
     for (fll=cmimage->first_fml, nll=0; fll; fll=fll->next, nll++);

     fwrite(&(nll),sizeof(unsigned int),1,fp);

     for (fll=cmimage->first_fml; fll; fll=fll->next)
	  _mw_write_cfml_mw2_cfml(fp,fll);

     /* Record the cmorpho sets */
     _mw_write_cmss_mw2_cmss(fp,cmimage->first_ms);

     /* Record the pointers between cmorpho lines and cmorpho sets */
     if (cmimage->first_ml)
     {
	  mw_num_cmorpho_line(cmimage->first_ml);
	  /* First, write the number of links */
	  size=0;
	  for (ll=cmimage->first_ml; ll; ll=ll->next)
	       if ((ll->cmorphosets)&&(ll->cmorphosets->cmorphoset)) size++;
	  fwrite(&(size),sizeof(unsigned int),1,fp);     
	  /* And now the couples (mlnum,msnum) */
	  for (ll=cmimage->first_ml; ll; ll=ll->next)
	       if ((ll->cmorphosets)&&(ll->cmorphosets->cmorphoset))
	       {
		    fwrite(&(ll->num),sizeof(unsigned int),1,fp);     
		    fwrite(&(ll->cmorphosets->cmorphoset->num),sizeof(unsigned int),1,fp);     
	       }
     }
     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_cmimage_create_native(char *fname, Cmimage cmimage, char *Type)
{
     if (strcmp(Type,"MW2_CMIMAGE") == 0)
	  return(_mw_create_cmimage_mw2_cmimage(fname,cmimage));
  
     return(-1);
}

/* Write file in different formats */
   
short _mw_create_cmimage(char *fname, Cmimage cmimage, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_cmimage_create_native(fname,cmimage,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,cmimage,"cmimage",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mimage_io.c
   
  Vers. 1.17
  Author : Jacques Froment
  Input/output functions for the 
  morpho_line
  Fmorpho_line
  Morpho_set
  Morpho_sets
  mimage
  structures

  Main changes :
  v1.17 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
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

#include "mimage.h"
#include "curve.h"
#include "fcurve.h"
#include "file_type.h"
#include "type_conv.h"
#include "mwio.h"

#include "mimage_io.h"


/* ---- I/O for morpho_line ---- */

/* Read one morpho_line from the file fp */

static Morpho_line _mw_read_ml_mw2_ml(char *fname, FILE *fp, int need_flipping)
{ 
     Morpho_line ll;
     Point_curve newpc,oldpc;
     Point_type newpt,oldpt;
     unsigned int npc, npt, i;

     ll = mw_new_morpho_line();
     if (ll == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }

     if (
	  (fread(&(ll->minvalue),sizeof(float),1,fp) == 0) || 
	  (fread(&(ll->maxvalue),sizeof(float),1,fp) == 0) || 
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
	  _mw_in_flip_float( (ll->minvalue) );
	  _mw_in_flip_float( (ll->maxvalue) );
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
	       mw_delete_morpho_line(ll);
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
	       mw_delete_morpho_line(ll);
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
	       mw_delete_morpho_line(ll);
	       return(NULL);
	  }
	  if (ll->first_type == NULL) ll->first_type = newpt;
	  if (oldpt != NULL) oldpt->next = newpt;
	  newpt->previous = oldpt;
	  newpt->next = NULL;
	  if (fread(&(newpt->type),sizeof(unsigned char),1,fp) == 0)
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       mw_delete_morpho_line(ll);
	       return(NULL);
	  }
	  oldpt=newpt;
     }
     return(ll);
}

/* Load one morpho_line from a file of MW2_MORPHO_LINE format */

Morpho_line _mw_load_ml_mw2_ml(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Morpho_line ll;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_MORPHO_LINE",15) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_ml_mw2_ml] File \"%s\" is not in the MW2_MORPHO_LINE format\n",fname);

     if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* read header */
     if (fread(header,hsize,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     ll = _mw_read_ml_mw2_ml(fname,fp,need_flipping);

     fclose(fp);
     return(ll);
}

/* Native formats (without conversion of the internal type) */

Morpho_line _mw_morpho_line_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_MORPHO_LINE") == 0)
	  return((Morpho_line)_mw_load_ml_mw2_ml(fname));

     return(NULL);
}


/* All available formats */

Morpho_line _mw_load_morpho_line(char *fname, char *type)
{ 
     Morpho_line ml;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     ml = _mw_morpho_line_load_native(fname,type);
     if (ml != NULL) return(ml);

     /* If failed, try other formats with memory conversion */
     ml = (Morpho_line) _mw_load_etype_to_itype(fname,mtype,"morpho_line",type);
     if (ml != NULL) return(ml);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Morpho_line !\n",fname,type);
     return NULL;
}


/* Write one morpho line in the file fp */  

void _mw_write_ml_mw2_ml(FILE *fp, Morpho_line ll, unsigned int nml)
{
     Point_curve pc;
     Point_type pt;
     unsigned int npc,npt;

     /* FIXME : unused parameter */
     nml = nml;

     fwrite(&(ll->minvalue),sizeof(float),1,fp);
     fwrite(&(ll->maxvalue),sizeof(float),1,fp);
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
	  mwerror(INTERNAL,1,"[_mw_write_ml_mw2_ml] Cannot create file: inconsistent Morpho_line structure \n");

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

/* Write file in MW2_MORPHO_LINE format */  

short _mw_create_ml_mw2_ml(char *fname, Morpho_line ll)
{
     FILE *fp;

     if (ll == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_ml_mw2_ml] Cannot create file: Morpho_line structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_MORPHO_LINE",1.00);
     if (fp == NULL) return(-1);

     _mw_write_ml_mw2_ml(fp,ll,1);

     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_morpho_line_create_native(char *fname, Morpho_line ll, char *Type)
{
     if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
	  return(_mw_create_ml_mw2_ml(fname,ll));
  
     return(-1);
}

/* Write file in different formats */
   
short _mw_create_morpho_line(char *fname, Morpho_line ll, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_morpho_line_create_native(fname,ll,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,ll,"morpho_line",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}

/* ---- I/O for Fmorpho_line ---- */

/* Read one fmorpho_line from the file fp */

static Fmorpho_line _mw_read_fml_mw2_fml(char *fname, FILE *fp,
					 int need_flipping)
{ 
     Fmorpho_line fll;
     Point_fcurve newpc,oldpc;
     Point_type newpt,oldpt;
     unsigned int npc, npt, i;

     fll = mw_new_fmorpho_line();
     if (fll == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }

     if (
	  (fread(&(fll->minvalue),sizeof(float),1,fp) == 0) || 
	  (fread(&(fll->maxvalue),sizeof(float),1,fp) == 0) || 
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
	  _mw_in_flip_float( (fll->minvalue) );
	  _mw_in_flip_float( (fll->maxvalue) );
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
	       mw_delete_fmorpho_line(fll);
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
	       mw_delete_fmorpho_line(fll);
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
	       mw_delete_fmorpho_line(fll);
	       return(NULL);
	  }
	  if (fll->first_type == NULL) fll->first_type = newpt;
	  if (oldpt != NULL) oldpt->next = newpt;
	  newpt->previous = oldpt;
	  newpt->next = NULL;
	  if (fread(&(newpt->type),sizeof(unsigned char),1,fp) == 0)
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       mw_delete_fmorpho_line(fll);
	       return(NULL);
	  }
	  oldpt = newpt;
     }
     return(fll);
}

/* Load Fmorpho_line from a file of MW2_FMORPHO_LINE format */

Fmorpho_line _mw_load_fml_mw2_fml(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Fmorpho_line fll;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_FMORPHO_LINE",16) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_fml_mw2_fml] File \"%s\" is not in the MW2_FMORPHO_LINE format\n",fname);

     if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* read header */
     if (fread(header,hsize,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     fll = _mw_read_fml_mw2_fml(fname,fp,need_flipping);

     fclose(fp);
     return(fll);
}


/* Native formats (without conversion of the internal type) */

Fmorpho_line _mw_fmorpho_line_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_FMORPHO_LINE") == 0)
	  return((Fmorpho_line)_mw_load_fml_mw2_fml(fname));

     return(NULL);
}


/* All available formats */

Fmorpho_line _mw_load_fmorpho_line(char *fname, char *type)
{ 
     Fmorpho_line ml;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     ml = _mw_fmorpho_line_load_native(fname,type);
     if (ml != NULL) return(ml);

     /* If failed, try other formats with memory conversion */
     ml = (Fmorpho_line) _mw_load_etype_to_itype(fname,mtype,"fmorpho_line",type);
     if (ml != NULL) return(ml);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Fmorpho_line !\n",fname,type);
     return NULL;
}

/* Write one fmorpho line in the file fp */  

void _mw_write_fml_mw2_fml(FILE *fp, Fmorpho_line fll)
{
     Point_fcurve pc;
     Point_type pt;
     unsigned int npc,npt;

     fwrite(&(fll->minvalue),sizeof(float),1,fp);
     fwrite(&(fll->maxvalue),sizeof(float),1,fp);
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
	  mwerror(INTERNAL,1,"[_mw_write_fml_mw2_fml] Cannot create file: inconsistent Fmorpho_line structure \n");

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

/* Write file in MW2_FMORPHO_LINE format */  

short _mw_create_fml_mw2_fml(char *fname, Fmorpho_line fll)
{
     FILE *fp;

     if (fll == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_fml_mw2_fml] Cannot create file: Fmorpho_line structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_FMORPHO_LINE",1.00);
     if (fp == NULL) return(-1);

     _mw_write_fml_mw2_fml(fp,fll);

     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_fmorpho_line_create_native(char *fname, Fmorpho_line ll, 
				     char *Type)
{
     if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
	  return(_mw_create_fml_mw2_fml(fname,ll));
  
     return(-1);
}

/* Write file in different formats */
   
short _mw_create_fmorpho_line(char *fname, Fmorpho_line ll, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_fmorpho_line_create_native(fname,ll,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,ll,"fmorpho_line",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}

/* ---- I/O for morpho_set ---- */

/* Read one morpho_set from the file fp */

Morpho_set _mw_read_ms_mw2_ms(char *fname, FILE *fp, int need_flipping)
{ 
     Morpho_set is;
     Hsegment news,olds;
     unsigned int ns, i;

     is = mw_new_morpho_set();
     if (is == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }

     if (
	  (fread(&(is->minvalue),sizeof(float),1,fp) == 0) || 
	  (fread(&(is->maxvalue),sizeof(float),1,fp) == 0) || 
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
	  _mw_in_flip_float( (is->minvalue) );
	  _mw_in_flip_float( (is->maxvalue) );
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
	       mw_delete_morpho_set(is);
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
	       mw_delete_morpho_set(is);
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

/* Load morpho_set from a file of MW2_MORPHO_SET format */

Morpho_set _mw_load_ms_mw2_ms(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Morpho_set is;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */
  
     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_MORPHO_SET",14) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_ms_mw2_ms] File \"%s\" is not in the MW2_MORPHO_SET format\n",fname);

     if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* read header */
     if (fread(header,hsize,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     is = _mw_read_ms_mw2_ms(fname,fp,need_flipping);

     fclose(fp);
     return(is);
}

/* Native formats (without conversion of the internal type) */

Morpho_set _mw_morpho_set_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_MORPHO_SET") == 0)
	  return((Morpho_set)_mw_load_ms_mw2_ms(fname));

     return(NULL);
}


/* All available formats */

Morpho_set _mw_load_morpho_set(char *fname, char *type)
{ 
     Morpho_set ms;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     ms = _mw_morpho_set_load_native(fname,type);
     if (ms != NULL) return(ms);

     /* If failed, try other formats with memory conversion */
     ms = (Morpho_set) _mw_load_etype_to_itype(fname,mtype,"morpho_set",type);
     if (ms != NULL) return(ms);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Morpho_set !\n",fname,type);
     return NULL;
}

/* Write one morpho set in the file fp */  

void _mw_write_ms_mw2_ms(FILE *fp, Morpho_set is)
{
     Hsegment s;
     unsigned int ns;

     /* Record the number of segments */
     for (s=is->first_segment, ns=0; s; s=s->next, ns++);

     fwrite(&(is->minvalue),sizeof(float),1,fp);
     fwrite(&(is->maxvalue),sizeof(float),1,fp);
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

/* Write file in MW2_MORPHO_SET format */  

short _mw_create_ms_mw2_ms(char *fname, Morpho_set is)
{
     FILE *fp;

     if (is == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_ms_mw2_ms] Cannot create file: Morpho_set structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_MORPHO_SET",1.00);
     if (fp == NULL) return(-1);

     _mw_write_ms_mw2_ms(fp,is);

     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_morpho_set_create_native(char *fname, Morpho_set ms, char *Type)
{
     if (strcmp(Type,"MW2_MORPHO_SET") == 0)
	  return(_mw_create_ms_mw2_ms(fname,ms));
  
     return(-1);
}

/* Write file in different formats */
   
short _mw_create_morpho_set(char *fname, Morpho_set ms, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_morpho_set_create_native(fname,ms,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,ms,"morpho_set",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}


/* ---- I/O for morpho_sets  ---- */

/* Read one morpho_sets from the file fp */

Morpho_sets _mw_read_mss_mw2_mss(char *fname, FILE *fp, int need_flipping)
{
     Morpho_sets iss,oldiss,newiss,p,q;
     unsigned int i,n,num;

     /* Get the number of morpho sets */
     if (fread(&(n),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  return(NULL);
     }

     if (need_flipping == 1) _mw_in_flip_b4(n);
     if (n == 0) return(NULL);
     /* printf("Number of morpho sets = %d\n",n); */

     iss = mw_new_morpho_sets();
     if (iss == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }

     /* Load the morpho sets */
     oldiss = NULL;
     newiss = iss;
     for (i = 1; i <= n; i++)
     {
	  if ((newiss == NULL)&&((newiss=mw_new_morpho_sets())==NULL))
	  {
	       mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	       mw_delete_morpho_sets(iss);
	       return(NULL);
	  }
	  newiss->morphoset = _mw_read_ms_mw2_ms(fname,fp,need_flipping);
	  if (newiss->morphoset== NULL)
	  {
	       mw_delete_morpho_sets(newiss);
	       mw_delete_morpho_sets(iss);
	       return(NULL);
	  }
	  if (oldiss != NULL) oldiss->next = newiss;
	  newiss->previous = oldiss;
	  newiss->next = NULL;
	  oldiss=newiss;
	  newiss=NULL;
     }
  
     /* Compute the Morpho set numbers */
     mw_num_morpho_sets(iss);

     /* Read the neighbor morpho sets (given by the Morpho set numbers list) */
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
	       /* Search the morpho sets containing the given Morpho set number */
	       for (q=iss; q && q->morphoset && (q->morphoset->num != num); q=q->next);
	       if (!q || !(q->morphoset)) 
	       {
		    mw_delete_morpho_sets(iss);
		    mwerror(INTERNAL, 0,"[_mw_load_mss_mw2_mss] Cannot find Morpho set number %d in file \"%s\"\n",num,fname);
	       }

	       if ((newiss=mw_new_morpho_sets())==NULL)
	       {
		    mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
		    mw_delete_morpho_sets(iss);
		    return(NULL);
	       }
	       newiss->morphoset = q->morphoset;
	       if (p->morphoset->neighbor == NULL) p->morphoset->neighbor=newiss;
	       if (oldiss) oldiss->next = newiss;
	       newiss->previous = oldiss;
	       oldiss = newiss;
	  }
     }
     return(iss);
}

/* Load morpho_sets from a file of MW2_MORPHO_SETS format */

Morpho_sets _mw_load_mss_mw2_mss(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Morpho_sets iss;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_MORPHO_SETS",15) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_mss_mw2_mss] File \"%s\" is not in the MW2_MORPHO_SETS format\n",fname);

     if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     /* read header */
     if (fread(header,hsize,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     iss = _mw_read_mss_mw2_mss(fname,fp,need_flipping);

     fclose(fp);
     return(iss);
}

/* Native formats (without conversion of the internal type) */

Morpho_sets _mw_morpho_sets_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_MORPHO_SETS") == 0)
	  return((Morpho_sets)_mw_load_mss_mw2_mss(fname));

     return(NULL);
}


/* All available formats */

Morpho_sets _mw_load_morpho_sets(char *fname, char *type)
{ 
     Morpho_sets ms;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     ms = _mw_morpho_sets_load_native(fname,type);
     if (ms != NULL) return(ms);

     /* If failed, try other formats with memory conversion */
     ms = (Morpho_sets) _mw_load_etype_to_itype(fname,mtype,"morpho_sets",type);
     if (ms != NULL) return(ms);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Morpho_sets !\n",fname,type);
     return NULL;
}


/* Write one morpho sets in the file fp */  

void _mw_write_mss_mw2_mss(FILE *fp, Morpho_sets iss)
{
     unsigned int n;
     Morpho_sets p,q,neig;

     /* Compute the Morpho set numbers */
     if (iss) n=mw_num_morpho_sets(iss); else n=0;
     /* fprintf(stderr,"Number of morpho sets = %d\n",n); */

     /* Record the number of morpho sets */
     fwrite(&(n),sizeof(unsigned int),1,fp);

     if (!iss || !iss->morphoset) return;

     /* Record each morpho set */
     for (p=iss; p; p=p->next) _mw_write_ms_mw2_ms(fp,p->morphoset);      

     /* Add the neighbor morpho sets (given by the Morpho set numbers list) */
     for (p=iss; p; p=p->next)
     {
	  /* Write first the number of neighbors */
	  neig = p->morphoset->neighbor;
	  n = mw_length_morpho_sets(neig);
	  /*
	    fprintf(stderr,"Number of neighbors = %d\n",n);
	  */
	  fwrite(&(n),sizeof(unsigned int),1,fp);
	  for (q=neig; q; q=q->next)
	       fwrite(&(q->morphoset->num),sizeof(unsigned int),1,fp);
     }
}

/* Write file in MW2_MORPHO_SETS format */  

short _mw_create_mss_mw2_mss(char *fname, Morpho_sets iss)
{
     FILE *fp;

     if (iss == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_mss_mw2_mss] Cannot create file: Morpho_sets structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_MORPHO_SETS",1.00);
     if (fp == NULL) return(-1);

     _mw_write_mss_mw2_mss(fp,iss);

     fclose(fp);
     return(0);
}


/* Create native formats (without conversion of the internal type) */

short _mw_morpho_sets_create_native(char *fname, Morpho_sets mss, char *Type)
{
     if (strcmp(Type,"MW2_MORPHO_SETS") == 0)
	  return(_mw_create_mss_mw2_mss(fname,mss));
  
     return(-1);
}

/* Write file in different formats */
   
short _mw_create_morpho_sets(char *fname, Morpho_sets mss, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_morpho_sets_create_native(fname,mss,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,mss,"morpho_sets",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}


/* ---- I/O for Mimage ---- */

/* Load Mimage from a file of MW2_MIMAGE format */

Mimage _mw_load_mimage_mw2_mimage(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Mimage mimage;
     Morpho_line ll,oldll,newll;
     Morpho_sets mss;
     Fmorpho_line oldfll,newfll;
     unsigned int size,nll,i,mlnum,msnum;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_MIMAGE",10) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_mimage_mw2_mimage] File \"%s\" is not in the MW2_MIMAGE format\n",fname);

     if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     /* read header = "MW2_MIMAGE" */
     if (fread(header,hsize,1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (header)...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     mimage = mw_new_mimage();
     if (mimage == NULL) 
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
     if ((size > 0)&& (fread(mimage->cmt,sizeof(char),size,fp) == 0))
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
     if ((size > 0)&&(fread(mimage->name,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name)...\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the other fields */
     if (
	  (fread(&(mimage->nrow),sizeof(int),1,fp) == 0) || 
	  (fread(&(mimage->ncol),sizeof(int),1,fp) == 0) || 
	  (fread(&(mimage->minvalue),sizeof(float),1,fp) == 0) ||
	  (fread(&(mimage->maxvalue),sizeof(float),1,fp) == 0) ||
	  (fread(&(nll),sizeof(unsigned int),1,fp) == 0)
	  )
     {      mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     if (need_flipping == 1)
     {
	  _mw_in_flip_b4(mimage->nrow);
	  _mw_in_flip_b4(mimage->ncol);
	  _mw_in_flip_float( (mimage->minvalue) );
	  _mw_in_flip_float( (mimage->maxvalue) );
	  _mw_in_flip_b4(nll);
     }


/*
  fprintf(stderr,"Number of Morpho lines nll=%d\n",nll);
*/

     oldll = newll = NULL;
     for (i = 1; i <= nll; i++)
     {
	  newll = _mw_read_ml_mw2_ml(fname,fp,need_flipping);
	  if (newll == NULL)
	  {
	       mw_delete_mimage(mimage);
	       fclose(fp);
	       return(NULL);
	  }
	  if (mimage->first_ml == NULL) mimage->first_ml = newll;
	  if (oldll != NULL) oldll->next = newll;
	  newll->previous = oldll;
	  newll->next = NULL;
	  oldll = newll;
     }

     /* Read the Fmorpho lines */

     if (fread(&(nll),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(nll);
/*
  fprintf(stderr,"Number of Fmorpho lines nll=%d\n",nll);
*/

     oldfll = newfll = NULL;
     for (i = 1; i <= nll; i++)
     {
	  newfll = _mw_read_fml_mw2_fml(fname,fp,need_flipping);
	  if (newfll == NULL)
	  {
	       mw_delete_mimage(mimage);
	       fclose(fp);
	       return(NULL);
	  }
	  if (mimage->first_fml == NULL) mimage->first_fml = newfll;
	  if (oldfll != NULL) oldfll->next = newfll;
	  newfll->previous = oldfll;
	  newfll->next = NULL;
	  oldfll = newfll;
     }

     /* Read the morpho sets */
     mimage->first_ms = _mw_read_mss_mw2_mss(fname,fp,need_flipping);

     /* Read link between morpho lines and morpho sets and set pointers */
     if (mimage->first_ml)
     {
	  mw_num_morpho_line(mimage->first_ml);
	  /* First, read the number of couples */
	  if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
	  {
	       mw_delete_mimage(mimage);
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       fclose(fp);
	       return(NULL);
	  }
	  if (need_flipping == 1) _mw_in_flip_b4(size);
	  for (i=1,ll=mimage->first_ml; i<=size; i++)
	  {
	       if ((fread(&(mlnum),sizeof(unsigned int),1,fp) == 0) ||
		   (fread(&(msnum),sizeof(unsigned int),1,fp) == 0))
	       {
		    mw_delete_mimage(mimage);
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
		    mw_delete_mimage(mimage);
		    mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
		    fclose(fp);
		    return(NULL);
	       }
	       /* Search the morpho sets containing the given morpho set number msnum */
	       for (mss=mimage->first_ms; 
		    mss && mss->morphoset && (mss->morphoset->num != msnum); mss=mss->next);
	       if (!mss || !(mss->morphoset)) 
	       {
		    mw_delete_mimage(mimage);
		    mwerror(INTERNAL, 0,"[_mw_load_mimage_mw2_mimage] Cannot find morpho set number %d in file \"%s\"\n",msnum,fname);
		    fclose(fp);
		    return(NULL);
	       }
	       ll->morphosets = mss;
	       mss->morpholine = ll;
	  }
     }

     fclose(fp);
     return(mimage);
}

/* Native formats (without conversion of the internal type) */

Mimage _mw_mimage_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_MIMAGE") == 0)
	  return((Mimage)_mw_load_mimage_mw2_mimage(fname));

     return(NULL);
}


/* All available formats */

Mimage _mw_load_mimage(char *fname, char *type)
{ 
     Mimage mim;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     mim = _mw_mimage_load_native(fname,type);
     if (mim != NULL) return(mim);

     /* If failed, try other formats with memory conversion */
     mim = (Mimage) _mw_load_etype_to_itype(fname,mtype,"mimage",type);
     if (mim != NULL) return(mim);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Mimage !\n",fname,type);
     return NULL;
}

/* Write file in MW2_MIMAGE format */  

short _mw_create_mimage_mw2_mimage(char *fname, Mimage mimage)
{
     FILE *fp;
     Morpho_line ll;
     Fmorpho_line fll;
     unsigned int size,nll,n;

     if (mimage == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_mimage_mw2_mimage] Cannot create file: Mimage structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_MIMAGE",1.00);
     if (fp == NULL) return(-1);

     size = strlen(mimage->cmt);
     fwrite(&(size),sizeof(unsigned int),1,fp);  
     if (size > 0) fwrite(mimage->cmt,sizeof(char),size,fp);
     size = strlen(mimage->name);
     fwrite(&(size),sizeof(unsigned int),1,fp);  
     if (size > 0) fwrite(mimage->name,sizeof(char),size,fp);

     fwrite(&(mimage->nrow),sizeof(int),1,fp);
     fwrite(&(mimage->ncol),sizeof(int),1,fp);
     fwrite(&(mimage->minvalue),sizeof(float),1,fp);
     fwrite(&(mimage->maxvalue),sizeof(float),1,fp);

     /* Record the number of morpho lines */
     for (ll=mimage->first_ml, nll=0; ll; ll=ll->next, nll++);

     fwrite(&(nll),sizeof(unsigned int),1,fp);

     for (ll=mimage->first_ml, n=1; ll; ll=ll->next, n++)
	  _mw_write_ml_mw2_ml(fp,ll,n);

     /* Record the number of fmorpho lines */
     for (fll=mimage->first_fml, nll=0; fll; fll=fll->next, nll++);

     fwrite(&(nll),sizeof(unsigned int),1,fp);

     for (fll=mimage->first_fml; fll; fll=fll->next)
	  _mw_write_fml_mw2_fml(fp,fll);

     /* Record the morpho sets */
     _mw_write_mss_mw2_mss(fp,mimage->first_ms);

     /* Record the pointers between morpho lines and morpho sets */
     if (mimage->first_ml)
     {
	  mw_num_morpho_line(mimage->first_ml);
	  /* First, write the number of links */
	  size=0;
	  for (ll=mimage->first_ml; ll; ll=ll->next)
	       if ((ll->morphosets)&&(ll->morphosets->morphoset)) size++;
	  fwrite(&(size),sizeof(unsigned int),1,fp);     
	  /* And now the couples (mlnum,msnum) */
	  for (ll=mimage->first_ml; ll; ll=ll->next)
	       if ((ll->morphosets)&&(ll->morphosets->morphoset))
	       {
		    fwrite(&(ll->num),sizeof(unsigned int),1,fp);     
		    fwrite(&(ll->morphosets->morphoset->num),sizeof(unsigned int),1,fp);     
	       }
     }
     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_mimage_create_native(char *fname, Mimage mimage, char *Type)
{
     if (strcmp(Type,"MW2_MIMAGE") == 0)
	  return(_mw_create_mimage_mw2_mimage(fname,mimage));
  
     return(-1);
}

/* Write file in different formats */
   
short _mw_create_mimage(char *fname, Mimage mimage, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_mimage_create_native(fname,mimage,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,mimage,"mimage",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
     return -1;
}


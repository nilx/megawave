/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   cmimage_io.c
   
   Vers. 1.2
   (C) 1999 Jacques Froment
   Input/output functions for the 
     Cmorpho_line
     Cfmorpho_line
     Cmorpho_set
     Cmorpho_sets
     Cmimage
   structures

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
#include <sys/types.h>
#include <sys/stat.h>

#include "mw.h"

/* ---- I/O for cmorpho_line ---- */

/* Read one cmorpho_line from the file fp */

static Cmorpho_line _mw_read_cml_mw2_cml(fname,fp,need_flipping)

char *fname;
FILE    *fp;
int need_flipping; /* 1 if flipping needed, 0 elsewhere */

{ 
  Cmorpho_line ll;
  Point_curve newpc,oldpc;
  Point_type newpt,oldpt;
  unsigned int npc, npt, i;
  unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */

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
      _mw_in_flip_float( &(ll->minvalue.red) );
      _mw_in_flip_float( &(ll->minvalue.green) );
      _mw_in_flip_float( &(ll->minvalue.blue) );
      _mw_in_flip_float( &(ll->maxvalue.red) );
      _mw_in_flip_float( &(ll->maxvalue.green) );
      _mw_in_flip_float( &(ll->maxvalue.blue) );
      _mw_in_flip_float( &(ll->data) );
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

Cmorpho_line _mw_load_cml_mw2_cml(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  char header[17];
  Cmorpho_line ll;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;

  need_flipping =  _mw_get_file_type(fname,ftype,mtype)-1;
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  /* read header = "MW2_CMORPHO_LINE" */
  if (fread(header,16,1,fp) == 0)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	fclose(fp);
	return(NULL);
      }
  if (strncmp(ftype,"MW2_CMORPHO_LINE",16) != 0)
    mwerror(INTERNAL, 0,"[_mw_load_cml_mw2_cml] File \"%s\" is not in the MW2_CMORPHO_LINE format\n",fname);

  ll = _mw_read_cml_mw2_cml(fname,fp,need_flipping);

  fclose(fp);
  return(ll);
}


/* Load cmorpho_line from file of different types */

Cmorpho_line _mw_load_cmorpho_line(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygon poly;
  Fpolygon fpoly;
  Cmimage cmimage;
  Cmorpho_line cmorpho_line;
  Cfmorpho_line cfmorpho_line;
  Mimage mimage;
  Morpho_line morpho_line;
  Fmorpho_line fmorpho_line;
  Curve curve;
 
#ifdef __STDC__
  Cmimage _mw_load_cmimage_mw2_cmimage(char *);
  Cfmorpho_line _mw_load_cfml_mw2_cfml(char *);
  Mimage _mw_load_mimage_mw2_mimage(char *);
  Fmorpho_line _mw_load_fml_mw2_fml(char *);
#else  
  Cmimage _mw_load_cmimage_mw2_cmimage();
  Cfmorpho_line _mw_load_cfml_mw2_cfml();
  Mimage _mw_load_mimage_mw2_mimage();
  Fmorpho_line _mw_load_fml_mw2_fml();
#endif

  _mw_get_file_type(fname,Type,mtype);

  /* Color Morpho structures */

  if (strcmp(Type,"MW2_CMORPHO_LINE") == 0)
    return(_mw_load_cml_mw2_cml(fname));

  if (strcmp(Type,"MW2_CFMORPHO_LINE") == 0)
    {
      cfmorpho_line = (Cfmorpho_line) _mw_load_cfml_mw2_cfml(fname);
      cmorpho_line = (Cmorpho_line) mw_cfmorpho_line_to_cmorpho_line(cfmorpho_line);
      mw_delete_cfmorpho_line(cfmorpho_line);
      return(cmorpho_line);
    }

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) _mw_load_cmimage_mw2_cmimage(fname);
      cmorpho_line = (Cmorpho_line) mw_cmimage_to_cmorpho_line(cmimage);
      mw_delete_cmimage(cmimage);
      return(cmorpho_line);
    }

  /* Gray levels Morpho structures */

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      morpho_line = (Morpho_line) _mw_load_ml_mw2_ml(fname);
      cmorpho_line = (Cmorpho_line) mw_morpho_line_to_cmorpho_line(morpho_line);
      mw_delete_morpho_line(morpho_line);
      return(cmorpho_line);
    }

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fmorpho_line = (Fmorpho_line) _mw_load_fml_mw2_fml(fname);
      cmorpho_line = (Cmorpho_line) mw_fmorpho_line_to_cmorpho_line(fmorpho_line);
      mw_delete_fmorpho_line(fmorpho_line);
      return(cmorpho_line);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      cmorpho_line = (Cmorpho_line) mw_mimage_to_cmorpho_line(mimage);
      mw_delete_mimage(mimage);
      return(cmorpho_line);
    }

  /* Else, try to recover the point curve only */
  {
    curve = (Curve) _mw_load_curve(fname,Type);
    cmorpho_line = (Cmorpho_line) mw_curve_to_cmorpho_line(curve);
    mw_delete_curve(curve);      
    return(cmorpho_line);
  }
}  


/* Write one cmorpho line in the file fp */  

void _mw_write_cml_mw2_cml(fp,ll,nml)

FILE *fp;
Cmorpho_line ll;
unsigned int nml;  /* Number of the cmorpho_line to write (for debug) */

{
  Point_curve pc;
  Point_type pt;
  unsigned int npc,npt;

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

short _mw_create_cml_mw2_cml(fname,ll)

char  *fname;                        /* file name */
Cmorpho_line ll;

{
  FILE *fp;

printf("_mw_create_cml_mw2_cml\n");
  if (ll == NULL)
    mwerror(INTERNAL,1,"[_mw_create_cml_mw2_cml] Cannot create file: Cmorpho_line structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_CMORPHO_LINE");
  if (fp == NULL) return(-1);

  _mw_write_cml_mw2_cml(fp,ll,1);

  fclose(fp);
  return(0);
}

/* Write file in different formats */
   
short _mw_create_cmorpho_line(fname,ll,Type)

char  *fname;                        /* file name */
Cmorpho_line ll;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Curve curve;
  Cmimage cmimage;
  Cfmorpho_line cfll;
  Mimage mimage;
  Fmorpho_line fll;
  Morpho_line ml;

#ifdef __STDC__
  short _mw_create_cmimage_mw2_cmimage(char *, Cmimage);
  short _mw_create_cfml_mw2_cfml(char *, Cfmorpho_line);
  short _mw_create_mimage_mw2_mimage(char *, Mimage);
  short _mw_create_fml_mw2_fml(char *, Fmorpho_line);
#else
  short _mw_create_cmimage_mw2_cmimage();
  short _mw_create_cfml_mw2_cfml();
  short _mw_create_mimage_mw2_mimage();
  short _mw_create_fml_mw2_fml();
#endif

  /* Color Morpho structures */

  if (strcmp(Type,"MW2_CMORPHO_LINE") == 0)
    return(_mw_create_cml_mw2_cml(fname,ll));

  if (strcmp(Type,"MW2_CFMORPHO_LINE") == 0)
    {
      cfll = (Cfmorpho_line) mw_cmorpho_line_to_cfmorpho_line(ll);
      ret = _mw_create_cfml_mw2_cfml(fname,cfll);
      mw_delete_cfmorpho_line(cfll);
      return(ret);
    }

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) mw_cmorpho_line_to_cmimage(ll);
      ret = _mw_create_cmimage_mw2_cmimage(fname,cmimage);
      mw_delete_cmimage(cmimage);
      return(ret);
    }

  /* Gray levels Morpho structures */

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ml = (Morpho_line) mw_cmorpho_line_to_morpho_line(ll);
      ret = _mw_create_ml_mw2_ml(fname,ml);
      mw_delete_morpho_line(ml);
      return(ret);
    }
  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fll = (Fmorpho_line) mw_cmorpho_line_to_fmorpho_line(ll);
      ret = _mw_create_fml_mw2_fml(fname,fll);
      mw_delete_fmorpho_line(fll);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) mw_cmorpho_line_to_mimage(ll);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      return(ret);
    }


  /* Else, try to save the point curve only */
    {
      curve = (Curve) mw_cmorpho_line_to_curve(ll);
      ret = _mw_create_curve_mw2_curve(fname,curve);
      mw_delete_curve(curve);
      return(ret);
    }
}

/* ---- I/O for Cfmorpho_line ---- */

/* Read one cfmorpho_line from the file fp */

static Cfmorpho_line _mw_read_cfml_mw2_cfml(fname,fp,need_flipping)

char *fname;
FILE    *fp;
int need_flipping; /* 1 if flipping needed, 0 elsewhere */

{ 
  Cfmorpho_line fll;
  Point_fcurve newpc,oldpc;
  Point_type newpt,oldpt;
  unsigned int npc, npt, i;
  unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */

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
      _mw_in_flip_float( &(fll->minvalue.red) );
      _mw_in_flip_float( &(fll->minvalue.green) );
      _mw_in_flip_float( &(fll->minvalue.blue) );
      _mw_in_flip_float( &(fll->maxvalue.red) );
      _mw_in_flip_float( &(fll->maxvalue.green) );
      _mw_in_flip_float( &(fll->maxvalue.blue) );
      _mw_in_flip_float( &(fll->data) );
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
	  _mw_in_flip_float(&(newpc->x));
	  _mw_in_flip_float(&(newpc->y));
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

Cfmorpho_line _mw_load_cfml_mw2_cfml(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  char header[18];
  Cfmorpho_line fll;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;

  need_flipping =  _mw_get_file_type(fname,ftype,mtype)-1;
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  /* read header = "MW2_CFMORPHO_LINE" */
  if (fread(header,17,1,fp) == 0)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	fclose(fp);
	return(NULL);
      }
  if (strncmp(ftype,"MW2_CFMORPHO_LINE",17) != 0)
    mwerror(INTERNAL, 0,"[_mw_load_cfml_mw2_cfml] File \"%s\" is not in the MW2_CFMORPHO_LINE format\n",fname);

  fll = _mw_read_cfml_mw2_cfml(fname,fp,need_flipping);

  fclose(fp);
  return(fll);
}


/* Load Cfmorpho_line from file of different types */

Cfmorpho_line _mw_load_cfmorpho_line(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygon poly;
  Fpolygon fpoly;
  Cmimage cmimage;
  Cfmorpho_line cfmorpho_line;
  Fcurve fcurve;

#ifdef __STDC__
  Cmimage _mw_load_cmimage_mw2_cmimage(char *);
#else  
  Cmimage _mw_load_cmimage_mw2_cmimage();
#endif

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_CFMORPHO_LINE") == 0)
    return(_mw_load_cfml_mw2_cfml(fname));

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) _mw_load_cmimage_mw2_cmimage(fname);
      cfmorpho_line = (Cfmorpho_line) mw_cmimage_to_cfmorpho_line(cmimage);
      mw_delete_cmimage(cmimage);
      return(cfmorpho_line);
    }

  /* Else, try to recover the point fcurve only */
  {
    fcurve = (Fcurve) _mw_load_fcurve(fname,Type);
    cfmorpho_line = (Cfmorpho_line) mw_fcurve_to_cfmorpho_line(fcurve);
    mw_delete_fcurve(fcurve);      
    return(cfmorpho_line);
  }
}  

/* Write one cfmorpho line in the file fp */  

void _mw_write_cfml_mw2_cfml(fp,fll)

FILE *fp;
Cfmorpho_line fll;

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

short _mw_create_cfml_mw2_cfml(fname,fll)

char  *fname;                        /* file name */
Cfmorpho_line fll;

{
  FILE *fp;

  if (fll == NULL)
    mwerror(INTERNAL,1,"[_mw_create_cfml_mw2_cfml] Cannot create file: Cfmorpho_line structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_CFMORPHO_LINE");
  if (fp == NULL) return(-1);

  _mw_write_cfml_mw2_cfml(fp,fll);

  fclose(fp);
  return(0);
}

/* Write file in different formats */
   
short _mw_create_cfmorpho_line(fname,fll,Type)

char  *fname;                        /* file name */
Cfmorpho_line fll;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Fcurve fcurve;
  Cmimage cmimage;

#ifdef __STDC__
  short _mw_create_cmimage_mw2_cmimage(char *, Cmimage);
#else
  short _mw_create_cmimage_mw2_cmimage();
#endif

  if (strcmp(Type,"MW2_CFMORPHO_LINE") == 0)
    return(_mw_create_cfml_mw2_cfml(fname,fll));

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) mw_cfmorpho_line_to_cmimage(fll);
      ret = _mw_create_cmimage_mw2_cmimage(fname,cmimage);
      mw_delete_cmimage(cmimage);
      return(ret);
    }

  /* Else, try to save the point fcurve only */
    {
      fcurve = (Fcurve) mw_cfmorpho_line_to_fcurve(fll);
      ret = _mw_create_fcurve_mw2_fcurve(fname,fcurve);
      mw_delete_fcurve(fcurve);
      return(ret);
    }
}

/* ---- I/O for cmorpho_set ---- */

/* Read one cmorpho_set from the file fp */

Cmorpho_set _mw_read_cms_mw2_cms(fname,fp,need_flipping)

char *fname;
FILE    *fp;
int need_flipping; /* 1 if flipping needed, 0 elsewhere */

{ 
  Cmorpho_set is;
  Segment news,olds;
  unsigned int ns, i;
  unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */

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
      _mw_in_flip_float( &(is->minvalue.red) );
      _mw_in_flip_float( &(is->minvalue.green) );
      _mw_in_flip_float( &(is->minvalue.blue) );
      _mw_in_flip_float( &(is->maxvalue.red) );
      _mw_in_flip_float( &(is->maxvalue.green) );
      _mw_in_flip_float( &(is->maxvalue.blue) );
      _mw_in_flip_b4(is->area);
      _mw_in_flip_b4(ns);
    }
  /* printf("Number of segments ns=%d\n",ns); */

  /* Load the segments */
  olds = news = NULL;
  for (i = 1; i <= ns; i++)
    {
      news = mw_new_segment();
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

Cmorpho_set _mw_load_cms_mw2_cms(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  char header[16];
  Cmorpho_set is;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;

  need_flipping =  _mw_get_file_type(fname,ftype,mtype)-1;
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  /* read header = "MW2_CMORPHO_SET" */
  if (fread(header,15,1,fp) == 0)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	fclose(fp);
	return(NULL);
      }
  if (strncmp(ftype,"MW2_CMORPHO_SET",15) != 0)
    mwerror(INTERNAL, 0,"[_mw_load_cms_mw2_cms] File \"%s\" is not in the MW2_CMORPHO_SET format\n",fname);

  is = _mw_read_cms_mw2_cms(fname,fp,need_flipping);

  fclose(fp);
  return(is);
}

/* Load cmorpho_set from file of different types */

Cmorpho_set _mw_load_cmorpho_set(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Cmimage cmimage;
  Cmorpho_set cmorpho_set;
  Cmorpho_sets cmorpho_sets;

#ifdef __STDC__
  Cmimage _mw_load_cmimage_mw2_cmimage(char *);
  Cmorpho_sets _mw_load_cmss_mw2_cmss(char *);
#else  
  Cmimage _mw_load_cmimage_mw2_cmimage();
  Cmorpho_sets _mw_load_cmss_mw2_cmss();
#endif

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_CMORPHO_SET") == 0)
    return(_mw_load_cms_mw2_cms(fname));

  if (strcmp(Type,"MW2_CMORPHO_SETS") == 0)
    {
      cmorpho_sets = (Cmorpho_sets) _mw_load_cmss_mw2_cmss(fname);
      cmorpho_set = (Cmorpho_set) mw_cmorpho_sets_to_cmorpho_set(cmorpho_sets);
      mw_delete_cmorpho_sets(cmorpho_sets);
      return(cmorpho_set);
    }

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) _mw_load_cmimage_mw2_cmimage(fname);
      cmorpho_sets = (Cmorpho_sets) mw_cmimage_to_cmorpho_sets(cmimage);
      cmorpho_set = (Cmorpho_set) mw_cmorpho_sets_to_cmorpho_set(cmorpho_sets);
      mw_delete_cmorpho_sets(cmorpho_sets);
      mw_delete_cmimage(cmimage);
      return(cmorpho_set);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write one cmorpho set in the file fp */  

void _mw_write_cms_mw2_cms(fp,is)

FILE *fp;
Cmorpho_set is;

{
  Segment s;
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

short _mw_create_cms_mw2_cms(fname,is)

char  *fname;                        /* file name */
Cmorpho_set is;

{
  FILE *fp;

  if (is == NULL)
    mwerror(INTERNAL,1,"[_mw_create_cms_mw2_cms] Cannot create file: Cmorpho_set structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_CMORPHO_SET");
  if (fp == NULL) return(-1);

  _mw_write_cms_mw2_cms(fp,is);

  fclose(fp);
  return(0);
}

/* Write file in different formats */
   
short _mw_create_cmorpho_set(fname,is,Type)

char  *fname;                        /* file name */
Cmorpho_set is;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Cmimage cmimage;
  Cmorpho_sets cmorpho_sets;

#ifdef __STDC__
  short _mw_create_cmimage_mw2_cmimage(char *, Cmimage);
  short _mw_create_cmss_mw2_cmss(char *, Cmorpho_sets);
#else
  short _mw_create_cmimage_mw2_cmimage();
  short _mw_create_cmss_mw2_cmss();
#endif

  if (strcmp(Type,"MW2_CMORPHO_SET") == 0)
    return(_mw_create_cms_mw2_cms(fname,is));

  if (strcmp(Type,"MW2_CMORPHO_SETS") == 0)
    {
      cmorpho_sets = (Cmorpho_sets) mw_cmorpho_set_to_cmorpho_sets(is);
      ret = _mw_create_cmss_mw2_cmss(fname,cmorpho_sets);
      mw_delete_cmorpho_sets(cmorpho_sets);
      return(ret);
    }

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmorpho_sets = (Cmorpho_sets) mw_cmorpho_set_to_cmorpho_sets(is);
      cmimage = (Cmimage) mw_cmorpho_sets_to_cmimage(cmorpho_sets);
      ret = _mw_create_cmimage_mw2_cmimage(fname,cmimage);
      mw_delete_cmimage(cmimage);
      mw_delete_cmorpho_sets(cmorpho_sets);
      return(ret);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}

/* ---- I/O for cmorpho_sets  ---- */

/* Read one cmorpho_sets from the file fp */

Cmorpho_sets _mw_read_cmss_mw2_cmss(fname,fp,need_flipping)

char *fname;
FILE    *fp;
int need_flipping; /* 1 if flipping needed, 0 elsewhere */

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
  mw_cmorpho_sets_num(iss);

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

Cmorpho_sets _mw_load_cmss_mw2_cmss(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  char header[17];
  Cmorpho_sets iss;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;

  need_flipping =  _mw_get_file_type(fname,ftype,mtype)-1;
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }
  /* read header = "MW2_CMORPHO_SETS" */
  if (fread(header,16,1,fp) == 0)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	fclose(fp);
	return(NULL);
      }
  if (strncmp(ftype,"MW2_CMORPHO_SETS",16) != 0)
    mwerror(INTERNAL, 0,"[_mw_load_cmss_mw2_cmss] File \"%s\" is not in the MW2_CMORPHO_SETS format\n",fname);

  iss = _mw_read_cmss_mw2_cmss(fname,fp,need_flipping);

  fclose(fp);
  return(iss);
}

/* Load cmorpho_sets from file of different types */

Cmorpho_sets _mw_load_cmorpho_sets(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Cmimage cmimage;
  Cmorpho_set cmorpho_set;
  Cmorpho_sets cmorpho_sets;

#ifdef __STDC__
  Cmimage _mw_load_cmimage_mw2_cmimage(char *);
#else  
  Cmimage _mw_load_cmimage_mw2_cmimage();
#endif

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_CMORPHO_SETS") == 0)
    return(_mw_load_cmss_mw2_cmss(fname));

  if (strcmp(Type,"MW2_CMORPHO_SET") == 0)
    {
      cmorpho_set = (Cmorpho_set) _mw_load_cms_mw2_cms(fname);
      cmorpho_sets = (Cmorpho_sets) mw_cmorpho_set_to_cmorpho_sets(cmorpho_set);
      mw_delete_cmorpho_set(cmorpho_set);
      return(cmorpho_sets);
    }

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) _mw_load_cmimage_mw2_cmimage(fname);
      cmorpho_sets = (Cmorpho_sets) mw_cmimage_to_cmorpho_sets(cmimage);
      mw_delete_cmimage(cmimage);
      return(cmorpho_sets);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write one cmorpho sets in the file fp */  

void _mw_write_cmss_mw2_cmss(fp,iss)

FILE *fp;
Cmorpho_sets iss;

{
  unsigned int n;
  Cmorpho_sets p,q,neig;

  /* Compute the Cmorpho set numbers */
  if (iss) n=mw_cmorpho_sets_num(iss); else n=0;
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
      n = mw_cmorpho_sets_length(neig);
      /*
	 fprintf(stderr,"Number of neighbors = %d\n",n);
      */
      fwrite(&(n),sizeof(unsigned int),1,fp);
      for (q=neig; q; q=q->next)
	fwrite(&(q->cmorphoset->num),sizeof(unsigned int),1,fp);
    }
}

/* Write file in MW2_CMORPHO_SETS format */  

short _mw_create_cmss_mw2_cmss(fname,iss)

char  *fname;                        /* file name */
Cmorpho_sets iss;

{
  FILE *fp;

  if (iss == NULL)
    mwerror(INTERNAL,1,"[_mw_create_cmss_mw2_cmss] Cannot create file: Cmorpho_sets structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_CMORPHO_SETS");
  if (fp == NULL) return(-1);

  _mw_write_cmss_mw2_cmss(fp,iss);

  fclose(fp);
  return(0);
}

/* Write file in different formats */
   
short _mw_create_cmorpho_sets(fname,iss,Type)

char  *fname;                        /* file name */
Cmorpho_sets iss;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Cmimage cmimage;
  Cmorpho_set cmorpho_set;

#ifdef __STDC__
  short _mw_create_cmimage_mw2_cmimage(char *, Cmimage);
#else
  short _mw_create_cmimage_mw2_cmimage();
#endif

  if (strcmp(Type,"MW2_CMORPHO_SETS") == 0)
    return(_mw_create_cmss_mw2_cmss(fname,iss));

  if (strcmp(Type,"MW2_CMORPHO_SET") == 0)
    {
      cmorpho_set = (Cmorpho_set) mw_cmorpho_sets_to_cmorpho_set(iss);
      ret = _mw_create_cms_mw2_cms(fname,cmorpho_set);
      mw_delete_cmorpho_set(cmorpho_set);
      return(ret);
    }

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) mw_cmorpho_sets_to_cmimage(iss);
      ret = _mw_create_cmimage_mw2_cmimage(fname,cmimage);
      mw_delete_cmimage(cmimage);
      return(ret);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}

/* ---- I/O for Cmimage ---- */

/* Load Cmimage from a file of MW2_CMIMAGE format */

Cmimage _mw_load_cmimage_mw2_cmimage(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  char header[12];
  Cmimage cmimage;
  Cmorpho_line ll,oldll,newll;
  Cmorpho_sets mss;
  Cfmorpho_line oldfll,newfll;
  unsigned int size,nll,i,mlnum,msnum;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;
  unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */

  need_flipping =  _mw_get_file_type(fname,ftype,mtype)-1;
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }
  /* read header = "MW2_CMIMAGE" */
  if (fread(header,11,1,fp) == 0)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\" (header)...\n",fname);
	fclose(fp);
	return(NULL);
      }
  if (strncmp(ftype,"MW2_CMIMAGE",11) != 0)
    mwerror(INTERNAL, 0,"[_mw_load_cmimage_mw2_cmimage] File \"%s\" is not in the MW2_CMIMAGE format\n",fname);

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
      _mw_in_flip_float( &(cmimage->minvalue.red) );
      _mw_in_flip_float( &(cmimage->minvalue.green) );
      _mw_in_flip_float( &(cmimage->minvalue.blue) );
      _mw_in_flip_float( &(cmimage->maxvalue.red) );
      _mw_in_flip_float( &(cmimage->maxvalue.green) );
      _mw_in_flip_float( &(cmimage->maxvalue.blue) );
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
      mw_cmorpho_line_num(cmimage->first_ml);
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

/* Load Cmimage from file of different types */

Cmimage _mw_load_cmimage(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygon poly;
  Fpolygon fpoly;
  Cmimage cmimage;
  Cmorpho_line ll;
  Cfmorpho_line fll;
  Curves curves;
  Cmorpho_set cmorpho_set;
  Cmorpho_sets cmorpho_sets;

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    return(_mw_load_cmimage_mw2_cmimage(fname));

  if (strcmp(Type,"MW2_CMORPHO_LINE") == 0)
    {
      ll = (Cmorpho_line) _mw_load_cml_mw2_cml(fname);
      cmimage = (Cmimage) mw_cmorpho_line_to_cmimage(ll);
      mw_delete_cmorpho_line(ll);
      return(cmimage);
    }

  if (strcmp(Type,"MW2_CFMORPHO_LINE") == 0)
    {
      fll = (Cfmorpho_line) _mw_load_cfml_mw2_cfml(fname);
      cmimage = (Cmimage) mw_cfmorpho_line_to_cmimage(fll);
      mw_delete_cfmorpho_line(fll);
      return(cmimage);
    }

  if (strcmp(Type,"MW2_CMORPHO_SET") == 0)
    {
      cmorpho_set = (Cmorpho_set) _mw_load_cms_mw2_cms(fname);
      cmorpho_sets = (Cmorpho_sets) mw_cmorpho_set_to_cmorpho_sets(cmorpho_set);
      cmimage = (Cmimage) mw_cmorpho_sets_to_cmimage(cmorpho_sets);
      mw_delete_cmorpho_set(cmorpho_set);
      mw_delete_cmorpho_sets(cmorpho_sets);
      return(cmimage);
    }

  if (strcmp(Type,"MW2_CMORPHO_SETS") == 0)
    {
      cmorpho_sets = (Cmorpho_sets) _mw_load_cmss_mw2_cmss(fname);
      cmimage = (Cmimage) mw_cmorpho_sets_to_cmimage(cmorpho_sets);
      mw_delete_cmorpho_sets(cmorpho_sets);
      return(cmimage);
    }

  /* Else, try to recover a curves structure */
  {
    curves = (Curves) _mw_load_curves(fname,Type);
    cmimage = (Cmimage) mw_curves_to_cmimage(curves);
    mw_delete_curves(curves);      
    return(cmimage);
  }
}  

/* Write file in MW2_CMIMAGE format */  

short _mw_create_cmimage_mw2_cmimage(fname, cmimage)

char  *fname;                        /* file name */
Cmimage cmimage;

{
  FILE *fp;
  Cmorpho_line ll;
  Cfmorpho_line fll;
  unsigned int size,nll,n;

  if (cmimage == NULL)
    mwerror(INTERNAL,1,"[_mw_create_cmimage_mw2_cmimage] Cannot create file: Cmimage structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_CMIMAGE");
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
      mw_cmorpho_line_num(cmimage->first_ml);
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

/* Write file in different formats */
   
short _mw_create_cmimage(fname,cmimage,Type)

char  *fname;                        /* file name */
Cmimage cmimage;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Curves curves;
  Cmorpho_line ll;
  Cfmorpho_line fll;
  Cmorpho_set cmorpho_set;
  Cmorpho_sets cmorpho_sets;

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    return(_mw_create_cmimage_mw2_cmimage(fname,cmimage));

  if (strcmp(Type,"MW2_CMORPHO_LINE") == 0)
    {
      ll = (Cmorpho_line) mw_cmimage_to_cmorpho_line(cmimage);
      ret = _mw_create_cml_mw2_cml(fname,ll);
      mw_delete_cmorpho_line(ll);
      return(ret);
    }

  if (strcmp(Type,"MW2_CFMORPHO_LINE") == 0)
    {
      fll = (Cfmorpho_line) mw_cmimage_to_cfmorpho_line(cmimage);
      ret = _mw_create_cfml_mw2_cfml(fname,fll);
      mw_delete_cfmorpho_line(fll);
      return(ret);
    }

  if (strcmp(Type,"MW2_CMORPHO_SET") == 0)
    {
      cmorpho_sets = (Cmorpho_sets) mw_cmimage_to_cmorpho_sets(cmimage);
      cmorpho_set = (Cmorpho_set) mw_cmorpho_sets_to_cmorpho_set(cmorpho_sets);
      ret = _mw_create_cms_mw2_cms(fname,cmorpho_set);
      mw_delete_cmorpho_set(cmorpho_set);
      mw_delete_cmorpho_sets(cmorpho_sets);
      return(ret);
    }

  if (strcmp(Type,"MW2_CMORPHO_SETS") == 0)
    {
      cmorpho_sets = (Cmorpho_sets) mw_cmimage_to_cmorpho_sets(cmimage);
      ret = _mw_create_cmss_mw2_cmss(fname,cmorpho_sets);
      mw_delete_cmorpho_sets(cmorpho_sets);
      return(ret);
    }

  /* Else, try to save the curves structure only */
    {
      curves = (Curves) mw_cmimage_to_curves(cmimage);
      ret = _mw_create_curves_mw2_curves(fname,curves);
      mw_delete_curves(curves);
      return(ret);
    }
}

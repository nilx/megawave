/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   mimage_io.c
   
   Vers. 1.7
   (C) 1996-99 Jacques Froment
   Input/output functions for the 
     morpho_line
     Fmorpho_line
     Morpho_set
     Morpho_sets
     mimage
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

/* ---- I/O for morpho_line ---- */

/* Read one morpho_line from the file fp */

static Morpho_line _mw_read_ml_mw2_ml(fname,fp,need_flipping)

char *fname;
FILE    *fp;
int need_flipping; /* 1 if flipping needed, 0 elsewhere */

{ 
  Morpho_line ll;
  Point_curve newpc,oldpc;
  Point_type newpt,oldpt;
  unsigned int npc, npt, i;
  unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */

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
      _mw_in_flip_float( &(ll->minvalue) );
      _mw_in_flip_float( &(ll->maxvalue) );
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

Morpho_line _mw_load_ml_mw2_ml(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  char header[16];
  Morpho_line ll;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;

  need_flipping =  _mw_get_file_type(fname,ftype,mtype)-1;
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  /* read header = "MW2_MORPHO_LINE" */
  if (fread(header,15,1,fp) == 0)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	fclose(fp);
	return(NULL);
      }
  if (strncmp(ftype,"MW2_MORPHO_LINE",15) != 0)
    mwerror(INTERNAL, 0,"[_mw_load_ml_mw2_ml] File \"%s\" is not in the MW2_MORPHO_LINE format\n",fname);

  ll = _mw_read_ml_mw2_ml(fname,fp,need_flipping);

  fclose(fp);
  return(ll);
}


/* Load morpho_line from file of different types */

Morpho_line _mw_load_morpho_line(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygon poly;
  Fpolygon fpoly;
  Mimage mimage;
  Cmimage cmimage;
  Morpho_line morpho_line;
  Cmorpho_line cmorpho_line;
  Fmorpho_line fmorpho_line;
  Cfmorpho_line cfmorpho_line;
  Curve curve;
 
#ifdef __STDC__
  Mimage _mw_load_mimage_mw2_mimage(char *);
  Cmimage _mw_load_cmimage_mw2_cmimage(char *);
  Fmorpho_line _mw_load_fml_mw2_fml(char *);
  Cmorpho_line _mw_load_cml_mw2_cml(char *);
  Cfmorpho_line _mw_load_cfml_mw2_cfml(char *);
#else  
  Mimage _mw_load_mimage_mw2_mimage();
  Cmimage _mw_load_cmimage_mw2_cmimage();
  Fmorpho_line _mw_load_fml_mw2_fml();
  Cmorpho_line _mw_load_cml_mw2_cml();
  Cfmorpho_line _mw_load_cfml_mw2_cfml();
#endif

  _mw_get_file_type(fname,Type,mtype);

  /* Gray levels Morpho structures */
  
  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    return(_mw_load_ml_mw2_ml(fname));

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fmorpho_line = (Fmorpho_line) _mw_load_fml_mw2_fml(fname);
      morpho_line = (Morpho_line) mw_fmorpho_line_to_morpho_line(fmorpho_line);
      mw_delete_fmorpho_line(fmorpho_line);
      return(morpho_line);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      morpho_line = (Morpho_line) mw_mimage_to_morpho_line(mimage);
      mw_delete_mimage(mimage);
      return(morpho_line);
    }

  /* Color Morpho structures */

  if (strcmp(Type,"MW2_CMORPHO_LINE") == 0)
    {
      cmorpho_line = (Cmorpho_line) _mw_load_cml_mw2_cml(fname);
      morpho_line = (Morpho_line) mw_cmorpho_line_to_morpho_line(cmorpho_line);
      mw_delete_cmorpho_line(cmorpho_line);
      return(morpho_line);
    }

  if (strcmp(Type,"MW2_CFMORPHO_LINE") == 0)
    {
      cfmorpho_line = (Cfmorpho_line) _mw_load_cfml_mw2_cfml(fname);
      morpho_line = (Morpho_line) mw_cfmorpho_line_to_morpho_line(cfmorpho_line);
      mw_delete_cfmorpho_line(cfmorpho_line);
      return(morpho_line);
    }

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) _mw_load_cmimage_mw2_cmimage(fname);
      morpho_line = (Morpho_line) mw_cmimage_to_morpho_line(cmimage);
      mw_delete_cmimage(cmimage);
      return(morpho_line);
    }

  /* Else, try to recover the point curve only */
  {
    curve = (Curve) _mw_load_curve(fname,Type);
    morpho_line = (Morpho_line) mw_curve_to_morpho_line(curve);
    mw_delete_curve(curve);      
    return(morpho_line);
  }
}  


/* Write one morpho line in the file fp */  

void _mw_write_ml_mw2_ml(fp,ll,nml)

FILE *fp;
Morpho_line ll;
unsigned int nml;  /* Number of the morpho_line to write (for debug) */

{
  Point_curve pc;
  Point_type pt;
  unsigned int npc,npt;

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

short _mw_create_ml_mw2_ml(fname,ll)

char  *fname;                        /* file name */
Morpho_line ll;

{
  FILE *fp;

  if (ll == NULL)
    mwerror(INTERNAL,1,"[_mw_create_ml_mw2_ml] Cannot create file: Morpho_line structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_MORPHO_LINE");
  if (fp == NULL) return(-1);

  _mw_write_ml_mw2_ml(fp,ll,1);

  fclose(fp);
  return(0);
}

/* Write file in different formats */
   
short _mw_create_morpho_line(fname,ll,Type)

char  *fname;                        /* file name */
Morpho_line ll;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Curve curve;
  Mimage mimage;
  Cmimage cmimage;
  Fmorpho_line fll;
  Cfmorpho_line cfll;
  Cmorpho_line cml;

#ifdef __STDC__
  short _mw_create_mimage_mw2_mimage(char *, Mimage);
  short _mw_create_cmimage_mw2_cmimage(char *, Cmimage);
  short _mw_create_fml_mw2_fml(char *, Fmorpho_line);
  short _mw_create_cfml_mw2_cfml(char *, Cfmorpho_line);
  short _mw_create_cml_mw2_cml(char *, Cmorpho_line);
#else
  short _mw_create_mimage_mw2_mimage();
  short _mw_create_cmimage_mw2_cmimage();
  short _mw_create_fml_mw2_fml();
  short _mw_create_cfml_mw2_cfml();
  short _mw_create_cml_mw2_cml();
#endif

  /* Gray levels Morpho structures */

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    return(_mw_create_ml_mw2_ml(fname,ll));

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fll = (Fmorpho_line) mw_morpho_line_to_fmorpho_line(ll);
      ret = _mw_create_fml_mw2_fml(fname,fll);
      mw_delete_fmorpho_line(fll);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) mw_morpho_line_to_mimage(ll);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      return(ret);
    }

  /* Color Morpho structures */

  if (strcmp(Type,"MW2_CMORPHO_LINE") == 0)
    {
      cml = (Cmorpho_line) mw_morpho_line_to_cmorpho_line(ll);
      ret = _mw_create_cml_mw2_cml(fname,cml);
      mw_delete_cmorpho_line(cml);
      return(ret);
    }

  if (strcmp(Type,"MW2_CFMORPHO_LINE") == 0)
    {
      cfll = (Cfmorpho_line) mw_morpho_line_to_cfmorpho_line(ll);
      ret = _mw_create_cfml_mw2_cfml(fname,cfll);
      mw_delete_cfmorpho_line(cfll);
      return(ret);
    }

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) mw_morpho_line_to_cmimage(ll);
      ret = _mw_create_cmimage_mw2_cmimage(fname,cmimage);
      mw_delete_cmimage(cmimage);
      return(ret);
    }

  /* Else, try to save the point curve only */
    {
      curve = (Curve) mw_morpho_line_to_curve(ll);
      ret = _mw_create_curve_mw2_curve(fname,curve);
      mw_delete_curve(curve);
      return(ret);
    }
}

/* ---- I/O for Fmorpho_line ---- */

/* Read one fmorpho_line from the file fp */

static Fmorpho_line _mw_read_fml_mw2_fml(fname,fp,need_flipping)

char *fname;
FILE    *fp;
int need_flipping; /* 1 if flipping needed, 0 elsewhere */

{ 
  Fmorpho_line fll;
  Point_fcurve newpc,oldpc;
  Point_type newpt,oldpt;
  unsigned int npc, npt, i;
  unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */

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
      _mw_in_flip_float( &(fll->minvalue) );
      _mw_in_flip_float( &(fll->maxvalue) );
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

Fmorpho_line _mw_load_fml_mw2_fml(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  char header[17];
  Fmorpho_line fll;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;

  need_flipping =  _mw_get_file_type(fname,ftype,mtype)-1;
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  /* read header = "MW2_FMORPHO_LINE" */
  if (fread(header,16,1,fp) == 0)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	fclose(fp);
	return(NULL);
      }
  if (strncmp(ftype,"MW2_FMORPHO_LINE",16) != 0)
    mwerror(INTERNAL, 0,"[_mw_load_fml_mw2_fml] File \"%s\" is not in the MW2_FMORPHO_LINE format\n",fname);

  fll = _mw_read_fml_mw2_fml(fname,fp,need_flipping);

  fclose(fp);
  return(fll);
}


/* Load Fmorpho_line from file of different types */

Fmorpho_line _mw_load_fmorpho_line(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygon poly;
  Fpolygon fpoly;
  Mimage mimage;
  Fmorpho_line fmorpho_line;
  Fcurve fcurve;

#ifdef __STDC__
  Mimage _mw_load_mimage_mw2_mimage(char *);
#else  
  Mimage _mw_load_mimage_mw2_mimage();
#endif

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    return(_mw_load_fml_mw2_fml(fname));

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      fmorpho_line = (Fmorpho_line) mw_mimage_to_fmorpho_line(mimage);
      mw_delete_mimage(mimage);
      return(fmorpho_line);
    }

  /* Else, try to recover the point fcurve only */
  {
    fcurve = (Fcurve) _mw_load_fcurve(fname,Type);
    fmorpho_line = (Fmorpho_line) mw_fcurve_to_fmorpho_line(fcurve);
    mw_delete_fcurve(fcurve);      
    return(fmorpho_line);
  }
}  

/* Write one fmorpho line in the file fp */  

void _mw_write_fml_mw2_fml(fp,fll)

FILE *fp;
Fmorpho_line fll;

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

short _mw_create_fml_mw2_fml(fname,fll)

char  *fname;                        /* file name */
Fmorpho_line fll;

{
  FILE *fp;

  if (fll == NULL)
    mwerror(INTERNAL,1,"[_mw_create_fml_mw2_fml] Cannot create file: Fmorpho_line structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_FMORPHO_LINE");
  if (fp == NULL) return(-1);

  _mw_write_fml_mw2_fml(fp,fll);

  fclose(fp);
  return(0);
}

/* Write file in different formats */
   
short _mw_create_fmorpho_line(fname,fll,Type)

char  *fname;                        /* file name */
Fmorpho_line fll;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Fcurve fcurve;
  Mimage mimage;

#ifdef __STDC__
  short _mw_create_mimage_mw2_mimage(char *, Mimage);
#else
  short _mw_create_mimage_mw2_mimage();
#endif

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    return(_mw_create_fml_mw2_fml(fname,fll));

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) mw_fmorpho_line_to_mimage(fll);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      return(ret);
    }

  /* Else, try to save the point fcurve only */
    {
      fcurve = (Fcurve) mw_fmorpho_line_to_fcurve(fll);
      ret = _mw_create_fcurve_mw2_fcurve(fname,fcurve);
      mw_delete_fcurve(fcurve);
      return(ret);
    }
}

/* ---- I/O for morpho_set ---- */

/* Read one morpho_set from the file fp */

Morpho_set _mw_read_ms_mw2_ms(fname,fp,need_flipping)

char *fname;
FILE    *fp;
int need_flipping; /* 1 if flipping needed, 0 elsewhere */

{ 
  Morpho_set is;
  Segment news,olds;
  unsigned int ns, i;
  unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */

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
      _mw_in_flip_float( &(is->minvalue) );
      _mw_in_flip_float( &(is->maxvalue) );
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

Morpho_set _mw_load_ms_mw2_ms(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  char header[15];
  Morpho_set is;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;

  need_flipping =  _mw_get_file_type(fname,ftype,mtype)-1;
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  /* read header = "MW2_MORPHO_SET" */
  if (fread(header,14,1,fp) == 0)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	fclose(fp);
	return(NULL);
      }
  if (strncmp(ftype,"MW2_MORPHO_SET",14) != 0)
    mwerror(INTERNAL, 0,"[_mw_load_ms_mw2_ms] File \"%s\" is not in the MW2_MORPHO_SET format\n",fname);

  is = _mw_read_ms_mw2_ms(fname,fp,need_flipping);

  fclose(fp);
  return(is);
}

/* Load morpho_set from file of different types */

Morpho_set _mw_load_morpho_set(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Mimage mimage;
  Morpho_set morpho_set;
  Morpho_sets morpho_sets;

#ifdef __STDC__
  Mimage _mw_load_mimage_mw2_mimage(char *);
  Morpho_sets _mw_load_mss_mw2_mss(char *);
#else  
  Mimage _mw_load_mimage_mw2_mimage();
  Morpho_sets _mw_load_mss_mw2_mss();
#endif

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_MORPHO_SET") == 0)
    return(_mw_load_ms_mw2_ms(fname));

  if (strcmp(Type,"MW2_MORPHO_SETS") == 0)
    {
      morpho_sets = (Morpho_sets) _mw_load_mss_mw2_mss(fname);
      morpho_set = (Morpho_set) mw_morpho_sets_to_morpho_set(morpho_sets);
      mw_delete_morpho_sets(morpho_sets);
      return(morpho_set);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      morpho_sets = (Morpho_sets) mw_mimage_to_morpho_sets(mimage);
      morpho_set = (Morpho_set) mw_morpho_sets_to_morpho_set(morpho_sets);
      mw_delete_morpho_sets(morpho_sets);
      mw_delete_mimage(mimage);
      return(morpho_set);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write one morpho set in the file fp */  

void _mw_write_ms_mw2_ms(fp,is)

FILE *fp;
Morpho_set is;

{
  Segment s;
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

short _mw_create_ms_mw2_ms(fname,is)

char  *fname;                        /* file name */
Morpho_set is;

{
  FILE *fp;

  if (is == NULL)
    mwerror(INTERNAL,1,"[_mw_create_ms_mw2_ms] Cannot create file: Morpho_set structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_MORPHO_SET");
  if (fp == NULL) return(-1);

  _mw_write_ms_mw2_ms(fp,is);

  fclose(fp);
  return(0);
}

/* Write file in different formats */
   
short _mw_create_morpho_set(fname,is,Type)

char  *fname;                        /* file name */
Morpho_set is;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Mimage mimage;
  Morpho_sets morpho_sets;

#ifdef __STDC__
  short _mw_create_mimage_mw2_mimage(char *, Mimage);
  short _mw_create_mss_mw2_mss(char *, Morpho_sets);
#else
  short _mw_create_mimage_mw2_mimage();
  short _mw_create_mss_mw2_mss();
#endif

  if (strcmp(Type,"MW2_MORPHO_SET") == 0)
    return(_mw_create_ms_mw2_ms(fname,is));

  if (strcmp(Type,"MW2_MORPHO_SETS") == 0)
    {
      morpho_sets = (Morpho_sets) mw_morpho_set_to_morpho_sets(is);
      ret = _mw_create_mss_mw2_mss(fname,morpho_sets);
      mw_delete_morpho_sets(morpho_sets);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      morpho_sets = (Morpho_sets) mw_morpho_set_to_morpho_sets(is);
      mimage = (Mimage) mw_morpho_sets_to_mimage(morpho_sets);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      mw_delete_morpho_sets(morpho_sets);
      return(ret);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}

/* ---- I/O for morpho_sets  ---- */

/* Read one morpho_sets from the file fp */

Morpho_sets _mw_read_mss_mw2_mss(fname,fp,need_flipping)

char *fname;
FILE    *fp;
int need_flipping; /* 1 if flipping needed, 0 elsewhere */

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
  mw_morpho_sets_num(iss);

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

Morpho_sets _mw_load_mss_mw2_mss(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  char header[16];
  Morpho_sets iss;
  char ftype[TYPE_SIZE],mtype[TYPE_SIZE];
  int need_flipping;

  need_flipping =  _mw_get_file_type(fname,ftype,mtype)-1;
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }
  /* read header = "MW2_MORPHO_SETS" */
  if (fread(header,15,1,fp) == 0)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	fclose(fp);
	return(NULL);
      }
  if (strncmp(ftype,"MW2_MORPHO_SETS",15) != 0)
    mwerror(INTERNAL, 0,"[_mw_load_mss_mw2_mss] File \"%s\" is not in the MW2_MORPHO_SETS format\n",fname);

  iss = _mw_read_mss_mw2_mss(fname,fp,need_flipping);

  fclose(fp);
  return(iss);
}

/* Load morpho_sets from file of different types */

Morpho_sets _mw_load_morpho_sets(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Mimage mimage;
  Morpho_set morpho_set;
  Morpho_sets morpho_sets;

#ifdef __STDC__
  Mimage _mw_load_mimage_mw2_mimage(char *);
#else  
  Mimage _mw_load_mimage_mw2_mimage();
#endif

  _mw_get_file_type(fname,Type,mtype);
  
  if (strcmp(Type,"MW2_MORPHO_SETS") == 0)
    return(_mw_load_mss_mw2_mss(fname));

  if (strcmp(Type,"MW2_MORPHO_SET") == 0)
    {
      morpho_set = (Morpho_set) _mw_load_ms_mw2_ms(fname);
      morpho_sets = (Morpho_sets) mw_morpho_set_to_morpho_sets(morpho_set);
      mw_delete_morpho_set(morpho_set);
      return(morpho_sets);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) _mw_load_mimage_mw2_mimage(fname);
      morpho_sets = (Morpho_sets) mw_mimage_to_morpho_sets(mimage);
      mw_delete_mimage(mimage);
      return(morpho_sets);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}  

/* Write one morpho sets in the file fp */  

void _mw_write_mss_mw2_mss(fp,iss)

FILE *fp;
Morpho_sets iss;

{
  unsigned int n;
  Morpho_sets p,q,neig;

  /* Compute the Morpho set numbers */
  if (iss) n=mw_morpho_sets_num(iss); else n=0;
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
      n = mw_morpho_sets_length(neig);
      /*
	 fprintf(stderr,"Number of neighbors = %d\n",n);
      */
      fwrite(&(n),sizeof(unsigned int),1,fp);
      for (q=neig; q; q=q->next)
	fwrite(&(q->morphoset->num),sizeof(unsigned int),1,fp);
    }
}

/* Write file in MW2_MORPHO_SETS format */  

short _mw_create_mss_mw2_mss(fname,iss)

char  *fname;                        /* file name */
Morpho_sets iss;

{
  FILE *fp;

  if (iss == NULL)
    mwerror(INTERNAL,1,"[_mw_create_mss_mw2_mss] Cannot create file: Morpho_sets structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_MORPHO_SETS");
  if (fp == NULL) return(-1);

  _mw_write_mss_mw2_mss(fp,iss);

  fclose(fp);
  return(0);
}

/* Write file in different formats */
   
short _mw_create_morpho_sets(fname,iss,Type)

char  *fname;                        /* file name */
Morpho_sets iss;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Mimage mimage;
  Morpho_set morpho_set;

#ifdef __STDC__
  short _mw_create_mimage_mw2_mimage(char *, Mimage);
#else
  short _mw_create_mimage_mw2_mimage();
#endif

  if (strcmp(Type,"MW2_MORPHO_SETS") == 0)
    return(_mw_create_mss_mw2_mss(fname,iss));

  if (strcmp(Type,"MW2_MORPHO_SET") == 0)
    {
      morpho_set = (Morpho_set) mw_morpho_sets_to_morpho_set(iss);
      ret = _mw_create_ms_mw2_ms(fname,morpho_set);
      mw_delete_morpho_set(morpho_set);
      return(ret);
    }

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    {
      mimage = (Mimage) mw_morpho_sets_to_mimage(iss);
      ret = _mw_create_mimage_mw2_mimage(fname,mimage);
      mw_delete_mimage(mimage);
      return(ret);
    }

  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",Type,fname);  
}

/* ---- I/O for Mimage ---- */

/* Load Mimage from a file of MW2_MIMAGE format */

Mimage _mw_load_mimage_mw2_mimage(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  char header[11];
  Mimage mimage;
  Morpho_line ll,oldll,newll;
  Morpho_sets mss;
  Fmorpho_line oldfll,newfll;
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
  /* read header = "MW2_MIMAGE" */
  if (fread(header,10,1,fp) == 0)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\" (header)...\n",fname);
	fclose(fp);
	return(NULL);
      }
  if (strncmp(ftype,"MW2_MIMAGE",10) != 0)
    mwerror(INTERNAL, 0,"[_mw_load_mimage_mw2_mimage] File \"%s\" is not in the MW2_MIMAGE format\n",fname);

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
      _mw_in_flip_float( &(mimage->minvalue) );
      _mw_in_flip_float( &(mimage->maxvalue) );
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
      mw_morpho_line_num(mimage->first_ml);
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

/* Load Mimage from file of different types */

Mimage _mw_load_mimage(fname,Type)

char  *fname;  /* Name of the file */
char  *Type;   /* Type de format du fichier */

{ char mtype[TYPE_SIZE];
  Polygon poly;
  Fpolygon fpoly;
  Mimage mimage;
  Cmimage cmimage;
  Morpho_line ll;
  Cmorpho_line cll;
  Cfmorpho_line cfll;
  Fmorpho_line fll;
  Curves curves;
  Morpho_set morpho_set;
  Morpho_sets morpho_sets;

#ifdef __STDC__
  Cmimage _mw_load_cmimage_mw2_cmimage(char *);
  Cmorpho_line _mw_load_cml_mw2_cml(char *);
  Cfmorpho_line _mw_load_cfml_mw2_cfml(char *);
#else  
  Cmimage _mw_load_cmimage_mw2_cmimage();
  Cmorpho_line _mw_load_cml_mw2_cml();
  Cfmorpho_line _mw_load_cfml_mw2_cfml();
#endif

  _mw_get_file_type(fname,Type,mtype);

  /* Gray level */

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    return(_mw_load_mimage_mw2_mimage(fname));

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) _mw_load_ml_mw2_ml(fname);
      mimage = (Mimage) mw_morpho_line_to_mimage(ll);
      mw_delete_morpho_line(ll);
      return(mimage);
    }

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fll = (Fmorpho_line) _mw_load_fml_mw2_fml(fname);
      mimage = (Mimage) mw_fmorpho_line_to_mimage(fll);
      mw_delete_fmorpho_line(fll);
      return(mimage);
    }

  if (strcmp(Type,"MW2_MORPHO_SET") == 0)
    {
      morpho_set = (Morpho_set) _mw_load_ms_mw2_ms(fname);
      morpho_sets = (Morpho_sets) mw_morpho_set_to_morpho_sets(morpho_set);
      mimage = (Mimage) mw_morpho_sets_to_mimage(morpho_sets);
      mw_delete_morpho_set(morpho_set);
      mw_delete_morpho_sets(morpho_sets);
      return(mimage);
    }

  if (strcmp(Type,"MW2_MORPHO_SETS") == 0)
    {
      morpho_sets = (Morpho_sets) _mw_load_mss_mw2_mss(fname);
      mimage = (Mimage) mw_morpho_sets_to_mimage(morpho_sets);
      mw_delete_morpho_sets(morpho_sets);
      return(mimage);
    }

  /* Color */

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) _mw_load_cmimage_mw2_cmimage(fname);
      mimage = (Mimage) mw_cmimage_to_mimage(cmimage);      
      mw_delete_cmimage(cmimage);
      return(mimage);
    }

  if (strcmp(Type,"MW2_CMORPHO_LINE") == 0)
    {
      cll = (Cmorpho_line) _mw_load_cml_mw2_cml(fname);
      mimage = (Mimage) mw_cmorpho_line_to_mimage(cll);
      mw_delete_cmorpho_line(cll);
      return(mimage);
    }

  /*
    if (strcmp(Type,"MW2_CFMORPHO_LINE") == 0)
    {
      cfll = (Cfmorpho_line) _mw_load_cfml_mw2_cfml(fname);
      mimage = (Mimage) mw_cfmorpho_line_to_mimage(cfll);
      mw_delete_cfmorpho_line(cfll);
      return(mimage);
    }
  */

  /* Else, try to recover a curves structure */
  {
    curves = (Curves) _mw_load_curves(fname,Type);
    mimage = (Mimage) mw_curves_to_mimage(curves);
    mw_delete_curves(curves);      
    return(mimage);
  }
}  

/* Write file in MW2_MIMAGE format */  

short _mw_create_mimage_mw2_mimage(fname, mimage)

char  *fname;                        /* file name */
Mimage mimage;

{
  FILE *fp;
  Morpho_line ll;
  Fmorpho_line fll;
  unsigned int size,nll,n;

  if (mimage == NULL)
    mwerror(INTERNAL,1,"[_mw_create_mimage_mw2_mimage] Cannot create file: Mimage structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_MIMAGE");
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
      mw_morpho_line_num(mimage->first_ml);
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

/* Write file in different formats */
   
short _mw_create_mimage(fname,mimage,Type)

char  *fname;                        /* file name */
Mimage mimage;
char  *Type;                         /* Type de format du fichier */

{
  short ret;
  Curves curves;
  Morpho_line ll;
  Cmorpho_line cll;
  Fmorpho_line fll;
  Morpho_set morpho_set;
  Morpho_sets morpho_sets;
  Cmimage cmimage;

#ifdef __STDC__
  short _mw_create_cmimage_mw2_cmimage(char *, Cmimage);
  short _mw_create_cfml_mw2_cfml(char *, Cfmorpho_line);
  short _mw_create_cml_mw2_cml(char *, Cmorpho_line);
#else
  short _mw_create_cmimage_mw2_cmimage();
  short _mw_create_cfml_mw2_cfml();
  short _mw_create_cml_mw2_cml();
#endif

  /* Gray level */

  if (strcmp(Type,"MW2_MIMAGE") == 0)
    return(_mw_create_mimage_mw2_mimage(fname,mimage));

  if (strcmp(Type,"MW2_MORPHO_LINE") == 0)
    {
      ll = (Morpho_line) mw_mimage_to_morpho_line(mimage);
      ret = _mw_create_ml_mw2_ml(fname,ll);
      mw_delete_morpho_line(ll);
      return(ret);
    }

  if (strcmp(Type,"MW2_FMORPHO_LINE") == 0)
    {
      fll = (Fmorpho_line) mw_mimage_to_fmorpho_line(mimage);
      ret = _mw_create_fml_mw2_fml(fname,fll);
      mw_delete_fmorpho_line(fll);
      return(ret);
    }

  if (strcmp(Type,"MW2_MORPHO_SET") == 0)
    {
      morpho_sets = (Morpho_sets) mw_mimage_to_morpho_sets(mimage);
      morpho_set = (Morpho_set) mw_morpho_sets_to_morpho_set(morpho_sets);
      ret = _mw_create_ms_mw2_ms(fname,morpho_set);
      mw_delete_morpho_set(morpho_set);
      mw_delete_morpho_sets(morpho_sets);
      return(ret);
    }

  if (strcmp(Type,"MW2_MORPHO_SETS") == 0)
    {
      morpho_sets = (Morpho_sets) mw_mimage_to_morpho_sets(mimage);
      ret = _mw_create_mss_mw2_mss(fname,morpho_sets);
      mw_delete_morpho_sets(morpho_sets);
      return(ret);
    }

  /* Color */

  if (strcmp(Type,"MW2_CMIMAGE") == 0)
    {
      cmimage = (Cmimage) mw_mimage_to_cmimage(mimage);
      ret = _mw_create_cmimage_mw2_cmimage(fname,cmimage);
      mw_delete_cmimage(cmimage);
      return(ret);
    }

  if (strcmp(Type,"MW2_CMORPHO_LINE") == 0)
    {
      cll = (Cmorpho_line) mw_mimage_to_cmorpho_line(mimage);
      ret = _mw_create_cml_mw2_cml(fname,cll);
      mw_delete_cmorpho_line(cll);
      return(ret);
    }

  /* Else, try to save the curves structure only */
    {
      curves = (Curves) mw_mimage_to_curves(mimage);
      ret = _mw_create_curves_mw2_curves(fname,curves);
      mw_delete_curves(curves);
      return(ret);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   dcurve_io.c
   
   Vers. 1.10
   (C) 2000-2002 Jacques Froment
   Input/Output functions for the dcurve & dcurves structure

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
#include <math.h>
#ifdef Linux
#include <unistd.h>
#endif

#include "mw.h"

/* Markers for old format 1.00 */

/* Marker to end a curve */
#define END_MARKER 1e+37
/* Threshold above which the value is said to be END_MARKER */
#define END_MARKER_THRESHOLD 1e+36

/* Markers for format 1.01 */

/* Marker to end a curve */
#define END_OF_CURVE 1e+308
/* Threshold above which the value is said to be END_OF_CURVE */
#define END_OF_CURVE_THRESHOLD 1e+307
/* End of curves */
#define END_OF_CURVES 1e+306
/* Threshold above which the value is said to be END_OF_CURVES */
#define END_OF_CURVES_THRESHOLD 1e+305

/* ----- I/O for Point_dcurve ----- */

Point_dcurve _mw_point_dcurve_load_native(fname,type)

char  *fname; /* Name of the file */
char  *type;  /* Type of the file */

{
  return(NULL);
}

/* ---- I/O for Dcurve ---- */

/* Max number of coordinates in the buffer (even number) */
#define MAXBUFSIZ 100000

/* Load dcurve from a file of MW2_DCURVE format */

Dcurve _mw_load_dcurve_mw2_dcurve(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Dcurve cv;
  Point_dcurve newcvc,oldcvc;
  struct stat buf;
  int fsize,buf_size,i;
  int readsize,remainsize;
  double vx,vy,*buffer;
  register double *ptr;
  char ftype[mw_ftype_size],mtype[mw_mtype_size];
  int need_flipping;
  unsigned long *lptr; /* for flip */
  unsigned long flip_double; /* buffer for macro _mw_in_flip_double */
  int hsize;  /* Size of the header, in bytes */
  float version;/* Version number of the file format */

  need_flipping = _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
  if (strcmp(ftype,"MW2_DCURVE") != 0)
    mwerror(INTERNAL, 0,"[_mw_load_dcurve_mw2_dcurve] File \"%s\" is not in the MW2_DCURVE format\n",fname);

  
  if ( (need_flipping==-1) ||
      (!(fp = fopen(fname, "r"))) || (fstat(fileno(fp),&buf) != 0) )
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  /* Size of the file - size of the header, in bytes */
  fsize = buf.st_size-hsize; 
  remainsize = fsize / sizeof(double); /* Number of coordinates */
  if ((remainsize % 2) != 0)
	{
	  mwerror(ERROR, 0,"Error into the file \"%s\": not an even number of coordinates !\n",fname);
	  return(NULL);
	}
  
  buf_size=remainsize;
  if (hsize>buf_size) buf_size=hsize;
  if (buf_size > MAXBUFSIZ) buf_size=MAXBUFSIZ;
  if ( (!(buffer = (double *) malloc(buf_size * sizeof(double)))) ||
       (!(cv = mw_new_dcurve())))
    {
      mwerror(ERROR, 0,"Not enough memory to dcurve file \"%s\" !\n",fname);
      fclose(fp);
      return(NULL);
    }

   if (fread(buffer,1,hsize,fp) != hsize)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	fclose(fp);
	return(NULL);
      }

  oldcvc = newcvc = NULL;

  while (remainsize > 0)
  {
    readsize=remainsize;
    if (readsize > buf_size) readsize=buf_size;
    if (fread(buffer,sizeof(double),readsize,fp) != readsize)
      {
	mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	free(buffer);
	fclose(fp);
	return(NULL);
      }
    remainsize -= readsize;
      
    for (ptr=buffer, i=1; i < readsize; i+=2)
      {
	if (need_flipping==1)
	  {	
	    lptr= (unsigned long *) ptr;
	    _mw_in_flip_double(lptr);
	    vx=*((double *) lptr);
	    ptr++;
	    lptr= (unsigned long *) ptr;
	    _mw_in_flip_double(lptr);
	    vy=*((double *) lptr);
	    ptr++;
	    /*printf("(flip) i=%d (%g,%g)\n",i,vx,vy); */
	  }
	else
	  {	
	    vx = *ptr++;
	    vy = *ptr++;
	    /* printf("(no flip) i=%d (%g,%g)\n",i,vx,vy); */
	  }
      newcvc = mw_new_point_dcurve();
      if (newcvc == NULL)
	{
	  mw_delete_dcurve(cv);
	  free(buffer);
	  return(NULL);
	}
      if (cv->first == NULL) cv->first = newcvc;
      if (oldcvc != NULL) oldcvc->next = newcvc;
      newcvc->previous = oldcvc;
      newcvc->next = NULL;
      newcvc->x = vx; newcvc->y = vy;
      oldcvc = newcvc;
      }
  }
  
  fclose(fp);
  free(buffer);
  return(cv);
}

/* Native formats (without conversion of the internal type) */

Dcurve _mw_dcurve_load_native(fname,type)

char  *fname; /* Name of the file */
char  *type;  /* Type of the file */

{
  if (strcmp(type,"MW2_DCURVE") == 0)
    return((Dcurve)_mw_load_dcurve_mw2_dcurve(fname));

  return(NULL);
}


/* All available formats */

Dcurve _mw_load_dcurve(fname,type)

char  *fname; /* Name of the file */
char  *type;  /* Type of the file */

{ 
  Dcurve cv;
  char mtype[mw_mtype_size];
  int hsize;  /* Size of the header, in bytes */
  float version;/* Version number of the file format */

  _mw_get_file_type(fname,type,mtype,&hsize,&version);

  /* First, try native formats */
  cv = _mw_dcurve_load_native(fname,type);
  if (cv != NULL) return(cv);

  /* If failed, try other formats with memory conversion */
  cv = (Dcurve) _mw_load_etype_to_itype(fname,mtype,"dcurve",type);
  if (cv != NULL) return(cv);

  if (type[0]=='?')
    mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
  else
    mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Dcurve !\n",fname,type);
}


/* Write file in MW2_DCURVE format */  

short _mw_create_dcurve_mw2_dcurve(fname,cv)

char  *fname;                        /* file name */
Dcurve cv;

{
  FILE *fp;
  Point_dcurve pc;
  int n;
  double vx=0.0,vy=0.0;

  if (cv == NULL)
    mwerror(INTERNAL,1,"[_mw_create_dcurve_mw2_dcurve] Cannot create file: Dcurve structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_DCURVE",1.00);
  if (fp == NULL) return(-1);
  
  /* Allow to create empty dcurve
  if (cv->first == NULL)
    mwerror(INTERNAL,1,"[_mw_create_dcurve_mw2_dcurve] Dcurve has no point dcurve !\n");
  */

  for (pc=cv->first; pc; pc=pc->next)
    {
      vx = pc->x;
      vy = pc->y;
      fwrite(&vx,sizeof(double),1,fp);
      fwrite(&vy,sizeof(double),1,fp);
    }

  fclose(fp);
  return(0);
}
   

/* Create native formats (without conversion of the internal type) */

short _mw_dcurve_create_native(fname,cv,Type)

char  *fname;                        /* file name */
Dcurve cv;
char  *Type;                         /* Type de format du fichier */

{
  if (strcmp(Type,"MW2_DCURVE") == 0)
    return(_mw_create_dcurve_mw2_dcurve(fname,cv));
  
  return(-1);
}


/* Write file in different formats */
   
short _mw_create_dcurve(fname,cv,Type)

char  *fname;                        /* file name */
Dcurve cv;
char  *Type;                         /* Type de format du fichier */

{
  short ret;

  /* First, try native formats */
  ret = _mw_dcurve_create_native(fname,cv,Type);
  if (ret == 0) return(0);

  /* If failed, try other formats with memory conversion */
  ret = _mw_create_etype_from_itype(fname,cv,"dcurve",Type);
  if (ret == 0) return(0);

  /* Invalid Type should have been detected before, but we can arrive here because
     of a write failure (e.g. the output file name is write protected).
  */
  mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
}

/* ---- I/O for Dcurves ---- */

/* Load dcurves from a file of MW2_DCURVES format 
   Read operation is no more bufferized to be able to load huge dcurves.
 */

/* Load old format V1.00 */

Dcurves _mw_load_dcurves_mw2_dcurves_1_00(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Dcurves cvs;
  Dcurve newcv,oldcv;
  Point_dcurve newcvc,oldcvc;
  double x,y;
  unsigned long *px,*py;
  char new_dcurve;
  char ftype[mw_ftype_size],mtype[mw_mtype_size];
  int need_flipping; 
  char header[BUFSIZ];
  double xsave;
  int eof;
  unsigned long flip_double; /* buffer for macro _mw_in_flip_double */
  int hsize;  /* Size of the header, in bytes */
  float version;/* Version number of the file format */

  need_flipping = _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
  if (strcmp(ftype,"MW2_DCURVES") != 0)
    mwerror(INTERNAL, 0,"[_mw_load_dcurves_mw2_dcurves] File \"%s\" is not in the MW2_DCURVES format\n",fname);
  
  
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))))
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  if (fread(header,1,hsize,fp) != hsize)
    {
      mwerror(ERROR, 0,"Error while reading header of file \"%s\" !\n",fname);
      fclose(fp);
      return(NULL);
    }

  /*
    printf("hsize=%d version=%f need_flipping=%d\n",hsize,version,need_flipping);
  */

  cvs = mw_new_dcurves();
  if (cvs == NULL) return(cvs);
  oldcv = newcv = NULL;

  new_dcurve = 1;
  xsave=END_MARKER;
  eof=0;
  while (eof != 1)
    {
      if (new_dcurve == 1)  /* Begin a new dcurve */
	{
	  oldcv = newcv;
	  newcv = mw_new_dcurve();
	  if (newcv == NULL)
	    {
	      mw_delete_dcurves(cvs);
	      return(NULL);
	    }
	  if (cvs->first == NULL) cvs->first = newcv;
	  if (oldcv != NULL) oldcv->next = newcv;
	  newcv->previous = oldcv;
	  newcv->next = NULL;
	  newcv->first = NULL;
	  oldcvc = newcvc = NULL;
	}
      
      if (xsave >= END_MARKER_THRESHOLD) /* Has to read x from file */
	{
	  if (fread(&x,1,sizeof(double),fp) != sizeof(double))
	    {
	      mwerror(ERROR, 0,
		      "Error into the file \"%s\" : EOF encountered before EOC.\n",fname);
	      return(NULL);		
	    }
	  if (need_flipping==1) 
	    {
	      px= (unsigned long *) &x;
	      _mw_in_flip_double(px);
	      x=*((double *)px);
	    }
	}	
      
      else x=xsave; /* set previous value */
      
      if (fread(&y,1,sizeof(double),fp) != sizeof(double))
	{
	  mwerror(ERROR, 0,
		  "Error into the file \"%s\" : cannot read y coordinate\n",fname);
	  return(NULL);		    
	}		
      if (need_flipping==1) 
	{
	  py= (unsigned long *) &y;
	  _mw_in_flip_double(py);
	  y=*((double *)py);
	}
      if (x>=END_MARKER_THRESHOLD)
	{
	  xsave=y;
	  if (y>=END_MARKER_THRESHOLD)
	    {
	      if (fread(&y,1,sizeof(double),fp) != 0)
		{
		  mwerror(ERROR, 0,
			  "Error into the file \"%s\" : no EOF after double EOC.\n",fname);
		  return(NULL);		    
		}
	      else eof=1;
	    }
	  else new_dcurve=1;
	}
      else /* Record (x,y) */
	{
	  xsave=END_MARKER;
	  new_dcurve = 0;
	  newcvc = mw_new_point_dcurve();
	  if (newcvc == NULL)
	    {
	      mw_delete_dcurves(cvs);
	      return(NULL);
	    }
	  if (newcv->first == NULL) newcv->first = newcvc;
	  if (oldcvc != NULL) oldcvc->next = newcvc;
	  newcvc->previous = oldcvc;
	  newcvc->next = NULL;
	  newcvc->x = x; newcvc->y = y;
	  oldcvc = newcvc;
	}
    }      /* while (eof != 1) */
  fclose(fp);
  return(cvs);
}

/* Load current format V1.01 
   Changes from V1.00 : - END_OF_CURVES introduced to be able
                          to record empty curve;
			- markers values changed (to double).
*/


Dcurves _mw_load_dcurves_mw2_dcurves(fname)

char  *fname;  /* Name of the file */

{ FILE    *fp;
  Dcurves cvs;
  Dcurve newcv,oldcv;
  Point_dcurve newcvc,oldcvc;
  double X[2];
  int n;
  unsigned long *px;
  char ftype[mw_ftype_size],mtype[mw_mtype_size];
  int need_flipping; 
  char header[BUFSIZ];
  unsigned long flip_double; /* buffer for macro _mw_in_flip_double */
  int hsize;  /* Size of the header, in bytes */
  float version;/* Version number of the file format */

  need_flipping = _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
  if (strcmp(ftype,"MW2_DCURVES") != 0)
    mwerror(INTERNAL, 0,"[_mw_load_dcurves_mw2_dcurves] File \"%s\" is not in the MW2_DCURVES format\n",fname);
  
  
  /*printf("[_mw_load_dcurves_mw2_dcurves] version=%f\n",version);*/
  if (version==1.0) return(_mw_load_dcurves_mw2_dcurves_1_00(fname));
  
  if ( (need_flipping==-1) || (!(fp = fopen(fname, "r"))))
    {
      mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
      fclose(fp);
      return(NULL);
    }

  if (fread(header,1,hsize,fp) != hsize)
    {
      mwerror(ERROR, 0,"Error while reading header of file \"%s\" !\n",fname);
      fclose(fp);
      return(NULL);
    }

  cvs = mw_new_dcurves();
  if (cvs == NULL) return(cvs);
  oldcv = newcv = NULL;

  n=0;
  while (1)
    {
      if (fread(&X[n],1,sizeof(double),fp) != sizeof(double))
	{
	  mwerror(ERROR, 0,
		  "Error into the file \"%s\" : EOF encountered before END_OF_CURVES.\n",fname);
	  return(NULL);		
	}
      if (need_flipping==1) 
	{
	  px= (unsigned long *) &X[n];
	  _mw_in_flip_double(px);
	  X[n]=*((double *)px);
	}
      if ((X[n]<END_OF_CURVE_THRESHOLD)&&(X[n]>=END_OF_CURVES_THRESHOLD))
	{
	  fclose(fp);
	  return(cvs);
	}
      else
	if (X[n]>=END_OF_CURVE_THRESHOLD)
	  {
	    n=-1;
	    oldcv = newcv;
	    newcv = mw_new_dcurve();
	    if (newcv == NULL)
	      {
		mw_delete_dcurves(cvs);
		fclose(fp);
		return(NULL);
	      }
	    if (cvs->first == NULL) cvs->first = newcv;
	    if (oldcv != NULL) oldcv->next = newcv;
	    newcv->previous = oldcv;
	    newcv->next = NULL;
	    newcv->first = NULL;
	    oldcvc = newcvc = NULL;
	  }
	else
	  if (n==1) /* record new point */
	    {
	      newcvc = mw_new_point_dcurve();
	      if (newcvc == NULL)
		{
		  mw_delete_dcurves(cvs);
		  return(NULL);
		}
	      if (newcv->first == NULL) newcv->first = newcvc;
	      if (oldcvc != NULL) oldcvc->next = newcvc;
	      newcvc->previous = oldcvc;
	      newcvc->next = NULL;
	      newcvc->x = X[0]; newcvc->y = X[1];
	      oldcvc = newcvc;
	    }
      n++; if (n==2) n=0;
    }
}


/* Native formats (without conversion of the internal type) */

Dcurves _mw_dcurves_load_native(fname,type)

char  *fname; /* Name of the file */
char  *type;  /* Type of the file */

{
  if (strcmp(type,"MW2_DCURVES") == 0)
    return((Dcurves)_mw_load_dcurves_mw2_dcurves(fname));

  return(NULL);
}

/* All available formats */

Dcurves _mw_load_dcurves(fname,type)

char  *fname; /* Name of the file */
char  *type;  /* Type of the file */

{ 
  Dcurves cvs;
  char mtype[mw_mtype_size];
  int hsize;  /* Size of the header, in bytes */
  float version;/* Version number of the file format */

  _mw_get_file_type(fname,type,mtype,&hsize,&version);

  /* First, try native formats */
  cvs = _mw_dcurves_load_native(fname,type);
  if (cvs != NULL) return(cvs);

  /* If failed, try other formats with memory conversion */
  cvs = (Dcurves) _mw_load_etype_to_itype(fname,mtype,"dcurves",type);
  if (cvs != NULL) return(cvs);

  if (type[0]=='?')
    mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
  else
    mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Dcurves !\n",fname,type);
}


/* Write file in MW2_DCURVES format */  

short _mw_create_dcurves_mw2_dcurves(fname,cvs)

char  *fname;                        /* file name */
Dcurves cvs;

{
  FILE *fp;
  Dcurve pl;
  Point_dcurve pc;
  int n;
  double vx=0.0,vy=0.0,end_of_dcurve=END_OF_CURVE;
  double end_of_dcurves = END_OF_CURVES;


  if (cvs == NULL)
    mwerror(INTERNAL,1,"[_mw_create_dcurves_mw2_dcurves] Cannot create file: Dcurves structure is NULL\n");

  fp=_mw_write_header_file(fname,"MW2_DCURVES",1.01);
  if (fp == NULL) return(-1);

  if (cvs->first) fwrite(&end_of_dcurve,sizeof(double),1,fp);
  for (pl=cvs->first, n=1; pl; pl=pl->next, n++)
    {
      for (pc=pl->first; pc; pc=pc->next)
	{
	  if ((pc->x >= END_OF_CURVES_THRESHOLD) ||
	      (pc->y >= END_OF_CURVES_THRESHOLD))
	    mwerror(INTERNAL,1,"[_mw_create_dcurves_mw2_dcurves] Dcurve #%d has a point which coordinates (%f,%f) exceed double capacity\n",n,pc->x,pc->y);
	  vx = pc->x;
	  vy = pc->y;
	  fwrite(&vx,sizeof(double),1,fp);
	  fwrite(&vy,sizeof(double),1,fp);
	}
      if (pl->next) fwrite(&end_of_dcurve,sizeof(double),1,fp);
    }      
  fwrite(&end_of_dcurves,sizeof(double),1,fp);
  fclose(fp);
  return(0);
}


/* Create native formats (without conversion of the internal type) */

short _mw_dcurves_create_native(fname,cvs,Type)

char  *fname;                        /* file name */
Dcurves cvs;
char  *Type;                         /* Type de format du fichier */

{
  if (strcmp(Type,"MW2_DCURVES") == 0)
    return(_mw_create_dcurves_mw2_dcurves(fname,cvs));
  
  return(-1);
}


/* Write file in different formats */
   
short _mw_create_dcurves(fname,cvs,Type)

char  *fname;                        /* file name */
Dcurves cvs;
char  *Type;                         /* Type de format du fichier */

{
  short ret;

  /* First, try native formats */
  ret = _mw_dcurves_create_native(fname,cvs,Type);
  if (ret == 0) return(0);

  /* If failed, try other formats with memory conversion */
  ret = _mw_create_etype_from_itype(fname,cvs,"dcurves",Type);
  if (ret == 0) return(0);

  /* Invalid Type should have been detected before, but we can arrive here because
     of a write failure (e.g. the output file name is write protected).
  */
  mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
}
   




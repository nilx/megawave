/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  fcurve_io.c
   
  Vers. 1.17
  (C) 1995-2002 Jacques Froment
  Input/Output functions for the fcurve & fcurves structure

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
#include <unistd.h>

#include "mw.h"

/* Marker to end a curve */
#define END_OF_CURVE 1e+37
/* Threshold above which the value is said to be END_OF_CURVE */
#define END_OF_CURVE_THRESHOLD 1e+36
/* End of curves */
#define END_OF_CURVES 1e+35
/* Threshold above which the value is said to be END_OF_CURVES */
#define END_OF_CURVES_THRESHOLD 1e+34

/* ----- I/O for Point_fcurve ----- */

Point_fcurve _mw_point_fcurve_load_native(char *fname, char *type)
{
     return(NULL);
}


/* ---- I/O for Fcurve ---- */

/* Max number of coordinates in the buffer (even number) */
#define MAXBUFSIZ 100000

/* Load fcurve from a file of MW2_FCURVE format */

Fcurve _mw_load_fcurve_mw2_fcurve(char *fname)
{
     FILE    *fp;
     Fcurve cv;
     Point_fcurve newcvc,oldcvc;
     struct stat buf;
     int fsize,buf_size,i;
     int readsize,remainsize;
     float vx,vy,*buffer;
     register float *ptr;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping = _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strcmp(ftype,"MW2_FCURVE") != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_fcurve_mw2_fcurve] File \"%s\" is not in the MW2_FCURVE format\n",fname);

  
     if ( (need_flipping==-1) ||
	  (!(fp = fopen(fname, "r"))) || (fstat(fileno(fp),&buf) != 0) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* Size of the file - size of the header, in bytes */
     fsize = buf.st_size-hsize; 
     remainsize = fsize / sizeof(float); /* Number of coordinates */
     if ((remainsize % 2) != 0)
     {
	  mwerror(ERROR, 0,"Error into the file \"%s\": not an even number of coordinates !\n",fname);
	  return(NULL);
     }
  
     buf_size=remainsize;
     if (hsize>buf_size) buf_size=hsize;
     if (buf_size > MAXBUFSIZ) buf_size=MAXBUFSIZ;
     if ( (!(buffer = (float *) malloc(buf_size * sizeof(float)))) ||
	  (!(cv = mw_new_fcurve())))
     {
	  mwerror(ERROR, 0,"Not enough memory to fcurve file \"%s\" !\n",fname);
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
	  if (fread(buffer,sizeof(float),readsize,fp) != readsize)
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
		    _mw_in_flip_float(ptr);
		    vx=*ptr;
		    ptr++;
		    _mw_in_flip_float(ptr);
		    vy=*ptr;
		    ptr++;
		    /* printf("(flip) i=%d (%f,%f)\n",i,vx,vy); */
	       }
	       else
	       {	
		    vx = *ptr++;
		    vy = *ptr++;
		    /* printf("(no flip) i=%d (%f,%f)\n",i,vx,vy); */
	       }
	       newcvc = mw_new_point_fcurve();
	       if (newcvc == NULL)
	       {
		    mw_delete_fcurve(cv);
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

Fcurve _mw_fcurve_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_FCURVE") == 0)
	  return((Fcurve)_mw_load_fcurve_mw2_fcurve(fname));

     return(NULL);
}


/* All available formats */

Fcurve _mw_load_fcurve(char *fname, char *type)
{ 
     Fcurve cv;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     cv = _mw_fcurve_load_native(fname,type);
     if (cv != NULL) return(cv);

     /* If failed, try other formats with memory conversion */
     cv = (Fcurve) _mw_load_etype_to_itype(fname,mtype,"fcurve",type);
     if (cv != NULL) return(cv);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Fcurve !\n",fname,type);
}



/* Write file in MW2_FCURVE format */  

short _mw_create_fcurve_mw2_fcurve(char *fname, Fcurve cv)
{
     FILE *fp;
     Point_fcurve pc;
     int n;
     float vx=0.0,vy=0.0;

     if (cv == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_fcurve_mw2_fcurve] Cannot create file: Fcurve structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_FCURVE",1.00);
     if (fp == NULL) return(-1);
  
     /* Allow to create empty fcurve
	if (cv->first == NULL)
	mwerror(INTERNAL,1,"[_mw_create_fcurve_mw2_fcurve] Fcurve has no point fcurve !\n");
     */

     for (pc=cv->first; pc; pc=pc->next)
     {
	  vx = pc->x;
	  vy = pc->y;
	  fwrite(&vx,sizeof(float),1,fp);
	  fwrite(&vy,sizeof(float),1,fp);
     }

     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_fcurve_create_native(char *fname, Fcurve cv, char *Type)
{
     if (strcmp(Type,"MW2_FCURVE") == 0)
	  return(_mw_create_fcurve_mw2_fcurve(fname,cv));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_fcurve(char *fname, Fcurve cv, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_fcurve_create_native(fname,cv,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,cv,"fcurve",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
}

/* ---- I/O for Fcurves ---- */

/* Load fcurves from a file of MW2_FCURVES format 
   Read operation is no more bufferized to be able to load huge fcurves.
*/

/* Load old format V1.00 */

Fcurves _mw_load_fcurves_mw2_fcurves_1_00(char *fname)
{
     FILE    *fp;
     Fcurves cvs;
     Fcurve newcv,oldcv;
     Point_fcurve newcvc,oldcvc;
     float x,y;
     float *px,*py;
     char new_fcurve;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping; 
     char header[BUFSIZ];
     float xsave;
     int eof;
     unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping = _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strcmp(ftype,"MW2_FCURVES") != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_fcurves_mw2_fcurves] File \"%s\" is not in the MW2_FCURVES format\n",fname);
  
  
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

     /*printf("hsize=%d version=%f\n",hsize,version);*/

     cvs = mw_new_fcurves();
     if (cvs == NULL) return(cvs);
     oldcv = newcv = NULL;

     new_fcurve = 1;
     xsave=END_OF_CURVE;
     eof=0;
     while (eof != 1)
     {
	  if (new_fcurve == 1)  /* Begin a new fcurve */
	  {
	       oldcv = newcv;
	       newcv = mw_new_fcurve();
	       if (newcv == NULL)
	       {
		    mw_delete_fcurves(cvs);
		    return(NULL);
	       }
	       if (cvs->first == NULL) cvs->first = newcv;
	       if (oldcv != NULL) oldcv->next = newcv;
	       newcv->previous = oldcv;
	       newcv->next = NULL;
	       newcv->first = NULL;
	       oldcvc = newcvc = NULL;
	  }
      
	  if (xsave >= END_OF_CURVE_THRESHOLD) /* Has to read x from file */
	  {
	       if (fread(&x,1,sizeof(float),fp) != sizeof(float))
	       {
		    mwerror(ERROR, 0,
			    "Error into the file \"%s\" : EOF encountered before EOC.\n",fname);
		    return(NULL);		
	       }
	       if (need_flipping==1) 
	       {
		    px=&x;
		    _mw_in_flip_float(px);
		    x=*px;
	       }
	  }	
      
	  else x=xsave; /* set previous value */
      
	  if (fread(&y,1,sizeof(float),fp) != sizeof(float))
	  {
	       mwerror(ERROR, 0,
		       "Error into the file \"%s\" : cannot read y coordinate\n",fname);
	       return(NULL);		    
	  }		
	  if (need_flipping==1) 
	  {
	       py=&y;
	       _mw_in_flip_float(py);
	       y=*py;
	  }
	  if (x>=END_OF_CURVE_THRESHOLD)
	  {
	       xsave=y;
	       if (y>=END_OF_CURVE_THRESHOLD)
	       {
		    if (fread(&y,1,sizeof(float),fp) != 0)
		    {
			 mwerror(ERROR, 0,
				 "Error into the file \"%s\" : no EOF after double EOC.\n",fname);
			 return(NULL);		    
		    }
		    else eof=1;
	       }
	       else new_fcurve=1;
	  }
	  else /* Record (x,y) */
	  {
	       xsave=END_OF_CURVE;
	       new_fcurve = 0;
	       newcvc = mw_new_point_fcurve();
	       if (newcvc == NULL)
	       {
		    mw_delete_fcurves(cvs);
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
   Changes from V1.00 : END_OF_CURVES introduced to be able
   to record empty curve.
*/

Fcurves _mw_load_fcurves_mw2_fcurves(char *fname)
{
     FILE    *fp;
     Fcurves cvs;
     Fcurve newcv,oldcv;
     Point_fcurve newcvc,oldcvc;
     float X[2];
     int n;
     float *px;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping; 
     char header[BUFSIZ];
     unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping = _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
  
     if (strcmp(ftype,"MW2_FCURVES") != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_fcurves_mw2_fcurves] File \"%s\" is not in the MW2_FCURVES format\n",fname);
  
     /*printf("[_mw_load_fcurves_mw2_fcurves] version=%f\n",version);*/
     if (version==1.0) return(_mw_load_fcurves_mw2_fcurves_1_00(fname));

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

     cvs = mw_new_fcurves();
     if (cvs == NULL) return(cvs);
     oldcv = newcv = NULL;

     n=0;
     while (1)
     {
	  if (fread(&X[n],1,sizeof(float),fp) != sizeof(float))
	  {
	       mwerror(ERROR, 0,
		       "Error into the file \"%s\" : EOF encountered before END_OF_CURVES.\n",fname);
	       return(NULL);		
	  }
	  if (need_flipping==1) 
	  {
	       px=&X[n];
	       _mw_in_flip_float(px);
	       X[n]=*px;
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
		    newcv = mw_new_fcurve();
		    if (newcv == NULL)
		    {
			 mw_delete_fcurves(cvs);
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
			 newcvc = mw_new_point_fcurve();
			 if (newcvc == NULL)
			 {
			      mw_delete_fcurves(cvs);
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

Fcurves _mw_fcurves_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_FCURVES") == 0)
	  return((Fcurves)_mw_load_fcurves_mw2_fcurves(fname));

     return(NULL);
}

/* All available formats */

Fcurves _mw_load_fcurves(char *fname, char *type)
{ 
     Fcurves cvs;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     cvs = _mw_fcurves_load_native(fname,type);
     if (cvs != NULL) return(cvs);

     /* If failed, try other formats with memory conversion */
     cvs = (Fcurves) _mw_load_etype_to_itype(fname,mtype,"fcurves",type);
     if (cvs != NULL) return(cvs);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Fcurves !\n",fname,type);
}

/* Write file in MW2_FCURVES format */  

short _mw_create_fcurves_mw2_fcurves(char *fname, Fcurves cvs)
{
     FILE *fp;
     Fcurve pl;
     Point_fcurve pc;
     int n;
     float vx=0.0,vy=0.0,end_of_fcurve=END_OF_CURVE;
     float end_of_fcurves = END_OF_CURVES;

     if (cvs == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_fcurves_mw2_fcurves] Cannot create file: Fcurves structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_FCURVES",1.01);
     if (fp == NULL) return(-1);
  
     if (cvs->first) fwrite(&end_of_fcurve,sizeof(float),1,fp);
     for (pl=cvs->first, n=1; pl; pl=pl->next, n++)
     {
	  for (pc=pl->first; pc; pc=pc->next)
	  {
	       if ((pc->x >= END_OF_CURVES_THRESHOLD) ||
		   (pc->y >= END_OF_CURVES_THRESHOLD))
		    mwerror(INTERNAL,1,"[_mw_create_fcurves_mw2_fcurves] Fcurve #%d has a point which coordinates (%f,%f) exceed float capacity\n",n,pc->x,pc->y);
	       vx = pc->x;
	       vy = pc->y;
	       fwrite(&vx,sizeof(float),1,fp);
	       fwrite(&vy,sizeof(float),1,fp);
	  }
	  if (pl->next) fwrite(&end_of_fcurve,sizeof(float),1,fp);
     }      
     fwrite(&end_of_fcurves,sizeof(float),1,fp);
     fclose(fp);
     return(0);
}


/* Create native formats (without conversion of the internal type) */

short _mw_fcurves_create_native(char *fname, Fcurves cvs, char *Type)
{
     if (strcmp(Type,"MW2_FCURVES") == 0)
	  return(_mw_create_fcurves_mw2_fcurves(fname,cvs));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_fcurves(char *fname, Fcurves cvs, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_fcurves_create_native(fname,cvs,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,cvs,"fcurves",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
}

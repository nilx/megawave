/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  curve_io.c
   
  Vers. 1.17
  (C) 1993-2002 Jacques Froment
  Input/Output functions for the curve & curves structure

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
#include <unistd.h>

#include "mw.h"

/* Those values of int type reserved */

/* End of curve (and end of curves with format V1.00) */
#define END_OF_CURVE -2147483647
/* End of curves */
#define END_OF_CURVES -2147483646

/* ----- I/O for Point_curve ----- */

Point_curve _mw_point_curve_load_native(char *fname, char *type)
{
     return(NULL);
}

/* ---- I/O for Curve ---- */

/* Max number of coordinates in the buffer (even number) */
#define MAXBUFSIZ 100000

/* Load curve from a file of MW2_CURVE format */

Curve _mw_load_curve_mw2_curve(char *fname)
{
     FILE    *fp;
     Curve cv;
     Point_curve newcvc,oldcvc;
     struct stat buf;
     int fsize,buf_size,i;
     int readsize,remainsize;
     int vx,vy,*buffer;
     register int *ptr;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping = _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strcmp(ftype,"MW2_CURVE") != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_curve_mw2_curve] File \"%s\" is not in the MW2_CURVE format\n",fname);
  
     if ( (need_flipping==-1) ||
	  (!(fp = fopen(fname, "r"))) || (fstat(fileno(fp),&buf) != 0) )
     {
	  mwerror(ERROR, 0,"File \"%s\" not found or unreadable\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* Size of the file - size of the header, in bytes */
     fsize = buf.st_size-hsize; 
     remainsize = (fsize / sizeof(int));
     if ((remainsize % 2) != 0)
     {
	  mwerror(ERROR, 0,"Error into the file \"%s\": not an even number of coordinates !\n",fname);
	  return(NULL);
     }

     buf_size=remainsize;
     if (hsize>buf_size) buf_size=hsize;
     if (buf_size > MAXBUFSIZ) buf_size=MAXBUFSIZ;

     if ( (!(buffer = (int *) malloc(buf_size * sizeof(int)))) ||
	  (!(cv=mw_new_curve())))
     {
	  mwerror(ERROR, 0,"Not enough memory to load curve file \"%s\" !\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     if (fread(buffer,1,hsize,fp) != hsize)
     {
	  mwerror(ERROR, 0,"Error while reading header of file \"%s\" !\n",fname);
	  fclose(fp);
	  return(NULL);
     }
  
     oldcvc = newcvc = NULL;

     while (remainsize > 0)
     {
	  readsize=remainsize;
	  if (readsize > buf_size) readsize=buf_size;

	  if (fread(buffer,sizeof(int),readsize,fp) != readsize)
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
		    vx = _mw_get_flip_b4(*ptr);
		    ptr++;
		    vy = _mw_get_flip_b4(*ptr);
		    ptr++;
		    /*printf("(flip) i=%d (%d,%d)\n",i,vx,vy);*/
	       }
	       else
	       {	
		    vx = *ptr++;
		    vy = *ptr++;
		    /*printf("(no flip) i=%d (%d,%d)\n",i,vx,vy);*/
	       }
	  
	       newcvc = mw_new_point_curve();
	       if (newcvc == NULL)
	       {
		    mw_delete_curve(cv);
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

Curve _mw_curve_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_CURVE") == 0)
	  return((Curve)_mw_load_curve_mw2_curve(fname));

     return(NULL);
}


/* All available formats */

Curve _mw_load_curve(char *fname, char *type)
{ 
     Curve cv;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     cv = _mw_curve_load_native(fname,type);
     if (cv != NULL) return(cv);

     /* If failed, try other formats with memory conversion */
     cv = (Curve) _mw_load_etype_to_itype(fname,mtype,"curve",type);
     if (cv != NULL) return(cv);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Curve !\n",fname,type);
}


/* Write file in MW2_CURVE format */  

short _mw_create_curve_mw2_curve(char *fname, Curve cv)
{
     FILE *fp;
     Point_curve pc;
     int n;

     if (cv == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_curve_mw2_curve] Cannot create file: Curve structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_CURVE",1.00);
     if (fp == NULL) return(-1);

     /* Now, allow to record empty curve
	if (cv->first == NULL)
	mwerror(INTERNAL,1,"[_mw_create_curve_mw2_curve] Curve has no point curve !\n");
     */

     for (pc=cv->first; pc; pc=pc->next)
     {
	  fwrite(&(pc->x),sizeof(int),1,fp);
	  fwrite(&(pc->y),sizeof(int),1,fp);
     }

     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_curve_create_native(char *fname, Curve cv, char *Type)
{
     if (strcmp(Type,"MW2_CURVE") == 0)
	  return(_mw_create_curve_mw2_curve(fname,cv));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_curve(char *fname, Curve cv, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_curve_create_native(fname,cv,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,cv,"curve",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
}


/* ---- I/O for Curves ---- */

/* Load curves from a file of MW2_CURVES format.
   Read operation is no more bufferized to be able to load huge curves.
*/

/* Load old format V1.00 */

static Curves _mw_load_curves_mw2_curves_1_00(char *fname)
{
     FILE    *fp;
     Curves cvs;
     Curve newcv,oldcv;
     Point_curve newcvc,oldcvc;
     int x,y;
     char new_curve;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     char header[BUFSIZ];
     int eof,xsave;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping = _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     printf("[_mw_load_curves_mw2_curves_1_00] version=%f\n",version);

     if (strcmp(ftype,"MW2_CURVES") != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_curves_mw2_curves] File \"%s\" is not in the MW2_CURVES format\n",fname);
  
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
  
     cvs = mw_new_curves();
     if (cvs == NULL) return(cvs);
     oldcv = newcv = NULL;

     new_curve = 1;
     xsave=END_OF_CURVE;
     eof=0;
     while (eof != 1)
     {
	  if (new_curve == 1)  /* Begin a new curve */
	  {
	       oldcv = newcv;
	       newcv = mw_new_curve();
	       if (newcv == NULL)
	       {
		    mw_delete_curves(cvs);
		    return(NULL);
	       }
	       if (cvs->first == NULL) cvs->first = newcv;
	       if (oldcv != NULL) oldcv->next = newcv;
	       newcv->previous = oldcv;
	       newcv->next = NULL;
	       newcv->first = NULL;
	       oldcvc = newcvc = NULL;
	  }
      
	  if (xsave == END_OF_CURVE) /* Has to read x from file */
	  {
	       if (fread(&x,1,sizeof(int),fp) != sizeof(int))
	       {
		    mwerror(ERROR, 0,
			    "Error into the file \"%s\" : EOF encountered before EOC.\n",fname);
		    return(NULL);		
	       }
	       if (need_flipping==1) x = _mw_get_flip_b4(x);    
	  }			
	  else x=xsave; /* set previous value */
      
	  if (fread(&y,1,sizeof(int),fp) != sizeof(int))
	  {
	       mwerror(ERROR, 0,
		       "Error into the file \"%s\" : cannot read y coordinate\n",fname);
	       return(NULL);		    
	  }		
	  if (need_flipping==1) y = _mw_get_flip_b4(y);
      
	  if (x==END_OF_CURVE)
	  {
	       xsave=y;
	       if (y==END_OF_CURVE)
	       {
		    if (fread(&y,1,sizeof(int),fp) != 0)
		    {
			 mwerror(ERROR, 0,
				 "Error into the file \"%s\" : no EOF after double EOC.\n",fname);
			 return(NULL);		    
		    }
		    else eof=1;
	       }
	       else new_curve=1;
	  }
	  else /* Record (x,y) */
	  {
	       xsave=END_OF_CURVE;
	       new_curve = 0;
	       newcvc = mw_new_point_curve();
	       if (newcvc == NULL)
	       {
		    mw_delete_curves(cvs);
		    return(NULL);
	       }
	       if (newcv->first == NULL) newcv->first = newcvc;
	       if (oldcvc != NULL) oldcvc->next = newcvc;
	       newcvc->previous = oldcvc;
	       newcvc->next = NULL;
	       newcvc->x = x; newcvc->y = y;
	       oldcvc = newcvc;
	  }
     } /* while (eof != 1) */
     fclose(fp);
     return(cvs);
}

/* Load current format V1.01 
   Changes from V1.00 : END_OF_CURVES introduced to be able
   to record empty curve.
*/

Curves _mw_load_curves_mw2_curves(char *fname)
{
     FILE    *fp;
     Curves cvs;
     Curve newcv,oldcv;
     Point_curve newcvc,oldcvc;
     int n,X[2];
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     char header[BUFSIZ];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping = _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
  
     if (strcmp(ftype,"MW2_CURVES") != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_curves_mw2_curves] File \"%s\" is not in the MW2_CURVES format\n",fname);
  
     /*printf("[_mw_load_curves_mw2_curves] version=%f\n",version);*/
     if (version==1.0) return(_mw_load_curves_mw2_curves_1_00(fname));

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
  
     cvs = mw_new_curves();
     if (cvs == NULL) return(cvs);
     oldcv = newcv = NULL;

     n=0;
     while (1)
     {
	  if (fread(&X[n],1,sizeof(int),fp) != sizeof(int))
	  {
	       mwerror(ERROR, 0,
		       "Error into the file \"%s\" : EOF encountered before END_OF_CURVES.\n",fname);
	       return(NULL);		
	  }
	  if (need_flipping==1) X[n] = _mw_get_flip_b4(X[n]);          
	  if (X[n]==END_OF_CURVES) 
	  {
	       fclose(fp);
	       return(cvs);
	  }
	  else
	       if (X[n]==END_OF_CURVE)
	       {
		    n=-1;
		    oldcv = newcv;
		    newcv = mw_new_curve();
		    if (newcv == NULL)
		    {
			 mw_delete_curves(cvs);
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
			 newcvc = mw_new_point_curve();
			 if (newcvc == NULL)
			 {
			      mw_delete_curves(cvs);
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

Curves _mw_curves_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_CURVES") == 0)
	  return((Curves)_mw_load_curves_mw2_curves(fname));

     return(NULL);
}

/* All available formats */

Curves _mw_load_curves(char *fname, char *type)
{ 
     Curves cvs;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     cvs = _mw_curves_load_native(fname,type);
     if (cvs != NULL) return(cvs);

     /* If failed, try other formats with memory conversion */
     cvs = (Curves) _mw_load_etype_to_itype(fname,mtype,"curves",type);
     if (cvs != NULL) return(cvs);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Curves !\n",fname,type);
}



/* Write file in MW2_CURVES format V 1.01 */  

short _mw_create_curves_mw2_curves(char *fname, Curves cvs)
{
     FILE *fp;
     Curve pl;
     Point_curve pc;
     int n;
     int end_of_curve = END_OF_CURVE;
     int end_of_curves = END_OF_CURVES;

     if (cvs == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_curves_mw2_curves] Cannot create file: Curves structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_CURVES",1.01);
     if (fp == NULL) return(-1);

     if (cvs->first) fwrite(&end_of_curve,sizeof(int),1,fp);
     for (pl=cvs->first, n=1; pl; pl=pl->next, n++)
     {
	  for (pc=pl->first; pc; pc=pc->next)
	  {
	       if ((pc->x == END_OF_CURVE) ||
		   (pc->y == END_OF_CURVE))
		    mwerror(INTERNAL,1,"[_mw_create_curves_mw2_curves] Curve #%d has a point which coordinates (%d,%d) has reserved value. Sorry !\n",n,pc->x,pc->y);	    
	       fwrite(&(pc->x),sizeof(int),1,fp);
	       fwrite(&(pc->y),sizeof(int),1,fp);
	  }
	  if (pl->next) fwrite(&end_of_curve,sizeof(int),1,fp);
     }      
     fwrite(&end_of_curves,sizeof(int),1,fp);
     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_curves_create_native(char *fname, Curves cvs, char *Type)
{
     if (strcmp(Type,"MW2_CURVES") == 0)
	  return(_mw_create_curves_mw2_curves(fname,cvs));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_curves(char *fname, Curves cvs, char *Type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_curves_create_native(fname,cvs,Type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,cvs,"curves",Type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname);  
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  list.c
   
  Vers. 1.4
  Author : Jacques Froment
  Input/Output functions for the flist, flists, dlist and dlists structures

  Main changes :
  v1.4 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
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

#include <sys/file.h>
#include <sys/types.h>

#include "mw.h"

/* ========== Flist / Flists ========== */

/* ---- I/O for Flist ---- */

/* Read one flist from the file fp */

Flist _mw_read_mw2_flist(char *fname, FILE *fp, int need_flipping)
{ 
     Flist lst;
     unsigned long * flip_float; /* buffer for macro _mw_in_flip_float */
     unsigned int i,n;

     lst = mw_new_flist();
     if (!lst) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }
  
     if (
	  (fread(&(lst->size),sizeof(int),1,fp) == 0) ||
	  (fread(&(lst->dim),sizeof(int),1,fp) == 0) ||
	  (fread(&(lst->data_size),sizeof(int),1,fp) == 0)
	  )
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  mw_delete_flist(lst);
	  return(NULL);
     }

     if (need_flipping == 1)
     {
	  _mw_in_flip_b4(lst->size);
	  _mw_in_flip_b4(lst->dim);
	  _mw_in_flip_b4(lst->data_size);
     }

     lst = mw_change_flist(lst,lst->size,lst->size,lst->dim);
     if (lst && (lst->data_size > 0))
	  lst->data = (char *) malloc(lst->data_size*sizeof(char));

     if ((!lst) || (((lst->data_size > 0) && !lst->data)))
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  mw_delete_flist(lst);
	  return(NULL);
     }
  
     /* Read the array values */
     n=lst->size * lst->dim;
     if (n > 0) fread(lst->values,sizeof(float),n,fp);

     if (need_flipping == 1) /* Strange that we need {} here to avoid run-time errors ! */
     {
	  for (i=0;i<n;i++) _mw_in_flip_float(&(lst->values[i]));
     }

     /* Read the array data */
     if (lst->data_size > 0) fread((char *)lst->data,sizeof(char),lst->data_size,fp);

     return(lst);
}

/* Load one flist from a file of MW2_FLIST format */

Flist _mw_load_mw2_flist(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Flist lst;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_FLIST",9) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_mw2_flist] File \"%s\" is not in the MW2_FLIST format\n",fname);
  
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
     lst = _mw_read_mw2_flist(fname,fp,need_flipping);

     fclose(fp);
     return(lst);
}

/* Native formats (without conversion of the internal type) */

Flist _mw_flist_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_FLIST") == 0)
	  return(_mw_load_mw2_flist(fname));

     return(NULL);
}

/* Load flist from file of different types */

Flist _mw_load_flist(char *fname, char *type)
{ 
     Flist lst;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     lst = _mw_flist_load_native(fname,type);
     if (lst != NULL) return(lst);

     /* If failed, try other formats with memory conversion */
     lst = (Flist) _mw_load_etype_to_itype(fname,mtype,"flist",type);
     if (lst != NULL) return(lst);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Curve !\n",fname,type);
}  


/* Write one flist in the file fp */  

int _mw_write_mw2_flist(FILE *fp, Flist lst)
{
     unsigned int n;

     if (fwrite(&(lst->size),sizeof(int),1,fp)!=1) return(1);
     if (fwrite(&(lst->dim),sizeof(int),1,fp)!=1) return(1);
     if (fwrite(&(lst->data_size),sizeof(int),1,fp)!=1) return(1);

     n=lst->size * lst->dim;
     if (n > 0) 
	  if (fwrite(lst->values,sizeof(float),n,fp)!=
	      n) return(1);

     if (lst->data_size > 0) 
	  if (fwrite((char *)lst->data,sizeof(char),lst->data_size,fp) != 
	      lst->data_size) return(1);

     return(0);
}

/* Write file in MW2_FLIST format */  

short _mw_create_mw2_flist(char *fname, Flist lst)
{
     FILE *fp;

     if (lst == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_mw2_flist] Cannot create file: Flist structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_FLIST",1.00);
     if (fp == NULL) return(-1);

     if (_mw_write_mw2_flist(fp,lst)!=0)
	  mwerror(ERROR,1,"Error while writing file %s !\n",fname);

     fclose(fp);
     return(0);
}


/* Create native formats (without conversion of the internal type) */

short _mw_flist_create_native(char *fname, Flist lst, char *type)
{
     if (strcmp(type,"MW2_FLIST") == 0)
	  return(_mw_create_mw2_flist(fname,lst));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_flist(char *fname, Flist lst, char *type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_flist_create_native(fname,lst,type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,lst,"flist",type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname); 
}


/* ---- I/O for Flists ---- */

/* Load Flists from a file of MW2_FLISTS format */

Flists _mw_load_mw2_flists(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Flists lsts;
     Flist lst;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int i,need_flipping;
     unsigned int size;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_FLISTS",10) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_mw2_flists] File \"%s\" is not in the MW2_FLISTS format\n",fname);

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

     lsts = mw_new_flists();
     if (lsts == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the cmt field */
     if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (cmt size)...\n",fname);
	  mw_delete_flists(lsts);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(size);
     if ((size > 0)&& (fread(lsts->cmt,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (cmt; cmt size=%d)...\n",fname,size);
	  mw_delete_flists(lsts);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the name field */
     if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name size)...\n",fname);
	  mw_delete_flists(lsts);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(size);
     if ((size > 0)&&(fread(lsts->name,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name)...\n",fname);
	  mw_delete_flists(lsts);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the other fields */
     if (
	  (fread(&(lsts->size),sizeof(int),1,fp) == 0) || 
	  (fread(&(lsts->data_size),sizeof(int),1,fp) == 0))
     {      
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  mw_delete_flists(lsts);
	  fclose(fp);
	  return(NULL);
     }
  
     if (need_flipping == 1)
     {
	  _mw_in_flip_b4(lsts->size);
	  _mw_in_flip_b4(lsts->data_size);
     }

     /* Allocate the lists */
     lsts = mw_change_flists(lsts,lsts->size,lsts->size);
     if (lsts && (lsts->data_size > 0))
	  lsts->data = (char *) malloc(lsts->data_size*sizeof(char));

     if ((!lsts) || (((lsts->data_size > 0) && !lsts->data)))
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  mw_delete_flists(lsts);
	  fclose(fp);
	  return(NULL);
     }
  
     /* Read the lists */
     for (i=0; i<lsts->size; i++)
     {
	  lst=_mw_read_mw2_flist(fname,fp,need_flipping);
	  if (!lst)
	  {
	       mw_delete_flists(lsts);
	       fclose(fp);
	       return(NULL);
	  }
	  lsts->list[i]=lst;
     }

     /* Read the array data */
     if (lsts->data_size > 0) 
	  fread((char *)lsts->data,sizeof(char),lsts->data_size,fp);

     fclose(fp);
     return(lsts);
}


/* Native formats (without conversion of the internal type) */

Flists _mw_flists_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_FLISTS") == 0)
	  return(_mw_load_mw2_flists(fname));

     return(NULL);
}

/* Load flists from file of different types */

Flists _mw_load_flists(char *fname, char *type)
{ 
     Flists lsts;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     lsts = _mw_flists_load_native(fname,type);
     if (lsts != NULL) return(lsts);

     /* If failed, try other formats with memory conversion */
     lsts = (Flists) _mw_load_etype_to_itype(fname,mtype,"flists",type);
     if (lsts != NULL) return(lsts);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Curve !\n",fname,type);
}  


/* Write file in MW2_FLISTS format */  

short _mw_create_mw2_flists(char *fname, Flists lsts)
{
     FILE *fp;
     unsigned int size;
     int i;

     if (lsts == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_mw2_flists] Cannot create file: flists structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_FLISTS",1.00);
     if (fp == NULL) return(-1);
  
     size = strlen(lsts->cmt);
     fwrite(&(size),sizeof(unsigned int),1,fp);  
     if (size > 0) fwrite(lsts->cmt,sizeof(char),size,fp);

     size = strlen(lsts->name);
     fwrite(&(size),sizeof(unsigned int),1,fp);  
     if (size > 0) fwrite(lsts->name,sizeof(char),size,fp);

     fwrite(&(lsts->size),sizeof(int),1,fp);
     fwrite(&(lsts->data_size),sizeof(int),1,fp);

     /* Write the lists */
     for (i=0; i<lsts->size; i++)
	  if (_mw_write_mw2_flist(fp,lsts->list[i])!=0)
	  {
	       mwerror(ERROR,1,"Error while writing file %s !\n",fname);
	       return(1);
	  }

     if (lsts->data_size > 0) 
	  if (fwrite((char *)lsts->data,sizeof(char),lsts->data_size,fp)!=
	      lsts->data_size) 
	  {
	       mwerror(ERROR,1,"Error while writing file %s !\n",fname);
	       return(1);
	  }

     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_flists_create_native(char *fname, Flists lsts, char *type)
{
     if (strcmp(type,"MW2_FLISTS") == 0)
	  return(_mw_create_mw2_flists(fname,lsts));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_flists(char *fname, Flists lsts, char *type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_flists_create_native(fname,lsts,type);
     if (ret == 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,lsts,"flists",type);
     if (ret == 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname); 
}


/* ========== Dlist / Dlists ========== */

/* ----- I/O for Dlist ---- */

/* Read one dlist from the file fp */

static Dlist _mw_read_mw2_dlist(char *fname, FILE *fp, int need_flipping)
{ 
     Dlist lst;
     unsigned long flip_double; /* buffer for macro _mw_in_flip_double */
     unsigned long *pdouble;
     unsigned int i,n;

     lst = mw_new_dlist();
     if (!lst) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }
  
     if (
	  (fread(&(lst->size),sizeof(int),1,fp) == 0) ||
	  (fread(&(lst->dim),sizeof(int),1,fp) == 0) ||
	  (fread(&(lst->data_size),sizeof(int),1,fp) == 0)
	  )
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  mw_delete_dlist(lst);
	  return(NULL);
     }

     if (need_flipping == 1)
     {
	  _mw_in_flip_b4(lst->size);
	  _mw_in_flip_b4(lst->dim);
	  _mw_in_flip_b4(lst->data_size);
     }

     lst = mw_change_dlist(lst,lst->size,lst->size,lst->dim);
     if (lst && (lst->data_size > 0))
	  lst->data = (char *) malloc(lst->data_size*sizeof(char));

     if ((!lst) || (((lst->data_size > 0) && !lst->data)))
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  mw_delete_dlist(lst);
	  return(NULL);
     }
  
     /* Read the array values */
     n=lst->size * lst->dim;
     if (n > 0) fread(lst->values,sizeof(double),n,fp);
     if (need_flipping == 1)
	  for (i=0;i<n;i++) 
	  {
	       pdouble= (unsigned long *) &(lst->values[i]);
	       _mw_in_flip_double(pdouble);
	       lst->values[i]=*((double *)pdouble);
	  }


     /* Read the array data */
     if (lst->data_size > 0) fread((char *)lst->data,sizeof(char),lst->data_size,fp);

     return(lst);
}

/* Load one dlist from a file of MW2_DLIST format */

Dlist _mw_load_mw2_dlist(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Dlist lst;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_DLIST",9) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_mw2_dlist] File \"%s\" is not in the MW2_DLIST format\n",fname);
  
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
     lst = _mw_read_mw2_dlist(fname,fp,need_flipping);

     fclose(fp);
     return(lst);
}

/* Native formats (without conversion of the internal type) */

Dlist _mw_dlist_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_DLIST") == 0)
	  return(_mw_load_mw2_dlist(fname));

     return(NULL);
}

/* Load dlist from file of different types */

Dlist _mw_load_dlist(char *fname, char *type)
{ 
     Dlist lst;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     lst = _mw_dlist_load_native(fname,type);
     if (lst != NULL) return(lst);

     /* If failed, try other formats with memory conversion */
     lst = (Dlist) _mw_load_etype_to_itype(fname,mtype,"dlist",type);
     if (lst != NULL) return(lst);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Curve !\n",fname,type);
}  



/* Write one dlist in the file fp */  

int _mw_write_mw2_dlist(FILE *fp, Dlist lst)
{
     unsigned int n;

     if (fwrite(&(lst->size),sizeof(int),1,fp)!=1) return(1);
     if (fwrite(&(lst->dim),sizeof(int),1,fp)!=1) return(1);
     if (fwrite(&(lst->data_size),sizeof(int),1,fp)!=1) return(1);

     n=lst->size * lst->dim;
     if (n > 0) 
	  if (fwrite(lst->values,sizeof(double),n,fp)!=n) return(1);
     if (lst->data_size > 0) 
	  if (fwrite((char *)lst->data,sizeof(char),lst->data_size,fp)!=
	      lst->data_size) return(1);

     return(0);
}

/* Write file in MW2_DLIST format */  

short _mw_create_mw2_dlist(char *fname, Dlist lst)
{
     FILE *fp;

     if (lst == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_mw2_dlist] Cannot create file: Dlist structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_DLIST",1.00);
     if (fp == NULL) return(-1);

     if (_mw_write_mw2_dlist(fp,lst)!=0)
	  mwerror(ERROR,1,"Error while writing file %s !\n",fname);    

     fclose(fp);
     return(0);
}


/* Create native formats (without conversion of the internal type) */

short _mw_dlist_create_native(char *fname, Dlist lst, char *type)
{
     if (strcmp(type,"MW2_DLIST") == 0)
	  return(_mw_create_mw2_dlist(fname,lst));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_dlist(char *fname, Dlist lst, char *type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_dlist_create_native(fname,lst,type);
     if (ret >= 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,lst,"dlist",type);
     if (ret >= 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname); 
}

/* ---- I/O for Dlists ---- */

/* Load Dlists from a file of MW2_DLISTS format */

Dlists _mw_load_mw2_dlists(char *fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Dlists lsts;
     Dlist lst;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int i,need_flipping;
     unsigned int size;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_DLISTS",10) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_mw2_dlists] File \"%s\" is not in the MW2_DLISTS format\n",fname);

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

     lsts = mw_new_dlists();
     if (lsts == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the cmt field */
     if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (cmt size)...\n",fname);
	  mw_delete_dlists(lsts);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(size);
     if ((size > 0)&& (fread(lsts->cmt,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (cmt; cmt size=%d)...\n",fname,size);
	  mw_delete_dlists(lsts);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the name field */
     if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name size)...\n",fname);
	  mw_delete_dlists(lsts);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(size);
     if ((size > 0)&&(fread(lsts->name,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name)...\n",fname);
	  mw_delete_dlists(lsts);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the other fields */
     if (
	  (fread(&(lsts->size),sizeof(int),1,fp) == 0) || 
	  (fread(&(lsts->data_size),sizeof(int),1,fp) == 0))
     {      
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  mw_delete_dlists(lsts);
	  fclose(fp);
	  return(NULL);
     }
  
     if (need_flipping == 1)
     {
	  _mw_in_flip_b4(lsts->size);
	  _mw_in_flip_b4(lsts->data_size);
     }

     /* Allocate the lists */
     lsts = mw_change_dlists(lsts,lsts->size,lsts->size);
     if (lsts && (lsts->data_size > 0))
	  lsts->data = (char *) malloc(lsts->data_size*sizeof(char));

     if ((!lsts) || (((lsts->data_size > 0) && !lsts->data)))
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  mw_delete_dlists(lsts);
	  fclose(fp);
	  return(NULL);
     }
  
     /* Read the lists */
     for (i=0; i<lsts->size; i++)
     {
	  lst=_mw_read_mw2_dlist(fname,fp,need_flipping);
	  if (!lst)
	  {
	       mw_delete_dlists(lsts);
	       fclose(fp);
	       return(NULL);
	  }
	  lsts->list[i]=lst;
     }

     /* Read the array data */
     if (lsts->data_size > 0) 
	  fread((char *)lsts->data,sizeof(char),lsts->data_size,fp);

     fclose(fp);
     return(lsts);
}


/* Native formats (without conversion of the internal type) */

Dlists _mw_dlists_load_native(char *fname, char *type)
{
     if (strcmp(type,"MW2_DLISTS") == 0)
	  return(_mw_load_mw2_dlists(fname));

     return(NULL);
}

/* Load dlists from file of different types */

Dlists _mw_load_dlists(char *fname, char *type)
{ 
     Dlists lsts;
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,type,mtype,&hsize,&version);

     /* First, try native formats */
     lsts = _mw_dlists_load_native(fname,type);
     if (lsts != NULL) return(lsts);

     /* If failed, try other formats with memory conversion */
     lsts = (Dlists) _mw_load_etype_to_itype(fname,mtype,"dlists",type);
     if (lsts != NULL) return(lsts);

     if (type[0]=='?')
	  mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",fname);
     else
	  mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Curve !\n",fname,type);
}  


/* Write file in MW2_DLISTS format */  

short _mw_create_mw2_dlists(char *fname, Dlists lsts)
{
     FILE *fp;
     unsigned int size;
     int i;

     if (lsts == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_mw2_dlists] Cannot create file: dlists structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_DLISTS",1.00);
     if (fp == NULL) return(-1);
  
     size = strlen(lsts->cmt);
     fwrite(&(size),sizeof(unsigned int),1,fp);  
     if (size > 0) fwrite(lsts->cmt,sizeof(char),size,fp);

     size = strlen(lsts->name);
     fwrite(&(size),sizeof(unsigned int),1,fp);  
     if (size > 0) fwrite(lsts->name,sizeof(char),size,fp);

     fwrite(&(lsts->size),sizeof(int),1,fp);
     fwrite(&(lsts->data_size),sizeof(int),1,fp);

     /* Write the lists */
     for (i=0; i<lsts->size; i++)
	  if (_mw_write_mw2_dlist(fp,lsts->list[i])!=0)
	  {
	       mwerror(ERROR,1,"Error while writing file %s !\n",fname);
	       return(1);
	  }

     if (lsts->data_size > 0) 
	  if (fwrite((char *)lsts->data,sizeof(char),lsts->data_size,fp)!=
	      lsts->data_size)
	  {
	       mwerror(ERROR,1,"Error while writing file %s !\n",fname);
	       return(1);
	  }

     fclose(fp);
     return(0);
}

/* Create native formats (without conversion of the internal type) */

short _mw_dlists_create_native(char *fname, Dlists lsts, char *type)
{
     if (strcmp(type,"MW2_DLISTS") == 0)
	  return(_mw_create_mw2_dlists(fname,lsts));
  
     return(-1);
}


/* Write file in different formats */
   
short _mw_create_dlists(char *fname, Dlists lsts, char *type)
{
     short ret;

     /* First, try native formats */
     ret = _mw_dlists_create_native(fname,lsts,type);
     if (ret >= 0) return(0);

     /* If failed, try other formats with memory conversion */
     ret = _mw_create_etype_from_itype(fname,lsts,"dlists",type);
     if (ret >= 0) return(0);

     /* Invalid Type should have been detected before, but we can arrive here because
	of a write failure (e.g. the output file name is write protected).
     */
     mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",fname); 
}

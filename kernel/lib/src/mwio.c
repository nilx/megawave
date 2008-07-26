/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mwio.c
   
  Vers. 2.17
  Author : Jacques Froment
  Input/Output functions as a link between External and Internal Types.

  Version history
  2.17 (JF, march 2006) : 
  - I/O function for wpack2d added
  - data directory path search changed (see _search_filename_in_dir())

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* FIXME : UNIX-centric and irregular */
/* FIXME : dirty fix*/
#define __USE_XOPEN
#include <sys/stat.h>
#undef __USE_XOPEN
#include <dirent.h>
#include <unistd.h>

#include "libmw.h"
#include "utils.h"
#include "cimage_io.h"
#include "fimage_io.h"
#include "ccimage_io.h"
#include "cfimage_io.h"
#include "cmovie_io.h"
#include "fmovie_io.h"
#include "ccmovie_io.h"
#include "cfmovie_io.h"
#include "curve_io.h"
#include "fcurve_io.h"
#include "dcurve_io.h"
#include "polygon_io.h"
#include "fpolygon_io.h"
#include "fsignal_io.h"
#include "wtrans1d_io.h"
#include "wtrans2d_io.h"
#include "wmax2d_io.h"
#include "wpack2d_io.h"
#include "module_io.h"
#include "mimage_io.h"
#include "cmimage_io.h"
#include "shape_io.h"
#include "rawdata_io.h"
#include "list_io.h"
#include "file_type.h"

#include "mwio.h"

/*===== Flip the image's buffer =====*/

void _mw_flip_image(register unsigned char *ptr,
		    short size, short dx, short dy, char flip)
{
     register unsigned short *ptr2;
     register unsigned long *ptr4;
     register long i;

     if (flip == TRUE)  /* Flip the image data (reverse byte order) */
     {
	  switch(size)
	  {
	  case 2 : /* Bytes 01 -> 10 */
	       if (sizeof(*ptr2) != size)
		    mwerror(FATAL, 0,
			    "Cannot flip %d-bytes data with this machine architecture\n",size);
	       for (i=0, ptr2 = (unsigned short *) ptr; 
		    i<dx*dy; i++,ptr2++)
		    _mw_in_flip_b2(*ptr2);
	       break;

	  case 4 : /* Bytes 0123 -> 3210 */
	       if (sizeof(*ptr4) != size)
		    mwerror(FATAL, 0,
			    "Cannot flip %d-bytes data with this machine architecture\n",size);
	       for (i=0, ptr4 = (unsigned long *) ptr;
		    i<dx*dy; i++,ptr4++)
		    _mw_in_flip_b4(*ptr4);
	       break;	  
	  
	  case 8 : /* Bytes 01234567 -> 76543210 */ 
	       /* Not implemented ! */
	  default:
	       mwerror(FATAL, 0,"Cannot flip %d-bytes data !\n",size);
	  }
     }
}

/*===== Write header for MW2 binary file types =====*/

FILE *_mw_write_header_file(char * fname, char * type, float IDvers)
{
     FILE *fp;
     unsigned short first2bytes;    /* The first 2 bytes */
     char HID[SIZE_OF_MW2_BIN_TYPE_ID+1];
     /*  Header ID for MW2 binary types, of the form e.g. /1.00/ when IDvers=1. 
      */ 
   
     if ((strlen(type) <= 4) || (strncmp(type,"MW2_",4) != 0))
	  mwerror(INTERNAL,1,"[_mw_write_header_file] Cannot create header for type %s\n",type);

     if (IDvers >= 10.0)
	  mwerror(INTERNAL,1,"[_mw_write_header_file] IDvers must be less than 10 (or change SIZE_OF_MW2_BIN_TYPE_ID)\n");

     /* Write first 4 bytes (MW2_) so that flipping can be checked */
     fp = fopen(fname, "w");
     if (fp == NULL) return(fp);
     /* On Intel processors, char order will be inversed */
     first2bytes=0x4D57; /* MW */
     fwrite(&first2bytes,2,1,fp);
     /* Normal char order on every processors */
     fwrite(type+2,strlen(type)-2,1,fp);

     /* write the header id */
     sprintf(HID,"/%.2f/",IDvers);
     fwrite(HID,strlen(HID),1,fp);
     return(fp);
}     

/*===== Default paths for seeking input files =====*/


/* Get the file status of <fname> : return
   0 if some errors occur (file probably isn't readable),
   1 if file is a regular file or a link,
   2 if file is a directory,
   3 if file is not readable,
   4 if file is of other type.
*/

static int _get_file_status(char * fname)
{
     struct stat statbuf;

     if (stat(fname,&statbuf) != 0) return(0);
     if ((statbuf.st_mode & S_IFDIR) == S_IFDIR) 
     {
	  mwerror(WARNING,1,"'%s' is a directory, trying another match...\n",fname);
	  return(2); /* directory */
     }
     if ((statbuf.st_mode & S_IRUSR) != S_IRUSR) 
     {
	  mwerror(WARNING,1,"no read permission on '%s', trying another match...\n",fname);
	  return(3); /* not readable */
     }

     if ((statbuf.st_mode & S_IFREG) == S_IFREG) return(1); /* regular */
     if ((statbuf.st_mode & S_IFLNK) == S_IFLNK) return(1); /* link */

     mwerror(WARNING,1,"'%s' is not a regular nor a link file, trying another match...\n",
	     fname);
     return(4);
}


/* Search fname in searchdir, recursively in each subdirectories.
   Return 1 if found and the found directory in founddir, 0 elsewhere.
*/

static int _search_filename_in_dir(char *fname, char *searchdir, char *founddir)
{
     char dname[BUFSIZ];
     DIR *D;
     struct dirent *dirbuf; 
     struct stat statbuf;
     char newdir[BUFSIZ];

     sprintf(dname,"%s/%s",searchdir,fname);

     if (_get_file_status(dname)==1)  /* searchdir/fname is found */
     {
	  strcpy(founddir,searchdir);
	  return(TRUE);
     }
     /* Not found : list the subdir in searchdir */
     if ((D=opendir(searchdir))==NULL) return(FALSE); /* cannot open searchdir */
  
     while ((dirbuf=readdir(D))!=NULL)
     {
	  if ((strcmp(dirbuf->d_name,".")==0) ||
	      strcmp(dirbuf->d_name,"..")==0) continue;
      
	  sprintf(newdir,"%s/%s",searchdir,dirbuf->d_name);	      
	  if ((stat(newdir,&statbuf) == 0) &&
	      ((statbuf.st_mode & S_IFDIR) == S_IFDIR))
	       /* dirbuf->d_name is a directory */
	       if (_search_filename_in_dir(fname,newdir,founddir)==TRUE)
		    return(TRUE);
     }
     closedir(D);
     return(FALSE);
}	     

/* Search fname in this order, <mwgroup> being the module's group :
   1) fname
   2) fname in $MY_MEGAWAVE2/data/<mwgroup>/ (including subdirectories)
   3) fname in all $MY_MEGAWAVE2/data/ (including subdirectories)
   4) fname in $MEGAWAVE2/data/<mwgroup> (including subdirectories)
   5) fname in all $MEGAWAVE2/data/ (including subdirectories)
*/

int _search_filename(char * fname)  /* Return 1 if found, 0 else */
{
     char *path,searchdir[BUFSIZ],founddir[BUFSIZ],newfname[BUFSIZ];

     *founddir = '\0';
     if (_get_file_status(fname)==1) /* fname is found */
	  return(TRUE);

     /* Not found : see if fname contains absolute pathname. 
	If yes, return not found.
     */
     if (fname[0]=='/') return(FALSE);

     /* Search in subdirs of $MY_MEGAWAVE2/data */
     if ((path = getenv("MY_MEGAWAVE2")) != NULL)
     {
	  sprintf(searchdir,"%s/data/%s",path,mwgroup);  
	  if (_search_filename_in_dir(fname,searchdir,founddir)==TRUE)
	  {
	       /* founddir/fname found ! */
	       sprintf(newfname,"%s/%s",founddir,fname);  	  
	       strcpy(fname,newfname);
	       return(TRUE);
	  }
	  sprintf(searchdir,"%s/data",path);  
	  if (_search_filename_in_dir(fname,searchdir,founddir)==TRUE)
	  {
	       /* founddir/fname found ! */
	       sprintf(newfname,"%s/%s",founddir,fname);  	  
	       strcpy(fname,newfname);
	       return(TRUE);
	  }
     }

     /* Search in subdirs of $MEGAWAVE2/data */
     if ((path = getenv("MEGAWAVE2")) != NULL)
     {
	  sprintf(searchdir,"%s/data/%s",path,mwgroup);  
	  if (_search_filename_in_dir(fname,searchdir,founddir)==TRUE)
	  {
	       /* founddir/fname found ! */
	       sprintf(newfname,"%s/%s",founddir,fname);  	  
	       strcpy(fname,newfname);
	       return(TRUE);
	  }
	  sprintf(searchdir,"%s/data",path);  
	  if (_search_filename_in_dir(fname,searchdir,founddir)==TRUE)
	  {
	       /* founddir/fname found ! */
	       sprintf(newfname,"%s/%s",founddir,fname);  	  
	       strcpy(fname,newfname);
	       return(TRUE);
	  }
      
     }
     return(FALSE);
}

static void search_filename(char * fname)  
{
     if (_search_filename(fname) != TRUE)
	  mwerror(FATAL, 0,"File \"%s\" not found in default path\n",fname);
}

/* Format in <out> the filename given in <in> to be set in the structure. */
static void format_filename(char * out, char * in)
{
     char *i;

     /* Remove the dirname if any */
     for (i=in+strlen(in)-1; (*i!='/')&&(i!=in); i--);
     if (i!=in) i++;
     strncpy(out,i,mw_namesize);
}

/*
  Search for the first string pattern <label> in the file pointed
  to by <fp>. 
  Return the current value of the file position, located at the next
  byte after the found pattern. If the pattern is not found, return
  a negative value.
*/

long _mw_find_pattern_in_file(FILE * fp, char * label)
{
     char buf[BUFSIZ];
     int l;
     long p;

     l=strlen(label);
     p=ftell(fp);
     if (p<0) return(p);

     while (1)
     {
	  if (fread(buf,l,1,fp)!=1) return(-1);
	  if (strncmp(label,buf,l)==0) break;
	  if (fseek(fp,1-l,SEEK_CUR)!=0) return(-1);
	  p++;
     }
     return(p+l);
}

/* Return 1 if the byte ordering on the architecture this code is running
   is little endian, or 0 if it is big endian.
   Example of little endian architecture : ix86
   Example of big endian architecture : Sun Sparc
*/

int _mw_byte_ordering_is_little_endian(void)
{
     int b;
     char *p;

     b = 1;
     p = (char *) &b;
     if (*p==1) return(1); else return(0);
}


/*========== I/O functions ==========*/

/*===== Internal type : Cimage =====*/

short _mwload_cimage(char * name, char type[], char comment[], Cimage * im)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *im = (Cimage) _mw_cimage_load_image(fname, type_in);
     if (*im == NULL) return(-1);  

     format_filename((*im)->name,fname);

     strcpy(comment_in,(*im)->cmt);
     _mw_make_type(type,type_in,"cimage");
     _mw_make_comment(comment,comment_in);
     return(0);
}

short _mwsave_cimage(char * name, char type[], char type_force[],
		     char comment[], Cimage im)
{
     _mw_choose_type(type,type_force,"cimage");
     if (im->cmt[0] == '?') sprintf(im->cmt,"%s(%s)",mwname,comment);

     if (_mw_cimage_create_image(name,im,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Fimage =====*/

short _mwload_fimage(char * name, char type[], char comment[],
		     Fimage * im)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */

     /* Load a new image */
     search_filename(fname);
     *im = (Fimage) _mw_fimage_load_image(fname, type_in);
     if (*im == NULL) return(-1);  

     format_filename((*im)->name,fname);

     strcpy(comment_in,(*im)->cmt);
     _mw_make_type(type,type_in,"fimage");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_fimage(char * name, char type[], char type_force[],
		     char comment[], Fimage im)
{

     _mw_choose_type(type,type_force,"fimage");
     if (im->cmt[0] == '?') sprintf(im->cmt,"%s(%s)",mwname,comment);

     if (_mw_fimage_create_image(name,im,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Cmovie =====*/

short _mwload_cmovie(char * name, char type[], char comment[], 
		     Cmovie * movie)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname); 

     *movie = (Cmovie) _mw_cmovie_load_movie(fname, type_in);
     if (*movie == NULL) return(-1);  

     format_filename((*movie)->name,fname);

     strcpy(comment_in,(*movie)->cmt);
     _mw_make_type(type,type_in,"cimage");
     _mw_make_comment(comment,comment_in);
     return(0);
}

short _mwsave_cmovie(char * name, char type[], char type_force[],
		     char comment[], Cmovie movie)
{
     _mw_choose_type(type,type_force,"cimage");
     if (movie->cmt[0] == '?') sprintf(movie->cmt,"%s(%s)",mwname,comment);

     if (_mw_cmovie_create_movie(name,movie,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Fmovie =====*/

short _mwload_fmovie(char * name, char type[], char comment[],
		     Fmovie * movie)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];
     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname); 

     *movie = (Fmovie) _mw_fmovie_load_movie(fname, type_in);
     if (*movie == NULL) return(-1);  

     format_filename((*movie)->name,fname);

     strcpy(comment_in,(*movie)->cmt);
     _mw_make_type(type,type_in,"fimage");
     _mw_make_comment(comment,comment_in);
     return(0);
}

short _mwsave_fmovie(char * name, char type[], char type_force[],
		     char comment[], Fmovie movie)
{
     _mw_choose_type(type,type_force,"fimage");
     if (movie->cmt[0] == '?') sprintf(movie->cmt,"%s(%s)",mwname,comment);

     if (_mw_fmovie_create_movie(name,movie,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Ccmovie =====*/

short _mwload_ccmovie(char * name, char type[], char comment[],
		      Ccmovie * movie)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname); 

     *movie = (Ccmovie) _mw_ccmovie_load_movie(fname, type_in);
     if (*movie == NULL) return(-1);  

     format_filename((*movie)->name,fname);

     strcpy(comment_in,(*movie)->cmt);
     _mw_make_type(type,type_in,"ccimage");
     _mw_make_comment(comment,comment_in);
     return(0);
}

short _mwsave_ccmovie(char * name, char type[], char type_force[],
		      char comment[], Ccmovie movie)
{
     /* ccimage and not ccmovie because a ccmovie is composed by images of
	ccimage type.
     */
     _mw_choose_type(type,type_force,"ccimage");

     if (movie->cmt[0] == '?') sprintf(movie->cmt,"%s(%s)",mwname,comment);

     if (_mw_ccmovie_create_movie(name,movie,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Cfmovie =====*/

short _mwload_cfmovie(char * name, char type[], char comment[],
		      Cfmovie * movie)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname); 

     *movie = (Cfmovie) _mw_cfmovie_load_movie(fname, type_in);
     if (*movie == NULL) return(-1);  

     format_filename((*movie)->name,fname);

     strcpy(comment_in,(*movie)->cmt);
     _mw_make_type(type,type_in,"cfimage");
     _mw_make_comment(comment,comment_in);
     return(0);
}

short _mwsave_cfmovie(char * name, char type[], char type_force[],
		      char comment[], Cfmovie movie)
{

     /* cfimage and not cfmovie because a cfmovie is composed by images of
	cfimage type.
     */
     _mw_choose_type(type,type_force,"cfimage");

     if (movie->cmt[0] == '?') sprintf(movie->cmt,"%s(%s)",mwname,comment);

     if (_mw_cfmovie_create_movie(name,movie,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Curve =====*/

short _mwload_curve(char * name, char type[], char comment[],
		    Curve * cv)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *cv = (Curve) _mw_load_curve(fname,type_in);
     if (*cv == NULL) return(-1);  

     _mw_make_type(type,type_in,"curve");

     return(0);
}

short _mwsave_curve(char * name, char type[], char type_force[], 
		    char comment[], Curve cv)
{

     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"curve");
     if (_mw_create_curve(name,cv,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Curves =====*/

short _mwload_curves(char * name, char type[], char comment[], 
		     Curves * cv)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *cv = (Curves) _mw_load_curves(fname,type_in);
     if (*cv == NULL) return(-1);  

     format_filename((*cv)->name,fname);

     strcpy(comment_in,(*cv)->cmt);
     _mw_make_type(type,type_in,"curves");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_curves(char * name, char type[], char type_force[],
		     char comment[], Curves cv)
{

     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"curves");
     if (cv->cmt[0] == '?') sprintf(cv->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_curves(name,cv,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Polygon =====*/

short _mwload_polygon(char * name, char type[], char comment[],
		      Polygon * poly)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *poly = (Polygon) _mw_load_polygon(fname,type_in);
     if (*poly == NULL) return(-1);  

     _mw_make_type(type,type_in,"polygon");

     return(0);
}

short _mwsave_polygon(char * name, char type[], char type_force[],
		      char comment[], Polygon poly)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"polygon");
     if (_mw_create_polygon(name,poly,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Polygons =====*/

short _mwload_polygons(char * name, char type[], char comment[],
		       Polygons * poly)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *poly = (Polygons) _mw_load_polygons(fname,type_in);
     if (*poly == NULL) return(-1);  

     format_filename((*poly)->name,fname);

     strcpy(comment_in,(*poly)->cmt);
     _mw_make_type(type,type_in,"polygons");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_polygons(char * name, char type[], char type_force[],
		       char comment[], Polygons poly)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"polygons");
     if (poly->cmt[0] == '?') sprintf(poly->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_polygons(name,poly,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Fcurve =====*/

short _mwload_fcurve(char * name, char type[], char comment[],
		     Fcurve * cv)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *cv = (Fcurve) _mw_load_fcurve(fname,type_in);
     if (*cv == NULL) return(-1);  

     _mw_make_type(type,type_in,"fcurve");

     return(0);
}

short _mwsave_fcurve(char * name, char type[], char type_force[],
		     char comment[], Fcurve cv)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"fcurve");
     if (_mw_create_fcurve(name,cv,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Fcurves =====*/

short _mwload_fcurves(char * name, char type[], char comment[],
		      Fcurves * cv)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *cv = (Fcurves) _mw_load_fcurves(fname,type_in);
     if (*cv == NULL) return(-1);  

     format_filename((*cv)->name,fname);

     strcpy(comment_in,(*cv)->cmt);
     _mw_make_type(type,type_in,"fcurves");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_fcurves(char * name, char type[], char type_force[],
		      char comment[], Fcurves cv)
{
     _mw_choose_type(type,type_force,"fcurves");
     if (cv->cmt[0] == '?') sprintf(cv->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_fcurves(name,cv,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Fpolygon =====*/

short _mwload_fpolygon(char * name, char type[], char comment[],
		       Fpolygon * poly)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *poly = (Fpolygon) _mw_load_fpolygon(fname,type_in);
     if (*poly == NULL) return(-1);  

     _mw_make_type(type,type_in,"fpolygon");

     return(0);
}

short _mwsave_fpolygon(char * name, char type[], char type_force[],
		       char comment[], Fpolygon poly)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"fpolygon");
     if (_mw_create_fpolygon(name,poly,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Fpolygons =====*/

short _mwload_fpolygons(char * name, char type[], char comment[],
			Fpolygons * poly)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *poly = (Fpolygons) _mw_load_fpolygons(fname,type_in);
     if (*poly == NULL) return(-1);  

     format_filename((*poly)->name,fname);

     strcpy(comment_in,(*poly)->cmt);
     _mw_make_type(type,type_in,"fpolygons");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_fpolygons(char * name, char type[], char type_force[],
			char comment[], Fpolygons poly)
{
     _mw_choose_type(type,type_force,"fpolygons");
     if (poly->cmt[0] == '?') sprintf(poly->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_fpolygons(name,poly,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Fsignal =====*/

short _mwload_fsignal(char * name, char type[], char comment[], 
		      Fsignal * signal)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *signal = (Fsignal) _mw_load_fsignal(fname,type_in,NULL);
     if (*signal == NULL) return(-1);

     format_filename((*signal)->name,fname);

     strcpy(comment_in,(*signal)->cmt);
     _mw_make_type(type,type_in,"fsignal");
     _mw_make_comment(comment,comment_in);

     return(0);
}


short _mwsave_fsignal(char * name, char type[], char type_force[],
		      char comment[], Fsignal signal)
{
     _mw_choose_type(type,type_force,"fsignal");
     if (signal->cmt[0] == '?') sprintf(signal->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_fsignal(name,signal,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Wtrans1d =====*/

short _mwload_wtrans1d(char * name, char type[], char comment[],
		       Wtrans1d * wtrans)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *wtrans = (Wtrans1d) _mw_wtrans1d_load_wtrans(fname, type_in);
     if (*wtrans == NULL) return(-1);  

     format_filename((*wtrans)->name,fname);

     strcpy(comment_in,(*wtrans)->cmt);
     _mw_make_type(type,type_in,"fsignal");
     _mw_make_comment(comment,comment_in);
     return(0);
}

short _mwsave_wtrans1d(char * name, char type[], char type_force[],
		       char comment[], Wtrans1d wtrans)
{
     _mw_choose_type(type,type_force,"fsignal");
     if (wtrans->cmt[0] == '?') sprintf(wtrans->cmt,"%s(%s)",mwname,comment);

     if (_mw_wtrans1d_create_wtrans(name,wtrans,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Wtrans2d =====*/

short _mwload_wtrans2d(char * name, char type[], char comment[],
		       Wtrans2d * wtrans)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *wtrans = (Wtrans2d) _mw_wtrans2d_load_wtrans(fname, type_in);
     if (*wtrans == NULL) return(-1);  

     format_filename((*wtrans)->name,fname);

     strcpy(comment_in,(*wtrans)->cmt);
     _mw_make_type(type,type_in,"fimage");
     _mw_make_comment(comment,comment_in);
     return(0);
}

short _mwsave_wtrans2d(char * name, char type[], char type_force[],
		       char comment[], Wtrans2d wtrans)
{
     _mw_choose_type(type,type_force,"fimage");
     if (wtrans->cmt[0] == '?') sprintf(wtrans->cmt,"%s(%s)",mwname,comment);

     if (_mw_wtrans2d_create_wtrans(name,wtrans,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Vchain_wmax =====*/

short _mwload_vchain_wmax(char * name, char type[], char comment[],
			  Vchain_wmax * vchain)
{
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;
     type = type;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *vchain = (Vchain_wmax) _mw_load_vchain_wmax(fname);
     if (*vchain == NULL) return(-1);  
     return(0);
}

short _mwsave_vchain_wmax(char * name, char type[], char type_force[],
			  char comment[], Vchain_wmax vchain)
{
     /* FIXME : unused parameter */
     comment = comment;

     if (type_force[0] != '?') strcpy(type,type_force);
     if (_mw_create_vchain_wmax(name,vchain) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Vchains_wmax (set of Vchain_wmax) =====*/

short _mwload_vchains_wmax(char * name, char type[], char comment[],
			   Vchains_wmax * vchains)
{
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     type = type;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *vchains = (Vchains_wmax) _mw_load_vchains_wmax(fname);
     if (*vchains == NULL) return(-1);  

     format_filename((*vchains)->name,fname);
     strcpy(comment,(*vchains)->cmt);

     return(0);
}

short _mwsave_vchains_wmax(char * name, char type[], char type_force[],
			   char comment[], Vchains_wmax vchains)
{
     /* FIXME : unused parameter */
     comment = comment;

     if (type_force[0] != '?') strcpy(type,type_force);
     if (vchains->cmt[0] == '?') sprintf(vchains->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_vchains_wmax(name,vchains) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Ccimage =====*/

short _mwload_ccimage(char * name, char type[], char comment[],
		      Ccimage * im)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *im = (Ccimage) _mw_ccimage_load_image(fname, type_in);
     if (*im == NULL) return(-1);  

     format_filename((*im)->name,fname);

     strcpy(comment_in,(*im)->cmt);
     _mw_make_type(type,type_in,"ccimage");
     _mw_make_comment(comment,comment_in);
     return(0);
}

short _mwsave_ccimage(char * name, char type[], char type_force[],
		      char comment[], Ccimage im)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"ccimage");
     if (im->cmt[0] == '?') sprintf(im->cmt,"%s(%s)",mwname,comment);

     if (_mw_ccimage_create_image(name,im,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Cfimage =====*/

short _mwload_cfimage(char * name, char type[], char comment[],
		      Cfimage * im)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */

     /* Load a new image */
     search_filename(fname);
     *im = (Cfimage) _mw_cfimage_load_image(fname, type_in);
     if (*im == NULL) return(-1);  

     format_filename((*im)->name,fname);

     strcpy(comment_in,(*im)->cmt);
     _mw_make_type(type,type_in,"cfimage");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_cfimage(char * name, char type[], char type_force[],
		      char comment[], Cfimage im)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"cfimage");
     if (im->cmt[0] == '?') sprintf(im->cmt,"%s(%s)",mwname,comment);

     if (_mw_cfimage_create_image(name,im,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Modules =====*/

#include "module.h"

short _mwload_modules(char * name, char type[], char comment[],
		      Modules * modules)
{
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     type = type;

     strcpy(fname,name);      /* Do Not Change the value of name */

     /* Load a new image */
     search_filename(fname);
     *modules = (Modules) _mw_load_modules(fname);
     if (*modules == NULL) return(-1);  

     format_filename((*modules)->name,fname);

     strcpy(comment_in,(*modules)->cmt);
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_modules(char * name, char type[], char type_force[],
		      char comment[], Modules modules)
{
     /* FIXME : unused parameter */
     comment = comment;

     if (type_force[0] != '?') strcpy(type,type_force);
     if (modules->cmt[0] == '?') sprintf(modules->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_modules(name,modules) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Morpho_line  =====*/

short _mwload_morpho_line(char * name, char type[], char comment[],
			  Morpho_line * ll)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *ll = (Morpho_line) _mw_load_morpho_line(fname,type_in);
     if (*ll == NULL) return(-1);  

     _mw_make_type(type,type_in,"morpho_line");

     return(0);
}

short _mwsave_morpho_line(char * name, char type[], char type_force[],
			  char comment[], Morpho_line ll)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"morpho_line");
     if (_mw_create_morpho_line(name,ll,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Fmorpho_line  =====*/

short _mwload_fmorpho_line(char * name, char type[], char comment[],
			   Fmorpho_line * fll)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *fll = (Fmorpho_line) _mw_load_fmorpho_line(fname,type_in);
     if (*fll == NULL) return(-1);  

     _mw_make_type(type,type_in,"fmorpho_line");

     return(0);
}

short _mwsave_fmorpho_line(char * name, char type[], char type_force[],
			   char comment[], Fmorpho_line fll)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"fmorpho_line");
     if (_mw_create_fmorpho_line(name,fll,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : morpho_set  =====*/

short _mwload_morpho_set(char * name, char type[], char comment[],
			 Morpho_set * morpho_set)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *morpho_set = (Morpho_set) _mw_load_morpho_set(fname,type_in);
     if (*morpho_set == NULL) return(-1);  

     _mw_make_type(type,type_in,"morpho_set");

     return(0);
}

short _mwsave_morpho_set(char * name, char type[], char type_force[],
			 char comment[], Morpho_set morpho_set)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"morpho_set");
     if (_mw_create_morpho_set(name,morpho_set,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : morpho_sets  =====*/

short _mwload_morpho_sets(char * name, char type[], char comment[],
			  Morpho_sets * morpho_sets)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *morpho_sets = (Morpho_sets) _mw_load_morpho_sets(fname,type_in);
     if (*morpho_sets == NULL) return(-1);  

     _mw_make_type(type,type_in,"morpho_sets");

     return(0);
}

short _mwsave_morpho_sets(char * name, char type[], char type_force[],
			  char comment[], Morpho_sets morpho_sets)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"morpho_sets");
     if (_mw_create_morpho_sets(name,morpho_sets,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Mimage  =====*/

short _mwload_mimage(char * name, char type[], char comment[],
		     Mimage * mimage)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *mimage = (Mimage) _mw_load_mimage(fname,type_in);
     if (*mimage == NULL) return(-1);  

     format_filename((*mimage)->name,fname);

     strcpy(comment_in,(*mimage)->cmt);
     _mw_make_type(type,type_in,"mimage");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_mimage(char * name, char type[], char type_force[],
		     char comment[], Mimage mimage)
{
     _mw_choose_type(type,type_force,"mimage");
     if (mimage->cmt[0] == '?') sprintf(mimage->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_mimage(name,mimage,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Cmorpho_line  =====*/

short _mwload_cmorpho_line(char * name, char type[], char comment[],
			   Cmorpho_line * ll)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *ll = (Cmorpho_line) _mw_load_cmorpho_line(fname,type_in);
     if (*ll == NULL) return(-1);  

     _mw_make_type(type,type_in,"cmorpho_line");

     return(0);
}

short _mwsave_cmorpho_line(char * name, char type[], char type_force[],
			   char comment[], Cmorpho_line ll)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"cmorpho_line");
     if (_mw_create_cmorpho_line(name,ll,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Cfmorpho_line  =====*/

short _mwload_cfmorpho_line(char * name, char type[], char comment[],
			    Cfmorpho_line * fll)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *fll = (Cfmorpho_line) _mw_load_cfmorpho_line(fname,type_in);
     if (*fll == NULL) return(-1);  

     _mw_make_type(type,type_in,"cfmorpho_line");

     return(0);
}

short _mwsave_cfmorpho_line(char * name, char type[], char type_force[],
			    char comment[], Cfmorpho_line fll)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"cfmorpho_line");
     if (_mw_create_cfmorpho_line(name,fll,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : cmorpho_set  =====*/

short _mwload_cmorpho_set(char * name, char type[], char comment[],
			  Cmorpho_set * cmorpho_set)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *cmorpho_set = (Cmorpho_set) _mw_load_cmorpho_set(fname,type_in);
     if (*cmorpho_set == NULL) return(-1);  

     _mw_make_type(type,type_in,"cmorpho_set");

     return(0);
}

short _mwsave_cmorpho_set(char * name, char type[], char type_force[],
			  char comment[], Cmorpho_set cmorpho_set)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"cmorpho_set");
     if (_mw_create_cmorpho_set(name,cmorpho_set,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : cmorpho_sets  =====*/

short _mwload_cmorpho_sets(char * name, char type[], char comment[],
			   Cmorpho_sets * cmorpho_sets)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *cmorpho_sets = (Cmorpho_sets) _mw_load_cmorpho_sets(fname,type_in);
     if (*cmorpho_sets == NULL) return(-1);  

     _mw_make_type(type,type_in,"cmorpho_sets");

     return(0);
}

short _mwsave_cmorpho_sets(char * name, char type[], char type_force[],
			   char comment[], Cmorpho_sets cmorpho_sets)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"cmorpho_sets");
     if (_mw_create_cmorpho_sets(name,cmorpho_sets,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Cmimage  =====*/

short _mwload_cmimage(char * name, char type[], char comment[],
		      Cmimage * cmimage)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *cmimage = (Cmimage) _mw_load_cmimage(fname,type_in);
     if (*cmimage == NULL) return(-1);  

     format_filename((*cmimage)->name,fname);

     strcpy(comment_in,(*cmimage)->cmt);
     _mw_make_type(type,type_in,"cmimage");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_cmimage(char * name, char type[], char type_force[],
		      char comment[], Cmimage cmimage)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"cmimage");
     if (cmimage->cmt[0] == '?') sprintf(cmimage->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_cmimage(name,cmimage,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Shape  =====*/

short _mwload_shape(char * name, char type[], char comment[],
		    Shape * shape)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *shape = (Shape) _mw_load_shape(fname,type_in);
     if (*shape == NULL) return(-1);  

     _mw_make_type(type,type_in,"shape");
     return(0);
}

short _mwsave_shape(char * name, char type[], char type_force[],
		    char comment[], Shape shape)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"shape");

     if (_mw_create_shape(name,shape,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Shapes  =====*/

short _mwload_shapes(char * name, char type[], char comment[],
		     Shapes * shapes)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *shapes = (Shapes) _mw_load_shapes(fname,type_in);
     if (*shapes == NULL) return(-1);  

     format_filename((*shapes)->name,fname);

     strcpy(comment_in,(*shapes)->cmt);
     _mw_make_type(type,type_in,"shapes");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_shapes(char * name, char type[], char type_force[],
		     char comment[], Shapes shapes)
{
     _mw_choose_type(type,type_force,"shapes");
     if (shapes->cmt[0] == '?') sprintf(shapes->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_shapes(name,shapes,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Dcurve =====*/

short _mwload_dcurve(char * name, char type[], char comment[],
		     Dcurve * cv)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *cv = (Dcurve) _mw_load_dcurve(fname,type_in);
     if (*cv == NULL) return(-1);  

     _mw_make_type(type,type_in,"dcurve");

     return(0);
}

short _mwsave_dcurve(char * name, char type[], char type_force[],
		     char comment[], Dcurve cv)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"dcurve");
     if (_mw_create_dcurve(name,cv,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Dcurves =====*/

short _mwload_dcurves(char * name, char type[], char comment[],
		      Dcurves * cv)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *cv = (Dcurves) _mw_load_dcurves(fname,type_in);
     if (*cv == NULL) return(-1);  

     format_filename((*cv)->name,fname);

     strcpy(comment_in,(*cv)->cmt);
     _mw_make_type(type,type_in,"dcurves");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_dcurves(char * name, char type[], char type_force[],
		      char comment[], Dcurves cv)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"dcurves");
     if (cv->cmt[0] == '?') sprintf(cv->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_dcurves(name,cv,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Rawdata =====*/

short _mwload_rawdata(char * name, char type[], char comment[],
		      Rawdata * rd)
{
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;
     type = type;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *rd = (Rawdata) _mw_load_rawdata(fname);
     if (*rd == NULL) return(-1);  
     return(0);
}

short _mwsave_rawdata(char * name, char type[], char type_force[],
		      char comment[], Rawdata rd)
{
     /* FIXME : unused parameter */
     comment = comment;
     type = type;
     type_force = type_force;

     if (_mw_create_rawdata(name,rd) >= 0)
	  return(0);
     else
	  return(-1);
}



/*===== Internal type : Flist =====*/

short _mwload_flist(char * name, char type[], char comment[],
		    Flist * lst)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *lst = (Flist) _mw_load_flist(fname,type_in);
     if (*lst == NULL) return(-1);  

     _mw_make_type(type,type_in,"flist");

     return(0);
}

short _mwsave_flist(char * name, char type[], char type_force[],
		    char comment[], Flist lst)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"flist");
     if (_mw_create_flist(name,lst,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Flists =====*/

short _mwload_flists(char * name, char type[], char comment[],
		     Flists * lsts)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *lsts = (Flists) _mw_load_flists(fname,type_in);
     if (*lsts == NULL) return(-1);  

     format_filename((*lsts)->name,fname);

     strcpy(comment_in,(*lsts)->cmt);
     _mw_make_type(type,type_in,"flists");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_flists(char * name, char type[], char type_force[],
		     char comment[], Flists lsts)
{
     _mw_choose_type(type,type_force,"flists");
     if (lsts->cmt[0] == '?') sprintf(lsts->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_flists(name,lsts,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Dlist =====*/

short _mwload_dlist(char * name, char type[], char comment[],
		    Dlist * lst)
{
     char type_in[mw_ftype_size];
     char fname[BUFSIZ];

     /* FIXME : unused parameter */
     comment = comment;

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *lst = (Dlist) _mw_load_dlist(fname,type_in);
     if (*lst == NULL) return(-1);  

     _mw_make_type(type,type_in,"dlist");

     return(0);
}

short _mwsave_dlist(char * name, char type[], char type_force[],
		    char comment[], Dlist lst)
{
     /* FIXME : unused parameter */
     comment = comment;

     _mw_choose_type(type,type_force,"dlist");
     if (_mw_create_dlist(name,lst,type) >= 0)
	  return(0);
     else
	  return(-1);
}

/*===== Internal type : Dlists =====*/

short _mwload_dlists(char * name, char type[], char comment[],
		     Dlists * lsts)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *lsts = (Dlists) _mw_load_dlists(fname,type_in);
     if (*lsts == NULL) return(-1);  

     format_filename((*lsts)->name,fname);

     strcpy(comment_in,(*lsts)->cmt);
     _mw_make_type(type,type_in,"dlists");
     _mw_make_comment(comment,comment_in);

     return(0);
}

short _mwsave_dlists(char * name, char type[], char type_force[],
		     char comment[], Dlists lsts)
{
     _mw_choose_type(type,type_force,"dlists");
     if (lsts->cmt[0] == '?') sprintf(lsts->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_dlists(name,lsts,type) >= 0)
	  return(0);
     else
	  return(-1);
}


/*===== Internal type : Wpack2d =====*/

short _mwload_wpack2d(char * name, char type[], char comment[],
		      Wpack2d * pack)
{
     char type_in[mw_ftype_size];
     char comment_in[BUFSIZ];
     char fname[BUFSIZ];

     strcpy(fname,name);      /* Do Not Change the value of name */
     search_filename(fname);
     *pack = (Wpack2d) _mw_load_wpack2d(fname, type_in);
     if (*pack == NULL) return(-1);  

     format_filename((*pack)->name,fname);

     strcpy(comment_in,(*pack)->cmt);
     _mw_make_type(type,type_in,"wpack2d");
     _mw_make_comment(comment,comment_in);
     return(0);
}

short _mwsave_wpack2d(char * name, char type[], char type_force[],
		      char comment[], Wpack2d pack)
{
     _mw_choose_type(type,type_force,"wpack2d");
     if (pack->cmt[0] == '?') sprintf(pack->cmt,"%s(%s)",mwname,comment);

     if (_mw_create_wpack2d(name,pack,type) >= 0)
	  return(0);
     else
	  return(-1);
}

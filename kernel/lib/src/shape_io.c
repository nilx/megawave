/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  shape_io.c
   
  Vers. 1.6
  Authors : Pascal Monasse, Frederic Guichard, Jacques Froment.
  Input/output functions for the 
  Shape
  Shapes
  structures

  Main changes :
  v1.5 : Bug corrected on _mw_load_mw2_shapes() (Luis Garrido, Coloma Ballester)
  v1.1 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
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

#include "libmw.h"
#include "utils.h"
#include "shape.h"
#include "curve.h"
#include "file_type.h"
#include "type_conv.h"
#include "mwio.h"

#include "shape_io.h"

/* ---- I/O for shape ---- */

/* Read one shape from the file fp */
/* Read old format V1.00 */

static Shape _mw_read_mw2_shape_1_00(char * fname, FILE * fp,
				     int need_flipping, Shape sh,
				     int * iparent)
{ 
     Point_curve newpc,oldpc;
     unsigned int npc;
     int i;
     Curve cv;

     if (sh == NULL) sh = mw_new_shape();
     if (sh == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }

     if (
	  (fread(&(sh->inferior_type),sizeof(char),1,fp) == 0) || 
	  (fread(&(sh->value),sizeof(float),1,fp) == 0) || 
	  (fread(&(sh->open),sizeof(char),1,fp) == 0) ||
	  (fread(&(sh->area),sizeof(int),1,fp) == 0) ||
	  (fread(&i,sizeof(int),1,fp) == 0) ||
	  (fread(&npc,sizeof(unsigned int),1,fp) == 0)
	  )
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  mw_delete_shape(sh);
	  return(NULL);
     }
  
     if (need_flipping == 1)
     {
	  _mw_in_flip_float( (sh->value) );
	  _mw_in_flip_b4(sh->area);
	  _mw_in_flip_b4(i);
	  _mw_in_flip_b4(npc);
     }

     if (iparent != NULL) * iparent = i;

     if ((sh->inferior_type!=0)&&(sh->inferior_type!=1))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" : inconsistent value for the field inferior_type (%d)\n",fname,(int) sh->inferior_type);
	  mw_delete_shape(sh);
	  return(NULL);
     }
     if ((sh->open!=0)&&(sh->open!=1))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" : inconsistent value for the field open (%d)\n",fname,(int) sh->open);
	  mw_delete_shape(sh);
	  return(NULL);
     }
     if (sh->area<=0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" : inconsistent value for the field area (%d)\n",fname,sh->area);
	  mw_delete_shape(sh);
	  return(NULL);
     }

     /* Read the curve, if any */
     if (npc == 0) return(sh);
  
     cv = mw_new_curve();
     if (cv == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  mw_delete_shape(sh);
	  return(NULL);
     }
     oldpc = newpc = NULL;
     /* FIXME: wrong types, dirty temporary fix */
     for (i = 1; i <= (int) npc; i++)
     {
	  newpc = mw_new_point_curve();
	  if (newpc == NULL)
	  {
	       mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	       mw_delete_shape(sh);
	       return(NULL);
	  }
	  if (cv->first == NULL) cv->first = newpc;
	  if (oldpc != NULL) oldpc->next = newpc;
	  newpc->previous = oldpc;
	  newpc->next = NULL;
	  if (
	       (fread(&(newpc->x),sizeof(int),1,fp) == 0) || 
	       (fread(&(newpc->y),sizeof(int),1,fp) == 0) 
	       )
	  {
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       mw_delete_shape(sh);
	       return(NULL);
	  }
	  if (need_flipping == 1)
	  {
	       _mw_in_flip_b4(newpc->x);
	       _mw_in_flip_b4(newpc->y);
	  }
	  oldpc = newpc;
     }

     sh->boundary = (Flist) mw_conv_internal_type(cv,"Curve","Flist");
     mw_delete_curve(cv);  
     return(sh);
}

Flist _mw_read_mw2_flist(char *,FILE *,int);

/* Read one shape from the file fp */

static Shape _mw_read_mw2_shape(char * fname, FILE * fp,
				int need_flipping, Shape sh, int * iparent)
{ 
     int i;
     char bound;
  
     if (sh == NULL) sh = mw_new_shape();
     if (sh == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  return(NULL);
     }

     if (
	  (fread(&(sh->inferior_type),sizeof(char),1,fp) == 0) || 
	  (fread(&(sh->value),sizeof(float),1,fp) == 0) || 
	  (fread(&(sh->open),sizeof(char),1,fp) == 0) ||
	  (fread(&(sh->area),sizeof(int),1,fp) == 0) ||
	  (fread(&i,sizeof(int),1,fp) == 0) ||
	  (fread(&bound,sizeof(char),1,fp) == 0) ||
	  (fread(&(sh->data_size),sizeof(int),1,fp) == 0)
	  )
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  mw_delete_shape(sh);
	  return(NULL);
     }

     if (need_flipping == 1)
     {
	  _mw_in_flip_float( (sh->value) );
	  _mw_in_flip_b4(sh->area);
	  _mw_in_flip_b4(i);
	  _mw_in_flip_b4(sh->data_size);
     }

     if (iparent != NULL) *iparent = i;

     if ((sh->inferior_type!=0)&&(sh->inferior_type!=1))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" : inconsistent value for the field inferior_type (%d)\n",fname,(int) sh->inferior_type);
	  mw_delete_shape(sh);
	  return(NULL);
     }
     if ((sh->open!=0)&&(sh->open!=1))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" : inconsistent value for the field open (%d)\n",fname,(int) sh->open);
	  mw_delete_shape(sh);
	  return(NULL);
     }
     if (sh->area<=0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" : inconsistent value for the field area (%d)\n",fname,sh->area);
	  mw_delete_shape(sh);
	  return(NULL);
     }

     /* Read the boundary, if any */
     if (bound == 1)  
	  sh->boundary = _mw_read_mw2_flist(fname,fp,need_flipping);
     else sh->boundary = NULL; 

     /* Read the array data */
     if (sh->data_size > 0) 
	  fread((char *)sh->data,sizeof(char),sh->data_size,fp);
     else sh->data = NULL;

     return(sh);
}


/* Load one shape from a file of MW2_SHAPE format */

Shape _mw_load_mw2_shape(char * fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Shape sh;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_SHAPE",9) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_mw2_shape] File \"%s\" is not in the MW2_SHAPE format\n",fname);
  
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

     /*printf("[_mw_load_mw2_shape] version=%f\n",version);*/
     if (version==1.0) 
	  sh = _mw_read_mw2_shape_1_00(fname,fp,need_flipping,NULL,NULL);
     else 
	  sh = _mw_read_mw2_shape(fname,fp,need_flipping,NULL,NULL);

     fclose(fp);
     return(sh);
}


/* Load shape from file of different types */

Shape _mw_load_shape(char * fname, char * Type)
{
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,Type,mtype,&hsize,&version);

     /* Native format */
     if (strcmp(Type,"MW2_SHAPE") == 0)
	  return(_mw_load_mw2_shape(fname));

     /* Other formats */

     mwerror(INTERNAL, 1,"[_mw_load_shape] Invalid external type \"%s\" for the file \"%s\"\n",Type,fname);

     return(NULL);
}  

void _mw_write_mw2_flist(FILE *, Flist);

/* Write one shape in the file fp */  

void _mw_write_mw2_shape(FILE * fp, Shape sh, int iparent)
{
     char bound; /* 1 if boundary defined (non NULL), 0 elsewhere */

     fwrite(&(sh->inferior_type),sizeof(char),1,fp);
     fwrite(&(sh->value),sizeof(float),1,fp);
     fwrite(&(sh->open),sizeof(char),1,fp);
     fwrite(&(sh->area),sizeof(int),1,fp);
     fwrite(&iparent,sizeof(int),1,fp);
     if (sh->boundary) bound=1; else bound=0;
     fwrite(&bound,sizeof(char),1,fp);  
     fwrite(&(sh->data_size),sizeof(int),1,fp);  

     /* Record the boundary, now a list rather than a curve */
     if (sh->boundary) _mw_write_mw2_flist(fp,sh->boundary);

     /* Record data field */
     if (sh->data_size > 0) 
	  fwrite((char *)sh->data,sizeof(char),sh->data_size,fp);
}

/* Write file in MW2_SHAPE format */  

short _mw_create_mw2_shape(char * fname, Shape sh)
{
     FILE *fp;

     if (sh == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_mw2_shape] Cannot create file: shape structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_SHAPE",1.01);
     if (fp == NULL) return(-1);

     _mw_write_mw2_shape(fp,sh,0);

     fclose(fp);
     return(0);
}

/* Write file in different formats */
   
short _mw_create_shape(char * fname, Shape sh, char * Type)
{
     /* Native format */
     if (strcmp(Type,"MW2_SHAPE") == 0)
	  return(_mw_create_mw2_shape(fname,sh));
  
     /* Other formats */
     return(-1);
}


/* ---- I/O for Shapes ---- */

/* Load Shapes from a file of MW2_SHAPE format */
/* Old version 1.00 */

Shapes _mw_load_mw2_shapes_1_00(char * fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Shapes shapes;
     Shape sh;
     unsigned int size;
     int i,IDs,absolute,iparent;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_SHAPES",10) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_mw2_shapes] File \"%s\" is not in the MW2_SHAPES format\n",fname);


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

     shapes = mw_new_shapes();
     if (shapes == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the cmt field */
     if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (cmt size)...\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(size);
     if ((size > 0)&& (fread(shapes->cmt,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (cmt; cmt size=%d)...\n",fname,size);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the name field */
     if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name size)...\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(size);
     if ((size > 0)&&(fread(shapes->name,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name)...\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the other fields */
     if (
	  (fread(&(shapes->nrow),sizeof(int),1,fp) == 0) || 
	  (fread(&(shapes->ncol),sizeof(int),1,fp) == 0) || 
	  (fread(&(shapes->nb_shapes),sizeof(int),1,fp) == 0))
     {      
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }
  
     if (need_flipping == 1)
     {
	  _mw_in_flip_b4(shapes->nrow);
	  _mw_in_flip_b4(shapes->ncol);
	  _mw_in_flip_b4(shapes->nb_shapes);
     }

     /* Alloc memory for the set of shapes */
     shapes->the_shapes = (Shape) malloc(shapes->nb_shapes*sizeof(struct shape));
     if (shapes->the_shapes==NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }

     /* Alloc memory for smallest_shape */
     shapes->smallest_shape = (Shape *) 
	  malloc(shapes->nrow*shapes->ncol*sizeof(Shape));
     if (shapes->smallest_shape==NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }

     /* read info for smallest_shape */
     absolute=0;
     for (i=(shapes->ncol*shapes->nrow)-1; i>=0; i--, absolute++)
     {
	  if(fread(&(IDs),sizeof(int),1,fp) == 0)
	  {      
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       mw_delete_shapes(shapes);
	       fclose(fp);
	       return(NULL);
	  }
	  if (need_flipping == 1) _mw_in_flip_b4(IDs);
	  shapes->smallest_shape[absolute] = &shapes->the_shapes[IDs];
     }

     /* read the shapes */
     sh = shapes->the_shapes;
     for (i=shapes->nb_shapes-1; i>=0; i--, sh++)
     {
	  if (_mw_read_mw2_shape_1_00(fname,fp,need_flipping,sh,&iparent) == NULL)
	  {      
	       mw_delete_shapes(shapes);
	       fclose(fp);
	       return(NULL);
	  }
	  sh->parent = &shapes->the_shapes[iparent];
     }

     shapes->the_shapes[0].parent = NULL;
     /* Correct the fields child and next_sibling of each shape */
     sh = &shapes->the_shapes[shapes->nb_shapes-1];
     for(i = shapes->nb_shapes-1; i > 0; i--, sh--) 
     {
	  sh->next_sibling = sh->parent->child;
	  sh->parent->child = sh;
     }
  
     fclose(fp);
     return(shapes);
}

/* Load Shapes from a file of MW2_SHAPE format */

Shapes _mw_load_mw2_shapes(char * fname)
{
     FILE    *fp;
     char header[BUFSIZ];
     Shapes shapes;
     Shape sh;
     unsigned int size;
     int i,IDs,absolute,iparent;
     char ftype[mw_ftype_size],mtype[mw_mtype_size];
     int need_flipping;
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     need_flipping =  _mw_get_file_type(fname,ftype,mtype,&hsize,&version)-1;
     if (strncmp(ftype,"MW2_SHAPES",10) != 0)
	  mwerror(INTERNAL, 0,"[_mw_load_mw2_shapes] File \"%s\" is not in the MW2_SHAPES format\n",fname);

     if (version==1.00) return(_mw_load_mw2_shapes_1_00(fname));

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

     shapes = mw_new_shapes();
     if (shapes == NULL) 
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the cmt field */
     if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (cmt size)...\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(size);
     if ((size > 0)&& (fread(shapes->cmt,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (cmt; cmt size=%d)...\n",fname,size);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the name field */
     if (fread(&(size),sizeof(unsigned int),1,fp) == 0)
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name size)...\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }
     if (need_flipping == 1) _mw_in_flip_b4(size);
     if ((size > 0)&&(fread(shapes->name,sizeof(char),size,fp) == 0))
     {
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (name)...\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }

     /* Read the other fields */
     if (
	  (fread(&(shapes->nrow),sizeof(int),1,fp) == 0) || 
	  (fread(&(shapes->ncol),sizeof(int),1,fp) == 0) || 
	  (fread(&(shapes->interpolation),sizeof(int),1,fp) == 0) || 
	  (fread(&(shapes->data_size),sizeof(int),1,fp) == 0) || 
	  (fread(&(shapes->nb_shapes),sizeof(int),1,fp) == 0))
     {      
	  mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }
  
     if (need_flipping == 1)
     {
	  _mw_in_flip_b4(shapes->nrow);
	  _mw_in_flip_b4(shapes->ncol);
	  _mw_in_flip_b4(shapes->interpolation);
	  _mw_in_flip_b4(shapes->data_size);
	  _mw_in_flip_b4(shapes->nb_shapes);
     }

     /* Alloc memory for the set of shapes */
     shapes->the_shapes = (Shape) malloc(shapes->nb_shapes*sizeof(struct shape));
     if (shapes->the_shapes==NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }

     /* Alloc memory for smallest_shape */
     shapes->smallest_shape = (Shape *) 
	  malloc(shapes->nrow*shapes->ncol*sizeof(Shape));
     if (shapes->smallest_shape==NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to read file \"%s\"\n",fname);
	  mw_delete_shapes(shapes);
	  fclose(fp);
	  return(NULL);
     }

     /* read info for smallest_shape */
     absolute=0;
     for (i=(shapes->ncol*shapes->nrow)-1; i>=0; i--, absolute++)
     {
	  if(fread(&(IDs),sizeof(int),1,fp) == 0)
	  {      
	       mwerror(ERROR, 0,"Error while reading file \"%s\"...\n",fname);
	       mw_delete_shapes(shapes);
	       fclose(fp);
	       return(NULL);
	  }
	  if (need_flipping == 1) _mw_in_flip_b4(IDs);
	  shapes->smallest_shape[absolute] = &shapes->the_shapes[IDs];
     }

     /* read the shapes */
     sh = shapes->the_shapes;
     for (i=shapes->nb_shapes-1; i>=0; i--, sh++)
     {
	  if (_mw_read_mw2_shape(fname,fp,need_flipping,sh,&iparent) == NULL)
	  {      
	       mw_delete_shapes(shapes);
	       fclose(fp);
	       return(NULL);
	  }
	  sh->parent = &shapes->the_shapes[iparent];
     }

     shapes->the_shapes[0].parent = NULL;
 
     /* Correct the fields child and next_sibling of each shape */

  
     for(i = shapes->nb_shapes - 1; i >= 0; i--)  
	  /* Thanks to Luis Garrido and Coloma Ballester for this addition */
     {
	  shapes->the_shapes[i].child = NULL;
	  shapes->the_shapes[i].next_sibling = NULL;
	  shapes->the_shapes[i].pixels = NULL;
     }

     sh = &shapes->the_shapes[shapes->nb_shapes-1];
     for(i = shapes->nb_shapes-1; i > 0; i--, sh--) 
     {
	  sh->next_sibling = sh->parent->child;
	  sh->parent->child = sh;
     }

     /* Read the array data */
     if (shapes->data_size > 0) 
	  fread((char *)shapes->data,sizeof(char),shapes->data_size,fp);
  
     fclose(fp);

     return(shapes);
}


/* Load shapes from file of different types */

Shapes _mw_load_shapes(char * fname, char * Type)
{
     char mtype[mw_mtype_size];
     int hsize;  /* Size of the header, in bytes */
     float version;/* Version number of the file format */

     _mw_get_file_type(fname,Type,mtype,&hsize,&version);

     /* Native format */
     if (strcmp(Type,"MW2_SHAPES") == 0)
	  return(_mw_load_mw2_shapes(fname));

     /* Other formats */

     mwerror(INTERNAL, 1,"[_mw_load_shapes] Invalid external type \"%s\" for the file \"%s\"\n",Type,fname);
  
     return(NULL);
}  


/* Write file in MW2_SHAPES format */  

short _mw_create_mw2_shapes(char * fname, Shapes shs)
{
     FILE *fp;
     unsigned int size;
     int i,*tabIDs,new_nb_shapes,absolute;
     Shape sh;
     long diff;

     if (shs == NULL)
	  mwerror(INTERNAL,1,"[_mw_create_mw2_shapes] Cannot create file: shapes structure is NULL\n");

     fp=_mw_write_header_file(fname,"MW2_SHAPES",1.01);
     if (fp == NULL) return(-1);

     size = strlen(shs->cmt);
     fwrite(&(size),sizeof(unsigned int),1,fp);  
     if (size > 0) fwrite(shs->cmt,sizeof(char),size,fp);

     size = strlen(shs->name);
     fwrite(&(size),sizeof(unsigned int),1,fp);  
     if (size > 0) fwrite(shs->name,sizeof(char),size,fp);

     fwrite(&(shs->nrow),sizeof(int),1,fp);
     fwrite(&(shs->ncol),sizeof(int),1,fp);
     fwrite(&(shs->interpolation),sizeof(int),1,fp);
     fwrite(&(shs->data_size),sizeof(int),1,fp);    

     tabIDs = (int*)malloc(sizeof(int) * shs->nb_shapes);
     if(tabIDs == NULL)
     {
	  mwerror(ERROR, 0,"Not enough memory to write file \"%s\"...\n",fname);
	  fclose(fp);
	  return(-1);
     }
     for(i = new_nb_shapes = 0; i < shs->nb_shapes; i++)
	  if(! shs->the_shapes[i].removed)
	       tabIDs[i] = new_nb_shapes++;
  
     fwrite(&new_nb_shapes,sizeof(int),1,fp);

     /* write info for smallest_shape */
     absolute=0;
     for (i=(shs->ncol*shs->nrow)-1; i>=0; i--, absolute++)
     {
	  sh=shs->smallest_shape[absolute];
	  if (sh->removed) sh=mw_get_parent_shape(sh);
	  fwrite(&tabIDs[sh-shs->the_shapes],sizeof(int),1,fp);
     }

     /* write the shapes */
     for (i=0; i<shs->nb_shapes; i++)
	  if (!shs->the_shapes[i].removed)
	  {
	       sh = &shs->the_shapes[i];
	       if (i==0) 
		    _mw_write_mw2_shape(fp,sh,0);
	       else 
	       {
		    diff = mw_get_parent_shape(sh) - shs->the_shapes;
		    if ((diff<0)||(diff>=shs->nb_shapes))
		    {
			 mwerror(ERROR, 0,"Cannot write file \"%s\" : inconsistent shapes structure\n",fname);
			 fclose(fp);
			 return(-1);
		    }
		    _mw_write_mw2_shape(fp,sh,tabIDs[diff]);
	       }
	  }	  

     free(tabIDs);

     /* Record data field */
     if (shs->data_size > 0) 
	  fwrite((char *)shs->data,sizeof(char),shs->data_size,fp);

     fclose(fp);
     return(0);
}

/* Write file in different formats */
   
short _mw_create_shapes(char * fname, Shapes shs, char * Type)
{
     /* Native format */
     if (strcmp(Type,"MW2_SHAPES") == 0)
	  return(_mw_create_mw2_shapes(fname,shs));
  
     /* Other formats */
     return(-1);
}

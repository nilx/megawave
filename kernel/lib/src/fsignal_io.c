/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   fsignal_io.c
   
   Vers. 1.5
   (C) 1993-2002 Jacques Froment
   Input/Output functions for the fsignal structure

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

#include "ascii_file.h"
#include "mw.h"

/*~~~~~~ ASCII Format ~~~~~~*/

Fsignal _mw_load_fsignal_ascii(fname,signal)

char  *fname;  /* Name of the file */
Fsignal signal;  /* pre-defined signal (may be NULL) */
     
{ FILE    *fp;
  int N;
  long pos0,pos1;
  register int i;
  float v;
  register float *ptr;
  int BitsPerSample;

  fp = _mw_open_data_ascii_file(fname);
  if (fp == NULL) return(NULL);

  if (_mw_fascii_search_string(fp,"def Fsignal\n") == EOF)
    {
      mwerror(ERROR, 0,
	      "No Fsignal description found in the file \"%s\"\n",fname);
      fclose(fp);
      return(NULL);
    }

  if (signal == NULL) signal = mw_new_fsignal();
  if (signal == NULL) { fclose(fp); return(signal); }

  /* Remember the position at the beginning of header */
  pos0=ftell(fp);

  if ((_mw_fascii_get_field(fp,fname,"comments:","%[^\n]",signal->cmt) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"size:","%d\n",&signal->size) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"scale:","%f\n",&signal->scale) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"shift:","%f\n",&signal->shift) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"gain:","%f\n",&signal->gain) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"sgrate:","%f\n",&signal->sgrate) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"firstp:","%d\n",&signal->firstp) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"lastp:","%d\n",&signal->lastp) != 1)
      ||
      (_mw_fascii_get_field(fp,fname,"param:","%f\n",&signal->param) != 1)
      )
    {
      mw_delete_fsignal(signal);
      fclose(fp);
      return(NULL);
    }

  /* Remember the position at the end of header */
  pos1=ftell(fp);

  /* Read extended fields */
  if (fseek(fp,pos0,SEEK_SET)!=0) 
    { mw_delete_fsignal(signal); fclose(fp); return(NULL); }

  if (_mw_fascii_get_optional_field(fp,fname,"bpsample:","%d\n",&BitsPerSample) == 1)
    signal->bpsample=BitsPerSample;

  /* Now read the data */
  if (fseek(fp,pos1,SEEK_SET)!=0) 
    { mw_delete_fsignal(signal); fclose(fp); return(NULL); }

  N = signal->size;
  if (mw_change_fsignal(signal,N) == NULL) 
    { mw_delete_fsignal(signal); fclose(fp); return(NULL); }

  i=0;
  while (i < N)
    {
      if (fscanf(fp,"%f\n",&v) !=  1)
	{
	  mwerror(ERROR, 0,
		  "Less values than expected in the Fsignal description\n");
	  mw_delete_fsignal(signal);
	  fclose(fp);
	  return(NULL);
	}	
      signal->values[i] = v;
      i++;
    }
      
  fclose(fp);
  return(signal);
}


short _mw_create_fsignal_ascii(fname,signal)

char  *fname;                        /* file name */
Fsignal signal;

{
  FILE *fp;
  int i;

  if (signal == NULL)
    mwerror(INTERNAL,1,"Cannot create file: Fsignal structure is NULL\n");

  if (signal->size <= 0)
    mwerror(INTERNAL,1,
	      "Cannot create file: No values in the Fsignal structure\n");

  fp =_mw_create_data_ascii_file(fname);
  if (fp == NULL) return(-1);

  fprintf(fp,"#----- Fsignal -----\n");
  fprintf(fp,"#def Fsignal\n");

  fprintf(fp,"#comments: %s\n",signal->cmt);

  fprintf(fp,"#size: %d\n",signal->size);
  fprintf(fp,"#scale: %e\n",signal->scale);
  fprintf(fp,"#shift: %e\n",signal->shift);
  fprintf(fp,"#gain: %e\n",signal->gain);
  fprintf(fp,"#sgrate: %e\n",signal->sgrate);
  fprintf(fp,"#bpsample: %d\n",signal->bpsample);
  fprintf(fp,"#firstp: %d\n",signal->firstp);
  fprintf(fp,"#lastp: %d\n",signal->lastp);
  fprintf(fp,"#param: %e\n\n",signal->param);

  for (i=0;i<signal->size;i++) fprintf(fp,"%e\n",signal->values[i]);
      
  fclose(fp);
  return(0);
}

/*~~~~~~ MegaWave2 formats ~~~~~*/

   

Fsignal _mw_load_fsignal(fname,type,signal)

char *fname;  /* Name of the file */
char *type;    /* type of the file */
Fsignal signal;  /* pre-defined signal (may be NULL) */

{
  char mtype[mw_mtype_size];
  int hsize;  /* Size of the header, in bytes */
  float version;/* Version number of the file format */
  int need_flipping;     
 
  need_flipping=_mw_get_file_type(fname,type,mtype,&hsize,&version)-1;
  if (need_flipping >=0)
    {
      if (strcmp(type,"A_FSIGNAL") == 0)
	return(_mw_load_fsignal_ascii(fname,signal));
      
      if (strcmp(type,"WAVE_PCM") == 0)
	return((Fsignal)_mw_fsignal_load_wave_pcm(fname,signal,need_flipping));
    }
  
  mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",type,fname);
}


short _mw_create_fsignal(fname,signal,type)

char  *fname;                        /* file name */
Fsignal signal;
char *type;    /* type of the file */

{
    if (strcmp(type,"A_FSIGNAL") == 0)
      return(_mw_create_fsignal_ascii(fname,signal));

    if (strcmp(type,"WAVE_PCM") == 0)
      return(_mw_fsignal_create_wave_pcm(fname,signal));

    mwerror(FATAL, 0,"Invalid type \"%s\" for the file \"%s\"\n",type,fname);
}

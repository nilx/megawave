/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  wave_io.c
   
  Vers. 1.0
  Author : Jacques Froment
  Input/Output functions for the Microsoft's RIFF WAVE sound file format
  Partly inspired by file 'wav.c' included in the SoX distribution :
  (C) 1992 Rick Richardson, (C) 1991 Lance Norskog And Sundry Contributors.

  Main changes :
  v1.0 (JF): added include <string> (Linux 2.6.12 & gcc 4.0.2)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
  MegaWave2 is a "soft-publication" for the scientific community. It has
  been developed for research purposes and it comes without any warranty.
  The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
  CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
  94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "definitions.h"
#include "error.h"

#include "mwio.h"
#include "fsignal.h"

#include "wave_io.h"

/*~~~~~~ WAVE PCM format ~~~~~~*/

Fsignal _mw_fsignal_load_wave_pcm(char *fname, Fsignal signal,
				  int need_flipping)
{ 
     FILE    *fp;
     unsigned int dwFmtSize; /* length of format chunk minus 8 byte header (dw=4 bytes) */
     unsigned short wFormatTag; /* Format Tag (w = 2 bytes) */
     unsigned short wChannels; /* Number of channel */
     unsigned int dwSamplesperSecond; /* samples per second per channel */
     unsigned int dwAvgBytesPerSec; /* Average bytes per Second */
     unsigned short wBlockAlign; /* basic block size */
     unsigned short wBitsPerSample; /* Bits per Sample */
     unsigned int dwDataLength; /* Number of bytes of the data */

     /* Size of buffer for I/O buffering */
#define BS 65536
     char *buf;
     short *wbuf;
     int *dwbuf;
     long toread,remain;
     int c,N;
     long i,maxi;
     float *ptr;
     double v;

     if (!(fp = fopen(fname, "r"))) return(NULL);
  
     /* Seek beginning of format chunk */
     if (_mw_find_pattern_in_file(fp,"fmt ")<0) 
     {
	  mwerror(ERROR,1,"Cannot load WAVE file '%s' : no format chunk !\n",fname);
	  fclose(fp); return(NULL);
     }
     /* Next is length of format chunk minus 8 byte header */
     if (fread(&dwFmtSize,4,1,fp)!=1) 
     {
	  mwerror(WARNING,1,"Corrupted WAVE file '%s' : unexpected EOF !\n",fname);
	  fclose(fp); return(NULL);
     }
     if (need_flipping==1) _mw_in_flip_b4(dwFmtSize);
     if (dwFmtSize < 16)
     {
	  mwerror(ERROR,1,"Cannot load WAVE file '%s' : format chunk too short !\n",fname);
	  fclose(fp); return(NULL);
     }
     /* Format Tag : indentifies WAVE encoding such as PCM, ADPCM, ULAW, ... */
     if (fread(&wFormatTag,2,1,fp)!=1)
     {
	  mwerror(WARNING,1,"Corrupted WAVE file '%s' : unexpected EOF !\n",fname);
	  fclose(fp); return(NULL);
     }
     if (need_flipping==1) _mw_in_flip_b2(wFormatTag);
     if (wFormatTag != 0x0001)
	  /* Not a WAVE PCM encoding : shouldn't be there */
	  mwerror(INTERNAL,1,"[_mw_fsignal_load_wave_pcm] WAVE file '%s' : no PCM encoding !\n",fname);
     /* Number of channels */
     if (fread(&wChannels,2,1,fp)!=1)      
     {
	  mwerror(WARNING,1,"Corrupted WAVE file '%s' : unexpected EOF !\n",fname);
	  fclose(fp); return(NULL);
     }
     if (need_flipping==1) _mw_in_flip_b2(wChannels);
     /*printf("Nb of channels=%d\n",wChannels);*/
     /* Samples per second per channel */
     if (fread(&dwSamplesperSecond,4,1,fp)!=1) 
     {
	  mwerror(WARNING,1,"Corrupted WAVE file '%s' : unexpected EOF !\n",fname);
	  fclose(fp); return(NULL);
     }
     if (need_flipping==1) _mw_in_flip_b4(dwSamplesperSecond);
     /*printf("samples per second per channel =%u\n",dwSamplesperSecond);*/
     /* Average bytes per Second */
     if (fread(&dwAvgBytesPerSec,4,1,fp)!=1) 
     {
	  mwerror(WARNING,1,"Corrupted WAVE file '%s' : unexpected EOF !\n",fname);
	  fclose(fp); return(NULL);
     }
     if (need_flipping==1) _mw_in_flip_b4(dwAvgBytesPerSec); 
     /*printf("Average bytes per Second =%u\n",dwAvgBytesPerSec); */
     /* basic block size */
     if (fread(&wBlockAlign,2,1,fp)!=1)      
     {
	  mwerror(WARNING,1,"Corrupted WAVE file '%s' : unexpected EOF !\n",fname);
	  fclose(fp); return(NULL);
     }
     if (need_flipping==1) _mw_in_flip_b2(wBlockAlign);
     /*printf("basic block size=%d\n",wBlockAlign);*/
     /* Bits per Sample */
     if (fread(&wBitsPerSample,2,1,fp)!=1)      
     {
	  mwerror(WARNING,1,"Corrupted WAVE file '%s' : unexpected EOF !\n",fname);
	  fclose(fp); return(NULL);
     }
     if (need_flipping==1) _mw_in_flip_b2(wBitsPerSample);
     /*printf("Bits per Sample = %d\n",wBitsPerSample);*/
     if (!((wBitsPerSample==8)||(wBitsPerSample==16)||(wBitsPerSample==32)))
     {
	  mwerror(WARNING,1,"Cannot load WAVE file '%s' : no code to handle data of %d bits/sample, sorry !\n",
		  fname,wBitsPerSample);
	  fclose(fp); return(NULL);
     }     
     /* Seek beginning of data chunk */
     if (_mw_find_pattern_in_file(fp,"data")<0) 
     {
	  mwerror(ERROR,1,"Cannot load WAVE file '%s' : no data chunk !\n",fname);
	  fclose(fp); return(NULL);
     }
     /* Next is length of format chunk minus 8 bytes header */
     if (fread(&dwDataLength,4,1,fp)!=1) 
     {
	  mwerror(WARNING,1,"Corrupted WAVE file '%s' : unexpected EOF !\n",fname);
	  fclose(fp); return(NULL);
     }
     if (need_flipping==1) _mw_in_flip_b4(dwDataLength);
     /*printf("Data Length = %u\n",dwDataLength);*/

     /* Begin to fill the structure */
     if (signal == NULL) signal = mw_new_fsignal();
     if (signal == NULL) { fclose(fp); return(signal); }

     /* Number of samples (using one channel) */
     N=8*dwDataLength/(wChannels*wBitsPerSample);
     if (mw_change_fsignal(signal,N) == NULL)  
     { 
	  mwerror(ERROR,1,"Cannot load WAVE file '%s' : not enough memory for the %d samples ! \n",fname,N);
	  mw_delete_fsignal(signal);
	  fclose(fp); return(NULL); 
     }
  
     /* Comments : WAVE PCM seems not to carry this field */
     sprintf(signal->cmt,"WAVE PCM format %d Hz %d bits %d channel(s)",
	     dwSamplesperSecond,wBitsPerSample,wChannels);

     /* Sampling rate */
     signal->sgrate=dwSamplesperSecond;
  
     /* NUmber of Bits per Sample */
     switch(wBitsPerSample)
     {
     case 8 : 
	  signal->bpsample=8;
	  break;
     case 16:
	  signal->bpsample=16;
	  break;
     case 32:
	  signal->bpsample=32;
	  break;
     default:
	  mwerror(INTERNAL,1,"Cannot load WAVE file '%s' : no code to handle data of %d bits/sample !\n",
		  fname,wBitsPerSample);      
     }

     if (wChannels==2)
	  mwerror(WARNING,1,"Converting stereo audio data '%s' to mono.\n",fname);
     if (wChannels>2)
	  mwerror(WARNING,1,"Converting %d-channels audio data '%s' to mono.\n",
		  wChannels,fname);
  
	    
     /* Now begin the buffered data read */
     remain=dwDataLength;
     ptr=signal->values;
     buf=(char *) malloc(BS);
     if (buf == NULL)  
     { 
	  mwerror(ERROR,1,"Cannot load WAVE file '%s' : not enough memory for I/O buffering ! \n",fname);
	  mw_delete_fsignal(signal);
	  fclose(fp); return(NULL); 
     }
     wbuf= (short *)buf;
     dwbuf=(int *)buf;
     while (remain > 0)
     {
	  toread = (remain>BS) ? BS : remain;
          /* FIXME: wrong types, dirty temporary fix */
	  if (fread(buf,1,toread,fp)!= (unsigned int) toread) 
	  {
	       mwerror(WARNING,1,"Corrupted WAVE file '%s' : unexpected EOF !\n",fname);
	       free(buf);
	       mw_delete_fsignal(signal);
	       fclose(fp); return(NULL);
	  }
	  remain-=toread;
	  switch(wBitsPerSample)
	  {
	  case 8 :
	       maxi=toread;
	       if (wChannels<=1) 
		    for (i=0; i<maxi;i++) *ptr++=buf[i];
	       else
		    for (i=0; i<maxi;i+=wChannels)
		    {
			 for (c=0, v=0.0 ; c<wChannels ; c++) v += buf[i+c];
			 v/=wChannels;
			 *ptr++ = v;
		    }
	       break;
	  case 16:
	       maxi=toread >> 1;
	       if (wChannels<=1) 
		    for (i=0; i<maxi;i++) *ptr++=wbuf[i];
	       else
		    for (i=0; i<maxi;i+=wChannels)
		    {
			 for (c=0, v=0.0 ; c<wChannels ; c++) v += wbuf[i+c];
			 v/=wChannels;
			 *ptr++ = v;
		    }
	       break;
	  case 32:
	       maxi=toread >> 2;
	       if (wChannels<=1) 
		    for (i=0; i<maxi;i++) *ptr++=dwbuf[i];
	       else
		    for (i=0; i<maxi;i+=wChannels)
		    {
			 for (c=0, v=0.0 ; c<wChannels ; c++) 
			      v += dwbuf[i+c];
			 v/=wChannels;
			 *ptr++ = v;
		    }
	       break;
	  }
     }
     free(buf);
     fclose(fp);
     return(signal);
} 

/* Save the Fsignal structure into a WAVE PCM file.
   Quantization (number of bits/sample) is choosen accordingly to signal->gain,
   following the computation done by _mw_fsignal_load_wave_pcm().
   Set signal->gain = 1 before calling this function to avoid quantization.
*/

short _mw_fsignal_create_wave_pcm(char *fname, Fsignal signal)
{
     FILE *fp;
     char Magic[5];
     unsigned short wFormatTag; /* Format Tag (w = 2 bytes) */
     unsigned int dwRiffLength ; /* length of file after 8 byte RIFF  header */
     unsigned int dwFmtSize; /* length of format chunk minus 8 byte header (dw=4 bytes) */
     unsigned int dwDataLength; /* Number of bytes of the data */
     unsigned short wChannels; /* Number of channel */
     unsigned int dwSamplesperSecond; /* samples per second per channel */
     unsigned int dwAvgBytesPerSec; /* Average bytes per Second */
     unsigned short wBlockAlign; /* basic block size */
     unsigned short wBitsPerSample; /* Bits per Sample */
     int lost=0;

     /* Size of buffer for I/O buffering */
#define BS 65536
     unsigned char *buf;
     short *wbuf;
     int *dwbuf;
     long towrite,remain;
     long i,maxi;
     float *ptr;

     if (signal == NULL)
	  mwerror(INTERNAL,1,"[_mw_fsignal_create_wave_pcm] Cannot create file: Fsignal structure is NULL\n");

     if (signal->size <= 0)
	  mwerror(INTERNAL,1,
		  "[_mw_fsignal_create_wave_pcm] Cannot create file: No values in the Fsignal structure\n");

     dwFmtSize = 16;
     wChannels = 1;
     wFormatTag = 0x0001; /* PCM */
     dwSamplesperSecond = signal->sgrate+0.5;
     wBitsPerSample=signal->bpsample;
     wBlockAlign = wChannels * ((wBitsPerSample + 7)/8);
     dwAvgBytesPerSec = (double)wBlockAlign*dwSamplesperSecond + 0.5;  
     dwDataLength=((long)wChannels*wBitsPerSample*signal->size)/8;
     dwRiffLength = 4 + (8+dwFmtSize) + (8+dwDataLength); 

     if (!(fp = fopen(fname, "w")))
     {
	  mwerror(ERROR, 0,"Cannot open file \"%s\" for writing !\n",fname);
	  return(-1);
     }

     /* Note on byte ordering with RIFF files : the default byte ordering is little-endian.
	File written using big-endian byte ordering scheme have the identifier RIFX 
	instead of RIFF. (At least for WAVE files, has to be checked with other RIFF files).
     */
     if (_mw_byte_ordering_is_little_endian()==1)
	  strcpy(Magic,"RIFF");
     else
	  strcpy(Magic,"RIFX");
  
     if ((fwrite(Magic,1,4,fp)!=4) ||
	 (fwrite(&dwRiffLength,1,4,fp)!=4) ||
	 (fwrite("WAVE",1,4,fp)!=4) ||
	 (fwrite("fmt ",1,4,fp)!=4) ||
	 (fwrite(&dwFmtSize,1,4,fp)!=4) ||
	 (fwrite(&wFormatTag,1,2,fp)!=2) ||
	 (fwrite(&wChannels,1,2,fp)!=2) ||
	 (fwrite(&dwSamplesperSecond,1,4,fp)!=4) ||
	 (fwrite(&dwAvgBytesPerSec,1,4,fp)!=4) ||
	 (fwrite(&wBlockAlign,1,2,fp)!=2) ||
	 (fwrite(&wBitsPerSample,1,2,fp)!=2) ||
	 (fwrite("data",1,4,fp)!=4) ||
	 (fwrite(&dwDataLength,1,4,fp)!=4))
     {
	  mwerror(ERROR, 0,"Cannot write WAVE PCM header into file \"%s\" !\n",fname);
	  fclose(fp); return(-1); 
     }


     /* Now begin the buffered data write */
     remain=dwDataLength;
     ptr=signal->values;
     buf= (unsigned char *) malloc(BS);
     if (buf == NULL)  
     { 
	  mwerror(ERROR,1,"Cannot load WAVE file '%s' : not enough memory for I/O buffering ! \n",fname);
	  fclose(fp); return(-1); 
     }
     wbuf= (short *)buf;
     dwbuf=(int *)buf;
     while (remain > 0)
     {
	  towrite = (remain>BS) ? BS : remain;
	  switch(wBitsPerSample)
	  {
	  case 8 :
	       maxi=towrite;
	       /* Change signed char -> unsigned char */
	       for (i=0; i<maxi;i++) 
	       {
		    buf[i] = ((char)*ptr)^0x80; 
		    if ((char)*ptr != *ptr++) lost=1;
	       }
	       break;
	  case 16:
	       maxi=towrite >> 1;
	       for (i=0; i<maxi;i++) 
	       {
		    wbuf[i]=*ptr;
		    if (wbuf[i]!=*ptr++) 
			 lost=1;
	       }
	       break;
	  case 32:
	       maxi=towrite >> 2;
	       for (i=0; i<maxi;i++) 
	       {
		    dwbuf[i] =*ptr;
		    if (dwbuf[i]!=*ptr++) lost=1;
	       }	    
	  }
          /* FIXME: wrong types, dirty temporary fix */
	  if (fwrite(buf,1,towrite,fp)!= (unsigned int) towrite) 
	  {
	       mwerror(ERROR, 0,"Cannot write WAVE PCM data into file \"%s\" !\n",fname);
	       free(buf);
	       fclose(fp); return(-1); 
	  }
	  remain-=towrite;
     }

     if (lost==1) mwerror(WARNING,0,"Information has been lost due to WAVE PCM quantization.\n");

     free(buf);  
     fclose(fp);
     return(0);
}

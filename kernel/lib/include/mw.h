/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   mw.h
   
   Vers. 1.16
   (C) 1993-2001 Jacques Froment & Sylvain Parrino
   Main include file for the MegaWave2 System Library.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef MW_FLG
#define MW_FLG

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef MW_LIB

#include <stdio.h>

#include <time.h>
#define	mw_time()	time((time_t *) NULL)

/* Function prototypes for ANSI compilers */
#ifdef __STDC__

/* defines malloc as void * */
#include <stdlib.h>

extern void  mwexit(int);
extern void  mw_exit(int);
extern void  *mwmalloc(size_t);
extern void  mwfree(void *);
extern void  *mwrealloc(void *, size_t);
extern void  *mwcalloc(size_t, size_t);
extern void  mwcfree(void *);
extern void  MegaWaveDefOpt(char *);
extern int   mw_opt_used(char);
extern void  mwdebug(char *, ...);
extern void  mwerror(int, int, char *, ...);
extern int   mwis_readable(char *);
extern int   mwis_writable(char *);
extern int   _mwis_open(char *, char *);
extern int   _mwgetopt(int, char **, char *);

/* Functions in mwio.c */
extern void _mw_flip_image(unsigned char *,short,short,short,char);
extern FILE *_mw_write_header_file(char *,char *,float);
extern long _mw_find_pattern_in_file(FILE *,char *);
extern int _mw_byte_ordering_is_little_endian(void);

/* I/O conversion functions called by mwp (data_io.c) 
   and defined in mw.c     
*/
extern char           *_mw_ctoa_(char);
extern char           *_mw_uctoa_(unsigned char);
extern char           *_mw_stoa_(short);
extern char           *_mw_ustoa_(unsigned short);
extern char           *_mw_itoa_(int);
extern char           *_mw_uitoa_(unsigned int);
extern char           *_mw_ltoa_(long);
extern char           *_mw_ultoa_(unsigned long);
extern char           *_mw_ftoa_(float);
extern char           *_mw_dtoa_(double);


/* Function prototypes for NON-ANSI compilers */

#else

/* defines malloc as char * */
#include <malloc.h>

extern       mwexit();
extern       mw_exit();
extern char  *mwmalloc();
extern int   mwfree();
extern char  *mwrealloc();
extern char  *mwcalloc();
extern int   mwcfree();
extern       MegaWaveDefOpt();
extern       mw_opt_used();
extern       mwdebug();
extern       mwerror();
extern int   mwis_readable();
extern int   mwis_writable();
extern int   _mwis_open();
extern int   _mwgetopt();

extern void _mw_flip_image();
extern FILE *_mw_write_header_file();
extern long _mw_find_pattern_in_file();
extern int _mw_byte_ordering_is_little_endian();

/* I/O conversion functions called by mwp (data_io.c) 
   and defined in mw.c     
*/
extern char           *_mw_ctoa_();
extern char           *_mw_uctoa_();
extern char           *_mw_stoa_();
extern char           *_mw_ustoa_();
extern char           *_mw_itoa_();
extern char           *_mw_uitoa_();
extern char           *_mw_ltoa_();
extern char           *_mw_ultoa_();
extern char           *_mw_ftoa_();
extern char           *_mw_dtoa_();
#endif

/* I/O conversion macros called by mwp (data_io.c) */
#define	_mw_atoq_(S)	(S)
#define	_mw_atoc_(S)	*(S)
#define	_mw_atouc_(S)	(unsigned char) *(S)
#define	_mw_atos_(S)	(short) atoi(S)
#define	_mw_atous_(S)	(unsigned short) atoi(S)
#define	_mw_atoi_(S)	(int) atoi(S)
#define	_mw_atoui_(S)	(unsigned int) atoi(S)
#define	_mw_atol_(S)	atol(S)
#define	_mw_atof_(S)	(float) atof(S)
#define	_mw_atod_(S)	(double) atof(S)
#define _mw_qtoa_(S)	(S)


#define _mwis_readable(S)	_mwis_open(S, "r")
#define _mwis_writable(S)	_mwis_open(S, "w")

extern int   mwerrcnt;
extern char *mwname;
extern char *mwgroup;

/* Mode in which a module is run */
/* 1 for command line - set in lib/src/mw.c */
/* 2 for XMegaWave2  (interpretor) - set in lib/src/module_io.c */
extern int mwrunmode;

/* Delay to refrech windows (in microseconds). Set in lib/src/window.c */
extern int mwwindelay;

extern int   _mwoptind;
extern char *_mwoptarg;

/* ifndef MWLIB */
#endif 

#define WARNING  0
#define ERROR    1
#ifndef FATAL
#define FATAL    2
#endif
#define USAGE    3
#define INTERNAL 4

/* Include MegaWave2 Include files */

#include "file_type.h"

/* Unix-Bsd Emulation, if necessary */

#if defined(HPUX) || defined(sun4_5) || defined(IRIX) || defined(Linux)

#include "unix_bsd.h"
#define rint(S)	my_rint(S)
#define cbrt(S)	my_cbrt(S)

#endif

/* bzero is a BSD function; use memset on System V */

#if defined(HPUX) || defined(SunOS) || defined(Linux)
#define mw_bzero(s,n)  memset((s),0,(n))
#else
#define mw_bzero(s,n)  bzero((s),(n))
#endif

/* Flip macros : invert bytes order */

/* Get flipped value */
/* invert 2 bytes data : 01 -> 10 */
#define _mw_get_flip_b2(b2) ( (( ((unsigned short) (b2)) & 0xff00)>>8) + \
                              ((((unsigned short) (b2)) & 0x00ff)<<8) )

/* invert 4 bytes data : 0123 -> 3210 */
#define _mw_get_flip_b4(b4) ( (( ((unsigned long) (b4)) & 0xff000000)>>24)+ \
                              ((((unsigned long) (b4)) & 0x00ff0000)>>8) + \
                              ((((unsigned long) (b4)) & 0x0000ff00)<<8) + \
			      ((((unsigned long) (b4)) & 0x000000ff)<<24) ) 

/* In-place flipping */
/* invert 2 bytes data : 01 -> 10 */
#define _mw_in_flip_b2(b2)  (b2) = _mw_get_flip_b2(b2)

/* invert 4 bytes data : 0123 -> 3210 */
#define _mw_in_flip_b4(b4)  (b4) = _mw_get_flip_b4(b4)


/* For float, need to perform the flip by means of an unsigned long buffer 
   fptr : input has to be a pointer to a float
   flip_float : buffer of type unsigned long * (to be defined in the calling func.)
   output : *fptr is the flipped float value.
*/
#define _mw_in_flip_float(fptr) flip_float=(unsigned long *)(fptr); \
                                  _mw_in_flip_b4(*flip_float); 


/* For double=(u1,u2), performs (flip(u2),flip(u1)) where u1,u2 are
   long (b4 = 32 bits). 
   b4 : input has to be a pointer to a unsigned long.
   flip_double : buffer of type unsigned long (to be defined in the calling func.)
   output : *((double *) b4) is the flipped double value.   
*/

#define _mw_in_flip_double(b4)   _mw_in_flip_b4(*(b4)); \
                                 _mw_in_flip_b4(*((b4)+1)); \
                                 flip_double=*((b4)+1);\
                                 *((b4)+1)=*(b4); \
                                 *(b4)=flip_double;

/* Include size of some strings */

#include "string_size.h"

/* Include all other include files about memory formats */

#include "cmovie.h"
#include "fmovie.h"
#include "cimage.h"
#include "fimage.h"
#include "fsignal.h"
#include "curve.h"
#include "polygon.h"
#include "fcurve.h"
#include "dcurve.h"
#include "fpolygon.h"
#include "wtrans2d.h"
#include "wtrans1d.h"
#include "wmax2d.h"
#include "cfimage.h"
#include "ccimage.h"
#include "ccmovie.h"
#include "cfmovie.h" 
#include "module.h"
#include "mimage.h"
#include "cmimage.h"
#include "shape.h"
#include "rawdata.h"
#include "list.h"

/* Prototypes for conversion functions */
#include "basic_conv.h"

#endif



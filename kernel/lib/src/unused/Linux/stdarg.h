/* starg.h : try to fix syntax errors.
   V1.0 : Update for linux kernel 2.4.22 (libc 2.3.2)
*/

#ifndef _KERNEL_MAJOR_
#error _KERNEL_MAJOR_ should be defined by cmw2 !
#endif
#ifndef _KERNEL_MINOR_
#error _KERNEL_MINOR_ should be defined by cmw2 !
#endif

#if (_KERNEL_MAJOR_ == 2)  
  /* 2.x */
#if (_KERNEL_MINOR_ < 4)
  /* 2.1.x -> 2.3.x */
#include <stdarg_2.1.h>
#else
 /* 2.4.x and > */
#if (_KERNEL_MINOR_ == 4)
 /* 2.4.x */
#if (_KERNEL_PATCH_ <= 19)
 /* 2.4.19 (Mandrake 9.0) */
#include <stdarg_2.2.h>
#else
/* 2.4.20 and > */
/*
  stdarg.h modified from one found in kernel 2.4.22 (Mandrake 9.2). 
  Hope it works on kernel 2.4.20 (RedHat 9.0).
*/
#include <stdarg_2.3.h>

#endif
#endif
#endif

#else
/* kernels 1.x or 3.x and greater */
#include <stdarg_2.3.h>

#endif

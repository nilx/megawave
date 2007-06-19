/* stdio.h : try to fix syntax errors.
   V1.2 : Update for linux kernel 2.6.3 (libc 2.3.3).
   Does not work e.g. for kernel 2.6.12 : use the
   'light' preprocessor instead.
*/

#ifndef _KERNEL_MAJOR_
#error _KERNEL_MAJOR_ should be defined by cmw2 !
#endif
#ifndef _KERNEL_MINOR_
#error _KERNEL_MINOR_ should be defined by cmw2 !
#endif

#if (_KERNEL_MAJOR_ == 2)
/* 2.x */
#if (_KERNEL_MINOR_ <= 1)
/* 2.0 & 2.1 */
#include <stdio_2.1.h>
#else
#if (_KERNEL_MINOR_ < 4)
/* 2.2 & 2.3 */
#include <stdio_2.2.h>
#else
#if (_KERNEL_MINOR_ == 4)
/* 2.4 */
#if (_KERNEL_PATCH_ <= 19)
#include <stdio_2.2.h>
#else
/*
  2.4.x x>19 
  stdio.h modified from one found in kernel 2.4.22 (Mandrake 9.2). 
  Hope it works on kernel 2.4.20 (RedHat 9.0).
*/
#include <stdio_2.3.h>
#endif
#else
/* 2.x with x>=5 */
#include <stdio_2.3.3.h>
#endif
#endif
#endif
#else
/* kernels 1.x or 3.x and greater */
#include "/usr/include/stdio.h"

#endif


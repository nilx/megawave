/* libio.h : try to fix syntax errors.
   V1.0 : Update for linux kernel 2.4.22 (libc 2.3.2)
*/

#ifndef _KERNEL_MAJOR_
#error _KERNEL_MAJOR_ should be defined by cmw2 !
#endif
#ifndef _KERNEL_MINOR_
#error _KERNEL_MINOR_ should be defined by cmw2 !
#endif

#if (_KERNEL_MAJOR_ == 2)
#if (_KERNEL_MINOR_ <= 1)
#include <libio_2.1.h>
#else
#if (_KERNEL_MINOR_ <= 2)
#include <libio_2.2.h>
#else
/*
  libio.h is clean on kernel 2.4.22. Hope it is also ok on kernel 2.3.x
*/
#include "/usr/include/libio.h"

#endif
#endif

#else
/* kernels 1.x or 3.x and greater */
#include "/usr/include/libio.h"
#endif


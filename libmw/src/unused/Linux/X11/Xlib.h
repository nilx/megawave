/* Xlib.h : try to fix syntax errors.
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
 /* 2.1.x -> 2.3.x such as RedHat 5.2, 6.2 , Mandrake 6.0 */
#include <X11/Xlib_2.1.h>
#else
/* 2.4.x and > */
#if (_KERNEL_MINOR_ == 4)
 /* 2.4.x */
#if (_KERNEL_PATCH_ < 19)
 /* e.g. 2.4.8 (Mandrake 8.1) */
#include <X11/Xlib_v1.5.h>
#else
  /* 2.4.19 and > such as Mandrake 9.0,9.1 and 9.2 
     Xlib.h v1.6 seems to be clean, can directly load it !
  */
#include "/usr/include/X11/Xlib.h"
#endif
#else
 /* 2.5.x and > */
#include "/usr/include/X11/Xlib.h"
#endif
#endif

#else
 /* 1.x or 3.x ...*/
#include "/usr/include/X11/Xlib.h"
#endif

#ifndef _KERNEL_MAJOR_
#error _KERNEL_MAJOR_ should be defined by cmw2 !
#endif
#ifndef _KERNEL_MINOR_
#error _KERNEL_MINOR_ should be defined by cmw2 !
#endif

#if (_KERNEL_MAJOR_ == 2) 
#if (_KERNEL_MINOR_ < 4)
#include <X11/Xlib_2.1.h>
#else
#include <X11/Xlib_2.4.h>
#endif
#endif

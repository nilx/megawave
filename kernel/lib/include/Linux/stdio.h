#ifndef _KERNEL_MAJOR_
#error _KERNEL_MAJOR_ should be defined by cmw2 !
#endif
#ifndef _KERNEL_MINOR_
#error _KERNEL_MINOR_ should be defined by cmw2 !
#endif

#if (_KERNEL_MAJOR_ == 2)
#if (_KERNEL_MINOR_ <= 1)
#include <stdio_2.1.h>
#else
#include <stdio_2.2.h>
#endif
#endif


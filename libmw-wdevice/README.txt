COMPILATION OF THE MEGAWAVE WDEVICE LIBRARY

# basic

* compile the library shared and static objects
  The result is `./libmw-wdevice.so` and `./libmw-wdevice.a`.
    make lib
* compile the library api header
  The result is `./libmw-wdevice.h`.
    make lib
* cleanup the compilation headers, objects, ...
    make clean
* cleanup everything, keep only the source
    make distclean
* compile everything and clean
   make && make clean

# other targets

* compile the manpage
  The result is `doc/libmw-wdevice.man` and `doc/libmw-wdevice.man.html`.
    make man
* beautify the source code
    make beautify
* lint the source code
    make lint
* do all the portable stuff for compiling later on a machine with only
  a compiler and a linker
    make prebuild

# options

* compile with strict warning options
  Append `STRICT=1` to the make command.
* compile with compilet optimization
  Append `OPTI=1` to the make command.
* compile for debugging
  Append `DEBUG=1` to the make command.
* compile for profiling
  Append `PROF=1` to the make command.

# required tools

* for lib
  * gcc
  * cproto
* for man
  * pandoc
* for beautify
  * expand
  * indent
  * sed
* for lint
  * splint
  * file
  * wc

# configuration

All the configuration is in `../common/makefile`.

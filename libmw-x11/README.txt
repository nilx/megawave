INSTRUCTIONS FOR COMPILING THE MEGAWAVE X11 LIBRARY

See `../README.txt` for generic instructions.

# basic

* compile the library shared and static objects
  The result is `./libmw-x11.so` and `./libmw-x11.a`.
    make lib
* compile the library api header
  The result is `./libmw-x11.h`.
    make api
* cleanup the compilation headers, objects, ...
    make clean
* cleanup everything, keep only the source
    make distclean
* compile everything and clean
    make && make clean

# other targets

* compile the manpage
  The result is `doc/libmw-x11.man` and `doc/libmw-x11.man.html`.
    make man
* beautify the source code
    make beautify
* lint the source code
    make lint
* do all the portable stuff for compiling later on a machine with only
  a compiler and a linker
    make prebuild

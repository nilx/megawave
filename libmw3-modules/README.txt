INSTRUCTIONS FOR COMPILING THE MEGAWAVE MODULES

See `../README.txt` for generic instructions.

# basic

* compile the library shared and static objects
  The result is `libmw3-modules.so` and `libmw3-modules.a` in $LIBDIR.
    make lib
* compile the library api header
  The result is `mw3-modules.h` in $INCDIR.
    make api
* compile the executable modules
  The result is in $BINDIR.
    make modules
* cleanup the compilation headers, objects, ...
    make clean
* cleanup everything, keep only the source
    make distclean
* compile everything and clean
    make && make clean

# other targets

* beautify the source code
    make beautify
* lint the source code
    make lint
* do all the portable stuff for compiling later on a machine with only
  a compiler and a linker
    make prebuild

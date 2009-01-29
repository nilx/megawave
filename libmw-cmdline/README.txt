INSTRUCTIONS FOR COMPILING THE MEGAWAVE COMMANDLINE LIBRARY

See `../README.txt` for generic instructions.

# basic

* compile the library shared and static objects
  The result is `./libmw-cmdline.so` and `./libmw-cmdline.a`.
    make lib
* compile the library api header
  The result is `./libmw-cmdline.h`.
    make api
* cleanup the compilation headers, objects, ...
    make clean
* cleanup everything, keep only the source
    make distclean
* compile everything and clean
    make && make clean

# other targets

* compile the manpage
  The result is `doc/libmw-cmdline.man` and `doc/libmw-cmdline.man.html`.
    make man
* beautify the source code
    make beautify
* lint the source code
    make lint
* do all the portable stuff for compiling later on a machine with only
  a compiler and a linker
    make prebuild

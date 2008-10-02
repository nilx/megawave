COMPILATION OF THE MEGAWAVE LIGHT COMPILER

# basic

* compile the executable
  The result is `./mwplight`.
    make bin
* cleanup the compilation headers, objects, ...
    make clean
* cleanup everything, keep only the source
    make distclean
* compile everything and clean
   make && make clean

# other targets

* compile the manpage
  The result is `./mwplight.man` and `./mwplight.man.html`.
    make man
* beautify the source code
    make beautify
* lint the source code
    make lint
* test the code and the resulting program
    make test
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

* for bin
  * gcc
  * cproto
  * gengetopt
  * csplit
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
* for test
  *

# configuration

All the configuration is in `../common/makefile`.

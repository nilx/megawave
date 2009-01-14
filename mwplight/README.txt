INSTRUCTIONS FOR COMPILING THE MEGAWAVE LIGHT PREPROCESSOR

See `../README.txt` for generic instructions.

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
  The result is `./doc/mwplight.man` and `./doc/mwplight.man.html`.
    make man
* beautify the source code
    make beautify
* lint the source code
    make lint
* test the code and the resulting program
    make test

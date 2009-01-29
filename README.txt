INSTRUCTIONS FOR COMPILING MEGAWAVE



# basic

* Prepare the source; this is only needed if you downloaded a "rawsrc"
  version of the source code. It will generate some automatic source
  and header files and the manpages.
  Required tools: `make`, `cproto`, `gengetopt`, `csplit` and `pandoc`.

    make prebuild

* Compile everything: the preprocessor, `libmw`, `libmw-wdevice`,
  `libmw-modules`, and the executable modules.
  Required tools: `make`, `gcc`.

    make

* Compile the system and user documentation manuals.
  This target is not thoroughly tested.
  Required tools: `pdflatex`, `hevea`, `hacha`, `html2text`.

    make doc

* Compile source documentation (warning: needs time)
  This target is not thoroughly tested.
  Required tools: `doxygen`, `graphviz`, `pdflatex`.

    make srcdoc

* Cleanup the compilation headers and objects.

    make clean

* Cleanup everything, keep only the raw source.

    make distclean



# options

Different options are available. Include these options in the
command-line to use them.

* MODE      The compilation mode can be
  - normal  : default standatd behaviour
  - opti    : use optimisation options
  - profile : include profiling instructions, for use with gprof
  - debug   : include debugging symbols
* LINK      The linking method can be
  - dynamic : default dynamic linking
  - static  : link statically
* CHECK     The code checking mode can be
  - relax   : default ansi mode
  - strict  : strict ansi confirmance with extra code quality check

Example:

Build the modules with the `ccache` compiler wrapper, using strict
syntax check, with static linking and debugging symbols.

    make CHECK=strict LINK=static MODE=debug modules



# parameters

Different parameters can be changed from the command-line. Some of
them are explained hereafter; please refer to `common/makefile` for a
full list.

* CC     the compiler, defaults to /usr/bin/gcc
* CCWRAP an optional wrapper over the compiler; intended for tools
         such as `ccache` or `distcc`.
* LD     the linker, defaults to be the same as CC
* LDWRAP an optional wrapper over the linker; intended for tools
         such as `diet`.

* .O  the file extension for objects files (default:.o)
* .A  the file extension for static  libraries files (default:.a)
* .SO the file extension for dynamic libraries files (default:.so)

Example:
Build the modules with the `gcc-3.4` and `ccache`.
    make CC=gcc-3.4 CCWRAP=ccache modules

This makefile is written on and for linux systems. Some predefined
parameter variations are available in `common/makefile.<SYSTEM>` for
other systems, but not thoroughly tested.
Uncomment the related include lines in `common/makefile` to use them.


# subfolders

You can also invoke make directly from within the subfolders
`mwp`, `libmw-x11`, `libmw`, `libmw-cmdline` and `modules`. The
previous options are still valid, and some other make targets are
available. Please refer to `<subfolder>/README.txt` for the details.

Some other targets are available from the subfolders, mainly for
development porpose (code linting, syntax cleanup, ...). They are not
extensively tested.



# configuration

All the build process configuration lies in `common/makefile`.



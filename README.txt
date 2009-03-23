MEGAWAVE 3.02



# OVERVIEW
##########

Megawave is a software library and a framework, dedicated to image
processing. It is produced by the image processing department of the
following math laboratory:
    CMLA (CNRS UMR-8536), Ecole Normale Superieure de Cachan
    61 avenue du President Wilson, 94235 Cachan cedex, France

It implements algorithms developed since 1988 in
* Universite Paris-IX Dauphine
* Ecole Normale Superieure de Cachan
* various associated laboratories.

You can find more information on
    http://mw.cmla.ens-cachan.fr/megawave/



# LICENCE AND COPYRIGHTS
########################

The licence for the previous versions was the "Megawave Licence"; see
LICENSE_MEGAWAVE.txt for details. A switch to BSD and GPL licences is
on the way, but not implemented yet, so the previous licencing scheme
still applies.

Unless specified otherwise, this software and the associated materials
(documentation, data) are 
(C) 1988-1998 Universite Paris-IX Dauphine
(C) 1999-2009 Ecole Normale Superieure de Cachan

Individual authors who contributed to megawave are listed in
AUTHORS.txt, and in the relevant files.



# REQUIREMENTS
##############

Megawave is expected to build and run on Linux, and maybe more POSIX
systems. It only works on a 32bit architecture. Running megawave
requires libtiff and libX11. Compiling megawave requires the
development parts of these libraries, plus make, gcc, and standard
unix utilities. gcc >= 4.3 may be required for code syntax check (see
below), but these check have been performed already, so an older
version of gcc is enough for compilation.

On Debian and Debian based systems, you can install
* the packages required for compilation and test with
    sudo aptitude install libtiff-dev libx11-dev make gcc bc
* the packages required for execution with
    sudo aptitude install libtiff libx11



# COMPILATION
#############

The compilation and compatibility status of megawave on various
platforms is tracked in the STATUS.txt file.


## BASIC

* Prepare the source; this is only needed if you downloaded a "rawsrc"
  version of the source code. It will generate some automatic source
  and header files.
  Required tools: `make`, `cproto`, `gengetopt` and `csplit`.

    make prebuild

* Compile everything: the `mwp` preprocessor, `libmw`,
  `libmw-cmdline`, `libmw-io`, `libmw-modules`, and the executable
  modules.
  Required tools: `make`, `gcc`.

    make

* Cleanup the compilation headers and objects.

    make clean

* Cleanup everything, keep only the raw source.

    make distclean


## OPTIONS

Different options are available. Include these options in the
command-line to use them.

* MODE       The compilation mode can be
  - normal   : default standatd behaviour
  - opti     : use optimisation options
  - profile  : include profiling instructions, for use with gprof
  - coverage : include code coverage instructions, for use with gcov
  - debug    : include debugging symbols
* LINK       The linking method can be
  - dynamic  : default dynamic linking
  - static   : link statically
* CHECK      The code checking mode can be
  - relax    : default ansi mode
  - strict   : strict ansi confirmance with extra code quality check
* SYSTEM     The sytem on the current machine
  - default  : default generic settings
  - osx	     : tweaks for Mac OSX
  - cygwin   : tweaks for cygwin
  - debian3  : tweaks for Debian 3.x

Example:

Build the modules with the `ccache` compiler wrapper, using strict
syntax check, with static linking and debugging symbols.

    make CHECK=strict LINK=static MODE=debug modules


## PARAMETERS

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


## CONFIGURATION

All the build process configuration lies in `common/makefile`.



# SUBPARTS OF THE PROJECT
#########################


## SUBFOLDERS

You can also invoke make directly from within the subfolders
`mwp`, `libmw`, `libmw-x11`, `libmw-io`, `libmw-cmdline` and
`modules`. The previous options are still valid, and some other make
targets are available. Please refer to `<subfolder>/README.txt` for
the details.

Some extra targets are available from the subfolders, mainly for
development purpose (code linting, syntax cleanup, ...). They are not
extensively tested.


## DOCUMENTATION

* Compile the system and user documentation manuals.
  This target is not thoroughly tested.
  Required tools: `pdflatex`, `hevea`, `hacha`, `html2text`.

    make doc

* Compile the man pages.
  This target is not thoroughly tested.
  Required tools: `pandoc`.

    make man

* Compile source documentation (warning: needs time)
  This target is not thoroughly tested.
  Required tools: `doxygen`, `graphviz`, `pdflatex`.

    make srcdoc

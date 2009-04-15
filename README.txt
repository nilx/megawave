% MEGAWAVE 3.02

1. OVERVIEW
2. LICENCE AND COPYRIGHTS
3. REQUIREMENTS
4. COMPILATION
5. INSTALLATION
6. CLEANUP
7. USAGE


# 1. OVERVIEW
#############

Megawave is a software library and a framework, dedicated to image
processing. It is produced by the image processing department of the
following math laboratory:

> CMLA (CNRS UMR-8536), Ecole Normale Superieure de Cachan
> 61 avenue du President Wilson, 94235 Cachan cedex, France

It implements algorithms developed since 1988 in Universite Paris-IX
Dauphine, Ecole Normale Superieure de Cachan, and various associated
laboratories.

You can find more information on `http://mw.cmla.ens-cachan.fr/megawave/`.



# 2. LICENCE AND COPYRIGHTS
###########################

The licence for the previous versions was the "Megawave Licence"; see
`LICENSE_MEGAWAVE.txt` for details. A switch to BSD and GPL licences
is on the way, but not implemented yet, so the previous licencing
scheme still applies.

Unless specified otherwise, this software and the associated materials
(documentation, data) are 

> (C) 1988-1998 Universite Paris-IX Dauphine
> (C) 1999-2009 Ecole Normale Superieure de Cachan

Individual authors who contributed to megawave are listed in
AUTHORS.txt, and in the relevant files.



# 3. REQUIREMENTS
#################

Megawave is expected to build and run on Linux, and maybe more POSIX
systems. It only works on a 32bit architecture. Running megawave
requires libtiff and libX11. Compiling megawave requires the
development parts of these libraries, plus make, gcc, and standard
unix utilities. gcc >= 4.3 may be required for code syntax check (see
below), but these check have been performed already, so an older
version of gcc is enough for compilation.

On Debian and Debian-based systems, you can install

* the packages required for compilation and test with
  `sudo aptitude install libtiff-dev libjpeg-dev libx11-dev make gcc bc`
* the packages required for execution with
  `sudo aptitude install libtiff libjpeg libx11`



# 4. COMPILATION
################

The compilation and compatibility status of megawave on various
platforms is tracked in the STATUS.txt file.


## BASIC

Prepare the source; this is only needed if you downloaded the source
directly from the git tree. It will generate some automatic source
and header files. If you downloaded a tarball, you don't need this
step.

Required tools: `make`, `cproto`, `gengetopt` and `csplit`.

    make prebuild

Compile everything: the `mwp` preprocessor, `libmw`,
`libmw-cmdline`, `libmw-io`, `libmw-modules`, and the executable
modules.

Required tools: `make`, `gcc`.

    make

Cleanup the compilation headers and objects.

    make clean

Cleanup everything, keep only the raw source.

    make distclean


## OPTIONS

Different options are available. Include these options in the
command-line to use them.

`MODE` : The compilation mode can be

* `normal`   : default standard options
* `opti`     : use optimisation options
* `profile`  : include profiling instructions, for use with gprof
* `coverage` : include code coverage instructions, for use with gcov
* `debug`    : include debugging symbols

`LINK` : The linking method can be

* `dynamic`  : default dynamic linking
* `static`   : link statically

`CHECK` : The code checking mode can be

* `relax`    : default ansi mode
* `strict`   : strict ansi confirmance with extra code quality check

`SYSTEM` : The sytem on the current machine

* `default`  : default generic settings
* `osx`	     : tweaks for Mac OSX
* `cygwin`   : tweaks for cygwin
* `debian3`  : tweaks for Debian 3.x

Example:

Build the modules with the `ccache` compiler wrapper, using strict
syntax check, with static linking and debugging symbols.

    make CHECK=strict LINK=static MODE=debug modules


## PARAMETERS

Different parameters can be changed from the command-line. Some of
them are explained hereafter; please refer to `common/makefile` for a
full list.

* `CC`     the compiler, defaults to /usr/bin/gcc
* `CCWRAP` an optional wrapper over the compiler; intended for tools
           such as `ccache` or `distcc`.
* `LD`     the linker, defaults to be the same as CC
* `LDWRAP` an optional wrapper over the linker; intended for tools
           such as `diet`.

Example:
Build the modules with the `gcc-3.4` compiler and `ccache`.

    make CC=gcc-3.4 CCWRAP=ccache modules


## CONFIGURATION

All the build process configuration lies in `common/makefile`.


## SUBFOLDERS

You can also invoke make directly from within the subfolders
`mwp`, `libmw3`, `libmw3-x11`, `libmw3-cmdline` and
`libmw3-modules`. The previous options are still valid, and some other
make targets are available. Please refer to `<subfolder>/README.txt` for
the details.

Some extra targets are available from the subfolders, mainly for
development purpose (code linting, syntax cleanup, ...). They are not
extensively tested.


## DOCUMENTATION

Compile the system and user documentation manuals. This target is not
thoroughly tested. These documentation manuals are also available as a
distinct archive to download.

Required tools: `pdflatex`, `hevea`, `hacha`, `html2text`.

    make doc

Compile the man pages. This target is not thoroughly tested.

Required tools: `pandoc`.

    make man

Compile source documentation (warning: needs time). This target is not
thoroughly tested.

Required tools: `doxygen`, `graphviz`, `pdflatex`.

    make srcdoc



# 5. INSTALLATION
#################

After a successful compilation, you can install the programs, scripts
and documents on your system.

## DESTINATION

The installation proceeds by copying the files to be installed into
subdirectories of a "destination directory" called DESTDIR, following
the file hierarchy standard:

* `$DESTDIR/include/`                 : development headers
* `$DESTDIR/lib/`                     : libraries (both static and dynamic) 
* `$DESTDIR/lib/megawave3/modules/`   : executable modules
* `$DESTDIR/share/doc/megawave3/`     : general documentation
* `$DESTDIR/share/man/`               : manual pages
* `$DESTDIR/share/megawave3/data/`    : example and utility data files
* `$DESTDIR/share/megawave3/scripts/` : convenience scripts

A wrapper script ans symlinks are also added to `$DESTDIR/bin/`.

The default value of `DESTDIR` is `/usr/local`, the usual destination
for non-packaged software. Installing there requires root access.

    make install

If you don't have root access, or if you don't want a system-wide
installation, you can override `DESTDIR` from the command-line.

    make DESTDIR=/path/to/your/destination install

An usual choice is a subfolder of your home directory, or a subfolder
of the `/opt` directory if you have root access:

    make DESTDIR=~/megawave3 install

or

    make DESTDIR=/opt/megawave3 install

Using an ad-hoc `DESTDIR` provides the extra advantage of making the
uninstallation easy by just deleting `DESTDIR`; if you install to a
system location (like `/usr` or `/usr/local`), knowing exactly which
files you can remove is can be tedious.

In this case, you will need to update your `PATH` variable if you want
to access the megawave programs directly; a post-install message will
detail the required settings.

In addition, you will need to add these options for any compilation
using the megawave headers or libraries:

    -I~/megawave3/include -L~/megawave3/lib

or

    -I/opt/megawave3/include -L/opt/megawave3/lib

## DOCUMENTATION

If you compiled the documentation (`make doc`, as explained before),
you can also install it in the default locations.

    make install-doc

The `DESTDIR` parameter discussion above still applies here.


# 6. CLEANUP
############

Two cleanup targets are possible.

* remove all the temporary files produced during the compilation
  process: `make clean`
* remove all the temporary files produced during the compilation
  process and the result of the compilation: `make distclean`

The installed files are not removed.


# 7. USAGE
##########

After the instalation step mentioned above, you should be able to use
megawave 3.02 on your system.

Running already compiled modules only requires to call them from the
command-line.

The convenience scripts found in megawave 3.01 or the concepts of
global/local modules are not available yet in this version.

For the moment, compiling a new module requires the following steps:

1. compile the module into an object

    cc -c module.c -o module.o

2. add it to your own modules archive

    ar ru libmw3-mymodules.a module.o

3. generate the command-line handler source

    mwp -S module.c -E module.cmd.c

4. compile the module command-line handler

    cc -c module.cmd.c -o module.cmd.o

5. statically link it into an executable prograqm

    cc module.cmd.o libmw-mymodules.a libmw-modules.a \
    libmw3.a libmw3-x11.a libmw3-cmdline.a -ltiff -ljpeg -lX11 -o module

The dynamic variant involves the following step:

5. dynamically it into an executable prograqm

    cc module.cmd.o libmw3-mymodules.a -lmw3-modules
    -lmw3 -lmw3-x11 -lmw3-cmdline -o module

Using dynamic links for the local modules is not recommended.

Note that such compilations now require the modules to be ANSI compliant
code, with all the required headers included.

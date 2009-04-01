#! /bin/sh
#
# command-line wrapper for megawave
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2009)

#
# SETTINGS
#

NAME=mw3
VERSION=0.01
AUTHOR="Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>"

# installation prefix, installation-dependant (set a install time)

PREFIX="%%DESTDIR%%"

# tools

DOCVIEWER=see

# megawave components path

LIB_PATH=${PREFIX}/lib
MODULES_PATH=${LIB_PATH}/megawave3/modules
SHARE_PATH=${PREFIX}/share
SCRIPTS_PATH=${SHARE_PATH}/megawave3/scripts
DATA_PATH=${SHARE_PATH}/megawave3/data
DOC_PATH=${SHARE_PATH}/doc/megawave3

#
# FUNCTIONS
#

error() {
    echo "$0: Command-line error. Use '$0 --help' for details."
}

usage() {
    cat <<EOF
$NAME $VERSION
execution wrapper for megawave

Usage:   $NAME <options>

Options: -h, --help                             show this usage information
         -v, --version                          show the version information
         -m, --run-module [module] [args]       run the given module
         -M, --show-modules                     list the available modules
         -s, --run-script [script] [args]       run the given script
         -S, --show-scripts                     list the available scripts
         -d, --documentation [document]         show the requested document
         -b, --build-module [module] [options]  build the module

This wrapper can also be called with different names through symbolic
links. In that case, it emulates the legacy interface available in
megawave 3.01, and can launch a megawave module (called by its name),
a megawave script (called by its name, lowercase, with the ".sh"
extension), the documentation viewer (called by mwdoc) or the
preprocessing and compilation of a module (called by cmw2).

This program is part of the megawave framework.
See http://megawave.cmla.ens-cachan.fr/ for details.
(C) 2008-2009 CMLA, ENS Cachan, 94235 Cachan cedex, France
EOF
}

show_doc() {
    if [ $# -lt 1 ]; then
        echo "$0: an option or command is required"
        exit 1
    fi

    case $1 in
        1)
            DOCFILE=${DOC_PATH}/user_manual_html/index.html
            ;;
        2)
            DOCFILE=${DOC_PATH}/system_manual_html/index.html
            ;;
        3)
            echo "$0: modules manual is not available yet"
            exit 1
            ;;
        *)
            echo "$0: module documentation not available yet"
            exit 1
            ;;
    esac

    exec $DOCVIEWER $DOCFILE
}

make_module() {
    echo "compiling $1"
}

#
# DO STUFF FROM NOW
#

# custom LD_LIBRARY_PATH is needed if the libraries are not in a
# default system location

if [ "${LIB_PATH}" != "/lib" \
   -a "${LIB_PATH}" != "/usr/lib" \
   -a "${LIB_PATH}" != "/usr/local/lib" ]; then
   export LD_LIBRARY_PATH=${LIB_PATH}:${LD_LIBRARY_PATH}
fi

# the name used to call this script (symbolic links detection)

CALLNAME=${0##*/}

# if the call name matches a module, run the module

if [ -x ${MODULES_PATH}/${CALLNAME} ]; then
   exec ${MODULES_PATH}/${CALLNAME} $*
fi

# if the call name matches a script, run the script
# to avoid ambiguity, scripts are supposed to be called with their
# full name, ie foo.sh (not foo)

if [ -f ${SCRIPTS_PATH}/${CALLNAME} \
    -a  -r ${SCRIPTS_PATH}/${CALLNAME} ]; then
   exec sh ${SCRIPTS_PATH}/${CALLNAME} $*
fi

# if the call name matches a built-in function, execute this function
# (legacy names are used)

case ${CALLNAME} in
    mwdoc)
	show_doc $*
	exit
	;;
    cmw2)
	make_module $*
	exit
	;;
esac

# else, try the default options

if [ $# -lt 1 ]; then
    error
    exit 1
fi

case $1 in
    -h|--help)
	usage
	exit 1
	;;
    -v|--version)
	echo "$NAME $VERSION"
	exit
	;;
    -m|--run-module)
	echo "not yet"
	exit
	;;
    -M|--show-modules)
	find ${MODULES_PATH} -type f \
	    | sed 's,.*/,,' | sort -u
	;;
    -s|--run-script)
	echo "not yet"
	exit
	;;
    -S|--show-scripts)
	find ${SCRIPTS_PATH} -type f -name "*.sh" \
	    | sed 's,.*/,,' | sort -u
	;;
    -d|--documentation)
	echo "not yet"
	exit
	;;
    -b|--build-module)
	echo "not yet"
	exit
	;;
    *)
	error
	exit 1;
	;;
esac

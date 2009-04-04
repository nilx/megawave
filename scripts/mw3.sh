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
BRIEF="command-line wrapper for megawave"
AUTHOR="Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>"
DATE=2008-2009

SYNOPSIS="\
        $NAME <options>"

OPTIONS="\
        -m, --run-module [module] [args]       run the given module
        -M, --show-modules                     list the available modules
        -s, --run-script [script] [args]       run the given script
        -S, --show-scripts                     list the available scripts
        -d, --documentation [document]         show the requested document
        -b, --build-module [module] [options]  build the module"

DESCRIPTION="\
This wrapper can also be called with different names through symbolic links.
In that case, it emulates the legacy interface available in megawave 3.01, and
can launch
* a megawave module (called by its name);
* a megawave script (called by its name, lowercase, with the '.sh' extension);
* the documentation viewer (called by 'mwdoc');
* the preprocessing and compilation of a module (called by 'cmw2')."

# path prefix, installation-dependant (set a install time)

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

# custom LD_LIBRARY_PATH is needed if the libraries are not in a
# default system location

if [ "${LIB_PATH}" != "/lib" \
   -a "${LIB_PATH}" != "/usr/lib" \
   -a "${LIB_PATH}" != "/usr/local/lib" ]; then
   export LD_LIBRARY_PATH=${LIB_PATH}:${LD_LIBRARY_PATH}
fi

# source common shell settings and functions

. ${SCRIPTS_PATH}/megawave3.shi

#
# FUNCTIONS
#

# the name used to call this script (symbolic links detection)
#TODO: use `basename` ? (better portability)
CALLNAME=${0##*/}

show_doc() {
#TODO: add alternative formats (mainly txt) 
    if [ $# -lt 1 ]; then
        echo "$CALLNAME: An option or command is required." \
	    "See 'mw3 --help' for details."
        exit 1
    fi

    if [ -r ${DOC_PATH}/$1.html ]; then
	DOCFILE=${DOC_PATH}/$1.html
	exec $DOCVIEWER $DOCFILE
    fi

    case $1 in
        1)
            DOCFILE=${DOC_PATH}/user_manual_html/index.html
            ;;
        2)
            DOCFILE=${DOC_PATH}/system_manual_html/index.html
            ;;
        3)
            echo "$CALLNAME: The modules manual is not available yet."
            exit 1
            ;;
        *)
            echo "$CALLNAME: The modules documentation not available yet."
            exit 1
            ;;
    esac
    if [ -r $DOCFILE ]; then
	exec $DOCVIEWER $DOCFILE
    else
        echo "$CALLNAME: The documentation file is not available " \
	    "or not readable."
        exit 1
    fi
}

build_module() {
    echo "compiling $1"
}

handle_callname() {

    # if the call name matches a module, run the module
    if [ -x ${MODULES_PATH}/${CALLNAME} ]; then
	exec ${MODULES_PATH}/${CALLNAME} $*
    fi

    # if the call name matches a script, run the script
    # to avoid ambiguity, scripts are supposed to be called with their
    # full name, ie foo.sh (not foo)
    if [ -r ${SCRIPTS_PATH}/${CALLNAME} ]; then
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
	    build_module $*
	    exit
	    ;;
    esac
}

#
# DO STUFF
#

# first, handle the case when this script is called with a known call name

handle_callname $*

# second, check the common options ...

base_options $*

# ... and handle local options

if [ $# -lt 1 ]; then
    error
    exit 1
fi

case $1 in
    -m|--run-module)
	shift
	MODULE=$1
	shift
	exec ${MODULES_PATH}/${MODULE} $*
	;;
    -M|--show-modules)
	find ${MODULES_PATH} -type f \
	    | sed 's,.*/,,' | sort -u
	;;
    -s|--run-script)
	shift
	SCRIPT=$1
	shift
	exec sh ${SCRIPTS_PATH}/${SCRIPT} $*
	;;
    -S|--show-scripts)
	find ${SCRIPTS_PATH} -type f -name "*.sh" \
	    | sed 's,.*/,,' | sort -u
	;;
    -d|--documentation)
	shift
	show_doc $*
	exit
	;;
    -b|--build-module)
	shift
	build_module $*
	exit
	;;
    *)
	error
	exit 1;
	;;
esac

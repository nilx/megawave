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
        -b, --build-module [options]  [module] build the module"

DESCRIPTION="\
This wrapper can also be called with different names through symbolic links.
In that case, it emulates the legacy interface available in megawave 3.01, and
can launch
* a megawave module (called by its name);
* a megawave script (called by its name, lowercase, with the '.sh' extension);
* the documentation viewer (called by 'mwdoc'); in this case, 'mwdoc'
  behaves like '$NAME -d';
* the preprocessing and compilation of a module (called by 'cmw2'); in
  this case, 'cmw2' behaves like '$NAME -b'."

# path prefix, installation-dependant (set a install time)

PREFIX="%%DESTDIR%%"

# the name used to call this script (symbolic links detection)
CALLNAME=$( basename $0 )

# source common shell settings and functions
# provides MW3_DOCVIEWER, MW3_DATAPATH, XXX_DIR

. %%DESTDIR%%/share/megawave3/scripts/megawave3.shi

# custom LD_LIBRARY_PATH is needed if the libraries are not in a
# default system location

if [ "$LIB_DIR" != "/lib" \
   -a "$LIB_DIR" != "/usr/lib" \
   -a "$LIB_DIR" != "/usr/local/lib" ]; then
   LD_LIBRARY_PATH=$LIB_DIR:$LD_LIBRARY_PATH
fi

#
# FUNCTIONS
#

# display the documentation, using $MW3_DOCVIEWER
# TODO: add alternative formats (mainly txt)
# $1: the document name
show_doc() {
    echo "$CALLNAME: Not available yet."
    exit 1

#     if [ $# -lt 1 ]; then
#         echo "$CALLNAME: An option or command is required." \
# 	    "See 'mw3 --help' for details."
#         exit 1
#     fi

#     base_options $*

#     if [ -r ${DOC_PATH}/$1.html ]; then
# 	DOCFILE=${DOC_PATH}/$1.html
# 	exec $MW3_DOCVIEWER $DOCFILE
#     fi

#     case $1 in
#         1)
#             DOCFILE=${DOC_PATH}/user_manual_html/index.html
#             ;;
#         2)
#             DOCFILE=${DOC_PATH}/system_manual_html/index.html
#             ;;
#         3)
#             echo "$CALLNAME: The modules manual is not available yet."
#             exit 1
#             ;;
#         *)
#             echo "$CALLNAME: The modules documentation not available yet."
#             exit 1
#             ;;
#     esac
#     if [ -r $DOCFILE ]; then
# 	exec $MW3_DOCVIEWER $DOCFILE
#     else
#         echo "$CALLNAME: The documentation file is not available " \
# 	    "or not readable."
#         exit 1
#     fi
}

# build a custom module
build_module() { 
    echo "$CALLNAME: Not available yet."
    exit 1

#     if [ $# -lt 1 ]; then
#         echo "$CALLNAME: An option or command is required." \
# 	    "See 'mw3 --help' for details."
#         exit 1
#     fi

#     base_options $*

#     echo "compiling $1"
}

# run a module
# $1 : the module name
# $2- : the options passed to the module
run_module() {
    MODULE=$1
    shift
    export MW3_DATAPATH
    export LD_LIBRARY_PATH
    exec $MODULES_DIR/$MODULE $*
}

# run a script
# $1 : the module name
# $2- : the options passed to the module
run_script() {
    SCRIPT=$1
    shift
    export LD_LIBRARY_PATH
    exec sh $SCRIPTS_DIR}/$SCRIPT $*
}

# handle the name used to call this script
# if it's a known name (module, script, mwdoc or cmw2, run these programs)
# $* : the original options
handle_callname() {
    # if the call name matches a module, run the module
    if [ -x $MODULES_DIR/$CALLNAME ]; then
	run_module $CALLNAME $*
    fi

    # if the call name matches a script, run the script
    # to avoid ambiguity, scripts are supposed to be called with their
    # full name, ie foo.sh (not foo)
    if [ -r $SCRIPTS_DIR/$CALLNAME ]; then
	run_script $CALLNAME $*
    fi

    # if the call name matches a built-in function, execute this function
    # (legacy names are used)
    # (dummy implementation for the moment)
    case $CALLNAME in
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

# from now, exit immediatly if a function exits with a non-zero exit code
set -e

# first, handle the case when this script is called with a known call name
handle_callname $*

# second, check the common options ...
base_options $*

# ... and handle local options
check_nb_params $# 1

#TODO: use getopts
case $1 in
    -m|--run-module)
	shift
	check_nb_params $# 1
	run_module $*
	;;
    -M|--show-modules)
	find ${MODULES_DIR} -type f \
	    | sed 's,.*/,,' | sort -u
	;;
    -s|--run-script)
	shift
	check_nb_params $# 1
	run_script $*
	;;
    -S|--show-scripts)
	find ${SCRIPTS_DIR} -type f -name "*.sh" \
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

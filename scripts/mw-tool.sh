#! /bin/sh

NAME=mw-tool
VERSION=0.01
AUTHOR="Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>"

# settings
BASEDIR=/home/nil/dev/mw/code/megawave.git
DOCVIEWER=see

version() {
    echo ${NAME} ${VERSION}
}

usage() {
    version
    cat <<EOF
Multiple tools wrapper for megawave.

Usage: ${NAME} [option]
       ${NAME} command [command-options]

This program performs various actions for megawave.

Author: ${AUTHOR}

Options:
  -h, --help              Print help and exit
  -V, --version           Print version and exit
  -l, --list-commands     Print the commands available

Commands:
  shell                   Open a sub-shell with the megawave
                          environment variables
  doc                     View the megawave documentation

This program is part of the megawave framework.
See http://megawave.cmla.ens-cachan.fr/ for details.
(C) 2008 CMLA, ENS Cachan, 94235 Cachan cedex, France
EOF
}

error() {
    echo "Command-line error. Use '--help' for details."
    exit 1
}

#
# shell
#

shell() {
    case `basename $SHELL` in
	bash)
	    NOINIT="--norc"
	    ;;
	csh | tcsh)
	    # FIXME: broken prompt setting
	    NOINIT="-f"
	    ;;
	dash)
	    NOINIT=""
	    ;;
	*)
	    NOINIT=""
    esac
    MW_LIBRARY_PATH=$BASEDIR/libmw:$BASEDIR/libmw-wdevice:$BASEDIR/modules
    MW_PATH=$BASEDIR/modules/build
    echo "# entering the megawave shell environment, using $SHELL"
    echo "# leave with 'exit' or Ctlr-D"
    env LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MW_LIBRARY_PATH \
	PATH=$PATH:$MW_PATH PS1="mw > " $SHELL $NOINIT
    echo "# quitting the megawave shell environment"
}

#
# doc
#

doc() {
    if [ $# -lt 1 ]; then
	echo "$0: an option or command is required"
	error
    fi
    
    case $1 in
	1)
	    DOCFILE=$DOCDIR/user/user_manual_html/index.html
	    ;;
	2)
	    DOCFILE=$DOCDIR/system/system_manual_html/index.html
	    ;;
	3)
	    echo "$0: modules manual is not available"
	    exit
	    ;;
	*)
	    echo "$0: bad parameter"
	    error
    esac

    $DOCVIEWER $DOCFILE
}

#
# main call
#

# no parameter
if [ $# -lt 1 ]; then
    echo "$0: an option or command is required"
    error
fi

# wrapper options
case $1 in
    -V|--version)
	version
	exit
	;;
    -h|--help)
	usage
	exit
	;;
    -l|--list-commands)
	list_commands
	exit
	;;
esac

# $1 exists and is not an option
ACTION=$1
shift

case $ACTION in
    shell)
	$ACTION $*
	exit
	;;
    *)
	echo "$0: unrecognized option or command"
	error
	;;
esac

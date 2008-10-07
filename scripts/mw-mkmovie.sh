#!/bin/sh 

# settings
BASEDIR=/home/nil/dev/mw/code/megawave.git
. $BASEDIR/scripts/mw-common.sh

NAME=mw-mkmovie
VERSION=1.2
AUTHOR="Lionel Moisan <lionel.moisan@math-info.univ-paris5.fr>"

USAGE="\n
Build a movie from an image sequence.\n
\n
Usage: ${NAME} type prefix n1 n2\n
\n
n1 and n2 must verufy n2 > n1 and n2 - n1 < 10000.\n
type must be Cmovie, Fmovie, Ccmovie or Cfmovie.\n
\n
Author: ${AUTHOR}\n
"

mkmovie() {
    echo > $DEST <<EOF
%
MegaWave2 - DATA ASCII file -
%
def $DEF

nimage:$N

EOF

    for N in `seq $N1 $N2`; do
	echo $PREFIX'_'$N >> $DEST
    done
}

#
# main call
#

# no option
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

# not enough parameters
if [ $# -lt 3 ]; then
    echo "$0: more parameters are required"
    error
fi

TYPE=$1
DEST=$2
PREFIX=`basename $DEST`
N1=$3
N2=$4

N=$(( $N2 - $N1 + 1))
if [ $N -le 0 ] || [ $N -gt 100000 ]; then
    echo "$0: bad interval"
    error
fi

if [ $TYPE != Cmovie ] && [ $TYPE != Fmovie ] \
    && [ $TYPE != Ccmovie ] && [ $TYPE != Cfmovie ]; then
    echo "$0: bad type"
    error
fi

mkmovie

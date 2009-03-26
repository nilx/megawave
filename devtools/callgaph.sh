#! /bin/sh
#
# builds
# * a map of the symbols and variables with ncc
# * a graph of the relevant function calls
# * a svg/png/... representation of the graph

set -x
set -e

LIBC_FUNCTIONS=$(grep -v "#" devtools/functions_libc.txt | tr "\n" " ")
LIBTIFF_FUNCTIONS=$(grep -v "#" devtools/functions_libtiff.txt | tr "\n" " ")
LIBJPEG_FUNCTIONS=$(grep -v "#" devtools/functions_libjpeg.txt | tr "\n" " ")
LIBX11_FUNCTIONS=$(grep -v "#" devtools/functions_libX11.txt | tr "\n" " ")
IGNORE_FUNCTIONS="$LIBC_FUNCTIONS _init _fini
    (*virtual __builtin_va_start __builtin_va_end"

NCC2DOT="python devtools/ncc2dot.py"
#FORMATS="png ps pdf"

FUNCTIONS_FILTEROUT="(_init|_fini|__)"
IGNORE_EXTRA_FUNCTIONS=""

DESTDIR=./doc/misc

makegraph() {
make CC="nccgen -ncgcc -ncld -ncfabs" AR=nccar lib$LIB
FUNCTIONS=$(nm -f posix build/lib/lib$LIB.a \
    | grep " T " | cut -d\  -f1 \
    | grep -v -E "$FUNCTIONS_FILTEROUT" | tr "\n" " ")
NCCOUT=build/lib/lib$LIB.a.nccout
$NCC2DOT -i "$IGNORE_FUNCTIONS $IGNORE_EXTRA_FUNCTIONS" \
    $NCCOUT $FUNCTIONS > $DESTDIR/lib$LIB.dot
dot -Tsvg -Gratio=0.75 $DESTDIR/lib$LIB.dot > $DESTDIR/lib$LIB.svg
for FMT in $FORMATS; do
    inkscape --export-$FMT=$DESTDIR/lib$LIB.$FMT $DESTDIR/lib$LIB.svg
done
}

# libmw-x11
LIB=mw-x11
IGNORE_EXTRA_FUNCTIONS="$LIBX11_FUNCTIONS"
makegraph

# libmw-cmdline
LIB=mw-cmdline
makegraph

# libmw
LIB=mw
FUNCTIONS_FILTEROUT="(_init|_fini|__|mwerror|mwdebug)"
IGNORE_EXTRA_FUNCTIONS="$LIBTIFF_FUNCTIONS $LIBJPEG_FUNCTIONS mwerror mwdebug"
makegraph

# libmw-modules
LIB=mw-modules
IGNORE_EXTRA_FUNCTIONS="mwerror mwdebug"
makegraph

find -name "*.nccout" -exec rm {} \;
#! /bin/sh
#
# library test for mwplight

error() {
    echo error on ${NAME}
    exit 1
}

for MODSRC in ${MODULES}; do
    NAME=`basename ${MODSRC} .c` || exit 1
    TMPFILE=/tmp/tmp.${NAME}_lib.c
    ${MWPLIGHT} -s ${MODSRC} -l ${TMPFILE} || exit 1
#TODO: -Wall -Wextra
    ccache gcc -c -ansi -pedantic -Werror -I${TESTDIR}/include \
	-o /tmp/tmp.library.o ${TMPFILE} \
	|| error
    rm -f ${TMPFILE}
    rm -f /tmp/tmp.library.o
    echo -n .
done
echo ok

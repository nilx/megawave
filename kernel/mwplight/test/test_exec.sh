#! /bin/sh
#
# exec test for mwplight

error() {
    echo error on ${NAME}
    exit 1
}

for MODSRC in ${MODULES}; do
    NAME=`basename ${MODSRC} .c` || exit 1
    TMPFILE=/tmp/tmp.${NAME}_exec.c
    ${MWPLIGHT} -s ${MODSRC} -e ${TMPFILE} || exit 1
#TODO: build a real executable against the lib
    ccache gcc -c -ansi -Werror -I${TESTDIR}/include \
	-o /tmp/tmp.exec.o ${TMPFILE} \
	|| error
    rm -f ${TMPFILE}
    rm -f /tmp/tmp.exec.o
    echo -n .
done
echo ok

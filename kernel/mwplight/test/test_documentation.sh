#! /bin/sh
#
# documentation test for mwplight

HEADER=${TESTDIR}/latex/header.tex
FOOTER=${TESTDIR}/latex/footer.tex

error() {
    echo error on ${NAME}
    exit 1
}

for MODSRC in ${MODULES}; do
    NAME=`basename ${MODSRC} .c` || exit 1
    TEXNAME=`echo ${NAME} | sed 's/_/\\\\_/g'` || exit 1
    TMPFILE=`mktemp` || exit 1
    ${MWPLIGHT} -s ${MODSRC} -d ${TMPFILE} || exit 1
    grep -F "\label{${NAME}}"              ${TMPFILE} > /dev/null \
	|| error
    grep -F "\index{${TEXNAME}@{\tt ${TEXNAME}}" ${TMPFILE} > /dev/null \
	|| error
    grep -F "\Name{${TEXNAME}}"              ${TMPFILE} > /dev/null \
	|| error
    grep -F "\Synopsis{${TEXNAME}}"          ${TMPFILE} > /dev/null \
	|| error
    grep -F "${TEXNAME} ("               ${TMPFILE} > /dev/null \
	|| error
    grep -F "\input{src/${NAME}.tex}"     ${TMPFILE} > /dev/null || error
    grep -F "\input{obj/DEPENDENCIES/${NAME}.dep}" ${TMPFILE} > /dev/null \
	|| error

    cat ${HEADER} ${TMPFILE} ${FOOTER} \
	| pdflatex -interaction errorstopmode -draftmode 1>/dev/null \
	|| error

    rm -f ${TMPFILE}
    rm -f texput.*
    echo -n .
done
echo ok

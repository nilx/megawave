#! /bin/sh
#
# name/group output test for mwplight

for MODSRC in ${MODULES}; do
    NAME=`basename ${MODSRC} .c` || exit 1
    OUT=`${MWPLIGHT} -s ${MODSRC} -n -` || exit 1
    if [ ${OUT} != "unknown/${NAME}" ]; then
	echo error : ${OUT} != "unknown/${NAME}"
	exit 1
    fi
    echo -n .
    OUT=`< ${MODSRC} ${MWPLIGHT} -m ${NAME} -n -` || exit 1
    if [ ${OUT} != "unknown/${NAME}" ]; then
	echo error : ${OUT} != "unknown/${NAME}"
	exit 1
    fi
    echo -n .
    GROUP=${NAME}
    OUT=`< ${MODSRC} ${MWPLIGHT} -m ${NAME} -g ${GROUP} -n -` || exit 1
    if [ ${OUT} != "${GROUP}/${NAME}" ]; then
	echo error : ${OUT} != "${GROUP}/${NAME}"
	exit 1
    fi
    echo -n .
done
echo ok

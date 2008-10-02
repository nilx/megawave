#! /bin/sh
#
# execution test for mwplight

for MODSRC in ${MODULES}; do
    NAME=`basename ${MODSRC} .c` || exit 1
    GROUP=foo
    ${MWPLIGHT} -s ${MODSRC} -l - -e - -d - -i - -n - > /dev/null || exit 1
    echo -n .
    < ${MODSRC} ${MWPLIGHT} -m ${NAME} -g ${GROUP} \
	-l - -e - -d - -i - -n - > /dev/null || exit 1
    echo -n .
done
echo ok

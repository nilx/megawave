#! /bin/sh
#
# automates the production and upload of src and rawsrc snapshots,
# archived into various formats; requires some MWDEV_XX environment
# variables

set -e
set -x

DATE=`date +%Y%m%d`
DEST=${MWDEV_SNAPSHOT_HOST}:${MWDEV_SNAPSHOT_PATH}

rm -rf ${MWDEV_SNAPSHOT_TMPDIR}
mkdir ${MWDEV_SNAPSHOT_TMPDIR}

git archive --format=tar --prefix=megawave_${DATE}/ \
    ${MWDEV_SNAPSHOT_BRANCH} \
    | (cd ${MWDEV_SNAPSHOT_TMPDIR} && tar xf - )

git log --no-color \
    > ${MWDEV_SNAPSHOT_TMPDIR}/megawave_${DATE}/CHANGES.gitlog.txt

cd ${MWDEV_SNAPSHOT_TMPDIR}/megawave_${DATE}

sloccount --addlangall \
    common mwp libmw libmw-x11 libmw-cmdline modules \
    > ../sloccount_${DATE}.txt

echo -e "\n\nDetails:\n" >> ../sloccount_${DATE}.txt

sloccount --addlangall --cached --details \
    common mwp libmw libmw-x11 libmw-cmdline modules \
    | sed "s,${MWDEV_SNAPSHOT_TMPDIR}/megawave_${DATE}/,," \
    >> ../sloccount_${DATE}.txt

cd ${MWDEV_SNAPSHOT_TMPDIR}

tar czf megawave_${DATE}_rawsrc.tgz megawave_${DATE}
tar cjf megawave_${DATE}_rawsrc.tbz megawave_${DATE}
tar cf - megawave_${DATE} | 7zr a -si -mx=9 megawave_${DATE}_rawsrc.t7z
zip -qr9 megawave_${DATE}_rawsrc.zip megawave_${DATE}

make -C megawave_${DATE} prebuild

tar czf megawave_${DATE}_src.tgz megawave_${DATE}
tar cjf megawave_${DATE}_src.tbz megawave_${DATE}
tar cf - megawave_${DATE} | 7zr a -si -mx=9 megawave_${DATE}_src.t7z
zip -qr9 megawave_${DATE}_src.zip megawave_${DATE}

mkdir upload
for EXT in t7z tbz tgz zip; do 
    for SRC in src rawsrc; do 
	mv megawave_${DATE}_${SRC}.${EXT} upload
	ln -s megawave_${DATE}_${SRC}.${EXT} \
	    upload/megawave_latest_${SRC}.${EXT};
    done;
done

mv sloccount_${DATE}.txt upload

rsync -av --rsh=ssh upload/ ${DEST}

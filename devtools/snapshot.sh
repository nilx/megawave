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

make -C ${MWDEV_SNAPSHOT_TMPDIR}/megawave_${DATE} prebuild

cd ${MWDEV_SNAPSHOT_TMPDIR}/megawave_${DATE}

sloccount --addlangall \
    common doc devtools mwp \
    libmw3 libmw3-x11 libmw3-cmdline libmw3-modules \
    > ../sloccount_${DATE}.txt

echo -e "\n\nDetails:\n" >> ../sloccount_${DATE}.txt

sloccount --addlangall --cached --details \
    common doc devtools mwp \
    libmw3 libmw3-x11 libmw3-cmdline libmw3-modules \
    | sed "s,${MWDEV_SNAPSHOT_TMPDIR}/megawave_${DATE}/,," \
    >> ../sloccount_${DATE}.txt

cd ${MWDEV_SNAPSHOT_TMPDIR}

tar czf megawave_${DATE}_src.tar.gz megawave_${DATE}
tar cjf megawave_${DATE}_src.tar.bz2 megawave_${DATE}
tar cf - megawave_${DATE} | 7zr a -si -mx=9 megawave_${DATE}_src.tar.lzma
zip -qr9 megawave_${DATE}_src.zip megawave_${DATE}

make -C megawave_${DATE}/doc
mkdir megawave_${DATE}_doc

for SECTION in system user; do
    DOC_FOLDER=megawave_${DATE}/doc/${SECTION}
    cp -r ${DOC_FOLDER}/${SECTION}_manual_html megawave_${DATE}_doc
    cp ${DOC_FOLDER}/${SECTION}_manual.html megawave_${DATE}_doc
    cp ${DOC_FOLDER}/${SECTION}_manual.pdf megawave_${DATE}_doc
    cp ${DOC_FOLDER}/${SECTION}_manual.txt megawave_${DATE}_doc
done

tar czf megawave_${DATE}_doc.tar.gz megawave_${DATE}_doc
tar cjf megawave_${DATE}_doc.tar.bz2 megawave_${DATE}_doc
tar cf - megawave_${DATE}_doc | 7zr a -si -mx=9 megawave_${DATE}_doc.tar.lzma
zip -qr9 megawave_${DATE}_doc.zip megawave_${DATE}_doc

mkdir upload
for EXT in tar.gz tar.bz2 tar.lzma zip; do 
    mv megawave_${DATE}_src.${EXT} upload
    ln -s megawave_${DATE}_src.${EXT} \
	upload/megawave_latest_src.${EXT};
    mv megawave_${DATE}_doc.${EXT} upload
    ln -s megawave_${DATE}_doc.${EXT} \
	upload/megawave_latest_doc.${EXT};
done

cp megawave_${DATE}/STATUS.txt upload
mv sloccount_${DATE}.txt upload

rsync -av --rsh=ssh upload/ ${DEST}

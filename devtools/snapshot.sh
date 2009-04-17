#! /bin/sh
#
# automates the production and upload of source and doc snapshots,
# archived into various formats; requires some MWDEV_XX environment
# variables

set -e
set -x

if [ $# -lt 1 ]; then
    TAG=`date -u +%Y%m%d`
else
    TAG="$1"
fi
DEST=$MWDEV_SNAPSHOT_HOST:$MWDEV_SNAPSHOT_PATH

rm -rf $MWDEV_SNAPSHOT_TMPDIR
mkdir $MWDEV_SNAPSHOT_TMPDIR

git archive --format=tar --prefix=megawave_$TAG/ $MWDEV_SNAPSHOT_BRANCH \
    | (cd $MWDEV_SNAPSHOT_TMPDIR && tar xf - )

git log --no-color \
    > $MWDEV_SNAPSHOT_TMPDIR/megawave_$TAG/CHANGES.gitlog.txt

make -C $MWDEV_SNAPSHOT_TMPDIR/megawave_$TAG prebuild

cd $MWDEV_SNAPSHOT_TMPDIR/megawave_$TAG

sloccount --addlangall \
    common doc devtools mwp \
    libmw3 libmw3-x11 libmw3-cmdline libmw3-modules \
    > ../sloccount_$TAG.txt

echo -e "\n\nDetails:\n" >> ../sloccount_$TAG.txt

sloccount --addlangall --cached --details \
    common doc devtools mwp \
    libmw3 libmw3-x11 libmw3-cmdline libmw3-modules \
    | sed "s,$MWDEV_SNAPSHOT_TMPDIR/megawave_$TAG/,," \
    >> ../sloccount_$TAG.txt

cd $MWDEV_SNAPSHOT_TMPDIR

tar cf - megawave_$TAG | gzip  -9 > megawave_${TAG}_src.tar.gz
tar cf - megawave_$TAG | bzip2 -9 > megawave_${TAG}_src.tar.bz2
tar cf - megawave_$TAG | lzma  -9 > megawave_${TAG}_src.tar.lzma
zip -qr9 megawave_${TAG}_src.zip megawave_$TAG

make -C megawave_$TAG/doc
mkdir megawave_${TAG}_doc

for SECTION in system user; do
    DOC_FOLDER=megawave_$TAG/doc/$SECTION
    cp -r $DOC_FOLDER/${SECTION}_manual_html megawave_${TAG}_doc
    cp $DOC_FOLDER/${SECTION}_manual.html megawave_${TAG}_doc
    cp $DOC_FOLDER/${SECTION}_manual.pdf megawave_${TAG}_doc
    cp $DOC_FOLDER/${SECTION}_manual.txt megawave_${TAG}_doc
done

tar cf - megawave_${TAG}_doc | gzip  -9 > megawave_${TAG}_doc.tar.gz
tar cf - megawave_${TAG}_doc | bzip2 -9 > megawave_${TAG}_doc.tar.bz2
tar cf - megawave_${TAG}_doc | lzma  -9 > megawave_${TAG}_doc.tar.lzma
zip -qr9 megawave_${TAG}_doc.zip megawave_${TAG}_doc

mkdir upload
for EXT in tar.gz tar.bz2 tar.lzma zip; do 
    mv megawave_${TAG}_src.$EXT upload
    ln -s megawave_${TAG}_src.$EXT \
	upload/megawave_latest_src.$EXT;
    mv megawave_${TAG}_doc.$EXT upload
    ln -s megawave_${TAG}_doc.$EXT \
	upload/megawave_latest_doc.$EXT;
done

cp megawave_$TAG/STATUS.txt upload
mv sloccount_$TAG.txt upload

rsync -av --rsh=ssh upload/ $DEST

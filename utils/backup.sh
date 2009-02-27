#! /bin/sh
#
# stupid resync backup one-liner; requires some MWDEV_XX environment
# variables


set -e
set -x

rsync -av --rsh=ssh ./ $MWDEV_BACKUP_HOST:$MWDEV_BACKUP_PATH

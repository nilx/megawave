#! /bin/sh
#
# simple LD_LIBRARY_PATH settings for dynamic execution of the modules
# this script fragment is supposed to be sourced, not executed

BASE_DIR=${MWDEV_WORK_DIR}

LD_LIBRARY_PATH_OLD=${LD_LIBRARY_PATH}
for LIBPATH in /modules /libmw-io /libmw-cmdline /libmw-x11 /libmw; do
    LD_LIBRARY_PATH=${BASE_DIR}${LIBPATH}:${LD_LIBRARY_PATH}
done
export LD_LIBRARY_PATH_OLD
export LD_LIBRARY_PATH

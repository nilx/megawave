#! /bin/sh
#
# simple LD_LIBRARY_PATH settings for dynamic execution of the modules
# this script fragment is supposed to be sourced, not executed

LD_LIBRARY_PATH=${LD_LIBRARY_PATH_OLD}
unset LD_LIBRARY_PATH_OLD
export LD_LIBRARY_PATH

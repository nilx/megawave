#!/bin/sh
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="CLEAR_BASE"
_Vers="1.0"
_Date="1996"
_Func="Clear the SR database directory (Shape Recognition Module)";
_Auth="Lionel Moisan";
_Usage=""

#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

#	input:		none
#	effect:		modify the $BASEDIR directory
#	output:		none
#
#       MegaWave2 modules used: none


#----- Check syntax -----
if [ $# -ne 0 ] 
then 
	. .mw2_help_lg_com
fi

#----- Check $MY_MEGAWAVE2/tmp directory -----
if [ ! -d $MY_MEGAWAVE2/tmp ]
then
	if mkdir $MY_MEGAWAVE2/tmp
	then
		echo $MY_MEGAWAVE2"/tmp directory created."
	else
		echo "Could not create $MY_MEGAWAVE2/tmp directory !"
		exit 1
	fi
fi		

#----- Check base directory -----
if [ -z "$BASEDIR" ] 
then
	BASEDIR=$MY_MEGAWAVE2/tmp/SR_BASE
	echo "No BASEDIR directory !"
	echo "-> Setting default value "$BASEDIR
fi
if [ ! -d $BASEDIR ]
then
	if mkdir $BASEDIR
	then
		echo "Base directory created."
	else
		echo "Could not create $BASEDIR directory !"
		exit 1
	fi
fi

rm -f $BASEDIR/*.crv $BASEDIR/references $BASEDIR/signature.fimg


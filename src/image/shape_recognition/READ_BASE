#!/bin/sh
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="READ_BASE"
_Vers="1.1"
_Date="1996"
_Func="Report informations about the SR database (Shape Recognition Module)";
_Auth="Lionel Moisan";
_Usage=""

#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

#	input:		none
#	effect:		print some parameters of the base
#	output:		none
#
#       MegaWave2 modules used: none


#----- Check syntax -----
if [ $# -ne 0 ] 
then
	. .mw2_help_lg_com
fi

#----- Check base directory -----
if [ -z "$BASEDIR" ] 
then
	echo "No BASEDIR directory ! Please set the BASEDIR variable."
	exit 1
fi
if [ ! -d $BASEDIR ]
then
	echo '$BASEDIR='$BASEDIR" is not a valid directory !"
	exit 1
fi

if [ -r $BASEDIR/references ]
then 
	echo "|------------------------------------------------------------"
	echo "| base  : "$BASEDIR
	echo "| size  : "`wc -l < $BASEDIR/references`" object(s)"
	echo "|------------------------------------------------------------"
	ls $BASEDIR | egrep -v 'references|signature'
	echo "|------------------------------------------------------------"
else
	echo "Base inconsistent or empty."
fi

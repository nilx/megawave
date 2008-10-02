#!/bin/sh
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="PUT_TO_BASE"
_Vers="1.0"
_Date="1996"
_Func="Put images in the SR database (Shape Recognition Module)";
_Auth="Lionel Moisan";
_Usage="<image 1> [ <image 2> ... <image n> ]"

#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

#	input:		images to put in the base
#	effect:		modifications in the $BASEDIR directory
#	output:		none
#
#       MegaWave2 modules used: extract_connex, 
#				sr_normalize, sr_signature

#----- Check syntax -----
if [ $# -lt 1  ] 
then
	. .mw2_help_lg_com
fi


#----- Check $MY_MEGAWAVE2/tmp directory -----
if [ ! -d $MY_MEGAWAVE2/tmp ]
then
	echo $MY_MEGAWAVE2"/tmp does not exist. Please call CLEAR_BASE."
	exit 1
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

#----- Main loop -----
while [ $# -ne 0 ]
do
	name=$1
	shift

	#----- Check the existence of the image file -----
	if [ ! -r $name ]
	then
		echo $name" : file not found or unreadable"
		exit 1
	fi

	nom=`basename $name | sed "s/.img//"`

	#----- Check that the image is not already in base  -----
	if [ -r $BASEDIR/references ] 
	then 
		if ( grep "$nom\," < $BASEDIR/references > /dev/null ) 
		then
			echo "*** "$nom" *** already exists in the base"
			break
		fi
	fi

	#----- Base completion -----

	tmp=$MY_MEGAWAVE2/tmp/toto.crv
	norm=$BASEDIR/$nom\.crv
	if extract_connex $name $tmp
	then
		sr_normalize $tmp $norm
		rm $tmp	
		if [ -w $BASEDIR/signature.fimg ]
		then 
			sr_signature -a $BASEDIR/signature.fimg $norm $BASEDIR/signature.fimg
		else 
			echo "Creating parameter file"
			sr_signature $norm $BASEDIR/signature.fimg
			touch $BASEDIR/references
		fi
		echo "Putting in base *** "$nom" ***"
		echo $nom $name >> $BASEDIR/references
	fi 
done

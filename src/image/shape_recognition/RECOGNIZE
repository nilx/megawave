#!/bin/sh
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="RECOGNIZE"
_Vers="1.0"
_Date="1996"
_Func="Recognize an image among the SR database (Shape Recognition Module)";
_Auth="Lionel Moisan";
_Usage="<image>"

#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

#	input:		the image to be identified
#	output:		the recognized image
#
#       MegaWave2 modules used: sr_genhypo, sr_distance


#----- Check syntax -----
if [ $# -ne 1 ] 
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

nom=$1

#----- Check the existence of the image file -----
if [ ! -r $nom ]

then
	echo $nom" : file not found or unreadable"
	exit 1
fi

#--- definition some functions to access the reference array
# N.B: all of this is evident in C-shell because of $a[..]
# but with the Bourne Shell it requires some work ...


name() {
if [ $1 != 1 ]
then
	ad1=`expr $1 - 1`
	ad2=`expr $1 + 1`
	sed -e '1,'$ad1'd' -e $ad2',$d' $BASEDIR/references | cut -f1 -d " "

else 
	sed -e 2',$d' $BASEDIR/references | cut -f1 -d " "
fi	
	 
}


path() {
if [ $1 != 1 ]
then
	ad1=`expr $1 - 1`
	ad2=`expr $1 + 1`
	sed -e '1,'$ad1'd' -e $ad2',$d' $BASEDIR/references | cut -f2 -d " "

else 
	sed -e 2',$d' $BASEDIR/references | cut -f2 -d " "
fi
}


#---------------------- produce the hypotheses list

echo "Producing hypotheses"

crvn=$MY_MEGAWAVE2/tmp/toto.crv
liste=`sr_genhypo $BASEDIR/signature.fimg $nom $crvn`

# NB: crvn is the normalized shape


#-------------------- compute the distances (to get the best hypothesis)

echo "Computing distances"

result=$MY_MEGAWAVE2/tmp/res
rm -f $result
touch $result

for hypo in $liste
do
	try=`name $hypo`

	val=`sr_distance $crvn $BASEDIR/$try.crv | cut -f2 -d "="`

	echo "Distance to" $try ":" $val

	echo $val $hypo >> $result
done

#--- Sort hypotheses according to the distances and select the best one

best=`sort -n $result`
i=`echo $best | cut -f2 -d " "`
p=`echo $best | cut -f1 -d " "`
q=`echo $p | cut -f1 -d "."`
if expr $q \< 10 > /dev/null
then
	echo "Best hypothesis : "`name $i`" ("`path $i`"), error = "$p" %"
else
	echo "Shape non recognized ! (minimum error = "$p" %)"
fi
rm -f $crvn $result

	





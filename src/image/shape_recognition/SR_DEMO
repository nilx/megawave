#!/bin/sh
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="SR_DEMO"
_Vers="1.1"
_Date="1996"
_Func="Shape Recognition demo macro";
_Auth="Lionel Moisan, Jacques Froment";
_Usage=""

#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

# Location where to put the base

if [ "$BASEDIR" = "" ]; then
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
  BASEDIR=$MY_MEGAWAVE2/tmp/SR_BASE
fi

# Location where to find the shape images

if [ -d $MEGAWAVE2/data/shape_recognition ]; then
  IMAGEDIR=$MEGAWAVE2/data/shape_recognition
else
  if [ -d $MY_MEGAWAVE2/data/shape_recognition ]; then
    IMAGEDIR=$MY_MEGAWAVE2/data/shape_recognition
  else
    echo "Don't know where to find the shape images"
    exit 1
  fi
fi

echo
echo "************************* Building base ************************"
export BASEDIR
CLEAR_BASE
PUT_TO_BASE $IMAGEDIR/f?.img 
READ_BASE

echo
echo "******************** Trying to recognize f2 ********************"
RECOGNIZE $IMAGEDIR/f2.img

echo
echo "*************** Trying to recognize rotated f1 *****************"
frot -ftype IMG -b 255 -a 30 $IMAGEDIR/f1.img $MY_MEGAWAVE2/tmp/f1.r30.img
cview -x 0 -y 0 $IMAGEDIR/f1.img&
cview -x 150 -y 0 $MY_MEGAWAVE2/tmp/f1.r30.img&
RECOGNIZE $MY_MEGAWAVE2/tmp/f1.r30.img
rm -f $MY_MEGAWAVE2/tmp/f1.r30.img

echo
echo "************* Trying to recognize noisy f9 **************"
fnoise -ftype IMG -g 50 $IMAGEDIR/f9.img $MY_MEGAWAVE2/tmp/f9_noisy.img
cview -x 0 -y 225 $IMAGEDIR/f9.img&
cview -x 100 -y 225 $MY_MEGAWAVE2/tmp/f9_noisy.img&
RECOGNIZE $MY_MEGAWAVE2/tmp/f9_noisy.img
rm -f $MY_MEGAWAVE2/tmp/f9_noisy.img

echo
echo "*************** Trying to recognize noisy rotated f4 *****************"
frot -ftype IMG -b 255 -a 10 $IMAGEDIR/f4.img $MY_MEGAWAVE2/tmp/f4.r10.img
fnoise -ftype IMG -g 50 $MY_MEGAWAVE2/tmp/f4.r10.img $MY_MEGAWAVE2/tmp/f4_noisy.r10.img
cview -x 0 -y 400 $IMAGEDIR/f4.img&
cview -x 110 -y 400 $MY_MEGAWAVE2/tmp/f4_noisy.r10.img&
RECOGNIZE $MY_MEGAWAVE2/tmp/f4_noisy.r10.img
rm -f $MY_MEGAWAVE2/tmp/f4.*.img

echo
echo "************ Trying to recognize an image not in base ************"
RECOGNIZE $IMAGEDIR/f10.img
echo










#!/bin/sh
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="KM_DEMO"
_Vers="1.0"
_Date="2002"
_Func="Demo for Curve Matching (km_*) algorithms";
_Auth="Jose-Luis Lisani, Pablo Muse, Frederic Sur";
_Usage=""

#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

# Recognition of pieces of level lines: recognition demo
# You can see AI-modules instead of SI-modules (please modify this file)
# 

# Location where to put the data

if [ "$KM_DEMO_DIR" = "" ]; then
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
  KM_DEMO_DIR=$MY_MEGAWAVE2/tmp
fi

# Location where to find the images

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


IMAGE1=$IMAGEDIR/f2.img
IMAGE2=$IMAGEDIR/formes.gif

echo
echo "******* Extracting Meaningful Boundaries (ll_boundaries) *******"
eps=-5
ll_boundaries -e $eps $IMAGE1 $KM_DEMO_DIR/image1mb
ll_boundaries -e $eps $IMAGE2 $KM_DEMO_DIR/image2mb

echo
echo "****************** Smoothing Curves (gass) *********************"
gass -n 10 $KM_DEMO_DIR/image1mb $KM_DEMO_DIR/image1ll
gass -n 10 $KM_DEMO_DIR/image2mb $KM_DEMO_DIR/image2ll

echo
echo "********* Creating dictionaries (km_createdict_si) *************"
km_createdict_si $KM_DEMO_DIR/image1ll $KM_DEMO_DIR/image1dictSI
km_createdict_si $KM_DEMO_DIR/image2ll $KM_DEMO_DIR/image2dictSI

echo
echo "********** Matching pieces of curves (km_match_si) *************"
km_match_si 0.3 2 30 3.14 $KM_DEMO_DIR/image1ll $KM_DEMO_DIR/image2ll $KM_DEMO_DIR/image1dictSI $KM_DEMO_DIR/image2dictSI $KM_DEMO_DIR/matchings $KM_DEMO_DIR/matching_pieces

echo
echo "********* Visualization (km_savematchings & fkview) ************"
km_savematchings $KM_DEMO_DIR/matching_pieces $KM_DEMO_DIR/image1ll $KM_DEMO_DIR/image2ll $KM_DEMO_DIR/matching_lines1 $KM_DEMO_DIR/matching_lines2 $KM_DEMO_DIR/matching_pieces1 $KM_DEMO_DIR/matching_pieces2
fkview -c 900 -s -b $IMAGE1 $KM_DEMO_DIR/matching_lines1 &
fkview -c 900 -s -b $IMAGE2 $KM_DEMO_DIR/matching_lines2 &
fkview -c 900 -s -b $IMAGE1 $KM_DEMO_DIR/matching_pieces1 &
fkview -c 900 -s -b $IMAGE2 $KM_DEMO_DIR/matching_pieces2 &

rm -f $KM_DEMO_DIR/image1ll 
rm -f $KM_DEMO_DIR/image2ll 
rm -f $KM_DEMO_DIR/image1mb 
rm -f $KM_DEMO_DIR/image2mb 
rm -f $KM_DEMO_DIR/image1dictSI 
rm -f $KM_DEMO_DIR/image2dictSI 
rm -f $KM_DEMO_DIR/matchings 
rm -f $KM_DEMO_DIR/matching_pieces
rm -f $KM_DEMO_DIR/matching_pieces1
rm -f $KM_DEMO_DIR/matching_pieces2
rm -f $KM_DEMO_DIR/matching_lines1
rm -f $KM_DEMO_DIR/matching_lines2

echo

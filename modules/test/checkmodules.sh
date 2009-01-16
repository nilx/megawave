#!/bin/sh 
#
# Testsuite for the modules
#
# Set $PATH, $SAMPLES, $SCRIPTS and $LD_LIBRARY_PATH according to your
# locations.
#
# example : 
# MWBASE=~/megawave
# PATH=$MWBASE/bin
# SCRIPTS=$MWBASE/scripts
# SAMPLES=$MWBASE/data
# LD_LIBRARY_PATH=$MWBASE/lib
#

usage() {
    echo "megawave_checkmodules (no option)"
    echo "input:  none"
    echo "output: unittests results"
}

if [ $# -ne 0 ]; then
    usage
    exit 1
fi

# used for integers
exact() {
    if [ "$1" = "$2" ]; then
	return 0
    else
	return 1
    fi
}

# used for float numbers (threshold at 1% error)
approx() {
    X=`echo "a=$1; b=$2; c=(a-b)^2; d=0.0001*(a^2+b^2); if (c<=d) 1" \
	| bc -l 2> /dev/null`
    if [ "$X" = "1" ]; then
	return 0
    else
	return 1
    fi
}

# used for random variables (threshold at 10% error)
rand_approx() {
    X=`echo "a=$1; b=$2; c=(a-b)^2; d=0.01*(a^2+b^2); if (c<=d) 1" \
	| bc -l 2> /dev/null`
    if [ "$X" = "1" ]; then
	return 0
    else
	return 1
    fi
}

fail() {
    FAIL=$(($FAIL + 1))
    echo -n "X"
    if [ $# -ne 0 ]; then
	FAILED="$FAILED $1"
    else
	FAILED="$FAILED ..."
    fi
}
    
pass() {
    OK=$(($OK + 1))
    echo -n "."
}

results() {
    echo "test completed: $OK success, $FAIL failures"
    if [ "x$FAILED" != "x" ]; then
	echo "failed modules:$FAILED"
    fi
    rm -rf $TMP
    rm -f $HISTFILE
    exit $FAIL
}

OK=0
FAIL=0
FAILED=""
TMP=/tmp/megawave_checkmodules.$$.tmp
mkdir $TMP

HISTFILE=`tempfile`

# check modules
echo "checking megawave modules"

# common modules
echo -n 'common modules: '

fmean $SAMPLES/images/cimage > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && approx $VAL 127.156 \
    && pass || fail fmean

fsize $SAMPLES/images/cimage > $TMP/1 \
    && VAL=`cat $TMP/1` \
    && exact "$VAL" "256 256" \
    && pass || fail fsize

fnorm -v $SAMPLES/images/cimage > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && approx $VAL 8.73267 \
    && pass || fail fnorm

funzoom -z 4 -o 0 $SAMPLES/images/cimage $TMP/1 > /dev/null \
    && VAL=`fnorm -v $TMP/1  | cut -d"=" -f2` \
    && approx $VAL 16.5741 \
    && pass || fail funzoom

cfunzoom -z 4 -o 0 $SAMPLES/images/ccimage $TMP/1 \
    && cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && fdiff $TMP/2 $TMP/3 $TMP/4 \
    && VAL=`fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 12.6513 \
    && pass || fail cfunzoom

cfgetchannels $SAMPLES/images/ccimage \
    $TMP/1 $TMP/2 $TMP/3 > /dev/null \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 14.6397 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 17.8047 \
    && VAL=`fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 18.248 \
    && pass || fail cfgetchannels

fdiff $SAMPLES/images/cimage $SAMPLES/images/fimage $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 15.9042 \
    && pass || fail fdiff

cfdiff $SAMPLES/images/ccimage $SAMPLES/images/cimage $TMP/1 \
    && cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 18.6356 \
    && VAL=`fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 21.3019 \
    && VAL=`fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 21.7275 \
    && pass || fail cfdiff

fadd $SAMPLES/images/cimage $SAMPLES/images/cimage $TMP/1 \
    && fdiff $TMP/1 $SAMPLES/images/cimage $TMP/2 \
    && VAL=`fnorm -p 2 -c $SAMPLES/images/cimage $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail fadd

fquant $SAMPLES/images/cimage $TMP/1 5 > /dev/null \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.42627 \
    && pass || fail fquant

echo "1 5 2 3" | freadasc $TMP/1 2 2 \
    && VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.75 \
    && VAL=`fvar $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.91667 \
    && pass || fail freadasc

snorm -v $SAMPLES/signals/fsignal > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && approx $VAL 37.827 \
    && pass || fail snorm

ccopy $SAMPLES/images/cimage $TMP/1_1 \
    && ccopy $SAMPLES/images/fimage $TMP/1_2 \
    && sh $SCRIPTS/mw-mkmovie.sh Cmovie $TMP/1 1 2 \
    && VAL=`grep nimage $TMP/1 | cut -d":" -f2` \
    && exact $VAL 2 \
    && pass || fail ccopy

fconst $TMP/1 10 100 100 \
    && VAL=`fnorm -p 1 $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 10 \
    && pass || fail fconst

VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 10 \
    && pass || fail fmean

dkinfo $SAMPLES/curves/curve > $TMP/1 \
    && VAL=`grep "Average step distance" $TMP/1 | cut -d":" -f2` \
    && approx $VAL 2.04765 \
    && pass || fail dkinfo

faxpb -a 2 -b 10 $SAMPLES/images/cimage $TMP/1 \
    && faxpb -a 0.5 -b -5 $TMP/1 $TMP/2 \
    && fdiff $TMP/2 $SAMPLES/images/cimage $TMP/3 \
    && VAL=`fnorm -p 2 $TMP/3 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail faxpb

fconst $TMP/1 0 10 10 \
    && fpset $TMP/1 5 5 100 $TMP/1 || fail \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2 \
    && pass || fail fconst

echo

# compression/ezwave
echo -n "compression/ezwave: "

cfunzoom -z 4 -o 0 $SAMPLES/images/ccimage $TMP/1

cfezw -R 0.5 -o $TMP/3 \
    $TMP/1 $DATA/wave/biortho/h/sd07.ir $TMP/2 > /dev/null \
    && VAL=`fnorm -v $TMP/2 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 11.0831 \
    && pass || fail cfezw

cfiezw $TMP/3 $DATA/wave/biortho/h/sd07.ir $TMP/4 > /dev/null \
    && VAL=`fnorm -p 2 -c $TMP/4 $TMP/2 2> /dev/null | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail cfiezw

env DATA=$DATA sh $SCRIPTS/mw-cfezw.sh $TMP/1 > $TMP/2 \
    && VAL=`tail -1 $TMP/2 | cut -f2` \
    && approx $VAL 31.18 \
    && pass || fail mw-cfezw

funzoom -z 4 -o 0 $SAMPLES/images/cimage $TMP/1

fezw -R 0.5 -o $TMP/3 \
    $TMP/1 $DATA/wave/biortho/h/sd07.ir $TMP/2 > /dev/null \
    && VAL=`fnorm -v $TMP/2 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 18.1235 \
    && pass || fail fezw

fiezw $TMP/3 $DATA/wave/biortho/h/sd07.ir $TMP/4 > /dev/null \
    && VAL=`fnorm -p 2 -c $TMP/4 $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail fiezw

env DATA=$DATA sh $SCRIPTS/mw-fezw.sh $TMP/1 > $TMP/2 \
    && VAL=`tail -1 $TMP/2 | cut -f2` \
    && approx $VAL 33.62 \
    && pass || fail mw-fezw

echo

# compression/lossless
echo -n "compression/lossless: "

funzoom -ftype IMG -z 4 -o 0 $SAMPLES/images/cimage $TMP/1 2> /dev/null \
    && arencode2 $TMP/1 $TMP/2 > $TMP/3 2> /dev/null \
    && VAL=`grep Rate $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 7.57324 \
    && VAL=`fsize $TMP/2` \
    && exact "$VAL" "3878 1" \
    && pass || fail arencode2

arencode2 -H $TMP/1 $TMP/2 > /dev/null 2> /dev/null\
    && ardecode2 -r 256 $TMP/2 $TMP/3 > /dev/null \
    && pass || fail ardecode2

cvsencode $SAMPLES/curves/france.crv > $TMP/1 \
    && VAL=`grep N $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 7887 \
    && VAL=`grep B $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 10098 \
    && pass || fail cvsencode

cvsfrecode $SAMPLES/curves/france.crv > $TMP/1 \
    && VAL=`grep N $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 7887 \
    && VAL=`grep B $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 10098 \
    && pass || fail cvsfrecode

cvsorgcode $SAMPLES/curves/france.crv > $TMP/1 \
    && VAL=`grep N $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 7887 \
    && pass || fail cvsorgcode

funzoom -ftype IMG -z 4 -o 0 $SAMPLES/images/cimage $TMP/1 2> /dev/null \
    && fencode $TMP/1 > $TMP/2 2> /dev/null \
    && VAL=`grep brate $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 7.34399 \
    && pass || fail fencode

echo

# compression/scalar
echo -n "compression/scalar: "

fscalq -p -n 10 $SAMPLES/images/cimage $TMP/1 > $TMP/2 \
    && VAL=`cat $TMP/2` \
    && VAL=`fentropy $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.91987 \
    && pass || fail fscalq

fscalq -p -n 10 -o $TMP/2 $SAMPLES/images/cimage $TMP/1 > /dev/null \
    && fiscalq $TMP/2 $TMP/3 > /dev/null \
    && fdiff $TMP/1 $TMP/3 $TMP/2 \
    && VAL=`fnorm -p 2 $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail fiscalq

echo

# compression/vector
echo -n "compression/vector: "

mk_codebook $TMP/1 \
    && VAL=`fsize $TMP/1` \
    && exact "$VAL" "4 6" \
    && pass || fail mk_codebook

# TODO
# fivq
# flbg_adap
# flbg
# flbg_train
# fvq
# mk_trainset

echo

# compression/vqwave

echo -n "compression/vqwave: "

# TODO
# Cfwivq
# Cfwvq
# Fwivq
# fwivq
# Fwlbg_adap
# fwlbg_adap
# Fwvq
# fwvq
# wlbg_adap

echo

# $SAMPLES/curves/curve

echo -n "curve: "

area $SAMPLES/curves/curve > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && exact $VAL 209718 \
    && pass || fail area

perimeter $SAMPLES/curves/curve > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && exact $VAL 8422 \
    && pass || fail perimeter

circle -r 10 -n 100 $TMP/1 \
    && VAL=`area $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 313.953 \
    && VAL=`perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 62.8215 \
    && pass || fail circle

disc $TMP/1 2.5 \
    && VAL=`perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 30.5708 \
    && pass || fail disc

dsplit_convex -c . $SAMPLES/curves/curve $TMP/1 > $TMP/2 \
    && VAL=`cut -d"=" -f2 $TMP/2` \
    && exact $VAL 1866 \
    && pass || fail dsplit_convex

fsplit_convex -c . $SAMPLES/curves/curve $TMP/1 > $TMP/2 \
    && VAL=`cut -d"=" -f2 $TMP/2` \
    && exact $VAL 1866 \
    && pass || fail fsplit_convex

extract_connex $SAMPLES/images/cimage $TMP/1 \
    && VAL=`dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 122 \
    && pass || fail extract_connex

fillpoly -x 1100 -y 1100 $SAMPLES/curves/france.crv $TMP/1 \
    && VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 136.473 \
    && pass || fail fillpoly

echo "10 10 60 20 30 50 e 100 10 110 50 150 20 q" \
    | flreadasc 2 $TMP/1 > /dev/null \
    && fillpolys -x 160 -y 60 $TMP/1 $TMP/1 \
    && VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 201.423 \
    && pass || fail fillpolys

fkbox $SAMPLES/curves/curve > $TMP/1 \
    && VAL=`grep xmin $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 63 \
    && VAL=`grep ymin $TMP/1 | cut -d"=" -f2` \
    && exact $VAL -912 \
    && VAL=`grep xmax $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 692 \
    && VAL=`grep ymax $TMP/1 | cut -d"=" -f2` \
    && exact $VAL -123 \
    && pass || fail fkbox

fkcenter $SAMPLES/curves/curve > $TMP/1 \
    && VAL=`grep xg $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 336.298 \
    && VAL=`grep yg $TMP/1 | cut -d"=" -f2` \
    && approx $VAL -467.331 \
    && pass || fail fkcenter

fkcrop 0 -100 700 -200 $SAMPLES/curves/curve $TMP/1 \
    && VAL=`dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 5 \
    && VAL=`fkcenter $TMP/1 | grep xg | cut -d"=" -f2` \
    && approx $VAL 324.922 \
    && pass || fail fkcrop

fkzrt $SAMPLES/curves/curve  $TMP/1 0.5 30 20 20 \
    && VAL=`fkcenter $TMP/1 | grep xg | cut -d"=" -f2` \
    && approx $VAL 282.455 \
    && pass || fail fkzrt

echo "40 40 60 30 100 100 200 80 80 240 e 240 20 280 20 260 240 200
150 240 40 q" \
    | fkreadasc $TMP/1 > /dev/null \
    && ksplines -j 3 $TMP/1 $TMP/2 \
    && VAL=`dkinfo $TMP/2 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 2 \
    && pass || fail ksplines

echo "40 40 60 30 100 100 200 80 80 240 q" \
    | fkreadasc $TMP/1 > /dev/null \
    && kspline -j 3 $TMP/1 $TMP/2 \
    && VAL=`perimeter $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 318.806 \
    && pass || fail kspline

echo "0.1 0.2" | sreadasc $TMP/1 2 \
    && flscale -ftype MW2_FCURVE $SAMPLES/curves/curve $TMP/1 $TMP/2 \
    && perimeter $TMP/2 > $TMP/3 \
    && VAL=`cut -d"=" -f2 $TMP/3` \
    && exact $VAL 1313 \
    && area $TMP/2 > $TMP/3 \
    && VAL=`cut -d"=" -f2 $TMP/3` \
    && exact $VAL 4194.36 \
    && pass || fail flscale

flconcat -ftype MW2_FCURVES $TMP/2 $SAMPLES/curves/curve $TMP/3 \
    && fkbox $TMP/3 | grep ymax > $TMP/4 \
    && VAL=`cut -d"=" -f2 $TMP/4` \
    && exact $VAL -24.6 \
    && pass || fail flconcat

echo

# $SAMPLES/curves/curve/io
echo -n "curve/io: "

echo "10 10 60 20 30 50 e 40 30 41 31 41 20 q" \
    | fkreadasc $TMP/1 > /dev/null \
    && VAL=`dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 2 \
    && VAL=`perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 93.4166 \
    && pass || fail fkreadasc

echo "10 10 60 20 30 50 e 40 30 41 31 41 20 q" \
    | flreadasc 2 $TMP/1 > /dev/null \
    && VAL=`dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 2 \
    && VAL=`perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 93.4166 \
    && pass || fail flreadasc

kplot $SAMPLES/curves/curve $TMP/1 \
    && VAL=`fsize $TMP/1` \
    && exact "$VAL" "630 790" \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 1.90955 \
    && pass || fail kplot

fkplot -s $SAMPLES/curves/curve $TMP/1 \
    && VAL=`fsize $TMP/1` \
    && exact "$VAL" "630 790" \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 1.90955 \
    && pass || fail fkplot

fkprintasc $SAMPLES/curves/curve > /dev/null \
    && VAL=`fkprintasc $SAMPLES/curves/curve | wc -l` \
    && exact $VAL 4115 \
    && pass || fail fkprintasc

flprintasc $SAMPLES/curves/curve > /dev/null \
    && VAL=`flprintasc $SAMPLES/curves/curve | wc -l` \
    && exact $VAL 4115 \
    && pass || fail flprintasc

fkprintfig $SAMPLES/curves/curve > /dev/null \
    && VAL=`fkprintfig $SAMPLES/curves/curve | wc -w` \
    && exact $VAL 8255 \
    && pass || fail fkprintfig

echo "10 10 60 20 30 50 e 40 30 41 31 41 20 q" \
    | flreadasc 2 $TMP/1 > /dev/null \
    && fkprintfig $TMP/1 | kreadfig $TMP/2 > /dev/null \
    && fkzrt $TMP/2 $TMP/2 0.001 0 0 0 \
    && VAL=`dkinfo $TMP/2 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 2 \
    && VAL=`area $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 3600 \
    && pass || fail fkzrt

fkview -o $TMP/1 -n $SAMPLES/curves/curve \
    && VAL=`fnorm -v $TMP/1 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 2.61157 \
    && pass || fail fkview

echo "1 1 6 2 3 5 4 3 4 4 4 2 q" | fkreadasc $TMP/1 > /dev/null \
    && kplotasc $TMP/1 > /dev/null \
    && VAL=`kplotasc $TMP/1 | wc -c` \
    && exact $VAL 84 \
    && pass || fail kplotasc

# TODO
# readpoly
# dkinfo

echo

# curve/smooth
echo -n "curve/smooth: "

fksmooth -n 20 -s 10 $SAMPLES/curves/curve $TMP/1 \
    && VAL=`area $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 204306 \
    && VAL=`perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3128.84 \
    && pass || fail fksmooth

iter_fksmooth -N 2 -n 10 -s 10 $SAMPLES/curves/curve $TMP/1 \
    && VAL=`dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 3 \
    && pass || fail iter_fksmooth

gass -l 10 $SAMPLES/curves/curve $TMP/1 \
    && VAL=`area $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 207081 \
    && VAL=`perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3914.79 \
    && pass || fail gass

iter_gass -N 2 -S 10 $SAMPLES/curves/curve $TMP/1 \
    && VAL=`dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 3 \
    && pass || fail iter_gass

gcsf -l 10 $SAMPLES/curves/curve $TMP/1 \
    && VAL=`area $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 209003 \
    && VAL=`perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3929.08 \
    && pass || fail gcsf

iter_gcsf -N 2 -l 20 $SAMPLES/curves/curve $TMP/1 \
    && VAL=`dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 3 \
    && pass || fail iter_gcsf

echo

# $SAMPLES/curves/curve/matching
echo -n "curve/matching: "

gass -l 3 -e 2 $SAMPLES/curves/curve $TMP/1 \
    && km_inflexionpoints $TMP/1 $TMP/2 \
    && VAL=`flprintasc $TMP/2 | wc -l` \
    && exact $VAL 43 \
    && pass || fail  km_inflexionpoints

km_bitangents $TMP/1 $TMP/2 $TMP/3 \
    && VAL=`flprintasc $TMP/3 | wc -l` \
    && exact $VAL 20 \
    && pass || fail km_bitangents

km_flatpoints $TMP/1 $TMP/2 $TMP/4 0.1 0.1 \
    && VAL=`flprintasc $TMP/4 | wc -l` \
    && exact $VAL 25 \
    && pass || fail km_flatpoints

# TODO
# km_codecurve_ai
# km_codecurve_si
# km_createdict_ai
# km_createdict_si
# KM_DEMO
# km_match_ai
# km_match_si
# km_prematchings
# km_savematchings

echo

# examples
echo -n "examples: "

# FIXME
#demohead1 $SAMPLES/images/cimage $TMP/1 > /dev/null \
#    && VAL=`fsize $TMP/1` \
#    && exact "$VAL" "1 1" \
#    && pass || fail

#demohead2 > /dev/null \
#    && pass || fail

#demohead3 $SAMPLES/images/cimage $TMP/1 > /dev/null \
#    && VAL=`fnorm -p 2 -c $SAMPLES/images/cimage $TMP/1 | cut -d"=" -f2` \
#    && exact $VAL 0 \
#    && pass || fail

#fadd $SAMPLES/images/cimage $SAMPLES/images/fimage $TMP/1 \
#    && fadd1 $SAMPLES/images/cimage $SAMPLES/images/fimage $TMP/2 \
#    && VAL=`fnorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
#    && exact $VAL 0 \
#    && pass || fail

#fadd2 $SAMPLES/images/cimage $SAMPLES/images/fimage $TMP/2 \
#    && VAL=`fnorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
#    && exact $VAL 0 \
#    && pass || fail

#fadd3 $SAMPLES/images/cimage $SAMPLES/images/fimage $TMP/2 \
#    && VAL=`fnorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
#    && exact $VAL 0 \
#    && pass || fail

#fadd4 $SAMPLES/images/cimage $SAMPLES/images/fimage $TMP/2 \
#    && VAL=`fnorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
#    && exact $VAL 0 \
#    && pass || fail

#make_cmovie $TMP/1 \
#    && VAL=`fsize $TMP/1_01` \
#    && exact "$VAL" "256 256" \
#    && VAL=`grep nimage $TMP/1 | cut -d":" -f2` \
#    && exact $VAL 20 \
#    && pass || fail

#make_fmovie $TMP/1 \
#    && VAL=`fsize $TMP/1_01` \
#    && exact "$VAL" "256 256" \
#    && VAL=`grep nimage $TMP/1 | cut -d":" -f2` \
#    && exact $VAL 21 \
#    && pass || fail

#make_ccmovie $TMP/1 \
#    && ccopy $TMP/1_01 $TMP/2 2> /dev/null \
#    && VAL=`fsize $TMP/2` \
#    && exact "$VAL" "256 256" \
#    && VAL=`grep nimage $TMP/1 | cut -d":" -f2` \
#    && exact $VAL 20 \
#    && pass || fail

#make_cfmovie $TMP/1 \
#    && ccopy $TMP/1_01 $TMP/2 2> /dev/null \
#    && VAL=`fsize $TMP/2` \
#    && exact "$VAL" "256 256" \
#    && VAL=`grep nimage $TMP/1 | cut -d":" -f2` \
#    && exact $VAL 20 \
#    && pass || fail

#make_cimage $TMP/1 \
#    && VAL=`fsize $TMP/1` \
#    && exact "$VAL" "256 256" \
#    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
#    && approx $VAL 0.992188 \
#    && pass || fail

# TODO
# view_demo

echo

# image/detection
echo -n "image/detection: "

funzoom -o 0 -z 4 $SAMPLES/images/cimage $TMP/1

canny $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.5183 \
    && pass || fail canny

falign -e 4.3 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`flprintasc $TMP/2 | wc -l` \
    && exact $VAL 14 \
    && pass || fail falign

falign_mdl -e -2 -l 1 -n 30 $SAMPLES/images/cimage $TMP/2 > /dev/null \
    && VAL=`fsize $TMP/2` \
    && exact "$VAL" "6 43" \
    && pass || fail falign_mdl

vpoint $SAMPLES/images/cimage $TMP/2 $TMP/3 > /dev/null \
    && VAL=`flprintasc $TMP/3 | cut -d" " -f2` \
    && approx $VAL 17.9527 \
    && pass || fail vpoint

ll_boundaries -e 11 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`flprintasc $TMP/2 | wc -l` \
    && exact $VAL 1139 \
    && pass || fail ll_boundaries

ll_boundaries2 -e 11 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`flprintasc $TMP/2 | wc -l` \
    && exact $VAL 1073 \
    && pass || fail ll_boundaries2

ll_edges -e 17 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`flprintasc $TMP/2 | wc -l` \
    && exact $VAL 526 \
    && pass || fail ll_edges

harris $TMP/1 $TMP/2 \
    && VAL=`flprintasc $TMP/2 | wc -l` \
    && approx $VAL 55 \
    && pass || fail harris

# TODO
# vpsegplot
# VP_DEMO

echo

# image/domain
echo -n "image/domain: "

fcrop -x 20 -y 20 -o 3 $SAMPLES/images/cimage $TMP/1 40 100 50 110 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3.21633 \
    && pass || fail fcrop

cccrop -x 20 -y 20 -o 3 $SAMPLES/images/ccimage \
    $TMP/1 40 100 50 110 2> /dev/null \
    && cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && fdiff $TMP/3 $TMP/2 $TMP/4 \
    && VAL=`fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 6.01568 \
    && pass || fail cccrop

cextract $SAMPLES/images/cimage $TMP/1 40 100 50 110 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 5.06073 \
    && pass || fail cextract

fextract $SAMPLES/images/cimage $TMP/1 40 100 50 110 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 5.06073 \
    && pass || fail fextract

ccextract $SAMPLES/images/ccimage $TMP/1 40 100 50 110 \
    && cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && fdiff $TMP/3 $TMP/2 $TMP/4 \
    && VAL=`fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 6.9226 \
    && pass || fail ccextract

clocal_zoom -x 100 -y 150 -W 64 -X 3 $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.07492 \
    && pass || fail clocal_zoom

flocal_zoom -x 100 -y 150 -W 64 -X 3 $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.07492 \
    && pass || fail flocal_zoom

cclocal_zoom -x 100 -y 150 -W 64 -X 3 $SAMPLES/images/ccimage $TMP/1 \
    && cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && fdiff $TMP/3 $TMP/2 $TMP/4 \
    && VAL=`fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 8.50785 \
    && pass || fail cclocal_zoom

funzoom -o 0 -z 4 $SAMPLES/images/cimage $TMP/1 \
    && fshift -h $TMP/1 $TMP/2 \
    && VAL=`fnorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 110.12 \
    && pass || fail funzoom

czoom -X 4 -o 5 $TMP/1 $TMP/2 2> /dev/null \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 5.97412 \
    && pass || fail czoom

cczoom -X 4 -o 5 $TMP/1 $TMP/2 2> /dev/null \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 5.97412 \
    && pass || fail cczoom

fzoom -X 4 -o 5 $TMP/1 $TMP/2 2> /dev/null \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 5.969 \
    && pass || fail fzoom

csample $SAMPLES/images/cimage $TMP/1 4 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 19.2588 \
    && pass || fail csample

fsample $SAMPLES/images/cimage $TMP/1 4 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 19.2588 \
    && pass || fail fsample

cextcenter -f 27 $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fsize $TMP/1` \
    && exact "$VAL" "243 243" \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.76071 \
    && pass || fail cextcenter

cfextcenter -ftype IMG -f 27 $SAMPLES/images/ccimage $TMP/1 2> /dev/null \
    && VAL=`fsize $TMP/1` \
    && exact "$VAL" "243 243" \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 15.6567 \
    && pass || fail cfextcenter

fmaskrot -s 30 -b 10 $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 6.09773 \
    && pass || fail fmaskrot

fproj -x 100 -y 120 -o 3 $SAMPLES/images/cimage $TMP/1 \
    10 20 250 40 80 210 130 200 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 9.06184 \
    && pass || fail fproj

fzrt -o 3 $SAMPLES/images/cimage $TMP/1 1.1 57 -10 -20 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 7.30864 \
    && pass || fail fzrt

frot -a 35 $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 5.02289 \
    && pass || fail frot

fdirspline $SAMPLES/images/cimage 5 $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 6.31651 \
    && pass || fail fdirspline

finvspline $SAMPLES/images/cimage 5 $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 41.7204 \
    && pass || fail finvspline

ccopy $SAMPLES/images/cimage $TMP/1_001 \
    && ccopy $SAMPLES/images/fimage $TMP/1_002 \
    && sh $SCRIPTS/mw-mkmovie.sh Cmovie $TMP/1 1 2 \
    && pass || fail mk-movie

cmzoom -o 3 -X 2 $TMP/1 $TMP/2 2> /dev/null \
    && fdiff $TMP/2_01 $TMP/2_02 $TMP/3 \
    && VAL=`fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 9.81426 \
    && pass || fail cmzoom

ccmzoom -o 3 -X 2 $TMP/1 $TMP/2 2> /dev/null \
    && fdiff $TMP/2_01 $TMP/2_02 $TMP/3 \
    && VAL=`fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 9.81426 \
    && pass || fail ccmzoom

cmextract -b 0 $TMP/2 $TMP/3 30 30 1 170 170 1 $TMP/1 50 50 2 \
    && fdiff $TMP/3_01 $TMP/3_02 $TMP/4 \
    && VAL=`fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 8.09011 \
    && pass || fail cmextract

cmparitysep -l $TMP/1 $TMP/2 \
    && fdiff $TMP/2_01 $TMP/2_03 $TMP/3 \
    && fdiff $TMP/2_02 $TMP/2_04 $TMP/4 \
    && VAL=`fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 14.6972 \
    && VAL=`fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 14.74 \
    && pass || fail cmparitysep

# TODO
# cmcollect
# ccmcollect

# common: funzoom cfunzoom

echo

# image/filter
echo -n "image/filter: "

cfunzoom -z 4 -o 0 $SAMPLES/images/ccimage $TMP/1 \
    && cfdiffuse $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 9.08767 \
    && pass || fail cfunzoom

cfmdiffuse -n 2 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2_01 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 9.08767 \
    && VAL=`fnorm -v $TMP/2_02 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 7.25923 \
    && pass || fail cfmdiffuse

funzoom -z 4 -o 0 -ftype IMG $SAMPLES/images/cimage $TMP/1 2> /dev/null \
    && erosion -r 1.5 -n 1 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.8314 \
    && pass || fail funzoom

opening -r 1.5 -n 1 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.77 \
    && pass || fail opening

median -r 1.5 -n 1 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.7447 \
    && pass || fail median

amss -l 2 -d $TMP/2 $TMP/1 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 11.838 \
    && pass || fail amss

fquant $TMP/1 $TMP/2 5 > /dev/null \
    && osamss -l 2 $TMP/2 $TMP/3 \
    && VAL=`fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 11.8249 \
    && pass || fail fquant

heat -n 10 -s 0.1 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 10.777 \
    && pass || fail heat

fsmooth -S 2 -W 1 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 7.20909 \
    && pass || fail fsmooth

echo "-1 1 -1 1" | freadasc $TMP/2 2 2 \
    && fconvol $TMP/1 $TMP/2 $TMP/3 \
    && VAL=`fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 26.6814 \
    && pass || fail fconvol

fsepconvol -g 2 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 9.13969 \
    && pass || fail fsepconvol

fgrain -a 20 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.8573 \
    && pass || fail fgrain

forder -e 5 -n 1 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.697 \
    && pass || fail forder

fsharpen -p 50 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 22.7716 \
    && pass || fail fsharpen

rotaffin -r 5 -a 3 -t 3 -T 0 -A 5 $TMP/2 \
    && VAL=`grep nimage $TMP/2 | cut -d":" -f2` \
    && exact $VAL 15 \
    && VAL=`fnorm -v $TMP/2_10 | cut -d"=" -f2` \
    && approx $VAL 29.1366 \
    && pass || fail rotaffin

infsup -n 2 $TMP/1 $TMP/2 $TMP/3 \
    && VAL=`fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 11.2894 \
    && pass || fail infsup

ll_sharp -p 20 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 16.9317 \
    && pass || fail ll_sharp

resthline $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 16.5496 \
    && pass || fail resthline

shock -n 10 -s 0.1 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 19.0514 \
    && pass || fail shock

tvdenoise $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 14.5648 \
    && pass || fail tvdenoise

tvdenoise2 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 14.4665 \
    && pass || fail tvdenoise2

nlmeans -s 3 -d 5 $SAMPLES/images/cimage $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 6.45733 \
    && pass || fail nlmeans

fconvol $TMP/1 $DATA/image/blur3x3.ir $TMP/2 \
    && tvdeblur -n 30 $TMP/2 $DATA/image/blur3x3.ir $TMP/3 \
    && VAL=`fnorm -p 2 -c $TMP/1 $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 7.51181 \
    && pass || fail fconvol

cmextract $SAMPLES/movies/cmovie $TMP/1 128 128 3 140 140 7 \
    && mam -n 20 -a 0 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`fnorm -v $TMP/2_03 | cut -d"=" -f2` \
    && approx $VAL 8.49737 \
    && pass || fail cmextract

prolate -n 128 3 0.5 $TMP/1 > /dev/null \
    && VAL=`fsize $TMP/1` \
    && exact "$VAL" "3 3" \
    && VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.111111 \
    && pass || fail prolate

# TODO
# cfsharpen
# flipschitz
# prolatef

echo

# image/fourier
echo -n "image/fourier: "

fft2d -A $TMP/1 -B $TMP/2 $SAMPLES/images/cimage  \
    && fextract $TMP/1 $TMP/1 20 20 230 230 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 1234.21 \
    && fextract $TMP/2 $TMP/2 20 20 230 230 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 1230.29 \
    && pass || fail fft2d

fft2dpol -M $TMP/1 -P $TMP/2 $SAMPLES/images/cimage  \
    && fextract $TMP/1 $TMP/1 20 20 230 230 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 784.174 \
    && fextract $TMP/2 $TMP/2 20 20 230 230 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 1.65156 \
    && pass || fail fft2dpol

fft2drad -l -s 100 $SAMPLES/images/cimage $TMP/1 \
    && VAL=`snorm -b 5 -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.0241473 \
    && VAL=`snorm -b 5 -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3.48635 \
    && pass || fail fft2drad

fft2dview -t 0 -o $TMP/1 $SAMPLES/images/cimage \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 1923.05 \
    && pass || fail fft2dview

fftgrad -n $TMP/1 $SAMPLES/images/cimage \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.00567 \
    && pass || fail fftgrad

fftrot -a 33 $SAMPLES/images/cimage $TMP/1  \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 9.72568 \
    && pass || fail fftrot

funzoom -z 4 -o 0 $SAMPLES/images/cimage $TMP/1 \
    && fftzoom -z 2 $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 10.8767 \
    && pass || fail fftzoom

fhamming $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.74239 \
    && pass || fail fhamming

frandphase $SAMPLES/images/fimage $TMP/1 \
    && fft2dpol -P $TMP/2 $TMP/1 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && rand_approx $VAL 1.63 \
    && pass || fail frandphase

fextract $SAMPLES/images/cimage $TMP/1 10 10 200 210 \
    && fft2dshrink $TMP/1 $TMP/2 \
    && VAL=`fsize $TMP/2` \
    && exact "$VAL" "189 200" \
    && pass || fail fextract

fshrink2 $TMP/1 $TMP/2 \
    && VAL=`fsize $TMP/2` \
    && exact "$VAL" "128 128" \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 8.33146 \
    && pass || fail fshrink2

fsym2 $TMP/2 $TMP/2 \
    && VAL=`fsize $TMP/2` \
    && exact "$VAL" "256 256" \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 8.36696 \
    && pass || fail fsym2

wiener -W 0.1 -g 1 $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fnorm -b 10 -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 11.1962 \
    && pass || fail wiener

fkeepphase $SAMPLES/images/cimage $SAMPLES/images/fimage $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 14.041 \
    && pass || fail fkeepphase

faxpb -a 0 -b 0 $SAMPLES/images/cimage $TMP/1 \
    && fpset $TMP/1 0 0 1 $TMP/1 \
    && fsepconvol -b 2 -g 3 $TMP/1 $TMP/2 \
    && fft2d -A $TMP/3 $TMP/2 \
    && fftconvol $SAMPLES/images/cimage $TMP/3 $TMP/4 \
    && fsepconvol -b 2 -g 3 $SAMPLES/images/cimage $TMP/5 \
    && VAL=`fnorm -t 0.0001 -p 2 -c $TMP/4 $TMP/5 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail faxpb

echo

# image/io
echo -n "image/io: "

ccopy $SAMPLES/images/cimage $TMP/1 \
    && fdiff $SAMPLES/images/cimage $TMP/1 $TMP/1 \
    && VAL=`fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail ccopy

fcopy $SAMPLES/images/fimage $TMP/1 \
    && fdiff $SAMPLES/images/fimage $TMP/1 $TMP/1 \
    && VAL=`fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail fcopy

cccopy $SAMPLES/images/ccimage $TMP/1 \
    && cfdiff $SAMPLES/images/ccimage $TMP/1 $TMP/1 \
    && cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && VAL=`fnorm -p 2 $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && VAL=`fnorm -p 2 $TMP/3 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && VAL=`fnorm -p 2 $TMP/4 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail cccopy

# TODO
# cview
# fview
# ccview
# cmview
# ccmview
# flip

fconst $TMP/1 0 60 20 \
    && echo "hi guys..." | \
    ccputstring -r 3 -c 900 -C 90 $TMP/1 10 1 $TMP/1 \
    && ccopy $TMP/1 $TMP/1 2> /dev/null \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 19.8406 \
    && pass || fail fconst

cfgetchannels $SAMPLES/images/ccimage $TMP/1 $TMP/2 $TMP/3 \
    && cfputchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && cfdiff -ftype IMG \
    $SAMPLES/images/ccimage $TMP/4 $TMP/1 2> /dev/null \
    && VAL=`fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail cfgetchannel

cfchgchannels $SAMPLES/images/ccimage $TMP/1 \
    && cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 7.36361 \
    && VAL=`fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 15.9369 \
    && VAL=`fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 9.98046 \
    && pass || fail cfchgetchannel

cline_extract $SAMPLES/images/cimage $TMP/1 30 \
    && VAL=`snorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 11.1133 \
    && pass || fail cline_extract

fline_extract $SAMPLES/images/cimage $TMP/1 30 \
    && VAL=`snorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 11.1133 \
    && pass || fail fline_extract

echo "1 5 2 3" | creadasc $TMP/1 2 2 \
    && VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.75 \
    && VAL=`fvar $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.91667 \
    && pass || fail creadasc

cprintasc $TMP/1 > $TMP/2 \
    && VAL=`cat $TMP/2 | wc -l` \
    && exact $VAL 1 \
    && pass || fail cprintasc

fprintasc $TMP/1 > $TMP/2 \
    && VAL=`cat $TMP/2 | wc -l` \
    && exact $VAL 1 \
    && pass || fail fprintasc

# common: freadasc Mkmovie cfgetchannels

echo

# image/level_lines
echo -n "image/level_lines: "

# TODO
# flst
# flst_pixels
# flst_reconstruct
# flst_boundary

# TODO
# flst_bilinear
# flstb_boundary
# flstb_dual
# flstb_dualchain
# flstb_quantize
# flstb_tv

# TODO
# fml_ml
# fsaddles
# ll_distance
# ll_extract
# llmap
# ll_remove
# llremove
# llview

# TODO
# ml_decompose
# ml_draw
# ml_extract
# ml_fml
# ml_reconstruct

# TODO
# mscarea
# tjmap
# tjpoint

# TODO
# cll_remove
# cml_decompose
# cml_draw
# cml_reconstruct

echo

# image/misc
echo -n "image/misc: "

cdisc $TMP/1 100 100 \
    && VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 126.888 \
    && pass || fail cdisc

funzoom -z 4 -o 0 -ftype IMG \
    $SAMPLES/images/cimage $TMP/1 2> /dev/null \
    && cdisc -r 16 $TMP/2 64 64 \
    && binarize -i $TMP/2 $TMP/2 \
    && fmask $TMP/3 $TMP/2 $TMP/2 $TMP/1 \
    && fmask -i -c 1 $TMP/2 $TMP/2 $TMP/2 \
    && disocclusion $TMP/3 $TMP/2 $TMP/4 > $TMP/1 \
    && VAL=`grep energy $TMP/1 | cut -d"=" -f5` \
    && approx $VAL 980.02 \
    && VAL=`fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 14.66 \
    && pass || fail disocclusion

# TODO
# drawocclusion

echo "10 10 60 20 30 50 e 100 10 110 50 150 20 q" \
    | flreadasc 2 $TMP/1 > /dev/null  \
    && fillpolys -x 160 -y 60 $TMP/1 $TMP/1 \
    && emptypoly $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 12.4411 \
    && pass || fail  fillpolys

binarize -i -t 120 $SAMPLES/images/cimage $TMP/1 \
    && thinning $TMP/1 $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 17.7433 \
    && pass || fail thinning

funzoom -z 4 -o 0 -ftype IMG $SAMPLES/images/cimage $TMP/1 2> /dev/null \
    && binarize -i -t 120 $TMP/1 $TMP/1 \
    && skeleton -n 10 $TMP/1 $DATA/image/seg_mask $TMP/2 > /dev/null \
    && VAL=`grep nimage $TMP/2 | cut -d":" -f2` \
    && exact $VAL 11 \
    && VAL=`fnorm -v $TMP/2_11 | cut -d"=" -f2` \
    && approx $VAL 32.5981 \
    && pass || fail skeleton

# TODO
# lsnakes
# lsnakes_demo
# mac_snakes
# ccdisocclusion

echo

# image/operations
echo -n "image/operations: "

fop -p -A $SAMPLES/images/cimage $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 287.596 \
    && pass || fail fop

faxpb -a -1 $SAMPLES/images/cimage $TMP/1 \
    && fabso $TMP/1 $TMP/2 \
    && fdiff $TMP/2 $SAMPLES/images/cimage $TMP/3 \
    && VAL=`fnorm -p 2 $TMP/3 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail faxpb

fentropy $SAMPLES/images/cimage > $TMP/1 \
    && VAL=`cat $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 7.51668 \
    && pass || fail fentropy

fderiv -n $TMP/1 $SAMPLES/images/cimage \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3.85916 \
    && pass || fail fderiv

finfo $SAMPLES/images/cimage > $TMP/1 \
    && VAL=`grep "bv norm" $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.732667 \
    && pass || fail finfo

fmse -n $SAMPLES/images/cimage $SAMPLES/images/fimage > $TMP/1 \
    && VAL=`grep "^SNR" $TMP/1 | cut -d"=" -f2` \
    && approx $VAL -3.48774 \
    && VAL=`grep PSNR $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.10192 \
    && VAL=`grep MSE $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.23241 \
    && VAL=`grep MRD $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 89.7111 \
    && pass || fail fmse

cdisc $TMP/1 256 256 \
    && fmask $TMP/2 $TMP/1 $SAMPLES/images/cimage $SAMPLES/images/fimage \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 11.5658 \
    && pass || fail cdisk

fpsnr255 $SAMPLES/images/fimage > $TMP/1 \
    && VAL=`cat $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.347482 \
    && pass || fail fpsnr255

frthre -l 100 $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 44.9395 \
    && pass || fail frthre

# common: faxpb fpset cfdiff fadd fconst fdiff fmean fnorm fsize fvar

echo

# image/seg
echo -n "image/seg: "

echo

# image/shape_recognition
echo -n "image/shape_recognition: "

echo

# image/values
echo -n "image/values: "

binarize -t 150 $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 77.9132 \
    && pass || fail binarize

funzoom -z 8 $SAMPLES/images/cimage $TMP/1 \
    && fquant $TMP/1 $TMP/1 5 > $TMP/2 \
    && VAL=`cut -d"=" -f2 $TMP/2` \
    && amle_init $TMP/1 $VAL $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 48.7051 \
    && pass || fail fquant

amle $TMP/2 $TMP/3 2> /dev/null \
    && VAL=`fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 17.4285 \
    && pass || fail amle

cmextract $SAMPLES/movies/cmovie $TMP/1 40 170 3 80 210 7 \
    && for I in 1 2 3 4 5; do
        faxpb -ftype IMG -a 0.1 \
	    $TMP/1_0$I $TMP/1_0$I 2> /dev/null
	faxpb -ftype IMG -a 10 -b 5 \
	    $TMP/1_0$I $TMP/1_0$I 2> /dev/null
    done \
    && amle3d_init $TMP/1 10 $TMP/2 \
    && VAL=`fnorm -v $TMP/2_03 | cut -d"=" -f2` \
    && approx $VAL  26.1018 \
    && pass || fail amle_init

amle3d $TMP/2 $TMP/3 \
    && VAL=`fnorm -v $TMP/3_03 | cut -d"=" -f2` \
    && approx $VAL 4.03325 \
    && pass || fail amle3d

fvalues -r $TMP/1 $SAMPLES/images/cimage $TMP/2 \
    && VAL=`grep size $TMP/2 | cut -d":" -f2` \
    && approx $VAL 256 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.73267 \
    && pass || fail fvalues

ccontrast $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 12.5844 \
    && pass || fail ccontrast

ccontrast_local -d 2 $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 13.2157 \
    && pass || fail ccontrast_local

fconst $TMP/1 0 100 100 \
    && cnoise -i 50 $TMP/1 $TMP/1 \
    && VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && rand_approx $VAL 64 \
    && pass || fail cnoise

fconst $TMP/1 0 100 100 \
    && fnoise -g 10 $TMP/1 $TMP/1 \
    && VAL=`fvar $TMP/1 | cut -d"=" -f2` \
    && rand_approx $VAL 100 \
    && pass || fail fnoise

cmextract $SAMPLES/movies/cmovie $TMP/1 10 10 1 210 210 10 \
    && cmnoise -i 50 $TMP/1 $TMP/2 \
    && VAL=`grep nimage $TMP/2 | cut -d":" -f2` \
    && exact $VAL 10 \
    && VAL=`fmean $TMP/2_05 | cut -d"=" -f2` \
    && rand_approx $VAL 116 \
    && pass || fail cmnoise

chisto $SAMPLES/images/cimage $TMP/1 \
    && VAL=`snorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 366.222 \
    && pass || fail chisto

fhisto $SAMPLES/images/cimage $TMP/1 \
    && VAL=`snorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 929.198 \
    && pass || fail fhisto

flgamma -f 256 $TMP/1 \
    && VAL=`flprintasc $TMP/1 | grep "^246" | cut -d" " -f2` \
    && approx $VAL 236.391 \
    && pass || fail flgamma

fcontrast $SAMPLES/images/cimage $TMP/1 $TMP/2 \
    && VAL=`fnorm -p 2 $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 103.585 \
    && pass || fail fcontrast

frank -r $TMP/1 $SAMPLES/images/cimage \
    && VAL=`fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.577342 \
    && pass || fail frank

fthre -N $SAMPLES/images/cimage $TMP/1 \
    && VAL=`fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 143.798 \
    && pass || fail fthre

# TODO
# cfquant
# bicontrast

# common: fquant

echo

# signal
echo -n "signal: "

entropy $SAMPLES/signals/fsignal > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && approx $VAL 11.0934 \
    && pass || fail entropy entropy

sprintasc $SAMPLES/signals/fsignal 101 101 > $TMP/1 \
    && exact `cat $TMP/1` 3014 \
    && pass || fail sprintasc sprintasc

sprintasc $SAMPLES/signals/fsignal 1 123 | sreadasc $TMP/1 123 \
    && fft1dshrink $TMP/1 $TMP/2 \
    && VAL=`grep "size:" $TMP/2 | cut -d":" -f2` \
    && exact $VAL 121 \
    && pass || fail fft1dshrink

sshrink2 $SAMPLES/signals/fsignal $TMP/1 \
    && VAL=`grep "size:" $TMP/1 | cut -d":" -f2` \
    && exact $VAL 2048 \
    && pass || fail sshrink2

fct1d $TMP/1 $TMP/2 \
    && VAL=`snorm -b 20 -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 3636.57 \
    && pass || fail fct1d

fft1d -A $TMP/2 $TMP/1 \
    && VAL=`snorm -b 20 -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 1888.35 \
    && pass || fail fft1d

sconst -s 256 -a 0.1 $TMP/1 \
    && VAL=`grep "size:" $TMP/1 | cut -d":" -f2` \
    && exact $VAL 256 \
    && VAL=`snorm -b 20 -v $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail sconst

sderiv $SAMPLES/signals/fsignal $TMP/1 \
    && VAL=`snorm -b 20 -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 42.9396 \
    && pass || fail sderiv

sdirac -s 100 -a 100 $TMP/1 \
    && VAL=`snorm -b 0 -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2 \
    && VAL=`snorm -b 0 -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 10 \
    && pass || fail sdirac

sgauss -s 20 $TMP/1 3 || lls \
    && VAL=`snorm -b 0 -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.0130369 \
    && VAL=`snorm -b 0 -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.0686237 \
    && pass || fail sgauss

sintegral $TMP/1 $TMP/2 \
    && VAL=`snorm -b 0 -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 0.05 \
    && pass || fail sintegral

sderiv $TMP/1 $TMP/2 \
    && smse -n $TMP/1 $TMP/2 > $TMP/3 \
    && VAL=`grep "^SNR" $TMP/3 | cut -d"=" -f2` \
    && approx $VAL -1.9738 \
    && VAL=`grep PSNR $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 6.66453 \
    && VAL=`grep MSE $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 1.57536 \
    && VAL=`grep MRD $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 70.6568 \
    && pass || fail sderiv

sconst -s 1000 -a 0 $TMP/1 \
    && snoise -g 1 $TMP/1 $TMP/2 \
    && VAL=`snorm -p 2 $TMP/2 | cut -d"=" -f2` \
    && rand_approx $VAL 1 \
    && pass || fail sconst

sop -p -A $SAMPLES/signals/fsignal $SAMPLES/signals/fsignal $TMP/1 \
    && VAL=`snorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 5349.16 \
    && pass || fail sop

saxpb -a 2 $SAMPLES/signals/fsignal $TMP/1 \
    && VAL=`snorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 5349.16 \
    && pass || fail saxpb

splot -ftype RIM -o $TMP/1 -n $SAMPLES/signals/fsignal 2> /dev/null \
    && VAL=`fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 15.6218 \
    && pass || fail splot

echo "0 2 3 4 5 4 3 2 3 4 5 4 3 2 1 0" \
    | sreadasc $TMP/1 16 \
    && VAL=`snorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 1 \
    && pass || fail snorm

ssinus -s 100 -a 1 -d 1 $TMP/1  \
    && VAL=`snorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.0785888 \
    && pass || fail ssinus

sprintasc $SAMPLES/signals/fsignal 1 1024 \
    | sreadasc $TMP/1 1024 \
    && env DATA=$DATA sh $SCRIPTS/mw-swtvdenoise.sh \
    -D 10 -N 200 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`snorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 11.52 \
    && pass || fail mw-swtvdenoise

# TODO
# stvrestore
# w1threshold
# sinfo

# common: snorm

echo

# wave
echo -n "wave: "

owave1 -e 0 $SAMPLES/signals/fsignal $TMP/1 $DATA/wave/ortho/da02.ir \
    && VAL=`grep size $TMP/1_01_A.wtrans1d | cut -d":" -f2` \
    && exact $VAL 1104 \
    && VAL=`grep size $TMP/1_01_D.wtrans1d | cut -d":" -f2` \
    && exact $VAL 1104 \
    && pass || fail owave1

iowave1 -e 0 $TMP/1 $TMP/2 $DATA/wave/ortho/da02.ir \
    && VAL=`snorm -t 0.001 -b 2 -p 2 -c $SAMPLES/signals/fsignal $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail owave1

biowave1 $SAMPLES/signals/fsignal $TMP/1 \
    $DATA/wave/biortho/h/sp02.ir $DATA/wave/biortho/htilde/sl05.ir \
    && VAL=`grep size $TMP/1_01_A.wtrans1d | cut -d":" -f2` \
    && exact $VAL 1104 \
    && VAL=`grep size $TMP/1_01_D.wtrans1d | cut -d":" -f2` \
    && exact $VAL 1104 \
    && pass || fail biowave1

ibiowave1 -e 0 $TMP/1 $TMP/2 \
    $DATA/wave/biortho/h/sp02.ir $DATA/wave/biortho/htilde/sl05.ir \
    && VAL=`snorm -t 0.001 -b 2 -p 2 -c $SAMPLES/signals/fsignal $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail ibiowave1

# TODO
# biowave2
# dybiowave2
# dyowave2
# ibiowave2
# iowave2
# owave2
# owtrans_fimage
# precond1d
# precond2d
# sconvolve

echo

# wave/packets
echo -n "wave/packets: "

wp2dmktree -w 4 $TMP/1 \
    && VAL=`fsize $TMP/1` \
    && exact "$VAL" "16 16" \
    && VAL=`fmean $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 1.32812 \
    && pass || fail wp2dmktree

wp2doperate -t 2 -s 15 -b $DATA/wave/packets/biortho/htilde/sd09.ir \
    $TMP/1 $DATA/wave/packets/biortho/h/sd07.ir $SAMPLES/images/cimage $TMP/2 \
    && VAL=`fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 6.15582 \
    && pass || fail wp2doperate

wp2ddecomp $TMP/1 $SAMPLES/images/cimage $DATA/wave/ortho/da05.ir $TMP/2 \
    && pass || fail wp2ddecomp

# TODO
# wp2dchangepack
# wp2dchangetree
# Wp2dcheck
# wp2dchecktree
# wp2deigenval
# wp2dfreqorder
# wp2drecomp
# wp2dview
# wpsconvolve

echo

# wave/ridgelet
echo -n "wave/ridgelet: "

# TODO
# iridgelet
# istkwave1
# ridgelet
# ridgpolrec
# ridgrecpol
# ridgthres
# stkwave1

echo

results

#!/bin/sh 
#
# Testsuite for the modules

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
}
    
pass() {
    OK=$(($OK + 1))
    echo -n "."
}

results() {
    echo "test completed: $OK success, $FAIL failures"
    rm -rf $TMP
    exit $FAIL
}

OK=0
FAIL=0
TMP=/tmp/megawave_checkmodules.$$.tmp
mkdir $TMP

# check modules
echo "checking megawave modules"

# common modules
echo -n 'common modules: '

$MODDIR/fmean $DATA/images/cimage > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && approx $VAL 127.156 \
    && pass || fail

$MODDIR/fsize $DATA/images/cimage > $TMP/1 \
    && VAL=`cat $TMP/1` \
    && exact "$VAL" "256 256" \
    && pass || fail

$MODDIR/fnorm -v $DATA/images/cimage > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && approx $VAL 8.73267 \
    && pass || fail

$MODDIR/funzoom -z 4 -o 0 $DATA/images/cimage $TMP/1 > /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/1  | cut -d"=" -f2` \
    && approx $VAL 16.5741 \
    && pass || fail

$MODDIR/cfunzoom -z 4 -o 0 $DATA/images/ccimage $TMP/1 \
    && $MODDIR/cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && $MODDIR/fdiff $TMP/2 $TMP/3 $TMP/4 \
    && VAL=`$MODDIR/fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 12.6513 \
    && pass || fail

$MODDIR/cfgetchannels $DATA/images/ccimage \
    $TMP/1 $TMP/2 $TMP/3 > /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 14.6397 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 17.8047 \
    && VAL=`$MODDIR/fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 18.248 \
    && pass || fail

$MODDIR/fdiff $DATA/images/cimage $DATA/images/fimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 15.9042 \
    && pass || fail

$MODDIR/cfdiff $DATA/images/ccimage $DATA/images/cimage $TMP/1 \
    && $MODDIR/cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 18.6356 \
    && VAL=`$MODDIR/fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 21.3019 \
    && VAL=`$MODDIR/fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 21.7275 \
    && pass || fail

$MODDIR/fadd $DATA/images/cimage $DATA/images/cimage $TMP/1 \
    && $MODDIR/fdiff $TMP/1 $DATA/images/cimage $TMP/2 \
    && VAL=`$MODDIR/fnorm -p 2 -c $DATA/images/cimage $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/fquant $DATA/images/cimage $TMP/1 5 > /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.42627 \
    && pass || fail

echo "1 5 2 3" | $MODDIR/freadasc $TMP/1 2 2 \
    && VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.75 \
    && VAL=`$MODDIR/fvar $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.91667 \
    && pass || fail

$MODDIR/snorm -v $DATA/signals/fsignal > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && approx $VAL 37.827 \
    && pass || fail

#$MODDIR/ccopy $DATA/images/cimage $TMP/1_1 \
#    && $MODDIR/ccopy $DATA/images/fimage $TMP/1_2 \
#    && $SCRIPTS/megawave_mkmovie Cmovie $TMP/1 1 2 \
#    && VAL=`grep nimage $TMP/1 | cut -d":" -f2` \
#    && exact $VAL 2 \
#    && pass || fail

$MODDIR/fconst $TMP/1 10 100 100 \
    && VAL=`$MODDIR/fnorm -p 1 $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 10 \
    && pass || fail

VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 10 \
    && pass || fail

$MODDIR/dkinfo $DATA/curves/curve > $TMP/1 \
    && VAL=`grep "Average step distance" $TMP/1 | cut -d":" -f2` \
    && approx $VAL 2.04765 \
    && pass || fail

$MODDIR/faxpb -a 2 -b 10 $DATA/images/cimage $TMP/1 \
    && $MODDIR/faxpb -a 0.5 -b -5 $TMP/1 $TMP/2 \
    && $MODDIR/fdiff $TMP/2 $DATA/images/cimage $TMP/3 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/3 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/fconst $TMP/1 0 10 10 \
    && $MODDIR/fpset $TMP/1 5 5 100 $TMP/1 || fail \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2 \
    && pass || fail

echo

# compression/ezwave
echo -n "compression/ezwave: "

$MODDIR/cfunzoom -z 4 -o 0 $DATA/images/ccimage $TMP/1

$MODDIR/cfezw -R 0.5 -o $TMP/3 \
    $TMP/1 $DATA/wave/biortho/h/sd07.ir $TMP/2 > /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/2 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 11.0831 \
    && pass || fail

$MODDIR/cfiezw $TMP/3 $DATA/wave/biortho/h/sd07.ir $TMP/4 > /dev/null \
    && VAL=`$MODDIR/fnorm -p 2 -c $TMP/4 $TMP/2 2> /dev/null | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

#$SCRIPTS/megawave_cfezw $TMP/1 > $TMP/2 \
#    && VAL=`tail -1 $TMP/2 | cut -f2` \
#    && approx $VAL 31.18 \
#    && pass || fail

$MODDIR/funzoom -z 4 -o 0 $DATA/images/cimage $TMP/1

$MODDIR/fezw -R 0.5 -o $TMP/3 \
    $TMP/1 $DATA/wave/biortho/h/sd07.ir $TMP/2 > /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/2 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 18.1235 \
    && pass || fail

$MODDIR/fiezw $TMP/3 $DATA/wave/biortho/h/sd07.ir $TMP/4 > /dev/null \
    && VAL=`$MODDIR/fnorm -p 2 -c $TMP/4 $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

#$SCRIPTS/megawave_fezw $TMP/1 > $TMP/2 \
#    && VAL=`tail -1 $TMP/2 | cut -f2` \
#    && approx $VAL 33.62 \
#    && pass || fail

echo

# compression/lossless
echo -n "compression/lossless: "

$MODDIR/funzoom -ftype IMG -z 4 -o 0 $DATA/images/cimage $TMP/1 2> /dev/null \
    && $MODDIR/arencode2 $TMP/1 $TMP/2 > $TMP/3 \
    && VAL=`grep Rate $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 7.57324 \
    && VAL=`$MODDIR/fsize $TMP/2` \
    && exact "$VAL" "3878 1" \
    && pass || fail

$MODDIR/arencode2 -H $TMP/1 $TMP/2 > /dev/null \
    && $MODDIR/ardecode2 -r 256 $TMP/2 $TMP/3 > /dev/null \
    && pass || fail

$MODDIR/cvsencode $DATA/curves/france.crv > $TMP/1 \
    && VAL=`grep N $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 7887 \
    && VAL=`grep B $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 10098 \
    && pass || fail

$MODDIR/cvsfrecode $DATA/curves/france.crv > $TMP/1 \
    && VAL=`grep N $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 7887 \
    && VAL=`grep B $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 10098 \
    && pass || fail

$MODDIR/cvsorgcode $DATA/curves/france.crv > $TMP/1 \
    && VAL=`grep N $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 7887 \
    && pass || fail

$MODDIR/funzoom -ftype IMG -z 4 -o 0 $DATA/images/cimage $TMP/1 2> /dev/null \
    && $MODDIR/fencode $TMP/1 > $TMP/2 \
    && VAL=`grep brate $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 7.34399 \
    && pass || fail

echo

# compression/scalar
echo -n "compression/scalar: "

$MODDIR/fscalq -p -n 10 $DATA/images/cimage $TMP/1 > $TMP/2 \
    && VAL=`cat $TMP/2` \
    && VAL=`$MODDIR/fentropy $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.91987 \
    && pass || fail

$MODDIR/fscalq -p -n 10 -o $TMP/2 $DATA/images/cimage $TMP/1 > /dev/null \
    && $MODDIR/fiscalq $TMP/2 $TMP/3 > /dev/null \
    && $MODDIR/fdiff $TMP/1 $TMP/3 $TMP/2 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

echo

# compression/vector
echo -n "compression/vector: "

$MODDIR/mk_codebook $TMP/1 \
    && VAL=`$MODDIR/fsize $TMP/1` \
    && exact "$VAL" "4 6" \
    && pass || fail

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

# $DATA/curves/curve

echo -n "curve: "

$MODDIR/area $DATA/curves/curve > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && exact $VAL 209718 \
    && pass || fail

$MODDIR/perimeter $DATA/curves/curve > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && exact $VAL 8422 \
    && pass || fail

$MODDIR/circle -r 10 -n 100 $TMP/1 \
    && VAL=`$MODDIR/area $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 313.953 \
    && VAL=`$MODDIR/perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 62.8215 \
    && pass || fail

$MODDIR/disc $TMP/1 2.5 \
    && VAL=`$MODDIR/perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 30.5708 \
    && pass || fail

$MODDIR/dsplit_convex -c . $DATA/curves/curve $TMP/1 > $TMP/2 \
    && VAL=`cut -d"=" -f2 $TMP/2` \
    && exact $VAL 1866 \
    && pass || fail

$MODDIR/fsplit_convex -c . $DATA/curves/curve $TMP/1 > $TMP/2 \
    && VAL=`cut -d"=" -f2 $TMP/2` \
    && exact $VAL 1866 \
    && pass || fail

$MODDIR/extract_connex $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 122 \
    && pass || fail

$MODDIR/fillpoly -x 1100 -y 1100 $DATA/curves/france.crv $TMP/1 \
    && VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 136.473 \
    && pass || fail

echo "10 10 60 20 30 50 e 100 10 110 50 150 20 q" \
    | $MODDIR/flreadasc 2 $TMP/1 > /dev/null \
    && $MODDIR/fillpolys -x 160 -y 60 $TMP/1 $TMP/1 \
    && VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 201.423 \
    && pass || fail

$MODDIR/fkbox $DATA/curves/curve > $TMP/1 \
    && VAL=`grep xmin $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 63 \
    && VAL=`grep ymin $TMP/1 | cut -d"=" -f2` \
    && exact $VAL -912 \
    && VAL=`grep xmax $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 692 \
    && VAL=`grep ymax $TMP/1 | cut -d"=" -f2` \
    && exact $VAL -123 \
    && pass || fail

$MODDIR/fkcenter $DATA/curves/curve > $TMP/1 \
    && VAL=`grep xg $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 336.298 \
    && VAL=`grep yg $TMP/1 | cut -d"=" -f2` \
    && approx $VAL -467.331 \
    && pass || fail

$MODDIR/fkcrop 0 -100 700 -200 $DATA/curves/curve $TMP/1 \
    && VAL=`$MODDIR/dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 5 \
    && VAL=`$MODDIR/fkcenter $TMP/1 | grep xg | cut -d"=" -f2` \
    && approx $VAL 324.922 \
    && pass || fail

$MODDIR/fkzrt $DATA/curves/curve  $TMP/1 0.5 30 20 20 \
    && VAL=`$MODDIR/fkcenter $TMP/1 | grep xg | cut -d"=" -f2` \
    && approx $VAL 282.455 \
    && pass || fail

echo "40 40 60 30 100 100 200 80 80 240 e 240 20 280 20 260 240 200
150 240 40 q" \
    | $MODDIR/fkreadasc $TMP/1 > /dev/null \
    && $MODDIR/ksplines -j 3 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/dkinfo $TMP/2 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 2 \
    && pass || fail

echo "40 40 60 30 100 100 200 80 80 240 q" \
    | $MODDIR/fkreadasc $TMP/1 > /dev/null \
    && $MODDIR/kspline -j 3 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/perimeter $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 318.806 \
    && pass || fail

echo "0.1 0.2" | $MODDIR/sreadasc $TMP/1 2 \
    && $MODDIR/flscale -ftype MW2_FCURVE $DATA/curves/curve $TMP/1 $TMP/2 \
    && $MODDIR/perimeter $TMP/2 > $TMP/3 \
    && VAL=`cut -d"=" -f2 $TMP/3` \
    && exact $VAL 1313 \
    && $MODDIR/area $TMP/2 > $TMP/3 \
    && VAL=`cut -d"=" -f2 $TMP/3` \
    && exact $VAL 4194.36 \
    && pass || fail

$MODDIR/flconcat -ftype MW2_FCURVES $TMP/2 $DATA/curves/curve $TMP/3 \
    && $MODDIR/fkbox $TMP/3 | grep ymax > $TMP/4 \
    && VAL=`cut -d"=" -f2 $TMP/4` \
    && exact $VAL -24.6 \
    && pass || fail

echo

# $DATA/curves/curve/io
echo -n "curve/io: "

echo "10 10 60 20 30 50 e 40 30 41 31 41 20 q" \
    | $MODDIR/fkreadasc $TMP/1 > /dev/null \
    && VAL=`$MODDIR/dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 2 \
    && VAL=`$MODDIR/perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 93.4166 \
    && pass || fail

echo "10 10 60 20 30 50 e 40 30 41 31 41 20 q" \
    | $MODDIR/flreadasc 2 $TMP/1 > /dev/null \
    && VAL=`$MODDIR/dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 2 \
    && VAL=`$MODDIR/perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 93.4166 \
    && pass || fail

$MODDIR/kplot $DATA/curves/curve $TMP/1 \
    && VAL=`$MODDIR/fsize $TMP/1` \
    && exact "$VAL" "630 790" \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 1.90955 \
    && pass || fail

$MODDIR/fkplot -s $DATA/curves/curve $TMP/1 \
    && VAL=`$MODDIR/fsize $TMP/1` \
    && exact "$VAL" "630 790" \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 1.90955 \
    && pass || fail

$MODDIR/fkprintasc $DATA/curves/curve > /dev/null \
    && VAL=`$MODDIR/fkprintasc $DATA/curves/curve | wc -l` \
    && exact $VAL 4115 \
    && pass || fail

$MODDIR/flprintasc $DATA/curves/curve > /dev/null \
    && VAL=`$MODDIR/flprintasc $DATA/curves/curve | wc -l` \
    && exact $VAL 4115 \
    && pass || fail

$MODDIR/fkprintfig $DATA/curves/curve > /dev/null \
    && VAL=`$MODDIR/fkprintfig $DATA/curves/curve | wc -w` \
    && exact $VAL 8255 \
    && pass || fail

echo "10 10 60 20 30 50 e 40 30 41 31 41 20 q" \
    | $MODDIR/flreadasc 2 $TMP/1 > /dev/null \
    && $MODDIR/fkprintfig $TMP/1 | $MODDIR/kreadfig $TMP/2 > /dev/null \
    && $MODDIR/fkzrt $TMP/2 $TMP/2 0.001 0 0 0 \
    && VAL=`$MODDIR/dkinfo $TMP/2 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 2 \
    && VAL=`$MODDIR/area $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 3600 \
    && pass || fail

$MODDIR/fkview -o $TMP/1 -n $DATA/curves/curve \
    && VAL=`$MODDIR/fnorm -v $TMP/1 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 2.61157 \
    && pass || fail

echo "1 1 6 2 3 5 4 3 4 4 4 2 q" | $MODDIR/fkreadasc $TMP/1 > /dev/null \
    && $MODDIR/kplotasc $TMP/1 > /dev/null \
    && VAL=`$MODDIR/kplotasc $TMP/1 | wc -c` \
    && exact $VAL 84 \
    && pass || fail

# TODO
# readpoly
# dkinfo

echo

# curve/smooth
echo -n "curve/smooth: "

$MODDIR/fksmooth -n 20 -s 10 $DATA/curves/curve $TMP/1 \
    && VAL=`$MODDIR/area $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 204306 \
    && VAL=`$MODDIR/perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3128.84 \
    && pass || fail

$MODDIR/iter_fksmooth -N 2 -n 10 -s 10 $DATA/curves/curve $TMP/1 \
    && VAL=`$MODDIR/dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 3 \
    && pass || fail

$MODDIR/gass -l 10 $DATA/curves/curve $TMP/1 \
    && VAL=`$MODDIR/area $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 207081 \
    && VAL=`$MODDIR/perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3914.79 \
    && pass || fail

$MODDIR/iter_gass -N 2 -S 10 $DATA/curves/curve $TMP/1 \
    && VAL=`$MODDIR/dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 3 \
    && pass || fail

$MODDIR/gcsf -l 10 $DATA/curves/curve $TMP/1 \
    && VAL=`$MODDIR/area $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 209003 \
    && VAL=`$MODDIR/perimeter $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3929.08 \
    && pass || fail

$MODDIR/iter_gcsf -N 2 -l 20 $DATA/curves/curve $TMP/1 \
    && VAL=`$MODDIR/dkinfo $TMP/1 | grep "Number of curves" | cut -d":" -f2` \
    && exact $VAL 3 \
    && pass || fail

echo

# $DATA/curves/curve/matching
echo -n "curve/matching: "

$MODDIR/gass -l 3 -e 2 $DATA/curves/curve $TMP/1 \
    && $MODDIR/km_inflexionpoints $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/flprintasc $TMP/2 | wc -l` \
    && exact $VAL 43 \
    && pass || fail

$MODDIR/km_bitangents $TMP/1 $TMP/2 $TMP/3 \
    && VAL=`$MODDIR/flprintasc $TMP/3 | wc -l` \
    && exact $VAL 20 \
    && pass || fail

$MODDIR/km_flatpoints $TMP/1 $TMP/2 $TMP/4 0.1 0.1 \
    && VAL=`$MODDIR/flprintasc $TMP/4 | wc -l` \
    && exact $VAL 25 \
    && pass || fail

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

$MODDIR/demohead1 $DATA/images/cimage $TMP/1 > /dev/null \
    && VAL=`$MODDIR/fsize $TMP/1` \
    && exact "$VAL" "1 1" \
    && pass || fail

$MODDIR/demohead2 > /dev/null \
    && pass || fail

$MODDIR/demohead3 $DATA/images/cimage $TMP/1 > /dev/null \
    && VAL=`$MODDIR/fnorm -p 2 -c $DATA/images/cimage $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/fadd $DATA/images/cimage $DATA/images/fimage $TMP/1 \
    && $MODDIR/fadd1 $DATA/images/cimage $DATA/images/fimage $TMP/2 \
    && VAL=`$MODDIR/fnorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/fadd2 $DATA/images/cimage $DATA/images/fimage $TMP/2 \
    && VAL=`$MODDIR/fnorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/fadd3 $DATA/images/cimage $DATA/images/fimage $TMP/2 \
    && VAL=`$MODDIR/fnorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/fadd4 $DATA/images/cimage $DATA/images/fimage $TMP/2 \
    && VAL=`$MODDIR/fnorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/make_cmovie $TMP/1 \
    && VAL=`$MODDIR/fsize $TMP/1_01` \
    && exact "$VAL" "256 256" \
    && VAL=`grep nimage $TMP/1 | cut -d":" -f2` \
    && exact $VAL 20 \
    && pass || fail

$MODDIR/make_fmovie $TMP/1 \
    && VAL=`$MODDIR/fsize $TMP/1_01` \
    && exact "$VAL" "256 256" \
    && VAL=`grep nimage $TMP/1 | cut -d":" -f2` \
    && exact $VAL 21 \
    && pass || fail

$MODDIR/make_ccmovie $TMP/1 \
    && $MODDIR/ccopy $TMP/1_01 $TMP/2 2> /dev/null \
    && VAL=`$MODDIR/fsize $TMP/2` \
    && exact "$VAL" "256 256" \
    && VAL=`grep nimage $TMP/1 | cut -d":" -f2` \
    && exact $VAL 20 \
    && pass || fail

$MODDIR/make_cfmovie $TMP/1 \
    && $MODDIR/ccopy $TMP/1_01 $TMP/2 2> /dev/null \
    && VAL=`$MODDIR/fsize $TMP/2` \
    && exact "$VAL" "256 256" \
    && VAL=`grep nimage $TMP/1 | cut -d":" -f2` \
    && exact $VAL 20 \
    && pass || fail

$MODDIR/make_cimage $TMP/1 \
    && VAL=`$MODDIR/fsize $TMP/1` \
    && exact "$VAL" "256 256" \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.992188 \
    && pass || fail

# TODO
# view_demo

echo

# image/detection
echo -n "image/detection: "

$MODDIR/funzoom -o 0 -z 4 $DATA/images/cimage $TMP/1

$MODDIR/canny $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.5183 \
    && pass || fail

$MODDIR/falign -e 4.3 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`$MODDIR/flprintasc $TMP/2 | wc -l` \
    && exact $VAL 14 \
    && pass || fail

$MODDIR/falign_mdl -e -2 -l 1 -n 30 $DATA/images/cimage $TMP/2 > /dev/null \
    && VAL=`$MODDIR/fsize $TMP/2` \
    && exact "$VAL" "6 43" \
    && pass || fail

$MODDIR/vpoint $DATA/images/cimage $TMP/2 $TMP/3 > /dev/null \
    && VAL=`$MODDIR/flprintasc $TMP/3 | cut -d" " -f2` \
    && approx $VAL 17.9527 \
    && pass || fail

$MODDIR/ll_boundaries -e 11 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`$MODDIR/flprintasc $TMP/2 | wc -l` \
    && exact $VAL 1139 \
    && pass || fail

$MODDIR/ll_boundaries2 -e 11 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`$MODDIR/flprintasc $TMP/2 | wc -l` \
    && exact $VAL 1073 \
    && pass || fail

$MODDIR/ll_edges -e 17 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`$MODDIR/flprintasc $TMP/2 | wc -l` \
    && exact $VAL 526 \
    && pass || fail

$MODDIR/harris $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/flprintasc $TMP/2 | wc -l` \
    && approx $VAL 55 \
    && pass || fail

# TODO
# vpsegplot
# VP_DEMO

echo

# image/domain
echo -n "image/domain: "

$MODDIR/fcrop -x 20 -y 20 -o 3 $DATA/images/cimage $TMP/1 40 100 50 110 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3.21633 \
    && pass || fail

$MODDIR/cccrop -x 20 -y 20 -o 3 $DATA/images/ccimage $TMP/1 40 100 50 110 2> /dev/null \
    && $MODDIR/cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && $MODDIR/fdiff $TMP/3 $TMP/2 $TMP/4 \
    && VAL=`$MODDIR/fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 6.01568 \
    && pass || fail

$MODDIR/cextract $DATA/images/cimage $TMP/1 40 100 50 110 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 5.06073 \
    && pass || fail

$MODDIR/fextract $DATA/images/cimage $TMP/1 40 100 50 110 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 5.06073 \
    && pass || fail

$MODDIR/ccextract $DATA/images/ccimage $TMP/1 40 100 50 110 \
    && $MODDIR/cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && $MODDIR/fdiff $TMP/3 $TMP/2 $TMP/4 \
    && VAL=`$MODDIR/fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 6.9226 \
    && pass || fail

$MODDIR/clocal_zoom -x 100 -y 150 -W 64 -X 3 $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.07492 \
    && pass || fail

$MODDIR/flocal_zoom -x 100 -y 150 -W 64 -X 3 $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.07492 \
    && pass || fail

$MODDIR/cclocal_zoom -x 100 -y 150 -W 64 -X 3 $DATA/images/ccimage $TMP/1 \
    && $MODDIR/cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && $MODDIR/fdiff $TMP/3 $TMP/2 $TMP/4 \
    && VAL=`$MODDIR/fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 8.50785 \
    && pass || fail

$MODDIR/funzoom -o 0 -z 4 $DATA/images/cimage $TMP/1 \
    && $MODDIR/fshift -h $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 110.12 \
    && pass || fail

$MODDIR/czoom -X 4 -o 5 $TMP/1 $TMP/2 2> /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 5.97412 \
    && pass || fail

$MODDIR/cczoom -X 4 -o 5 $TMP/1 $TMP/2 2> /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 5.97412 \
    && pass || fail

$MODDIR/fzoom -X 4 -o 5 $TMP/1 $TMP/2 2> /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 5.969 \
    && pass || fail

$MODDIR/csample $DATA/images/cimage $TMP/1 4 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 19.2588 \
    && pass || fail

$MODDIR/fsample $DATA/images/cimage $TMP/1 4 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 19.2588 \
    && pass || fail

$MODDIR/cextcenter -f 27 $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fsize $TMP/1` \
    && exact "$VAL" "243 243" \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.76071 \
    && pass || fail

$MODDIR/cfextcenter -ftype IMG -f 27 $DATA/images/ccimage $TMP/1 2> /dev/null \
    && VAL=`$MODDIR/fsize $TMP/1` \
    && exact "$VAL" "243 243" \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 15.6567 \
    && pass || fail

$MODDIR/fmaskrot -s 30 -b 10 $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 6.09773 \
    && pass || fail

$MODDIR/fproj -x 100 -y 120 -o 3 $DATA/images/cimage $TMP/1 10 20 250 40 80 210 130 200 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 9.06184 \
    && pass || fail

$MODDIR/fzrt -o 3 $DATA/images/cimage $TMP/1 1.1 57 -10 -20 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 7.30864 \
    && pass || fail

$MODDIR/frot -a 35 $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 5.02289 \
    && pass || fail

$MODDIR/fdirspline $DATA/images/cimage 5 $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 6.31651 \
    && pass || fail

$MODDIR/finvspline $DATA/images/cimage 5 $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 41.7204 \
    && pass || fail

#$MODDIR/ccopy $DATA/images/cimage $TMP/1_001 \
#    && $MODDIR/ccopy $DATA/images/fimage $TMP/1_002 \
#    && $SCRIPTS/megawave_mkmovie Cmovie $TMP/1 1 2 \
#    && pass || fail

$MODDIR/cmzoom -o 3 -X 2 $TMP/1 $TMP/2 2> /dev/null \
    && $MODDIR/fdiff $TMP/2_01 $TMP/2_02 $TMP/3 \
    && VAL=`$MODDIR/fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 9.81426 \
    && pass || fail

$MODDIR/ccmzoom -o 3 -X 2 $TMP/1 $TMP/2 2> /dev/null \
    && $MODDIR/fdiff $TMP/2_01 $TMP/2_02 $TMP/3 \
    && VAL=`$MODDIR/fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 9.81426 \
    && pass || fail

$MODDIR/cmextract -b 0 $TMP/2 $TMP/3 30 30 1 170 170 1 $TMP/1 50 50 2 \
    && $MODDIR/fdiff $TMP/3_01 $TMP/3_02 $TMP/4 \
    && VAL=`$MODDIR/fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 8.09011 \
    && pass || fail

$MODDIR/cmparitysep -l $TMP/1 $TMP/2 \
    && $MODDIR/fdiff $TMP/2_01 $TMP/2_03 $TMP/3 \
    && $MODDIR/fdiff $TMP/2_02 $TMP/2_04 $TMP/4 \
    && VAL=`$MODDIR/fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 14.6972 \
    && VAL=`$MODDIR/fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 14.74 \
    && pass || fail

# TODO
# cmcollect
# ccmcollect

# common: funzoom cfunzoom

echo

# image/filter
echo -n "image/filter: "

$MODDIR/cfunzoom -z 4 -o 0 $DATA/images/ccimage $TMP/1 \
    && $MODDIR/cfdiffuse $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 9.08767 \
    && pass || fail

$MODDIR/cfmdiffuse -n 2 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2_01 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 9.08767 \
    && VAL=`$MODDIR/fnorm -v $TMP/2_02 2> /dev/null | cut -d"=" -f2` \
    && approx $VAL 7.25923 \
    && pass || fail

$MODDIR/funzoom -z 4 -o 0 -ftype IMG $DATA/images/cimage $TMP/1 2> /dev/null \
    && $MODDIR/erosion -r 1.5 -n 1 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.8314 \
    && pass || fail

$MODDIR/opening -r 1.5 -n 1 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.77 \
    && pass || fail

$MODDIR/median -r 1.5 -n 1 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.7447 \
    && pass || fail

$MODDIR/amss -l 2 -d $TMP/2 $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 11.838 \
    && pass || fail

$MODDIR/fquant $TMP/1 $TMP/2 5 > /dev/null \
    && $MODDIR/osamss -l 2 $TMP/2 $TMP/3 \
    && VAL=`$MODDIR/fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 11.8249 \
    && pass || fail

$MODDIR/heat -n 10 -s 0.1 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 10.777 \
    && pass || fail

$MODDIR/fsmooth -S 2 -W 1 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 7.20909 \
    && pass || fail

echo "-1 1 -1 1" | $MODDIR/freadasc $TMP/2 2 2 \
    && $MODDIR/fconvol $TMP/1 $TMP/2 $TMP/3 \
    && VAL=`$MODDIR/fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 26.6814 \
    && pass || fail

$MODDIR/fsepconvol -g 2 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 9.13969 \
    && pass || fail

$MODDIR/fgrain -a 20 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.8573 \
    && pass || fail

$MODDIR/forder -e 5 -n 1 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 13.697 \
    && pass || fail

$MODDIR/fsharpen -p 50 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 22.7716 \
    && pass || fail

$MODDIR/rotaffin -r 5 -a 3 -t 3 -T 0 -A 5 $TMP/2 \
    && VAL=`grep nimage $TMP/2 | cut -d":" -f2` \
    && exact $VAL 15 \
    && VAL=`$MODDIR/fnorm -v $TMP/2_10 | cut -d"=" -f2` \
    && approx $VAL 29.1366 \
    && pass || fail

$MODDIR/infsup -n 2 $TMP/1 $TMP/2 $TMP/3 \
    && VAL=`$MODDIR/fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 11.2894 \
    && pass || fail

$MODDIR/ll_sharp -p 20 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 16.9317 \
    && pass || fail

$MODDIR/resthline $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 16.5496 \
    && pass || fail

$MODDIR/shock -n 10 -s 0.1 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 19.0514 \
    && pass || fail

$MODDIR/tvdenoise $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 14.5648 \
    && pass || fail

$MODDIR/tvdenoise2 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 14.4665 \
    && pass || fail

$MODDIR/nlmeans -s 3 -d 5 $DATA/images/cimage $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 6.45733 \
    && pass || fail

$MODDIR/fconvol $TMP/1 $DATA/image/blur3x3.ir $TMP/2 \
    && $MODDIR/tvdeblur -n 30 $TMP/2 $DATA/image/blur3x3.ir $TMP/3 \
    && VAL=`$MODDIR/fnorm -p 2 -c $TMP/1 $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 7.51181 \
    && pass || fail

$MODDIR/cmextract $DATA/movies/cmovie $TMP/1 128 128 3 140 140 7 \
    && $MODDIR/mam -n 20 -a 0 $TMP/1 $TMP/2 > /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/2_03 | cut -d"=" -f2` \
    && approx $VAL 8.49737 \
    && pass || fail

$MODDIR/prolate -n 128 3 0.5 $TMP/1 > /dev/null \
    && VAL=`$MODDIR/fsize $TMP/1` \
    && exact "$VAL" "3 3" \
    && VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.111111 \
    && pass || fail

# TODO
# cfsharpen
# flipschitz
# prolatef

echo

# image/fourier
echo -n "image/fourier: "

$MODDIR/fft2d -A $TMP/1 -B $TMP/2 $DATA/images/cimage  \
    && $MODDIR/fextract $TMP/1 $TMP/1 20 20 230 230 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 1234.21 \
    && $MODDIR/fextract $TMP/2 $TMP/2 20 20 230 230 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 1230.29 \
    && pass || fail

$MODDIR/fft2dpol -M $TMP/1 -P $TMP/2 $DATA/images/cimage  \
    && $MODDIR/fextract $TMP/1 $TMP/1 20 20 230 230 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 784.174 \
    && $MODDIR/fextract $TMP/2 $TMP/2 20 20 230 230 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 1.65156 \
    && pass || fail

$MODDIR/fft2drad -l -s 100 $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/snorm -b 5 -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.0241473 \
    && VAL=`$MODDIR/snorm -b 5 -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3.48635 \
    && pass || fail

$MODDIR/fft2dview -t 0 -o $TMP/1 $DATA/images/cimage \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 1923.05 \
    && pass || fail

$MODDIR/fftgrad -n $TMP/1 $DATA/images/cimage \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.00567 \
    && pass || fail

$MODDIR/fftrot -a 33 $DATA/images/cimage $TMP/1  \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 9.72568 \
    && pass || fail

$MODDIR/funzoom -z 4 -o 0 $DATA/images/cimage $TMP/1 \
    && $MODDIR/fftzoom -z 2 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 10.8767 \
    && pass || fail

$MODDIR/fhamming $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.74239 \
    && pass || fail

$MODDIR/frandphase $DATA/images/fimage $TMP/1 \
    && $MODDIR/fft2dpol -P $TMP/2 $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && rand_approx $VAL 1.63 \
    && pass || fail

$MODDIR/fextract $DATA/images/cimage $TMP/1 10 10 200 210 \
    && $MODDIR/fft2dshrink $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fsize $TMP/2` \
    && exact "$VAL" "189 200" \
    && pass || fail

$MODDIR/fshrink2 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fsize $TMP/2` \
    && exact "$VAL" "128 128" \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 8.33146 \
    && pass || fail

$MODDIR/fsym2 $TMP/2 $TMP/2 \
    && VAL=`$MODDIR/fsize $TMP/2` \
    && exact "$VAL" "256 256" \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 8.36696 \
    && pass || fail

$MODDIR/wiener -W 0.1 -g 1 $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -b 10 -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 11.1962 \
    && pass || fail

$MODDIR/fkeepphase $DATA/images/cimage $DATA/images/fimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 14.041 \
    && pass || fail

$MODDIR/faxpb -a 0 -b 0 $DATA/images/cimage $TMP/1 \
    && $MODDIR/fpset $TMP/1 0 0 1 $TMP/1 \
    && $MODDIR/fsepconvol -b 2 -g 3 $TMP/1 $TMP/2 \
    && $MODDIR/fft2d -A $TMP/3 $TMP/2 \
    && $MODDIR/fftconvol $DATA/images/cimage $TMP/3 $TMP/4 \
    && $MODDIR/fsepconvol -b 2 -g 3 $DATA/images/cimage $TMP/5 \
    && VAL=`$MODDIR/fnorm -t 0.0001 -p 2 -c $TMP/4 $TMP/5 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

echo

# image/io
echo -n "image/io: "

$MODDIR/ccopy $DATA/images/cimage $TMP/1 \
    && $MODDIR/fdiff $DATA/images/cimage $TMP/1 $TMP/1 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/fcopy $DATA/images/fimage $TMP/1 \
    && $MODDIR/fdiff $DATA/images/fimage $TMP/1 $TMP/1 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/cccopy $DATA/images/ccimage $TMP/1 \
    && $MODDIR/cfdiff $DATA/images/ccimage $TMP/1 $TMP/1 \
    && $MODDIR/cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/3 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/4 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

# TODO
# cview
# fview
# ccview
# cmview
# ccmview
# flip

$MODDIR/fconst $TMP/1 0 60 20 \
    && echo "hi guys..." | \
    $MODDIR/ccputstring -r 3 -c 900 -C 90 $TMP/1 10 1 $TMP/1 \
    && $MODDIR/ccopy $TMP/1 $TMP/1 2> /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 19.8406 \
    && pass || fail

$MODDIR/cfgetchannels $DATA/images/ccimage $TMP/1 $TMP/2 $TMP/3 \
    && $MODDIR/cfputchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && $MODDIR/cfdiff -ftype IMG \
    $DATA/images/ccimage $TMP/4 $TMP/1 2> /dev/null \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/cfchgchannels $DATA/images/ccimage $TMP/1 \
    && $MODDIR/cfgetchannels $TMP/1 $TMP/2 $TMP/3 $TMP/4 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 7.36361 \
    && VAL=`$MODDIR/fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 15.9369 \
    && VAL=`$MODDIR/fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 9.98046 \
    && pass || fail

$MODDIR/cline_extract $DATA/images/cimage $TMP/1 30 \
    && VAL=`$MODDIR/snorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 11.1133 \
    && pass || fail

$MODDIR/fline_extract $DATA/images/cimage $TMP/1 30 \
    && VAL=`$MODDIR/snorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 11.1133 \
    && pass || fail

echo "1 5 2 3" | $MODDIR/creadasc $TMP/1 2 2 \
    && VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.75 \
    && VAL=`$MODDIR/fvar $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.91667 \
    && pass || fail

$MODDIR/cprintasc $TMP/1 > $TMP/2 \
    && VAL=`cat $TMP/2 | wc -l` \
    && exact $VAL 1 \
    && pass || fail

$MODDIR/fprintasc $TMP/1 > $TMP/2 \
    && VAL=`cat $TMP/2 | wc -l` \
    && exact $VAL 1 \
    && pass || fail

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

$MODDIR/cdisc $TMP/1 100 100 \
    && VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 126.888 \
    && pass || fail

$MODDIR/funzoom -z 4 -o 0 -ftype IMG \
    $DATA/images/cimage $TMP/1 2> /dev/null \
    && $MODDIR/cdisc -r 16 $TMP/2 64 64 \
    && $MODDIR/binarize -i $TMP/2 $TMP/2 \
    && $MODDIR/fmask $TMP/3 $TMP/2 $TMP/2 $TMP/1 \
    && $MODDIR/fmask -i -c 1 $TMP/2 $TMP/2 $TMP/2 \
    && $MODDIR/disocclusion $TMP/3 $TMP/2 $TMP/4 > $TMP/1 \
    && VAL=`grep energy $TMP/1 | cut -d"=" -f5` \
    && approx $VAL 980.02 \
    && VAL=`$MODDIR/fnorm -v $TMP/4 | cut -d"=" -f2` \
    && approx $VAL 14.66 \
    && pass || fail

# TODO
# drawocclusion

echo "10 10 60 20 30 50 e 100 10 110 50 150 20 q" \
    | $MODDIR/flreadasc 2 $TMP/1 > /dev/null  \
    && $MODDIR/fillpolys -x 160 -y 60 $TMP/1 $TMP/1 \
    && $MODDIR/emptypoly $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 12.4411 \
    && pass || fail

$MODDIR/binarize -i -t 120 $DATA/images/cimage $TMP/1 \
    && $MODDIR/thinning $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 17.7433 \
    && pass || fail

$MODDIR/funzoom -z 4 -o 0 -ftype IMG $DATA/images/cimage $TMP/1 2> /dev/null \
    && $MODDIR/binarize -i -t 120 $TMP/1 $TMP/1 \
    && $MODDIR/skeleton -n 10 $TMP/1 $DATA/image/seg_mask $TMP/2 > /dev/null \
    && VAL=`grep nimage $TMP/2 | cut -d":" -f2` \
    && exact $VAL 11 \
    && VAL=`$MODDIR/fnorm -v $TMP/2_11 | cut -d"=" -f2` \
    && approx $VAL 32.5981 \
    && pass || fail

# TODO
# lsnakes
# lsnakes_demo
# mac_snakes
# ccdisocclusion

echo

# image/operations
echo -n "image/operations: "

$MODDIR/fop -p -A $DATA/images/cimage $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 287.596 \
    && pass || fail

$MODDIR/faxpb -a -1 $DATA/images/cimage $TMP/1 \
    && $MODDIR/fabso $TMP/1 $TMP/2 \
    && $MODDIR/fdiff $TMP/2 $DATA/images/cimage $TMP/3 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/3 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/fentropy $DATA/images/cimage > $TMP/1 \
    && VAL=`cat $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 7.51668 \
    && pass || fail

$MODDIR/fderiv -n $TMP/1 $DATA/images/cimage \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 3.85916 \
    && pass || fail

$MODDIR/finfo $DATA/images/cimage > $TMP/1 \
    && VAL=`grep "bv norm" $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.732667 \
    && pass || fail

$MODDIR/fmse -n $DATA/images/cimage $DATA/images/fimage > $TMP/1 \
    && VAL=`grep "^SNR" $TMP/1 | cut -d"=" -f2` \
    && approx $VAL -3.48774 \
    && VAL=`grep PSNR $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.10192 \
    && VAL=`grep MSE $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2.23241 \
    && VAL=`grep MRD $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 89.7111 \
    && pass || fail

$MODDIR/cdisc $TMP/1 256 256 \
    && $MODDIR/fmask $TMP/2 $TMP/1 $DATA/images/cimage $DATA/images/fimage \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 11.5658 \
    && pass || fail

$MODDIR/fpsnr255 $DATA/images/fimage > $TMP/1 \
    && VAL=`cat $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.347482 \
    && pass || fail

$MODDIR/frthre -l 100 $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 44.9395 \
    && pass || fail

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

$MODDIR/binarize -t 150 $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 77.9132 \
    && pass || fail

$MODDIR/funzoom -z 8 $DATA/images/cimage $TMP/1 \
    && $MODDIR/fquant $TMP/1 $TMP/1 5 > $TMP/2 \
    && VAL=`cut -d"=" -f2 $TMP/2` \
    && $MODDIR/amle_init $TMP/1 $VAL $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 48.7051 \
    && pass || fail

$MODDIR/amle $TMP/2 $TMP/3 2> /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 17.4285 \
    && pass || fail

$MODDIR/cmextract $DATA/movies/cmovie $TMP/1 40 170 3 80 210 7 \
    && for I in 1 2 3 4 5; do
        $MODDIR/faxpb -ftype IMG -a 0.1 \
	    $TMP/1_0$I $TMP/1_0$I 2> /dev/null
	$MODDIR/faxpb -ftype IMG -a 10 -b 5 \
	    $TMP/1_0$I $TMP/1_0$I 2> /dev/null
    done \
    && $MODDIR/amle3d_init $TMP/1 10 $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2_03 | cut -d"=" -f2` \
    && approx $VAL  26.1018 \
    && pass || fail

$MODDIR/amle3d $TMP/2 $TMP/3 \
    && VAL=`$MODDIR/fnorm -v $TMP/3_03 | cut -d"=" -f2` \
    && approx $VAL 4.03325 \
    && pass || fail

$MODDIR/fvalues -r $TMP/1 $DATA/images/cimage $TMP/2 \
    && VAL=`grep size $TMP/2 | cut -d":" -f2` \
    && approx $VAL 256 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 8.73267 \
    && pass || fail

$MODDIR/ccontrast $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 12.5844 \
    && pass || fail

$MODDIR/ccontrast_local -d 2 $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 13.2157 \
    && pass || fail

$MODDIR/fconst $TMP/1 0 100 100 \
    && $MODDIR/cnoise -i 50 $TMP/1 $TMP/1 \
    && VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && rand_approx $VAL 64 \
    && pass || fail

$MODDIR/fconst $TMP/1 0 100 100 \
    && $MODDIR/fnoise -g 10 $TMP/1 $TMP/1 \
    && VAL=`$MODDIR/fvar $TMP/1 | cut -d"=" -f2` \
    && rand_approx $VAL 100 \
    && pass || fail

$MODDIR/cmextract $DATA/movies/cmovie $TMP/1 10 10 1 210 210 10 \
    && $MODDIR/cmnoise -i 50 $TMP/1 $TMP/2 \
    && VAL=`grep nimage $TMP/2 | cut -d":" -f2` \
    && exact $VAL 10 \
    && VAL=`$MODDIR/fmean $TMP/2_05 | cut -d"=" -f2` \
    && rand_approx $VAL 116 \
    && pass || fail

$MODDIR/chisto $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/snorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 366.222 \
    && pass || fail

$MODDIR/fhisto $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/snorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 929.198 \
    && pass || fail

$MODDIR/flgamma -f 256 $TMP/1 \
    && VAL=`$MODDIR/flprintasc $TMP/1 | grep "^246" | cut -d" " -f2` \
    && approx $VAL 236.391 \
    && pass || fail

$MODDIR/fcontrast $DATA/images/cimage $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 103.585 \
    && pass || fail

$MODDIR/frank -r $TMP/1 $DATA/images/cimage \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.577342 \
    && pass || fail

$MODDIR/fthre -N $DATA/images/cimage $TMP/1 \
    && VAL=`$MODDIR/fnorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 143.798 \
    && pass || fail

# TODO
# cfquant
# bicontrast

# common: fquant

echo

# signal
echo -n "signal: "

$MODDIR/entropy $DATA/signals/fsignal > $TMP/1 \
    && VAL=`cut -d"=" -f2 $TMP/1` \
    && approx $VAL 11.0934 \
    && pass || fail

$MODDIR/sprintasc $DATA/signals/fsignal 101 101 > $TMP/1 \
    && exact `cat $TMP/1` 3014 \
    && pass || fail

$MODDIR/sprintasc $DATA/signals/fsignal 1 123 | $MODDIR/sreadasc $TMP/1 123 \
    && $MODDIR/fft1dshrink $TMP/1 $TMP/2 \
    && VAL=`grep "size:" $TMP/2 | cut -d":" -f2` \
    && exact $VAL 121 \
    && pass || fail

$MODDIR/sshrink2 $DATA/signals/fsignal $TMP/1 \
    && VAL=`grep "size:" $TMP/1 | cut -d":" -f2` \
    && exact $VAL 2048 \
    && pass || fail

$MODDIR/fct1d $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/snorm -b 20 -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 3636.57 \
    && pass || fail

$MODDIR/fft1d -A $TMP/2 $TMP/1 \
    && VAL=`$MODDIR/snorm -b 20 -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 1888.35 \
    && pass || fail

$MODDIR/sconst -s 256 -a 0.1 $TMP/1 \
    && VAL=`grep "size:" $TMP/1 | cut -d":" -f2` \
    && exact $VAL 256 \
    && VAL=`$MODDIR/snorm -b 20 -v $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/sderiv $DATA/signals/fsignal $TMP/1 \
    && VAL=`$MODDIR/snorm -b 20 -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 42.9396 \
    && pass || fail

$MODDIR/sdirac -s 100 -a 100 $TMP/1 \
    && VAL=`$MODDIR/snorm -b 0 -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 2 \
    && VAL=`$MODDIR/snorm -b 0 -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 10 \
    && pass || fail

$MODDIR/sgauss -s 20 $TMP/1 3 || lls \
    && VAL=`$MODDIR/snorm -b 0 -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.0130369 \
    && VAL=`$MODDIR/snorm -b 0 -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.0686237 \
    && pass || fail

$MODDIR/sintegral $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/snorm -b 0 -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 0.05 \
    && pass || fail

$MODDIR/sderiv $TMP/1 $TMP/2 \
    && $MODDIR/smse -n $TMP/1 $TMP/2 > $TMP/3 \
    && VAL=`grep "^SNR" $TMP/3 | cut -d"=" -f2` \
    && approx $VAL -1.9738 \
    && VAL=`grep PSNR $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 6.66453 \
    && VAL=`grep MSE $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 1.57536 \
    && VAL=`grep MRD $TMP/3 | cut -d"=" -f2` \
    && approx $VAL 70.6568 \
    && pass || fail

$MODDIR/sconst -s 1000 -a 0 $TMP/1 \
    && $MODDIR/snoise -g 1 $TMP/1 $TMP/2 \
    && VAL=`$MODDIR/snorm -p 2 $TMP/2 | cut -d"=" -f2` \
    && rand_approx $VAL 1 \
    && pass || fail

$MODDIR/sop -p -A $DATA/signals/fsignal $DATA/signals/fsignal $TMP/1 \
    && VAL=`$MODDIR/snorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 5349.16 \
    && pass || fail

$MODDIR/saxpb -a 2 $DATA/signals/fsignal $TMP/1 \
    && VAL=`$MODDIR/snorm -p 2 $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 5349.16 \
    && pass || fail

$MODDIR/splot -ftype RIM -o $TMP/1 -n $DATA/signals/fsignal 2> /dev/null \
    && VAL=`$MODDIR/fnorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 15.6218 \
    && pass || fail

echo "0 2 3 4 5 4 3 2 3 4 5 4 3 2 1 0" \
    | $MODDIR/sreadasc $TMP/1 16 \
    && VAL=`$MODDIR/snorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 1 \
    && pass || fail

$MODDIR/ssinus -s 100 -a 1 -d 1 $TMP/1  \
    && VAL=`$MODDIR/snorm -v $TMP/1 | cut -d"=" -f2` \
    && approx $VAL 0.0785888 \
    && pass || fail

#$MODDIR/sprintasc $DATA/signals/fsignal 1 1024 \
#    | $MODDIR/sreadasc $TMP/1 1024 \
#    && $SCRIPTS/megawave_swtvdenoise -D 10 -N 200 $TMP/1 $TMP/2 > /dev/null \
#    && VAL=`$MODDIR/snorm -p 2 -c $TMP/1 $TMP/2 | cut -d"=" -f2` \
#    && approx $VAL 11.52 \
#    && pass || fail

# TODO
# stvrestore
# w1threshold
# sinfo

# common: snorm

echo

# wave
echo -n "wave: "

$MODDIR/owave1 -e 0 $DATA/signals/fsignal $TMP/1 $DATA/wave/ortho/da02.ir \
    && VAL=`grep size $TMP/1_01_A.wtrans1d | cut -d":" -f2` \
    && exact $VAL 1104 \
    && VAL=`grep size $TMP/1_01_D.wtrans1d | cut -d":" -f2` \
    && exact $VAL 1104 \
    && pass || fail

$MODDIR/iowave1 -e 0 $TMP/1 $TMP/2 $DATA/wave/ortho/da02.ir \
    && VAL=`$MODDIR/snorm -t 0.001 -b 2 -p 2 -c $DATA/signals/fsignal $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

$MODDIR/biowave1 $DATA/signals/fsignal $TMP/1 \
    $DATA/wave/biortho/h/sp02.ir $DATA/wave/biortho/htilde/sl05.ir \
    && VAL=`grep size $TMP/1_01_A.wtrans1d | cut -d":" -f2` \
    && exact $VAL 1104 \
    && VAL=`grep size $TMP/1_01_D.wtrans1d | cut -d":" -f2` \
    && exact $VAL 1104 \
    && pass || fail

$MODDIR/ibiowave1 -e 0 $TMP/1 $TMP/2 \
    $DATA/wave/biortho/h/sp02.ir $DATA/wave/biortho/htilde/sl05.ir \
    && VAL=`$MODDIR/snorm -t 0.001 -b 2 -p 2 -c $DATA/signals/fsignal $TMP/2 | cut -d"=" -f2` \
    && exact $VAL 0 \
    && pass || fail

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

$MODDIR/wp2dmktree -w 4 $TMP/1 \
    && VAL=`$MODDIR/fsize $TMP/1` \
    && exact "$VAL" "16 16" \
    && VAL=`$MODDIR/fmean $TMP/1 | cut -d"=" -f2` \
    && exact $VAL 1.32812 \
    && pass || fail

$MODDIR/wp2doperate -t 2 -s 15 -b $DATA/wave/packets/biortho/htilde/sd09.ir \
    $TMP/1 $DATA/wave/packets/biortho/h/sd07.ir $DATA/images/cimage $TMP/2 \
    && VAL=`$MODDIR/fnorm -v $TMP/2 | cut -d"=" -f2` \
    && approx $VAL 6.15582 \
    && pass || fail

$MODDIR/wp2ddecomp $TMP/1 $DATA/images/cimage $DATA/wave/ortho/da05.ir $TMP/2 \
    && pass || fail

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

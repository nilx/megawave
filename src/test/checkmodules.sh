#!/bin/sh 
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="Checkmodules"
_Vers="1.08"
_Date="2002-2007"
_Func="Test system modules"
_Auth="Lionel Moisan";
_Usage="[-help]"
# v 1.03 07/02/02 (JF) : changed test [ $numvar == 0 ] to [ $numvar -eq 0 ]
# v 1.08 04/07 (LM): bc -> bc -l in Check_approx() and Check_more_approx()
#----------------------------------------------------------------------------#
# This file is part of the MegaWave2 system macros. 
# MegaWave2 is a "soft-publication" for the scientific community. It has
# been developed for research purposes and it comes without any warranty.
# The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
# CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
#       94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
#-----------------------------------------------------------------------------

Incerr()
{
    err=`expr $err + 1`
}

# used for integers
Check_exact()
{
    if [ "$1" != "$2" ]; then Incerr; fi
}

# used for float numbers (threshold at 1% error)
Check_approx()
{
    r=`echo "a=$1;b=$2;c=(a-b)^2;d=0.0001*(a^2+b^2);if (c<=d) 1" | bc -l 2> /dev/null`
    if [ "$r" != "1" ]; then Incerr; fi
}

# used for random variables (threshold at 10% error)
Check_more_approx()
{
    r=`echo "a=$1;b=$2;c=(a-b)^2;d=0.01*(a^2+b^2);if (c<=d) 1" | bc -l 2> /dev/null`
    if [ "$r" != "1" ]; then Incerr; fi
}

Pass()
{
    if [ $err -ne 0 ]; then
	echo
	echo "*****" module '<'$1'>' failed
	toterr=`expr $toterr + 1`
	echo $1 >> $tmp/failed
    else
	mwecho -n "."
    fi
    err=0
    totmod=`expr $totmod + 1`
}

NoPass()
{
    err=0
    totno=`expr $totno + 1`
}

Error()
{
    echo $1
    exit 2
}

Settmpdir()
{
    rmtmproot=0
    if [ ! -d $1 ]; then return 1; fi
    if [ ! -x $1 ]; then return 1; fi
    if [ ! -w $1 ]; then return 1; fi
    tmproot=$1/tmp
    if [ ! -d $tmproot ]; then
	mkdir $tmproot || return 1
	rmtmproot=1
    fi
    if [ -d $tmproot ]; then 
	if [ ! -x $tmproot ]; then return 1; fi
	if [ ! -w $tmproot ]; then return 1; fi
	tmp=$tmproot/checkmodules_tmp
	rm -rf $tmp || return 1
	mkdir $tmp || return 1
    fi
    return 0
}


#----------------------------- MACRO BEGINS HERE -----------------------

#--- Usage
if [ $# -ne 0 ]; then
  . .mw2_help_lg_com
fi

#--- define tmp dir

Settmpdir $MEGAWAVE2 || Settmpdir $MY_MEGAWAVE2 || Error "Cannot set tmp directory. Exit."
rm -f $tmp/failed
touch $tmp/failed

#--- check needed commands

which cut > /dev/null || Error "Command <cut> not found. Exit."


#--- check modules

totmod=0
totno=0
toterr=0 
err=0

echo "---------- Checking system modules ----------"


# common modules (needed for test)

mwecho -n 'common modules (needed for test): '

fmean cimage > $tmp/1 || Incerr
v=`cut -d"=" -f2 $tmp/1`
Check_approx $v 127.156
Pass fmean

fsize cimage > $tmp/1 || Incerr
v=`cat $tmp/1`
Check_exact "$v" "256 256"
Pass fsize

fnorm -v cimage > $tmp/1 || Incerr
v=`cut -d"=" -f2 $tmp/1`
Check_approx $v 8.73267
Pass fnorm

funzoom -z 4 -o 0 cimage $tmp/1 > /dev/null || Incerr
v=`fnorm -v $tmp/1  | cut -d"=" -f2`
Check_approx $v 16.5741
Pass funzoom

cfunzoom -z 4 -o 0 ccimage $tmp/1 || Incerr
cfgetchannels $tmp/1 $tmp/2 $tmp/3 $tmp/4
fdiff $tmp/2 $tmp/3 $tmp/4 
v=`fnorm -v $tmp/4 | cut -d"=" -f2`
Check_approx $v 12.6513
Pass cfunzoom

cfgetchannels ccimage $tmp/1 $tmp/2 $tmp/3 > /dev/null || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 14.6397
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 17.8047
v=`fnorm -v $tmp/3 | cut -d"=" -f2`
Check_approx $v 18.248
Pass cfgetchannels

fdiff cimage fimage $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 15.9042
Pass fdiff

cfdiff ccimage cimage $tmp/1 || Incerr
cfgetchannels $tmp/1 $tmp/2 $tmp/3 $tmp/4
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 18.6356
v=`fnorm -v $tmp/3 | cut -d"=" -f2`
Check_approx $v 21.3019
v=`fnorm -v $tmp/4 | cut -d"=" -f2`
Check_approx $v 21.7275
Pass cfdiff

fadd cimage cimage $tmp/1
fdiff $tmp/1 cimage $tmp/2
v=`fnorm -p 2 -c cimage $tmp/2 | cut -d"=" -f2`
Check_exact $v 0
Pass fadd

fquant cimage $tmp/1 5 > /dev/null || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 8.42627
Pass fquant

echo "1 5 2 3" | freadasc $tmp/1 2 2 || Incerr
v=`fmean $tmp/1 | cut -d"=" -f2`
Check_approx $v 2.75
v=`fvar $tmp/1 | cut -d"=" -f2`
Check_approx $v 2.91667
Pass freadasc

snorm -v fsignal > $tmp/1 || Incerr
v=`cut -d"=" -f2 $tmp/1`
Check_approx $v 37.827
Pass snorm

ccopy cimage $tmp/1_1
ccopy fimage $tmp/1_2
Mkmovie Cmovie $tmp/1 1 2 || Incerr
v=`grep nimage $tmp/1 | cut -d":" -f2`
Check_exact $v 2
Pass Mkmovie

fconst $tmp/1 10 100 100 || Incerr
v=`fnorm -p 1 $tmp/1 | cut -d"=" -f2`
Check_exact $v 10
Pass fconst

v=`fmean $tmp/1 | cut -d"=" -f2`
Check_approx $v 10
Pass fmean

dkinfo curve > $tmp/1 || Incerr
v=`grep "Average step distance" $tmp/1 | cut -d":" -f2`
Check_approx $v 2.04765
Pass dkinfo

faxpb -a 2 -b 10 cimage $tmp/1 || Incerr
faxpb -a 0.5 -b -5 $tmp/1 $tmp/2 || Incerr
fdiff $tmp/2 cimage $tmp/3
v=`fnorm -p 2 $tmp/3 | cut -d"=" -f2`
Check_exact $v 0
Pass faxpb

fconst $tmp/1 0 10 10
fpset $tmp/1 5 5 100 $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 2
Pass fpset

echo


# compression/ezwave

mwecho -n "compression/ezwave: "

cfunzoom -z 4 -o 0 ccimage $tmp/1

cfezw -R 0.5 -o $tmp/3 $tmp/1 wave/biortho/h/sd07.ir $tmp/2 > /dev/null || Incerr
v=`fnorm -v $tmp/2 2> /dev/null | cut -d"=" -f2`
Check_approx $v 11.0831
Pass cfezw

cfiezw $tmp/3 wave/biortho/h/sd07.ir $tmp/4 > /dev/null || Incerr
v=`fnorm -p 2 -c $tmp/4 $tmp/2 2> /dev/null | cut -d"=" -f2`
Check_exact $v 0
Pass cfiezw

Cfezw $tmp/1 > $tmp/2 || Incerr
v=`tail -1 $tmp/2 | cut -f2`
Check_approx $v 31.18
Pass Cfezw

funzoom -z 4 -o 0 cimage $tmp/1

fezw -R 0.5 -o $tmp/3 $tmp/1 wave/biortho/h/sd07.ir $tmp/2 > /dev/null || Incerr
v=`fnorm -v $tmp/2 2> /dev/null | cut -d"=" -f2`
Check_approx $v 18.1235
Pass fezw
Pass ezw

fiezw $tmp/3 wave/biortho/h/sd07.ir $tmp/4 > /dev/null || Incerr
v=`fnorm -p 2 -c $tmp/4 $tmp/2 | cut -d"=" -f2`
Check_exact $v 0
Pass fiezw
Pass iezw

Fezw $tmp/1 > $tmp/2 || Incerr
v=`tail -1 $tmp/2 | cut -f2`
Check_approx $v 33.62
Pass Fezw

echo


# compression/lossless

mwecho -n "compression/lossless: "

funzoom -ftype IMG -z 4 -o 0 cimage $tmp/1 2> /dev/null
arencode2 $tmp/1 $tmp/2 > $tmp/3 || Incerr
v=`grep Rate $tmp/3 | cut -d"=" -f2`
Check_approx $v 7.57324
v=`fsize $tmp/2` 
Check_exact "$v" "3878 1"
Pass arencode2

arencode2 -H $tmp/1 $tmp/2 > /dev/null || Incerr
ardecode2 -r 256 $tmp/2 $tmp/3 > /dev/null || Incerr
Pass ardecode2

cvsencode france.crv > $tmp/1 || Incerr
v=`grep N $tmp/1 | cut -d"=" -f2`
Check_exact $v 7887
v=`grep B $tmp/1 | cut -d"=" -f2`
Check_exact $v 10098
Pass cvsencode

cvsfrecode france.crv > $tmp/1 || Incerr
v=`grep N $tmp/1 | cut -d"=" -f2`
Check_exact $v 7887
v=`grep B $tmp/1 | cut -d"=" -f2`
Check_exact $v 10098
Pass cvsfrecode

cvsorgcode france.crv > $tmp/1 || Incerr
v=`grep N $tmp/1 | cut -d"=" -f2`
Check_exact $v 7887
Pass cvsorgcode

funzoom -ftype IMG -z 4 -o 0 cimage $tmp/1 2> /dev/null
fencode $tmp/1 > $tmp/2 || Incerr
v=`grep brate $tmp/2 | cut -d"=" -f2`
Check_approx $v 7.34399
Pass fencode

echo


# compression/scalar

mwecho -n "compression/scalar: "

fscalq -p -n 10 cimage $tmp/1 > $tmp/2 || Incerr
v=`cat $tmp/2`
#Check_exact "$v" "..."
v=`fentropy $tmp/1 | cut -d"=" -f2`
Check_approx $v 2.91987
Pass fscalq

fscalq -p -n 10 -o $tmp/2 cimage $tmp/1 > /dev/null || Incerr
fiscalq $tmp/2 $tmp/3 > /dev/null || Incerr
fdiff $tmp/1 $tmp/3 $tmp/2
v=`fnorm -p 2 $tmp/2 | cut -d"=" -f2`
Check_exact $v 0
Pass fiscalq

echo


# compression/vector

mwecho -n "compression/vector: "

mk_codebook $tmp/1 || Incerr
v=`fsize $tmp/1`
Check_exact "$v" "4 6"
Pass mk_codebook

NoPass fivq
NoPass flbg_adap
NoPass flbg
NoPass flbg_train
NoPass fvq
NoPass mk_trainset

echo


# compression/vqwave

mwecho -n "compression/vqwave: "

NoPass Cfwivq
NoPass Cfwvq
NoPass Fwivq
NoPass fwivq
NoPass Fwlbg_adap
NoPass fwlbg_adap
NoPass Fwvq
NoPass fwvq
NoPass wlbg_adap

echo


# curve

mwecho -n "curve: "

area curve > $tmp/1 || Incerr
v=`cut -d"=" -f2 $tmp/1`
Check_exact $v 209718
Pass area

perimeter curve > $tmp/1 || Incerr
v=`cut -d"=" -f2 $tmp/1`
Check_exact $v 8422
Pass perimeter

circle -r 10 -n 100 $tmp/1 || Incerr
v=`area $tmp/1 | cut -d"=" -f2`
Check_approx $v 313.953
v=`perimeter $tmp/1 | cut -d"=" -f2`
Check_approx $v 62.8215
Pass circle

disc $tmp/1 2.5 || Incerr
v=`perimeter $tmp/1 | cut -d"=" -f2`
Check_approx $v 30.5708
Pass disc

dsplit_convex -c . curve $tmp/1 > $tmp/2 || Incerr
v=`cut -d"=" -f2 $tmp/2`
Check_exact $v 1866
Pass dsplit_convex

fsplit_convex -c . curve $tmp/1 > $tmp/2 || Incerr
v=`cut -d"=" -f2 $tmp/2`
Check_exact $v 1866
Pass fsplit_convex

extract_connex cimage $tmp/1 || Incerr
v=`dkinfo $tmp/1 | grep "Number of curves" | cut -d":" -f2`
Check_exact $v 122
Pass extract_connex

fillpoly -x 1100 -y 1100 france.crv $tmp/1 || Incerr
v=`fmean $tmp/1 | cut -d"=" -f2`
Check_approx $v 136.473
Pass fillpoly

echo "10 10 60 20 30 50 e 100 10 110 50 150 20 q" \
| flreadasc 2 $tmp/1 > /dev/null 
fillpolys -x 160 -y 60 $tmp/1 $tmp/1 || Incerr
v=`fmean $tmp/1 | cut -d"=" -f2`
Check_approx $v 201.423
Pass fillpolys

fkbox curve > $tmp/1 || Incerr
v=`grep xmin $tmp/1 | cut -d"=" -f2`
Check_exact $v 63
v=`grep ymin $tmp/1 | cut -d"=" -f2`
Check_exact $v -912
v=`grep xmax $tmp/1 | cut -d"=" -f2`
Check_exact $v 692
v=`grep ymax $tmp/1 | cut -d"=" -f2`
Check_exact $v -123
Pass fkbox

fkcenter curve > $tmp/1 || Incerr
v=`grep xg $tmp/1 | cut -d"=" -f2`
Check_approx $v 336.298
v=`grep yg $tmp/1 | cut -d"=" -f2`
Check_approx $v -467.331
Pass fkcenter

fkcrop 0 -100 700 -200 curve $tmp/1 || Incerr
v=`dkinfo $tmp/1 | grep "Number of curves" | cut -d":" -f2`
Check_exact $v 5
v=`fkcenter $tmp/1 | grep xg | cut -d"=" -f2`
Check_approx $v 324.922
Pass fkcrop

fkzrt curve  $tmp/1 0.5 30 20 20 || Incerr
v=`fkcenter $tmp/1 | grep xg | cut -d"=" -f2`
Check_approx $v 282.455
Pass fkzrt

echo "40 40 60 30 100 100 200 80 80 240 e 240 20 280 20 260 240 200 150 240 40 q" | fkreadasc $tmp/1 > /dev/null
ksplines -j 3 $tmp/1 $tmp/2 || Incerr
v=`dkinfo $tmp/2 | grep "Number of curves" | cut -d":" -f2`
Check_exact $v 2
Pass ksplines

echo "40 40 60 30 100 100 200 80 80 240 q" | fkreadasc $tmp/1 > /dev/null
kspline -j 3 $tmp/1 $tmp/2 || Incerr
v=`perimeter $tmp/2 | cut -d"=" -f2`
Check_approx $v 318.806
Pass kspline

echo "0.1 0.2" | sreadasc $tmp/1 2
flscale -ftype MW2_FCURVE curve $tmp/1 $tmp/2 || Incerr
perimeter $tmp/2 > $tmp/3 
v=`cut -d"=" -f2 $tmp/3`
Check_exact $v 1313
area $tmp/2 > $tmp/3
v=`cut -d"=" -f2 $tmp/3`
Check_exact $v 4194.36
Pass flscale

flconcat -ftype MW2_FCURVES $tmp/2 curve $tmp/3
fkbox $tmp/3 | grep ymax > $tmp/4
v=`cut -d"=" -f2 $tmp/4`
Check_exact $v -24.6
Pass flconcat

echo


# curve/io

mwecho -n "curve/io: "

echo "10 10 60 20 30 50 e 40 30 41 31 41 20 q" \
| fkreadasc $tmp/1 > /dev/null || Incerr
v=`dkinfo $tmp/1 | grep "Number of curves" | cut -d":" -f2`
Check_exact $v 2
v=`perimeter $tmp/1 | cut -d"=" -f2`
Check_approx $v 93.4166
Pass fkreadasc

echo "10 10 60 20 30 50 e 40 30 41 31 41 20 q" \
| flreadasc 2 $tmp/1 > /dev/null || Incerr
v=`dkinfo $tmp/1 | grep "Number of curves" | cut -d":" -f2`
Check_exact $v 2
v=`perimeter $tmp/1 | cut -d"=" -f2`
Check_approx $v 93.4166
Pass flreadasc

kplot curve $tmp/1 || Incerr
v=`fsize $tmp/1`
Check_exact "$v" "630 790"
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 1.90955
Pass kplot

fkplot -s curve $tmp/1 || Incerr
v=`fsize $tmp/1`
Check_exact "$v" "630 790"
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 1.90955
Pass fkplot

fkprintasc curve > /dev/null || Incerr
v=`fkprintasc curve | wc -l`
Check_exact $v 4115
Pass fkprintasc

flprintasc curve > /dev/null || Incerr
v=`flprintasc curve | wc -l`
Check_exact $v 4115
Pass flprintasc

fkprintfig curve > /dev/null || Incerr
v=`fkprintfig curve | wc -w`
Check_exact $v 8255
Pass fkprintfig

echo "10 10 60 20 30 50 e 40 30 41 31 41 20 q" \
| flreadasc 2 $tmp/1 > /dev/null
fkprintfig $tmp/1 | kreadfig $tmp/2 > /dev/null || Incerr
fkzrt $tmp/2 $tmp/2 0.001 0 0 0
v=`dkinfo $tmp/2 | grep "Number of curves" | cut -d":" -f2`
Check_exact $v 2
v=`area $tmp/2 | cut -d"=" -f2`
Check_exact $v 3600
Pass kreadfig

fkview -o $tmp/1 -n curve || Incerr
v=`fnorm -v $tmp/1 2> /dev/null | cut -d"=" -f2`
Check_approx $v 2.61157
Pass fkview

echo "1 1 6 2 3 5 4 3 4 4 4 2 q" | fkreadasc $tmp/1 > /dev/null
kplotasc $tmp/1 > /dev/null || Incerr
v=`kplotasc $tmp/1 | wc -c`
Check_exact $v 84
Pass kplotasc

NoPass readpoly

# common: dkinfo

echo


# curve/smooth

mwecho -n "curve/smooth: "

fksmooth -n 20 -s 10 curve $tmp/1 || Incerr
v=`area $tmp/1 | cut -d"=" -f2`
Check_approx $v 204306
v=`perimeter $tmp/1 | cut -d"=" -f2`
Check_approx $v 3128.84
Pass fksmooth

iter_fksmooth -N 2 -n 10 -s 10 curve $tmp/1 || Incerr
v=`dkinfo $tmp/1 | grep "Number of curves" | cut -d":" -f2`
Check_exact $v 3
Pass iter_fksmooth

gass -l 10 curve $tmp/1 || Incerr
v=`area $tmp/1 | cut -d"=" -f2`
Check_approx $v 207081
v=`perimeter $tmp/1 | cut -d"=" -f2`
Check_approx $v 3914.79
Pass gass

iter_gass -N 2 -S 10 curve $tmp/1 || Incerr
v=`dkinfo $tmp/1 | grep "Number of curves" | cut -d":" -f2`
Check_exact $v 3
Pass iter_gass

gcsf -l 10 curve $tmp/1 || Incerr
v=`area $tmp/1 | cut -d"=" -f2`
Check_approx $v 209003
v=`perimeter $tmp/1 | cut -d"=" -f2`
Check_approx $v 3929.08
Pass gcsf

iter_gcsf -N 2 -l 20 curve $tmp/1 || Incerr
v=`dkinfo $tmp/1 | grep "Number of curves" | cut -d":" -f2`
Check_exact $v 3
Pass iter_gcsf

echo


# curve/matching

mwecho -n "curve/matching: "

gass -l 3 -e 2 curve $tmp/1
km_inflexionpoints $tmp/1 $tmp/2 || Incerr
v=`flprintasc $tmp/2 | wc -l`
Check_exact $v 43
Pass km_inflexionpoints

km_bitangents $tmp/1 $tmp/2 $tmp/3 || Incerr
v=`flprintasc $tmp/3 | wc -l`
Check_exact $v 20
Pass km_bitangents

km_flatpoints $tmp/1 $tmp/2 $tmp/4 0.1 0.1 || Incerr
v=`flprintasc $tmp/4 | wc -l`
Check_exact $v 25
Pass km_flatpoints

NoPass km_codecurve_ai
NoPass km_codecurve_si
NoPass km_createdict_ai
NoPass km_createdict_si
NoPass KM_DEMO
NoPass km_match_ai
NoPass km_match_si
NoPass km_prematchings
NoPass km_savematchings

echo


# examples

mwecho -n "examples: "

demohead1 cimage $tmp/1 > /dev/null || Incerr
v=`fsize $tmp/1`
Check_exact "$v" "1 1"
Pass demohead1

demohead2 > /dev/null || Incerr
Pass demohead2

demohead3 cimage $tmp/1 > /dev/null || Incerr
v=`fnorm -p 2 -c cimage $tmp/1 | cut -d"=" -f2`
Check_exact $v 0
Pass demohead3

fadd cimage fimage $tmp/1
fadd1 cimage fimage $tmp/2 || Incerr
v=`fnorm -p 2 -c $tmp/1 $tmp/2 | cut -d"=" -f2`
Check_exact $v 0
Pass fadd1

fadd2 cimage fimage $tmp/2 || Incerr
v=`fnorm -p 2 -c $tmp/1 $tmp/2 | cut -d"=" -f2`
Check_exact $v 0
Pass fadd2

fadd3 cimage fimage $tmp/2 || Incerr
v=`fnorm -p 2 -c $tmp/1 $tmp/2 | cut -d"=" -f2`
Check_exact $v 0
Pass fadd3

fadd4 cimage fimage $tmp/2 || Incerr
v=`fnorm -p 2 -c $tmp/1 $tmp/2 | cut -d"=" -f2`
Check_exact $v 0
Pass fadd4

make_cmovie $tmp/1 || Incerr
v=`fsize $tmp/1_01`
Check_exact "$v" "256 256"
v=`grep nimage $tmp/1 | cut -d":" -f2`
Check_exact $v 20
Pass make_cmovie

make_fmovie $tmp/1 || Incerr
v=`fsize $tmp/1_01`
Check_exact "$v" "256 256"
v=`grep nimage $tmp/1 | cut -d":" -f2`
Check_exact $v 21
Pass make_fmovie

make_ccmovie $tmp/1 || Incerr
ccopy $tmp/1_01 $tmp/2 2> /dev/null
v=`fsize $tmp/2`
Check_exact "$v" "256 256"
v=`grep nimage $tmp/1 | cut -d":" -f2`
Check_exact $v 20
Pass make_ccmovie

make_cfmovie $tmp/1 || Incerr
ccopy $tmp/1_01 $tmp/2 2> /dev/null
v=`fsize $tmp/2`
Check_exact "$v" "256 256"
v=`grep nimage $tmp/1 | cut -d":" -f2`
Check_exact $v 20
Pass make_cfmovie

make_cimage $tmp/1 || Incerr
v=`fsize $tmp/1`
Check_exact "$v" "256 256"
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 0.992188
Pass make_cimage

NoPass view_demo

echo


# image/detection

mwecho -n "image/detection: "

funzoom -o 0 -z 4 cimage $tmp/1

canny $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 13.5183
Pass canny

falign -e 4.3 $tmp/1 $tmp/2 > /dev/null || Incerr
v=`flprintasc $tmp/2 | wc -l`
Check_exact $v 14
Pass falign

falign_mdl -e -2 -l 1 -n 30 cimage $tmp/2 > /dev/null || Incerr
v=`fsize $tmp/2`
Check_exact "$v" "6 43"
Pass falign_mdl

vpoint cimage $tmp/2 $tmp/3 > /dev/null || Incerr
v=`flprintasc $tmp/3 | cut -d" " -f2`
Check_approx $v 17.9527
Pass vpoint

ll_boundaries -e 11 $tmp/1 $tmp/2 > /dev/null || Incerr
v=`flprintasc $tmp/2 | wc -l`
Check_exact $v 1139
Pass ll_boundaries

ll_boundaries2 -e 11 $tmp/1 $tmp/2 > /dev/null || Incerr
v=`flprintasc $tmp/2 | wc -l`
Check_exact $v 1073
Pass ll_boundaries2

ll_edges -e 17 $tmp/1 $tmp/2 > /dev/null || Incerr
v=`flprintasc $tmp/2 | wc -l`
Check_exact $v 526
Pass ll_edges

harris $tmp/1 $tmp/2 || Incerr
v=`flprintasc $tmp/2 | wc -l`
Check_approx $v 55
Pass harris

NoPass vpsegplot
NoPass VP_DEMO

echo


# image/domain

mwecho -n "image/domain: "

fcrop -x 20 -y 20 -o 3 cimage $tmp/1 40 100 50 110 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 3.21633
Pass fcrop

cccrop -x 20 -y 20 -o 3 ccimage $tmp/1 40 100 50 110 2> /dev/null || Incerr
cfgetchannels $tmp/1 $tmp/2 $tmp/3 $tmp/4
fdiff $tmp/3 $tmp/2 $tmp/4
v=`fnorm -v $tmp/4 | cut -d"=" -f2`
Check_approx $v 6.01568
Pass cccrop

cextract cimage $tmp/1 40 100 50 110 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 5.06073
Pass cextract

fextract cimage $tmp/1 40 100 50 110 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 5.06073
Pass fextract

ccextract ccimage $tmp/1 40 100 50 110 || Incerr
cfgetchannels $tmp/1 $tmp/2 $tmp/3 $tmp/4
fdiff $tmp/3 $tmp/2 $tmp/4
v=`fnorm -v $tmp/4 | cut -d"=" -f2`
Check_approx $v 6.9226
Pass ccextract

clocal_zoom -x 100 -y 150 -W 64 -X 3 cimage $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 8.07492
Pass clocal_zoom

flocal_zoom -x 100 -y 150 -W 64 -X 3 cimage $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 8.07492
Pass flocal_zoom

cclocal_zoom -x 100 -y 150 -W 64 -X 3 ccimage $tmp/1 || Incerr
cfgetchannels $tmp/1 $tmp/2 $tmp/3 $tmp/4
fdiff $tmp/3 $tmp/2 $tmp/4
v=`fnorm -v $tmp/4 | cut -d"=" -f2`
Check_approx $v 8.50785
Pass cclocal_zoom

funzoom -o 0 -z 4 cimage $tmp/1

fshift -h $tmp/1 $tmp/2 || Incerr
v=`fnorm -p 2 -c $tmp/1 $tmp/2 | cut -d"=" -f2`
Check_approx $v 110.12
Pass fshift

czoom -X 4 -o 5 $tmp/1 $tmp/2 2> /dev/null || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 5.97412
Pass czoom

cczoom -X 4 -o 5 $tmp/1 $tmp/2 2> /dev/null || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 5.97412
Pass cczoom

fzoom -X 4 -o 5 $tmp/1 $tmp/2 2> /dev/null || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 5.969
Pass fzoom

csample cimage $tmp/1 4 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 19.2588
Pass csample

fsample cimage $tmp/1 4 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 19.2588
Pass fsample

cextcenter -f 27 cimage $tmp/1 || Incerr
v=`fsize $tmp/1`
Check_exact "$v" "243 243"
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 8.76071
Pass cextcenter

cfextcenter -ftype IMG -f 27 ccimage $tmp/1 2> /dev/null || Incerr
v=`fsize $tmp/1` 
Check_exact "$v" "243 243"
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 15.6567
Pass cfextcenter

fmaskrot -s 30 -b 10 cimage $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 6.09773
Pass fmaskrot

fproj -x 100 -y 120 -o 3 cimage $tmp/1 10 20 250 40 80 210 130 200 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 9.06184
Pass fproj

fzrt -o 3 cimage $tmp/1 1.1 57 -10 -20 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 7.30864
Pass fzrt

frot -a 35 cimage $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 5.02289
Pass frot

fdirspline cimage 5 $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 6.31651
Pass fdirspline

finvspline cimage 5 $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 41.7204
Pass finvspline

ccopy cimage $tmp/1_1
ccopy fimage $tmp/1_2
Mkmovie Cmovie $tmp/1 1 2

cmzoom -o 3 -X 2 $tmp/1 $tmp/2 2> /dev/null || Incerr
fdiff $tmp/2_01 $tmp/2_02 $tmp/3
v=`fnorm -v $tmp/3 | cut -d"=" -f2`
Check_approx $v 9.81426
Pass cmzoom

ccmzoom -o 3 -X 2 $tmp/1 $tmp/2 2> /dev/null || Incerr
fdiff $tmp/2_01 $tmp/2_02 $tmp/3
v=`fnorm -v $tmp/3 | cut -d"=" -f2`
Check_approx $v 9.81426
Pass ccmzoom

cmextract -b 0 $tmp/2 $tmp/3 30 30 1 170 170 1 $tmp/1 50 50 2 || Incerr
fdiff $tmp/3_01 $tmp/3_02 $tmp/4
v=`fnorm -v $tmp/4 | cut -d"=" -f2`
Check_approx $v 8.09011
Pass cmextract

cmparitysep -l $tmp/1 $tmp/2 || Incerr
fdiff $tmp/2_01 $tmp/2_03 $tmp/3
fdiff $tmp/2_02 $tmp/2_04 $tmp/4
v=`fnorm -v $tmp/3 | cut -d"=" -f2`
Check_approx $v 14.6972
v=`fnorm -v $tmp/4 | cut -d"=" -f2`
Check_approx $v 14.74
Pass cmparitysep

NoPass cmcollect
NoPass ccmcollect

# common: funzoom cfunzoom

echo


# image/filter

mwecho -n "image/filter: "

cfunzoom -z 4 -o 0 ccimage $tmp/1
cfdiffuse $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 2> /dev/null | cut -d"=" -f2`
Check_approx $v 9.08767
Pass cfdiffuse

cfmdiffuse -n 2 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2_01 2> /dev/null | cut -d"=" -f2`
Check_approx $v 9.08767
v=`fnorm -v $tmp/2_02 2> /dev/null | cut -d"=" -f2`
Check_approx $v 7.25923
Pass cfmdiffuse

funzoom -z 4 -o 0 -ftype IMG cimage $tmp/1 2> /dev/null

erosion -r 1.5 -n 1 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 13.8314
Pass erosion

opening -r 1.5 -n 1 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 13.77
Pass opening

median -r 1.5 -n 1 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 13.7447
Pass median

amss -l 2 -d $tmp/2 $tmp/1 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 11.838
Pass amss

fquant $tmp/1 $tmp/2 5 > /dev/null
osamss -l 2 $tmp/2 $tmp/3 || Incerr
v=`fnorm -v $tmp/3 | cut -d"=" -f2`
Check_approx $v 11.8249
Pass osamss

heat -n 10 -s 0.1 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 10.777
Pass heat

fsmooth -S 2 -W 1 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 7.20909
Pass fsmooth

echo "-1 1 -1 1" | freadasc $tmp/2 2 2
fconvol $tmp/1 $tmp/2 $tmp/3 || Incerr
v=`fnorm -v $tmp/3 | cut -d"=" -f2`
Check_approx $v 26.6814
Pass fconvol

fsepconvol -g 2 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 9.13969
Pass fsepconvol

fgrain -a 20 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 13.8573
Pass fgrain

forder -e 5 -n 1 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 13.697
Pass forder

fsharpen -p 50 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 22.7716
Pass fsharpen

rotaffin -r 5 -a 3 -t 3 -T 0 -A 5 $tmp/2 || Incerr
v=`grep nimage $tmp/2 | cut -d":" -f2`
Check_exact $v 15
v=`fnorm -v $tmp/2_10 | cut -d"=" -f2`
Check_approx $v 29.1366
Pass rotaffin

infsup -n 2 $tmp/1 $tmp/2 $tmp/3 || Incerr
v=`fnorm -v $tmp/3 | cut -d"=" -f2`
Check_approx $v 11.2894
Pass infsup

ll_sharp -p 20 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 16.9317
Pass ll_sharp

resthline $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 16.5496
Pass resthline

shock -n 10 -s 0.1 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 19.0514
Pass shock

tvdenoise $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 14.5648
Pass tvdenoise

tvdenoise2 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 14.4665
Pass tvdenoise2

nlmeans -s 3 -d 5 cimage $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 6.45733
Pass nlmeans

fconvol $tmp/1 blur3x3.ir $tmp/2
tvdeblur -n 30 $tmp/2 blur3x3.ir $tmp/3 || Incerr
v=`fnorm -p 2 -c $tmp/1 $tmp/3 | cut -d"=" -f2`
Check_approx $v 7.51181
Pass tvdeblur

cmextract cmovie $tmp/1 128 128 3 140 140 7
mam -n 20 -a 0 $tmp/1 $tmp/2 > /dev/null || Incerr
v=`fnorm -v $tmp/2_03 | cut -d"=" -f2`
Check_approx $v 8.49737
Pass mam

prolate -n 128 3 0.5 $tmp/1 > /dev/null || Incerr
v=`fsize $tmp/1` 
Check_exact "$v" "3 3"
v=`fmean $tmp/1 | cut -d"=" -f2`
Check_approx $v 0.111111
Pass prolate

NoPass cfsharpen 
NoPass flipschitz
NoPass prolatef

echo


# image/fourier

mwecho -n "image/fourier: "

fft2d -A $tmp/1 -B $tmp/2 cimage  || Incerr
fextract $tmp/1 $tmp/1 20 20 230 230
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 1234.21
fextract $tmp/2 $tmp/2 20 20 230 230
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 1230.29
Pass fft2d

fft2dpol -M $tmp/1 -P $tmp/2 cimage  || Incerr
fextract $tmp/1 $tmp/1 20 20 230 230
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 784.174
fextract $tmp/2 $tmp/2 20 20 230 230
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 1.65156
Pass fft2dpol

fft2drad -l -s 100 cimage $tmp/1 || Incerr
v=`snorm -b 5 -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 0.0241473
v=`snorm -b 5 -p 2 $tmp/1 | cut -d"=" -f2`
Check_approx $v 3.48635
Pass fft2drad

fft2dview -t 0 -o $tmp/1 cimage || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 1923.05
Pass fft2dview

fftgrad -n $tmp/1 cimage || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 8.00567
Pass fftgrad

fftrot -a 33 cimage $tmp/1  || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 9.72568
Pass fftrot

funzoom -z 4 -o 0 cimage $tmp/1
fftzoom -z 2 $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 10.8767
Pass fftzoom

fhamming cimage $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 2.74239
Pass fhamming

frandphase fimage $tmp/1 || Incerr
fft2dpol -P $tmp/2 $tmp/1
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_more_approx $v 1.63
Pass frandphase

fextract cimage $tmp/1 10 10 200 210 
fft2dshrink $tmp/1 $tmp/2 || Incerr
v=`fsize $tmp/2` 
Check_exact "$v" "189 200"
Pass fft2dshrink

fshrink2 $tmp/1 $tmp/2 || Incerr
v=`fsize $tmp/2` 
Check_exact "$v" "128 128"
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 8.33146
Pass fshrink2

fsym2 $tmp/2 $tmp/2 || Incerr
v=`fsize $tmp/2` 
Check_exact "$v" "256 256"
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 8.36696
Pass fsym2

wiener -W 0.1 -g 1 cimage $tmp/1 || Incerr
v=`fnorm -b 10 -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 11.1962
Pass wiener

fkeepphase cimage fimage $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 14.041
Pass fkeepphase

faxpb -a 0 -b 0 cimage $tmp/1
fpset $tmp/1 0 0 1 $tmp/1
fsepconvol -b 2 -g 3 $tmp/1 $tmp/2 
fft2d -A $tmp/3 $tmp/2
fftconvol cimage $tmp/3 $tmp/4 || Incerr
fsepconvol -b 2 -g 3 cimage $tmp/5
v=`fnorm -t 0.0001 -p 2 -c $tmp/4 $tmp/5 | cut -d"=" -f2`
Check_exact $v 0
Pass fftconvol

echo


# image/io

mwecho -n "image/io: "

ccopy cimage $tmp/1 || Incerr
fdiff cimage $tmp/1 $tmp/1
v=`fnorm -p 2 $tmp/1 | cut -d"=" -f2`
Check_exact $v 0
Pass ccopy

fcopy fimage $tmp/1 || Incerr
fdiff fimage $tmp/1 $tmp/1
v=`fnorm -p 2 $tmp/1 | cut -d"=" -f2`
Check_exact $v 0
Pass fcopy

cccopy ccimage $tmp/1 || Incerr
cfdiff ccimage $tmp/1 $tmp/1
cfgetchannels $tmp/1 $tmp/2 $tmp/3 $tmp/4
v=`fnorm -p 2 $tmp/2 | cut -d"=" -f2`
Check_exact $v 0
v=`fnorm -p 2 $tmp/3 | cut -d"=" -f2`
Check_exact $v 0
v=`fnorm -p 2 $tmp/4 | cut -d"=" -f2`
Check_exact $v 0
Pass cccopy

NoPass cview
NoPass fview
NoPass ccview
NoPass cmview
NoPass ccmview
NoPass flip

fconst $tmp/1 0 60 20
echo "hi guys..." | ccputstring -r 3 -c 900 -C 90 $tmp/1 10 1 $tmp/1 || Incerr
ccopy $tmp/1 $tmp/1 2> /dev/null
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 19.8406
Pass ccputstring

cfgetchannels ccimage $tmp/1 $tmp/2 $tmp/3
cfputchannels $tmp/1 $tmp/2 $tmp/3 $tmp/4 || Incerr
cfdiff -ftype IMG ccimage $tmp/4 $tmp/1 2> /dev/null
v=`fnorm -p 2 $tmp/1 | cut -d"=" -f2`
Check_exact $v 0
Pass cfputchannels

cfchgchannels ccimage $tmp/1 || Incerr
cfgetchannels $tmp/1 $tmp/2 $tmp/3 $tmp/4
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 7.36361
v=`fnorm -v $tmp/3 | cut -d"=" -f2`
Check_approx $v 15.9369
v=`fnorm -v $tmp/4 | cut -d"=" -f2`
Check_approx $v 9.98046
Pass cfchgchannels

cline_extract cimage $tmp/1 30 || Incerr
v=`snorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 11.1133
Pass cline_extract

fline_extract cimage $tmp/1 30 || Incerr
v=`snorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 11.1133
Pass fline_extract

echo "1 5 2 3" | creadasc $tmp/1 2 2 || Incerr
v=`fmean $tmp/1 | cut -d"=" -f2`
Check_approx $v 2.75
v=`fvar $tmp/1 | cut -d"=" -f2`
Check_approx $v 2.91667
Pass creadasc

cprintasc $tmp/1 > $tmp/2 || Incerr
v=`cat $tmp/2 | wc -l`
Check_exact $v 1
Pass cprintasc

fprintasc $tmp/1 > $tmp/2 || Incerr
v=`cat $tmp/2 | wc -l`
Check_exact $v 1
Pass fprintasc

#common: freadasc Mkmovie cfgetchannels

echo


# image/level_lines

mwecho -n "image/level_lines: "

NoPass flst
NoPass flst_pixels
NoPass flst_reconstruct
NoPass flst_boundary

NoPass flst_bilinear
NoPass flstb_boundary
NoPass flstb_dual
NoPass flstb_dualchain
NoPass flstb_quantize
NoPass flstb_tv

NoPass fml_ml
NoPass fsaddles
NoPass ll_distance
NoPass ll_extract
NoPass llmap
NoPass ll_remove
NoPass llremove
NoPass llview

NoPass ml_decompose
NoPass ml_draw
NoPass ml_extract
NoPass ml_fml
NoPass ml_reconstruct

NoPass mscarea
NoPass tjmap
NoPass tjpoint

NoPass cll_remove
NoPass cml_decompose
NoPass cml_draw
NoPass cml_reconstruct

echo


# image/misc

mwecho -n "image/misc: "

cdisc $tmp/1 100 100 || Incerr
v=`fmean $tmp/1 | cut -d"=" -f2`
Check_approx $v 126.888
Pass cdisc

funzoom -z 4 -o 0 -ftype IMG cimage $tmp/1 2> /dev/null
cdisc -r 16 $tmp/2 64 64
binarize -i $tmp/2 $tmp/2
fmask $tmp/3 $tmp/2 $tmp/2 $tmp/1
fmask -i -c 1 $tmp/2 $tmp/2 $tmp/2 
disocclusion $tmp/3 $tmp/2 $tmp/4 > $tmp/1 || Incerr
v=`grep energy $tmp/1 | cut -d"=" -f2`
#Check_approx $v 980.02
v=`fnorm -v $tmp/4 | cut -d"=" -f2`
#Check_approx $v 14.66
Pass disocclusion

NoPass drawocclusion

echo "10 10 60 20 30 50 e 100 10 110 50 150 20 q" \
| flreadasc 2 $tmp/1 > /dev/null 
fillpolys -x 160 -y 60 $tmp/1 $tmp/1 
emptypoly $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 12.4411
Pass emptypoly

binarize -i -t 120 cimage $tmp/1
thinning $tmp/1 $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 17.7433
Pass thinning

funzoom -z 4 -o 0 -ftype IMG cimage $tmp/1 2> /dev/null
binarize -i -t 120 $tmp/1 $tmp/1
skeleton -n 10 $tmp/1 seg_mask $tmp/2 > /dev/null || Incerr
v=`grep nimage $tmp/2 | cut -d":" -f2`
Check_exact $v 11
v=`fnorm -v $tmp/2_11 | cut -d"=" -f2`
Check_approx $v 32.5981
Pass skeleton

NoPass lsnakes
NoPass lsnakes_demo
NoPass mac_snakes
NoPass ccdisocclusion

echo


# image/operations

mwecho -n "image/operations: "

fop -p -A cimage cimage $tmp/1 || Incerr
v=`fnorm -p 2 $tmp/1 | cut -d"=" -f2`
Check_approx $v 287.596
Pass fop

faxpb -a -1 cimage $tmp/1
fabso $tmp/1 $tmp/2
fdiff $tmp/2 cimage $tmp/3
v=`fnorm -p 2 $tmp/3 | cut -d"=" -f2`
Check_exact $v 0
Pass fabso

fentropy cimage > $tmp/1 || Incerr
v=`cat $tmp/1 | cut -d"=" -f2`
Check_approx $v 7.51668
Pass fentropy

fderiv -n $tmp/1 cimage || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 3.85916
Pass fderiv

finfo cimage > $tmp/1 || Incerr
v=`grep "bv norm" $tmp/1 | cut -d"=" -f2`
Check_approx $v 8.732667
Pass finfo

fmse -n cimage fimage > $tmp/1 || Incerr
v=`grep "^SNR" $tmp/1 | cut -d"=" -f2`
Check_approx $v -3.48774
v=`grep PSNR $tmp/1 | cut -d"=" -f2`
Check_approx $v 8.10192
v=`grep MSE $tmp/1 | cut -d"=" -f2`
Check_approx $v 2.23241
v=`grep MRD $tmp/1 | cut -d"=" -f2`
Check_approx $v 89.7111
Pass fmse

cdisc $tmp/1 256 256
fmask $tmp/2 $tmp/1 cimage fimage || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 11.5658
Pass fmask

fpsnr255 fimage > $tmp/1 || Incerr
v=`cat $tmp/1 | cut -d"=" -f2`
Check_approx $v 0.347482
Pass fpsnr255

frthre -l 100 cimage $tmp/1 || Incerr
v=`fmean $tmp/1 | cut -d"=" -f2`
Check_approx $v 44.9395
Pass frthre

#common: faxpb fpset cfdiff fadd fconst fdiff fmean fnorm fsize fvar

echo


# image/seg

mwecho -n "image/seg: "

Pass one_levelset
Pass segct
Pass msegct
Pass mschannel
Pass segtxt

echo


# image/shape_recognition

mwecho -n "image/shape_recognition: "

Pass CLEAR_BASE
Pass PUT_TO_BASE
Pass READ_BASE
Pass RECOGNIZE
Pass SR_DEMO
Pass sr_distance
Pass sr_genhypo
Pass sr_normalize
Pass sr_signature

echo


# image/values

mwecho -n "image/values: "

binarize -t 150 cimage $tmp/1 || Incerr
v=`fmean $tmp/1 | cut -d"=" -f2`
Check_approx $v 77.9132
Pass binarize

funzoom -z 8 cimage $tmp/1
fquant $tmp/1 $tmp/1 5 > $tmp/2
v=`cut -d"=" -f2 $tmp/2`
amle_init $tmp/1 $v $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 48.7051
Pass amle_init

amle $tmp/2 $tmp/3 2> /dev/null || Incerr
v=`fnorm -v $tmp/3 | cut -d"=" -f2`
Check_approx $v 17.4285
Pass amle

cmextract cmovie $tmp/1 40 170 3 80 210 7
for v in 1 2 3 4 5; do
faxpb -ftype IMG -a 0.1 $tmp/1_0$v $tmp/1_0$v 2> /dev/null
faxpb -ftype IMG -a 10 -b 5 $tmp/1_0$v $tmp/1_0$v 2> /dev/null
done
amle3d_init $tmp/1 10 $tmp/2 || Incerr
v=`fnorm -v $tmp/2_03 | cut -d"=" -f2`
Check_approx $v  26.1018
Pass amle3d_init

amle3d $tmp/2 $tmp/3 || Incerr
v=`fnorm -v $tmp/3_03 | cut -d"=" -f2`
Check_approx $v 4.03325
Pass amle3d

fvalues -r $tmp/1 cimage $tmp/2 || Incerr
v=`grep size $tmp/2 | cut -d":" -f2`
Check_approx $v 256
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 8.73267
Pass fvalues

ccontrast cimage $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 12.5844
Pass ccontrast

ccontrast_local -d 2 cimage $tmp/1 || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 13.2157
Pass ccontrast_local

fconst $tmp/1 0 100 100
cnoise -i 50 $tmp/1 $tmp/1 || Incerr
v=`fmean $tmp/1 | cut -d"=" -f2`
Check_more_approx $v 64
Pass cnoise

fconst $tmp/1 0 100 100
fnoise -g 10 $tmp/1 $tmp/1 || Incerr
v=`fvar $tmp/1 | cut -d"=" -f2`
Check_more_approx $v 100
Pass fnoise

cmextract cmovie $tmp/1 10 10 1 210 210 10
cmnoise -i 50 $tmp/1 $tmp/2 || Incerr
v=`grep nimage $tmp/2 | cut -d":" -f2`
Check_exact $v 10
v=`fmean $tmp/2_05 | cut -d"=" -f2`
Check_more_approx $v 116
Pass cmnoise

chisto cimage $tmp/1 || Incerr
v=`snorm -p 2 $tmp/1 | cut -d"=" -f2`
Check_approx $v 366.222
Pass chisto

fhisto cimage $tmp/1 || Incerr
v=`snorm -p 2 $tmp/1 | cut -d"=" -f2`
Check_approx $v 929.198
Pass fhisto

flgamma -f 256 $tmp/1 || Incerr
v=`flprintasc $tmp/1 | grep "^246" | cut -d" " -f2`
Check_approx $v 236.391
Pass flgamma

fcontrast cimage $tmp/1 $tmp/2 || Incerr
v=`fnorm -p 2 $tmp/2 | cut -d"=" -f2`
Check_approx $v 103.585
Pass fcontrast

frank -r $tmp/1 cimage || Incerr
v=`fnorm -p 2 $tmp/1 | cut -d"=" -f2`
Check_approx $v 0.577342
Pass frank

fthre -N cimage $tmp/1 || Incerr
v=`fnorm -p 2 $tmp/1 | cut -d"=" -f2`
Check_approx $v 143.798
Pass fthre

NoPass cfquant
NoPass bicontrast

# common: fquant

echo


# signal

mwecho -n "signal: "

entropy fsignal > $tmp/1 || Incerr
v=`cut -d"=" -f2 $tmp/1`
Check_approx $v 11.0934
Pass entropy

sprintasc fsignal 101 101 > $tmp/1 || Incerr
Check_exact `cat $tmp/1` 3014
Pass sprintasc

sprintasc fsignal 1 123 | sreadasc $tmp/1 123
fft1dshrink $tmp/1 $tmp/2 || Incerr
v=`grep "size:" $tmp/2 | cut -d":" -f2`
Check_exact $v 121
Pass fft1dshrink

sshrink2 fsignal $tmp/1 || Incerr
v=`grep "size:" $tmp/1 | cut -d":" -f2`
Check_exact $v 2048
Pass sshrink2

fct1d $tmp/1 $tmp/2 || Incerr
v=`snorm -b 20 -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 3636.57
Pass fct1d

fft1d -A $tmp/2 $tmp/1 || Incerr
v=`snorm -b 20 -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 1888.35
Pass fft1d

sconst -s 256 -a 0.1 $tmp/1 || Incerr
v=`grep "size:" $tmp/1 | cut -d":" -f2`
Check_exact $v 256
v=`snorm -b 20 -v $tmp/1 | cut -d"=" -f2`
Check_exact $v 0
Pass sconst

sderiv fsignal $tmp/1 || Incerr
v=`snorm -b 20 -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 42.9396
Pass sderiv

sdirac -s 100 -a 100 $tmp/1 || Incerr
v=`snorm -b 0 -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 2
v=`snorm -b 0 -p 2 $tmp/1 | cut -d"=" -f2`
Check_approx $v 10
Pass sdirac

sgauss -s 20 $tmp/1 3 || lls
v=`snorm -b 0 -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 0.0130369
v=`snorm -b 0 -p 2 $tmp/1 | cut -d"=" -f2`
Check_approx $v 0.0686237
Pass sgauss

sintegral $tmp/1 $tmp/2 || Incerr
v=`snorm -b 0 -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 0.05
Pass sintegral

sderiv $tmp/1 $tmp/2
smse -n $tmp/1 $tmp/2 > $tmp/3 || Incerr
v=`grep "^SNR" $tmp/3 | cut -d"=" -f2`
Check_approx $v -1.9738
v=`grep PSNR $tmp/3 | cut -d"=" -f2`
Check_approx $v 6.66453
v=`grep MSE $tmp/3 | cut -d"=" -f2`
Check_approx $v 1.57536
v=`grep MRD $tmp/3 | cut -d"=" -f2`
Check_approx $v 70.6568
Pass smse

sconst -s 1000 -a 0 $tmp/1
snoise -g 1 $tmp/1 $tmp/2 || Incerr
v=`snorm -p 2 $tmp/2 | cut -d"=" -f2`
Check_more_approx $v 1
Pass snoise

sop -p -A fsignal fsignal $tmp/1 || Incerr
v=`snorm -p 2 $tmp/1 | cut -d"=" -f2`
Check_approx $v 5349.16
Pass sop

saxpb -a 2 fsignal $tmp/1 || Incerr
v=`snorm -p 2 $tmp/1 | cut -d"=" -f2`
Check_approx $v 5349.16
Pass saxpb

splot -ftype RIM -o $tmp/1 -n fsignal 2> /dev/null || Incerr
v=`fnorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 15.6218
Pass splot

echo "0 2 3 4 5 4 3 2 3 4 5 4 3 2 1 0" | sreadasc $tmp/1 16 || Incerr
v=`snorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 1
Pass sreadasc

ssinus -s 100 -a 1 -d 1 $tmp/1  || Incerr
v=`snorm -v $tmp/1 | cut -d"=" -f2`
Check_approx $v 0.0785888
Pass ssinus

sprintasc fsignal 1 1024 | sreadasc $tmp/1 1024
Swtvdenoise -D 10 -N 200 $tmp/1 $tmp/2 > /dev/null || Incerr
v=`snorm -p 2 -c $tmp/1 $tmp/2 | cut -d"=" -f2`
Check_approx $v 11.52
Pass Swtvdenoise

NoPass stvrestore
NoPass w1threshold
NoPass sinfo

# common: snorm

echo


# wave

mwecho -n "wave: "

owave1 -e 0 fsignal $tmp/1 wave/ortho/da02.ir || Incerr
v=`grep size $tmp/1_01_A.wtrans1d | cut -d":" -f2`
Check_exact $v 1104
v=`grep size $tmp/1_01_D.wtrans1d | cut -d":" -f2`
Check_exact $v 1104
Pass owave1

iowave1 -e 0 $tmp/1 $tmp/2 wave/ortho/da02.ir || Incerr
v=`snorm -t 0.001 -b 2 -p 2 -c fsignal $tmp/2 | cut -d"=" -f2`
Check_exact $v 0
Pass iowave1

biowave1 fsignal $tmp/1 wave/biortho/h/sp02.ir wave/biortho/htilde/sl05.ir || Incerr
v=`grep size $tmp/1_01_A.wtrans1d | cut -d":" -f2`
Check_exact $v 1104
v=`grep size $tmp/1_01_D.wtrans1d | cut -d":" -f2`
Check_exact $v 1104
Pass biowave1

ibiowave1 -e 0 $tmp/1 $tmp/2 wave/biortho/h/sp02.ir wave/biortho/htilde/sl05.ir || Incerr
v=`snorm -t 0.001 -b 2 -p 2 -c fsignal $tmp/2 | cut -d"=" -f2`
Check_exact $v 0
Pass ibiowave1

NoPass biowave2
NoPass dybiowave2
NoPass dyowave2
NoPass ibiowave2
NoPass iowave2
NoPass owave2
NoPass owtrans_fimage
NoPass precond1d
NoPass precond2d
NoPass sconvolve

echo


# wave/packets

mwecho -n "wave/packets: "

wp2dmktree -w 4 $tmp/1 || Incerr
v=`fsize $tmp/1`
Check_exact "$v" "16 16"
v=`fmean $tmp/1 | cut -d"=" -f2`
Check_exact $v 1.32812
Pass wp2dmktree

wp2doperate -t 2 -s 15 -b biortho/htilde/sd09.ir $tmp/1 biortho/h/sd07.ir cimage $tmp/2 || Incerr
v=`fnorm -v $tmp/2 | cut -d"=" -f2`
Check_approx $v 6.15582
Pass wp2doperate

wp2ddecomp $tmp/1 cimage ortho/da05.ir $tmp/2 || Incerr
Pass wp2ddecomp

NoPass wp2dchangepack
NoPass wp2dchangetree
NoPass Wp2dcheck
NoPass wp2dchecktree
NoPass wp2deigenval
NoPass wp2dfreqorder
NoPass wp2drecomp
NoPass wp2dview
NoPass wpsconvolve

echo


# wave/ridgelet

mwecho -n "wave/ridgelet: "

NoPass iridgelet
NoPass istkwave1
NoPass ridgelet
NoPass ridgpolrec
NoPass ridgrecpol
NoPass ridgthres
NoPass stkwave1

echo


#--- print results and set exit code

if [ $toterr -eq 0 ]; then
    echo "Test completed: $totmod modules successfully tested."
    code=0
else
    echo
    echo "***** $toterr module(s) failed ($totmod tested)."
    echo "Note that some errors may have been caused by dependencies."
    echo "Please fix the modules in the order given below."
    echo
    echo "***** the following modules failed:"
    cat $tmp/failed
    code=1
fi

#--- delete temporary files and dirs
if [ $rmtmproot = 1 ]; then 
    rm -rf $tmproot/
else
    rm -rf $tmp
fi

#--- exit
exit $code

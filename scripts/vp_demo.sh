#!/bin/sh
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="VP_DEMO"
_Vers="1.0"
_Date="2002"
_Func="Detect and visualize alignments and vanishing points";
_Auth="Andrés Almansa";
_Usage="[ -fftdequant ] [ -no_align ] [ -quant q ] [ -masked_vps ] [ -all_vps ] [ image ]"
#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

#	input:		the image to be analyzed
#
#	options for alignment detection:
#	  -fftdequant	dequantize gradient orientation by
#			translating image by half a pixel
#	  -quant q	ignore orientation of small gradients
#			knowing that quantization noise is q (default q=1)
#         -no_align     use precomputed alignments image.segs image.ksegs
#
#	options for vanishing point detection:
#	  -masked_vps	compute and show "masked" vps that become
#			meaningful only when removing alignments
#			contributing to other meaningful vps
#	  -all_vps	compute and show all maximal vps
#			(before MDL)
#
#	output:		visualization of alignments and vanishing points
#
#       MegaWave2 modules used: fzrt, align_mdl, vpoint, vpsegplot, fkview

#----- Default settings -----
image=building.tif
# original name : ciup_b1.tif
# other images:
# route.rim ciup_b1.tif ciup_b2.tif ciup_fdm_appel1.tif ciup_fdm_central1.tif ciup_fdm_central2.tif
fftdequant=0
quant=1
compute_alignments=1
vp_opts=""

#----- Parse command line -----
while [ $# -ge 1 ] 
do
    case $1 in
    -fftdequant )
	fftdequant=1
	;;
    -quant )
	shift
        if [ $# -lt 1 ]; then
	    echo "-quant option requires an argument"
	    exit 1
	else
	    quant=$1
	fi
	;;
    -no_align )
	compute_alignments=0
	;;
    -masked_vps )
	vp_opts="$vp_opts -m"
	;;
    -all_vps )
	vp_opts="$vp_opts -a"
	;;
    * )
	image=$1
	;;
    esac
    shift
done

#----- Check $MY_MEGAWAVE2/tmp/vpoint directory -----
if [ ! -d $MY_MEGAWAVE2 ]; then
	echo $MY_MEGAWAVE2" does not exist. Please call mwnewuser."
	exit 1
fi
if [ ! -d $MY_MEGAWAVE2/tmp ]; then
    mkdir $MY_MEGAWAVE2/tmp
fi
if [ ! -d $MY_MEGAWAVE2/tmp/vpoint ]; then
    mkdir $MY_MEGAWAVE2/tmp/vpoint
fi
outdir=$MY_MEGAWAVE2/tmp/vpoint

#----- Main -----

img=`basename $image`

#----- Compute alignments and their graphical representation as curves

if [ $compute_alignments -eq 1 ]; then
    if [ $fftdequant -eq 1 ]; then
	fzrt -o 7 ${image} ${outdir}/${img} 1 0 0.5 0.5
    else
	fcopy ${image} ${outdir}/${img}
    fi
    segs=${outdir}/${img}.segs
    ksegs=${outdir}/${img}.ksegs
    align_mdl -n 256 -d 8 -l 3 -g $quant -c $ksegs ${outdir}/${img} $segs
else
    fcopy ${image} ${outdir}/${img}
    segs=${img}.segs
    ksegs=${img}.ksegs
    if [ ! -r $segs ]; then
	echo ${img}.segs": file not found or unreadable. Call VP_DEMO without the -no_align option."
	exit 1
    fi
    if [ ! -r $ksegs ]; then
	echo ${img}.ksegs": file not found or unreadable. Call VP_DEMO without the -no_align option."
	exit 1
    fi
fi
fkview -s -b $image $ksegs &

#----- Compute vanishing points

vpoint $vp_opts -v -s ${outdir}/${img}.csegs ${outdir}/${img} $segs ${outdir}/${img}.vpoints > ${outdir}/${img}.nvpoints

# Number of maximal vanishing points
nvp0=`awk -F= '(match($1,/NVP /)){ print $2}' ${outdir}/${img}.nvpoints`
# Number of masked maximal vanishing points
nvp1=`awk -F= '(match($1,/NVP_masked /)){ print $2}' ${outdir}/${img}.nvpoints`
# Total number of maximal vanishing points
nvp=`expr $nvp0 + $nvp1`

#----- Visualize vanishing points and corresponding segments
i=0
while [ $i -le $nvp ]
do
    if [ $i -lt $nvp0 ]; then
       kvp="${outdir}/${img}.vp${i}"
    elif [ $i -lt $nvp ]; then
       kvp="${outdir}/${img}.vp${i}masked"
    else
       kvp="${outdir}/${img}.vp-none"
    fi
    if vpsegplot ${outdir}/${img} $segs ${outdir}/${img}.vpoints ${outdir}/${img}.csegs $i $kvp; then
	fkview -s -b ${outdir}/${img} $kvp &
    else
        break
    fi
    i=`expr $i + 1`;
done

/bin/rm ${outdir}/${img}*

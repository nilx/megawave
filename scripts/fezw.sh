#!/bin/sh 
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="Fezw"
_Vers="1.05"
_Date="1997-2002"
_Func="Compresses an image with EZW algorithm";
_Auth="Jean-Pierre D'Ales, Jacques Froment";
_Usage="[-R Rate] [-b1 BiFilt1] [-b2 BiFilt2] [-o OrthoFilt] [-e EdgeFilt] image"

#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

#	input:		the image to be compressed, the wavelet filters
#	output:		the quantized image, the compressed file
#
#       MegaWave2 modules used: fezw

# v1.05: MY_MEGAWAVE2/tmp removed (JF+LM).

#----- Check syntax -----
if [ $# -le 0 ] 
then
	. .mw2_help_lg_com
fi

# FIXME
MWDATA=$DATA
MY_MWDATA=$DATA
GROUP=compression/ezwave


# option to compute rate distortion curve
RDOPT="-d"

# Default wavelet filters
WAVELETFILTER=$DATA/wave/biortho/h/sd07.ir
WAVELETFILTER2=$DATA/wave/biortho/htilde/sd09.ir
WAVELETOPT="-b $DATA/wave/biortho/htilde/sd09.ir"
WAVELETEDGEFILTER="no filter"

IMAGE=""

STOP="FALSE"

while [ "$1" != "" -a "$STOP" = "FALSE" ]
do
  case "$1" in
    # Compress with a target rate 
    -R)		shift
		RATE="$1"
		RDOPT="-R $1"
		;;

    # Use customized wavelet filters
    -b1)        shift
		WAVELETFILTER="$1"
		;;
    -b2)        shift
		WAVELETOPT="-b $1"
		;;
    -o)         shift
		WAVELETFILTER="$1"
		;;
    -e)         shift
		WAVELETOPT="-e $1"
		;;

    # Input image
    [!-]*)	IMAGE=$1
		PREFIMAGE='basename $1'
		STOP="TRUE"
		;;

    # Error
    *)		echo "Unrecognized option $1"
		exit 1
		;;
  esac
  shift
done



# Check that wavelet filters are available

if [ ! -f "${WAVELETFILTER}" ] && [ ! -f "${MWDATA}/${WAVELETFILTER}" ] && [ ! -f "${MY_MWDATA}/${WAVELETFILTER}" ] && [ ! -f "${MY_MWDATA}/${GROUP}/${WAVELETFILTER}" ]; then
  echo "Cannot find ${WAVELETFILTER} in Megawave2 data directories!"
  exit 1
fi

if [ "${WAVELETEDGEFILTER}" = "no filter" ]; then

  if [ "${WAVELETFILTER2}" != "" ]; then
    if [ ! -f "${WAVELETFILTER2}" ] && [ ! -f "${MWDATA}/${WAVELETFILTER2}" ] && [ ! -f "${MY_MWDATA}/${WAVELETFILTER2}" ] && [ ! -f "${MY_MWDATA}/${GROUP}/${WAVELETFILTER2}" ]; then
      echo "Cannot find ${WAVELETFILTER2} in Megawave2 data directories!"
      exit 1
    fi
  fi

else

  if [ ! -f ${WAVELETEDGEFILTER} ] && [ ! -f ${MWDATA}/${WAVELETEDGEFILTER} ] && [ ! -f ${MY_MWDATA}/${WAVELETEDGEFILTER} ] && [ ! -f ${MY_MWDATA}/${GROUP}/${WAVELETEDGEFILTER} ]; then
    echo "Cannot find ${WAVELETEDGEFILTER} in Megawave2 data directories!"
    exit 1
  fi

fi

# Check if compressed file and quantized image are needed
if [ "$RDOPT" != "-d" ]; then
  COMPRESSFILE=`basename ${IMAGE}`
  PREFIX=`echo "${COMPRESSFILE}" | cut -f3 -d.`
  if [ "$COMPRESSFILE" = "$IMAGE" ]; then
    PREFIX=""
  fi
  NGROUP=1
  while [ "${PREFIX}" != "" ]
  do
    NGROUP=`echo "${NGROUP} 1 + p" | dc`
    NGSUFFIX=`echo "${NGROUP} 2 + p" | dc`
    PREFIX=`echo "${COMPRESSFILE}" | cut -f"${NGSUFFIX}" -d.`
  done
  PREFIX=`echo "${COMPRESSFILE}" | cut -f1-"${NGROUP}" -d.`
  COMPRESSOPT="-o ${PREFIX}_${RATE}c.comp"
  QUANTIZEARG="${PREFIX}_${RATE}q.rim"
else
  COMPRESSOPT=""
  QUANTIZEARG=""
fi

# Make compression

echo "fezw $RDOPT $WAVELETOPT $COMPRESSOPT $IMAGE $WAVELETFILTER $QUANTIZEARG"
fezw $RDOPT $WAVELETOPT $COMPRESSOPT $IMAGE $WAVELETFILTER $QUANTIZEARG
	





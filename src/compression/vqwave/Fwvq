#!/bin/sh
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="Fwvq"
_Vers="1.3"
_Date="1997"
_Func="Compresses an image by vector quantizing its wavelet transform";
_Auth="Jean-Pierre D'Ales";
_Usage="[-R Rate] [-m] [-s] [-u] [-r NLevel] [-b1 BiFilt1] [-b2 BiFilt2] [-o OrthoFilt] [-e EdgeFilt] image codebook"

#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

#	input:		the image to be compressed, the wavelet filters, 
#                       the codebook sequence
#	output:		the quantized image, the compressed file
#
#       MegaWave2 modules used: fwvq

# v1.2: fixed owave directory bug (L.Moisan)
# v1.3: fixed ${MY_MEGAWAVE2}/tmp bug (JF)

#----- Check syntax -----
if [ $# -le 1 ] 
then
	. .mw2_help_lg_com
fi

#----- Choose tmp directory -----
MWTMP="${MY_MEGAWAVE2}/tmp"
if [ ! -d $MWTMP ]; then
  if [ -w $${MY_MEGAWAVE2} ]; then
    mkdir -p $MWTMP
  else
    MWTMP="${MEGAWAVE2}/tmp"
    if [ ! -d  $MWTMP ]; then
      if [ -w $${MEGAWAVE2} ]; then
        mkdir -p $MWTMP
      else
        MWTMP=/tmp
      fi
    fi
  fi
fi

MWDATA="${MEGAWAVE2}/data"
MY_MWDATA="${MY_MEGAWAVE2}/data"
GROUP=compression/vqwave


# option to compute rate distortion curve
RDOPT="-d"

# Default memory allocation mode : approximate one.
ALLOCMODEOPT="-m 1"

# Default scalar quantization option 
SCALAROPT=""

# Number of level for wavelet transform
NLEVELOPT=""
NLEVEL=4

# Default wavelet filters
WAVELETFILTER="wave/biortho/h/sd07.ir"
WAVELETFILTER2="wave/biortho/htilde/sd09.ir"
WAVELETOPT="-b wave/biortho/htilde/sd09.ir"
WAVELETEDGEFILTER="no filter"

IMAGE=""
CBOPT=""

STOP="FALSE"

while [ "$1" != "" -a "$STOP" = "FALSE" ]
do
  case "$1" in
    # Compress with a target rate 
    -R)		shift
		RATE=$1
		RDOPT="-R $1"
		;;

    # Bit rate allocation mode
    -m)	        ALLOCMODEOPT="-m 2"
		;;

    # Scalar quantization mode
    -s)	        shift
		SCALAROPT="${SCALAROPT} -s $1"
		;;

    # Scalar quantization step
    -u)	        shift
		SCALAROPT="${SCALAROPT} -u $1"
		;;


    # Number of levels for wavelet transform 
    -r)		shift
		NLEVEL="$1"
		NLEVELOPT="-r $1"
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
		IMAGEFILE=`basename $1`
		shift

    # Prefix of codebooks
		PREFCODEBOOK=$1
		CODEBOOK=${PREFCODEBOOK}.cb
		if  [ ! -f $CODEBOOK ]; then
		  echo "Cannot find codebookfile ${CODEBOOK}"
		  exit 0
		fi
		CODEBOOKQ=${PREFCODEBOOK}_q.cb
		if  [ -f $CODEBOOKQ ]; then
		  CBOPT="${CBOPT} -A ${CODEBOOKQ}"
		fi
		CODEBOOKQR=${PREFCODEBOOK}_qr.cb
		if  [ -f $CODEBOOKQR ]; then
		  CBOPT="${CBOPT} -D ${CODEBOOKQR}"
		fi
		CODEBOOKX=${PREFCODEBOOK}_x.cb
		if  [ -f $CODEBOOKX ]; then
		  CBOPT="${CBOPT} -x ${CODEBOOKX}"
		fi
		CODEBOOKXQ=${PREFCODEBOOK}_xq.cb
		if  [ -f $CODEBOOKXQ ]; then
		  CBOPT="${CBOPT} -B ${CODEBOOKXQ}"
		fi
		CODEBOOKXQR=${PREFCODEBOOK}_xqr.cb
		if  [ -f $CODEBOOKXQR ]; then
		  CBOPT="${CBOPT} -E ${CODEBOOKXQR}"
		fi
		CODEBOOKY=${PREFCODEBOOK}_y.cb
		if  [ -f $CODEBOOKY ]; then
		  CBOPT="${CBOPT} -y ${CODEBOOKY}"
		fi
		CODEBOOKYQ=${PREFCODEBOOK}_yq.cb
		if  [ -f $CODEBOOKYQ ]; then
		  CBOPT="${CBOPT} -C ${CODEBOOKYQ}"
		fi
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




### Check if compressed file and quantized image are needed

PREFIX=`echo "${IMAGEFILE}" | cut -f3 -d.`
NGROUP=1
while [ "${PREFIX}" != "" ]
do
  NGROUP=`echo "${NGROUP} 1 + p" | dc`
  NGSUFFIX=`echo "${NGROUP} 2 + p" | dc`
  PREFIX=`echo "${IMAGEFILE}" | cut -f"${NGSUFFIX}" -d.`
done
PREFIX=`echo "${IMAGEFILE}" | cut -f1-"${NGROUP}" -d.`

if [ "$RDOPT" != "-d" ]; then
  COMPRESSOPT="-o ${PREFIX}_${RATE}c.comp"
  QUANTIZEARG="${PREFIX}_${RATE}q.rim"
  REDIR_DR=""
else
  COMPRESSOPT=""
  QUANTIZEARG=""
  DR="${MWTMP}/dr_${PREFIX}"
fi



### Extract part of image in order that size be compatible 

FACTOR=`echo "2 ${NLEVEL} ^ p" | dc`
FACTOR=`echo "2 ${FACTOR} * p" | dc`
cextcenter -f $FACTOR ${IMAGE} ${MWTMP}/${IMAGEFILE}.ext


### Make compression

#echo "fwvq $RDOPT $ALLOCMODEOPT $SCALAROPT $NLEVELOPT $WAVELETOPT $CBOPT $COMPRESSOPT $IMAGE $CODEBOOK $WAVELETFILTER $QUANTIZEARG"
	
if [ "$RDOPT" != "-d" ]; then
  fwvq $RDOPT $ALLOCMODEOPT $SCALAROPT $NLEVELOPT $WAVELETOPT $CBOPT $COMPRESSOPT ${MWTMP}/${IMAGEFILE}.ext $CODEBOOK $WAVELETFILTER $QUANTIZEARG 
else
  fwvq $RDOPT $ALLOCMODEOPT $SCALAROPT $NLEVELOPT $WAVELETOPT $CBOPT $COMPRESSOPT ${MWTMP}/${IMAGEFILE}.ext $CODEBOOK $WAVELETFILTER $QUANTIZEARG >${DR}
fi


### Compute rate distortion curve

if [ "$RDOPT" = "-d" ]; then

  d=1
  D=14
  while [ $d -le $D ]
  do

    # Compute compression ratio
    head -n $d ${DR} > ${DR}_head
    tail -n 1 ${DR}_head > ${DR}_1
    r=`cut -c1-6 ${DR}_1`
    cr=`echo "8000000 ${r} / p" | dc`
    cr=`echo "0.000001 ${cr} * p" | dc`
    cr=`echo ${cr} | cut -f1 -d.`

    # Extract psnr
    psnr=`cut -c8-12 ${DR}_1`

    echo $cr $psnr
    rm -f ${DR}_head ${DR}_1 
    d=`echo "$d 1 + p" | dc`
  done
  rm -f ${DR} 
fi
	

rm -f ${MWTMP}/${IMAGEFILE}.ext




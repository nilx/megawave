#!/bin/sh
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="Cfwvq"
_Vers="1.3"
_Date="1997"
_Func="Compresses a color image by vector quantizing its wavelet transform";
_Auth="Jean-Pierre D'Ales, Jacques Froment";
_Usage="[-R Rate] [-c] [-m] [-s] [-u] [-r NLevel] [-b1 BiFilt1] [-b2 BiFilt2] [-o OrthoFilt] [-e EdgeFilt] image codebook_red [codebook_green codebook_blue]"

#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

#	input:		the image to be compressed, the wavelet filters, 
#                       the codebook sequence(s)
#	output:		the quantized image, the compressed file
#
#       MegaWave2 modules used: fwvq, cfgetchannels, cfputchannels

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

# Default color conversion : no conversion
COLORCONVERT="no"

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
CBOPT_R=""
CBOPT_G=""
CBOPT_B=""

STOP="FALSE"

while [ "$1" != "" -a "$STOP" = "FALSE" ]
do
  case "$1" in
    # Compress with a target rate 
    -R)		shift
		RATE=$1
		# RATE=`echo "${RATE} 1000000 * p" | dc`
		# RATE=`echo "${RATE} 3 / p" | dc`
		# RATE=`echo "${RATE} 0.000001 * p" | dc`
		RDOPT="-R ${RATE}"
		shift
		;;

    # Perform color conversion to YUV
    -c)	        COLORCONVERT="yes"
		shift
		;;

    # Bit rate allocation mode
    -m)	        ALLOCMODEOPT="-m 2"
		shift
		;;

    # Scalar quantization mode
    -s)	        shift
		SCALAROPT="${SCALAROPT} -s $1"
		shift
		;;

    # Scalar quantization step
    -u)	        shift
		SCALAROPT="${SCALAROPT} -u $1"
		shift
		;;


    # Number of levels for wavelet transform 
    -r)		shift
		NLEVEL="$1"
		NLEVELOPT="-r $1"
		shift
		;;

    # Use customized wavelet filters
    -b1)        shift
		WAVELETFILTER="$1"
		shift
		;;
    -b2)        shift
		WAVELETOPT="-b $1"
		shift
		;;
    -o)         shift
		WAVELETFILTER="$1"
		shift
		;;
    -e)         shift
		WAVELETOPT="-e $1"
		shift
		;;

    # Input image
    [!-]*)	IMAGE=$1
		IMAGEFILE=`basename $1`
		shift

    # Prefix of codebooks
                # Set codebooks for red channel
		PREFCODEBOOK_R=$1
		CODEBOOK_R=${PREFCODEBOOK_R}.cb
		if  [ ! -f $CODEBOOK_R ]; then
		  echo "Cannot find codebook file ${CODEBOOK_R}"
		  exit 0
		fi
		CODEBOOKQ_R=${PREFCODEBOOK_R}_q.cb
		if  [ -f $CODEBOOKQ_R ]; then
		  CBOPT_R="${CBOPT_R} -A ${CODEBOOKQ_R}"
		fi
		CODEBOOKQR_R=${PREFCODEBOOK_R}_qr.cb
		if  [ -f $CODEBOOKQR_R ]; then
		  CBOPT_R="${CBOPT_R} -D ${CODEBOOKQR_R}"
		fi
		CODEBOOKX_R=${PREFCODEBOOK_R}_x.cb
		if  [ -f $CODEBOOKX_R ]; then
		  CBOPT_R="${CBOPT_R} -x ${CODEBOOKX_R}"
		fi
		CODEBOOKXQ_R=${PREFCODEBOOK_R}_xq.cb
		if  [ -f $CODEBOOKXQ_R ]; then
		  CBOPT_R="${CBOPT_R} -B ${CODEBOOKXQ_R}"
		fi
		CODEBOOKXQR_R=${PREFCODEBOOK_R}_xqr.cb
		if  [ -f $CODEBOOKXQR_R ]; then
		  CBOPT_R="${CBOPT_R} -E ${CODEBOOKXQR_R}"
		fi
		CODEBOOKY_R=${PREFCODEBOOK_R}_y.cb
		if  [ -f $CODEBOOKY_R ]; then
		  CBOPT_R="${CBOPT_R} -y ${CODEBOOKY_R}"
		fi
		CODEBOOKYQ_R=${PREFCODEBOOK_R}_yq.cb
		if  [ -f $CODEBOOKYQ_R ]; then
		  CBOPT_R="${CBOPT_R} -C ${CODEBOOKYQ_R}"
		fi
		shift

                # Check if different codebooks must be used 
		# for green and blue channels
		PREFCODEBOOK_G=""
		PREFCODEBOOK_B=""
		if [ "$1" != "" ]; then
		  PREFCODEBOOK_G=$1
		  shift
		  if [ "$1" != "" ]; then
		    PREFCODEBOOK_B=$1

                    # Set codebooks for green channel
		    CODEBOOK_G=${PREFCODEBOOK_G}.cb
		    if  [ ! -f $CODEBOOK_G ]; then
		      echo "Cannot find codebook file ${CODEBOOK_G}"
		      exit 0
		    fi
		    CODEBOOKQ_G=${PREFCODEBOOK_G}_q.cb
		    if  [ -f $CODEBOOKQ_G ]; then
		      CBOPT_G="${CBOPT_G} -A ${CODEBOOKQ_G}"
		    fi
		    CODEBOOKQR_G=${PREFCODEBOOK_G}_qr.cb
		    if  [ -f $CODEBOOKQR_G ]; then
		      CBOPT_G="${CBOPT_G} -D ${CODEBOOKQR_G}"
		    fi
		    CODEBOOKX_G=${PREFCODEBOOK_G}_x.cb
		    if  [ -f $CODEBOOKX_G ]; then
		      CBOPT_G="${CBOPT_G} -x ${CODEBOOKX_G}"
		    fi
		    CODEBOOKXQ_G=${PREFCODEBOOK_G}_xq.cb
		    if  [ -f $CODEBOOKXQ_G ]; then
		      CBOPT_G="${CBOPT_G} -B ${CODEBOOKXQ_G}"
		    fi
		    CODEBOOKXQR_G=${PREFCODEBOOK_G}_xqr.cb
		    if  [ -f $CODEBOOKXQR_G ]; then
		      CBOPT_G="${CBOPT_G} -E ${CODEBOOKXQR_G}"
		    fi
		    CODEBOOKY_G=${PREFCODEBOOK_G}_y.cb
		    if  [ -f $CODEBOOKY_G ]; then
		      CBOPT_G="${CBOPT_G} -y ${CODEBOOKY_G}"
		    fi
		    CODEBOOKYQ_G=${PREFCODEBOOK_G}_yq.cb
		    if  [ -f $CODEBOOKYQ_G ]; then
		      CBOPT_G="${CBOPT_G} -C ${CODEBOOKYQ_G}"
		    fi

                    # Set codebooks for blue channel
		    CODEBOOK_B=${PREFCODEBOOK_B}.cb
		    if  [ ! -f $CODEBOOK_B ]; then
		      echo "Cannot find codebook file ${CODEBOOK_B}"
		      exit 0
		    fi
		    CODEBOOKQ_B=${PREFCODEBOOK_B}_q.cb
		    if  [ -f $CODEBOOKQ_B ]; then
		      CBOPT_B="${CBOPT_B} -A ${CODEBOOKQ_B}"
		    fi
		    CODEBOOKQR_B=${PREFCODEBOOK_B}_qr.cb
		    if  [ -f $CODEBOOKQR_B ]; then
		      CBOPT_B="${CBOPT_B} -D ${CODEBOOKQR_B}"
		    fi
		    CODEBOOKX_B=${PREFCODEBOOK_B}_x.cb
		    if  [ -f $CODEBOOKX_B ]; then
		      CBOPT_B="${CBOPT_B} -x ${CODEBOOKX_B}"
		    fi
		    CODEBOOKXQ_B=${PREFCODEBOOK_B}_xq.cb
		    if  [ -f $CODEBOOKXQ_B ]; then
		      CBOPT_B="${CBOPT_B} -B ${CODEBOOKXQ_B}"
		    fi
		    CODEBOOKXQR_B=${PREFCODEBOOK_B}_xqr.cb
		    if  [ -f $CODEBOOKXQR_B ]; then
		      CBOPT_B="${CBOPT_B} -E ${CODEBOOKXQR_B}"
		    fi
		    CODEBOOKY_B=${PREFCODEBOOK_B}_y.cb
		    if  [ -f $CODEBOOKY_B ]; then
		      CBOPT_B="${CBOPT_B} -y ${CODEBOOKY_B}"
		    fi
		    CODEBOOKYQ_B=${PREFCODEBOOK_B}_yq.cb
		    if  [ -f $CODEBOOKYQ_B ]; then
		      CBOPT_B="${CBOPT_B} -C ${CODEBOOKYQ_B}"
		    fi
                  fi
		fi
		if [ "${PREFCODEBOOK_B}" = "" ]; then
		  CODEBOOK_G=${CODEBOOK_R}
		  CBOPT_G=${CBOPT_R}
		  CODEBOOK_B=${CODEBOOK_R}
		  CBOPT_B=${CBOPT_R}
		fi
		STOP="TRUE"
		;;

    # Error
    *)		echo "Unrecognized option $1"
		exit 1
		;;
  esac
done


### Check that wavelet filters are available

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



### Extract part of image in order that size be compatible 

FACTOR=`echo "2 ${NLEVEL} ^ p" | dc`
FACTOR=`echo "2 ${FACTOR} * p" | dc`
cfextcenter -f $FACTOR ${IMAGE} ${MWTMP}/${IMAGEFILE}.ext

### Perform color conversion

if [ "${COLORCONVERT}" = "yes" ]; then

  if [ "$RDOPT" = "-d" ]; then 
    echo WARNING! Color conversion will be ignored!
  else
    cfchgchannels -c 0 ${MWTMP}/${IMAGEFILE}.ext ${MWTMP}/${IMAGEFILE}.ext
  fi

fi



### Extract channels from color image

PREFIX=`echo "${IMAGEFILE}" | cut -f3 -d.`
NGROUP=1
while [ "${PREFIX}" != "" ]
do
  NGROUP=`echo "${NGROUP} 1 + p" | dc`
  NGSUFFIX=`echo "${NGROUP} 2 + p" | dc`
  PREFIX=`echo "${IMAGEFILE}" | cut -f"${NGSUFFIX}" -d.`
done
PREFIX=`echo "${IMAGEFILE}" | cut -f1-"${NGROUP}" -d.`
DR="dr_${PREFIX}"

cfgetchannels ${MWTMP}/${IMAGEFILE}.ext ${MWTMP}/${PREFIX}_red ${MWTMP}/${PREFIX}_green ${MWTMP}/${PREFIX}_blue

rm -f ${MWTMP}/${IMAGEFILE}.ext



### Compress red channel ###

# Check if compressed file and quantized image are needed
if [ "$RDOPT" != "-d" ]; then
  COMPRESSOPT="-o ${PREFIX}_${RATE}r.comp"
  QUANTIZEARG="${MWTMP}/${PREFIX}_${RATE}q_r.rim"
else
  COMPRESSOPT=""
  QUANTIZEARG=""
fi

# Make compression

if [ "$RDOPT" != "-d" ]; then

  echo 
  echo "Compress red channel : "
  echo 

  if [ "${COLORCONVERT}" = "yes" ]; then
    RATE_R=`echo "${RATE} 0.15 * p" | dc`
    RDOPT="-R ${RATE_R}"
  fi

fwvq $RDOPT $ALLOCMODEOPT $SCALAROPT $NLEVELOPT $WAVELETOPT $CBOPT_R $COMPRESSOPT ${MWTMP}/${PREFIX}_red $CODEBOOK_R $WAVELETFILTER $QUANTIZEARG
	
else
  echo >${MWTMP}/trash
fwvq $RDOPT $ALLOCMODEOPT $SCALAROPT $NLEVELOPT $WAVELETOPT $CBOPT_R $COMPRESSOPT ${MWTMP}/${PREFIX}_red $CODEBOOK_R $WAVELETFILTER $QUANTIZEARG >${MWTMP}/${DR}_red
fi



### Compress green channel ###

# Check if compressed file and quantized image are needed
if [ "$RDOPT" != "-d" ]; then
  COMPRESSOPT="-o ${PREFIX}_${RATE}g.comp"
  QUANTIZEARG="${MWTMP}/${PREFIX}_${RATE}q_g.rim"
else
  COMPRESSOPT=""
  QUANTIZEARG=""
fi

# Make compression

if [ "$RDOPT" != "-d" ]; then

  echo 
  echo 
  echo "Compress green channel : "
  echo 

  if [ "${COLORCONVERT}" = "yes" ]; then
    RATE_G=`echo "${RATE} 2.7 * p" | dc`
    RDOPT="-R ${RATE_G}"
  fi

fwvq $RDOPT $ALLOCMODEOPT $SCALAROPT $NLEVELOPT $WAVELETOPT $CBOPT_G $COMPRESSOPT ${MWTMP}/${PREFIX}_green $CODEBOOK_G $WAVELETFILTER $QUANTIZEARG
	
else
  echo >${MWTMP}/trash
fwvq $RDOPT $ALLOCMODEOPT $SCALAROPT $NLEVELOPT $WAVELETOPT $CBOPT_G $COMPRESSOPT ${MWTMP}/${PREFIX}_green $CODEBOOK_G $WAVELETFILTER $QUANTIZEARG >${MWTMP}/${DR}_green
fi



### Compress blue channel ###

# Check if compressed file and quantized image are needed
if [ "$RDOPT" != "-d" ]; then
  COMPRESSOPT="-o ${PREFIX}_${RATE}b.comp"
  QUANTIZEARG="${MWTMP}/${PREFIX}_${RATE}q_b.rim"
else
  COMPRESSOPT=""
  QUANTIZEARG=""
fi

# Make compression

if [ "$RDOPT" != "-d" ]; then

  echo 
  echo 
  echo "Compress blue channel : "
  echo 

  if [ "${COLORCONVERT}" = "yes" ]; then
    RATE_B=`echo "${RATE} 0.15 * p" | dc`
    RDOPT="-R ${RATE_B}"
  fi

fwvq $RDOPT $ALLOCMODEOPT $SCALAROPT $NLEVELOPT $WAVELETOPT $CBOPT_B $COMPRESSOPT ${MWTMP}/${PREFIX}_blue $CODEBOOK_B $WAVELETFILTER $QUANTIZEARG
	
else
  echo >${MWTMP}/trash
fwvq $RDOPT $ALLOCMODEOPT $SCALAROPT $NLEVELOPT $WAVELETOPT $CBOPT_B $COMPRESSOPT ${MWTMP}/${PREFIX}_blue $CODEBOOK_B $WAVELETFILTER $QUANTIZEARG >${MWTMP}/${DR}_blue
fi




### Compute rate distortion curve

if [ "$RDOPT" = "-d" ]; then

  d=1
  D=14
  while [ $d -le $D ]
  do

    # Compute compression ratio
    head -n $d ${MWTMP}/${DR}_red > ${MWTMP}/${DR}_red_head
    tail -n 1 ${MWTMP}/${DR}_red_head > ${MWTMP}/${DR}_red1
    head -n $d ${MWTMP}/${DR}_green > ${MWTMP}/${DR}_green_head
    tail -n 1 ${MWTMP}/${DR}_green_head > ${MWTMP}/${DR}_green1
    head -n $d ${MWTMP}/${DR}_blue > ${MWTMP}/${DR}_blue_head
    tail -n 1 ${MWTMP}/${DR}_blue_head > ${MWTMP}/${DR}_blue1
    rr=`cut -c1-6 ${MWTMP}/${DR}_red1`
    rg=`cut -c1-6 ${MWTMP}/${DR}_green1`
    rb=`cut -c1-6 ${MWTMP}/${DR}_blue1`
    rtot=`echo "${rr} ${rg} + p" | dc`
    rtot=`echo "${rtot} ${rb} + p" | dc`
    cr=`echo "24000000 ${rtot} / p" | dc`
    cr=`echo "0.000001 ${cr} * p" | dc`
    cr=`echo ${cr} | cut -f1 -d.`

    # Compute psnr
    psnrr=`cut -c8-12 ${MWTMP}/${DR}_red1`
    psnrg=`cut -c8-12 ${MWTMP}/${DR}_green1`
    psnrb=`cut -c8-12 ${MWTMP}/${DR}_blue1`
    psnr=`echo "scale = 20 \
 define p(x){ \
 return (e(x * l(10))) \
 } \
 -10 * l( (p(-${psnrr}/10) + p(-${psnrg}/10) + p(- ${psnrb}/10))/3 )/l(10)"\\
  | bc -l`
    echo $cr $psnr
    rm -f ${MWTMP}/${DR}_red_head ${MWTMP}/${DR}_red1 ${MWTMP}/${DR}_green_head ${MWTMP}/${DR}_green1 ${MWTMP}/${DR}_blue_head ${MWTMP}/${DR}_blue1
    d=`echo "$d 1 + p" | dc`
  done

  rm -f ${MWTMP}/${DR}_red ${MWTMP}/${DR}_green ${MWTMP}/${DR}_blue 
else

  cfputchannels ${MWTMP}/${PREFIX}_${RATE}q_r.rim ${MWTMP}/${PREFIX}_${RATE}q_g.rim ${MWTMP}/${PREFIX}_${RATE}q_b.rim ${PREFIX}_${RATE}q.pm 2>${MWTMP}/trash

  ### Perform inversecolor conversion

  if [ "${COLORCONVERT}" = "yes" ]; then

    cfchgchannels -c 0 -i ${PREFIX}_${RATE}q.pm ${PREFIX}_${RATE}q.pm

  fi

  rm -f ${MWTMP}/${PREFIX}_${RATE}q_r.rim ${MWTMP}/${PREFIX}_${RATE}q_g.rim ${MWTMP}/${PREFIX}_${RATE}q_b.rim

fi

rm -f ${MWTMP}/${PREFIX}_red ${MWTMP}/${PREFIX}_green ${MWTMP}/${PREFIX}_blue

rm -f ${MWTMP}/trash

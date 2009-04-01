#!/bin/sh
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="Fwlbg_adap"
_Vers="1.3"
_Date="1997"
_Func="Construct a complete set of codebook sequences adapted to vector quantization of image wavelet transform";
_Auth="Jean-Pierre D'Ales";
_Usage="[-b1 BiFilt1] [-b2 BiFilt2] [-o OrthoFilt] [-e EdgeFilt] codebook trainimage1 [trainimage2 [trainimage3 [trainimage4]]]"

#---- MegaWave2 - Copyright (C) 1992-94 J.F. & S.P. All Rights Reserved. ----#

#	input:		the images to be used for the training sets 
#			of vectors, the wavelet filters
#	output:		the codebook sequences
#
#       MegaWave2 modules used: fwlbg_adap

# v1.2: fixed owave directory bug (L.Moisan)
# v1.3: fixed ${MY_MEGAWAVE2}/tmp bug (JF)

#----- Check syntax -----
if [ $# -le 1 ] 
then
	. .mw2_help_lg_com
fi

MWDATA="${MEGAWAVE2}/data"
MY_MWDATA="${MY_MEGAWAVE2}/data"
GROUP=compression/vqwave

# Number of level for wavelet transform
NLEVEL=4
NLEVELOPT="-r 4"

# Generate sequence of codebooks
MULTICB="-M"

# Take overlapping blocks in training images
OVERLAP="-l"

# Default wavelet filters
WAVELETFILTER="wave/biortho/h/sd07.ir"
WAVELETFILTER2="wave/biortho/htilde/sd09.ir"
WAVELETOPT="-b wave/biortho/htilde/sd09.ir"
WAVELETEDGEFILTER="no filter"

# Sizes of codebooks
SIZE1="-s 1024"
SIZE1Q="-s 1024"
SIZE1QR="-s 1024"
SIZE2="-t 1024"
SIZE2Q="-s 1024"
SIZE2QR="-s 1024"
SIZE3="-u 4096"
SIZE3Q="-s 4096"
SIZERES="-s 4096"
SIZERESQ="-s 4096"
SIZERESQR="-s 4096"

# Energy thresholds for classified vq
THRESHOLD="-S 3.0 -T 1.0"

# Training images
trainima2=""
trainima3=""
trainima4=""


STOP="FALSE"

while [ "$1" != "" -a "$STOP" = "FALSE" ]
do
  case "$1" in
    # Number of levels 
    -r)		shift
		NLEVEL="-r $1"
		shift
		;;

    # Use customized wavelet filters
    -b1)        shift
		WAVELETFILTER="$1"
		shift
		;;
    -b2)        shift
		WAVELETOPT="-b $1"
		WAVELETFILTER2="$1"
		shift
		;;
    -o)         shift
		WAVELETFILTER="$1"
		WAVELETFILTER2=""
		shift
		;;
    -e)         shift
		WAVELETOPT="-e $1"
		WAVELETEDGEFILTER="$1"
		shift
		;;

    # Prefix of codebooks
    [!-]*)	prefcodebook=$1
		codebook=${prefcodebook}.cb
		codebookx=${prefcodebook}_x.cb
		codebooky=${prefcodebook}_y.cb
		codebookq=${prefcodebook}_q.cb
		codebookqr=${prefcodebook}_qr.cb
		codebookxq=${prefcodebook}_xq.cb
		codebookxqr=${prefcodebook}_xqr.cb
		codebookyq=${prefcodebook}_yq.cb
		info=info_${prefcodebook}
		shift

    # Input training images
		if  [ "$1" = "" ]; then
		  echo "Need at least one training image!"
		  exit 0
		fi
		trainima1=$1
		shift
		if  [ "$1" != "" ]; then
		  trainima2="-A $1"
		  shift
		  if  [ "$1" != "" ]; then
		    trainima3="-B $1"
		    shift
		    if  [ "$1" != "" ]; then
		      trainima4="-C $1"
		    fi
		  fi
		fi
		STOP="TRUE"
		;;

    # Error
    *)		echo "Unrecognized option $1"
		exit 1
		;;
  esac
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


### Codebooks for first stage quantization 

fwlbg_adap ${NLEVELOPT} -q 1 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1} ${SIZE2} ${SIZE3} ${THRESHOLD} -x ${codebookx} -y ${codebooky} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebook} >> $info

if [ $NLEVEL -ge 2 ]; then

  fwlbg_adap ${NLEVELOPT} -q 2 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1} ${SIZE2} ${SIZE3} ${THRESHOLD} -x ${codebookx} -y ${codebooky} -O ${codebook} -X ${codebookx} -Y ${codebooky} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebook} >> ${info}

  if [ $NLEVEL -ge 3 ]; then

    fwlbg_adap ${NLEVELOPT} -q 3 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1} ${SIZE2} ${SIZE3} ${THRESHOLD} -x ${codebookx} -y ${codebooky} -O ${codebook} -X ${codebookx} -Y ${codebooky} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebook} >> ${info}

    fwlbg_adap ${NLEVELOPT} -q 3 -o 0 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZERES} -O ${codebook} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebook} >> ${info}

    if [ $NLEVEL -ge 4 ]; then

      fwlbg_adap ${NLEVELOPT} -q 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1} ${SIZE2} ${SIZE3} ${THRESHOLD} -x ${codebookx} -y ${codebooky} -O ${codebook} -X ${codebookx} -Y ${codebooky} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebook} >> ${info}

      fwlbg_adap ${NLEVELOPT} -q 4 -o 0 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZERES} -O ${codebook} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebook} >> ${info}

    fi
  fi
fi

echo test1
### Codebooks for second stage quantization after quantization with ${codebook}

fwlbg_adap ${NLEVELOPT} -q 1 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1Q} -Q ${codebook} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookq} >> ${info}

if [ $NLEVEL -ge 2 ]; then

  fwlbg_adap ${NLEVELOPT} -q 2 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1Q} -Q ${codebook} -O ${codebookq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookq} >> ${info}

  if [ $NLEVEL -ge 3 ]; then

    fwlbg_adap ${NLEVELOPT} -q 3 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1Q} -Q ${codebook} -O ${codebookq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookq} >> ${info}

    fwlbg_adap ${NLEVELOPT} -q 3 -o 0 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZERESQ} -Q ${codebook} -O ${codebookq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookq} >> ${info}

    if [ $NLEVEL -ge 4 ]; then

      fwlbg_adap ${NLEVELOPT} -q 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1Q} -Q ${codebook} -O ${codebookq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookq} >> ${info}

      fwlbg_adap ${NLEVELOPT} -q 4 -o 0 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZERESQ} -Q ${codebook} -O ${codebookq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookq} >> ${info}

    fi
  fi
fi

echo test2
### Codebooks for third stage quantization after quantization with ${codebook}
### and ${codebookq}

fwlbg_adap ${NLEVELOPT} -q 1 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1QR} -Q ${codebook} -R ${codebookq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookqr} >> ${info}

if [ $NLEVEL -ge 2 ]; then

  fwlbg_adap ${NLEVELOPT} -q 2 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1QR} -Q ${codebook} -R ${codebookq} -O ${codebookqr} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookqr} >> ${info}

  if [ $NLEVEL -ge 3 ]; then

    fwlbg_adap ${NLEVELOPT} -q 3 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1QR} -Q ${codebook} -R ${codebookq} -O ${codebookqr} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookqr} >> ${info}

    fwlbg_adap ${NLEVELOPT} -q 3 -o 0 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZERESQR} -Q ${codebook} -R ${codebookq} -O ${codebookqr} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookqr} >> ${info}

    if [ $NLEVEL -ge 4 ]; then

      fwlbg_adap ${NLEVELOPT} -q 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE1QR} -Q ${codebook} -R ${codebookq} -O ${codebookqr} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookqr} >> ${info}

      fwlbg_adap ${NLEVELOPT} -q 4 -o 0 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZERESQR} -Q ${codebook} -R ${codebookq} -O ${codebookqr} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookqr} >> ${info}

    fi
  fi
fi

echo test3
### Codebooks for second stage quantization after quantization 
### with ${codebookx}

fwlbg_adap ${NLEVELOPT} -q 1 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE2Q} -Q ${codebookx} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookxq} >> ${info}

if [ $NLEVEL -ge 2 ]; then

  fwlbg_adap ${NLEVELOPT} -q 2 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE2Q} -Q ${codebookx} -O ${codebookxq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookxq} >> ${info}

  if [ $NLEVEL -ge 3 ]; then

    fwlbg_adap ${NLEVELOPT} -q 3 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE2Q} -Q ${codebookx} -O ${codebookxq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookxq} >> ${info}

    if [ $NLEVEL -ge 4 ]; then

      fwlbg_adap ${NLEVELOPT} -q 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE2Q} -Q ${codebookx} -O ${codebookxq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookxq} >> ${info}

    fi
  fi
fi

echo test4
### Codebooks for third stage quantization after quantization 
### with ${codebookx} and ${codebookxq}

fwlbg_adap ${NLEVELOPT} -q 1 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE2QR} -Q ${codebookx} -R ${codebookxq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookxqr} >> ${info}

if [ $NLEVEL -ge 2 ]; then

  fwlbg_adap ${NLEVELOPT} -q 2 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE2QR} -Q ${codebookx} -R ${codebookxq} -O ${codebookxqr} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookxqr} >> ${info}

  if [ $NLEVEL -ge 3 ]; then

    fwlbg_adap ${NLEVELOPT} -q 3 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE2QR} -Q ${codebookx} -R ${codebookxq} -O ${codebookxqr} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookxqr} >> ${info}

    if [ $NLEVEL -ge 4 ]; then

      fwlbg_adap ${NLEVELOPT} -q 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE2QR} -Q ${codebookx} -R ${codebookxq} -O ${codebookxqr} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookxqr} >> ${info}

    fi
  fi
fi

echo test5
### Codebooks for second stage quantization after quantization 
### with ${codebooky}

fwlbg_adap ${NLEVELOPT} -q 1 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE3Q} -Q ${codebooky} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookyq} >> ${info}

if [ $NLEVEL -ge 2 ]; then

  fwlbg_adap ${NLEVELOPT} -q 2 -w 4 -h 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE3Q} -Q ${codebooky} -O ${codebookyq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookyq} >> ${info}

  if [ $NLEVEL -ge 3 ]; then

    fwlbg_adap ${NLEVELOPT} -q 3 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE3Q} -Q ${codebooky} -O ${codebookyq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookyq} >> ${info}

    if [ $NLEVEL -ge 4 ]; then

      fwlbg_adap ${NLEVELOPT} -q 4 -b $WAVELETFILTER2 ${MULTICB} ${OVERLAP} ${SIZE3Q} -Q ${codebooky} -O ${codebookyq} ${trainima2} ${trainima3} ${trainima4} ${trainima1} $WAVELETFILTER ${codebookyq} >> ${info}

    fi
  fi
fi



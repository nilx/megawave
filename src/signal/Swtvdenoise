#!/bin/sh 
#--------------------------- Shell MegaWave2 Macro --------------------------#
_Prog="Swtvdenoise"
_Vers="1.02"
_Date="2000-2002"
_Func="Signal denoising using wavelet and total variation";
_Auth="Jacques Froment";
_Usage="[-P % coeff. to threshold] [-T Threshold] [-D standard dev. (Donoho threshold)] [-O ortho. wavelet] [-E] [-s] [-J number of levels] [-N number of iterations] [-a alpha] [-I input_partially_denoised_signal] [-S output_thresholded_signal] [-V output_minimal_tv_signal] [-r ] input_noisy_signal [output_denoised_signal]"
#
# -P : give the percent of wavelet coefficients to remove.
# -T : give the threshold.
# -D : compute the threshold from the noise standard deviation.
# -O : give the wavelet filter.
# -E : special edge processing (default is periodized).
# -s : soft thresholding (default is hard thresholding).
# -J : number of levels in the wavelet transform.
# -N : number of iterations of the TV minimization algorithm.
# -a : without -a compute exact TV, with -a a smooth version.
# -I : allows to resume the TV minimization with previously denoised signal.
# -S : output the reconstructed signal after wavelet thresholding.
# -V : allows to output the signal having the minimal TV.
# -r : Relax constraint on approximation space V_J.
#----------------------------------------------------------------------------#

RemoveTmpFiles()

{
  if [ "$Win" != "" ]; then
    /bin/rm -f $Win* 
  fi
  if [ "$WTin" != "" ]; then
    /bin/rm -f $WTin* 
  fi
  if [ "$Mask" != "" ] && [ -f $Mask ]; then
    /bin/rm $Mask 
  fi
  if [ "$Tin" != "" ] && [ -f $Tin ]; then
    /bin/rm -f $Tin 
  fi
}

#----- Check syntax -----

if [ $# -lt 1 ] 
then
  echo "Missing needed argument !"
  . .mw2_help_lg_com
fi

P=""; T=""; D=""; J="-r 5"; N=""; S=""; s=""; I=""; V=""; alpha=""; r=""
O="da08.ir"
Edge="-e 1"
Wonly=1
argno=1
dbg=""

while [ "$1" != "" ]
do
 case "$1" in
    -P)		shift
		P="-P $1"
		;;

    -T)		shift
		T="-T $1"
                if [ "$P" != "" ]; then
                  echo "Choose only one option -P, -T or -D ! Exit."
                  exit 1
                fi
		;;

    -D)		shift
		D="-D $1"
                if [ "$P" != "" ] || [ "$T" != "" ]; then
                  echo "Choose only one option -P, -T or -D ! Exit."
                  exit 1
                fi

		;;

    -O)		shift
		O="$1"
		;;

    -E)	        Edge="-e 3"
		;;

    -J)		shift
		J="-r $1"
		;;

    -N)		shift
		N="-n $1"
		;;

    -a)		shift
		alpha="-a $1"
		;;

    -S)		shift
		S="$1"
		;;

    -V)		shift
		V="-V $1"
		;;

    -I)		shift
		I="$1"
		;;

    -s)		s="-s"
		;;

    -r)         r="-r"
                ;;

    -debug)  	dbg="-debug"
		;;

		
    [!-]*)      if [ $argno -eq 1 ]; then
                  in=$1
                  argno=2
                else
                  if [ $argno -eq 2 ]; then
                    out=$1
                    argno=3
                    Wonly=0
                  else
                    echo "Too many arguments : $1"
                    . .mw2_help_lg_com
                  fi
                fi 
                ;;
     
     *)         echo "Invalid option $1"
                . .mw2_help_lg_com
                ;;
     
     esac
   shift
done

if [ "$P" = "" ] && [ "$T" = "" ] && [ "$D" = "" ]; then
  echo "Choose one option -P, -T or -D ! Exit."
  exit 1
fi

if [ ! -r $in ]; then
  echo "Cannot read input signal '$in' ! Exit." 
  exit 1
fi

if [ "$I" != "" ] && [ ! -r $I ]; then
  echo "Cannot read partially denoised signal '$I' ! Exit." 
  exit 1
fi


TMP=/usr/tmp
if  [ ! -w $TMP ]; then
  TMP=/tmp
  if [ ! -w $TMP ]; then
    TMP=.
  fi
fi



#----- Run -----

FilterPath="wave/ortho"
if [ "$Edge" = "-e 3" ]; then
  F=${FilterPath}/edge/${O}
  IF="-I $F"
else
  F=""
  IF=""
fi

kin=`basename ${in}`
kin=`echo $kin | cut -f 1 -d "."`
Win=${TMP}/$$-W_${kin}
WTin=${TMP}/$$-WT_${kin}
Mask=${TMP}/$$-WMask_${kin}
Tin=${TMP}/$$-T_${kin}

# new trap
trap "echo interrupt; RemoveTmpFiles; exit 1" 2
trap "echo quit; RemoveTmpFiles; exit 1" 3

owave1 $J $Edge $in $Win ${FilterPath}/$O $F
if [ $? -ne 0 ] || [ ! -r $Win ]; then
  echo "Cannot run owave1 ! Exit."
  RemoveTmpFiles
  exit 1
fi

w1threshold $P $T $D -M $Mask $s $Win $WTin
if [ $? -ne 0 ] || [ ! -r $WTin ]; then
  echo "Cannot run w1threshold ! Exit."
  RemoveTmpFiles  
  exit 1
fi

if [ "$I" = "" ]; then 
  iowave1 $J $Edge $WTin $Tin ${FilterPath}/$O $F
  if [ $? -ne 0 ] || [ ! -r $Tin ]; then
    echo "Cannot run iowave1 ! Exit."
    RemoveTmpFiles
    exit 1 
  fi
  if [ "$S" != "" ]; then
    cp $Tin $S
  fi
  Nin=$Tin
else
  Nin=$I
fi

if [ $Wonly -ne 1 ]; then
  stvrestore $r $dbg $N $alpha $V -O ${FilterPath}/$O $IF $Mask $Nin $out
  if [ $? -ne 0 ] || [ ! -r $out ]; then
    echo "Cannot run stvrestore ! Exit."
    RemoveTmpFiles
    exit 1
  fi
fi

RemoveTmpFiles
exit 0


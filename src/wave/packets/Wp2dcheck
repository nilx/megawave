#!/bin/sh 
#--------------------------- Shell MegaWave2 Macro --------------------------#
_Prog="Wp2dcheck"
_Vers="1.01"
_Date="2006"
_Func="Check and demonstrate the 2D-wavelet packet representation";
_Auth="Jacques Froment, Francois Malgouyres";
_Usage="image"
#------------------------------------------------------------------------------
# Versions history
# 1.00 (JF, initial release)
# 1.01 (JF, April, 2006) update according to wp2dview 1.01

Pause()
{
  echo "Type <return> to continue"
  read ans
  if [ "$pid" != "" ]; then
    kill $pid > /dev/null 2> /dev/null
  fi
  pid=""
}

Wp2dmktree()
{
 echo "- Creating the quad-tree '$BPARAM' in '$BNAME'..."
 wp2dmktree $BPARAM $BNAME
 if [ $? -ne 0 ]; then
  echo "Unexpected error while running wp2dmktree : exit !"
  exit 1
 fi
}

Wp2ddecomp()
{
 echo "- Computing wavelet packets coefficients in '$WP'..."
 if [ "$WAVELETFILTER2"="" ]; then
  wp2ddecomp $BNAME $1 $WAVELETFILTER $WP
 else
  wp2ddecomp -b $WAVELETFILTER2 $BNAME $1 $WAVELETFILTER $WP
 fi
 if [ $? -ne 0 ]; then
  echo "Unexpected error while running wp2ddecomp : exit !"
  exit 1
 fi
}

Fmse()
{
 echo "- Error between the original and the reconstructed image :"
 fmse -p $IMAGE $R 
}

Wp2drecomp()
{
 echo "- Reconstructing image from wavelet packets in '$R'..."
 wp2drecomp $WP $R
 if [ $? -ne 0 ]; then
  echo "Unexpected error while running wp2drecomp : exit !"
  exit 1
 fi
}

Wp2dview()
{
 echo "- Visualization of the wavelet packets coefficients..."
 wp2dview  -R $V -p $WP
 if [ $? -ne 0 ]; then
  echo "Unexpected error while running wp2dview : exit !"
  exit 1
 fi
 zoom=""
 if [ $xsize -lt 512 ] && [ $ysize -lt 512 ]; then
  zoom="-z 2"
 fi
 fview $zoom $V&
 pid="$!"
 Pause
}

Remove()
{
 /bin/rm -f $BNAME $WP* $R $NIMAGE
}

Wp2doperate()
{
 echo "- Denoise the image by wavelet packets soft thresholding with cycle spinning..."
 wp2doperate -t 2 -s 15 -b $WAVELETFILTER2 $BNAME $WAVELETFILTER $NIMAGE $R
 if [ $? -ne 0 ]; then
  echo "Unexpected error while running wp2operate : exit !"
  exit 1
 fi
}

Fnoise()
{
 echo "- Computing noisy image '$NIMAGE'..."
 fnoise -g 10 $IMAGE ${NIMAGE}
 if [ $? -ne 0 ]; then
  echo "Unexpected error while running fnoise : exit !"
  exit 1
 fi
}

ViewDenoise()
{
 x=30
 cview -x $x $NIMAGE > /dev/null 2> /dev/null &
 pid="$!"
 x=`expr $xsize + $x + 30`
 cview -x $x $R > /dev/null 2> /dev/null &
 pid="$pid $!"
 echo "- Left : original noisy image. Right : denoised image"
 Pause
}

FmseDenoise()
{
 echo "- Error between the original and the noisy image :"
 fmse -p $IMAGE $NIMAGE
 echo "- Error between the original and the denoised image :"
 fmse -p $IMAGE $R 
}


#----- Check syntax -----
if [ $# -ne 1 ] 
then
	. .mw2_help_lg_com
fi

#----- Demonstrate decomposition/reconstruction -----

echo "***** Demonstrate decomposition/reconstruction *****"

# Wavelet to use
WAVELETFILTER="ortho/da05.ir"
WAVELETFILTER2=""

# Image filename
IMAGE=$1

# Base to use
BPARAM="-s 4"
BNAME=${IMAGE}_$$_base

# Wavelet packets prefix
WP=${IMAGE}_$$_wp

# Reconstructed image
R=${IMAGE}_$$_rec

# Visualization of wavelet packets 
V=${WP}_visu


Wp2dmktree 
Wp2ddecomp $IMAGE
Wp2drecomp
Fmse
xsize=`fsize $IMAGE | cut -f 1 -d " "`
ysize=`fsize $IMAGE | cut -f 2 -d " "`
Wp2dview $IMAGE
NIMAGE=""
Remove

#----- Demonstrate image denoising -----

echo "***** Demonstrate image denoising *****"

# Wavelet to use
WAVELETFILTER="biortho/h/sd07.ir"
WAVELETFILTER2="biortho/htilde/sd09.ir"

# Noisy image to restore
NIMAGE=${IMAGE}_$$_noisy

# Base to use
BPARAM="-w 4"
BNAME=${NIMAGE}_$$_base

# Reconstructed image
R=${NIMAGE}_$$_rec

Fnoise
Wp2dmktree 
Wp2doperate
FmseDenoise
ViewDenoise
Remove

echo "***** End of $_Prog *****"

exit 0
	





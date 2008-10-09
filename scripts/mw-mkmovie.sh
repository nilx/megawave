#!/bin/sh 
#--------------------------- Shell MegaWave2 Macro --------------------------#
_sccsid="%Z%MegaWave2 %R%, %M% %I%, %G%";
_Prog="Mkmovie"
_Vers="1.2"
_Date="2002"
_Func="Build a movie from an image sequence"
_Auth="Lionel Moisan";
_Usage="type prefix n1 n2     (type=Cmovie/Fmovie/Ccmovie/Cfmovie)"
#----------------------------------------------------------------------------#
# This file is part of the MegaWave2 system macros. 
# MegaWave2 is a "soft-publication" for the scientific community. It has
# been developed for research purposes and it comes without any warranty.
# The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
# CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
#       94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
#-----------------------------------------------------------------------------

# Usage
if [ $# -ne 4 ]; then
  . .mw2_help_lg_com
fi

def=$1
dest=$2
prefix=`basename $dest`
n1=$3
n2=$4
n=`expr $n2 - $n1 + 1`

if [ $n -le 0 ] || [ $n -gt 100000 ]; then
  . .mw2_help_lg_com
fi
if [ $def != Cmovie ] && [ $def != Fmovie ] && [ $def != Ccmovie ] && [ $def != Cfmovie ]; then
  . .mw2_help_lg_com
fi

\rm -f $dest
echo "%" > $dest
echo "MegaWave2 - DATA ASCII file -" >> $dest
echo "%" >> $dest
echo "def $def" >> $dest
echo "" >> $dest
echo "nimage:$n" >> $dest
echo "" >> $dest

n=$n1
while [ $n -le $n2 ]
do
    echo $prefix'_'$n >> $dest
    n=`expr $n + 1`
done

exit 0

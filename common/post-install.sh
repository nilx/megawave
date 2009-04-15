#! /bin/sh
#
# utility script to be run after the install process
#
# author: Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2009)


CLEAN_DESTDIR=${DESTDIR%%/}

#
# EVERYTHING FINE
#

cat <<EOF

Megawave v3.02 is now installed in $CLEAN_DESTDIR.
EOF

#
# PATH SETTINGS
#

cat <<EOF

# PATH CONFIGURATION
EOF

if [ "$DESTDIR" != "/" \
    -a  "$DESTDIR" != "/usr/" \
    -a "$DESTDIR" != "/usr/local/" ]; then
    cat <<EOF

To use it, you should add "$CLEAN_DESTDIR/bin" to your PATH environment
variable. Depending on your shell you should:
* bash users, add this line to you ~/.bashrc file:
    PATH=\$PATH:$CLEAN_DESTDIR/bin
* sh users, add this line to you ~/.profile file:
    PATH=\$PATH:$CLEAN_DESTDIR/bin
* csh/tcsh users, add this line to you ~/.cshrc file:
    setenv PATH \$PATH:$CLEAN_DESTDIR/bin
* ksh, zsh, fish users, ... we need more information about your shell

It seems that you are currently using the "$( basename $SHELL )" shell.
EOF
else
    cat<<EOF

The intallation seems to be a standard location, no PATH setting
required.
EOF
fi

#
# CONFIGURATION FILE
#

cat <<EOF

# OTHER SETTINGS
EOF

cat <<EOF

You can set some settings used by megawave in your personal
configuration file ~/.config/megawave3.config :
* MW3_DOCVIEWER : the program used to see the documentation
* MW3_DATAPATH  : the list of places where megawave will look for data
  files
* MW3_LOCALPATH : the location where megawave will store your own
  compiled modules and libraries
Administrators can also do it in /etc/megawave3.config.

You can find an example and more details in
    $CLEAN_DESTDIR/share/doc/megawave3/example3/megawave3.config
EOF

#
# UNINSTALLATION
#

cat <<EOF

# UNINSTALLATION
EOF

cat <<EOF

Not implemented yet...

It your installed megawave with a custom \$DESTDIR (in /opt or in your
own folder), then simply deleting this folder and its content should be
enough; don't forget to remove also any configuration file and custom
data folder.
EOF

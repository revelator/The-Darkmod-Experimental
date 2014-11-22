#!/bin/bash

mkdir -p ~/games/tdm/darkmod

# make sure this file exists
touch scons.signatures.dblite

# FAST=true
# --debug=explain

# move the system zlib.h header out of the place so we can use darkmods
mv -f /usr/include/zlib.h /usr/include/zlib.h.bak
cp -pf `pwd`/include/zlib/zlib.h /usr/include/zlib.h

# $@ = pass along the flags like "BUILD=profile" or "BUILD=debug"
time scons -j2 BUILD_GAMEPAK=1 NO_GCH=0 BUILD=release --debug=explain "$@"
mv gamex86-base.so gamex86.so
#strip gamex86.so
#strip thedarkmod.x86

cp thedarkmod.x86 ~/games/tdm/darkmod/
cp thedarkmod.x86 ~/games/tdm/

cp tdm_game02.pk4 ~/games/tdm/darkmod/tdm_game02.pk4
if [ -f ~/games/tdm/darkmod/gamex86.so ]; then
  rm ~/games/tdm/darkmod/gamex86.so
fi
# move zlib.h back to the original
rm -f /usr/include/zlib.h
mv -f /usr/include/zlib.h.bak /usr/include/zlib.h

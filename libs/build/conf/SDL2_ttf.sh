#!/bin/bash
set -e

SDL2_DIR="SDL2-2.0.10"
FREETYPE_DIR="freetype-2.10.1"

export CC="gcc"
export CFLAGS="-O2 -flto -I$PWD/../$SDL2_DIR/include -I$PWD/../$FREETYPE_DIR/include"
export LDFLAGS="-lm -flto"

export FT2_CONFIG="$PWD/../$FREETYPE_DIR/builds/unix/freetype-config"
export PKG_CONFIG_PATH=`dirname $FT2_CONFIG`

./configure \
	--enable-static \
	--disable-shared \
	--disable-sdltest \
	\
	--with-sdl-prefix=$PWD/../000res \
	--with-ft-prefix=$PWD/../000res

make clean

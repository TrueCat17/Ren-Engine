#!/bin/bash
set -e

SDL2_DIR="SDL2-2.0.10"
FREETYPE_DIR="freetype-2.10.1"

export CC="gcc"
export CFLAGS="-O2 -flto -I$PWD/../$SDL2_DIR/include -I$PWD/../$FREETYPE_DIR/include"
export LDFLAGS="-lm -flto"
export LIBS="-L$PWD/../000res/ -lfreetype -lbrotlidec-static -lbrotlicommon-static -lpng16 -lz -lm"

./autogen.sh

./configure \
	--enable-static \
	--disable-shared \
	--disable-sdltest \
	--disable-freetypetest \
	\
	--with-sdl-prefix=$PWD/../000res \
	--with-ft-prefix=$PWD/../000res

make clean

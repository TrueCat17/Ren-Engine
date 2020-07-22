#!/bin/bash
set -e

SDL2_DIR="SDL2-2.0"
JPEG_DIR="jpeg-9c"
LIBPNG_DIR="libpng-1.6.37"
LIBWEBP_DIR="libwebp-1.0.3"

export CC="gcc"
export CFLAGS="-O2 -flto"
export CPPFLAGS="-I$PWD/../$SDL2_DIR/include -I$PWD/../$JPEG_DIR -I$PWD/../$LIBPNG_DIR -I$PWD/../$LIBWEBP_DIR/src"
export LDFLAGS="-lm -pthread -flto -L$PWD/../000res"

./configure \
	--enable-static \
	--disable-shared \
	--disable-sdltest \
	\
	--with-sdl-prefix=$PWD/../000res \
	\
	--disable-imageio \
	--disable-bmp \
	--disable-gif \
	--disable-lbm \
	--disable-pcx \
	--disable-pnm \
	--disable-svg \
	--disable-tga \
	--disable-tif \
	--disable-xcf \
	--disable-xpm \
	--disable-xv \
	\
	--enable-jpg \
	--enable-png \
	--enable-webp \
	\
	--disable-jpg-shared \
	--disable-png-shared \
	--disable-webp-shared

make clean

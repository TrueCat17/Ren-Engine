#!/bin/bash
set -e

export CC="gcc"
export CFLAGS="-O2 -flto"
export LDFLAGS="-lm -flto -pthread"

./autogen.sh

./configure \
	--disable-shared \
	--enable-static \
	\
	--disable-gl \
	--disable-sdl \
	--disable-png \
	--disable-jpeg \
	--disable-tiff \
	--disable-gif \
	--disable-wic

make clean

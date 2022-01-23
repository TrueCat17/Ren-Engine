#!/bin/bash
set -e

export CC="gcc"
export CFLAGS="-O2 -flto"
export LDFLAGS="-lm -flto"

./configure \
	--target-os=mingw32 \
	--cpu=I686 \
	--enable-gpl --enable-version3 \
	\
	--cc="$CC" \
	--enable-static \
	--disable-shared \
	\
	--disable-all \
	--disable-network \
	--enable-avcodec \
	--enable-avformat \
	--enable-swresample \
	\
	--enable-decoder=mp3 \
	--enable-decoder=opus \
	--enable-decoder=vorbis \
	\
	--enable-protocol=file \
	\
	--enable-demuxer=mp3 \
	--enable-demuxer=ogg \
	\
	--disable-alsa \
	--disable-iconv \
	\
	--disable-libxcb \
	--disable-libxcb-shm \
	--disable-libxcb-xfixes \
	--disable-libxcb-shape \
	\
	--disable-sndio \
	--disable-sdl2 \
	\
	--disable-xlib \
	--disable-zlib \
	\
	--disable-d3d11va \
	--disable-dxva2 \
	--disable-cuda-llvm \
	--disable-vaapi \
	--disable-v4l2-m2m
	
make clean

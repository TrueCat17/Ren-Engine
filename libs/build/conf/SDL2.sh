#!/bin/bash
set -e

export CC="gcc"
export CFLAGS="-O2 -flto"
export LDFLAGS="-lm -flto"

./configure \
	--host=mingw32 \
	--enable-sse2 \
	\
	--enable-static \
	--disable-shared \
	\
	--disable-haptic \
	--disable-sensor \
	--disable-power \
	\
	--disable-oss --disable-jack --disable-sndio --disable-esd \
	--disable-alsatest --disable-esdtest \
	--disable-arts --disable-nas --disable-fusionsound --disable-diskaudio \
	--disable-libsamplerate \
	\
	--disable-video-wayland \
	--disable-video-rpi \
	--disable-video-x11-scrnsaver \
	--disable-video-vivante \
	--disable-video-cocoa \
	--disable-render-metal \
	--disable-video-vulkan \
	--disable-video-directfb \
	--disable-directx \
	--disable-wasapi \
	--disable-video-opengles \
	--disable-video-opengles1 \
	--disable-video-opengles2 \
	\
	--disable-video-x11-xinerama \
	--disable-video-x11-xdbe \
	--disable-input-tslib \
	\
	--disable-rpath \
	--disable-render-d3d

make clean

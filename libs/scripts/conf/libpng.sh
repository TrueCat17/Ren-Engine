#!/bin/bash
set -e

ZLIB_DIR='zlib-1.2.11'

export CC="gcc"
export CFLAGS="-O2 -flto"
export CPPFLAGS="-I$PWD/../$ZLIB_DIR"
export LDFLAGS="-lm -flto -L$PWD/../000res"

./configure \
	--disable-shared \
	--enable-static \
	\
	--enable-hardware-optimizations \
	--enable-intel-sse

make clean

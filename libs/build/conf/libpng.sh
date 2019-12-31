#!/bin/bash
set -e

export CC="gcc"
export CFLAGS="-O2 -flto"
export LDFLAGS="-lm -flto"

./configure \
	--disable-shared \
	--enable-static \
	\
	--enable-hardware-optimizations \
	--enable-intel-sse

make clean

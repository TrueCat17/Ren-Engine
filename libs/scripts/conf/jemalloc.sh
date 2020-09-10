#!/bin/bash
set -e

./autogen.sh

export CC="gcc"
export CFLAGS="-O2 -flto"
export LDFLAGS="-lm -flto"

./configure \
	--enable-static \
	--disable-shared \
	--disable-stats \
	--disable-cxx

make clean

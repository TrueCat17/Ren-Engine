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
	--disable-fill \
	--disable-doc \
	--disable-cxx

make clean

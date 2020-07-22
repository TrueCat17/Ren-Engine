#!/bin/bash
set -e

export CC="gcc"
export CFLAGS="-O2 -flto"
export LDFLAGS="-lm -flto"

./configure \
	--disable-shared \
	--enable-static \
	--with-bzip2=no \
	--with-brotli=yes

make clean

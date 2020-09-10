#!/bin/bash
set -e

BROTLI_DIR="brotli"

export CC="gcc"
export CFLAGS="-O2 -flto"
export LDFLAGS="-lm -flto"

export BROTLI_CFLAGS="-I$PWD/../$BROTLI_DIR/c/include"
export BROTLI_LIBS="-L$PWD/../000res"

./configure \
	--disable-shared \
	--enable-static \
	--with-bzip2=no \
	--with-brotli=yes

make clean

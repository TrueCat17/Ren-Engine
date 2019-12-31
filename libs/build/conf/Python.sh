#!/bin/bash
set -e

export CC="gcc"
export CFLAGS="-O2 -flto"
export LDFLAGS="-lm -flto -static"

./configure \
	--disable-shared \
	\
	--enable-optimizations \
	--with-lto \
	\
	--disable-ipv6 \
	--enable-unicode=ucs4

make clean

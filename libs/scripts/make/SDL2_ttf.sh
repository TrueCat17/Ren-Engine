#!/bin/bash

rm -f ./.libs/libSDL2_ttf.a
make -j4

if [ ! -f ./.libs/libSDL2_ttf.a ]; then
	exit 1
fi

cp ./.libs/libSDL2_ttf.a ../000res

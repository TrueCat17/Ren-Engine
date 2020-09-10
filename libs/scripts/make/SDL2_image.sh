#!/bin/bash

rm -f ./.libs/libSDL2_image.a
make -j4

if [ ! -f ./.libs/libSDL2_image.a ]; then
	exit 1
fi

cp ./.libs/libSDL2_image.a ../000res

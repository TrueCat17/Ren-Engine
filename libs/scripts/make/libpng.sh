#!/bin/bash
set -e

make -j4
cp ./.libs/libpng16.a ../000res

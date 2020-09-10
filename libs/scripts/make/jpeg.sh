#!/bin/bash
set -e

make -j4
cp ./.libs/libjpeg.a ../000res

#!/bin/bash
set -e

make -j4
cp ./src/.libs/libwebp.a ../000res

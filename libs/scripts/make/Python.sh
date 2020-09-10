#!/bin/bash
set -e

make -j4 LDFLAGS="-lm"
cp ./libpython2.7.a ../000res

#!/bin/bash
set -e

make -j4
cp ./lib/libjemalloc.a ../000res

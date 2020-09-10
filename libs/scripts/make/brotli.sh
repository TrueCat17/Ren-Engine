#!/bin/bash
set -e

cd ./out
cmake --build . --config Release --target install -j4

cd ./installed/lib
cp ./libbrotlicommon-static.a ../../../../000res
cp ./libbrotlidec-static.a ../../../../000res
# 4 [..] = lib, installed, out, brotli

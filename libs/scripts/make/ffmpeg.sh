#!/bin/bash
set -e

make -j4
cp ./libavcodec/libavcodec.a       ../000res
cp ./libavformat/libavformat.a     ../000res
cp ./libavutil/libavutil.a         ../000res
cp ./libswresample/libswresample.a ../000res

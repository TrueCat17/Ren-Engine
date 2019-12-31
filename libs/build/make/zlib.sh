#!/bin/bash
set -e

make -j4
cp ./libz.a ../000res

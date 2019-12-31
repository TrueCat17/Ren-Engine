#!/bin/bash
set -e

make -j4
cp ./objs/.libs/libfreetype.a ../000res

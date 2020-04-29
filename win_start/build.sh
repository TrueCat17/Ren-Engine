#!/bin/bash
set -e

CC=i686-w64-mingw32-gcc-posix

$CC -O2 -c main.c -o main.o
$CC -mwindows -s -static -static-libgcc main.o -o start.exe

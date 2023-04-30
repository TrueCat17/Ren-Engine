#!/bin/bash
set -e

CC=i686-w64-mingw32-gcc

$CC -Os -c main.c -o main.o
$CC -s -static -static-libgcc main.o -o start.exe

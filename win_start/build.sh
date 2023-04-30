#!/bin/bash
set -e

CC=i686-w64-mingw32-gcc

$CC -Os -c main.c -o main.o
$CC -s -mwindows -static -static-libgcc main.o -o start.exe # -mwindows = dont show console

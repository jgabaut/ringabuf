#!/bin/sh

CC="gcc"
SOURCE="./demo/main.c"
TARGET="ringabuf-demo"
FLAGS="-Wall"

"$CC" "$SOURCE" -o "$TARGET" "$FLAGS"

#!/bin/bash
set -e
if [ -z "$1" ]; then
  echo "Specify the target in arg 1"
  exit
fi
echo "$1"
rm -rf resources/generated/shaders/$1
mkdir -p resources/generated/shaders/$1
prof=""
if [ ! -z "$2" ]; then
  prof="-profile $2"
fi
ext=$1
if [ ! -z "$3" ]; then
  ext=$3
fi

slangc resources/shader/basic.slang \
  -no-mangle \
  -entry main -stage vertex \
  -o resources/generated/shaders/$1/basic.vert.$ext # -target $1 -o generated/shaders/$1/basicFrag.$1

slangc resources/shader/basic.slang \
  -no-mangle \
  -entry mainFrag -stage pixel \
  -o resources/generated/shaders/$1/basic.frag.$ext

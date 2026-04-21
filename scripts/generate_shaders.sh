#!/bin/bash
set -e
if [ -z "$1" ]; then
  echo "Specify the target in arg 1"
  exit
fi
echo "$1"
rm -rf generated/shaders/$1
mkdir -p generated/shaders/$1
prof=""
if [ ! -z "$2" ]; then
  prof="-profile $2"
fi

slangc resources/shader/basic.slang \
  -entry main -stage vertex \
  -o generated/shaders/$1/basic.vert.$1 # -target $1 -o generated/shaders/$1/basicFrag.$1

slangc resources/shader/basic.slang \
  -entry mainFrag -stage fragment \
  -o generated/shaders/$1/basic.frag.$1

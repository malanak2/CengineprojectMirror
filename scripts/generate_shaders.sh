#!/bin/bash
set -e
if [ -z "$1" ]; then
  echo "Specify the target in arg 1"
  exit
fi
echo "$1"
rm -rf generated/shaders/$1
mkdir -p generated/shaders/$1
if [ ! -z "$2" ]; then
  prof="-profile $2"
fi
slangc resources/shader/basic.slang $prof -target $1 -entry main -o generated/shaders/$1/basic.$1

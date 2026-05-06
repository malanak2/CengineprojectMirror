#!/bin/bash
set -e

slangc resources/shader/basic.slang \
  -no-mangle \
  -fvk-use-gl-layout \
  -o resources/generated/shaders/spirv/basic.spv # -target $1 -o generated/shaders/$1/basicFrag.$1

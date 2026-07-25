#!/bin/bash

shaderDir="resources/shader"
spirvDir="resources/generated/shaders/spirv"

if [ -d "$spirvDir" ]; then
  rm -rf "$spirvDir"
fi

mkdir -p "$spirvDir"

if [ ! -d "$shaderDir" ]; then
  echo "Error: Source directory '$shaderDir' does not exist!" >&2
  exit 1
fi

for inputFile in "$shaderDir"/*; do
  if [ -f "$inputFile" ]; then
    basename_with_ext=$(basename "$inputFile")
    filename="${basename_with_ext%.*}"

    outputFile="$spirvDir/$filename.spv"

    echo "Compiling: $basename_with_ext -> $filename.spv"

    slangc "$inputFile" \
      -no-mangle \
      -fvk-use-gl-layout \
      -o "$outputFile"
  fi
done

echo -e "\e[32mShader compilation complete!\e[0m"

#!/usr/bin/env bash

# Publish.sh: copy headers and binaries, define preprocessor flags
# Usage: ./Publish.sh [BuildDir]
# Default BuildDir: ./out/build/clang

set -e  # exit on error
set -u  # treat unset variables as error

BuildDir="${1:-$(pwd)/out/build/clang}"
PublishDir="$BuildDir/publish"
IncludeDir="$PublishDir/include"
BinDir="$PublishDir/bin"

mkdir -p "$IncludeDir" "$BinDir"

CacheFile="$BuildDir/CMakeCache.txt"

function get_cmake_var() {
    local name="$1"
    local line
    line=$(grep -E "^${name}:" "$CacheFile" || true)
    if [ -n "$line" ]; then
        echo "${line#*=}"
    fi
}

USE_ASSIMP=$(get_cmake_var "USE_ASSIMP")
USE_BULLET=$(get_cmake_var "USE_BULLET")

echo "USE_ASSIMP = $USE_ASSIMP"
echo "USE_BULLET = $USE_BULLET"

rsync -av --include '*/' --include '*.h' --include '*.hpp' --exclude '*' ./src/ "$IncludeDir/" > /dev/null

if [ -f "$BuildDir/Syng.so" ]; then
    cp -f "$BuildDir/Syng.so" "$BinDir/"
elif [ -f "$BuildDir/Syng.dylib" ]; then
    cp -f "$BuildDir/Syng.dylib" "$BinDir/"
else
    echo "Warning: Syng shared library not found!"
fi

Header="$IncludeDir/Syngine/Syngine.hpp"

if [ -f "$Header" ]; then
    TempFile="$(mktemp)"
    awk -v assimp="$USE_ASSIMP" -v bullet="$USE_BULLET" '
    { print }
    /^#pragma once$/ {
        if (assimp == "ON") print "#define USE_ASSIMP"
        if (bullet == "ON") print "#define USE_BULLET"
    }' "$Header" > "$TempFile"
    mv "$TempFile" "$Header"
fi

#!/bin/bash

set -e

./platforms/macos/arm64/external.sh

cmake -DPLATFORM=macos -DARCH=arm64 -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build

rm -rf ppuc
mkdir ppuc

cp build/ppuc_pinmame ppuc/
cp -P third-party/runtime-libs/macos/arm64/*.dylib ppuc/

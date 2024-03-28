#!/bin/bash

set -e

./platforms/linux/x64/external.sh

cmake -DPLATFORM=linux -DARCH=x64 -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build

rm -rf ppuc
mkdir ppuc

cp build/ppuc_pinmame ppuc/
cp -P third-party/runtime-libs/linux/x64/*.so* ppuc/

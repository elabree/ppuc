#!/bin/bash

set -e

./platforms/linux/aarch64/external.sh

cmake -DPLATFORM=linux -DARCH=aarch64 -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build

rm -rf ppuc
mkdir ppuc

cp build/ppuc_pinmame ppuc/
cp -P third-party/runtime-libs/linux/aarch64/*.so* ppuc/

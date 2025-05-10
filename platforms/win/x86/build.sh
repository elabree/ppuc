#!/bin/bash

set -e

if [ -z "${BUILD_TYPE}" ]; then
   BUILD_TYPE="Release"
fi

BUILD_TYPE=${BUILD_TYPE} ./platforms/win/x86/external.sh

cmake -G "Visual Studio 17 2022" -A Win32 -DPLATFORM=win -DARCH=x86 -B build
cmake --build build --config ${BUILD_TYPE}

rm -rf ppuc
mkdir ppuc

cp build/${BUILD_TYPE}/ppuc-pinmame ppuc/
cp -P third-party/runtime-libs/win-x86/*.dll ppuc/

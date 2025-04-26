#!/bin/bash

set -e

LIBOPENAL_SHA=d3875f333fb6abe2f39d82caca329414871ae53b
LIBPINMAME_SHA=c69f68aca1fe28d5bb65ab10a17c09fb2593d57b
LIBPPUC_SHA=2bb464dd10e37649e0ec3321edfaf71b4c1d3216
LIBDMDUTIL_SHA=c7b28ff9b26d206820f438a54c9bc89171a3ae02

echo "Building libraries..."
echo "  LIBOPENAL_SHA: ${LIBOPENAL_SHA}"
echo "  LIBPINMAME_SHA: ${LIBPINMAME_SHA}"
echo "  LIBPPUC_SHA: ${LIBPPUC_SHA}"
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo ""

if [ -z "${BUILD_TYPE}" ]; then
   BUILD_TYPE="Release"
fi

if [ -z "${CACHE_DIR}" ]; then
   CACHE_DIR="external/cache/${BUILD_TYPE}"
fi

echo "Build type: ${BUILD_TYPE}"
echo "Cache dir: ${CACHE_DIR}"
echo ""

mkdir -p external ${CACHE_DIR}
cd external

#
# libdmdutil
#

CACHE_NAME="libdmdutil-${LIBDMDUTIL_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
    rm -f ../${CACHE_DIR}/libdmdutil-*.cache
    rm -rf libdmdutil-*
    curl -sL https://github.com/PPUC/libdmdutil/archive/${LIBDMDUTIL_SHA}.zip -o libdmdutil.zip
    unzip libdmdutil.zip
    cd libdmdutil-${LIBDMDUTIL_SHA}
    cp -r include/DMDUtil ../../third-party/include/
    BUILD_TYPE=${BUILD_TYPE} platforms/win/x64/external.sh
    cp -a third-party/. ../../third-party
    cmake \
      -G "Visual Studio 17 2022" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=OFF \
      -DBUILD_STATIC=ON \
      -B build
    cmake --build build --config ${BUILD_TYPE}
    cp build/${BUILD_TYPE}/dmdutil64.lib ../../third-party/build-libs/win/x64/
    cp build/${BUILD_TYPE}/dmdutil64.dll ../../third-party/runtime-libs/win/x64/
    cd ..
    touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

#
# libpiname
#

CACHE_NAME="pinmame-${LIBPINMAME_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
    rm -f ../${CACHE_DIR}/pinmame-*.cache
    rm -rf pinmame-*
    curl -sL https://github.com/vpinball/pinmame/archive/${LIBPINMAME_SHA}.zip -o pinmame.zip
    unzip pinmame.zip
    cd pinmame-${LIBPINMAME_SHA}
    cp src/libpinmame/libpinmame.h ../../third-party/include/
    cp cmake/libpinmame/CMakeLists.txt CMakeLists.txt
    cmake \
      -G "Visual Studio 17 2022" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=OFF \
      -DBUILD_STATIC=ON \
      -B build
    cmake --build build --config ${BUILD_TYPE}
    cp build/${BUILD_TYPE}/pinmame64.lib ../../third-party/build-libs/win/x64/
    cp build/${BUILD_TYPE}/pinamme64.dll ../../third-party/runtime-libs/win/x64/
    cd ..
    touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

#
# libppuc
#

CACHE_NAME="libppuc-${LIBPPUC_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
    rm -f ../${CACHE_DIR}/libppuc-*.cache
    rm -rf libppuc-*
    curl -sL https://github.com/PPUC/libppuc/archive/${LIBPPUC_SHA}.zip -o libppuc.zip
    unzip libppuc.zip
    cd libppuc-${LIBPPUC_SHA}
    cp src/PPUC.h ../../third-party/include/
    cp src/PPUC_structs.h ../../third-party/include/
    BUILD_TYPE=${BUILD_TYPE} platforms/win/x64/external.sh
    cp -a third-party/. ../../third-party
    cmake \
      -G "Visual Studio 17 2022" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=OFF \
      -DBUILD_STATIC=ON \
      -B build
    cmake --build build --config ${BUILD_TYPE}
    cp build/${BUILD_TYPE}/ppuc64.lib ../../third-party/build-libs/win/x64/
    cp build/${BUILD_TYPE}/ppuc64.dll ../../third-party/runtime-libs/win/x64/
    cd ..
    touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

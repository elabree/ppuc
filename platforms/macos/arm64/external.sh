#!/bin/bash

set -e

LIBCARGS_SHA=6535721d9f3373cf8f37df4df62957e14620d4d5
LIBOPENAL_SHA=d3875f333fb6abe2f39d82caca329414871ae53b
LIBPINMAME_SHA=4ab8bfc9436f7ff54983f2daea87f76bde64c0fe
LIBPPUC_SHA=ad4af67c58dd214e34a630b4503d1b751be50b38
LIBDMDUTIL_SHA=cbda0fda8cb2cd16b90fdc24fb001f2f7f0a3e1a

NUM_PROCS=$(sysctl -n hw.ncpu)

echo "Building libraries..."
echo "  LIBCARGS_SHA: ${LIBCARGS_SHA}"
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
echo "Procs: ${NUM_PROCS}"
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
    BUILD_TYPE=${BUILD_TYPE} platforms/macos/arm64/external.sh
    rsync -a third-party/ ../../third-party
    cmake -DPLATFORM=macos -DARCH=arm64 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
    cmake --build build -- -j${NUM_PROCS}
    cp -P build/*.dylib ../../third-party/runtime-libs/macos/arm64/
    cd ..
    touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

#
# libcargs
#

CACHE_NAME="cargs-${LIBCARGS_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
    rm -f ../${CACHE_DIR}/cargs-*.cache
    rm -rf cargs-*
    curl -sL https://github.com/likle/cargs/archive/${LIBCARGS_SHA}.zip -o cargs.zip
    unzip cargs.zip
    cd cargs-${LIBCARGS_SHA}
    cmake \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DCMAKE_OSX_ARCHITECTURES=arm64 \
        -B build
    cmake --build build -- -j${NUM_PROCS}
    cp include/cargs.h ../../third-party/include/
    cp -a build/*.dylib ../../third-party/runtime-libs/macos/arm64/
    cd ..
    touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

#
# libopenal
#

CACHE_NAME="openal-soft-${LIBOPENAL_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
    rm -f ../${CACHE_DIR}/openal-soft-*.cache
    rm -rf openal-soft-*
    curl -sL https://github.com/kcat/openal-soft/archive/${LIBOPENAL_SHA}.zip -o openal-soft.zip
    unzip openal-soft
    cd openal-soft-${LIBOPENAL_SHA}
    cp -r include/AL ../../third-party/include/
    cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_OSX_ARCHITECTURES=arm64 -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_INSTALL_EXAMPLES=OFF -DALSOFT_INSTALL_UTILS=OFF -B build
    cmake --build build -- -j${NUM_PROCS}
    cp -P build/libopenal*.dylib ../../third-party/runtime-libs/macos/arm64/
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
    cp cmake/libpinmame/CMakeLists_osx-arm64.txt CMakeLists.txt
    cmake -DPPUC_SUPPORT=1 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_OSX_ARCHITECTURES=arm64 -B build
    cmake --build build -- -j${NUM_PROCS}
    cp -P build/libpinmame*.dylib ../../third-party/runtime-libs/macos/arm64/
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
    BUILD_TYPE=${BUILD_TYPE} platforms/macos/arm64/external.sh
    rsync -a third-party/ ../../third-party
    cmake -DPLATFORM=macos -DARCH=arm64 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
    cmake --build build -- -j${NUM_PROCS}
    cp -P build/libppuc*.dylib ../../third-party/runtime-libs/macos/arm64/
    cd ..
    touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

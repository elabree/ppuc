#!/bin/bash

set -e

LIBCARGS_SHA=6535721d9f3373cf8f37df4df62957e14620d4d5
LIBOPENAL_SHA=d3875f333fb6abe2f39d82caca329414871ae53b
LIBPINMAME_SHA=7bb409ec08cf9e4221a1463f58defcfb4e558437
LIBPPUC_SHA=ad4af67c58dd214e34a630b4503d1b751be50b38
LIBDMDUTIL_SHA=b7fce4f3a5c73acca9a1f4858500b8c4e2874119

NUM_PROCS=$(nproc)

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
    BUILD_TYPE=${BUILD_TYPE} platforms/linux/aarch64/external.sh
    rsync -a third-party/ ../../third-party
    cmake -DPLATFORM=linux -DARCH=aarch64 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
    cmake --build build -- -j${NUM_PROCS}
    cp -P build/*.so* ../../third-party/runtime-libs/linux/aarch64/
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
      -B build
    cmake --build build -- -j${NUM_PROCS}
    cp include/cargs.h ../../third-party/include/
    cp -a build/*.so ../../third-party/runtime-libs/linux/aarch64/
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
    cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_INSTALL_EXAMPLES=OFF -DALSOFT_INSTALL_UTILS=OFF -B build
    cmake --build build -- -j${NUM_PROCS}
    cp -P build/libopenal*.so* ../../third-party/runtime-libs/linux/aarch64/
    cd ..
    touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

#
# libpinmame
#

CACHE_NAME="pinmame-${LIBPINMAME_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
    rm -f ../${CACHE_DIR}/pinmame-*.cache
    rm -rf pinmame-*
    curl -sL https://github.com/vpinball/pinmame/archive/${LIBPINMAME_SHA}.zip -o pinmame.zip
    unzip pinmame.zip
    cd pinmame-${LIBPINMAME_SHA}
    cp src/libpinmame/libpinmame.h ../../third-party/include/
    cp cmake/libpinmame/CMakeLists_linux-x64.txt CMakeLists.txt
    cmake -DPPUC_SUPPORT=1 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
    cmake --build build -- -j${NUM_PROCS}
    cp -P build/libpinmame*.so* ../../third-party/runtime-libs/linux/aarch64/
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
    BUILD_TYPE=${BUILD_TYPE} platforms/linux/aarch64/external.sh
    rsync -a third-party/ ../../third-party
    cmake -DPLATFORM=linux -DARCH=aarch64 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
    cmake --build build -- -j${NUM_PROCS}
    cp -P build/libppuc*.so* ../../third-party/runtime-libs/linux/aarch64/
    cd ..
    touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

#!/bin/bash

set -e

LIBCARGS_SHA=5949a20a926e902931de4a32adaad9f19c76f251
LIBOPENAL_SHA=d3875f333fb6abe2f39d82caca329414871ae53b
LIBPINMAME_SHA=master
LIBPPUC_SHA=main
LIBDMDUTIL_SHA=master

echo "Building libraries..."
echo "  LIBCARGS_SHA: ${LIBCARGS_SHA}"
echo "  LIBOPENAL_SHA: ${LIBOPENAL_SHA}"
echo "  LIBPINMAME_SHA: ${LIBPINMAME_SHA}"
echo "  LIBPPUC_SHA: ${LIBPPUC_SHA}"
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo ""

rm -rf external
mkdir external
cd external

#
# libdmdutil
#

curl -sL https://github.com/vpinball/libdmdutil/archive/${LIBDMDUTIL_SHA}.zip -o libdmdutil.zip
unzip libdmdutil.zip
cd libdmdutil-${LIBDMDUTIL_SHA}
cp -r include/DMDUtil ../../third-party/include/
platforms/linux/x64/external.sh
rsync -a third-party/ ../../third-party
cmake -DPLATFORM=linux -DARCH=x64 -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
cp build/*.a ../../third-party/build-libs/linux/x64/
cp -P build/*.so* ../../third-party/runtime-libs/linux/x64/
cd ..

#
# libcargs
#

curl -sL https://github.com/likle/cargs/archive/${LIBCARGS_SHA}.zip -o cargs.zip
unzip cargs.zip
cd cargs-${LIBCARGS_SHA}
cp include/cargs.h ../../third-party/include/
mkdir build
cd build
cmake ..
make
cp libcargs.a ../../../third-party/build-libs/linux/x64/
cmake -DBUILD_SHARED_LIBS=ON ..
make
cp -P libcargs.so* ../../../third-party/runtime-libs/linux/x64/
cd ../..

#
# libopenal
#

curl -sL https://github.com/kcat/openal-soft/archive/${LIBOPENAL_SHA}.zip -o openal-soft.zip
unzip openal-soft
cd openal-soft-${LIBOPENAL_SHA}
cp -r include/AL ../../third-party/include/
cmake -DCMAKE_BUILD_TYPE=Release -DLIBTYPE=STATIC -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_INSTALL_EXAMPLES=OFF -DALSOFT_INSTALL_UTILS=OFF -B build
cmake --build build
cp build/libopenal.a ../../third-party/build-libs/linux/x64/
rm -rf build
cmake -DCMAKE_BUILD_TYPE=Release -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_INSTALL_EXAMPLES=OFF -DALSOFT_INSTALL_UTILS=OFF -B build
cmake --build build
cp -P build/libopenal*.so* ../../third-party/runtime-libs/linux/x64/
cd ..

#
# libpiname
#

curl -sL https://github.com/vpinball/pinmame/archive/${LIBPINMAME_SHA}.zip -o pinmame.zip
unzip pinmame.zip
cd pinmame-${LIBPINMAME_SHA}
cp src/libpinmame/libpinmame.h ../../third-party/include/
cp cmake/libpinmame/CMakeLists_linux-x64.txt CMakeLists.txt
cmake -DPPUC_SUPPORT=1 -DCMAKE_BUILD_TYPE=Release -B build/Release
cmake --build build/Release
#cp build/Release/libpinmame.a ../../third-party/build-libs/linux/x64/
cp -P build/Release/libpinmame*.so* ../../third-party/runtime-libs/linux/x64/
cd ..

#
# libppuc
#

curl -sL https://github.com/PPUC/libppuc/archive/${LIBPPUC_SHA}.zip -o libppuc.zip
unzip libppuc.zip
cd libppuc-${LIBPPUC_SHA}
cp src/PPUC.h ../../third-party/include/
platforms/linux/x64/external.sh
rsync -a third-party/ ../../third-party
cmake -DPLATFORM=linux -DARCH=x64 -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
cp build/libppuc.a ../../third-party/build-libs/linux/x64/
cp -P build/libppuc*.so* ../../third-party/runtime-libs/linux/x64/
cd ..

